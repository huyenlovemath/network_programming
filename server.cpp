
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
	int port = DEFAULT_PORT;
	char* p = NULL;
	char port_str[6] = { 0 };
	char topics[BUFFER_SIZE];
	char* topic_name = NULL;

	if (strstr("@START ", command) != NULL)
	{
		if (*isrunning)
		{
			printf("[COMMAND FAILED]\tServer has already started\n");
			return;
		}
		if ((p = strstr("-port=", command)) != NULL)
		{
			strcpy(port_str, p + 6);
			port = atoi(port_str);
		}

		*isrunning = true;

		InitAll();
		StartServer(port);

		
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
	else
	

}
void StartServer(int port)
{

}



void AddNewTopic(const char* topic_name)
{
	PTOPIC new_topic = NULL;
	size_t len;

	if (!CheckTopicName(topic_name))
	{
		printf("[ADD FAILED]\tTopic name %s is invalid\n", topic_name);
		return;
	}

	// allocate mem for new topic instance

	new_topic = (PTOPIC)malloc(sizeof(TOPIC));

	// allocate and store name of topic

	len = strlen(topic_name);
	new_topic->topic_name = (char*)malloc(len + 1);
	strcpy(new_topic->topic_name, topic_name);
	new_topic->topic_name[len] = '\0';

	// init mutex for sub_list adn pub_list

	if (pthread_mutex_init(&new_topic->mutex_sub_list, NULL) != 0 || pthread_mutex_init(&new_topic->mutex_pub_list, NULL) != 0)
	{
		perror("topic mutex");
		return;
	}
	

	printf("[ADD TOPIC]\tTopic name %s create\n", topic_name);

	// update topic_list
	UpdateTopicsList(new_topic);
}

//
// function checks if topic_name exists or doesnt
// return yes if doesnt, it means that new topic_name is valid
// 
bool CheckTopicName(const char* topic_name)
{
	PTOPIC topic;

	if (!topic_name)
		return false;

	if (GetTopic(topic_name) == NULL)
		//topic_name doesnt exist
		return true;
	else
		//topic_name exists
		return false;

}
void UpdateTopicsList(PTOPIC new_topic)
{
	char current_topics[BUFFER_SIZE] = { 0 };
	/* add to topics_list */

	if (!new_topic)
		return;

	// mutex
	pthread_mutex_lock(&mutex_topics_list);

	topics_list.push_back(new_topic);

	pthread_mutex_unlock(&mutex_topics_list);
	//end mutex

	printf("[UPDATE TOPICS]:\tAdd new topic to topics list\n");
	GetCurrentTopics(current_topics);

}

//
// get string of all topic_names split by ',' 
// topic_name1,topic_name2,topic_name3
//
void GetCurrentTopics(char* current_topics_str)
{
	PTOPIC topic;

	// mutex
	pthread_mutex_lock(&mutex_topics_list);

	for (int i = 0; i < topics_list.size(); i++)
	{
		topic = topics_list.at(i);

		if (i)
			strcat(current_topics, ",");
		strcat(current_topics, topic->topic_name);
	}
	
	pthread_mutex_unlock(&mutex_topics_list);
	
	//end mutex

	printf("[CURRENT TOPICS]:\t%s", current_topics);
}

//
// Return pointer to topic which has topic_name, NULL if topic_name doesn't exist
//

PTOPIC GetTopic(char* topic_name)
{
	PTOPIC topic = NULL;

	// mutex
	pthread_mutex_lock(&mutex_topics_list);

	for (int i = 0; i < topics_list.size(); i++)
	{
		topic = topics_list.at(i);

		if (!strcmp(topic->topic_name, topic_name))
			break;

		if (i == topics_list.size() - 1)
			topic = NULL;
	}

	pthread_mutex_unlock(&mutex_topics_list);

	//end mutex
	return topic;

}
void FreeTopic(PTOPIC topic)
{
	free
}