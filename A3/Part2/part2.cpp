//Ta's have two tasks
//  1) pick a student from db
//  2)save the mark for that sutdent in an individual file 
// Each TA has their own file

#include "part2.hpp"




//2 a)
// 5 semaphore, 1 for each TA
// TA accesses db
// picks up the next number & saves to local var
// 1-4 second delay (random num)
//releases semaphore

//once TA has student number, assign random num 1-10 for mark
// once 9999 reached, loops through again, for 3 times


//sem_post() is the unlock of a semaphore
//sem_wait locks the semaphore

vector<string> studentIds;
vector<sem_t*> SemList;

int main (void){

    srand(time(0));



    //Create the TA semaphores
    sem_t* Sem1;
    char Ta1_name[] = "Sem1";
    Sem1 = sem_open(Ta1_name, O_CREAT);
    SemList.push_back(Sem1);

    sem_t* Sem2;
    char Sem2_name[] = "Sem2";
    Sem2 = sem_open(Sem2_name, O_CREAT);
    SemList.push_back(Sem2);

    sem_t* Sem3;
    char Sem3_name[] = "Sem3";
    Sem3 = sem_open(Sem3_name, O_CREAT);
    SemList.push_back(Sem3);

    sem_t* Sem4;
    char Sem4_name[] = "Sem4";
    Sem4 = sem_open(Sem4_name, O_CREAT);
    SemList.push_back(Sem4);

    // sem_t* Sem5;
    // char Sem5_name[] = "Sem5";
    // Sem5 = sem_open(Ta1_name, O_CREAT);
    // SemList.push_back(Sem5);


    

    ifstream file("database.txt");

    if (!file.is_open()) {
        cerr << "Error opening the file!";
        return 1;
    }

    // String variable to store the student ID
    string student_ID;
    while (getline(file, student_ID)){
        studentIds.push_back(student_ID);
    }   
    file.close();

    //creating the ta processes

    for (int i = 1; i < NUMTA+1; ++i){
        pid_t pid = fork();
        if (pid == 0){
            taMarking(i);
        }

    }

    for (int i = 0; i< NUMTA; i++){
        wait(nullptr);
    }


    return 0;
}


void taMarking(int ta_id){
    

    //create the output files for the TA's to write to

    ofstream ta_file("TA"+ to_string(ta_id)+ ".txt");
    if (!ta_file.is_open()){
        cerr << "Error openning file" << ta_id << endl;
        exit(1);
    }

    for (int i = 0; i<3;){

        for(const string &student : studentIds){
            if (student == "9999"){i++;} //if the student ID is 9999 then increment the for big for loop


            // Lock the semaphores of taJ and taJ +1
            sem_wait(SemList[ta_id]);
            sem_wait(SemList[ta_id+1]);

            //access the database
            cout<< "TA " << ta_id << "is accessing the databse for student " << student << endl;
            delay();

            cout << "TA" << ta_id << "is marking student" << student << endl;
            delay();
            int markScore = mark();
            ta_file << "Student" << student << ": " << markScore << "Points" << endl;


        } 

    }

    ta_file.close();

    cout<< "TA" << ta_id <<"Finished marking" << endl;




}

void delay(){
    srand(time(0));
    int delayNum = 1+ (rand() % 10);
    sleep(delayNum);
    
}

int mark(){
    srand(time(0));
    int markValue = rand() % 10;
    return markValue;
}



