//
//  main.cpp
//  nprobe
//
//  Created by Ed Cole on 27/09/2018.
//  Copyright © 2018, 2019 Ed Cole. All rights reserved.
//

// See: readme.txt

#include "neohub.hpp"
void errorUsage();

bool debug = _DEBUG;

// ************   holdStat()
//
void holdStat(Neohub *myHub, std::string hold_stat, int hold_temp, int hold_hours, int hold_mins){
    
    bool held = false;
    
    for(auto it = myHub->stats.begin(); it != myHub->stats.end(); ++it){
        if( it->getName() == hold_stat){
            if(it->hold(hold_temp,hold_hours,hold_mins)){
                if (debug) printf("Hold Succeded\n");
                held = true;
            };
        }
        
    }//for
    if(!held){
        //fprintf(stderr,"HOLD FAILED\n");
        std::cout << "HOLD FAILED" << std::endl;

        exit(EXIT_FAILURE);
    }

}

// ************   holdTimer()
//
void holdTimer( Neohub *myHub, std::string hold_timer, int hold_hours, int hold_mins){
    
    bool held = false;
    
    for(auto it = myHub->timers.begin(); it != myHub->timers.end(); ++it){
        if( it->getName() == hold_timer){
            hold_mins = hold_mins + 60*hold_hours;
            if(it->holdOn(hold_mins)){
                if (debug) printf("Overide Succeded\n");
                held = true;
            };
        }
        
    }
    if(!held){
        //fprintf(stderr,"HOLD FAILED\n");
        std::cout << "HOLD FAILED" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// ************   unholdTimer()
//
void unholdTimer( Neohub *myHub, std::string unhold_timer){

    bool unheld = false;
    
    for(auto it = myHub->timers.begin(); it != myHub->timers.end(); ++it){
        if( it->getName() == unhold_timer){
            if(it->holdOff()){
                if (debug) printf("unhold Succeded\n");
                    unheld = true;
                    };
        }
        
    }
    if(!unheld){
        //fprintf(stderr,"UNHOLD FAILED\n");
        std::cout << "UNHOLD FAILED" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// timeTest()
//
// quick hack to test Time class.
// to be deleted

void timeTest(){
    Time t;
    std::string input;
    
    while(true){
        std::cout << "Enter time string: ";
        std::cin >> input;
        if (! t.setTime(input)){
            std::cout << "Bad time string" << std::endl;
        }else{
            std::cout << "OK: " << t.asStr() << std::endl;
        }
        
    }
}

//*****************************************
//
//          MAIN MAIN MAIN
//
//*****************************************
//
int main(int argc, char *argv[]) {
    
    Neohub myHub;
    
    int opt;
    
    bool temp_flag = false;
    bool timer_flag = false;
    bool log_flag = false;
    bool comfort_flag = false;
    bool event_flag = false;
    bool gui_flag = false;
    bool todo = false;		// set true if I've been asked to do anything
    
    std::string hold_stat = "";
    std::string zone = "";
    std::string hold_timer = "";
    std::string unhold_timer = "";
    
    int hold_temp = DEF_HOLD_TEMP;
    int hold_hours = DEF_HOLD_HOURS;
    int hold_mins = DEF_HOLD_MINS;

    char *set_server;
    char *cmd_file = nullptr;
    
    // finder launch guard
    // if this is run under MacOS from a app bundle from the finder it is invoked
    // with a command line  nprobe -psn_XXXXXXXX
    //
    if (strncmp("-psn_", argv[1], 5) == 0){
        // we have been involked as a MacOS X app using open
        strcpy(argv[1], "-G");
    }
    
    while ((opt = getopt(argc, argv, "qGCDVlO:o:tcep:s:H:T:h:m:f:z:")) != -1){
        switch (opt){
                
            case 'q':   // timeTest TDB
                timeTest();
                exit(0);    // timeTest never returns
                
            case 'D': //
                debug = true;
                break;
                
            case 's': // Alternate server (neohub)
                set_server=(char *)malloc(strlen(optarg)+1);
                strcpy(set_server,optarg);
                myHub.setServer(set_server);
                break;
                
            case 'p': // Alternate neohub port number
                myHub.setPort(atoi(optarg));
                break;
                
            case 'f': // command file
                cmd_file=(char *)malloc(strlen(optarg)+1);
                strcpy(cmd_file,optarg);
                todo = true;
                break;
                
            case 'z': // Hold
                zone = optarg;
                break;
                
            case 'O': // tImer overide on
                hold_timer = optarg;
                todo = true;
                break;
                
            case 'o': // timer override off
                unhold_timer = optarg;
                todo = true;
                break;
                
            case 'H': // Hold
                hold_stat = optarg;
                todo = true;
                break;
                
            case 'T': // Hold Temp
                hold_temp = atoi(optarg);
                if((hold_temp < 15) || (hold_temp > 30)){
                    fprintf(stderr,"Invalid hold temp\n");
                    exit(EXIT_FAILURE);
                }
                break;
                
            case 'h': // Hold Hours
                hold_hours = atoi(optarg);
                if((hold_hours < 0) || (hold_hours > 24)){
                    fprintf(stderr,"Invalid hold hours\n");
                    exit(EXIT_FAILURE);
                }
                break;
                
            case 'm': // Hold Minutes
                hold_mins = atoi(optarg);
                if((hold_mins < 0) || (hold_mins > 60)){
                    fprintf(stderr,"Invalid hold mins\n");
                    exit(EXIT_FAILURE);
                }
                break;
                
            case 'C': //
                comfort_flag = true;
                todo = true;
                break;
                
            case 'e': //
                event_flag = true;
                todo = true;
                break;
                
            case 'l': // log output
                log_flag = true;
                break;
                
            case 'c': // Timer status output
                timer_flag = true;
                todo = true;
                break;
                
            case 't': // Temperatures
                temp_flag = true;
                todo = true;
                break;
                
            case 'G': // GUI
                gui_flag = true;
                todo = true;
                break;
                
            case 'V':
                printf("%s. Verion %s\n",MYNAME,VERSION);
                exit(EXIT_SUCCESS);
                break;
                
            default:
                errorUsage();
                exit(EXIT_FAILURE);
                
        }//switch
    }//while

    if (!todo){	// command line arguments dont do anything useful
                // so just print usage and exit
        errorUsage();
        exit(EXIT_SUCCESS); // it's not a failure as such
    }
    
    myHub.init();
    
    if (gui_flag)   gui(&myHub);
    if (temp_flag) myHub.printStats();
    if (log_flag) myHub.printLog();
    if (timer_flag) myHub.printTimers();
    if (hold_stat != "") holdStat( &myHub, hold_stat, hold_temp, hold_hours, hold_mins);
    if (hold_timer != "") holdTimer( &myHub, hold_timer, hold_hours, hold_mins);
    if (unhold_timer != "") unholdTimer( &myHub, unhold_timer);
    if (comfort_flag){ // print comfort levels for all or just one (-z) thermostat
        for(auto it = myHub.stats.begin(); it != myHub.stats.end(); ++it){
            if ( (it->getName() == zone) || (zone == "")){
                it->printComfortLevels();
            }
        }
    }
    if (event_flag){ // print timer events for all or just one (-z) timer
        for(auto it = myHub.timers.begin(); it != myHub.timers.end(); ++it){
            if ( (it->getName() == zone) || (zone == "")){
                it->printTimerEvents();
            }
        }
    }
    if (cmd_file != nullptr){
        wneohub(cmd_file);
    }
    
    exit(EXIT_SUCCESS);
}
