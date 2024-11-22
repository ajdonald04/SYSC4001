#ifndef PART2_HPP
#define PART2_HPP

#define NUM_TAS 5
#define SEMNUM 5


#include <iostream>
#include <semaphore.h>
#include <fstream>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

// void delay();
// int mark();
void init_semaphores();
void cleanup_semaphores();
int get_next_student(ifstream& file);
void ta_process(int ta_id);

#endif