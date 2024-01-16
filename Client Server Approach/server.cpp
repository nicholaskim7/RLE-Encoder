#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <string.h>
#include <map>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <queue>
using namespace std;


struct args
{
    vector<string> com;
    map<string,char> m;
    char* finalstr;
};


struct Node {
    char c;
    int freq;
    Node *left, *right;

    Node(char c, int freq) {
        this->c = c;
        this->freq = freq;
        left = right = NULL;
    }
};


struct compare {
    bool operator()(Node* l, Node* r) {
        if((l->freq == r->freq)){  
            if(l->c == r->c){   
                return l < r;  //tie breaker
            }
            return(l->c > r->c);  //compare ascii
        }
        return (l->freq > r->freq);  //compare freq
    } 
};


Node* HuffmanCodes(vector<char> data, vector<int> freq, int size) {
    Node *left, *right, *top;

    priority_queue<Node*, vector<Node*>, compare> minHeap;

    for (int i = 0; i < size; i++)
        minHeap.push(new Node(data[i], freq[i]));

    while (minHeap.size() != 1) {
        left = minHeap.top();
        minHeap.pop();
        right = minHeap.top();
        minHeap.pop();
        top = new Node(31, left->freq + right->freq);
        top->left = left;
        top->right = right;
        minHeap.push(top);
    }

    Node* root = minHeap.top();
    return root;
}

map<string, char> printCodes(Node* root, string str, map<string, char> &m){
    //create and return map containing code and symbol of that code
    if(root->c != 31){
        cout << "Symbol: " << root->c << ", Frequency: " << root->freq << ", Code: " << str << endl;
        m[str] = root->c;  //create map
    }
    else{
        printCodes(root->left, str + "0", m);
        printCodes(root->right, str + "1", m);
    }
    return m;
}


//non-blocking manner, ensuring that no zombie processes are left unattended
void fireman(int){
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
}



void error(char *msg){
    perror(msg);
    exit(1);
}




int main(int argc, char *argv[]){
    string line;
    char symb;
    int freq;
    int size = 0;
    string c;
    vector<char> alphas;
    vector <int> freqs;
    map<string, char> m;
    while(getline(cin, line)){
        symb = line[0];
        freq = line[2] - '0';
        alphas.push_back(symb);
        freqs.push_back(freq);
        size++;
    }
    Node* root = HuffmanCodes(alphas, freqs, size);
    m = printCodes(root, c, m);

    int sockfd, newsockfd, portno, clilen;

    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    //if less than two arguments than port number was not provided
    if(argc < 2){
        cout << "Error no port provided" << endl;
        exit(1);
    }

    //socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        error("Error opening socket");
        exit(1);
    }

    //used to zero out a block of memory
    bzero((char *)&serv_addr, sizeof(serv_addr));

    //portnumber
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    //listen for incoming connections (5 is the max pending conections)
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    //whenever a child process changes state (exits or stops), the fireman() function will be invoked to perform any necessary cleanup actions.
    signal(SIGCHLD, fireman);

    while (true){  //infinate loop to continuously accept incomming client conections
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (fork() == 0){
            if (newsockfd < 0)
                error("new socket wont open");
            bzero(buffer, 256);
            //read data from the client and store in buffer
            n = read(newsockfd, buffer, 255);
            if (n < 0)
                error("ERROR reading from socket");
            string temp=buffer;  
            //clear the buffer using bzero()
            bzero(buffer, 256);
            //setting up response which is stored in m
            buffer[0] = m[temp];
            //write to client
            n = write(newsockfd, buffer, 255);
            if (n < 0)
                error("ERROR writing to socket");
            close(newsockfd);
            _exit(0);
        }
    }
    close(sockfd);
    return 0;
}