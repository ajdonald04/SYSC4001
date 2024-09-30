/**
 * Definition of functions for SYSC4001 A1
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include "utils.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>  
#include <fstream>

void logExecution(uint32_t duration, std::string eventName);

void eventHandler(TraceEvent event, std::string fileName);

void inputRead(std::string fileName, std::string vectorFileName); 

std::vector<uint16_t> vectorTableHandler(std::string fileName); 

#endif
