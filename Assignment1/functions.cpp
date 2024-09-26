/**
 * Implementation file for methods used
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101259149
 */
#include <fstream>
#include "functions.h"

static int sim_time = 0; 
void eventHandler(EventType event)
{
    if(event.eventName == "CPU")
    {
        logExecution(event.duration, "CPU Execution");
    }
}

void logExecution(EventType& event);
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