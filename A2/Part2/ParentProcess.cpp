#include <iostream>
using namespace std;
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <time.h>

int main() {

    // generate a key to use as an "id" for the shared memory
    key_t key = ftok("shmfile", 65); 
    
    // Create shared memory segment (store it in shmid)
    // 0666 is a permission mode OR IPC_CREAT means create if it doenst exist.
    int shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT); // shmget, allocate the shared memory, 
    if (shmid < 0) {
        cerr << "Failed to create shared memory" << endl;
        exit(1);
    }

    // link the shared memory segment
    int *shared_var = (int *)shmat(shmid, nullptr, 0);
    if (shared_var == (void *)-1) {
        cerr << "Failed to attach shared memory" << endl;
        exit(1);
    }

    // Initialize the shared variable
    *shared_var = -1;

    pid_t pid = fork();

    if (pid < 0) {
        cerr << "Fork failed!" << endl;
        exit(1);
    } else if (pid == 0) {
        while (*shared_var != 0) {
             // generate random number between 0 and 10
            *shared_var = rand() % 11;
            cout << "Process 1: Generated random number: " << *shared_var << endl;

            if (*shared_var > 5) {
                cout << "Process 1: High value" << endl;
            } else {
                cout << "Process 1: Low value" << endl;
            }

            // execute process 2 if the number is 9
            if (*shared_var == 9) {
                execl("/Users/jayven/sysc4001/A2/Part2/process2", "process2", nullptr);
                cerr << "Failed to launch Process 2!" << endl;
                exit(1);
            }

            sleep(1); 
        }

        // shmdt is used to detach the memory 
        shmdt(shared_var);
        cout << "Process 1 finished" << endl;
    } else {
        // Parent waits for child to finish 
        waitpid(pid, nullptr, 0);

        // cleanup shared memory fter processes are finished
        shmctl(shmid, IPC_RMID, nullptr);
        cout << "Parent cleaned up shared memory" << endl;
    }

    return 0;
}
