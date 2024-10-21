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
    key_t key = ftok("shmfile", 65); // same key as P1
    int shmid = shmget(key, sizeof(int), 0666); 
    
    // checking validity
    if (shmid < 0) {
        cerr << "Process 2: Failed to access shared memory" << endl;
        exit(1);
    }

    // attach the memory segment
    int *shared_var = (int *)shmat(shmid, nullptr, 0);

    // error / validity checking, ensuring memory is attached correctly
    if (shared_var == (void *)-1) {
        cerr << "Process 2: Failed to attach shared memory" << endl;
        exit(1);
    }

    // access the sempahore
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
