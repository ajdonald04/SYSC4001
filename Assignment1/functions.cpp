/**
 * Implementation file for methods used
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364
 */
#include <fstream>
#include "utils.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream> 

static int sim_time = 0; 
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Global variable to track simulation time
static uint32_t sim_time = 0;

void eventHandler(TraceEvent& event)
{
    if (event.name == "CPU") {
        event.name = "CPU Execution";
        logExecution(event.duration, event.name);
    }
    else if (event.name == "SYSCALL") {
        // Mode switching = 1ms
        logExecution(1, "Switch to Kernel Mode");

        // Save context is random between 1 - 3 ms
        logExecution(rand() % 3 + 1, "Save Context");
        logExecution(1, "Find vector in memory");

        // Execute ISR and IRET
        logExecution(event.duration, "Execute ISR");
        logExecution(1, "IRET");
    }
    else if (event.name == "END_IO") {
        logExecution(event.duration, "I/O Completed");
    }
}

// Function to log the execution of each event
void logExecution(uint32_t duration, const std::string &eventName) {
    // Open the file in append mode
    std::ofstream outputFile("execution.txt", std::ios::app);

    if (outputFile.is_open()) {
        // Log the event (simulation time, duration, and event name)
        outputFile << sim_time << ", " << duration << ", " << eventName << std::endl;

        // Add event duration to total simulation time
        sim_time += duration;

        // Close the file after writing
        outputFile.close();
    }
    else {
        std::cerr << "Error: Unable to open execution.txt file for logging" << std::endl;
    }

void vectorTableHandler(){

    // change for test
    std::string filename = "vector_table.txt";
    std::string lines;

    vector<VectorTableEntry> vectorTable;
    

    std::fstream.file(filename);

    if (!file){
        std::cerr << "Unable to open file" << std::endl;
        return 0;
    }

    while(std::getline(filename, line)){
        //init a new vector table entry structure
        //split the vector table line into correct segments for the structure
        //cast the string values to ints
        //cast the int values to uint16
        //VectorTableEntry vectorInstruction; 
        //vectorTable.pushback(vectorInstruction.
    }

    

}
void inputRead()
{
    // create an input file object
    std::string fileName; 
    
    std::ifstream inputFile(fileName); 

    if(!inputFile)
    {
        std::cerr << "Error when opening file : " << fileName << std::endl; 
    }

    std::string line;

    while(std::getline(inputFile, line))
    {
        std::istringstream iss(line);
        TraceEvent event; 

        if(line.find("CPU") != std::string::npos) // might not need std::string::npos, no need to check 
        {
            event.name = "CPU";
            char comma; 
            iss >> event.name >> comma >> event.duration; 
        }

        else if (line.find("SYSCALL" != std::string::npos)
        { 
            std::string sysCALL; 
            iss >> sysCALL >> event.ID
            char comma;
            iss >> comma >> 
            event.name = "SYSCCALL";
        

        }

    }
    std::ifstream inputFile("trace1.txt");
}
