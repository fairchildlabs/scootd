


#include "scootd.h"


int main(int argc, char **argv)
{
	unsigned int prev_val = 0;
	unsigned int new_val;
	scoot_device aScootDevice;


	if(argc > 1)
	{
		new_val = (unsigned int)strtol(argv[1], NULL, 0);
	}
	
	if(scootd_util_open_shared_memory("/scootd_shared.mem", &aScootDevice))
	{
			printf("Error opening shared memory\n");
	}
	else
	{
		prev_val = aScootDevice.pState->state;
	
		printf("scootdtest previous value set 0x%08x value = 0x%08x\n", prev_val, new_val);
		aScootDevice.pState->state = new_val;
		
	}

	return 0;
}
