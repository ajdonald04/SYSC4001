g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace11.o
./test_trace11.o<<EOF 
../vector_table.txt
test_trace11.txt
execution_test11
EOF 