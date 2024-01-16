#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
using namespace std;

struct threadData{
    string input;
    string rleString;
    vector<int> freq;
};

void* encode(void* void_ptr){
    struct threadData *ptr = (struct threadData*) void_ptr;
    string& input = ptr->input;
    string& rleString = ptr->rleString;
    vector<int>& freq = ptr->freq;

    int count = 1;
    for(int i = 1; i <= input.length(); i++){
        if(i < input.length() && input[i] == input[i - 1]){
            count++;
        }
        else{
            if(count == 1){
                rleString += input[i-1];
            }
            else{
                rleString += input[i - 1];
                rleString += input[i - 1];
                freq.push_back(count);
            }
            count = 1;
        }
    }
    return nullptr;
}



int main(){
    vector<threadData> threadVec;
    vector<pthread_t> threads;
    string input;

    while(getline(cin, input)){
        if(input.empty()){
            break;
        }

        threadData td;
        td.input = input;
        threadVec.push_back(td);
    }

    for(int i = 0; i < threadVec.size(); i++){
        pthread_t thread;
        pthread_create(&thread, NULL, encode, &threadVec[i]);
        threads.push_back(thread);
    }

    for(int i = 0; i < threadVec.size(); i++){
        pthread_join(threads[i], NULL);

        cout << "\nInput string: " << threadVec[i].input << endl;
        cout << "RLE String: " << threadVec[i].rleString << endl;

        cout << "RLE Frequencies:";
        for(int f : threadVec[i].freq){
            cout << " " << f;
        }
        cout << endl;

    }
    return 0;
}