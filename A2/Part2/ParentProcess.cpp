#include <iostream>
using namespace std; 
#include <unistd.h>
#include <sys/wait.h>

int main(void){

    
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Fork failed!" << std::endl;
        exit(1);
    }

    
    else if(pid == 0){

        while(1){
            for(int j=0; j< 1; j++){
                //waitpid(pids[j], nullptr, 0);
                cout << "I am process " << j + 1 << endl;
                
            }

            int randomNum = rand() %11;
            if (randomNum > 5){
                cout << "High value" <<endl;
            }

            else if(randomNum <=5){
                cout<<"Low value"<<  endl;
            }
            if (randomNum == 9){
                
                execl("/Users/jayven/sysc4001/A2/Part2/process2","process2",nullptr);
                std::cerr << "Failed to launch Process 2!" << std::endl;
            }
            sleep(1);
        }
    }

    else if (pid > 0){
        waitpid(pid, nullptr,0);
        cout << "Process 1 finished" << endl;
    }

   



    return 0;
}