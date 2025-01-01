#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

std::string testBackendFunction(const std::string& username, const std::string& password, const std::string& message) {
return "User: " + username + " sent message: " + message;
}

std::string processRequest(const char* request) {
std::string req(request);
size_t firstSpace = req.find(' ');
size_t secondSpace = req.find(' ', firstSpace + 1);

if(firstSpace == std::string::npos || secondSpace == std::string::npos) {
return "Invalid format";
}

std::string username = req.substr(0, firstSpace);
std::string password = req.substr(firstSpace + 1, secondSpace - firstSpace - 1);
std::string message = req.substr(secondSpace + 1);

return testBackendFunction(username, password, message);
}

int main() {
const int PORT = 7432;
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

if (serverSocket < 0) {
std::cout << "Socket creation failed" << std::endl;
return -1;
}

struct sockaddr_in serverAddress;
serverAddress.sin_family = AF_INET;
serverAddress.sin_addr.s_addr = INADDR_ANY;
serverAddress.sin_port = htons(PORT);

std::cout << "Server PORT: " << PORT << std::endl;

if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
std::cout << "Binding failed" << std::endl;
return -1;
}

listen(serverSocket, 3);
std::cout << "Server is listening..." << std::endl;

while (true) {
sockaddr_in clientAddress;
socklen_t clientLength = sizeof(clientAddress);
int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);

std::cout << "New connection from: "
<< inet_ntoa(clientAddress.sin_addr)
<< ":" << ntohs(clientAddress.sin_port) << std::endl;

char buffer[1024] = {0};
read(clientSocket, buffer, 1024);
std::cout << "Received: " << buffer << std::endl;

std::string response = processRequest(buffer);
send(clientSocket, response.c_str(), response.length(), 0);
close(clientSocket);
}

return 0;
}
