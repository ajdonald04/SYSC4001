/** 
 * SYSC4001 Assignment 3 - Part 1 
 * 
 * Authors: Aj Donald 101259149, Jayven Larsen 101260364
 * hpp file
 */

#include <iostream> 
#include <fstream> 
#include <sstream> 
#include <vector> 
#include <queue> 
#include <iomanip> 
#include <string> 
#include <cstdint>

// memory partition structure (probably move this to header)
struct MemoryPartition {
    uint16_t size; 
    int occupiedBy; // used for PID of the process occupying the partition

    MemoryPartition(unsigned int size, int occupiedBy) : size(size), occupiedBy(occupiedBy) {}

}; 

struct Process { 
    uint16_t pid;
    uint16_t memorySize; 
    uint16_t arrivalTime;
    uint16_t totalCPUTime; 
    uint16_t ioFrequency; 
    uint16_t ioDuration; 
    uint16_t remainingCPUTime; 
    uint8_t state; // 0 = new, 1 = ready, 2 = running, 3 = waiting, 4 = terminated
    int8_t memoryPartition; 
    uint8_t priority; 

};


struct PriorityComparator {
    bool operator()(const Process &a, const Process &b) const {
        return (a.priority > b.priority) || 
               (a.priority == b.priority && a.arrivalTime > b.arrivalTime);
    }
};
void logMemoryStatus(std::ofstream &memoryLog, const std::vector<MemoryPartition> &partitions);
