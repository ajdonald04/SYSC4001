g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace2.o
./test_trace2.o<<EOF 
../vector_table.txt
test_trace2.txt
execution_test2
EOF 