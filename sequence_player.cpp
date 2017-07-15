#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "drive_five.h"
#include "sequence_player.hpp"



#define PID_TIMEOUT_MS 10000
double time_step = 1.0;	// in seconds
int line_number = 0;

void play_back( char* mFilename, char* mTimeStep )
{
	time_step = atof( mTimeStep );
	
	string line;
	std::ifstream text (mFilename);
	if (text.is_open())
	{
		while (text.good())
		{
			getline(text,line);
			// PAUSE FOR TIME STEP:	
			usleep( time_step*1000000 );		// Pause between every line of execution.  ie Playback rate.			
			execute_one_line( line );			
			line_number++;
			//std::cout << line << std::endl;
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
	// SEPARATE DEVICE NAME FROM COMMAND :
	char    device[512];
	size_t  ptr = mSequenceLine.find(' ', 0);	// First space is end of the device name.
	if (ptr==string::npos)
		return;	 
	string  command = mSequenceLine.substr( ptr+1, string::npos );	
	mSequenceLine[ptr] = 0;
	strcpy(device, mSequenceLine.c_str() );
	
	char cmd_key[] = "wait for pid";
	int selected_board = find_board( device  );
	printf("%d - %d; Cmd=%s\n", line_number, selected_board, command.c_str() );
	
	// PAUSE FOR USER REQUESTED DELAY :
	if (strstr(command.c_str(), "delay")!=NULL)	// if the command is a "delay 500 ms"
	{
		size_t delay_index = command.find(' ', 1);
		size_t    ms_index = command.find("ms", 0);
		string delay_str   = command.substr( delay_index, (ms_index-delay_index) );
		int delay_ms = atoi(delay_str.c_str());
		printf("\tdelaying...%d ms\n", delay_ms);
		sleep( delay_ms / 1000 );					// Delay!
	}
	// WAIT FOR PID axis : 
	else if (strstr(command.c_str(),cmd_key)!=NULL)	// if the command is a "delay 500 ms"
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
	} else {
		// SEND COMMAND TO FIVE:
		fives[selected_board].send_command( command.c_str() );
		// Wait for response:
		while (fives[selected_board].get_has_responded()==false) {};
	}
}
