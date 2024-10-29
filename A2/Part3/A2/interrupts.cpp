/**
 * Main code for SYSC4001 A2
 * Authors: Aj Donald 101259149, Simon Hagos ()
 * October 23rd, 2024
 * 
 * Note: Due to the difficulties encountered during the assignment of the teams for A2, 
 * this assignment had already been completed with my previous partner from A1, (Jayven Larsen, 101260364).
 * As a result, the solutions of this submission for all parts was completed with him. Having discussed with Professor Wainer,
 * we have understood that there shouldn't be any risk of plagiarism between our two groups given this mistake. 
 */

#include "interrupts.hpp"


// global variable to track simulation time.
static uint32_t sim_time = 0;
std::string filename; // Used for logging execution events

// declaring vectors for structures (mem partitions, PCB, and external files).
std::vector<memoryPartition> memoryPartitions;
std::vector<PCB> pcbTable; 
std::vector<ExternalFile> externalFiles; 

// Memory initialization
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

// helper function for loading the programs in external_file.txt
void loadExternalFiles(std::string fileName) {
    std::ifstream inputFile(fileName);
    std::string line;
    externalFiles.clear();

    if (!inputFile) {
        std::cerr << "Error when opening file: " << fileName << std::endl;
        return;
    }

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        ExternalFile file;
        std::string programSizeStr;

        if (std::getline(iss, file.program_name, ',') && std::getline(iss, programSizeStr)) {
            file.program_name.erase(std::remove_if(file.program_name.begin(), file.program_name.end(), ::isspace), file.program_name.end());
            programSizeStr.erase(std::remove_if(programSizeStr.begin(), programSizeStr.end(), ::isspace), programSizeStr.end());
            
            file.size = static_cast<uint8_t>(std::stoi(programSizeStr));
            externalFiles.push_back(file);

            std::cout << "Loaded external file: " << file.program_name << " with size " << static_cast<int>(file.size) << " MB" << std::endl;
        } else {
            std::cerr << "Error parsing line in external_files.txt: " << line << std::endl;
        }
    }
    inputFile.close();
}

// function that generates the log for the system stats
void logSystemStatus() {
    std::ofstream outputFile("system_status.txt", std::ios::app);

    if (outputFile.is_open()) {
        outputFile << "!-----------------------------------------------------------!\n";
        outputFile << "Save Time: " << sim_time << " ms\n";
        outputFile << "+--------------------------------------------+\n";
        outputFile << "| PID |Program Name |Partition Number | size |\n";
        outputFile << "+--------------------------------------------+\n";
        // for each loop to iterate over the table
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
        PCB child = *parentIt; // Copy parent PCB
        child.pid = pcbTable.size(); // Assign a new PID
        child.state = "Ready"; // Set child's state to Ready
        pcbTable.push_back(child); // Add child to PCB table

    } else {
        std::cerr << "Error: Parent PID " << parentPid << " not found." << std::endl;
    }
}


void execProcess(uint8_t childPid, std::string programName, std::string vectorFileName) {
    // Ensure that EXEC is only called by a child process
    auto childIt = std::find_if(pcbTable.begin(), pcbTable.end(),
                                [childPid](const PCB& pcb) {
                                    return pcb.pid == childPid;
                                });
    if (childIt == pcbTable.end() || childIt->state != "Ready") {
        std::cerr << "Error: EXEC can only be called by a child process that is in 'Ready' state." << std::endl;
        return;
    }

    // Trim whitespace from program
    programName.erase(std::remove_if(programName.begin(), programName.end(), ::isspace), programName.end());
    programName.erase(std::remove(programName.begin(), programName.end(), ','), programName.end()); // Remove trailing commas


    // Search for the program in the external files list
    auto programIt = std::find_if(externalFiles.begin(), externalFiles.end(),
                                  [&programName](const ExternalFile& file) {
                                      return file.program_name == programName;
                                  });
    if (programIt == externalFiles.end()) {
        std::cerr << "Error: Program " << programName << " not found in external files." << std::endl;
        return;
    }

    uint8_t programSize = programIt->size;

    int partitionIndex = BestFitPartition(programSize);
    if (partitionIndex == -1) {
        std::cerr << "Error: No suitable partition found for program " << programName << ".\n";
        return;
    }

    // Update mem partition and PCB
    memoryPartitions[partitionIndex].code = programName; 
    childIt->partition_num = memoryPartitions[partitionIndex].num; // Update partition num in PCB
    childIt->state = "Running"; // process can be considered running now

    logExecution(1, "Switch to Kernel Mode");
    logExecution(rand() % 3 + 1, "Save Context");
    logExecution(1, "Find vector #3 in memory position 0x0006");
    logExecution(1, "Load address 0X042B into the PC");
    logExecution(rand() % 10 + 2, "EXEC: load " + programName + " of size " + std::to_string(programSize) + "MB");
    logExecution(rand() % 10 + 2, "Found partition " + std::to_string(memoryPartitions[partitionIndex].num) + 
                 " with " + std::to_string(memoryPartitions[partitionIndex].size) + "MB of space");
    logExecution(rand() % 10 + 2, "Partition " + std::to_string(memoryPartitions[partitionIndex].num) + 
                 " marked as occupied");
    logExecution(rand() % 10 + 2, "Updating PCB with new information");

    // Log the system status after EXEC
    logSystemStatus();

    // Read the program trace file
    std::string programTraceFile = programName + ".txt";
    std::cout << "Switching to program trace: " << programTraceFile << std::endl;

    // Call the function to read events from the program's trace file
    inputRead(programTraceFile, vectorFileName, filename);
}


void logExecution(uint32_t duration, const std::string eventName) {
    std::ofstream outputFile(filename, std::ios::app); // Open in append mode

    if (outputFile.is_open()) {
        outputFile << sim_time << ", " << duration << ", " << eventName << std::endl;
        sim_time += duration;
        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open execution.txt file for logging" << std::endl;
    }
}

void eventHandler(TraceEvent event, std::string fileName) {
    std::vector<uint16_t> isrAddresses = vectorTableHandler(fileName);
    int vectorTableSize = isrAddresses.size();

    if (vectorTableSize == 0) {
        std::cerr << "Error: Vector table is empty!" << std::endl;
        return;
    }

    if (event.name == "CPU") {
        logExecution(event.duration, "CPU Execution");
    } else if (event.name == "FORK") {
        uint8_t parentPid = event.ID; // Assuming ID is used as parent PID
    
        logExecution(1, "Switch to Kernel Mode");
        logExecution(2, "Save Context");
        logExecution(1, "Find vector #2 in memory position 0x0004"); // Assuming vector 2 for FORK
        logExecution(1, "Load address 0X0695 into the PC"); // Example address for FORK
        logExecution(4, "FORK: copy parent PCB to child PCB");
        logExecution(1, "Scheduler called"); 
        logExecution(1, "IRET");

        // Log the system status after FORK
        logSystemStatus();
        forkProcess(parentPid); // Process the fork
    }

    if (event.ID > 0 && event.ID <= vectorTableSize) {
        uint16_t ISRAddress = isrAddresses[event.ID - 1];
        uint16_t memoryPosition = event.ID * 2; 

        // handling system calls 
        if (event.name == "SYSCALL") {
            logExecution(1, "Switch to Kernel Mode");
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #" + std::to_string(event.ID) + 
                            " in memory position 0x" + toHex(memoryPosition, 4));
            logExecution(1, "Load address 0x" + toHex(ISRAddress, 4) + " into PC");
            logExecution(event.duration, "SYSCALL: run the ISR");
            logExecution(1, "IRET");
        } 
        // handling io events
        else if (event.name == "END_IO") {
            logExecution(1, "Check the priority of the Interrupt");
            logExecution(1, "Check if the interrupt is masked");
            logExecution(1, "Switch to Kernel Mode");
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #" + std::to_string(event.ID) + 
                            " in memory position 0x" + toHex(memoryPosition, 4));
            logExecution(1, "Load address 0x" + toHex(ISRAddress, 4));
            logExecution(event.duration, "I/O Completed");
            logExecution(1, "IRET");
        }
    }
}

void inputRead(std::string traceFileName, std::string vectorFileName, std::string outputFileName) {
    std::ifstream inputFile(traceFileName);
    filename = outputFileName;

    if (!inputFile) {
        std::cerr << "Error when opening file: " << traceFileName << std::endl;
        return;
    }

    std::string line;

    // read line of the file and hten process the event from the trace
    while (std::getline(inputFile, line)) {
        TraceEvent event;
        std::stringstream ss(line);
        std::string activity;
        std::string durationOrID;

        // get the event and duration or ID
        if (std::getline(ss, activity, ',') && std::getline(ss, durationOrID, ',')) {
            std::stringstream durationStream(durationOrID);

            if (activity.find("CPU") != std::string::npos) {
                event.name = "CPU";
                durationStream >> event.duration;
                event.ID = 0; // No ID for CPU events
            } 
            else if (activity.find("FORK") != std::string::npos) {
                event.name = "FORK";
                try {
                    event.ID = std::stoi(durationOrID);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument for FORK: " << durationOrID << std::endl;
                    continue; 
                }
            } 
            else if (activity.find("EXEC") != std::string::npos) {
                event.name = "EXEC";
                std::string programName = activity.substr(activity.find(' ') + 1);
                event.ID = pcbTable.size(); // Use current size for new child PID
                execProcess(event.ID, programName, vectorFileName); 
                continue;
            } 
            else if (activity.find("SYSCALL") != std::string::npos || activity.find("END_IO") != std::string::npos) {
                event.name = activity.substr(0, activity.find_first_of(' '));  
                try {
                    event.ID = std::stoi(activity.substr(activity.find_last_of(' ') + 1));  
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument for SYSCALL/END_IO: " << durationOrID << std::endl;
                    continue; 
                }
                durationStream >> event.duration;
            }

            // eventHandler for the given event we're trying to process
            eventHandler(event, vectorFileName);
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    inputFile.close();
    std::cout << "Finished processing CPU, SYSCALL, FORK, and EXEC events." << std::endl;
}

// function to convert the addresses to hex 
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

int main() {
    std::string vectorFileName, traceFileName, outputFileName, externalFilesName;

    std::cout << "Enter the vector table file name: ";
    std::cin >> vectorFileName;

    std::cout << "Enter the trace file name: ";
    std::cin >> traceFileName;

    std::cout << "Enter the output file name (without extension): ";
    std::cin >> outputFileName;

    std::cout << "Enter the external files list name: ";
    std::cin >> externalFilesName;

    initMemory();
    loadExternalFiles(externalFilesName);
    filename = outputFileName + ".txt"; // file for logging
    inputRead(traceFileName, vectorFileName, filename); 

    std::cout << "Simulation completed. Check '" << outputFileName << ".txt' and 'system_status.txt' for details." << std::endl;

    return 0;
}
