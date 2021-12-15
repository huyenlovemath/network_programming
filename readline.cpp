#include "common.h"


/*------- READLINE THREAD-SAFE-----------*/

typedef struct Rline {
	int		rl_cnt;				/* initialize to 0 */
	char*	rl_bufptr;			/* initialize to rl_buf */
	char	rl_buf[MAXLINE];    /* store line data*/
};

//
// read out up to MAXLINE bytes to buffer (rlbuf) of thread specific data
// rl_cnt and ptr to read one by one byte from rl_buf each time 
//
size_t ReadAByteFromBuffer(Rline* thread_specific_data, int sock, char* ptr)
{

	if (thread_specific_data->rl_cnt <= 0)
	{
	again:
		// start read new MAXLINE bytes if had read all bytes in rl_buf before
		if ((thread_specific_data->rl_cnt = read(sock, thread_specific_data->rl_buf, MAXLINE)) < 0)
		{
			if (errno == EINTR)
				goto again;
			return -1;
		}
		else if (thread_specific_data->rl_cnt == 0)
			return 0;
		// set pointer to first byte to read
		thread_specific_data->rl_bufptr = thread_specific_data->rl_buf;
	}

	// read one by one byte 
	thread_specific_data->rl_cnt--;
	*ptr = *thread_specific_data->rl_bufptr++;

	return 1;

}


size_t ReadLineSafe(PCLIENT client, void* line, size_t maxlen)
{
	size_t n;
	char c, *ptr;
	Rline* pthread_specific_data;

	// key create once when add new client

	if ((pthread_specific_data = pthread_getspecific(client->readline_key)) == NULL)
	{
		//allocate and set all bytes to zero
		pthread_specific_data = calloc(sizeof(Rline), 1);

		Pthread_setspecific(client->readline_key, pthread_specific_data);
	}

	// ptr is used for updating one by one byte to line
	ptr = line;

	// read until max_len or get '\n'
	for (n = 1; n < max_len; n++)
	{
		nread = ReadAByteFromBuffer(pthread_specific_data, client->client_sock, &c);
		
		if (nread == 1)
		{
			*ptr++ = c;
			if (c == '\n')
				break;
		}
		
		// EOF, n-1 bytes read 
		if (nread == 0)
		{
			*ptr = 0;
			return (n - 1);
		}
		
		// error, errno set by read()
		return -1;
		
		
	}

	// '\n' -> 0
	*ptr = 0;
	return n;

}

bool HandleRead(PCLIENT client, char* buffer, int bufSize)
{
	size_t nread;

	nread = ReadLineSafe(client, buffer, bufSize);

	if (nreads < 0)
	{
		printf("[READLINE ERR]\tReadlineSafe return negative value\n");
		return false;
	}
	if (nreads == 0)
	{
		printf("[READLINE ERR]\tClient closed connection\n");
	}

	printf("[%s > SERVER]\t%s\n", client->client_id, buffer);

	return true;
}



/*-------------WRITE SAFE------------*/

bool HandleWrite(PCLIENT client, char* buffer)
{
	size_t nwrite;

	strcat(buffer, "\n");

	// mutex
	pthread_mutex_lock(&client->mutex_write);
	nwrite = write(client->client_sock, buffer, strlen(buffer));
	pthread_mutex_unlock(&client->mutex_write);

	if (nwrite < 0)
	{
		printf("[WRITELINE ERR]\tWrite return neg value\n");
		return false;
	}

	if (nwrite == 0)
	{
		printf("[WRITELINE ERR]\tClient closed connection\n");
		return false;
	}

	printf("[SERVER > %s]\t%s\n", client->client_id, buffer);

	//bzero(buffer, sizeof(buffer));
	return true;


}

