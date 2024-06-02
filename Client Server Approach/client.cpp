#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <vector>

void processResponse(int sockfd) {
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

    std::string rleString(buffer);
    delete[] buffer;

    n = read(sockfd, &sMessage, sizeof(int));
    if (n < 0) {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }

    std::vector<int> frequencyArray(sMessage / sizeof(int));
    n = read(sockfd, frequencyArray.data(), sMessage);
    if (n < 0) {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }

    // Print the received information
    std::cout << "RLE String: " << rleString << std::endl;
    std::cout << "RLE Frequencies: ";
    for (const auto &freq : frequencyArray) {
        std::cout << freq << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " hostname port_no < input_filename\n";
        exit(1);
    }

    char *hostname = argv[1];
    int portno = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "ERROR opening socket";
        exit(1);
    }

    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        std::cerr << "ERROR, no such host";
        exit(1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR connecting";
        exit(1);
    }

    // Read input from the file
    std::vector<std::string> inputStrings;
    std::string inputString;

    while (getline(std::cin, inputString)) {
        inputStrings.push_back(inputString);
    }

    for (const auto &str : inputStrings) {
        int sMessage = static_cast<int>(str.length());
        int n = write(sockfd, &sMessage, sizeof(int));
        if (n < 0) {
            std::cerr << "ERROR writing to socket";
            exit(1);
        }

        n = write(sockfd, str.c_str(), sMessage);
        if (n < 0) {
            std::cerr << "ERROR writing to socket";
            exit(1);
        }

        processResponse(sockfd);
    }

    close(sockfd);
    return 0;
}
