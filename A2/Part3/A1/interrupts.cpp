#include "interrupts.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <iomanip>
#include <algorithm>
#include <cstdint>

// Variable to track simulation time.
static uint32_t sim_time = 0;
std::string filename;

// Declaring vectors for structures (partitions, PCB, and external files).
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

        logExecution(4, "FORK: copy parent PCB to child PCB");
        logExecution(rand() % 10 + 6, "Scheduler called");
        logExecution(1, "IRET");
    }
}

void execProcess(uint8_t childPid, std::string programName, std::string vectorFileName) {
    programName.erase(std::remove_if(programName.begin(), programName.end(), ::isspace), programName.end());

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
    int partitionIndex = BestFitPartition(programSize);
    if (partitionIndex == -1) {
        std::cerr << "Error: No suitable partition found for program " << programName << ".\n";
        return;
    }

    memoryPartitions[partitionIndex].code = programName;
    childIt->partition_num = memoryPartitions[partitionIndex].num;
    childIt->state = "Running";

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

    scheduler();
    logSystemStatus();
    logExecution(1, "IRET");

    std::string programTraceFile = programName + ".txt";
    inputRead(programTraceFile, vectorFileName, filename);
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
    }

    if (event.ID > 0 && event.ID <= vectorTableSize) {
        uint16_t ISRAddress = isrAddresses[event.ID - 1];
        uint16_t memoryPosition = event.ID * 2; 

        if (event.name == "SYSCALL") {
            logExecution(1, "Switch to Kernel Mode");
            logExecution(rand() % 3 + 1, "Save Context");
            logExecution(1, "Find vector #" + std::to_string(event.ID) + 
                            " in memory position 0x" + toHex(memoryPosition, 4));
            logExecution(1, "Load address 0x" + toHex(ISRAddress, 4) + " into PC");
            logExecution(event.duration, "SYSCALL: run the ISR");
            logExecution(1, "IRET");
        } 
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

void logExecution(uint32_t duration, const std::string eventName) {
    std::ofstream outputFile(filename, std::ios::app);

    if (outputFile.is_open()) {
        outputFile << sim_time << ", " << duration << ", " << eventName << std::endl;
        sim_time += duration;
        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open execution.txt file for logging" << std::endl;
    }
}

void inputRead(std::string traceFileName, std::string vectorFileName, std::string outputFileName) {
    std::queue<std::string> fileQueue;
    fileQueue.push(traceFileName);

    filename = outputFileName + ".txt";

    while (!fileQueue.empty()) {
        std::string currentFile = fileQueue.front();
        fileQueue.pop();
        std::ifstream inputFile(currentFile);

        if (!inputFile) {
            std::cerr << "Error when opening file: " << currentFile << std::endl;
            continue;
        }

        std::string line;
        while (std::getline(inputFile, line)) {
            std::stringstream ss(line);
            std::string command;
            ss >> command;

            if (command == "FORK") {
                uint8_t parentPid;
                ss.ignore(1, ',');
                ss >> parentPid;

                if (ss.fail()) {
                    std::cerr << "Error parsing FORK command: " << line << std::endl;
                    continue;
                }

                forkProcess(parentPid);
                logSystemStatus();
                logExecution(1, "IRET");
            } else if (command == "EXEC") {
                std::string programName;
                ss.ignore(1, ',');
                ss >> programName;

                programName.erase(std::remove_if(programName.begin(), programName.end(), ::isspace), programName.end());
                programName.erase(std::remove(programName.begin(), programName.end(), ','), programName.end());

                if (ss.fail()) {
                    std::cerr << "Error parsing EXEC command: " << line << std::endl;
                    continue;
                }

                uint8_t childPid = pcbTable.size();
                pcbTable.push_back(PCB{childPid, 0, 0, 0, 0, "Ready"});

                execProcess(childPid, programName, vectorFileName);
                fileQueue.push(programName + ".txt");
            } else {
                TraceEvent event;
                std::string durationOrID;
                if (std::getline(ss, durationOrID, ',')) {
                    std::stringstream durationStream(durationOrID);

                    if (command == "CPU") {
                        event.name = "CPU";
                        durationStream >> event.duration;
                        logExecution(event.duration, "CPU Execution");
                    } else if (command.find("SYSCALL") != std::string::npos) {
                        event.name = "SYSCALL";
                        ss >> event.ID;
                        durationStream >> event.duration;
                        eventHandler(event, vectorFileName);
                    } else if (command.find("END_IO") != std::string::npos) {
                        event.name = "END_IO";
                        ss >> event.ID;
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

    std::cout << "Finished processing all traces." << std::endl;
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
    inputRead(traceFileName, vectorFileName, outputFileName);

    std::cout << "Simulation completed. Check 'execution.txt' and 'system_status.txt' for details." << std::endl;

    return 0;
}
