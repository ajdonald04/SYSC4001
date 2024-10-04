g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace7.o
./test_trace7.o<<EOF 
../vector_table.txt
test_trace7.txt
execution_test7
EOF 