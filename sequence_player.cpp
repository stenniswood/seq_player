#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "drive_five.h"
#include "sequence_player.hpp"



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
			execute_one_line( line );
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
	char    device[512];
	size_t  ptr = mSequenceLine.find(' ', 0);	// First space is end of the device name.
	string  command = mSequenceLine.substr( ptr, string::npos );
	
	mSequenceLine[ptr] = 0;
	strcpy(device, mSequenceLine.c_str() );

	int selected_board = find_board   ( device  );
	fives[selected_board].send_command( command.c_str() );

	// PAUSE FOR TIME STEP:	
	usleep( time_step*1000 );		// Pause between every line of execution.  ie Playback rate.

	// PAUSE FOR USER REQUESTED DELAY :
	if (strstr(command.c_str(), "delay")!=NULL)	// if the command is a "delay 500 ms"
	{
		size_t delay_index = command.find(' ', 0);
		size_t    ms_index = command.find("ms", 0);
		string delay_str   = command.substr( delay_index, (ms_index-delay_index) );
		int delay_ms = atoi(delay_str.c_str());
		usleep( delay_ms * 1000 );					// Delay!
	}

	// WAIT FOR PID axis:
	char cmd_key[] = "wait for pid";
	if (strstr(command.c_str(),cmd_key)!=NULL)	// if the command is a "delay 500 ms"
	{
//		size_t delay_ptr = command+strlen(cmd_key);		// search after 'Wait PID' for v w x y z

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
