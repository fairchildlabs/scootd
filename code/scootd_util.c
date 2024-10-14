
#include <scootd.h>



//Used AI to generate example code I used.
//https://copilot.microsoft.com/sl/iEEDiJEa6PQ
int scootd_util_open_shared_memory(char * strFileName, scoot_device * pScoot)
{

	// Create shared memory ob	ject
	pScoot->shm_fd		= shm_open(strFileName, O_CREAT | O_RDWR, 0666);

	if (pScoot->shm_fd == -1)
	{
		perror("shm_open");
		return 1;
	}

	// Set the size of shared memory
	if (ftruncate(pScoot->shm_fd, sizeof(scoot_device)) == -1)
	{
		perror("ftruncate");
		return 1;
	}

	// Map the shared memory object into the process address space
	pScoot->pState		= mmap(0, sizeof(scoot_state), PROT_READ | PROT_WRITE, MAP_SHARED, pScoot->shm_fd, 0);

	if (pScoot->pState == MAP_FAILED)
	{
		perror("mmap");
		return 1;
	}

	return 0;


}


int scootd_util_close_shared_memroy(scoot_device *pScoot)
{
	close(pScoot->shm_fd);
	return 0;
}



//AI: https://copilot.microsoft.com/sl/huDnvWgVZZY

pthread_t scootd_util_create_thread(void * (*thread_func) (void *), scootd_threads *pThread)
{
	pthread_t		thread;
	int 			result;
	scoot_device *pScootDevice;
	int idx = pThread->idx;

	pScootDevice = pThread->pvScootDevice;
	pThread = &pScootDevice->threads[idx]; 
	pThread->bRun = true;
	// Create the thread
	result				= pthread_create(&thread, NULL, thread_func, pThread);

	if (result != 0)
	{
		fprintf(stderr, "Error creating thread\n");
		exit(EXIT_FAILURE);
	}
	else
	{	
		pThread->thread_handle = thread;
		printf("Running Thread %p\n", thread);
	}
	

	return thread;
}

//I found this here : https://stackoverflow.com/questions/548063/kill-a-process-started-with-popen

#define READ 0
#define WRITE 1
pid_t popen2(const char * command, int * infp, int * outfp)
{
	int 			p_stdin[2], p_stdout[2];
	pid_t			pid;

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
		return - 1;

	pid 				= fork();

	if (pid < 0)
	{
		return pid;
	}
	else if (pid == 0)
	{
		dup2(p_stdin[READ], STDIN_FILENO);
		dup2(p_stdout[WRITE], STDOUT_FILENO);

		//close unuse descriptors on child process.
		close(p_stdin[READ]);
		close(p_stdin[WRITE]);
		close(p_stdout[READ]);
		close(p_stdout[WRITE]);

		//can change to any exec* function family.
		execl("/bin/bash", "bash", "-c", command, NULL);
		perror("execl");
		exit(1);
	}

	// close unused descriptors on parent process.
	close(p_stdin[READ]);
	close(p_stdout[WRITE]);

	if (infp == NULL)
		close(p_stdin[WRITE]);
	else 
		* infp = p_stdin[WRITE];

	if (outfp == NULL)
		close(p_stdout[READ]);
	else 
		* outfp = p_stdout[READ];

	return pid;
}


int pclose2(pid_t pid) 
{
    int internal_stat;
    waitpid(pid, &internal_stat, 0);
    return WEXITSTATUS(internal_stat);
}

//AI: https://copilot.microsoft.com/sl/jS6aPunFSKa

int scootd_util_kill_thread(scoot_device *pScootDevice, scootd_threads	 *pThread)
{
	printf("scootd_util_kill_thread(%d) \n", pThread->idx);

	if(scootd_util_character_to_pipe(pThread, 'q'))
	{
		printf("ERROR - CHAR 'q' to pipe(%d)\n", pThread->idx);
		return -1;
	}
	else
	{
		printf("GOOD -  CHAR 'q' to pipe(%d)\n", pThread->idx);
	}

	pThread->bRun = false;
	printf("Sending SIGTERM\n");
	kill(pThread->pid, SIGTERM);
	printf("Sending SIGTERM kill done\n");
	waitpid(pThread->pid, NULL, 0);
	printf("Done WAIT PID(%d)\n", pThread->pid);
	pclose2(pThread->pid);
	printf("Done pclose2\n");
	pThread->pid = (-1);


	while(false == pThread->bDone)	
	{
		usleep(10);
	}

	printf("bDone Thread (%d)\n", pThread->idx);
	//CLEANUP 

	close(pThread->outfd);
	close(pThread->infd);
	


	pThread->bRun = false;
	pThread->bDone = false;

	return 0;
}




int scootd_util_run_command_nonblocking(scootd_threads *pThread, const char * command)
{
	scoot_device *pScootDevice;
	FILE			*pipe;
	char			*buffer  ;
	char *			result = NULL; 
	size_t			result_size = 0;
	int idx = pThread->idx;
	int count = 0;
	const int       usSelectTimeout = 500000;	
	pid_t pid;	


	pScootDevice = pThread->pvScootDevice;

	pThread = &pScootDevice->threads[idx]; 

	printf("scootd_util_run_command_nonblocking(%d) POPEN usSelectTimeout = %d (%s) pThread=%p\n", idx, usSelectTimeout, command, pThread);

	pid = popen2(command, &pThread->infd, &pThread->outfd);

	
	pThread->outpipe  = fdopen(pThread->outfd, "r");
	pThread->inpipe = fdopen(pThread->infd, "w");

	printf("POPEN PID = %d INFD = %d OUTFD %d (%p:%p)\n", pid, pThread->infd, pThread->outfd, pThread->outpipe, pThread->inpipe);

	pThread->pid = pid;
	
	buffer = pThread->szBuffer;
	result = pThread->pOutBuffer;
	pipe = pThread->outpipe;

	if (!pipe)
	{
		fprintf(stderr, "popen() failed!\n");
		return -1;
	}

	pThread->pid = pid;

	printf("pThread(%p) PID = %d\n", pThread, pid);
	
	
	while (true == pThread->bRun) 
	{
		
		usleep(usSelectTimeout);
		//printf("scootd_util_run_command_nonblocking() usSelectTimeout = %d\n", usSelectTimeout);

	}

	if(false == pThread->bRun)
	{
		printf("Non-blocking bRun == false\n");
	}

	pThread->bDone = true;
	
	return 0;
}




int scootd_util_character_to_pipe(scootd_threads * pThread, char character)
{
	FILE *			pipe;
	pipe				= pThread->inpipe;

	printf("scootd_util_character_to_pipe(%d)(%p:%p) char = %c\n", pThread->idx, pThread, pipe, character);

	if (pipe == NULL)
	{
		printf("Invalid pipe!\n");
		return - 2;
	}

	if (fputc(character, pipe) == EOF)
	{
		printf("Failed to write character to pipe!\n");
		return - 1;
	}
	else
	{
		printf("CHAR %c sent\n", character);
	}

	printf("scootd_util_character_to_pipe(%p) RETURN char = %c\n", pipe, character);

	return 0;
}




