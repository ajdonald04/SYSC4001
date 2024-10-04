g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace3.o
./test_trace3.o<<EOF 
../vector_table.txt
test_trace3.txt
execution_test3
EOF 