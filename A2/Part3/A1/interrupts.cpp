/**
 * Implementation of functions for SYSC4001 A1
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364
 */
#include "interrupts.hpp"


// variable to track simulation time
static uint32_t sim_time = 0;
std::string filename;

// declaring vectors for structures (partitions, pcb and external files)
std::vector<memoryPartition> memoryPartitions;
std::vector<PCB> pcbTable; 
std::vector<ExternalFile> externalFiles; 


void initMemory()
{ 
    memoryPartitions = 
    { 
        // from the instructions
        {1,40, "free"},
        {2, 25, "free"},
        {3, 15, "free"}, 
        {4, 10, "free"}, 
        {5, 8, "free"}, 
        {6, 2, "init"} 
    }; 
    // initialize the process in the 6th partition. 
    pcbTable.push_back({0, 0, 0, 0, 6, "Running"});
}

void loadExternalFiles(std::string fileName) {

    // similar to A1, reading an input file 
    std::ifstream inputFile(fileName);
    std::string line;

    // loop over the file, line by line
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        // declaring the file as a two component structure
        ExternalFile file;

        iss >> file.program_name >> file.size;

        // add the files structs into the vector
        externalFiles.push_back(file);
    }
    inputFile.close();
}

void forkProcess(uint8_t parentPid) {
    // Find the parent's associated PCB to get its details
    auto parentIt = std::find_if(pcbTable.begin(), pcbTable.end(),
                                 [parentPid](const PCB& pcb){ return pcb.pid == parentPid; 
    });
    if (parentIt != pcbTable.end()) {
        PCB child = *parentIt;
        child.pid = pcbTable.size(); // Assign a new PID
        child.state = "Ready";
        pcbTable.push_back(child);

        logExecution(2, "Forked process PID " + std::to_string(child.pid));
    }
}

void logSystemStatus() {

    // similar to the previous writing to an output file
    std::ofstream outputFile("system_status.txt", std::ios::app);

    // quick check if the file is open 
    if (outputFile.is_open()) {
        outputFile << "Current Simulated Time: " << sim_time << " ms\n";
        outputFile << "PCB Table:\n";
        outputFile << "PID\tPartition\tState\tRemaining CPU Time\n";
        for (const auto& pcb : pcbTable) {
            outputFile << pcb.pid << "\t"
                       << pcb.partition_num << "\t"
                       << pcb.state << "\t"
                       << pcb.rem_cpu << " ms\n";
        }

        outputFile << "\nMemory Partitions:\n";
        outputFile << "Partition\tSize\tStatus\n";
        for (const auto& partition : memoryPartitions) {
            outputFile << partition.num << "\t\t"
                       << partition.size << " MB\t"
                       << partition.code << "\n";
        }
        outputFile << "---------------------------------------------\n";
        outputFile.close();
    } else { // error reporting if the file cant be opened
        std::cerr << "Error: Unable to open system_status.txt for logging\n";
    }
}

int BestFitPartition(uint8_t programSize) {
    int bestIndex = -1;
    uint8_t smallestSize = UINT32_MAX;

    for (size_t i = 0; i < memoryPartitions.size(); ++i) {
        if (memoryPartitions[i].code == "free" && memoryPartitions[i].size >= programSize) {
            if (memoryPartitions[i].size < smallestSize) {
                smallestSize = memoryPartitions[i].size;
                bestIndex = i;
            }
        }
    }
    return bestIndex;
}

void scheduler() {
    logExecution(1, "Scheduler called");
    std::cout << "Scheduler called" << std::endl;
}

void execProcess(uint8_t childPid, const std::string& programName) {
    // Find the child process PCB
    auto childIt = std::find_if(pcbTable.begin(), pcbTable.end(),
                                [childPid](const PCB& pcb) { return pcb.pid == childPid; });
    if (childIt == pcbTable.end()) {
        std::cerr << "Error: Child process with PID " << childPid << " not found.\n";
        return;
    }

    // Find the program size from the external files list
    auto programIt = std::find_if(externalFiles.begin(), externalFiles.end(),
                                  [&programName](const ExternalFile& file) { return file.program_name == programName; });
    if (programIt == externalFiles.end()) {
        std::cerr << "Error: Program " << programName << " not found in external files.\n";
        return;
    }
    uint8_t programSize = programIt->size;

    // Find bestfit memory partition 
    int partitionIndex = BestFitPartition(programSize);
    if (partitionIndex == -1) {
        std::cerr << "Error: No suitable partition found for program " << programName << ".\n";
        return;
    }

    // Mark the partition as occupied by the program
    memoryPartitions[partitionIndex].code = programName;

    // Update the child's PCB with the new partition information
    childIt->partition_num = memoryPartitions[partitionIndex].num;
    childIt->state = "Running";
    
    logExecution(rand() % 10 + 1, "Program " + programName + " loaded into Partition " +
                 std::to_string(memoryPartitions[partitionIndex].num));
    
    // Call the scheduler
    scheduler();

    // Return from ISR (simulated)
    logExecution(1, "Return from EXEC ISR");
}



std::string toHex(uint16_t value, int width) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;
    return ss.str();
}

void eventHandler(TraceEvent event, std::string fileName)
{
    std::vector<uint16_t> isrAddresses = vectorTableHandler(fileName);
    int vectorTableSize = isrAddresses.size();

    if (vectorTableSize == 0) {
        std::cerr << "Error: Vector table is empty!" << std::endl;
        return;
    }

    if(event.name == "CPU")
    {
        logExecution(event.duration, "CPU Execution");
    }

    // check that event ID is within bounds of the vector table
    if (event.ID >= 0 && event.ID < vectorTableSize) {
        uint16_t ISRAddress = isrAddresses[event.ID-1];
        uint16_t memoryPosition = event.ID * 2;  // Memory position formula

        if (event.name == "SYSCALL") {
            logExecution(1, "Switch to Kernel Mode");
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #" + std::to_string(event.ID) + " in memory position 0x" + toHex(memoryPosition, 4));
            logExecution(1, "Load address 0x" + toHex(ISRAddress, 4) + " into PC");
            logExecution(event.duration, "SYSCALL: run the ISR");
            logExecution(1, "IRET");
        } 
        else if (event.name == "END_IO") {
            logExecution(1, "Check the priority of the Interrupt");
            logExecution(1, "Check if the interrupt is masked");
            logExecution(1, "Switch to Kernel Mode");
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #" + std::to_string(event.ID) + " in memory position 0x" + toHex(memoryPosition, 4));
            logExecution(1, "Load address 0x" + toHex(ISRAddress, 4));
            logExecution(event.duration, "I/O Completed");
            logExecution(1, "IRET");
        }
    } 
}


void logExecution(uint32_t duration, const std::string eventName) {
    

    std::ofstream outputFile(filename, std::ios::app);


    if (outputFile.is_open()) {
        // log an event (sim time, duration, and event name)
        outputFile << sim_time << ", " << duration << ", " << eventName << std::endl;

        sim_time += duration;

        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open execution.txt file for logging" << std::endl;
    }
}

void inputRead(std::string traceFileName, std::string vectorFileName, std::string outputFileName) {
    std::ifstream inputFile(traceFileName);

    filename = outputFileName+".txt";

    if (!inputFile) {
        std::cerr << "Error when opening file: " << traceFileName << std::endl;
        return;
    }

    std::string line;
    std::vector<TraceEvent> events;

    
    while (std::getline(inputFile, line)) {
        TraceEvent event;
        std::stringstream ss(line);
        std::string activity;
        std::string durationOrID;

        if (std::getline(ss, activity, ',') && std::getline(ss, durationOrID, ',')) {
            std::stringstream durationStream(durationOrID);  

            if (activity.find("CPU") != std::string::npos) {
                event.name = "CPU";
                durationStream >> event.duration;
            } 
            // Handle SYSCALL and END_IO events with multi-digit IDs
            else if (activity.find("SYSCALL") != std::string::npos || activity.find("END_IO") != std::string::npos) {
                event.name = activity.substr(0, activity.find_first_of(' '));  
                event.ID = std::stoi(activity.substr(activity.find_last_of(' ') + 1));  
                durationStream >> event.duration;
            }

            // add events to vector
            events.push_back(event);
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    inputFile.close();

    // process events
    for (const auto& event : events) {
        eventHandler(event, vectorFileName);  
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

void initMemory() 
{
    
}

int main()
{

    std::string vectorFileName, TraceFile, outputFileName; 

    std::cout << "Enter the vector table file name: "; 
    std::cin >> vectorFileName; 
    

    std::cout << "Enter the trace file name: ";
    std::cin >> TraceFile; 

    std::cout << "Enter the file name: (without extension): ";
    std::cin >> outputFileName; 

    inputRead(TraceFile, vectorFileName,outputFileName);
    
    return 0;
}
