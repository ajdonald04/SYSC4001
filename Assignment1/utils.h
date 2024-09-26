#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <stdint.h>

// Structure to define an event
typedef struct TraceEvent
{
    std::string name; 
    uint8_t ID; 
    uint32_t duration; 
} trace_t;

// Structure for vector table entry
struct VectorTableEntry { 
    uint16_t InitMemAddress; 
    uint16_t InterruptNum; 
    uint16_t ISRAddress; 
};



#endif
