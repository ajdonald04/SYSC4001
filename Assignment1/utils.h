/**
 * Utilities used for Assignemnt 1
 * 
 * Declaring functions, structures
 * 
 */
#include <iostream>  
#include <stdint.h> 
#

#ifndef UTILS_H
#define UTILS_H


struct EventType
{
    std::string name; 
    uint8_t eventID; 
    uint32_t duration; 
}; 

struct VectorTableEntry { 
    uint32_t interruptNum; 
    uint32_t ISRAddress; 
};

#endif