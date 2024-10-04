g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace15.o
./test_trace15.o<<EOF 
../vector_table.txt
test_trace15.txt
execution_test15
EOF 