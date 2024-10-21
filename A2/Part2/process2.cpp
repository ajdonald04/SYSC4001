#include <iostream>
using namespace std;
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>

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

    // loop until the value in shared memory is zero
    while (*shared_var != 0) {
        cout << "Process 2: Shared random number is: " << *shared_var << endl;
        sleep(1);
    }

    // detach shared memory
    shmdt(shared_var);
    cout << "Process 2 finished" << endl;

    return 0;
}
