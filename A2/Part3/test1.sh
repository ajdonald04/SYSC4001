g++ -std=c++11 interrupts.cpp -I interrupts.hpp -o test1.o 
./test1.o<<EOF 
vector_table.txt
trace1.txt
execution
external_files.txt
EOF 
