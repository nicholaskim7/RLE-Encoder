#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <bits/stdc++.h>
using namespace std;

struct threadParams{
    string subMsg;
    int numToDecode;
    int finalstrsize;
    char symbol;
    struct sockaddr_in server_addr;
    struct hostent *serv;
    int portnumber;
};

void *sendServerThreads(void *params){
    threadParams *args = (threadParams *)params;

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char *buffer = new char[args->numToDecode+2];

    portno = args->portnumber;
    server = args->serv;
    serv_addr = args->server_addr;

    //make a new socket per thread
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cout << "Error opening socket" << endl;
        exit(0);
    }

    //connecting to server
    bzero((char *)&serv_addr, sizeof(serv_addr));  //zero out a block of memory
    serv_addr.sin_family = AF_INET;
    //copies the data from the source memory location to the destination memory location
    //three parameters: the source pointer, the destination pointer, and the length in bytes to be copied.
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cout<<"Error connecting to server\n";
        exit(0);
    }

    //set a range of bytes in the buffer to zero. starting from the beginning
    //of the buffer and extends for args->numToDecode + 2 bytes.
    bzero(buffer, args->numToDecode + 2);

    //copy whats in the submsg into buffer
    args->subMsg.copy(buffer, sizeof(args->subMsg.length()));
    //write to server send the binary code of a symbol
    n = write(sockfd, buffer, args->numToDecode + 1);
    if(n < 0){
        cout << "Error writing to socket" << endl;
        exit(0);
    }
    //clear the buffer
    bzero(buffer, args->numToDecode + 2);

    //read from server recieve the decoded char back
    n = read(sockfd, buffer, args->numToDecode + 1);
    if(n < 0){
        cout << "Error reading from socket" << endl;
    }

    //set the buffer which should be the decoded char from the server
    args->symbol = buffer[0];
    close(sockfd);
    return nullptr;
}



int main(int argc, char const *argv[]){
    int sockfd, portno, n, numToDecode, finalstrsize, numthreads, temp;
    string encoded, decodedmssg;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    int count = 0;
    vector<string> codes;
    vector<string> poss;

    if(argc < 3){
        cout << "Too little arguments for client.cpp" << endl;
        return -1;
    }

    //create socket
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0){
        cout << "Error opening socket" << endl;
        return -1;
    }

    //get server hostname
    server = gethostbyname(argv[1]);
    if(server == nullptr){
        cout << "Error, no such host" << endl;
        exit(0);
    }


    while(getline(cin, encoded)){
        string code, pos;
        
        code = encoded.substr(0, encoded.find(" "));
        pos = encoded.substr(encoded.find(" ")+1);

        codes.push_back(code);
        poss.push_back(pos);
    }

    //count how many positions the symbol will have
    for(int j = 0; j < poss.size(); j++){
        stringstream ss(poss[j]);
        string inte;
        while(ss >> inte){
            count++;
        }
    }

    //resize final decoded str so we will never be out of bounds
    decodedmssg.resize(count);

    //creating threads
    numthreads = poss.size();
    pthread_t *threads = new pthread_t[numthreads];
    //sending vars needed in all threads
    threadParams *var = new threadParams[numthreads];

    for(int i = 0; i < codes.size(); i++){//number of symbols to decode / lines in compressed.txt
    //set thread variables
        var[i].numToDecode = codes.size();
        var[i].subMsg = codes[i];
        var[i].portnumber = portno;
        var[i].serv = server;
        var[i].server_addr = serv_addr;
    }

    for(int i = 0; i < numthreads; i++){
                    //(the address of pthread_t object, default attributes, thread function, address holding arguments for the thread)
        pthread_create(&threads[i], NULL, sendServerThreads, &var[i]);
    }
    
    //must have a seperate for loop for calling pthread join to take full advantage of multithreading
    for(int p = 0; p < numthreads; p++){
        //allows the main thread to wait for the completion of each individual thread before proceeding further.
        pthread_join(threads[p], NULL);
    }

    delete[] threads;

    for(int j = 0; j < poss.size(); j++){
        stringstream ss(poss[j]);
        string inte;
        while(ss >> inte){
            //the decoded symbol will replace all its respective indexs in the final str
            decodedmssg[stoi(inte)] = var[j].symbol;
        }
    }

    delete[] var;
    cout << "Original message: " << decodedmssg << endl;

    return 0;
}
