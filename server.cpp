
#include "common.h"



int main(int argc, char** argv)
{
	/* loop  to handle user's input (from keyboard) 
	* @START -port=9999  (or default = 9999)
	* @ADD -topics=topic1,topic2    (one or multi topics)
	* @STOP
	*/
	char* command = NULL;
	size_t len = 0;
	bool isrunning = false;


	while (true)
	{
		if (getline(&command, &len, stdin) != -1)
		{
			printf("Command: %s\n", command);
			HandleUserCommand(command, &isrunning);
		}
		
		
	}
	if (command)
		free(command);
	return 0;

}

void HandleUserCommand(const char* command, bool* isrunning)
{
	int* port;
	char* p = NULL;
	char port_str[6] = { 0 };
	char topics[BUFFER_SIZE];
	char* topic_name = NULL;
	thread_t thread_id;

	if (strstr("@START ", command) != NULL)
	{
		if (*isrunning)
		{
			printf("[COMMAND FAILED]\tServer has already started\n");
			return;
		}

		port = (int*)malloc(sizeof(int));

		*port = DEFAULT_PORT;
		if ((p = strstr("-port=", command)) != NULL)
		{
			strcpy(port_str, p + 6);
			*port = atoi(port_str);
		}

		

		InitAll();

		if (pthread_create(&thread_id, NULL, ThreadStartServer, (void*)port) != 0)
		{
			perror("pthread_create");
			close(server);
			exit(1);
		}
		*isrunning = true;
	
		
	}
	else if (strstr("@ADD ", command) != NULL)
	{
		if (!(*isrunning))
		{
			printf("[COMMAND FAILED]\tStart server before adding any topic\n");
			return;
		}
		if ((p = strstr("-topics=", command)) != NULL)
		{
			strcpy(topics, p + 8);

			// split input string to get topics list (separated by ',')
			topic_name = strtok(topics, ",");
			while (topic_name != NULL)
			{
				AddNewTopic(topic_name);
				topic_name = strtok(NULL, ",");
			}

		}

	}
	else if (strstr("@STOP ", command) != NULL)
	{
		if (!(*isrunning))
		{
			printf("[COMMAND FAILED]\tStart server firstly\n");
			return;
		}

		ClearAll();
		close(server_sock);
		*isrunning = false;
		printf("[SERVER STOP]");
	}
	

}
void* ThreadStartServer(void* lpPort)
{
	// thread safe
	int port = *((int*)lpPort);
	free(lpPort);
	lpPort = NULL;
	
	pthread_detach(pthread_self());

	struct sockaddr_in serverAddr, clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	const int opt = 1;
	int* client_sock;

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family	= AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}

	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind");
		exit(1);
	}

	if (listen(server, SOMAXCONN) < 0)
	{
		perror("listen");
		exit(1);
	}

	printf("[START SERVER]\tServer start listening at port %d\n", port);

	while (true)
	{
		client_sock = (int*)malloc(sizeof(int));

		if ((*client_sock = accept(server, (sockaddr*)&clientAddr, &clientAddrLen)) < 0)
		{
			//perror("accept");
			break;
		}

		printf("[CONNECTION]\t New connection from client [%s:%d]\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		// create thread for each client

		pthread_t tid;

		if (pthread_create(&tid, NULL, ThreadHandleClient, (void*)client_sock) != 0)
		{
			perror("pthread_create");
			free(client);
			continue;
		}
	}


}

void* ThreadHandleClient(void* lpClientSock)
{
	int client_sock = *((int*)lpClientSock);
	free(lpClientSock);
	lpClientSock = NULL;

	pthread_detach(pthread_self());

	Status status = ESTABLISH;
	char sendBuf[BUFFER_SIZE];
	char recvBuf[BUFFER_SIZE];
	PCLIENT client;

	client = (PCLIENT)malloc(sizeof(CLIENT));
	client->client_sock = client_sock;

	while (true)
	{
		
		if (!handleRead)
		switch (status)
		{
		case ESTABLISH:

			break;
		case REGISTRY:
			break;
		case EXCHANGE:
			break;
		default:
			break;
		}
	}
	

	

}

bool handleRead(PCLIENT client, char* buffer, int bufSize)
{
	int nBytes;
	nBytes = read(client, buffer, bufSize - 1);
	if (nBytes < 0)
	{
		close(client);
		perror("read");
		exit(1);
	}
	if (nBytes == 0)
	{
		printf("[Client may close]\n");
		close(client);
		return false;
	}
	buffer[nBytes] = '\0';
	printf(">CLIENT:%s\n", buffer);
	return true;

}

void InitAll()
{
	// init mutex for topics and clients list
	pthread_mutex_init(&mutex_topics_list);
	pthread_mutex_init(&mutex_clients_list);

}
void ClearAll()
{

	FreeAllClients();
	FreeAllTopics();

	// destroy mutex for topics list and client list
	pthread_mutex_destroy(&mutex_topics_list);
	pthread_mutex_destroy(&mutex_clients_list);

}


