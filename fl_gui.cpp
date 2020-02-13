//
//  fl_gui.cpp
//  nprobe
//
//  Created by Ed Cole on 05/02/2020.
//  Copyright © 2020 colege. All rights reserved.
//

#include "fl_gui.hpp"
#include "neohub.hpp"

// scrollable browser of widgets based on that published by erco
// see http://seriss.com/people/erco

const int idWidth = 120;
const int temperatureWidth = 40;
const int hhoursW = 25;
const int htempW = 25;
const int defaultHeight = 25;

std::string test = "test";

//
// Class   ScrollItem
//
class ScrollItem : public Fl_Group {
    
friend int      gui(Neohub*);
friend void     hold_cb(Fl_Widget*, void*);
friend void     sync_cb(Fl_Widget*, void *);
friend class    MyScroll;
    
    Fl_Box *id;
    char id_str[32];
    Fl_Box *temperature;
    char temp_str[5];
    Fl_Box* set_temp;
    char set_temp_str[5];
    Fl_Input *hhours;
    char hhours_str[5];
    Fl_Input *htemp;
    char htemp_str[5];
    
    Fl_Box *stretchBox;
    
    Stat*   stat;
    
    char status_label[32];
    
    
public:
    //
    // ScrollItem   CONSTRUCTOR
    //
    ScrollItem(int X, int Y, int W, int H, const char* L=0) : Fl_Group(X,Y,W,H) {
        int stretchXpos,stretchWidth;
        int posOffset = 0;
        begin();
            // Fixed width box
        
            id = new Fl_Box(X,Y,idWidth,defaultHeight,L);
            id->box(FL_UP_BOX);
            posOffset+=idWidth;
        
            temperature = new Fl_Box(X+posOffset,Y,temperatureWidth,defaultHeight,"TMP");
            temperature->box(FL_UP_BOX);
            posOffset+= temperatureWidth; // X 2 to make room for label on left
        
            set_temp = new Fl_Box(X+posOffset,Y,temperatureWidth,defaultHeight,"TMP");
            set_temp->box(FL_UP_BOX);
            posOffset+= 2 * temperatureWidth + 10; // X 2 to make room for label on left
                
             // Input
            hhours = new Fl_Input(X+posOffset,Y,hhoursW,defaultHeight,"HOLD");
            hhours->box(FL_UP_BOX);
            posOffset+= 3 * hhoursW;
        
            htemp = new Fl_Input(X+posOffset,Y,htempW,defaultHeight,"h @");
            htemp->box(FL_UP_BOX);
            posOffset+=htempW;
        
            // Stretchy box
            stretchXpos = X + posOffset;
            stretchWidth = W - posOffset;
            stretchBox = new Fl_Box(stretchXpos,Y,stretchWidth,defaultHeight, "");
            stretchBox->box(FL_UP_BOX);
            resizable(stretchBox);
        end();
    }// constructor
    
    
    //
    //      ScrollIrem      updateData()
    //
    //
    void updateData(){
        
        std::ostringstream status;
        
               //Name
               strcpy(id_str,stat->getName().c_str());
               id->label(id_str);
        std::cout << id->label() << " ";
               
               //Temperature
               strcpy(temp_str,stat->curr_temp.c_str());
               temperature->label(temp_str);
               if (stat->heating){
                   temperature->color(FL_RED);
               }else{
                   temperature->color(FL_BACKGROUND_COLOR);
               }
        
        std::cout << temperature->label() << " ";
           
               //Set Temperature
               strcpy(set_temp_str,stat->curr_set_temp.c_str());
               set_temp->label(set_temp_str);
        std::cout << set_temp->label() << std::endl;
        
        hhours->value("");
        htemp->value("");
        
        if ( stat->holdTimeHours<int>() + stat->holdTimeMins<int>() ){ // time remaining != 0
                        status << "Holding "
                          << stat->holdTemp()
                          << " for "
                          << stat->holdTimeHours<std::string>()
                          << ":"
                          << stat->holdTimeMins<std::string>();
        }else{
            status << "Timer control ";
        }
        strcpy(status_label, status.str().c_str());
        stretchBox->label(status_label);
        
        redraw();
       }
    
    
    
private:
    std::string value01;
    std::string value02;
    
}; // Class ScrollItem

//
// Class MyScroll
//
class MyScroll : public Fl_Scroll {
    
friend void sync_cb(Fl_Widget*, void *);
friend void hold_cb(Fl_Widget*, void *);
friend  int gui(Neohub*);
    
    int nchild;
    std::vector<ScrollItem*> scrollPtrs;
    Neohub* neohub;
    
public:
    //
    // MyScroll CONSTRUCTOR
    //
    MyScroll(int X, int Y, int W, int H, const char* L=0) : Fl_Scroll(X,Y,W,H,L) {
        nchild = 0;
    }
    
    //
    //  MyScroll    resize()
    //
    
    void resize(int X, int Y, int W, int H) {
        // Tell children to resize to our new width
        for ( int t=0; t<nchild; t++ ) {
            Fl_Widget *w = child(t);
            w->resize(w->x(), w->y(), W-20, w->h());    // W-20: leave room for scrollbar
        }
        // Tell scroll children changed in size
        init_sizes();
        Fl_Scroll::resize(X,Y,W,H);
    }// resize()
    
    //
    // MyScroll     AddItem()
    // Append new scrollitem to bottom
    //     Note: An Fl_Pack would be a good way to do this too
    //
    ScrollItem* AddItem(Stat* statptr) {
        int X = x() + 1,
            Y = y() - yposition() + (nchild*defaultHeight) + 1,
            W = w() - 20,                           // -20: compensate for vscroll bar
            H = defaultHeight;
        //Stat* thisStat = new Stat(*stat); // creates new stat instance
        ScrollItem* line = new ScrollItem(X,Y,W,H);
        
        line->stat = statptr;  // establish pointer to Neostat object
            
        line->updateData();     // populate irems with values from Stat
        
        add(line);
        line->updateData();     // populate irems with values from Stat
        redraw(); // now doing this in updateData()
        nchild++;
        return line;
        
    }// AddItem()
    
   
    
};// Class MyScroll

//*************************************
//      CALLBACKS
//
// sync_cb()    to re-sync gui with neohub
//
void sync_cb(Fl_Widget*, void *data) {
    MyScroll *scroll = (MyScroll*)data;
    //
    // TODO::::  NEED TO RESYNC with NEOHUB HERE
    //
    //  problem is that Neohub::init() is all abour creating new stats
    //      rather than updating existing ones so we need a
    //      Neohub::update() function which can be called by init() also
    //
    scroll->neohub->init(); // re-load state from Neohub
    ScrollItem *w;
    for ( int t=0; t < scroll->nchild; t++ ) {
        w = (ScrollItem *)scroll->child(t);
        std::cout << "syncing " << w->id->label() << " ";
        w->updateData();
        
    }
    scroll->redraw();
    
}//sync_cb


//
// hold_cb()    to re-sync gui with neohub
//

void hold_cb(Fl_Widget*, void *data) {
    
    ScrollItem*  thisItem;
    int hhours;
    int hmins = 0;
    int htemp;
    
    MyScroll* scrowll = (MyScroll*)data;
    
    Stat thisStat;
    for(auto it = scrowll->scrollPtrs.begin(); it != scrowll->scrollPtrs.end(); ++it){
        thisItem = *it;
        thisStat = *(thisItem->stat);
        htemp = atoi(thisItem->htemp->value());
        hhours = atoi(thisItem->hhours->value());
        if((htemp > 0) || (hhours > 0)){
            
            std::cout << thisItem->id_str << std::endl;
            if (htemp == 0) htemp = 24;
            if (hhours == 0 ) hhours = 2;
            std::cout << "holding" << thisStat.getName() << std::endl;
            thisStat.hold(htemp, hhours, hmins);
        }
       
    }
}// hold_cb()

extern bool debug;

//***************************************************
//      The GUI function
//
//

int    gui(Neohub* myHub){
    
    std::vector<Stat>* stats = myHub->getStats(); // pointer to vector of Thermostats
    
    Fl_Double_Window *win = new Fl_Double_Window(600,700);
        MyScroll *scroll = new MyScroll(10,10,win->w()-20,win->h()-160);
            scroll->neohub = myHub;
            scroll->box(FL_BORDER_BOX);
    
            // create scroll items
            ScrollItem* i;
            Stat* thisStat;
            for(auto it = stats->begin(); it != stats->end(); ++it){
            
                    thisStat = &(*it); // thisStat is pointer to element in stats vector
                                
                    i = scroll->AddItem(thisStat);
                    scroll->scrollPtrs.push_back(i); // push/copy pointer this scroll item onto vector
               
            }
    
        scroll->end();
    
        // HOLD and SYNC buttons
        //
        Fl_Button *hold_butt = new Fl_Button(win->w()-250, win->h()-140, 100, 25, "Hold");
            hold_butt->callback(hold_cb, (void*)scroll);
    
        Fl_Button *sync_butt = new Fl_Button(win->w()-150, win->h()-140, 100, 25, "Sync");
            sync_butt->callback(sync_cb, (void*)scroll);
    
        
        Fl_Multi_Browser *console = new Fl_Multi_Browser(10, win->h()-100, win->w()-20, 95, "");
    console->box(FL_UP_BOX);
    for (int q = 1; q < 50; ++q){
        console->add("some text");
        //console->add("\n");
        
    }
        //console->bottomline();
    
        win->resizable(scroll);
        win->show();
    return(Fl::run());
    
}// gui()
