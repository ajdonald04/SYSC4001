/**
 * Implementation file for methods used
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101259149
 */
#include <fstream>
#include "functions.h"
#include <cstdlib> 
#include <ctime>
static int sim_time = 0; 
void eventHandler(TraceEvent& event)
{
    if(event.name == "CPU")
    {
        event.name = "CPU Execution";
        logExecution(event.duration, event.name);
    }
    else if(event.name == "SysCall" && event.ID == 1)
    {
        // assumption here is we use IDs to help identify the different types of system calls
        // struct member only needed for that
        
        // mode switching = 1ms 
        logExecution(1, "Switch to Kernel Mode");
        // save context is random between 1 - 3 ms 

        logExecution(rand() % 3 + 1, "Save Context");
        logExecution(1, "Find vector in memory");
        logExecution(event.duration, "Execute ISR"); 
        logExecution(1, "IRET");

    }
    else if(event.name == "SysCall" && event.ID != 1)
    {
        // per assignment description 
        
    }


}

void logExecution(uint32_t duration, std::string &eventname);
{
    // using ofstream as a variable to store the file, it's name is execution.txt and it's in append mode

    std::ofstream outputFile("execution.txt", std::ios::app); 

    if(outputFile.is_open())
    {
        // accessing the simulation time variable, which is global and static. 
        extern int sim_time; 

        // writing data to execution.txt 
        outputFile << sim_time << ", " << event.duration << ", " << event.name << std::endl; 

        // add event duration to total sim_time
        sim_time += event.duration; 

        outputFile.close(); 
    }

    else
    {
        std::cerr << "Error: Unable to open execution.txt file for logging" << std::endl;
    }

}