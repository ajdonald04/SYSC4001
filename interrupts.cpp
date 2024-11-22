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
    std::vector<std::pair<Process, int>> waitingProcesses; // Pair: Process + I/O End Time
    std::queue<Process> executionQueue;                   // Processes ready to execute

    while (!readyQueue.empty() || !waitingProcesses.empty() || !executionQueue.empty()) {
        // Step 1: Allocate memory for newly arrived processes
        while (!readyQueue.empty() && readyQueue.front().arrivalTime <= currentTime) {
            Process process = readyQueue.front();
            readyQueue.pop();

            // Try to allocate memory
            int partitionIndex = allocateMemory(process);
            if (partitionIndex != -1) {
                logMemoryStatus(memoryLog, memoryPartitions); // Log memory allocation
                logExecutionStatus(executionLog, process.pid, "NEW", "READY"); // Log state transition
                executionQueue.push(process); // Add process to execution queue
            } else {
                // Requeue for memory allocation later
                readyQueue.push(process);
                break; // Exit allocation loop if no memory is available
            }
        }

        // Step 2: Handle execution (CPU utilization)
        if (!executionQueue.empty()) {
            Process process = executionQueue.front();
            executionQueue.pop();

            // Log `READY -> RUNNING` transition if needed
            if (process.state != 1 /* RUNNING */) {
                logExecutionStatus(executionLog, process.pid, "READY", "RUNNING");
                process.state = 1; // Update state to RUNNING
            }

            process.remainingCPUTime--;
            currentTime++;

            // Handle I/O
            if (process.ioFrequency > 0 &&
                (process.totalCPUTime - process.remainingCPUTime) % process.ioFrequency == 0 &&
                process.remainingCPUTime > 0) {
                logExecutionStatus(executionLog, process.pid, "RUNNING", "WAITING");

                // Move the process to waiting state
                int ioEndTime = static_cast<int>(currentTime) + process.ioDuration;
                waitingProcesses.push_back({process, ioEndTime});
            } else if (process.remainingCPUTime > 0) {
                executionQueue.push(process); // Add back to execution queue if not completed
            } else {
                logExecutionStatus(executionLog, process.pid, "RUNNING", "TERMINATED");
                deallocateMemory(process); // Free memory
                logMemoryStatus(memoryLog, memoryPartitions); // Log memory deallocation
            }
        } else if (!waitingProcesses.empty()) {
            // If no process is running, advance time to the next I/O completion
            int nextIOCompletionTime = waitingProcesses.front().second;
            for (const auto &pair : waitingProcesses) {
                nextIOCompletionTime = std::min(nextIOCompletionTime, pair.second);
            }
            currentTime = static_cast<uint16_t>(std::max(static_cast<int>(currentTime), nextIOCompletionTime));
        } else if (!readyQueue.empty()) {
            // If no processes are ready or waiting, skip to the next process arrival
            currentTime = static_cast<uint16_t>(std::max(static_cast<int>(currentTime), static_cast<int>(readyQueue.front().arrivalTime)));
        }

        // Step 3: Handle waiting processes (I/O completion)
        for (auto it = waitingProcesses.begin(); it != waitingProcesses.end();) {
            auto &waitingProcess = it->first;
            int ioEndTime = it->second;

            if (currentTime >= ioEndTime) {
                // I/O has completed, transition back to READY state
                if (waitingProcess.state != 3 /* READY */) {
                    logExecutionStatus(executionLog, waitingProcess.pid, "WAITING", "READY");
                    waitingProcess.state = 3; // Update state to READY
                }
                executionQueue.push(waitingProcess); // Add back to execution queue
                it = waitingProcesses.erase(it);    // Remove from waiting list
            } else {
                ++it;
            }
        }
    }
}

#include <limits> // For std::numeric_limits
void runPriorityScheduler(std::queue<Process> &readyQueue, std::ofstream &executionLog, std::ofstream &memoryLog) {
    std::vector<std::pair<Process, int>> waitingProcesses; // Pair: Process + I/O End Time
    std::priority_queue<Process, std::vector<Process>, PriorityComparator> executionQueue; // Priority queue for scheduling

    while (!readyQueue.empty() || !waitingProcesses.empty() || !executionQueue.empty()) {
        // Step 1: Allocate memory for newly arrived processes
        while (!readyQueue.empty() && readyQueue.front().arrivalTime <= currentTime) {
            Process process = readyQueue.front();
            readyQueue.pop();

            // Try to allocate memory
            int partitionIndex = allocateMemory(process);
            if (partitionIndex != -1) {
                logMemoryStatus(memoryLog, memoryPartitions); // Log memory allocation
                logExecutionStatus(executionLog, process.pid, "NEW", "READY"); // Log state transition
                executionQueue.push(process); // Add process to priority queue
            } else {
                readyQueue.push(process); // Requeue if memory not available
                break; // Exit allocation loop if no memory is available
            }
        }

        // Step 2: Handle execution (CPU utilization)
        if (!executionQueue.empty()) {
            Process process = executionQueue.top();
            executionQueue.pop();

            // Log `READY -> RUNNING` transition if needed
            if (process.state != 1 /* RUNNING */) {
                logExecutionStatus(executionLog, process.pid, "READY", "RUNNING");
                process.state = 1; // Update state to RUNNING
            }

            process.remainingCPUTime--;
            currentTime++;

            // Handle I/O
            if (process.ioFrequency > 0 &&
                (process.totalCPUTime - process.remainingCPUTime) % process.ioFrequency == 0 &&
                process.remainingCPUTime > 0) {
                logExecutionStatus(executionLog, process.pid, "RUNNING", "WAITING");

                // Move the process to waiting state
                int ioEndTime = static_cast<int>(currentTime) + process.ioDuration;
                waitingProcesses.push_back({process, ioEndTime});
            } else if (process.remainingCPUTime > 0) {
                executionQueue.push(process); // Add back to execution queue if not completed
            } else {
                logExecutionStatus(executionLog, process.pid, "RUNNING", "TERMINATED");
                deallocateMemory(process); // Free memory
                logMemoryStatus(memoryLog, memoryPartitions); // Log memory deallocation
            }
        } else if (!waitingProcesses.empty()) {
            // If no process is running, advance time to the next I/O completion
            int nextIOCompletionTime = waitingProcesses.front().second;
            for (const auto &pair : waitingProcesses) {
                nextIOCompletionTime = std::min(nextIOCompletionTime, pair.second);
            }
            currentTime = static_cast<uint16_t>(std::max(static_cast<int>(currentTime), nextIOCompletionTime));
        } else if (!readyQueue.empty()) {
            // If no processes are ready or waiting, skip to the next process arrival
            currentTime = static_cast<uint16_t>(std::max(static_cast<int>(currentTime), static_cast<int>(readyQueue.front().arrivalTime)));
        }

        // Step 3: Handle waiting processes (I/O completion)
        for (auto it = waitingProcesses.begin(); it != waitingProcesses.end();) {
            auto &waitingProcess = it->first;
            int ioEndTime = it->second;

            if (currentTime >= ioEndTime) {
                // I/O has completed, transition back to READY state
                if (waitingProcess.state != 3 /* READY */) {
                    logExecutionStatus(executionLog, waitingProcess.pid, "WAITING", "READY");
                    waitingProcess.state = 3; // Update state to READY
                }
                executionQueue.push(waitingProcess); // Add back to execution queue
                it = waitingProcesses.erase(it);    // Remove from waiting list
            } else {
                ++it;
            }
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
    std::string executionFile = "execution_prio.txt";
    std::string memoryFile = "memory_stat_prio.txt";

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

    // Run the scheduler (FCFS)
    //runScheduler(processes, executionLog, memoryLog);

    // run the priority scheduler: 
    runPriorityScheduler(processes, executionLog, memoryLog);
    // Close logs
    executionLog.close();
    memoryLog.close();

    return 0;
}
