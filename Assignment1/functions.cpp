/**
 * Implementation file for methods used
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364
 */
#include <fstream>
#include "functions.h"
#include <cstdlib> 
#include <ctime>
#include <string>
#include <iostream> 

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
        logExecution(1, "Switch to Kernel Mode");
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
