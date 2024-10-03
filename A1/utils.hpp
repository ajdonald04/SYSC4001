#ifndef UTILS_HPP
#define UTILS_HPP

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


#endif
