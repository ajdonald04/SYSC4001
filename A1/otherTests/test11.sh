g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace9.o
./test_trace9.o<<EOF 
../vector_table.txt
test_trace9.txt
execution_test9
EOF 