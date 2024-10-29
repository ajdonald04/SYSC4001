/*
* SYSC4001 A2 Part2 
*
* Authors: Aj Donald 101259149 & Simon Hagos
*
* Note: Due to the difficulties encountered during the assignment of the teams for A2, 
this assignment had already been completed with my previous partner from A1, (Jayven Larsen, 101260364).
As a result, the solutions of this submission for all parts was completed with him. Having discussed with Professor Wainer,
we have understood that there shouldn't be any risk of plagiarism between our two groups given this mistake. 
*/

#include <iostream>
using namespace std;
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>


// semaphore structures for operations (P and V, same as parent process)
struct sembuf sem_lock = {0, -1, 0}; // P: wait/decrement
struct sembuf sem_unlock = {0, 1, 0}; // V: signal/increment

int main() {
    
    // similarly to the other file, create a key before shared memory allocation occurs (shmget)
    key_t key = ftok("shmfile", 65); // same key as Process 1 
    int shmid = shmget(key, sizeof(int), 0666); 
    
    // error checking
    if (shmid < 0) {
        cerr << "Process 2: Failed to access shared memory" << endl;
        exit(1);
    }

    // attach the segment
    int *shared_var = (int *)shmat(shmid, nullptr, 0);

    // more error checking
    if (shared_var == (void *)-1) {
        cerr << "Process 2: Failed to attach shared memory" << endl;
        exit(1);
    }

    // access sempahore
    int semid = semget(key, 1, 0666);
    if (semid < 0) {
        cerr << "Process 2: Failed to access semaphore" << endl;
        exit(1);
    }

    // loop until the value in shared memory is zero
    while (*shared_var != 0) {
        
        // lock the semaphore before using shared memory (similarly to parent process)
        semop(semid, &sem_lock, 1); 
        cout << "Process 2: Shared random number is: " << *shared_var << endl;
        sleep(1);

        semop(semid, &sem_unlock, 1); // unlock the semaphore after reading shared memory
    }

    // detach shared memory
    shmdt(shared_var);
    cout << "Process 2 finished" << endl;

    return 0;
}
