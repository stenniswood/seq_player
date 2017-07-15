#include <sys/time.h>
#include <stdlib.h>
//#include <linux/serial.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include "global.h"
#include "drive_five.h"
#include <signal.h>
#include <vector>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

vector<DriveFive> fives;

#define MAXRETRY 2
#define SetDWORDval(arg) (uint8_t)(((uint32_t)arg)>>24),(uint8_t)(((uint32_t)arg)>>16),(uint8_t)(((uint32_t)arg)>>8),(uint8_t)arg
#define SetWORDval(arg)  (uint8_t)(((uint16_t)arg)>>8),(uint8_t)arg

#define Debug 1

uint64_t GetTimeStamp2() 
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    uint64_t t = tv.tv_sec + tv.tv_usec*(uint64_t)1000000;
    //printf("GetTimeStamp()  %ld  %ld  = %ld\n", tv.tv_sec, tv.tv_usec, t );
    return t;
}
extern uint64_t 	   GetTimeStamp2();


using namespace std;
vector<string> DriveFive_device_paths;
vector<string> DriveFive_device_names;
void populate_file_in_directory(char* mPath,
								vector<string> &filelist_paths,
								vector<string> &filelist_names )
{
	DIR           *d;
	struct dirent *dir;
	d 	= opendir(mPath);
	if (d==NULL)
		perror("\n\n opendir() error: ");
	else 
	{
		// go thru all FILES:
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type==DT_CHR)		// Character Device
			{			
				bool match = (strstr(dir->d_name, "ttyU")!=NULL);
				if (match) {
					printf("FOUND: %s\n", dir->d_name );
					DriveFive_device_paths.push_back( mPath );
					DriveFive_device_names.push_back( dir->d_name );
				}
			}        
		}
		closedir(d);
	}
}


/* File directory on /dev/ttyUSB* */
void scan_available_boards()
{
	DriveFive_device_paths.clear();
	DriveFive_device_names.clear();		
	//printf("Searching for Five boards...\n\n");
	populate_file_in_directory("/dev/", DriveFive_device_paths, DriveFive_device_names );
	printf("Found %ld Drive Five boards.\n", DriveFive_device_names.size() );		
}

void open_all_available()
{
	DriveFive one;
	
	for (int i=0; i<DriveFive_device_names.size(); i++)
	{				
		string fulldevname = DriveFive_device_paths[i] + DriveFive_device_names[i];
		fives.push_back(one);
		fives[i].open( fulldevname.c_str() );
		fives[i].set_baud(38400);
	}
}

int find_board( char* mDeviceName )
{
	for (int i=0; i<fives.size(); i++)
	{
		char* match = strstr(fives[i].m_port_name, mDeviceName );
		if (match!=NULL)
			return i;		
	}	
	return -1;
}

void close_all_fives()
{
	for (int i=0; i<DriveFive_device_names.size(); i++)
	{
		fives[i].close();				
	}	
	fives.clear();
	DriveFive_device_names.clear();
	DriveFive_device_paths.clear();
}

/************************/
/****	Constructor: ****/
/************************/
DriveFive::DriveFive( )
{
	Initialize();
}

// 	mDeviceName : for instance "/dev/ttyUSB0"	
DriveFive::DriveFive( const char* mDeviceName)
{
	Initialize();
	strcpy (m_port_name, mDeviceName );	
}

DriveFive::DriveFive( const char* mDeviceName, uint32_t time_out )
{
	Initialize();
	strcpy (m_port_name, mDeviceName );	
	timeout = time_out;
}

void* serial_interface(void* object)
{
	DriveFive* df = (DriveFive*)object;
	while(1)
	{
		// Read
		// Append to RxBuffer
		int c = ::read( df->fd, (void*)&(df->rx_buffer[df->rx_bytes]), 500 );
		df->rx_bytes += c;
		printf("\nRxBuffer=%s\n", &(df->rx_buffer[df->rx_bytes]) );		
	}
}


void DriveFive::Initialize()
{
	for (int i=0; i<5; i++)
		pid_done[0] = TRUE;					// We say True because initially no PIDS are running. (semantics)

	fd=0;
	rx_bytes=0;		 			// rx data bytes in buffer
 	memset( rx_buffer, 0, RX_BUFFER_SIZE );
 	memset( tx_buffer, 0, TX_BUFFER_SIZE );
	memset( m_port_name, 0, PORT_NAME_SIZE );
	tx_bytes  = 0;
	connected = false;
	//struct 		pollfd 	serial_poll;

	// Create a thread for reading.  It constantly listens and appends to it's response.
	// Until a new command is issued.	
	int thread_result = pthread_create( &read_thread_id, NULL,
										serial_interface, (void*)this);
	if (thread_result)
	{
		fprintf( stderr, "Error - Could not create right_foot thread. return code: %d\n", thread_result );
		exit(EXIT_FAILURE);
	}							
}
	
//
// Destructor
//
DriveFive::~DriveFive()
{
}

void DriveFive::set_device_name	( const char* mDeviceName )
{
	if (mDeviceName)
		strcpy (m_port_name, mDeviceName );
}

void DriveFive::set_baud(int speed)
{
	//_cl_baud = speed;
	//printf("Baudrate = %d\n", speed);
	struct termios options;
	tcgetattr  ( fd, 	   		&options);
	cfsetispeed( &options, 		speed );
	cfsetospeed( &options, 		speed );
	tcsetattr  ( fd, TCSANOW, 	&options);
}

void DriveFive::open(const char* mDeviceName)
{
	if (mDeviceName)
		strcpy(m_port_name, mDeviceName );

	fd = ::open( m_port_name, O_RDWR  ); 		// | O_NONBLOCK		
//	fd = ::fopen( m_port_name, "r+"  ); 		// | O_NONBLOCK
	if (fd < 0)
	{
		printf("Unable to open serial device: %s - %s\n", m_port_name, strerror(errno) );
    	return ;
	}	

	connected = true;
	serial_poll.fd = fd;
	serial_poll.events  |= POLLIN;
	serial_poll.revents |= POLLIN;	
	printf("DriveFive : opened port=%s\n", m_port_name );	
}

char DriveFive::serialGetchar()
{
	if (!connected) {
		printf( "Getchar()  not connected\n");		//return -1;
		return 0;
	}

	int c;	
	char      buff[3];
	const long int LOOPS = 600000;
	long int count=0;
	
	if (available()) {
//			c = ::fread( fd, (void*)&buff, 1 );
			c = ::read( fd, (void*)&buff, 1 );
			printf("%c", buff[0]);		
			return buff[0];
	} else {
			printf("no data\n");
			return 0;
	}
		
	if (count>=LOOPS) {
		buff[0] = -1;
		printf("timeout!\n");
		return 0;
	}
}

int DriveFive::available()
{
	int retval = poll( &serial_poll, 1, 5 );	// 20 ms timeout
	if (serial_poll.revents & POLLIN)
		return TRUE;
	return FALSE;
}

void DriveFive::clear()
{
	while(available())
		serialGetchar();
}

/* NOTE: Instead of all this,
		Just make 1 function which sends whatever telegram you want!
		Don't need separate member functions for each command!
*/
bool DriveFive::send_command( const char* mFiveCommand) 
{
	// Clear Response buffer (start over)
	restart_response();
	
	tx_bytes = strlen(mFiveCommand);	
	size_t retval = ::write( fd, mFiveCommand, tx_bytes );	// if this blocks, we are okay.	
	if (retval == -1)
		perror("Error - send_command() ");

	// Send the Deliminator!
	retval = ::write( fd, (char*)"\r\n", 2 );	// if this blocks, we are okay.	
	if (retval == -1)
		perror("Error - send_command() ");

	printf("send_command() done!\n");	
	return retval>0;
}

bool DriveFive::is_pid_done( char Letter )
{	
	return pid_done[Letter];
}

bool DriveFive::contains_NAK( )
{
	bool isNAK = strstr(m_response, "NAK:") != NULL;	
	return isNAK;
}

void DriveFive::restart_response( )
{
	memset(m_response, 0, sizeof(m_response) );
	rx_bytes = 0;
}

/* Would like this to continually receive data, adding to the rx buffer.
		and after a slight (~20ms) timeout, then we add the response to the GUI.
	So we need either a timeslice (with or without a thread), or interrupts for Rxchar
	
*/
bool DriveFive::read_response() 
{
	restart_response();
	
	// blah get from comm port, don't know how - interrupt driven or blocking?	
	while (!available()) { };
	
	do {
	//	size_t retval = ::read(fd, m_reponse, 255);	// if this blocks, we are okay.	
		rx_bytes = ::read(fd, m_response, 2048);	// if this blocks, we are okay.	
		printf("read_reponse (%d bytes) = %s\n", rx_bytes, m_response);
		if (rx_bytes==-1)
			perror("Error - read_response() ");
		usleep(100000);
	} while (available());	// b/c it may not all come at once
	
	return rx_bytes>0;
}

void DriveFive::close() 
{
	::close(fd);
}

char* get_error_string( uint32_t mStatus )
{
	static char Text[255];
	memset(Text, 0, 254);
	if (mStatus == 0x0000) 	{
		strcat(Text, "Normal Working");
		return Text;
	}
	if (mStatus & 0x0001) 	strcat(Text, "M1 OverCurrent Warning;");
	if (mStatus & 0x0002) 	strcat(Text, "M2 OverCurrent Warning;");
	if (mStatus & 0x0004) 	strcat(Text, "E-Stop;");
	if (mStatus & 0x0008) 	strcat(Text, "Temperature Error;");
	
	if (mStatus & 0x0010) 	strcat(Text, "Temperature2 Error;");
	if (mStatus & 0x0020) 	strcat(Text, "Main Battery High Error;");
	if (mStatus & 0x0040) 	strcat(Text, "Logic Battery High Error;");
	if (mStatus & 0x0080) 	strcat(Text, "Logic Battery Low Error;");
	
	if (mStatus & 0x0100) 	strcat(Text, "M1 Driver Fault;");
	if (mStatus & 0x0200) 	strcat(Text, "M2 Driver Fault;");
	if (mStatus & 0x0400) 	strcat(Text, "Main Battery High Warning;");
	if (mStatus & 0x0800) 	strcat(Text, "Main Battery Low Warning;");
	
	if (mStatus & 0x1000) 	strcat(Text, "Temperature Warning;");
	if (mStatus & 0x2000) 	strcat(Text, "Temperature2 Warning;");
	if (mStatus & 0x4000) 	strcat(Text, "M1 Home;");
	if (mStatus & 0x8000) 	strcat(Text, "M2 Home;");

	return Text;
}
