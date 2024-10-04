g++ interrupts.cpp -I interrupts.hpp -o test_trace20.o 
./test_trace18.o <<EOF 
vector_table.txt
test_trace20.txt
execution20
EOF 