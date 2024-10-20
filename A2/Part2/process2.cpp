#include <iostream>
using namespace std; 
#include <unistd.h>

int main(void){
    for(int i =0; i<10; i++){

        cout << "I am process 2!" << endl;
        sleep(1);
    }

    return 0;
}

