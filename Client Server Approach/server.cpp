#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

void handleRequest(int sockfd) {
    int sMessage;
    int n = read(sockfd, &sMessage, sizeof(int));
    if (n < 0) {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }

    char *buffer = new char[sMessage + 1];
    bzero(buffer, sMessage + 1);
    n = read(sockfd, buffer, sMessage);
    if (n < 0) {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }

    std::string inputString(buffer);
    delete[] buffer;

    // Perform RLE encoding
    std::string rleString;
    std::vector<int> freq;

    //char prevChar = inputString[0];
    int count = 1;
    
    for (int i = 1; i <= inputString.length(); i++) {
        if (i < inputString.length() && inputString[i] == inputString[i - 1]) {
            count++;
        } else {
            if (count == 1) {
                //if only one then just add the one char
                rleString += inputString[i - 1];
            } else {
                //if count more than one than "running" add twice
                rleString += inputString[i - 1];
                rleString += inputString[i - 1];
                freq.push_back(count);  //consider frequency
            }
            count = 1; //reset count
        }
    }

    // Add the last character and its count
    // rleString += prevChar;
    // freq += std::to_string(count) + " ";

    // Return the RLE string and the frequency array to the client program
    sMessage = static_cast<int>(rleString.length()) + 1; // Increment the size by one
    n = write(sockfd, &sMessage, sizeof(int));
    if (n < 0) {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }

    n = write(sockfd, rleString.c_str(), sMessage);
    if (n < 0) {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }

    sMessage = static_cast<int>(freq.size()) * sizeof(int); // Increment the size by one
    n = write(sockfd, &sMessage, sizeof(int));
    if (n < 0) {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }

    n = write(sockfd, freq.data(), sMessage);
    if (n < 0) {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }

    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " port\n";
        exit(1);
    }

    int portno = atoi(argv[1]);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "ERROR opening socket";
        exit(1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR on binding";
        exit(1);
    }

    listen(sockfd, 5);

    while (true) {
        int newsockfd = accept(sockfd, NULL, NULL);
        if (newsockfd < 0) {
            std::cerr << "ERROR on accept";
            exit(1);
        }

        // Create a new process to handle the client request
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "ERROR on fork";
            exit(1);
        }

        if (pid == 0) {
            // Child process
            close(sockfd);
            handleRequest(newsockfd);
            exit(0);
        } else {
            // Parent process
            close(newsockfd);
        }
    }

    close(sockfd);
    return 0;
}
