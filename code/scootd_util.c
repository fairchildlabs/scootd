
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





SystemInfo get_system_info()
{
	FILE *			file;
	char			buffer[128];
	SystemInfo		sys_info =
	{
		0
	};

	// Get CPU utilization
	file				= fopen("/proc/stat", "r");

	if (file != NULL)
	{
		fgets(buffer, sizeof(buffer), file);
		unsigned long	user, nice, system, idle, iowait, irq, softirq;

		sscanf(buffer, "cpu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
		fclose(file);
		unsigned long	total = user + nice + system + idle + iowait + irq + softirq;
		unsigned long	total_idle = idle + iowait;

		sys_info.cpu_utilization = 100.0 * (total - total_idle) / total;
	}

	// Get memory usage
	file				= fopen("/proc/meminfo", "r");

	if (file != NULL)
	{
		unsigned long	mem_total = 0, mem_available = 0;

		while (fgets(buffer, sizeof(buffer), file))
		{
			sscanf(buffer, "MemTotal: %lu kB", &mem_total);
			sscanf(buffer, "MemAvailable: %lu kB", &mem_available);
		}

		fclose(file);
		sys_info.mem_usage_mb = (mem_total - mem_available) / 1024.0;
	}

	// Get system uptime
	file				= fopen("/proc/uptime", "r");

	if (file != NULL)
	{
		fscanf(file, "%lf", &sys_info.uptime_seconds);
		fclose(file);
	}

	return sys_info;
}



void scootd_util_sys_info(void)
{
	int 			verbose = scootd_get_verbosity(SCOOTD_DBGLVL_NONE);
	SystemInfo sysinfo;

	sysinfo = get_system_info();

	SCOOTD_PRINT(verbose, "scootd_util_sys_info() CPuUtil = %f mem = %f uptime = %f\n", sysinfo.cpu_utilization, sysinfo.mem_usage_mb, sysinfo.uptime_seconds);

	scootd_event_sysinfo(sysinfo);
	


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

int scootd_util_kill_thread(scoot_device* pScootDevice, scootd_threads* pThread)
{
	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);

	SCOOTD_PRINT(verbose, "scootd_util_kill_thread(%d) \n", pThread->idx);

	if (pThread->idx < SCOOTD_THREAD_GPS)
	{

		if (scootd_util_character_to_pipe(pThread, 'q'))
		{
			printf("ERROR - CHAR 'q' to pipe(%d)\n", pThread->idx);
			return -1;
		}
		else
		{
			printf("GOOD -  CHAR 'q' to pipe(%d)\n", pThread->idx);
		}

		printf("Sending SIGTERM\n");
		kill(pThread->pid, SIGTERM);
		printf("Sending SIGTERM kill done\n");
		waitpid(pThread->pid, NULL, 0);
		printf("Done WAIT PID(%d)\n", pThread->pid);
		pclose2(pThread->pid);
		printf("Done pclose2\n");
		pThread->pid = (-1);

	}

	pThread->bRun = false;

	usleep(10);

	while (false == pThread->bDone)
	{
		usleep(10);
	}

	printf("bDone Thread (%d)\n", pThread->idx);
	//CLEANUP 
	if (pThread->outfd)
	{
		close(pThread->outfd);
		pThread->outfd = 0;
	}
	if (pThread->infd)
	{
		close(pThread->infd);
		pThread->infd = 0;
	}


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





int scootd_GPS_setupSerial(const char * device)
{
	int 			fd	= open(device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (fd == -1)
	{
		perror("open_port: Unable to open device");
		return - 1;
	}

	struct termios tty;
	tcgetattr(fd, &tty);

	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);

	tty.c_cflag &= ~PARENB; // No parity bit
	tty.c_cflag &= ~CSTOPB; // One stop bit
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8; // 8 bits per byte
	tty.c_cflag &= ~CRTSCTS; // No flow control
	tty.c_cflag |= CREAD | CLOCAL; // Read and enable receiver

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;
	tty.c_lflag &= ~ECHOE;
	tty.c_lflag &= ~ECHONL;
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No flow control
	tty.c_iflag &= ~(ICRNL | INLCR); // Disable carriage return and newline conversion

	tty.c_oflag &= ~OPOST; // Disable output processing

	tcsetattr(fd, TCSANOW, &tty);

	return fd;
}


void scootd_dump_gps_data(GPSData gpsData)
{
	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);

	SCOOTD_PRINT(verbose, "GPS: Lat: %f Long: %f altitude: %f ground_speed %f\n", gpsData.latitude, gpsData.longitude, gpsData.altitude, gpsData.ground_speed);
}


double convert_to_decimal_degrees(const char * nmea_pos, char quadrant)
{

	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);

	double			raw = atof(nmea_pos);
	int 			degrees = (int) (raw / 100);
	double			minutes = raw - (degrees * 100);
	double			decimal = degrees + (minutes / 60);
	double          result;
		

	result = (quadrant == 'S' || quadrant == 'W') ? -decimal: decimal;

	SCOOTD_PRINT(verbose, "convert_to_decimal_degrees(%s, %c) raw=%f degrees=%d minutes=%f decimal = %f result = %f\n", nmea_pos, quadrant, raw, degrees, minutes, decimal, result);

	return result;
}



//via copilot 



GPSData scootd_parse_gps_data(const char * nmea_sentence)
{
	int 			verbose = scootd_get_verbosity(SCOOTD_DBGLVL_NONE);

	SCOOTD_PRINT(verbose, "++++++++++++++++++++++++++++++++++++++++++(%d)\n", 0);
	SCOOTD_PRINT(verbose, "scootd_parse_gps_data() nmea_sentence = [(%s)]\n", nmea_sentence);
	SCOOTD_PRINT(verbose, "------------------------------------------(%d)\n\n", 0);

	int 			field_count;
	GPSData 		data =
	{
		0
	};
	char *			gnrmc = strstr(nmea_sentence, "$GNRMC");
	char *			gngga = strstr(nmea_sentence, "$GNGGA");

	if (gnrmc)
	{
		char *			token = strtok(gnrmc, ",");

		field_count 		= 1;

		while (token != NULL)
		{
			if (field_count == 4)
			{
				char *			lat_token = token;
				char *			lat_dir_token = strtok(NULL, ",");

				data.latitude		= convert_to_decimal_degrees(lat_token, *lat_dir_token);
				SCOOTD_PRINT(verbose, "scootd_parse_gps_data(%s) lat = %f\n", lat_token, data.latitude);
				field_count++;
			}
			else if (field_count == 6)
			{
				char *			lon_token = token;
				char *			lon_dir_token = strtok(NULL, ",");

				data.longitude		= convert_to_decimal_degrees(lon_token, *lon_dir_token);
				SCOOTD_PRINT(verbose, "scootd_parse_gps_data(%s) long = %f\n", lon_token, data.longitude);
				field_count++;
			}
			else if (field_count == 8)
			{
				data.ground_speed	= atof(token) * 1.852; // Convert knots to km/h
				SCOOTD_PRINT(verbose, "scootd_parse_gps_data(%s) ground_speed = %f\n", token, data.ground_speed);
				field_count++;
			}

			field_count++;

			if (field_count > 8)
			{
				break;
			}

			token				= strtok(NULL, ",");
		}
	}

	if (gngga)
	{
		char *			token = strtok(gngga, ",");
		int 			field_count = 0;

		while (token != NULL)
		{
			field_count++;

			if (field_count == 10)
			{
				data.altitude		= atof(token);
				SCOOTD_PRINT(verbose, "scootd_parse_gps_data() altitude = %f\n", data.altitude);
				break;
			}

			SCOOTD_PRINT(verbose, "gngga(%d):%s\n", field_count, token);
			token				= strtok(NULL, ",");
		}
	}

	return data;
}


double get_time_in_fseconds()
{
	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);
	struct timeval tv;
	double fsec;
	gettimeofday(&tv, NULL);

	fsec = (double)tv.tv_sec + (double)(tv.tv_usec / 1000000.0);
	
	SCOOTD_PRINT(verbose, "get_time_in_fseconds(%d:%d) fsec = %f\n", tv.tv_sec, tv.tv_usec, fsec);

	
	return fsec;
}
void scootd_log_event(int ecode, float f1, float f2, float f3, float f4, int i1, int i2, int i3, int i4, char *s1, char* s2)
{
	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);
	double fsec = get_time_in_fseconds();
	
	SCOOTD_PRINT(verbose, "LOG_EVENT(%p): %d, %f, %f, %f, %f, %f, %d, %d, %d, %d, '%s','%s'\n", gEvtLogFd, ecode, fsec, f1, f2, f3, f4, i1, i2, i3, i4, s1, s2);

	if (gEvtLogFd)
	{
		fprintf(gEvtLogFd, "%d, %f, %f, %f, %f, %f, %d, %d, %d, %d, '%s', '%s'\n", ecode, fsec, f1, f2, f3, f4, i1, i2, i3, i4, s1, s2);

		fflush(gEvtLogFd);

	}


}
void scootd_event_gps(GPSData gpsData)
{
	scootd_log_event(EVT_SCOOTD_GPS, gpsData.latitude, gpsData.longitude, gpsData.altitude, gpsData.ground_speed, 0, 0, 0, 0, NULL, NULL);

}

void scootd_event_state_change(unsigned int old_state, unsigned int new_state)
{
	scootd_log_event(EVT_SCOOTD_STATE_CHANGE, 0.0, 0.0, 0.0, 0.0, old_state, new_state, 0, 0, NULL, NULL);
}

void scootd_event_video(int video_device, int fr, int res, int raw, char* fn, char* cmdbuf)
{
	scootd_log_event(EVT_SCOOTD_VIDEO, 0.0, 0.0, 0.0, 0.0, video_device, fr, res, raw, fn, cmdbuf);
}

void scootd_event_sysinfo(SystemInfo sysinfo)
{
	scootd_log_event(EVT_SCOOTD_SYSINFO, sysinfo.cpu_utilization, sysinfo.mem_usage_mb, sysinfo.uptime_seconds, 0.0, 0, 0, 0, 0, NULL, NULL);
}
	



