// Nguyen Thanh Huyen
// 18020666
// Nhom 4

#include <stdio.h>
#include <string.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <arpa/inet.h>          //inet_ntop
#include <cstdlib>
#include <pthread.h>        //pthread, mutex...
#include <sys/stat.h>       //lstat
#include <unistd.h>         //read, write

#include <vector>

#define	BUFFER_SIZE 1024
#define DEFAULT_PORT 9999


#define	UNKNOWN				'0' 
#define	CONNECT				'1'
#define	CONACK				'2'
#define	PUBLISH				'3'
#define	PUBACK				'4'
#define	SUBSCRIBE			'5'
#define	SUBACK				'6'
#define	MESSAGE				'7'
#define	MSGACK				'8'
#define	CHANGEPUB			'9'
#define	CHANGEPUBACK		'a'
#define	CHANGESUB			'b'
#define	CHANGESUBACK		'c'
#define	DISCONNECT			'd'
#define	DISCONACK			'e'

enum ClientType {
	PUBLISHER,
	SUBSCRIBER
};

enum Status {
	ESTABLISH,			/* establish connection between server and client (HELLO) */
	REGISTRY,			/* registry with clientID and role Producer or Subscriber  */
	EXCHANGE
};
/*--------------------------------- CUSTOM STRUCTURE-------------------------*/


typedef struct CLIENT {
	
	char*		client_id;
	int			client_sock;
	ClientType	type;

	// mutex_write for writing message to consumer_sock_fd (multi thread_topic can write at the same time) 
	pthread_mutex_t mutex_write;

	// for thread-safe
	pthread_key_t readline_key;

	// may be written by request of CHANGESUB/CHANGEPUB and be read to specify which topics received message from publishers
	pthread_mutex_t mutex_follow_topics_list;		


	// list pointers pointing to all topics which this client follows (publish_topic for producer, subcribe_topic for customer)
	// update after client send request "CHANGESUB/CHANGEPUB"
	std::vector<PTOPIC> follow_topics_list;

} *PCLIENT;


typedef struct TOPIC
{
	char* topic_name;

	

	// list pointers pointing to all publishers and subscribers of this topic
	// multi clients can request update their following-topic at the same time

	std::vector<PCLIENT> clients_list;

	// mutex for reading and writing to clients_list, init at AddNewTopic() and destroy at FreeAllTopics()
	pthread_mutex_t clients_list;
	

	// queue of message, is written by publishers, and is read and deleted by thread_topic to send to all subcribers
	pthread_mutex_t mutex_msg_queue;
	std::vector<char*> message_queue;

} *PTOPIC;



/*--------------------------------- GLOBAL VARIABLES-------------------------*/

// server socket to listen connection
int server_sock;

// list clients
// mutex list clients

std::vector<PCLIENT> clients_list;
pthread_mutex_t mutex_clients_list;



// list current topics
std::vector<PTOPIC> topics_list;
pthread_mutex_t mutex_topics_list;


/*--------------------------------- FUNCTION-------------------------*/ /

//
// handle each command from keyboard
//
void HandleUserCommand(const char* command, bool* isrunning);


/* START */

//
// init mutex 
//
void InitAll();

//
// thread start server and listen connection
//
void* ThreadStartServer(void* lpPort);
void* ThreadHandleClient(void* lpClientSock);



/* CLIENT */

void FreeClient(int client_sock);
void FreeAllClients();

void AddNewClient(PCLIENT pClient, char* client_id, ClientType type, char* topics);



/* READ&WRITE SAFE */

bool HandleRead(PCLIENT client, char* buffer, int bufSize);
bool HandleWrite(PCLIENT client, char* buffer);


/* TOPIC */

PTOPIC	GetTopic(const char* topic_name);
//void	FreeTopic(PTOPIC topic);
void	FreeAllTopics();

bool	CheckTopicName(const char* topic_name);
void	AddNewTopic(const char* topic_name);
void	UpdateTopicsList(PTOPIC new_topic);

void	GetCurrentTopics(char* current_topics_str);

/* STOP */

//
// clear all if command is STOP: close all mutex, socket, free memory
//
void ClearAll();