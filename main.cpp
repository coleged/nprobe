//
//  main.cpp
//  nprobe
//
//  Created by Ed Cole on 27/09/2018.
//  Copyright © 2018, 2019 Ed Cole. All rights reserved.
//

// See: readme.txt

#include "neohub.hpp"



bool debug = _DEBUG;

// ************   errorUsage()
//
void errorUsage(){
    
    char myname[] = MYNAME;

    printf("USAGE: %s [options] \n",myname);
    printf("    -G           - Runs GUI window (beta)\n");
    printf("    -t           - Prints temperatures and state of all stats\n");
    printf("    -c           - Prints state of all timers (clocks)\n");
    printf("    -l           - Prints temperatures of all stats in one log line\n");
    printf("    -C           - Prints comfort levels of stats. All stats unless -z used\n");
    printf("    -e           - Prints timed events of all timers or use -z for just one\n");
    printf("    -D           - Debug. Prints debug detail\n");
    printf("    -V           - Prints version number and exits\n");
    printf("    -O <timer>   - Timer overide on\n");
    printf("    -o <timer>   - Timer overide off\n");
    printf("    -s <server>  - Specify hostname of Neohub device (default neohub)\n");
    printf("    -p <port>    - Specify port of Neohub device (default 4242)\n");
    printf("    -H <stat>    - Hold thermostat\n");
    printf("    -z <zone>    - Specify the zone for -C | -e option\n");
    printf("    -T <temp>    - Temperature for hold (defaut = 24C)\n");
    printf("    -h <hours>   - Hours of hold time (default = 1 hour)\n");
    printf("    -m <mins>    - minutes of hold time (default = 0 mins\n");
    printf("    -f <file>    - apply JSON formatted neohub commands contained in <file>\n");
    
    // WIP: playing with Time class
    Time a, b, c(23, 45);
    std::cout << a.getNow();
    std::cout << ' ';
    std::cout << b.getNow();
    std::cout << ' ';
    std::cout << c.remaining();
    std::cout << std::endl;
    
    
    std::cout << a.asStr();
    std::cout << ' ';
    std::cout << b;
    std::cout << ' ';
    std::cout << c;
    std::cout << ' ';
    std::cout << std::endl;

    
}

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
        fprintf(stderr,"HOLD FAILED\n");
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
        fprintf(stderr,"HOLD FAILED\n");
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
        fprintf(stderr,"UNHOLD FAILED\n");
        exit(EXIT_FAILURE);
    }
}

/*
 // moved to fl_gui.cpp
void    gui(Neohub* myHub){
    std::vector<Stat>* stats = myHub->getStats();
    for(auto it = stats->begin(); it != stats->end(); ++it){
        //std::cout << it->device;
        std::cout << it->getName();
        std::cout << " : ";
        std::cout << it->getTemp();
        std::cout << std::endl;
    }
}
 */

// ************   main()
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
    
    while ((opt = getopt(argc, argv, "GCDVlO:o:tcep:s:H:T:h:m:f:z:")) != -1){
        switch (opt){
                
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
