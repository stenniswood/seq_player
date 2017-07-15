#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drive_five.h"
#include "sequence_player.hpp"


void print_file( char* name )
{
	std::string line;
	std::ifstream text (name);
	if (text.is_open())
	{
		while (text.good())
		{
			getline(text,line);
			std::cout << line << std::endl;
		}
		text.close();
	}
	else
	{
		std::cout << "Unable to open file" << std::endl << std::endl;
	}
}

void save_file( char* name )
{
	std::string line;
	std::ofstream text (name);
	if (text.is_open())
	{
		while (text.good())
		{
			//getline(text,line);
			//std::cout << line << std::endl;
		}
		text.close();
	}
	else
	{
		std::cout << "Unable to open file" << std::endl << std::endl;
	}
}


void help()
{
	printf("Useage:  ./seqplay [filename] [timestep=0.1]\n");
	
}

int main(int argc, char ** argv)
{
	
	if (argc==1)
		print_file( argv[1] );
	else if (argc==2) {
	
		scan_available_boards();
		open_all_available();
		char timeStep[32];
		strcpy( timeStep, "0.001" );
		play_back( argv[1], timeStep );
	}
	else help();	
}

