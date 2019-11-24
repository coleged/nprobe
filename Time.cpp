//
//  Time.cpp
//  nprobe
//
//  Created by Ed Cole on 23/11/2019.
//  Copyright © 2019 colege. All rights reserved.
//

#include "Time.hpp"


//*****************************
//
//  Time       CLASS
//
//*****************************
Time::Time(){}
Time::Time(int h, int m){
    setTime(h,m);
    }
Time::~Time(){};


//*****************************************
void Time::setTime(int h, int m){
    hours = h;
    mins = m;
}//setHoursMins


//*****************************************
std::string Time::asStr(){
    std::string ret;
    
    std::stringstream h_ss,m_ss;
    h_ss.width(2);  m_ss.width(2);
    h_ss.fill('0'); m_ss.fill('0');
    h_ss << hours;      m_ss << mins;
    
    ret = h_ss.str() + ":" + m_ss.str();
    
    return ret;
}//toStr



//*****************************************
std::string Time::asStr(int h, int m){
    std::string ret;
    
    std::stringstream h_ss,m_ss;
    h_ss.width(2);  m_ss.width(2);
    h_ss.fill('0'); m_ss.fill('0');
    h_ss << h;      m_ss << m;
    
    ret = h_ss.str() + ":" + m_ss.str();
    
    return ret;
}//toStr


//*****************************************
std::string Time::getNow(){
    
    static std::string now;
    static int now_h, now_m;
    
    if (now.empty()){ // first time this has been called
        int h,m;
        std::string h_s,m_s;
        
        time_t now_t = time(0);
        tm *ltm = localtime(&now_t);
        h = ltm->tm_hour + 1;
        m = ltm->tm_min + 1;
        if ( ltm->tm_isdst == 0 ){ // not daylight saving time
            --h;
        }
        now_h = h; now_m = m;
        now = asStr(h,m);
    }
    now_hours = now_h;
    now_mins = now_m;
    return now;
}//getNow

int Time::getHours(){
    return hours;
}

int Time::getMins(){
    return mins;
}

//*****************************************
std::string Time::remaining(){
    std::string ret;
    // kludge. doesn't account for -ve time remaining
    
    this->getNow();
    int hr = (hours - now_hours);
    int mr;
    if ((mr = (mins - now_mins)) < 0){
        --hr;
        mr += 60;
    };
    
    ret = asStr(hr,mr);
    return ret;
}// remaining


// operator overload for output - non member function
std::ostream& operator<< (std::ostream& out, const Time& t){
    Time c = t;
    out << c.asStr();
    return out;
    }

