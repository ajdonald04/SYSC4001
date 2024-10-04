g++ interrupts.cpp -I interrupts.hpp -o test_trace19.o 
./test_trace18.o <<EOF 
vector_table.txt
test_trace19.txt
execution19
EOF 