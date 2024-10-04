g++ interrupts.cpp -I interrupts.hpp -o test_trace18.o 
./test_trace18.o <<EOF 
vector_table.txt
test_trace18.txt
execution18
EOF 