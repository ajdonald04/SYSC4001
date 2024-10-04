g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace16.o 
./test_trace16.o<<EOF 
../vector_table.txt
test_trace16.txt
execution_test16
EOF 