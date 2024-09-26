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

void inputRead(){
    // Set the input file name (this was missing in your code)
    std::string fileName = "trace1.txt"; 
    
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

    // Output the parsed events for verification
    for (const auto& event : events)
    {
        if(event.name == "CPU")
        {
            std::cout << "Event: " << event.name << ", Duration: " << event.duration << std::endl; 
        }
        else
        {
            std::cout << "Event: " << event.name << " " << event.ID << ", Duration: " << event.duration << std::endl; 
        }
    }
}