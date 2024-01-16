#include <iostream>
#include <pthread.h>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
using namespace std;
//using hw1

struct threadArgs {
    string input;
    string rleString;
    vector<int> freq;
    pthread_mutex_t* bsem;
    pthread_cond_t* printTurn;
    int* turn;
    int threadID;
    sem_t* s;
};

void* encode(void* void_ptr) {
    threadArgs ptr = *((threadArgs*)void_ptr);
    sem_post(ptr.s);
    string& input = ptr.input;
    string& rleString = ptr.rleString;
    vector<int>& freq = ptr.freq;

    int count = 1;
    for (int i = 1; i <= input.length(); i++) {
        if (i < input.length() && input[i] == input[i - 1]) {
            count++;
        } else {
            if (count == 1) {
                //if only one then just add the one char
                rleString += input[i - 1];
            } else {
                //if count more than one than "running" add twice
                rleString += input[i - 1];
                rleString += input[i - 1];
                freq.push_back(count);  //consider frequency
            }
            count = 1;  //reset count
        }
    }

    pthread_mutex_lock(ptr.bsem);
    //cout << "Thread ID =" << ptr.threadID << " " << *ptr.turn << endl;
    while (*ptr.turn != ptr.threadID)
        pthread_cond_wait(ptr.printTurn, ptr.bsem);
    pthread_mutex_unlock(ptr.bsem);

    cout << "\nInput string: " << input << endl;
    cout << "RLE String: " << rleString << endl;

    cout << "RLE Frequencies:";
    for (int f : freq) {
        cout << " " << f;
    }
    cout << endl;

    pthread_mutex_lock(ptr.bsem);
    (*ptr.turn)++;
    pthread_cond_broadcast(ptr.printTurn);
    pthread_mutex_unlock(ptr.bsem);

    return nullptr;
}

int main() {
    string input;
    vector<string> in;
    
    pthread_mutex_t bsem;
    pthread_cond_t printTurn = PTHREAD_COND_INITIALIZER;
    pthread_mutex_init(&bsem, nullptr);
    static int turn = 0;
    
    //read n strings from stdin first
    while (getline(cin, input) && !input.empty()) {
        in.push_back(input);
    }
    
    pthread_t *tid = new pthread_t[in.size()];
    sem_t s;
    sem_init(&s, 0, 0);
    

    threadArgs arg;
    arg.bsem = &bsem;
    arg.printTurn = &printTurn;
    arg.turn = &turn;
    arg.s = &s;

    //then create n threads
    for(int i = 0; i < in.size(); i++){
        arg.input = in[i];
        arg.rleString = "";
        arg.threadID = i;
        pthread_create(&tid[i], nullptr, encode, &arg);
        //sleep(1);  cannot use sleep to syncronize
        sem_wait(&s);
    }

    //pthread_join in seperate loop to take full advantage of multithreading
    for (int i = 0; i < in.size(); i++) {
        pthread_join(tid[i], nullptr);
    }
    
    sem_destroy(&s);
    pthread_mutex_destroy(&bsem);
    delete[] tid;

    return 0;
}