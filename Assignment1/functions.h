/**
 * Definition of functions for SYSC4001 A1
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364?? 
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "utils.h"

void logExecution(uint32_t duration, std::string eventName);

void eventHandler(TraceEvent& event);

void inputRead(std::string fileName); 

//void vectorTableHandler(); 
#endif