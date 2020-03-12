//
//  usage.cpp
//  nprobe
//
//  Created by Ed Cole on 15/02/2020.
//  Copyright © 2020 colege. All rights reserved.
//


#include <iostream>

// ************   errorUsage()
//
void errorUsage(){
    
std::cout << " USAGE:\n"
"        -G           - Runs GUI window (beta)\n"
"        -t           - Prints temperatures and state of all stats\n"
"        -c           - Prints state of all timers (clocks)\n"
"        -l           - Prints temperatures of all stats in one log line\n"
"        -C           - Prints comfort levels of stats. All stats unless -z used\n"
"        -e           - Prints timed events of all timers or use -z for just one\n"
"        -D           - Debug. Prints debug detail\n"
"        -V           - Prints version number and exits\n"
"        -O <timer>   - Timer overide on\n"
"        -o <timer>   - Timer overide off\n"
"        -s <server>  - Specify hostname of Neohub device (default neohub)\n"
"        -p <port>    - Specify port of Neohub device (default 4242)\n"
"        -H <stat>    - Hold thermostat\n"
"        -z <zone>    - Specify the zone for -C | -e option\n"
"        -T <temp>    - Temperature for hold (defaut = 24C)\n"
"        -h <hours>   - Hours of hold time (default = 1 hour)\n"
"        -m <mins>    - minutes of hold time (default = 0 mins\n"
"        -f <file>    - apply JSON formatted neohub commands contained in <file>\n"
"";

   
}
