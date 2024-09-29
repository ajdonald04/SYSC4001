#include <fstream>
#include "functions.h"
#include "utils.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>  // For hex formatting

// Global variable to track simulation time
static uint32_t sim_time = 0;

// Utility function to convert decimal to hex with a fixed width
std::string toHex(uint16_t value, int width) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;
    return ss.str();
}

void eventHandler(TraceEvent event)
{
    std::vector<uint16_t> isrAddresses = vectorTableHandler("vector_table.txt");
    int vectorTableSize = isrAddresses.size();

    if (vectorTableSize == 0) {
        std::cerr << "Error: Vector table is empty!" << std::endl;
        return;
    }

    if(event.name == "CPU")
    {
        logExecution(event.duration, "CPU Execution");
    }

    // Ensure that the event ID is within bounds of the vector table
    if (event.ID >= 0 && event.ID < vectorTableSize) {
        uint16_t ISRAddress = isrAddresses[event.ID-1];
        uint16_t memoryPosition = event.ID * 2;  // Memory position formula

        if (event.name == "SYSCALL") {
            // SYSCALL handling
            logExecution(event.duration, "Switch to Kernel Mode");
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #" + std::to_string(event.ID) + " in memory position 0x" + toHex(memoryPosition, 4));
            logExecution(1, "Load address 0x" + toHex(ISRAddress, 4) + " into PC");
            logExecution(event.duration, "SYSCALL: run the ISR");
            logExecution(1, "IRET");
        } 
        else if (event.name == "END_IO") {
            // END_IO handling
            logExecution(1, "Check the priority of the Interrupt");
            logExecution(1, "Check if the interrupt is masked");
            logExecution(1, "Switch to Kernel Mode");
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #" + std::to_string(event.ID) + " in memory position 0x" + toHex(memoryPosition, 4));
            logExecution(1, "Load address 0x" + toHex(ISRAddress, 4));
            logExecution(event.duration, "I/O Completed");
            logExecution(1, "IRET");
        }
    } else {
        std::cerr << "Error: Invalid event ID for vector table lookup: " << event.ID 
                  << ". The vector table size is " << vectorTableSize << std::endl;
    }
}

void logExecution(uint32_t duration, const std::string eventName) {
    std::ofstream outputFile("execution.txt", std::ios::app);

    if (outputFile.is_open()) {
        // Log the event (simulation time, duration, and event name)
        outputFile << sim_time << ", " << duration << ", " << eventName << std::endl;

        // Add event duration to total simulation time
        sim_time += duration;

        // Close the file after writing
        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open execution.txt file for logging" << std::endl;
    }
}

void inputRead(std::string fileName) {
    std::ifstream inputFile(fileName);

    if (!inputFile) {
        std::cerr << "Error when opening file: " << fileName << std::endl;
        return;
    }

    std::string line;
    std::vector<TraceEvent> events;

    // Read the file line by line
    while (std::getline(inputFile, line)) {
        TraceEvent event;
        std::stringstream ss(line);
        std::string activity;
        std::string durationOrID;

        // Split the line by comma
        if (std::getline(ss, activity, ',') && std::getline(ss, durationOrID, ',')) {
            std::stringstream durationStream(durationOrID);  // For parsing duration after the comma

            // Handle CPU events
            if (activity.find("CPU") != std::string::npos) {
                event.name = "CPU";
                durationStream >> event.duration;
                std::cout << "Parsed CPU event: duration = " << event.duration << std::endl;
            } 
            // Handle SYSCALL and END_IO events with multi-digit IDs
            else if (activity.find("SYSCALL") != std::string::npos || activity.find("END_IO") != std::string::npos) {
                event.name = activity.substr(0, activity.find_first_of(' '));  // Extract the name (SYSCALL/END_IO)
                event.ID = std::stoi(activity.substr(activity.find_last_of(' ') + 1));  // Extract multi-digit ID
                durationStream >> event.duration;
                std::cout << "Parsed " << event.name << " event: ID = " << event.ID << ", duration = " << event.duration << std::endl;
            }

            // Add the event to the vector
            events.push_back(event);
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    inputFile.close();

    // Process the events
    for (const auto& event : events) {
        eventHandler(event);  // Call eventHandler for each event
    }
}


std::vector<uint16_t> vectorTableHandler(std::string fileName) {
    std::vector<uint16_t> isrAddresses;
    std::ifstream inputFile(fileName);

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        uint16_t ISRAddress;
        iss >> std::hex >> ISRAddress;
        isrAddresses.push_back(ISRAddress);
    }
    inputFile.close();

    return isrAddresses;
}
