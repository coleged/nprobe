
/******************************************************************
 
Rewrite of neoprobe using classes and looking more like C++ than C
 
 Version 2.2.4
 Feb 2020	  FLTK based GUI

USAGE: nprobe [options] 
    -G           - Runs GUI window (beta)
    -t           - Prints temperatures and state of all stats
    -c           - Prints state of all timers (clocks)
    -l           - Prints temperatures of all stats in one log line
    -C           - Prints comfort levels of stats. All stats unless -z used
    -e           - Prints timed events of all timers or use -z for just one
    -D           - Debug. Prints debug detail
    -V           - Prints version number and exits
    -O <timer>   - Timer overide on
    -o <timer>   - Timer overide off
    -s <server>  - Specify hostname of Neohub device (default neohub)
    -p <port>    - Specify port of Neohub device (default 4242)
    -H <stat>    - Hold thermostat
    -z <zone>    - Specify the zone for -C | -e option
    -T <temp>    - Temperature for hold (defaut = 24C)
    -h <hours>   - Hours of hold time (default = 1 hour)
    -m <mins>    - minutes of hold time (default = 0 mins
    -f <file>    - apply JSON formatted neohub commands contained in <file> 
 
 
 ***********************************************************************/
