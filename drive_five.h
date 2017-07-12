#ifndef _DriveFive_h
#define _DriveFive_h

#include <stdarg.h>
#include <inttypes.h>
#include <cstddef>
#include <poll.h>
#include <vector>
#include <string>


#define TX_BUFFER_SIZE 100
#define RX_BUFFER_SIZE 100


/******************************************************************************
* Definitions
******************************************************************************/
char* get_error_string( uint32_t mStatus );

#define PORT_NAME_SIZE 256
#define _SS_VERSION 16

using namespace std;

extern vector<string> DriveFive_device_paths;
extern vector<string> DriveFive_device_names;

void scan_available_boards();
void open_all_available();
void close_all_fives();
int find_board( char* mDeviceName );
void* serial_interface(void* object);


class DriveFive 
{
	uint16_t crc;
	uint32_t timeout;
	friend void* serial_interface(void* object);
	friend int find_board( char* mDeviceName );
	
public:
	// public methods
	DriveFive( );
	DriveFive( const char* mDeviceName );
	DriveFive( const char* mDeviceName, uint32_t time_out );
	~DriveFive();
	void 	Initialize();

	void	set_baud		( int speed );
	
	void	set_device_name	( const char* mDeviceName );
	void	open			( const char* mDeviceName=NULL );	
	void 	close			( );
	
	int 	available		( );
	char	serialGetchar	( );
	bool 	send_command	( char* mFiveCommand  );
	bool 	read_response	(  );
	void 	clear			( );
	void 	restart_response( );
	bool 	contains_NAK    ( );

	bool 	is_pid_done		( char Letter );
//	void* 	serial_interface( void* );
	
private:
	char	 m_response[2048];
	char	 m_port_name[PORT_NAME_SIZE];
	int  		fd;

	bool		pid_done[5];
	pthread_t 	read_thread_id;				
				

	int			rx_bytes;		 			// rx data bytes in buffer
 	char		rx_buffer[RX_BUFFER_SIZE];
 	char		tx_buffer[TX_BUFFER_SIZE];
	int 		tx_bytes;


	bool	 	connected;
	struct 		pollfd 	serial_poll;	
	
	bool 	 	write_n	(uint8_t mbyte,...);
	bool 	 	read_n		(uint8_t mbyte,uint8_t address,uint8_t cmd,...);
	
};


extern vector<DriveFive> fives;

#endif
