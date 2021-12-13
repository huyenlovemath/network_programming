
#include "server.h"



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
				Topic_AddNew(topic);
				topic_name = strtok(NULL, ",");
			}

		}

	}
	

}
void StartServer(int port)
{

}
void Topic_AddNew (const char* topic_name)
{
	PTOPIC new_topic = NULL;
	

	// update topic_list
	Topic_UpdateTopicsList(new_topic);
}

void Topic_UpdateTopicsList(PTOPIC new_topic)
{
	// mutex

	// write to list
	//end mutex


}