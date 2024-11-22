/** 
 * SYSC4001 Assignment 3 - Part 1 
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364
 */


#include "interrupts.hpp" 


// initialize the a vector with the memory partitions, -1 since they're unoccupied by default. 
std::vector<MemoryPartition> memoryPartitions = {{40, -1}, {25, -1}, {15, -1}, {10, -1}, {8, -1}, {2, -1}};
std::queue<Process> readyQueue; 
uint16_t currentTime = 0; 


// 
void logMemoryStatus(std::ofstream &memoryLog, const std::vector<MemoryPartition> &partitions) {
    static bool headerWritten = false;
    if (!headerWritten) {
        memoryLog << "+------------------------------------------------------------------------------------------+\n";
        memoryLog << "| Time of Event | Memory Used |      Partitions State     | Total Free Memory | Usable Free Memory |\n";
        memoryLog << "+------------------------------------------------------------------------------------------+\n";
        headerWritten = true;
    }

    unsigned int memoryUsed = 0;
    unsigned int totalFree = 0;
    unsigned int usableFree = 0;
    std::ostringstream partitionState;

    for (const auto &partition : partitions) {
        if (partition.occupiedBy != -1) {
            memoryUsed += partition.size;
        } else {
            totalFree += partition.size;
            usableFree += partition.size;
        }
        partitionState << (partition.occupiedBy == -1 ? "-1" : std::to_string(partition.occupiedBy)) << ", ";
    }

    // Format the output as a table row
    memoryLog << "| " << std::setw(12) << currentTime << " | " << std::setw(11) << memoryUsed << " | "
              << std::setw(25) << partitionState.str() << " | " << std::setw(17) << totalFree << " | "
              << std::setw(17) << usableFree << " |\n";
}


// Function to log execution status
void logExecutionStatus(std::ofstream &executionLog, int pid, const std::string &oldState, const std::string &newState) {
    static bool headerWritten = false;
    if (!headerWritten) {
        executionLog << "+------------------------------------------------+\n";
        executionLog << "| Time of Transition | PID | Old State | New State |\n";
        executionLog << "+------------------------------------------------+\n";
        headerWritten = true;
    }

    // Format the output as a table row
    executionLog << "| " << std::setw(18) << currentTime << " | " << std::setw(3) << pid << " | "
                 << std::setw(10) << oldState << " | " << std::setw(9) << newState << " |\n";
}

int allocateMemory(Process &process) {
    int bestFitIndex = -1;
    unsigned int smallestSize = UINT_MAX;

    for (int i = 0; i < memoryPartitions.size(); i++) {
        if (memoryPartitions[i].occupiedBy == -1 && memoryPartitions[i].size >= process.memorySize) {
            if (memoryPartitions[i].size < smallestSize) {
                smallestSize = memoryPartitions[i].size;
                bestFitIndex = i;
            }
        }
    }

    if (bestFitIndex != -1) {
        memoryPartitions[bestFitIndex].occupiedBy = process.pid;
        process.memoryPartition = bestFitIndex;
    }

    return bestFitIndex;
}


// Function to deallocate memory
void deallocateMemory(Process &process) {
    if (process.memoryPartition != -1) {
        memoryPartitions[process.memoryPartition].occupiedBy = -1;
        process.memoryPartition = -1;
    }
}

void runScheduler(std::queue<Process> &readyQueue, std::ofstream &executionLog, std::ofstream &memoryLog) {
    std::vector<Process> waitingProcesses; // Processes waiting for memory or I/O

    while (!readyQueue.empty() || !waitingProcesses.empty()) {
        // Allocate memory for all processes arriving at the current time
        while (!readyQueue.empty() && readyQueue.front().arrivalTime <= currentTime) {
            Process process = readyQueue.front();
            readyQueue.pop();

            // Try to allocate memory
            int partitionIndex = allocateMemory(process);
            if (partitionIndex != -1) {
                // Memory allocated, log allocation
                logMemoryStatus(memoryLog, memoryPartitions);

                // Transition process to READY state and log
                logExecutionStatus(executionLog, process.pid, "NEW", "READY");

                // Add process to the execution-ready list
                waitingProcesses.push_back(process);
            } else {
                // Memory not available, requeue process for later
                waitingProcesses.push_back(process);
            }
        }

        // Handle processes ready for execution
        if (!waitingProcesses.empty()) {
            Process &process = waitingProcesses.front();

            // Start execution if process is READY
            logExecutionStatus(executionLog, process.pid, "READY", "RUNNING");

            unsigned int timeSinceLastIO = 0;
            while (process.remainingCPUTime > 0) {
                timeSinceLastIO++;

                // Handle I/O interruptions
                if (timeSinceLastIO == process.ioFrequency && process.remainingCPUTime > 0) {
                    logExecutionStatus(executionLog, process.pid, "RUNNING", "WAITING");

                    // Simulate I/O duration
                    currentTime += process.ioDuration;

                    logExecutionStatus(executionLog, process.pid, "WAITING", "RUNNING");

                    // Reset I/O timer
                    timeSinceLastIO = 0;
                }

                // Execute one time unit
                process.remainingCPUTime--;
                currentTime++;
            }

            // Transition to TERMINATED
            logExecutionStatus(executionLog, process.pid, "RUNNING", "TERMINATED");

            // Deallocate memory
            deallocateMemory(process);
            logMemoryStatus(memoryLog, memoryPartitions);

            // Remove process from the list
            waitingProcesses.erase(waitingProcesses.begin());
        } else if (!readyQueue.empty()) {
            // Advance time to the next process arrival if no processes are running
            currentTime = std::max(currentTime, readyQueue.front().arrivalTime);
        }
    }
}





std::queue<Process> readInputData(const std::string &filename) {
    std::ifstream inputFile(filename);
    std::queue<Process> processes;

    if (!inputFile) {
        std::cerr << "Error opening input file: " << filename << std::endl;
        return processes;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream ss(line);
        Process process;
        std::string token;

        // Read each attribute separated by commas
        if (std::getline(ss, token, ',')) process.pid = std::stoi(token);
        if (std::getline(ss, token, ',')) process.memorySize = std::stoi(token);
        if (std::getline(ss, token, ',')) process.arrivalTime = std::stoi(token);
        if (std::getline(ss, token, ',')) process.totalCPUTime = std::stoi(token);
        if (std::getline(ss, token, ',')) process.ioFrequency = std::stoi(token);
        if (std::getline(ss, token, ',')) process.ioDuration = std::stoi(token);

        // Initialize remaining attributes
        process.remainingCPUTime = process.totalCPUTime;
        process.state = 0;                // NEW state
        process.memoryPartition = -1;    // Not yet allocated

        // Add the process to the queue
        processes.push(process);

        // Debugging: Print each parsed process
        std::cout << "Added Process: PID=" << process.pid 
                  << ", MemorySize=" << process.memorySize
                  << ", ArrivalTime=" << process.arrivalTime 
                  << ", TotalCPUTime=" << process.totalCPUTime
                  << ", IOFrequency=" << process.ioFrequency 
                  << ", IODuration=" << process.ioDuration << std::endl;
    }

    inputFile.close();
    return processes;
}

// Main function
int main() {
    std::string inputFile = "input_data2.txt";
    std::string executionFile = "execution2.txt";
    std::string memoryFile = "memory_status2.txt";

    // Open output files
    std::ofstream executionLog(executionFile);
    std::ofstream memoryLog(memoryFile);

    if (!executionLog || !memoryLog) {
        std::cerr << "Error opening output files." << std::endl;
        return 1;
    }

    // Read input data
    std::queue<Process> processes = readInputData(inputFile);

    // initial state of the memory.
    logMemoryStatus(memoryLog, memoryPartitions); 

    // Run the scheduler
    runScheduler(processes, executionLog, memoryLog);

    // Close logs
    executionLog.close();
    memoryLog.close();

    return 0;
}
