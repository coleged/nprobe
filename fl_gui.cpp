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

const int idboxWidth = 120;
const int temperatureWidth = 40;
const int inputW = 50;
const int defaultHeight = 25;

std::string test = "test";

// Combo widget to appear in the scroll, two boxes: one fixed, the other stretches
class ScrollItem : public Fl_Group {
    
friend int    gui(Neohub* myHub);
friend class MyScroll;
    
    Fl_Box *id;
    char id_str[32];
    Fl_Box *temperature;
    char temp_str[32];
    
    Fl_Input *input02;
    Fl_Box *stretchBox;
public:
    // constructor
    ScrollItem(int X, int Y, int W, int H, const char* L=0) : Fl_Group(X,Y,W,H) {
        int inputs = 2;
        int stretchXpos,stretchWidth;
        int posOffset = 0;
        begin();
            // Fixed width box
            id = new Fl_Box(X,Y,idboxWidth,defaultHeight,L);
            id->box(FL_UP_BOX);
            posOffset+=idboxWidth;
            temperature = new Fl_Box(X+posOffset,Y,temperatureWidth,defaultHeight,"TMP");
            temperature->box(FL_UP_BOX);
            posOffset+=temperatureWidth;
                
             // Input
            //input02 = new Fl_Input(X+posOffset + inputW,Y,inputW,defaultHeight,"");
            //posOffset+=inputW;
            // Stretchy box
            stretchXpos = X + posOffset;
            stretchWidth = W - posOffset;
            stretchBox = new Fl_Box(stretchXpos,Y,stretchWidth,defaultHeight, "Stretch");
            stretchBox->box(FL_UP_BOX);
            resizable(stretchBox);
        end();
    }
    
private:
    std::string value01;
    std::string value02;
    
};

// Custom scroll that tells children to follow scroll's width when resized
class MyScroll : public Fl_Scroll {
    int nchild;
public:
    MyScroll(int X, int Y, int W, int H, const char* L=0) : Fl_Scroll(X,Y,W,H,L) {
        nchild = 0;
    }
    
    void resize(int X, int Y, int W, int H) {
        // Tell children to resize to our new width
        for ( int t=0; t<nchild; t++ ) {
            Fl_Widget *w = child(t);
            w->resize(w->x(), w->y(), W-20, w->h());    // W-20: leave room for scrollbar
        }
        // Tell scroll children changed in size
        init_sizes();
        Fl_Scroll::resize(X,Y,W,H);
    }
    
    // Append new scrollitem to bottom
    //     Note: An Fl_Pack would be a good way to do this too
    //
    ScrollItem* AddItem(Stat* stat) {
        int X = x() + 1,
            Y = y() - yposition() + (nchild*defaultHeight) + 1,
            W = w() - 20,                           // -20: compensate for vscroll bar
            H = defaultHeight;
        ScrollItem* line = new ScrollItem(X,Y,W,H);
        //Name
        std::cout << stat->getName() << std::endl;
        strcpy(line->id_str,stat->getName().c_str());
        line->id->label(line->id_str);
        //Temperature
        //std::cout << stat->ftoa(getTemp()) << std::endl;
        //strcpy(line->id_str,stat->getName().c_str());
        //line->id->label(line->id_str);
        add(line);
        redraw();
        nchild++;
        return line;
    }
};

// Callback to add new item to scroll
void add_cb(Fl_Widget*, void *data) {
    MyScroll *scroll = (MyScroll*)data;
    scroll->AddItem((Stat*)nullptr);
}

extern bool debug;

int    gui(Neohub* myHub){
    std::vector<Stat>* stats = myHub->getStats();
    /*
    for(auto it = stats->begin(); it != stats->end(); ++it){
        //std::cout << it->device;
        std::cout << it->getName();
        std::cout << " : ";
        std::cout << it->getTemp();
        std::cout << std::endl;
    }
     */
    
    Fl_Double_Window *win = new Fl_Double_Window(300,600);
        MyScroll *scroll = new MyScroll(10,10,win->w()-20,win->h()-60);
        scroll->box(FL_BORDER_BOX);
        scroll->end();
        Fl_Button *add_butt = new Fl_Button(win->w()-150, win->h()-40, 100, 25, "Add");
        add_butt->callback(add_cb, (void*)scroll);
        
    // Create a few widgets to start with
    ScrollItem* i;
    Stat thisStat;
    for(auto it = stats->begin(); it != stats->end(); ++it){
        
        //std::cout << it->getName() << std::endl;
        thisStat = *it;
            i = scroll->AddItem(&thisStat);
       
    }
    
        win->resizable(scroll);
        win->show();
    return(Fl::run());
}
