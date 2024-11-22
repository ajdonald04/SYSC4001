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
    std::vector<Process> waitingProcesses; // Processes waiting for I/O
    std::queue<Process> executionQueue;    // Processes ready to execute

    while (!readyQueue.empty() || !waitingProcesses.empty() || !executionQueue.empty()) {
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

                // Add process to the execution queue
                executionQueue.push(process);
            } else {
                // Memory not available, requeue process for later
                waitingProcesses.push_back(process);
            }
        }

        // Handle processes ready for execution
        if (!executionQueue.empty()) {
            Process process = executionQueue.front();
            executionQueue.pop();

            // Log transition to RUNNING
            logExecutionStatus(executionLog, process.pid, "READY", "RUNNING");

            unsigned int timeSinceLastIO = process.totalCPUTime - process.remainingCPUTime;

            // Simulate execution for one unit of time
            process.remainingCPUTime--;
            currentTime++;

            // Check if I/O is required
            if (process.ioFrequency > 0 && timeSinceLastIO % process.ioFrequency == 0 && process.remainingCPUTime > 0) {
                logExecutionStatus(executionLog, process.pid, "RUNNING", "WAITING");

                // Simulate I/O and add the process to the waiting list
                process.ioDuration = process.ioDuration; // Reset I/O duration
                waitingProcesses.push_back(process);
            } else if (process.remainingCPUTime > 0) {
                // Add the process back to the execution queue if it's not finished
                executionQueue.push(process);
            } else {
                // Process is completed
                logExecutionStatus(executionLog, process.pid, "RUNNING", "TERMINATED");

                // Deallocate memory and log memory state
                deallocateMemory(process);
                logMemoryStatus(memoryLog, memoryPartitions);
            }
        }

        // Process the I/O waiting list
        for (auto it = waitingProcesses.begin(); it != waitingProcesses.end();) {
            Process &waitingProcess = *it;

            // Decrement I/O duration
            waitingProcess.ioDuration--;

            if (waitingProcess.ioDuration <= 0) {
                // Transition process back to READY state
                logExecutionStatus(executionLog, waitingProcess.pid, "WAITING", "READY");

                // Add process back to the execution queue
                executionQueue.push(waitingProcess);

                // Remove process from the waiting list
                it = waitingProcesses.erase(it);
            } else {
                ++it;
            }
        }

        // Advance time if no processes are ready or running
        if (executionQueue.empty() && waitingProcesses.empty() && !readyQueue.empty()) {
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
