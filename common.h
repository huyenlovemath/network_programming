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
	UNDEFINED,
	PRODUCER,
	CUSTOMER
};

/*--------------------------------- CUSTOM STRUCTURE-------------------------*/

typedef struct CLIENT {
	
	char*		client_id;
	int			client_sock;
	ClientType	type = UNDEFINED;

	// mutex_write for writing message to consumer_sock_fd (multi thread_topic can write at the same time) 
	// only init this mutex if type is CONSUMER
	pthread_mutex_t mutex_write;

	// may be written by request of CHANGESUB/CHANGEPUB and be read to specify which topics received message from publishers
	pthread_mutex_t mutex_follow_topics_list;

	// list pointers pointing to all topics which this client follows (publish_topic for producer, subcribe_topic for customer)
	// update after client send request "CHANGESUB/CHANGEPUB"
	std::vector<PTOPIC> follow_topics_list;

} *PCLIENT;


typedef struct TOPIC
{
	char* topic_name;

	// list pointers pointing to all subcribers of this topic
	// multi subcribers can request update their sub_topic at the same time
	pthread_mutex_t mutex_sub_list;
	std::vector<PCLIENT> sub_list;				
	
	// list pointers pointing to all publishers of this topic
	// multi publisher can request update their pub_topic at the same time
	pthread_mutex_t mutex_pub_list;
	std::vector<PCLIENT> pub_list;

	// queue of message, is written by publishers, and is read and deleted by thread_topic to send to all subcribers
	pthread_mutex_t mutex_msg_queue;
	std::vector<char*> message_queue;

} *PTOPIC;



/*--------------------------------- GLOBAL VARIABLES-------------------------*/


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


/* after command @start */

//
// init mutex 
//
void InitAll();

//
// start server listen at a port (default 9999)
//
void StartServer(int port);


/* after command @stop */

//
// clear all if command is STOP: close all mutex, socket, free memory
//
void ClearAll()
{

	freeAllTopics();
	freeAllClients();
}




/* CLIENT */
void FreeClient(int client_sock);
void FreeAllClients();

void AddNewClient(PCLIENT pClient, char* client_id, int client_sock, ClientType type);

/* TOPIC */
PTOPIC	GetTopic(const char* topic_name);
void	FreeTopic(PTOPIC topic);
void	FreeAllTopics();

bool	CheckTopicName(const char* topic_name);
void	AddNewTopic(const char* topic_name);
void	UpdateTopicsList(PTOPIC new_topic);

void	GetCurrentTopics(char* current_topics_str);