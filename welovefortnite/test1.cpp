
#include "functions.h"
int main()
{
    srand(time(0));

    // std::string vectorFileName, TraceFile; 

    // std::cout << "Enter the vector table file name: "; 
    // std::cin >> vectorFileName; 
    

    // std::cout << "Enter the trace file name: ";
    // std::cin >> TraceFile; 

    inputRead("trace1.txt", "vector_table.txt");
    
    return 0;
}
