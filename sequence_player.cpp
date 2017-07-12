#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "drive_five.h"

#define PID_TIMEOUT_MS 10000
long int time_step = 0.01;

void play_back( char* mFilename, char* mTimeStep )
{
	time_step = atoi( mTimeStep );
	
	string line;
	std::ifstream text (mFilename);
	if (text.is_open())
	{
		while (text.good())
		{
			getline(text,line);
			execute_one_line( line.c_str() )
			std::cout << line << std::endl;
		}
		text.close();
	}
	else
	{
		std::cout << "Unable to open file" << std::endl << std::endl;
	}			
}


void execute_one_line( std::string mSequenceLine )
{
	// SEND COMMAND TO FIVE:
	char device[512];
	char* ptr = mSequenceLine.find(' ', 0);
	
	char* ptr = strchr(mSequenceLine.c_str(), ' ');
	char* command = (ptr+1);
	*ptr = 0;
	strcpy(device, mSequenceLine );

	int selected_board = find_board   ( device  );
	fives[selected_board].send_command( command );

	// PAUSE FOR TIME STEP:	
	usleep( time_step*1000 );		// Pause between every line of execution.  ie Playback rate.

	// PAUSE FOR USER REQUESTED DELAY :
	if (strstr(command,"delay")!=NULL)	// if the command is a "delay 500 ms"
	{
		char* delay_ptr = strchr(command, ' ')+1;
		char* ms_ptr    = strchr(delay_ptr, ' ');
		*ms_ptr = 0;
		int delay_ms = atoi(delay_ptr);
		usleep( delay_ms * 1000 );					// Delay!
	}

	// WAIT FOR PID axis:
	char cmd_key[] = "wait for pid";
	if (strstr(command,cmd_key)!=NULL)	// if the command is a "delay 500 ms"
	{
		char* delay_ptr = command+strlen(cmd_key);		// search after 'Wait PID' for v w x y z

		long int timeout = 0;
		while (fives[selected_board].is_pid_done( 'v' )==false )
		{
			usleep(1000);
			// nothing
			timeout++;
			if (timeout > PID_TIMEOUT_MS)
				break;
		}
	}
}
