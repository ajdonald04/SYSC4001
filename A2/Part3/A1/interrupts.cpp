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


// Memory initialization function.
void initMemory() {
    memoryPartitions.emplace_back(memoryPartition{1, 40, "free"});
    memoryPartitions.emplace_back(memoryPartition{2, 25, "free"});
    memoryPartitions.emplace_back(memoryPartition{3, 15, "free"});
    memoryPartitions.emplace_back(memoryPartition{4, 10, "free"});
    memoryPartitions.emplace_back(memoryPartition{5, 8, "free"});
    memoryPartitions.emplace_back(memoryPartition{6, 2, "init"});

    // Initialize the PCB with the 'init' process.
    pcbTable.emplace_back(PCB{0, 0, 0, 0, 6, "Running"});
}

void logSystemStatus() {
    std::ofstream outputFile("system_status.txt", std::ios::app);

    if (outputFile.is_open()) {
        outputFile << "!-----------------------------------------------------------!\n";
        outputFile << "Save Time: " << sim_time << " ms\n";
        outputFile << "+--------------------------------------------+\n";
        outputFile << "| PID |Program Name |Partition Number | size |\n";
        outputFile << "+--------------------------------------------+\n";
        for (const auto& pcb : pcbTable) {
            auto programName = memoryPartitions[pcb.partition_num - 1].code;
            outputFile << "| " << std::setw(3) << pcb.pid << " | "
                       << std::setw(12) << programName << " | "
                       << std::setw(15) << pcb.partition_num << " | "
                       << std::setw(4) << memoryPartitions[pcb.partition_num - 1].size << " |\n";
        }
        outputFile << "+--------------------------------------------+\n";
        outputFile << "!-----------------------------------------------------------!\n";
        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open system_status.txt for logging\n";
    }
}

int BestFitPartition(uint8_t programSize) {
    int bestIndex = -1;
    uint32_t smallestSize = UINT32_MAX;

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

void forkProcess(uint8_t parentPid) {
    auto parentIt = std::find_if(pcbTable.begin(), pcbTable.end(),
                                 [parentPid](const PCB& pcb) {
                                     return pcb.pid == parentPid;
                                 });
    if (parentIt != pcbTable.end()) {
        PCB child = *parentIt;
        child.pid = pcbTable.size(); // Assign a new PID.
        child.state = "Ready";
        pcbTable.push_back(child);

        logExecution(2, "Forked process PID " + std::to_string(child.pid));
    }
}

void execProcess(uint8_t childPid, std::string programName, std::string vectorFileName) {
    auto childIt = std::find_if(pcbTable.begin(), pcbTable.end(),
                                [childPid](const PCB& pcb) {
                                    return pcb.pid == childPid;
                                });
    if (childIt == pcbTable.end()) {
        std::cerr << "Error: Child process with PID " << childPid << " not found.\n";
        return;
    }

    auto programIt = std::find_if(externalFiles.begin(), externalFiles.end(),
                                  [&programName](const ExternalFile& file) {
                                      return file.program_name == programName;
                                  });
    if (programIt == externalFiles.end()) {
        std::cerr << "Error: Program " << programName << " not found in external files.\n";
        return;
    }

    uint8_t programSize = programIt->size;

    // Find the best-fit memory partition.
    int partitionIndex = BestFitPartition(programSize);
    if (partitionIndex == -1) {
        std::cerr << "Error: No suitable partition found for program " << programName << ".\n";
        return;
    }

    // Mark the partition as occupied by the program.
    memoryPartitions[partitionIndex].code = programName;

    // Update the child's PCB with the new partition information.
    childIt->partition_num = memoryPartitions[partitionIndex].num;
    childIt->state = "Running";
    
    logExecution(rand() % 10 + 1, "EXEC: load " + programName + " of size " + std::to_string(programSize) + "MB");
    logExecution(rand() % 10 + 1, "found partition " + std::to_string(memoryPartitions[partitionIndex].num) + 
                 " with " + std::to_string(memoryPartitions[partitionIndex].size) + "MB of space");
    logExecution(rand() % 10 + 1, "partition " + std::to_string(memoryPartitions[partitionIndex].num) + 
                 " marked as occupied");
    logExecution(rand() % 10 + 1, "updating PCB with new information");

    // Call the scheduler.
    scheduler();

    // Log the current system status.
    logSystemStatus();

    // Simulate reading the program trace.
    std::string programTraceFile = programName + ".txt";
    inputRead(programTraceFile, vectorFileName, filename);

    // Return from ISR (simulated).
    logExecution(1, "Return from EXEC ISR");
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

    // Set the global filename for logging execution events.
    filename = outputFileName + ".txt";

    // Check if the input trace file opens successfully.
    if (!inputFile) {
        std::cerr << "Error when opening file: " << traceFileName << std::endl;
        return;
    }

    std::string line;

    // Read the trace file line by line.
    while (std::getline(inputFile, line)) {
        std::stringstream ss(line);
        std::string command;
        ss >> command;

        // Handle FORK commands.
        if (command == "FORK") {
            uint8_t parentPid;
            ss.ignore(1, ','); // Skip the comma.
            ss >> parentPid;

            if (ss.fail()) {
                std::cerr << "Error parsing FORK command: " << line << std::endl;
                continue;
            }

            std::cout << "Parsed FORK command: parent PID = " << static_cast<int>(parentPid) << std::endl;
            forkProcess(parentPid);
            logSystemStatus();
        } 
        // Handle EXEC commands.
        else if (command == "EXEC") {
            std::string programName;
            ss.ignore(1, ','); // Skip the comma.
            ss >> programName;

            if (ss.fail()) {
                std::cerr << "Error parsing EXEC command: " << line << std::endl;
                continue;
            }

            // Create a new child process based on the last forked process or use the next PID.
            uint8_t childPid = pcbTable.size(); // Use the next available PID.

            // Add the child process to the PCB table.
            pcbTable.push_back(PCB{childPid, 0, 0, 0, 0, "Ready"});

            std::cout << "Parsed EXEC command: child PID = " << static_cast<int>(childPid)
                      << ", program name = " << programName << std::endl;

            // Now call execProcess with the created child PID.
            execProcess(childPid, programName, vectorFileName);
        } 
        // Handle other events like CPU, SYSCALL, END_IO.
        else {
            TraceEvent event;
            std::string durationOrID;
            if (std::getline(ss, durationOrID, ',')) {
                std::stringstream durationStream(durationOrID);

                if (command.find("CPU") != std::string::npos) {
                    event.name = "CPU";
                    durationStream >> event.duration;
                    logExecution(event.duration, "CPU Execution");
                } 
                // Handle SYSCALL and END_IO with IDs.
                else if (command.find("SYSCALL") != std::string::npos || command.find("END_IO") != std::string::npos) {
                    event.name = command.substr(0, command.find_first_of(' '));
                    event.ID = std::stoi(command.substr(command.find_last_of(' ') + 1));
                    durationStream >> event.duration;
                    eventHandler(event, vectorFileName);
                }
            } else {
                std::cerr << "Error parsing line: " << line << std::endl;
            }
        }
    }

    inputFile.close();
}



std::string toHex(uint16_t value, int width) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;
    return ss.str();
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
