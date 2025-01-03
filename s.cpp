#include <net/if.h>
#include <sys/ioctl.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <cstring>

#include <thread>
#include <mutex>


using namespace std;

std::mutex mtx;

std::string testBackendFunction(const std::string& username, const std::string& password, const std::string& message) {
return "User: " + username + " sent message: \n" + subp(message);
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

void handleClient(int clientSocket) {
{
std::lock_guard<std::mutex> lock(mtx);
std::cout << "Handling client in thread... Socket: " << clientSocket << std::endl;
}

while(true) {
std::cout << "Waiting for message..." << std::endl;  // Debug print
char buffer[1024] = {0};
int bytesRead = read(clientSocket, buffer, 1024);

std::cout << "Bytes read: " << bytesRead << std::endl;  // Debug print

if(bytesRead <= 0) {
std::lock_guard<std::mutex> lock(mtx);
std::cout << "Client disconnected (bytes: " << bytesRead << ")" << std::endl;
break;
}

{
std::lock_guard<std::mutex> lock(mtx);
std::cout << "Received: " << buffer << std::endl;
}

std::string response = processRequest(buffer);
std::cout << "Sending response..." << std::endl;  // Debug print
send(clientSocket, response.c_str(), response.length(), 0);
}

close(clientSocket);
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

struct ifreq ifr;
ifr.ifr_addr.sa_family = AF_INET;
strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
ioctl(serverSocket, SIOCGIFADDR, &ifr);

std::cout << "Server IP: " 
<< inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) 
<< std::endl;

int reuseAddr = 1;
setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
std::cout << "Binding failed" << std::endl;
return -1;
}

listen(serverSocket, 5);
std::cout << "Server is listening..." << std::endl;

/// ///////////

// Получим IP
char actualIP[INET_ADDRSTRLEN];
struct sockaddr_in actualAddr;
socklen_t len = sizeof(actualAddr);


char hostname[1024];
gethostname(hostname, 1024);
std::cout << "Hostname: " << hostname << std::endl;


hostent* he;
he = gethostbyname(hostname);  // hostname - имя вашей машины

if (he == NULL) {
    std::cout << "Failed to get host name" << std::endl;
    return -1;
}

char* ip = inet_ntoa(*(struct in_addr*)he->h_addr_list[0]);
std::cout << "Server is listening on IP: " << ip << std::endl;

if (getsockname(serverSocket, (struct sockaddr*)&actualAddr, &len) == -1) {
    std::cout << "Failed to get socket address" << std::endl;
    return -1;
}


/// /////////////////

while (true) {
    sockaddr_in clientAddress;
    socklen_t clientLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);

    if (clientSocket == -1) {
        std::cerr << "Error occurred while connecting to client!" << std::endl;
        continue;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "New connection from: "
                  << inet_ntoa(clientAddress.sin_addr)
                  << ":" << ntohs(clientAddress.sin_port) << std::endl;
		cout<<"\nDetaching...";
    }
	std::cout << "Handling client in thread... Socket: " << clientSocket << std::endl;
    std::thread clientThread(handleClient, clientSocket);
    clientThread.detach();
}


return 0;
}
