//
//  fl_gui.cpp
//  nprobe
//
//  Created by Ed Cole on 05/02/2020.
//  Copyright © 2020 colege. All rights reserved.
//

#include "fl_gui.hpp"
#include "neohub.hpp"

#define LEN_STATUS_LABEL 32

extern bool debug;

// the scrollable browser of widgets is based on that
// published by erco. see http://seriss.com/people/erco

const int idWidth = 120;
const int temperatureWidth = 40;
const int hhoursW = 25;
const int htempW = 25;
const int defaultHeight = 25;


std::ostringstream strCout;     // string stream for cout redirection
                                // used to display std::cout calls to the
                                // GUI console window (a Fl_Browser widget)

//****************************************
//
// Class   ScrollItem (a row), one per neostat
//
class ScrollItem : public Fl_Group {
    
    friend int      gui(Neohub*);
    friend void     hold_cb(Fl_Widget*, void*);
    friend void     sync_cb(Fl_Widget*, void *);
    friend void     showStats_cb(Fl_Widget*, void *);
    friend void     showTimers_cb(Fl_Widget*, void *);
    friend class    MyScroll;
    
    Fl_Box*     id;             // Thermostat ID i.e. it's name
    char        id_str[32];
    Fl_Box*     temperature;    // Actual temp recorded by thermostat
    char        temp_str[5];
    Fl_Box*     set_temp;       // Set temperature of thermostat
    char        set_temp_str[5];
    Fl_Input*   hhours;         // HOLD time in hours
    char        hhours_str[5];
    Fl_Input*   htemp;          // HOLD temperature in DegC
    char        htemp_str[5];
    Fl_Box*     infoBox;     // Information box displaying thermostat status
    
    bool        timerON;
    
    Stat*       stat;           // pointer to the Stat object associated with the row
    Timer*      timer;
    bool        isStat;            // true if item is a thermostat, false if it's a timer.
    char        status_label[LEN_STATUS_LABEL];  // text of the info box
   
public:
    //
    // CONSTRUCTOR
    //
    ScrollItem(int X, int Y, int W, int H, const char* L=0,bool st=true) : Fl_Group(X,Y,W,H) {
        int infoXpos,infoWidth;
        int posOffset = 0;  // horizontal offset for widget placement
        begin();
        
            // Fixed width box - The Name (or id) for the neostat
            //
            id = new Fl_Box(X,Y,idWidth,defaultHeight,L);
            id->box(FL_UP_BOX);
            posOffset+=idWidth;
            //
            // Fixed width box - Actual temperature
            //
            temperature = new Fl_Box(X+posOffset,Y,temperatureWidth,defaultHeight,"");
            temperature->box(FL_UP_BOX);
            posOffset+= temperatureWidth; // X 2 to make room for label on left
            if(st){         // A Stat; we dont have this box for a timer
                //
                // Fixed width box  Set temperature
                //
                set_temp = new Fl_Box(X+posOffset,Y,temperatureWidth,defaultHeight,"");
                set_temp->box(FL_UP_BOX);
            }
            posOffset+= 2 * temperatureWidth + 10; // X 2 to make room for label on left
            //
            // Input  HOLD time in hours
            //
            hhours = new Fl_Input(X+posOffset,Y,hhoursW,defaultHeight,"HOLD");
            hhours->box(FL_UP_BOX);
            posOffset+= 3 * hhoursW;
            if(st){         // A Stat; we dont have this box for a timer
                //
                // Input  HOLD temperature in DegC
                //
                htemp = new Fl_Input(X+posOffset,Y,htempW,defaultHeight,"h @");
                htemp->box(FL_UP_BOX);
            }
            posOffset+=htempW;
            //
            // info box. Fills remaining width. Displays status of neostat
            //
            infoXpos = X + posOffset;
            infoWidth = W - posOffset;
            infoBox = new Fl_Box(infoXpos,Y,infoWidth,defaultHeight, "");
            infoBox->box(FL_UP_BOX);
            infoBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
            resizable(infoBox);
        
        end();
    }// ScrollItem constructor
    
    
    //
    //      ScrollItem      updateData()
    //                      populates the values in ScrollItem widgets
    //
    //
    void updateData(){

        NeoStatBase* neostat;
        
        //Name
        if(isStat) strcpy(id_str,stat->getName().c_str());
            else   strcpy(id_str,timer->getName().c_str());
        id->label(id_str);
        //std::cout << id->label() << " ";    // TBD
               
        //Temperature for stat
        temp_str[0] = '\0';
        temperature->color(FL_BACKGROUND_COLOR);
        if(isStat) {
            // Stat temperature
            strcpy(temp_str,stat->curr_temp.c_str());
            temperature->label(temp_str);
            if (stat->isOn()){ // if heat is on set background ro RED
               temperature->color(FL_RED);
            }
            //std::cout << temperature->label() << " ";   // TBD
            // Stat Set Temperature
            strcpy(set_temp_str,stat->curr_set_temp.c_str());
            set_temp->label(set_temp_str);
            //std::cout << set_temp->label() << std::endl;    // TBD
        }else{
            // Timer "Temperature"
            if (timer->isOn()){
                strcpy(temp_str,"ON");
                temperature->label(temp_str);
                temperature->color(FL_RED);
                //std::cout << "ON" << std::endl; // TBD
            }else{
               
                strcpy(temp_str,"OFF");
                temperature->label(temp_str);
                //std::cout << "OFF" << std::endl; // TBD
            }
            temperature->label(temp_str);
        }
        
        // clear HOLD input fields
        hhours->value("");
        if(isStat) htemp->value("");
        
        // Status box
        
        // build string
        std::ostringstream status;
        
        if(isStat){
            neostat = (NeoStatBase*)stat;
        }else{
            neostat = (NeoStatBase*)timer;
        }
        if ( neostat->holdTimeHours<int>() + neostat->holdTimeMins<int>() ){
                // hold time remaining != 0
            status  << "Holding for "
                    << neostat->holdTimeHours<std::string>()
                    << ":"
                    << neostat->holdTimeMins<std::string>();
        }else{
            status << "Neostat programming";
        }
        
        strncpy(status_label, status.str().c_str(),LEN_STATUS_LABEL);
        status_label[LEN_STATUS_LABEL - 1] = '\0'; // in case strncpy() overrun
        infoBox->label(status_label);
        
        redraw();
       }// updateData()
    
}; // Class ScrollItem

//***************************************
//
// Class MyScroll
//
class MyScroll : public Fl_Scroll {
    
    friend void sync_cb(Fl_Widget*, void *);
    friend void hold_cb(Fl_Widget*, void *);
    friend void showStats_cb(Fl_Widget*, void *);
    friend void showTimers_cb(Fl_Widget*, void *);
    friend  int gui(Neohub*);
    
    int         nchild;                     // number of children / ScrollItems
    bool        visible;
    Neohub*     neohub;
    Fl_Browser* console;                    // pointer to the console
    
    std::vector<ScrollItem*> scrollPtrs;    // pointers to children
    
public:
    //
    // MyScroll CONSTRUCTOR
    //
    MyScroll(int X, int Y, int W, int H, const char* L=0) : Fl_Scroll(X,Y,W,H,L) {
        nchild = 0;
        visible = false;
    }
    
    
    //  MyScroll    resize()
    //
    void resize(int X, int Y, int W, int H) {
        // Tell children to resize to our new width
        for ( int t=0; t<nchild; t++ ) {
            Fl_Widget *w = child(t);
            w->resize(w->x(), w->y(), W-20, w->h());    // W-20: leave room for scrollbar
        }
        // Tell scroll that children changed in size
        init_sizes();
        Fl_Scroll::resize(X,Y,W,H);
    }// resize()
    
    // MyScroll     AddItem()
    // Append new scrollitem to bottom
    //     Note: An Fl_Pack would be a good way to do this too
    //
    ScrollItem* AddRow(NeoStatBase* neostat, bool st) {
        
        int X = x() + 1,
            Y = y() - yposition() + (nchild*defaultHeight) + 1,
            W = w() - 20,            // -20: compensate for vscroll bar
            H = defaultHeight;
       
        ScrollItem* thisRow = new ScrollItem(X,Y,W,H,"",st);
        if(st){
            thisRow->stat = (Stat *)neostat;  // establish pointer to Neostat object
            thisRow->isStat = true;
        }else{
            thisRow->timer = (Timer *)neostat;
            thisRow->isStat = false;
        }
        thisRow->updateData();     // populate item with values from Stat
        add(thisRow);
        redraw();
        nchild++;
        return thisRow;
        
    }// AddItem()
    
};// Class MyScroll

// consoleOutput()
//
// - displays the contents of the redirected cout buffer to the console window
//
void consoleOutput(Fl_Browser* console){
    std::stringstream iss(strCout.str()); // copy redirected cout buffer into stringstream
    strCout.str("");
    strCout.clear();                      // clear the cout buffer
    
    // add the string stream to the console line by line
    while(iss.good())
    {
        std::string SingleLine;
        getline(iss,SingleLine,'\n');
        if(!SingleLine.empty()){
            console->add(SingleLine.c_str());
        }
    }
    console->bottomline(console->size()+1);
    
    console->redraw();
}

//*************************************
//      CALLBACKS
//*************************************


// sync_cb()    re-sync gui with neohub
//
void sync_cb(Fl_Widget* wgt, void *data) {
    
    Neohub *myHub;
    Fl_Browser* console {nullptr};
    // scrolls is a pointer to a vector of scroll pointers
    std::vector<MyScroll*>  scrolls = *((std::vector<MyScroll*>*) data);
    MyScroll *firstScroll = (MyScroll*)scrolls[1];
    //
    myHub = firstScroll->neohub;
    myHub->init(); // re-load state from Neohub
    
    //MyScroll *scroll = (MyScroll*)data;
    MyScroll *scroll;
    
    ScrollItem *w;
    std::cout << "Syncing with Neohub [";
    for( auto it = scrolls.begin(); it != scrolls.end(); ++it){
        scroll = *it;
        console = scroll->console;
        for ( int t=0; t < scroll->nchild; t++ ) {
            w = (ScrollItem *)scroll->child(t);
            std::cout << "syncing " << w->id->label() << " ";   // TBD
            std::cout << ".";
            w->updateData();
        }
        scroll->redraw();
    }
    std::cout << "] complete" << std::endl;
    
    //scroll->toggle();
    consoleOutput(console);

}//sync_cb

// heartbeat_cb()
//
void heartbeat_cb(void* w) {
  sync_cb((Fl_Widget*)w,w);
  Fl::repeat_timeout(HEART_BEAT, heartbeat_cb,w);    // retrigger timeout
}


// hold_cb()    iterate thru rows (scrollItems) and if either hold-time or hold-temp
//              have been filled, hold the stat
//
void hold_cb(Fl_Widget* w, void *data) {
    
    // data is a pointer to a vector of scroll pointers
    std::vector<MyScroll*>  scrolls = *((std::vector<MyScroll*>*) data);
    
    ScrollItem*  thisItem;
    MyScroll* thisScroll;
    Stat* thisStat;
    Timer* thisTimer;
    Fl_Browser* console {nullptr};
    
    int hhours;
    int hmins = 0;
    int htemp;
    
    for( auto scroll = scrolls.begin(); scroll != scrolls.end(); ++scroll){
        thisScroll = *scroll;  // pointer to scroll on the vector
        console = thisScroll->console;
        for(auto it = thisScroll->scrollPtrs.begin(); it != thisScroll->scrollPtrs.end(); ++it){
            thisItem = *it;   // pointer to a row on the scroll
            if (thisItem->isStat){ // HOLD Stat
                //thisStat = &(*(thisItem->stat));
                thisStat = thisItem->stat;
                htemp = atoi(thisItem->htemp->value());
                hhours = atoi(thisItem->hhours->value());
                if((htemp > 0) || (hhours > 0)){
                    //std::cout << thisItem->id_str << std::endl; // TBD
                    if (htemp == 0) htemp = DEF_HOLD_TEMP;
                    if (hhours == 0 ) hhours = DEF_HOLD_HOURS;
                    //std::cout << "holding" << thisStat->getName() << std::endl; // TBD
                    thisStat->hold(htemp, hhours, hmins);
                }
            }else{                  // HOLD Timer
                //thisTimer = &(*(thisItem->timer));
                thisTimer = thisItem->timer;
                //htemp = atoi(thisItem->htemp->value());
                hhours = atoi(thisItem->hhours->value());
                if(hhours > 0){
                    //std::cout << thisItem->id_str << std::endl; // TBD
                    //if (htemp == 0) htemp = DEF_HOLD_TEMP;
                    if (hhours == 0 ) hhours = DEF_HOLD_HOURS;
                    //std::cout << "holding" << thisTimer->getName() << std::endl; // TBD
                    thisTimer->holdOn(hhours * 60);
                }
            }
        }
    }
    
    consoleOutput(console);
    
    sync_cb(w, data);
    
}// hold_cb()

void showStats_cb(Fl_Widget* w, void *data){
    MyScroll* scroll = (MyScroll*)data;
    int nchild {1}; // local scope
    scroll->clear(); // clears scroll
    for ( ScrollItem* item : scroll->scrollPtrs){
        if( item->isStat){
            item->x(scroll->x()+1);
            item->y(scroll->y() - scroll->yposition() + (nchild*defaultHeight) + 1);
            item->w(scroll->w() - 20);
            item->h(defaultHeight);
            scroll->add(item);
            //scroll->redraw();
            ++nchild;
        }
    }
}

void showTimers_cb(Fl_Widget* w, void *data){
    MyScroll* scroll = (MyScroll*)data;
    int nchild {1}; // local scope
    scroll->clear(); // clears scroll
    scroll->init_sizes();
    for ( ScrollItem* item : scroll->scrollPtrs){
        if( !item->isStat){
            item->x(scroll->x()+1);
            item->y(scroll->y() - scroll->yposition() + (nchild*defaultHeight) + 1);
            item->w(scroll->w() - 20);
            item->h(defaultHeight);
            scroll->add(item);
            //scroll->redraw();
            ++nchild;
        }
    }
}

void exit_cb(void* w) {
    exit(0);
}

void menu_cb(void* w) {
  // menu callback template
    
}

void about_cb(void* w) {
  const char *text =
  "This text is pretty long, but will be\n"
  "concatenated into just a single string.\n"
  "The disadvantage is that you have to quote\n"
  "each part, and newlines must be literal as\n"
  "usual.";
    
    fl_message("%s", text);
    
}



void set_m_items(Fl_Menu_Bar* menu_bar, MyScroll* scroll){
    Fl_Menu_Item menuitems[] = {
        { "File", 0, 0, 0, FL_SUBMENU },
        { "Exit", FL_COMMAND + 'q', (Fl_Callback *)exit_cb},
        { 0 },
        { "View", 0, 0, 0, FL_SUBMENU },
        { "Thermostats", FL_COMMAND + 's', (Fl_Callback *)menu_cb, scroll, FL_MENU_DIVIDER },
        { "Timers", FL_COMMAND + 't', (Fl_Callback *)menu_cb, scroll },
        { 0 },
        { "&Help", 0, 0, 0, FL_SUBMENU },
        { "&Help", FL_COMMAND + 'h', (Fl_Callback *)menu_cb, menu_bar },
        { "&About", FL_COMMAND + 'a', (Fl_Callback *)about_cb, menu_bar },
        { 0 },
        { 0 }
    };
    menu_bar->copy(menuitems);
}


//***************************************************
//      The GUI function
//      The GUI function
//      The GUI function
//      The GUI function
//***************************************************

int    gui(Neohub* myHub){
    
    //reinstate this when youve fixed hold/sync buttons - ed 12/3/20
    // grab std::cout buffer for use in the console
    std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();   // save original cout buffer
    // reinstate with - std::cout.rdbuf( oldCoutStreamBuf ); // reinstate original buffer to cout
    std::cout.rdbuf( strCout.rdbuf() );                     // assign buffer to cout
    
    
    
    std::vector<Stat>* stats = myHub->getStats(); // pointer to vector of Thermostats
    std::vector<Timer>* timers = myHub->getTimers();
    
    std::vector<MyScroll*> scrolls;                 // vector of pointers to scrolls
    
    // Window
    //
    Fl_Double_Window *win = new Fl_Double_Window(600,700);
        win->label(GUI_TITLE);
        // Menu
        //
        
        // TABS
        Fl_Tabs *tabs = new Fl_Tabs(10,30,win->w()-20,win->h()-220);
        // THERMOSTAT TAB
        Fl_Group *statTab = new Fl_Group(10,50,win->w()-20,win->h()-220," Thermostats ");
            statTab->begin();
                MyScroll *scrollStats = new MyScroll(10,50,win->w()-20,win->h()-220);
        
                scrollStats->neohub = myHub;
                scrollStats->box(FL_BORDER_BOX);

                // Scroll Items
                //
                ScrollItem* i;
                // Stats
                Stat* thisStat;
                for(auto it = stats->begin(); it != stats->end(); ++it){
                        thisStat = &(*it); // thisStat is pointer to element in stats vector
                        i = scrollStats->AddRow(thisStat, true);
                        scrollStats->scrollPtrs.push_back(i); // push/copy pointer this scroll item
                }
            
            statTab->end();
            // TIMER TAB
            Fl_Group *TimerTab = new Fl_Group(10,50,win->w()-20,win->h()-220," Timers ");
            MyScroll *scrollTimers = new MyScroll(10,50,win->w()-20,win->h()-220);
            scrollTimers->neohub = myHub;
            Timer* thisTimer;
                for(auto it = timers->begin(); it != timers->end(); ++it){
                        thisTimer = &(*it); // thisTimer is pointer to element in timers vector
                        i = scrollTimers->AddRow(thisTimer, false);
                        scrollTimers->scrollPtrs.push_back(i); // push/copy pointer this scroll item
                }
                scrollTimers->end();
            TimerTab->end();
            tabs->end();
            // Menu
            //
            Fl_Menu_Bar *menu_bar = new Fl_Menu_Bar(0,0,600,25);
            set_m_items(menu_bar,scrollStats); //??
        
            // HOLD and SYNC buttons
            //
    
            Fl_Button *hold_butt = new Fl_Button(win->w()-250, win->h()-128, 100, 25, "Hold");
                    hold_butt->callback(hold_cb, (void*)&scrolls);
        
            // need to sort this out
            Fl_Button *sync_butt = new Fl_Button(win->w()-150, win->h()-128, 100, 25, "Sync");
                sync_butt->callback(sync_cb, (void*)&scrolls);
            
            // Console
            //
            Fl_Browser *console = new Fl_Browser(10, win->h()-100, win->w()-20, 95, "");
            console->box(FL_UP_BOX);
            console->has_scrollbar(Fl_Browser_::BOTH);
            
            console->add(VERSION);
            console->bottomline(console->size());
    
            scrollStats->console=console;  // set pointer to the console in scroll objects
            scrollTimers->console=console; // for reference by callbacks
    
            scrolls.push_back(scrollStats);
            scrolls.push_back(scrollTimers);
    
        win->end();
    
        win->resizable(scrollStats);
        win->show();
    
        // timeout callback to periodically resync with neohub
        // the problem here is that if user is filling out hold
        // fields and the heatbeat timeout calls sync before user
        // has pressed the hold button the edits will be deleted
        Fl::add_timeout(HEART_BEAT, heartbeat_cb, (void*)&scrolls);
    
    int ret = Fl::run();    // it shouldn't return from run()
    
    // but if it does
    
    std::cout.rdbuf( oldCoutStreamBuf ); // reinstate original buffer to cout
    std::cout << "window terminated" << std::endl;
    return(ret);
    
    
}// gui()
