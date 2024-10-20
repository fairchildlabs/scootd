#include <scootd.h>

/******************************************************/
/********************* Globals ************************/
/******************************************************/


FILE *gDbgLogFd = NULL;

/******************************************************/
/********************* Functions ************************/
/******************************************************/

void * dummy_thread( void * pvScootDevice)
{
	int count = 0;

	while(1)
	{	
		printf("dummy_thread(%d)\n", count);
		count++;
		sleep(1);
	}

	return NULL;

}


char *szUSBCamResolution [] =
{
	"640x480",      //0
	"800x600",      //1
	"1280x720",     //2
	"1920x1080",    //3
	"2560x1440",    //4
	"2688x1520",    //5
	"3264x2448"     //6 
	
};

int iFrameRateCam[] =
{
	30,
	60
};

//ffmpeg -f v4l2 -framerate 30 -video_size 1920x1080 -i /dev/video0 /var/www/html/video_13/001727636398_00000301_1920x1080.mp4


//ffmpeg -f v4l2 -framerate 60 -video_size 1280x720 -input_format mjpeg -i /dev/video0 -preset faster -pix_fmt yuv420p /var/www/html/video_13/1out.mkv
//ffmpeg -f v4l2 -framerate 60 -video_size 1920x1080 -input_format mjpeg -i /dev/video0 -preset faster -pix_fmt yuv420p /var/www/html/video_13/13out.mkv

//sudo mount -t auto -v /dev/sda1 /var/www/html/usb

//Library to do raw capture from C. https://libuvc.github.io/libuvc/
//ffmpeg -f video4linux2 -list_formats all -i /dev/video0

//https://stackoverflow.com/questions/44960632/ffmpeg-records-5-frames-per-second-on-a-device-that-cheese-records-at-20-fps

//char *szBaseVideoPath = "/media/astros/F0C1-F9A3/";
//char *szBaseVideoPath = "/var/www/html/video_13/";

//cat /proc/cpuinfo | grep --ignore-case serial
/*
0 : imx708_wide [4608x2592 10-bit RGGB] (/base/axi/pcie@120000/rp1/i2c@88000/imx708@1a)
    Modes: 'SRGGB10_CSI2P' : 1536x864 [120.13 fps - (768, 432)/3072x1728 crop]
                             2304x1296 [56.03 fps - (0, 0)/4608x2592 crop]
                             4608x2592 [14.35 fps - (0, 0)/4608x2592 crop]
*/

//ffmpeg -f v4l2 -framerate 30 -video_size 800x600 -i /dev/video0 -preset faster -pix_fmt yuv420p out.mp4

//ffmpeg -f video4linux2 -input_format h264 -video_size 800x600 -framerate 30 -i /dev/video0 -vcodec h264 -preset faster -pix_fmt yuv420p out.mp4

//ffprobe -v error -select_streams v:0 -show_entries stream=avg_frame_rate -of default=noprint_wrappers=1:nokey=1 


char *szBaseVideoPath = "/mnt/";

void * videoX_usb_run(void * pvThread)
{
	char fn[128];
	char cmdbuf[512];
	scootd_threads *pThread = pvThread;
	scoot_device *pScootDevice = pThread->pvScootDevice;
	int idx = pThread->idx;
		
	int fr = iFrameRateCam[pScootDevice->pState->vid[idx].frame_rate];	
	char *szRes = szUSBCamResolution[pScootDevice->pState->vid[idx].resolution];
	int raw = pScootDevice->pState->vid[idx].raw;
	int video_device = 0;	
	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);
	char *ffmt = "mp4";	
	if(1 == idx)
	{
		video_device = 2;
	}

	if(raw)
	{
		ffmt = "mov";
	}

	
	sprintf(fn, "%s00%10d_%d_%08x_%s.%s", szBaseVideoPath, time(NULL), video_device, pScootDevice->pState->state, szRes, ffmt);

	if(raw)
	{
		sprintf(cmdbuf, "ffmpeg -f v4l2 -framerate %d -video_size %s -c:v mjpeg -i /dev/video%d  -c:v copy %s", fr, szRes, video_device, fn);
	}
	else
	{
		sprintf(cmdbuf, "ffmpeg -f v4l2 -framerate %d -video_size %s -i /dev/video%d -preset faster -pix_fmt yuv420p %s", fr, szRes, video_device, fn);
	}


	SCOOTD_PRINT(verbose, "SENDING CMD> %s\n", cmdbuf);

	scootd_util_run_command_nonblocking(pThread, cmdbuf);
	return NULL;

}





void scootd_start_log_file(char *logfn)
{
	gDbgLogFd = fopen(logfn, "a");

	if(gDbgLogFd)
	{
		SCOOTD_PRINT(SCOOTD_DBGLVL_ERROR, "**OPEN LOG(%s)%p**************************************************\n", logfn, gDbgLogFd);	

	}
	else
	{
		printf("CAN NOT OPEN %s [%d] %s\n", logfn, errno, strerror(errno));
	}
}	

void scootd_close_log_file(void)
{
	if(gDbgLogFd)
	{
		fclose(gDbgLogFd);
	}
			
}	



void scootd_state_change(unsigned int old_state, scoot_device *	pScootDevice)
{
	scoot_state *	pOldState = (scoot_state *) &old_state;
	scootd_threads * pThread;
	int 			i;
	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);

	SCOOTD_PRINT(verbose, "scootd_state_change = 0x%08x  old_state = 0x%08x\n", pScootDevice->pState->state, pOldState->state);

	for (i = 0; i < SCOOTD_MAX_VIDEO; i++)
	{
		unsigned char vid = *(unsigned char *)(&pOldState->vid[i]);

		SCOOTD_PRINT(verbose, "VID[%d] %p = 0x%02x\n", i, &pOldState->vid[i], (int)vid);
		

		if (pOldState->vid[i].video)
		{
			pThread = &pScootDevice->threads[SCOOTD_THREAD_VIDEO_0 + i];

			if (pThread->thread_handle)
			{
				scootd_util_kill_thread(pScootDevice, pThread);
			}
		}

		if (pScootDevice->pState->vid[i].video)
		{
			SCOOTD_PRINT(verbose, "VIDEO[%d] SET\n", i);
			
			pThread = &pScootDevice->threads[SCOOTD_THREAD_VIDEO_0 + i];
		
			pThread->thread_handle = scootd_util_create_thread(videoX_usb_run, pThread);
		}

	}

}



int main(int argc, char **argv)
{
	unsigned int old_state = 0;
	scoot_device aScootDevice;
	int i;
	time_t t;
	struct tm *tmp;
	char formatted_time[50];
	int verbose = scootd_get_verbosity(SCOOTD_DBGLVL_ERROR);
	
	scootd_start_log_file("scootdx.log");
	
	

	//initialize memory zero
	memset(&aScootDevice, 0, sizeof(scoot_device));
	for(i = 0; i < SCOOTD_MAX_THREADS; i++)
	{
		aScootDevice.threads[i].idx = i;
		aScootDevice.threads[i].pvScootDevice = &aScootDevice;
		
	}


	SCOOTD_PRINT(verbose, "scootd(argc = %d)\n", argc);


	if(	scootd_util_open_shared_memory("scootd_shared.mem", &aScootDevice))
	{
		SCOOTD_PRINT(SCOOTD_DBGLVL_ERROR, "Error opening shared memory(%d)\n", 0);
	}
	else
	{
		while(1)
		{
			if(old_state != aScootDevice.pState->state)
			{
				usleep(10);
				time(&t);
    			tmp = localtime(&t);
				strftime(formatted_time, sizeof(formatted_time), "%a %b %d %H:%M:%S %Y", tmp);
				
				SCOOTD_PRINT(verbose, "SCOOTD:State Change old_state = 0x%08x new_state = 0x%08x @ %s \n", old_state, aScootDevice.pState->state, formatted_time);

				scootd_state_change(old_state, &aScootDevice);

				SCOOTD_PRINT(verbose, "SCOOTD: State Return state = %08x\n", aScootDevice.pState->state);	

				old_state = aScootDevice.pState->state;

				usleep(1000);
			}
			else
			{
			//	printf("SCOOTD: NO Change\n");
			}
			
			usleep(1000);
		}
		scootd_util_close_shared_memroy(&aScootDevice);
		
	}

	scootd_close_log_file();

	return 0;
}


