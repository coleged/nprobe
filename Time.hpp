//
//  Time.hpp
//  nprobe
//
//  Created by Ed Cole on 23/11/2019.
//  Copyright © 2019 colege. All rights reserved.
//
// A class to deal with time represented notionally in the form "hh:mm"

#ifndef Time_hpp
#define Time_hpp

#include <stdio.h>
#include <sstream>
#include <string>


//*****************************
//
//  Time       CLASS
//
//*****************************

class Time{
    
public:
    
    Time();
    Time(int h, int m);
    ~Time();
    
    //copy constructor
    Time(const Time &t);
    
    bool    setTime(int h, int m);
    bool    setTime(std::string t);
    
    int     getHours();
    int     getMins();
    
    std::string asStr();        // returns time as string "HH:MM"
    std::string remaining();    // returns time remaining from now until time
    std::string getNow();       // returns current time "HH:MM"
    
    
private:
    
    friend std::ostream& operator<<(std::ostream& out, const Time& t);
    
    int             hours, mins;
    int             now_hours, now_mins;
    
    std::string     asStr(int h, int m);
    bool            check();
    
};//Time



#endif /* Time_hpp */
