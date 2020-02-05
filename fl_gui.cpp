//
//  fl_gui.cpp
//  nprobe
//
//  Created by Ed Cole on 05/02/2020.
//  Copyright © 2020 colege. All rights reserved.
//

#include "fl_gui.hpp"
#include "neohub.hpp"

extern bool debug;

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
