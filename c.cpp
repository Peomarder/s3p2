#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

int main() {
int client_fd;
struct sockaddr_in server_addr;
int server_len = sizeof(server_addr);
char message[1024] = {0};
char reply[1024] = {0};

// Create socket
if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
std::cerr << "Failed to create socket" << std::endl;
return -1;
}

// Set server address
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(7432);
string ipcon = "";
cout << "\nEnter ip: ";
cin >> ipcon; //"127.0.0.1"
inet_pton(AF_INET, ipcon.c_str(), &server_addr.sin_addr);

// Connect to server
if (connect(client_fd, (struct sockaddr*)&server_addr, server_len) < 0) {
std::cerr << "Failed to connect to server" << std::endl;
return -1;
}

std::cout << "Connected to server..." << std::endl;

while (true) {
std::cout << "Enter message (or QUIT to exit): ";
    if (firstRun) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        firstRun = false;
    }
    
std::cin.getline(message, 1024);

if (std::string(message) == "QUIT") {
break;
}

// Send message to server
char paddedMessage[1024] = {0};  // Create padded buffer
strcpy(paddedMessage, message);   // Copy message into it
send(client_fd, paddedMessage, 1024, MSG_WAITALL);  // Send full buffer

// Receive reply from server
read(client_fd, reply, 1024);

std::cout << "Server replied: " << reply << std::endl;
}

// Close socket
close(client_fd);

return 0;
}
