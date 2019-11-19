//
//  neohub.hpp
//  nprobe
//
//  Created by Ed Cole on 27/09/2018.
//  Copyright © 2018 Ed Cole. All rights reserved.
//

/*
 TODO:
    There is a mix of std::string and *char or char[] types for string values. This is a
    hang over from some code snipets taken from some C programms. Look at more consistant
    use of std::string types. Examples of *char that could be rationalised include
                                    server name
                                    
 
 */

/**
Classes:
    Neohub          - models the Heatmizer Neohub.
    NeoStatBase -  Base class inherited by Stat and Timer classes
    Stat                - models a Heatmiser Neostat device of which there will be a number networked to
                the Neohub
    Timer             - models a Heatmiser Neostat device configured as a Timer of which there will be
                a number networked to the Neohub
    Comfort         - models a thermostat comfort level i.e. temperature @ time
    Event            - models a timer switching cycle on/off times

 **/

#ifndef neohub_hpp
#define neohub_hpp

// Config constants

#ifndef _DEBUG
#define _DEBUG  false
#endif

#define MYNAME  "nprobe"
#define VERSION "2.2.2 November 2019. Ed Cole <colege@gmail.com>"
// V2.2 uses nlohmann/jsoncpp

#define NEOHUB_NAME "neohub.rainbow-futures.com"
#define NEOHUB_PORT 4242
#define READ_BUFFER_SZ  64*1024 // Used to hold the JSON response
                                // from the Neohub prior to parsing

#define SOC_BUFFER_SZ 4096      // Socket buffer to communication with Neohub
#define NEO_SOC_TIMEOUT 2       // Socket timeout for Neohub comms - in seconds

#define DEF_HOLD_TEMP 24
#define DEF_HOLD_HOURS 1
#define DEF_HOLD_MINS 0

// includes

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <nlohmann/json.hpp>
#include <sstream>

// prototypes in functions.cpp

char *stripString(char *);
bool readJson(char *, char *);
char *timestamp();
bool wneohub(char *);

// classes

//*****************************
//
//  Switch       CLASS
//
//*****************************

class Switch{
    
    
public:
    Switch();
    ~Switch();
    
    std::string getTimeOn();
    std::string getTimeOff();
    
    int getHoursOn();
    int getMinsOn();
    int getHoursOff();
    int getMinsOff();
    
    std::string timeOn = "HH:MM";
    std::string timeOff = "HH:MM";
    
}; // Class Switch


//*****************************
//
//  Comfort       CLASS
//
//*****************************

class Comfort: public Switch{
    
    friend class Stat;
    
public:
    Comfort();
    Comfort(std::string time,int temp);
    ~Comfort();
    
    void setComfort(std::string time,int temp);
    void print(); // debug method
    
    int getTemp();
    
private:

    int temp;
    
}; // Class Comfort

//*****************************
//
//  Event       CLASS
//
//*****************************
 
 class Event: public Switch{
 
 friend class Timer;
 
 public:
 Event();
 Event(std::string tmOn,std::string tmOff);
 ~Event();
 
 void setTimerEvent(std::string tmOn,std::string tmOff);
 void print(); // debug method
 
 
 
 
 
 }; // Class Event

//*****************************
//
//  NeoStatBase  CLASS
//
//      Base Class for Stat and Timer
//
//*****************************

class NeoStatBase{

public:
    NeoStatBase();
    ~NeoStatBase();
    
    std::string getName();          // returns the name of the device
    
    
    
    
    bool    away = false;
    bool    cooling = false;
    bool    cooling_enabled = false;
    int     cooling_temperature_in_whole_degrees = 0;
    bool    cool_inp = false;
    struct  {
        int     hours;
        int     mins;
    }count_down_time = {0,0};
    bool    cradle_paired_to_remote_sensor = false;
    bool    cradle_paired_to_stat = false;
    int     current_floor_temperature = 0;
    float   current_set_temperature = 9.9;
    float   current_temperature = 9.9;
    bool    demand = false;
    int     device_type = 0;
    bool    enable_boiler = false;
    bool    enable_cooling = false;
    bool    enable_pump = false;
    bool    enable_valve = false;
    bool    enable_zone = false;
    bool    failsafe_state = false;
    bool    fail_safe_enabled = false;
    bool    floor_limit = false;
    bool    full_partial_lock_available = false;    // full/partial_lock_available
    bool    heat_cool_mode = false;            // heat/cool_mode
    bool    heating = false;
    int     hold_temperature = 9;
    struct    {
        int hours;
        int mins;
    } hold_time = {0,0};
    bool    holiday = false;
    int     holiday_days = 0;
    int     humidity = 0;
    bool    lock = false;
    char    *lock_pin_number = nullptr;
    bool    low_battery = false;
    float   max_temperature = 9.9;
    float   min_temperature = 9.9;
    int     modulation_level = 0;
    char    *next_on_time = nullptr;
    bool    offline = false;
    bool    ouput_delay = false;
    int     output_delay = 0;
    bool    preheat = false;
    struct  {
        int hours;
        int mins;
    }preheat_time = {0,0};
    char    *program_mode = nullptr;
    bool    pump_delay = false;
    bool    radiators_or_underfloor = false;
    char    *sensor_selection = nullptr;
    int     set_countdown_time = 0;
    bool    standby = false;
    struct    {
        bool    four_heat_levels;            // 4_heat_levels
        bool    manual_off;
        bool    thermostat;
        bool    timeclock;
    }stat_mode = {false,false,false,false};
    bool    temperature_format = false;
    bool    temp_hold = false;
    bool    timeclock_mode = false;
    bool    timer = false;
    bool    time_clock_overide_bit = false;
    int     ultra_version = 0;
    int     version_number = 0;
    int     write_count = 0;
    bool    zone_1paired_to_multilink = false;
    bool    zone_1_or_2 = false;
    bool    zone_2_paired_to_multilink = false;
    std::string  device = "unassigned";
    
    template<typename T>
        T holdTimeHours() {return hold_time.hours;
        };
    
    template<typename T>
    T holdTimeMins() {return hold_time.mins;
    };
    
    
}; // Class NeoStatBase

template<> int NeoStatBase::holdTimeHours();
template<> std::string NeoStatBase::holdTimeHours();

template<> int NeoStatBase::holdTimeMins();
template<> std::string NeoStatBase::holdTimeMins();

//*****************************
//
//  Stat       CLASS
//
//*****************************

class Stat: public NeoStatBase{     // A heatmiser Neostat
    
friend class Neohub;
    
public:
    Stat();
    ~Stat();
    
    bool isOn();
    
    float getTemp();
    
    float getSetTemp();
    
    
    bool hold(int temp, int hours, int min);  // puts Themostat into hold
    
    void getComfortLevels(); // gets comfort from neohub -> memory private perhaps?
    void printComfortLevels();
    
    //int holdTimeHours();
    //int holdTimeMins();
    
    float holdTemp();
    
private:
    
    //  comfort levels
    
    Comfort comfortLevels[2][4];     // 2 periods - moday, sunday (i.e. weekday, weekend)
                                     // 4 events wake, leave, return, sleep
    
}; // Class Stat

//*****************************
//
//  Timer       CLASS
//
//*****************************

class Timer: public NeoStatBase{
    
friend class Neohub;
    
public:
    Timer();
    ~Timer();
    
    bool isOn();
    
    bool holdOn(int hold_mins);
    bool holdOff();
    void getTimerEvents();
    void printTimerEvents();
    
    //int holdTimeHours();
    //int holdTimeMins();
    
private: 

    Event timerEvents[2][4]; 
 
    
}; // Class Timer

//*****************************
//
//  Neohub       CLASS
//
//*****************************

class Neohub{
public:
    Neohub();
    ~Neohub();
    
    void init();                // probe the hub for connected devices
    void setServer(char *ss);   
    void setPort(int sp);
    
    char *getHub(char *cmd);    // takes JSON command and returns result
                                // in buffer containing JSON response
   
    void newStat(Stat); // push a Stat object onto the neohub stat vector
    void printStats();  // print out the Stat vector
    
    void printLog();    // prints single line of stat status data
    
    void newTimer(Timer);   // push a Timer object onto the neohub timer vector
    void printTimers();     // print out the Timer vector
    
    std::vector<Stat> stats;    // vector of [thermo]Stat objects
    std::vector<Timer> timers;  // vector of timer objects
    
private:
    
    
    char *server_name = nullptr;
    int port = 0;
    
    char buffer[READ_BUFFER_SZ];
    

}; // Class Neohub


#endif /* neohub_hpp */
