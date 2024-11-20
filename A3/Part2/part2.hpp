#ifndef PART2_HPP
#define PART2_HPP

#define NUMTA 5
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
using namespace std;

// void delay();
// int mark();
int delayAndMark();
void taMarking(int ta_id);

#endif