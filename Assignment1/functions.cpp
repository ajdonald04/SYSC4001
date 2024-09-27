/**
 * Implementation file for methods used
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364
 */
#include <fstream>
#include "functions.h"
#include "utils.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream> 
#include <sstream>
#include <vector>

// Global variable to track simulation time
static uint32_t sim_time = 0;


void eventHandler(TraceEvent event)
{
    std::vector<uint16_t> isrAddresses = vectorTableHandler("vector_table.txt");
    if (event.name == "CPU") {
        event.name = "CPU Execution";
        logExecution(event.duration, event.name);
    }
    if(event.ID < isrAddresses.size())
    {
        uint16_t ISRAddress = isrAddresses[event.ID];
        uint16_t memoryPosition = event.ID * 2; 
        
        if (event.name == "SYSCALL") {
            // Mode switching = 1ms
            logExecution(1, "Switch to Kernel Mode");

            // Save context is random between 1 - 3 ms
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #"+std::to_string(event.ID) + " in memory position 0x"+std::to_string(memoryPosition));
            logExecution(1, "Load address 0x"+std::to_string(ISRAddress)+ " into PC")

            // Execute ISR and IRET
            logExecution(event.duration, "SYSCALL: run the ISR");
            logExecution(1, "IRET");
        }
        else if (event.name == "END_IO") {
            logExecution(1, "Check the priority of the Interrupt");
            logExecution(1, "Check if the interrupt is masked"); 
            logExecution(1, "Switch to Kernel Mode"); 
            logExecution(rand() % 3 + 1, "Save Context"); 
            logExecution(1, "Find vector #"+std::to_string(event.ID)+ " in memory position 0x"+std::to_string(memoryPosition))
                logExecution(1, "Load address 0x" + std::to_string(ISRAddress));
            logExecution(event.duration, "I/O Completed");
            logExecution(1, "IRET");
        }
    }
}


// Function to log the execution of each event
void logExecution(uint32_t duration, const std::string eventName) {
    
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
}

void inputRead(std::string fileName){
    // Set the input file name (this was missing in your code)
    
    std::ifstream inputFile(fileName); 

    if(!inputFile) 
    {
        std::cerr << "Error when opening file: " << fileName << std::endl; 
        return;  // Make sure to return if the file can't be opened!
    }

    std::string line;
    std::vector<TraceEvent> events;  // Vector to store parsed events
    
    // Read the file line by line
    while(std::getline(inputFile, line))
    {
        std::istringstream iss(line);
        TraceEvent event; 

        if(line.find("CPU") != std::string::npos)
        {
            event.name = "CPU";
            char comma; 
            iss >> event.name >> comma >> event.duration; 
        }
        else if (line.find("SYSCALL") != std::string::npos)
        { 
            std::string sysCALL; 
            iss >> sysCALL >> event.ID;
            char comma;
            iss >> comma >> event.duration;
            event.name = "SYSCALL";
        }
        else if (line.find("END_IO") != std::string::npos)
        {
            std::string endIO; 
            iss >> endIO >> event.ID; 
            char comma; 
            iss >> comma >> event.duration; 
            event.name = "END_IO";
        }
        // Adding events to the vector 
        events.push_back(event);
    }
    
    inputFile.close(); 

    // Output the events for verification
    for (const auto& event : events)
    {
        eventHandler(event);
    }
}

std::vector<uint16_t> vectorTableHandler(std::string fileName)
{
    std::vector<uint16_t> isrAddresses; 

    std::ifstream inputFile(fileName);

    std::string line; 

    while(getfile(inputFile, line))
    {
        std::istringstream iss(line); 

        uint16_t ISRAddress; 

        iss >> std::hex >> ISRAddress; 

        isrAddresses.push_back(ISRAddress);
    }
    inputFile.close();

    return isrAddresses;
    
}