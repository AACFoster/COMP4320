//COMP4320 Project 1
//Lindsey Rafalsky lcr0029
//Isabelle Cochran idc0004
//Ida Foster azf0046





#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <vector>
#include <cstdlib>
#include <netdb.h>
#define PORT 8080
#define TESTFILE "./src/TestFile"
using std::cout;
using std::endl;

std::stringstream buffer;
int damageProb;
int lossProb;
int sockfd;
struct sockaddr_in servaddr;
char packetBuffer[128] = {0};

// connect to the client
int connect()
{
	memset(&servaddr, 0, sizeof(servaddr));

	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	return 0;
}

int calculateChecksum(char packet[])
{
	int checksum = 0;
	// 7 is the first index of the message body
	for (int i = 7; i < 128; i++)
	{
		checksum += packet[i];
	}
	return checksum;
}

// calculate checksum by summing bytes of the packet
void setupChecksum(char packet[])
{
	int checksum = calculateChecksum(packet);

	// place in packet
	char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	packet[2] = digits[checksum / 10000 % 10];
	packet[3] = digits[checksum / 1000 % 10];
	packet[4] = digits[checksum / 100 % 10];
	packet[5] = digits[checksum / 10 % 10];
	packet[6] = digits[checksum % 10];
	cout << "Checksum: " << std::to_string(checksum) << endl;
}

int getGremlinProbabilities()
{
	cout << "Enter probability for packet damage (0-100): ";
	std::cin >> damageProb;
	cout << "Enter probability for packet loss (0-100): ";
	std::cin >> lossProb;
	cout << "Gremlin probabilities are (" << std::to_string(damageProb) << "% damage) and (" << std::to_string(lossProb) << "% loss)" << endl;
	return 0;
}

void damage(char packet[], int amount)
{
	// get random index
	for (int i = 0; i < amount; i++)
	{
		int dice = rand() % 127;
		packet[dice] = 'a' + rand() % 26;
	}
	cout << "GREMLIN: Packet damaged " << amount << " times" << endl;
}

void gremlin(char packet[])
{
	// get random number 1-100
	int dice = rand() % 100 + 1;

	// check if damanged
	if (dice <= damageProb)
	{
		dice = rand() % 10 + 1;
		if (dice == 10)
		{
			damage(packet, 3);
		}
		else if (dice >= 8)
		{
			damage(packet, 2);
		}
		else
		{
			damage(packet, 1);
		}
		return;
	}

	// check if lost, set to null
	dice = rand() % 100 + 1;
	if (dice <= lossProb)
	{
		cout << "GREMLIN: Packet lost" << endl;
		packet[1] = 'B';
	}
	// packet successful
	else
	{
		cout << "GREMLIN: Packet correctly delivered" << endl;
	}
}

void sendPacket(char packet[])
{
	// server is cutting it short atm
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	sendto(sockfd, (const char *)packet, 128,
		   0, (const struct sockaddr *)&servaddr,
		   sizeof(servaddr));
	cout << "Packet sent" << endl;
	int n;
	socklen_t len;
	n = recvfrom(sockfd, (char *)packetBuffer, 128,
				 MSG_WAITALL, (struct sockaddr *)&servaddr,
				 &len);
	packetBuffer[n] = '\0';
	printf("Server : %s\n", packetBuffer);
	close(sockfd);
}

// create packets
void createPackets()
{
	int totalCharCount = 0;
	char sequenceNum = '0';
	int headerSize = 7;

	while (totalCharCount < buffer.str().length())
	{
		char packet[128];
		int charCountInBuffer = headerSize; // start after header

		// setup header
		packet[0] = sequenceNum;
		packet[1] = 'A'; // Error protocol: A if OK, B if ERROR

		cout << "writing data to packet #" << sequenceNum << endl;

		// loop until packet is full or buffer is completely read
		while (totalCharCount < buffer.str().length() && charCountInBuffer < 128)
		{
			packet[charCountInBuffer] = buffer.str()[totalCharCount];
			charCountInBuffer++;
			totalCharCount++;
		}

		// check if packet is full and fill with null
		while (charCountInBuffer < 128)
		{
			packet[charCountInBuffer] = '\0';
			charCountInBuffer++;
		}

		setupChecksum(packet);
		gremlin(packet);
		sequenceNum = (sequenceNum == '0') ? '1' : '0';

		// if packet not lost
		if (packet[1] == 'A')
		{
			// show packet info
			std::string packetString = "";
			for (int i = 0; i < 48; i++)
			{
				packetString += packet[i];
			}

			cout << "Packet #" << sequenceNum << " to be sent: " << packetString << endl;
			sendPacket(packet);
		}
	}

	// create blank ending packet
	cout << "Creating end packet" << endl;
	char endPacket[] = {'\0'};
	sendPacket(endPacket);
}

// read test file into buffer
bool readFile(std::string fileName)
{
	std::fstream file(fileName);
	if (!file.is_open())
	{
		cout << "File could not be found" << endl;
		return false;
	}

	buffer << file.rdbuf();
	return true;
}

bool sendRequest()
{
	char request[] = "PUT TestFile";
	sendto(sockfd, (const char *)request, strlen(request),
		   0, (const struct sockaddr *)&servaddr,
		   sizeof(servaddr));
	cout << "Sending: " << request << endl;
	cout << endl;

	return true;
}

int main(int argc, char const *argv[])
{
	srand(time(0));
	sockfd = connect();
	if (sockfd != 0)
	{
		return -1;
	}

	if (!readFile(TESTFILE))
	{
		return -1;
	}

	getGremlinProbabilities();

	if (!sendRequest())
	{
		return -1;
	}

	createPackets();
	return 0;
}