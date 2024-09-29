#include "utils.h"
#include "iostream"
#include "functions.h"
int main()
{
    srand(time(0));

    std::string VectorTableFile, TraceFile; 

    std::cout << "Enter the vector table file name: "; 
    std::cin >> VectorTableFile; 
    

    std::cout << "Enter the trace file name: ";
    std::cin >> TraceFile; 

    std::vector<uint16_t> isrAddresses = vectorTableHandler(VectorTableFile);
    inputRead(TraceFile);
    
    return 0;
}