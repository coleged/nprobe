//
//  neohub.cpp
//  nprobe
//
//  Created by Ed Cole on 27/09/2018.
//  Copyright © 2018 Ed Cole. All rights reserved.
//

#include "neohub.hpp"

extern bool debug;

using json = nlohmann::json;


//*****************************
//
//  Neohub       CLASS
//
//*****************************

Neohub::Neohub(){} // Neohub()
Neohub::~Neohub(){}

//*****************************
void Neohub::setServer(char *ss){
    strcpy(server_name, ss);
}//setServer()

//*****************************
void Neohub::setPort(int sp){
    port = sp;
}//setPort()

//*****************************
void Neohub::init(){
    
    int stat_count = 0;
    int timer_count = 0;
    
    char *info_buffer;
    char cmd[]="{\"INFO\":0}";
    
    if(debug) printf("init():initialising hub\n");
    
    info_buffer = getHub(cmd);       // get info from hub
    
    if(info_buffer == nullptr){     // getHub failed
        fprintf(stderr,"ERROR: Neohub::init() Neohub::getHub() failed\n");
        exit(EXIT_FAILURE);
    }
    
    json root = json::parse(info_buffer);
    json element,mode,hold_time;
    
    Stat s;
    Timer t;
    
    std::string hold_time_s = "00:00";
    
    json j_devices = root["devices"];
    
    std::vector<json> v_devices = j_devices;
        
    for(std::vector<json>::iterator it = v_devices.begin();
                                    it != v_devices.end();
                                            it++){
        
        element = *it;
        mode = element["STAT_MODE"];
        
        //    THERMOSTATS
        if(mode.find("THERMOSTAT") != mode.end()){ // if key THERMOSTAT exists
            if((s.stat_mode.thermostat=mode["THERMOSTAT"].get<bool>())==true){
                s.device=element["device"];
                
                s.curr_temp = element["CURRENT_TEMPERATURE"].get<std::string>();
                s.current_temperature=stof(s.curr_temp,nullptr);
                
                s.curr_set_temp = element["CURRENT_SET_TEMPERATURE"].get<std::string>();
                s.current_set_temperature=stof(s.curr_set_temp,nullptr);
                
                s.hold_temperature=element["HOLD_TEMPERATURE"].get<int>();
                
                s.heating = element["HEATING"].get<bool>();
                hold_time_s = element["HOLD_TIME"].get<std::string>(); // of the form HH:MM
                s.hold_time.hours = atoi(hold_time_s.substr(0,hold_time_s.find(":")).c_str());
                s.hold_time.mins = atoi(hold_time_s.substr(hold_time_s.find(":")+1,hold_time_s.npos).c_str());
                s.getComfortLevels();
                ++stat_count;
                newStat(s, stat_count);                // and push the stat onto vector
            }//if
        }
        
        //    TIMERS
        //
        if(mode.find("TIMECLOCK") != mode.end()){ // if key TIMECLOCK exists
            if((t.stat_mode.timeclock=mode["TIMECLOCK"].get<bool>())==true){ 
                t.device=element["device"];
                t.timer=element["TIMER"].get<bool>(); //   if TIMER = true, timer is in on state
                hold_time_s = element["HOLD_TIME"].get<std::string>(); // of the form HH:MM
                t.hold_time.hours = atoi(hold_time_s.substr(0,hold_time_s.find(":")).c_str());
                t.hold_time.mins = atoi(hold_time_s.substr(hold_time_s.find(":")+1,hold_time_s.npos).c_str());
               
                // nuke repeater nodes here with continue (for loop) if substr of name matches.
                if(t.device.substr(0,8) == "repeater"){
                    continue; // skip reapeater nodes
                }
                t.getTimerEvents();
                ++timer_count;
                newTimer(t, timer_count);              // push the timer onto vector
            }//if
        }
    }// for
    neostats = stat_count + timer_count;
}//init()

//*****************************
char* Neohub::getHub(char *cmd){ // takes Neohub command and returns result
    
    //static char buffer[READ_BUFFER_SZ];   // Moved to private class variable.
                                            // Non static as it has object scope
    char d_server_name[] = NEOHUB_NAME;
    int d_port = NEOHUB_PORT;
    
    int  sockfd;
    struct sockaddr_in serv_addr = {};
    struct hostent *server;
    char *b_point;
    int bytes_in;
    int total_bytes = 0;
    int b,i;
    
    struct timeval tv;    // used for timeout in recv on socket
    tv.tv_sec = NEO_SOC_TIMEOUT;
    tv.tv_usec = 0;
    
    if (server_name[0] == '\0') strcpy(server_name,d_server_name);
    if (port == 0 ) port = d_port;
    
    if(debug) printf("getHub(): connecting to %s:%i\n",server_name,port);
    
    if( (server = gethostbyname(server_name)) == nullptr){
        fprintf(stderr,"ERROR: host lookup failed on %s",server_name);
        exit(EXIT_FAILURE);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr,"ERROR creating socket");
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family = AF_INET;
    bcopy(  (char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
        
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        fprintf(stderr,"ERROR connecting");
        exit(EXIT_FAILURE);
    }
    // Set recv timeout as neohub doesn't close socket. It
    // just stops taking
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    if(debug) printf("getHub(): writing to neohub: %s\n",cmd);
    write(sockfd,stripString(cmd),strlen(stripString(cmd)));
    write(sockfd,"\0\n",2);         // JSON needs \0 terminated string
    bzero(buffer,READ_BUFFER_SZ);
    
    b_point=buffer;
    
    if(debug) printf("getHub(): Reading neohub. ");
    while((bytes_in=(int)recv(sockfd,b_point,SOC_BUFFER_SZ,0))>0){ // read response
        if(debug) printf(" >> %i ",bytes_in);
        total_bytes += bytes_in;
        b_point=b_point+bytes_in; // advance buffer write offset
     
       
      if (total_bytes >= READ_BUFFER_SZ){ // buffer overflow
            fprintf(stderr,"getHub buffer overflow \n");
            exit(EXIT_FAILURE);
        }
      
        
        // break the while loop when complete JSON message fully received.
        // i.e. count {} sets in buffer and break on close of the opening {
        // This is faster than waiting on the recv timeout
        b = 0;
            for( i = 0 ; buffer[i] != '\0' ; ++i){
                if ((char)buffer[i] == '{') ++b;
                if ((char)buffer[i] == '}') --b;
            }
        if (b == 0) { // JSON structure complete
            buffer[i] = 0; // terminate with a '\0'
            break; // while loop
        }
    }
    if(debug) printf(" >> %i bytes read\n", total_bytes);
    
    if(bytes_in == -1){
	fprintf(stderr,"ERROR Neohub::getHub recv error");
        //exit(EXIT_FAILURE);
    }
        
    if(debug) printf("getHub(): read: %s\n",buffer);
    close(sockfd);
    if (buffer[0] == 0){ // failed to read anything
        return(nullptr);
    }else{
        /* an error looks like this
        {"error":"Invalid argument to READ_COMFORT_LEVELS, should be a valid device or array of valid devices"}
        */
        //if( strncmp("{\"error",buffer,6)==0){
        if( strncmp(R"({"error")",buffer,6)==0){
            fprintf(stderr,"ERROR Neohub::getHub json command error\n");
            fprintf(stderr,"\tProbably a neostat offline\n");
            return(nullptr);
        }
        return(buffer);
    }
}//getHub()

//*****************************
void Neohub::newStat(Stat n, int index){
    if(neostats == 0 ){// initialisation: push device onto vector
        stats.push_back(n);
    }else{               // Update
        std::cout << "update" << std::endl;
        stats.at(index-1) = n;    // replace existing vector element
    }//
}//newStat()

//*****************************
void Neohub::newTimer(Timer n, int index){ // push Timer onto vector
    if(neostats == 0 ){// initialisation: push device onto vector
        timers.push_back(n);
    }else{               // Update
        timers.at(index-1) = n;    // replace existing vector element
    }//
}//newTimer()

//*****************************
void Neohub::printStats(){ // iterate through the stats
    for(auto it = stats.begin(); it != stats.end(); ++it){
        //std::cout << it->device;
        std::cout << it->getName();
        std::cout << " : ";
        std::cout << it->getTemp();
        std::cout << " (" << it->getSetTemp() << ") ";
        if ( it->isOn()){
            std::cout << " HEATING";
        }
        if ( it->holdTimeHours<int>() + it->holdTimeMins<int>() ){ // time remaining != 0
            std::cout << " Holding "
            << it->holdTemp()
            << " for "
            << it->holdTimeHours<std::string>()
            << ":"
            << it->holdTimeMins<std::string>();
        }
        std::cout << std::endl;
    }
}//printStats()

std::vector<Stat>* Neohub::getStats(){
    return &stats;
}

//*****************************
void Neohub::printTimers(){ // iterate through the timers
    for(auto it = timers.begin(); it != timers.end(); ++it){
        std::cout << it->getName();
        if ( it->isOn()){
            std::cout   << " : ON";
            if ( it->holdTimeHours<int>() + it->holdTimeMins<int>() ){ // time remaining != 0
                std::cout << " Holding for "
                << it->holdTimeHours<std::string>()
                << ":"
                << it->holdTimeMins<std::string>();
            }
        }else{
            std::cout << " : OFF";
        }
        std::cout << std::endl;
    }
}//printTimers()

std::vector<Timer>* Neohub::getTimers(){
    return &timers;
}

//*****************************
void Neohub::printLog(){ // iterate through the stats
    std::cout << timestamp();
    for(auto it = stats.begin(); it != stats.end(); ++it){
        std::cout << ",";
        std::cout << it->getTemp();
    }
    std::cout << std::endl;
}//printLog()

//*****************************
//
//  NeoStatBase   CLASS
//
//*****************************

NeoStatBase::NeoStatBase(){};
NeoStatBase::~NeoStatBase(){};

std::string NeoStatBase::getName(){
    return device;
}//getName()

template<>
std::string NeoStatBase::holdTimeHours<std::string>(){
    std::stringstream ss;
    ss.precision(2);
    ss.width(2);
    ss.fill('0');
    ss << hold_time.hours;
    std::string ret(ss.str());
    return ret;
}//holdTimeHours()
template<> int         NeoStatBase::holdTimeHours<int>(){
    return hold_time.hours;
};

template<>
std::string NeoStatBase::holdTimeMins<std::string>(){
    std::stringstream ss;
    ss.precision(2);
    ss.width(2);
    ss.fill('0');
    ss << hold_time.mins;
    std::string ret(ss.str());
    return ret;
}//holdTimeMins()
template<> int         NeoStatBase::holdTimeMins<int>(){
    return hold_time.mins;
};



//*****************************
//
//  Stat       CLASS
//
//*****************************

Stat::Stat(){}
Stat::~Stat(){}

float Stat::getTemp(){
    return current_temperature;
}//getTemp()

float Stat::getSetTemp(){
    return current_set_temperature;
}//getSetTemp()

bool Stat::isOn(){
    return heating;
}//isOn()


float Stat::holdTemp(){
    return hold_temperature;
}//holdTemp()


void Stat::getComfortLevels(){
    
    char *response;
    // JSON command = {"READ_COMFORT_LEVELS":"<device>"}
    std::string prefix = "{\"READ_COMFORT_LEVELS\":\"";
    std::string postfix = "\"}";
    std::string cmd = prefix + this->device + postfix;
    
    if (debug) printf("[%s].getComfortLevels(): cmd = %s\n",this->device.c_str(),cmd.c_str());
    
    Neohub a;
    response=a.getHub((char *)cmd.c_str());
    if(response==nullptr){
        fprintf(stderr,"Failed to get comfort levels for %s\n",this->device.c_str());
    }else{
        if(debug) printf("  >>%s\n",response);
        json root = json::parse(response);  // parse response into JSON object
        json zone = root[this->device];
        
        std::string periods[] = {"monday","sunday"};
        std::string events[] = {"wake","leave","return","sleep"};
        int event_idx;
        int period_idx = 0;
        
        for(std::string period : periods){
            event_idx = 0;
            json device = zone[period];
            for(std::string event : events){
                json evt = device[event.c_str()];   // evt (JSON array) ::=  [(std::string)"HH:MM",(int)temp]
                comfortLevels[period_idx][event_idx].setComfort(evt[0].get<std::string>(), evt[1].get<int>());
                ++event_idx;
            }// for event
            ++period_idx;
        }// for period
    }//else
}//getComfortLevels()

void Stat::printComfortLevels(){
    
    std::string periods[] = {"monday","sunday"};
    std::string events[] = {"wake","leave","return","sleep"};
    int event_idx;
    int period_idx =0;
    
    std::cout << this->getName() << std::endl;
    for(std::string period : periods){
        std::cout << " " << period;
        event_idx = 0;
        for(std::string event : events){   // // ) ::=  ["HH:MM",(int)temp]
            std::cout << " " << event << " ";
            std::cout << comfortLevels[period_idx][event_idx].getTimeOn();
            
            /*
            std::cout << "[";
            std::cout << (60 * comfortLevels[period_idx][event_idx].getHoursOn()+
                          comfortLevels[period_idx][event_idx].getMinsOn());
            std::cout << "]";
            */
            
            std::cout << " ";
            std::cout << comfortLevels[period_idx][event_idx].getTemp();
            ++event_idx;
        }
        std::cout << std::endl;
        ++period_idx;
    }
    
}//printComfortLevels()

bool Stat::hold(int temp, int hours, int min){
    /*
     JSON Command is of the form
     {"HOLD":[{"temp":24,"id":"radiators","hours":2,"minutes":0},"Eds Office",]}
     The id ("radiators" in this case) is a handle to enable cancellation of a HOLD
     */
    
    char cmd[128]; // big enough for the command string.
    char *result; // and the result
    
    sprintf(cmd,"{\"HOLD\":[{\"temp\":%i,\"id\":\"hold1\",\"hours\":%i,\"minutes\":%i},\"%s\"]}",
                            temp,
                            hours,
                            min,
                            this->device.c_str());
    
    std::cout << cmd << std::endl;
    
    Neohub a;
    result = a.getHub(cmd);
    if(result==nullptr){
        fprintf(stderr, "Hold Failed on %s\n",this->device.c_str());
        return false;
    }
    json R = json::parse(result);
    std::cout << R["result"].get<std::string>() << std::endl;
    if(R["result"].get<std::string>() == "temperature on hold"){
        return true;
    }
    return false;
}


//*****************************
//
//  Switch       CLASS
//
//*****************************
Switch::Switch(){};
Switch::~Switch(){};

void Switch::setTimeOn(int h, int m){
    on.setTime(h,m);
}

void Switch::setTimeOff(int h, int m){
    off.setTime(h,m);
}

std::string Switch::getTimeOn(){
    return (on.asStr());
}//getTimeOn()

std::string Switch::getTimeOff(){
    return (off.asStr());
}//getTimeOff()

int Switch::getHoursOn(){
    return(on.getHours());
}

int Switch::getMinsOn(){
    return(on.getMins());
}


//*****************************
//
//  Comfort       CLASS
//
//*****************************

Comfort::Comfort(){};
Comfort::~Comfort(){};

void Comfort::setComfort(std::string stime, int stemp){
    int h = stoi(stime.substr(0,2));
    int m = stoi(stime.substr(3,2));
    on.setTime(h,m); // on is of class Time
    temp = stemp;
}//setComfort()

void Comfort::print(){
    printf("%s %i",on.asStr().c_str(),temp);
    //printf("%s[%i] %i",timeOn.c_str(),60 * getHoursOn()+getMinsOn(),temp); // DEBUG VERSION
}//print()


int Comfort::getTemp(){
    return (temp);
}//getTemp()

//*****************************
//
//  Event       CLASS
//
//*****************************

Event::Event(){};
Event::~Event(){};

Event::Event(std::string stimeOn,std::string stimeOff){
    int h = stoi(stimeOn.substr(0,2));
    int m = stoi(stimeOn.substr(3,2));
    on.setTime(h,m);
    h = stoi(stimeOff.substr(0,2));
    m = stoi(stimeOff.substr(3,2));
    off.setTime(h,m);
}

void Event::setTimerEvent(std::string stimeOn, std::string stimeOff){
    int h = stoi(stimeOn.substr(0,2));
    int m = stoi(stimeOn.substr(3,2));
    on.setTime(h,m);
    h = stoi(stimeOff.substr(0,2));
    m = stoi(stimeOff.substr(3,2));
    off.setTime(h,m);
}//setTimerEvent()

void Event::print(){
    printf("%s %s",on.asStr().c_str(),off.asStr().c_str());
}//print



//*****************************
//
//  Timer       CLASS
//
//*****************************

Timer::Timer(){}
Timer::~Timer(){}

bool Timer::isOn(){
    return timer;
}//isOn()


bool Timer::holdOn(int mins){
    /*
     JSON Command is of the form
     {"TIMER_ON":"Eds Office"}
     */
    
    char cmd[128]; // big enough for the command string.
    char *result; // and the result
    
    int hold_mins = mins;
    
    if(hold_mins == 0) hold_mins = 59;
    
    sprintf(cmd,"{\"TIMER_HOLD_ON\":[%i,\"%s\"]}",
                    hold_mins,
                    this->device.c_str());
    
    std::cout << cmd << std::endl;
    
    Neohub a;
    result = a.getHub(cmd);
    if(result==nullptr){
        //fprintf(stderr, "Hold Failed on %s\n",this->device.c_str());
        std::cout << "Hold Failed on " << this->device.c_str() << std::endl;
        return false;
    }
    json R = json::parse(result);
    std::cout << R["result"].get<std::string>() << std::endl;
    if(R["result"].get<std::string>() == "timer hold on"){
        return true;
    }
    return false;
}//holdOn()

bool Timer::holdOff(){
    /*
     JSON Command is of the form
     {"TIMER_OFF":"Eds Office"}
     
     */
    
    char cmd[128]; // big enough for the command string.
    char *result; // and the result
    
    sprintf(cmd,"{\"TIMER_HOLD_OFF\":[0,\"%s\"]}",this->device.c_str());
    std::cout << cmd << std::endl;
    
    Neohub a;
    result = a.getHub(cmd);
    if(result==nullptr){
        //fprintf(stderr, "Hold Off Failed on %s\n",this->device.c_str());
        std::cout << "Hold Off Failed on " << this->device.c_str() << std::endl;
        return false;
    }
    json R = json::parse(result);
    std::cout << R["result"].get<std::string>() << std::endl;
   
    if(R["result"].get<std::string>() == "timer hold off"){
        return true;
    }
    return false;
}//holdOff()

void Timer::getTimerEvents(){
    
    char *response;
    // JSON command = {"READ_TIMECLOCK":"<device>"}
    std::string prefix = "{\"READ_TIMECLOCK\":\"";
    std::string postfix = "\"}";
    std::string cmd = prefix + this->device + postfix;
    
    
    Neohub a;
    response=a.getHub((char *)cmd.c_str());
    //std::cout << response << std::endl;
    if(response==nullptr){
        fprintf(stderr,"Failed to get timer events for %s\n",this->device.c_str());
    }else{
        json root = json::parse(response);  // parse response into JSON object
        json zone = root[this->device];
        
        std::string periods[] = {"monday","sunday"};
        std::string events[] = {"time1","time2","time3","time4"};
        int event_idx;
        int period_idx = 0;
        
        for(std::string period : periods){
            event_idx = 0;
            json device = zone[period];
            for(std::string event : events){   // // ) ::=  ["HH:MM","HH:MM"]
                json evt = device[event.c_str()];
                timerEvents[period_idx][event_idx].setTimerEvent(evt[0].get<std::string>(), evt[1].get<std::string>());
                ++event_idx;
            }//for event
            ++period_idx;
        }//for period
    }
}//getTimerEvents

void Timer::printTimerEvents(){
    
    std::string periods[] = {"monday","sunday"};
    std::string events[] = {"time1","time2","time3","time4"};
    int event_idx;
    int period_idx =0;
    
    std::cout << this->getName() << std::endl;
    for(std::string period : periods){
        std::cout << " " << period;
        event_idx = 0;
        for(std::string event : events){   // // ) ::=  ["HH:MM","HH:MM"]
            std::cout << " " << event << " ";
            std::cout << timerEvents[period_idx][event_idx].getTimeOn();
            std::cout << " ";
            std::cout << timerEvents[period_idx][event_idx].getTimeOff();
            ++event_idx;
        }//for event
        std::cout << std::endl;
        ++period_idx;
    }//for period
}//printTimerEvents()
