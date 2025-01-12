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
#include "subp.h"
#include <thread>
#include <mutex>


using namespace std;

mutex mtx;

string BackendFunction(const string& username, const string& password, const string& message) {
	return "User: " + username + " sent message: \n" + subp(message);
}

string processRequest(const char* request) {
	string req(request);
	size_t firstSpace = req.find(' ');
	size_t secondSpace = req.find(' ', firstSpace + 1);

	if(firstSpace == string::npos || secondSpace == string::npos) {
		return "Invalid format";
	}

	string username = req.substr(0, firstSpace);
	string password = req.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	string message = req.substr(secondSpace + 1);

	return BackendFunction(username, password, message);
}

void handleClient(int clientSocket) {
	{
		lock_guard<mutex> lock(mtx);
		cout << "Handling client in thread... Socket: " << clientSocket << endl;
	}

	while(true) {
		cout << "Waiting for message..." << endl;  // Debug print
		char buffer[1024] = {0};
		int bytesRead = read(clientSocket, buffer, 1024);

		cout << "Bytes read: " << bytesRead << endl;  // Debug print

		if(bytesRead <= 0) {
			lock_guard<mutex> lock(mtx);
			cout << "Client disconnected (bytes: " << bytesRead << ")" << endl;
			break;
		}

		{
			lock_guard<mutex> lock(mtx);
			cout << "Received: " << buffer << endl;
		}

		string response = processRequest(buffer);
		cout << "Sending response..." << endl;  // Debug print
		send(clientSocket, response.c_str(), response.length(), 0);
	}

	close(clientSocket);
}

int main() {
	const int PORT = 7432;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket < 0) {
	cout << "Socket creation failed" << endl;
	return -1;
	}

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(PORT);

	cout << "Server PORT: " << PORT << endl;

	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	ioctl(serverSocket, SIOCGIFADDR, &ifr);

	cout << "Server IP: " 
	<< inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) 
	<< endl;

	int reuseAddr = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
	cout << "Binding failed" << endl;
	return -1;
	}

	listen(serverSocket, 5);
	cout << "Server is listening..." << endl;

	/// ///////////
	char actualIP[INET_ADDRSTRLEN];
	struct sockaddr_in actualAddr;
	socklen_t len = sizeof(actualAddr);


	char hostname[1024];
	gethostname(hostname, 1024);
	cout << "Hostname: " << hostname << endl;


	hostent* he;
	he = gethostbyname(hostname); 

	if (he == NULL) {
		cout << "Failed to get host name" << endl;
		return -1;
	}

	char* ip = inet_ntoa(*(struct in_addr*)he->h_addr_list[0]);
	cout << "Server is listening on IP: " << ip << endl;

	if (getsockname(serverSocket, (struct sockaddr*)&actualAddr, &len) == -1) {
		cout << "Failed to get socket address" << endl;
		return -1;
	}


	/// /////////////////

	while (true) {
		sockaddr_in clientAddress;
		socklen_t clientLength = sizeof(clientAddress);
		int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLength);

		if (clientSocket == -1) {
			cerr << "Error occurred while connecting to client!" << endl;
			continue;
		}

		{
			lock_guard<mutex> lock(mtx);
			cout << "New connection from: "
					  << inet_ntoa(clientAddress.sin_addr)
					  << ":" << ntohs(clientAddress.sin_port) << endl;
			cout<<"\nDetaching...";
		}
		cout << "Handling client in thread... Socket: " << clientSocket << endl;
		thread clientThread(handleClient, clientSocket);
		clientThread.detach();
	}


return 0;
}
