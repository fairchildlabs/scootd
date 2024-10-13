#ifndef __SCOOTD_H_
#define __SCOOTD_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/wait.h>

#define SCOOTD_ASSERT(_X) assert(_X)


#define SCOOTD_THREAD_VIDEO_0 0

typedef union
{
	unsigned int state;
	struct
	{
		unsigned int video        :  1; //bit 0
		unsigned int frame_rate   :  1; //bit 1 
		unsigned int raw          :  1; //bit 2
		unsigned int rsvd         :  1; //bit 3
		unsigned int resolution   :  4; 

	} vid[2];

} scoot_state;


//  0xABCD_EF0 12
//                        1111 = f = 15
//                    11111111 = ff = 256





#define SCOOTD_MAX_THREADS 16

#define SCOOTD_THREAD_UTIL_BUFFER_SIZE 64


typedef struct
{
	int        idx;
	pthread_t thread_handle;
	char      *pOutBuffer;
	char      szBuffer[SCOOTD_THREAD_UTIL_BUFFER_SIZE];
	FILE            *outpipe;
	FILE            *inpipe;

	int       infd;
	int      outfd;
	pid_t      pid;	
	

	bool      bRun;
	bool      bDone;
	
	void      *pvScootDevice;

} scootd_threads;




typedef struct
{
	scoot_state *pState;
	
	int shm_fd;

	scootd_threads threads[SCOOTD_MAX_THREADS];
	
	
} scoot_device;

typedef struct
{
	int thread_index;
	scoot_device *pScootDevice;

} scootd_thread_config;

int scootd_util_open_shared_memory(char *strFileName, scoot_device *pScoot);

pthread_t scootd_util_create_thread(void * (*thread_func) (void *), scootd_thread_config *pScootThreadConfig);

int scootd_util_run_command(scootd_thread_config *pScootThread, const char * command);
int scootd_util_run_command_nonblocking(scootd_thread_config *pScootThread, const char * command);




int scootd_util_character_to_pipe(scootd_threads * pThread, char character);
int scootd_util_kill_thread(scoot_device *pScootDevice, scootd_threads	 *pThread);

int scootd_util_close_shared_memroy(scoot_device *pScoot);




#endif
