#include <topics.h>

void AddNewTopic(const char* topic_name)
{
	PTOPIC new_topic = NULL;
	size_t len;

	if (!CheckTopicName(topic_name))
	{
		printf("[ADD FAILED]\tTopic name %s is invalid\n", topic_name);
		return;
	}

	// allocate memory for new topic

	new_topic = (PTOPIC)malloc(sizeof(TOPIC));

	// allocate and store name of topic

	len = strlen(topic_name);
	new_topic->topic_name = (char*)malloc(len + 1);
	strcpy(new_topic->topic_name, topic_name);
	new_topic->topic_name[len] = '\0';

	// init mutex for sub_list, pub_list and msg_queue 

	if (pthread_mutex_init(&new_topic->mutex_sub_list, NULL) != 0 || pthread_mutex_init(&new_topic->mutex_pub_list, NULL) != 0 || pthread_mutex_init(&new_topic->mutex_msg_queue, NULL) != 0)
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

//
// Update topics list: add pointer to new_topic instance to the end of topics list
//
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

	printf("[UPDATE TOPICS]\tAdd new topic to topics list\n");
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

	printf("[CURRENT TOPICS]\t%s", current_topics);
}

//
// Return pointer to topic which has topic_name, NULL if topic_name doesn't exist
//

PTOPIC GetTopic(const char* topic_name)
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

// 
// make sure that you have already free all clients before using this function to make sure that no topic data in use
//

void FreeAllTopics()
{
	PTOPIC topic = NULL;
	char* msg = NULL;

	// mutex
	pthread_mutex_lock(&mutex_topics_list);

	while (!topics_list.empty())
	{
		topic = topics_list.back();

		// free topic name
		free(topic->topic_name);

		// destroy mutex of topic
		pthread_mutex_destroy(&topic->mutex_sub_list);
		pthread_mutex_destroy(&topic->mutex_pub_list);
		pthread_mutex_destroy(&topic->mutex_msg_queue);

		// clear all message_queue
		while (!topic->message_queue.empty())
		{
			msg = topic->message_queue.back();
			if (msg)
				free(msg);
			topic->message_queue.pop_back();
		}
		topic->sub_list.clear();
		topic->pub_list.clear();

		// free memory of topic 
		free(topic);
		topics_list.pop_back();
	}

	pthread_mutex_unlock(&mutex_topics_list);

	//end mutex
	printf("[FREE TOPICS]\tAll topics are free\n");
}