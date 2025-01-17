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
		cerr << "Failed to create socket" << endl;
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
		cerr << "Failed to connect to server" << endl;
		return -1;
	}

	cout << "Connected to server..." << endl;
	bool firstRun = true;
	while (true) {
		cout << "Enter message (or QUIT to exit): ";
		if (firstRun) {
			cin.clear();
			cin.ignore(10000, '\n');
			firstRun = false;
		}
		
		cin.getline(message, 1024);

		if (string(message) == "QUIT") {
			break;
		}

		// Send message to server
		char paddedMessage[1024] = {0};  // Create padded buffer
		strcpy(paddedMessage, message);   // Copy message into it
		send(client_fd, paddedMessage, 1024, MSG_WAITALL);  // Send full buffer

		// Receive reply from server
		read(client_fd, reply, 1024);

		cout << "Server replied: " << reply << endl;
	}

	// Close socket
	close(client_fd);

	return 0;
}
