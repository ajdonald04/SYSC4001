/**
 * Definition of functions used for SYSC4001 A2
 * 
 * Authors: Aj Donald 101259149, Simon Hagos
 * 
 *Note: Due to the difficulties encountered during the assignment of the teams for A2, 
this assignment had already been completed with my previous partner from A1, (Jayven Larsen, 101260364).
 As a result, the solutions of this submission for all parts was completed with him. Having discussed with Professor Wainer,
 we have understood that there shouldn't be any risk of plagiarism between our two groups given this mistake. 
 */

#ifndef INTERRUPTS_HPP
#define INTERRUPTS_HPP

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>  
#include <fstream>
#include <stdint.h>
#include <algorithm> 


struct TraceEvent
{
    std::string name; 
    uint8_t ID; 
    uint32_t duration; 
} ;

struct ExternalFile
{
    std::string program_name; 
    uint8_t size; 
};
struct PCB
{ 
    uint8_t pid;
    uint8_t cpu_time; 
    uint8_t IO_time; 
    uint8_t rem_cpu; 
    uint8_t partition_num; 
    std::string state;
};

struct memoryPartition
{
    uint8_t num; 
    uint8_t size; 
    std::string code; //
};


// A1 functions 
void logExecution(uint32_t duration, std::string eventName);

void eventHandler(TraceEvent event, std::string fileName);

void inputRead(std::string traceFileName, std::string vectorFileName, std::string outputFileName);

std::vector<uint16_t> vectorTableHandler(std::string fileName); 

std::string toHex(uint16_t value, int width);


// new functions 

void initMemory(); 

void loadExternalFiles(std::string fileName);

void forkProcess(uint8_t parentPID);

void execProcess(uint8_t childPid, std::string programName, std::string vectorFileName);

void logSystemStatus(); 

#endif
