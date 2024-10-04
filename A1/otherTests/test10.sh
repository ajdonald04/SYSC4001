g++ ../interrupts.cpp -I ../interrupts.hpp -o test_trace8.o
./test_trace8.o<<EOF 
../vector_table.txt
test_trace8.txt
execution_test8
EOF 