//
//  functions.cpp
//  nprobe
//
//  Created by Ed Cole on 28/09/2018.
//  Copyright © 2018 Ed Cole. All rights reserved.
//

#include "neohub.hpp"
extern bool debug;

//************ stripString()
char *stripString(char *str){
    // strip out unquoted whitespace and new lines from a string.
    // modifies contents at *str and and returns str
    bool quote = false;
    char *from = str;
    char *to = str;
    
    while( *from != '\0'){
        if( *from == '"') quote = !quote;
        if( isspace(*from) == 0 ){ // not a space
            *to = *from;
            to++;
        }else{ // it is a space
            if( quote ){
                *to = *from;
                to++;
            }
        }
        from++;
    }
    *to = '\0';
    return(str);
}
//END************* stripString()

//
//*************** readJson()
bool readJson(char *file, char *buffer, int max){
    // read a file into buffer - used for debugging

    FILE *jsonfd;
    int n =0;


    if( file == nullptr){
        jsonfd = stdin;
    }else{
        if( (jsonfd = fopen(file, "r")) == nullptr){
            fprintf(stderr,"ERROR: Unable to open file %s\n",file);
            return false;
        };
    }
    
    while ( (buffer[n++] = fgetc(jsonfd)) != EOF){
        if (n >= max){
            fprintf(stderr,"ERROR: Buffer overflow\n");
            return false;
        }//if
    }//while

    return true;
}//END***********readJson

// ************   timestamp()
char *timestamp(){ // for log line output
    // returns pointer to static string containing current date & time
    static char timestamp[] = "dd-mm-yyyy,hh:mm";
    time_t now = time(0);
    tm *ltm = localtime(&now);
    sprintf(timestamp,"%02d-%02d-%4d,%02d:%02d",
            ltm->tm_mday,
            1 + ltm->tm_mon,
            1900 + ltm->tm_year,
            ltm->tm_hour,
            ltm->tm_min);
    return(timestamp);
}// ************   timestamp()


//************* wneohub()
/*
 reads the file containing one or more JSON commands and squirts it/them to the neohub.
 Messy kludge retains backwards compatability with format of JSON file used by the
 predesessor of nprobe (wneohub) which needs a '&' between each JSON command if multiple
 commands are contained in one file. This is still supported, but not required as matching
 paired top level braces ({}) delimit each JSON command.
 */
bool wneohub(char *file){

char buffer[READ_BUFFER_SZ];
char cmd[READ_BUFFER_SZ];
int n=0;
int c=0;
int brace;
Neohub hub;
    bzero(buffer,READ_BUFFER_SZ);
    readJson((char *)file, &buffer[0],READ_BUFFER_SZ);
    while(buffer[n] != '\0'){
        if(buffer[n] == '#'){ // scan over comment lines
            while(buffer[n++] != '\n'){}; // read to end of line.
            continue;
        }
        if(buffer[n] == '\n'){ // loose newlines
            n++;
            continue;
        }
        // command parse loop
        brace = 0;
        c=0;
        
        do{
            switch ((int)buffer[n]){
                case '{':
                    ++brace;
                    cmd[c++] = buffer[n++];
                break;
                case '}':
                    --brace;
                    cmd[c++] = buffer[n++];
                break;
                
                case '#':    // scan over comment embedded within command.
                    while(buffer[n++] != '\n'){}; // read to end of line.
            
                default:
                    cmd[c++] = buffer[n++];
            }//switch
        }while(brace != 0);
        
        if(buffer[n] == '&') {n++;} // backward compatability with old wneohub tool
        cmd[c]='\0';
        if(cmd[0] == '{'){ // catches the last newline in the file.
            if (debug) {
                std::cout << "========================================\n" << cmd << std::endl;
            }
            // send command to neohub and print result
            std::cout << hub.getHub(cmd) << std::endl;
        }//if(cmd[0]
    }//while
    return true;
}

//********** END OF FILE





