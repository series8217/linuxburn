

/*(C) copyright 2008, Steven Snyder, All Rights Reserved

Steven T. Snyder, <stsnyder@ucla.edu> http://www.steventsnyder.com

LICENSING INFORMATION:
 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <popt.h> // for command line parsing
#include <string.h> // for memcpy
#include "linuxburn.h"
#include "linuxburn_gui.h"
#include "sts_serial.h"

// global vars
// ===========
linuxburn_settings burn_settings = { 0, NULL, 1, 0x0000, 0x7FFF,CHIP_AT29C256, NULL,0, 0, 0 };



// ==================================================================|
//  int main ()														 |
// ==================================================================|
// ==================================================================|

int main(int argc, const char* argv[]){
	int opt_flags[2] = {0,0};	// opt_flags[0] = read flag
						// opt_flags[1] = write flag
#define READ 0
#define WRITE 1
 
	unsigned int i;
	int res; // temporary storage for function results
	char *burnport; // path to serial port
	char *binfile;  // path to input or output binary file

	int start_addr = 0x0000; // start address
	int end_addr = 0x7FFF; // end address

	// COMMAND LINE APPLICATION MODE 
	// ==================================================

	poptContext  popt_burn;

	// popt option table
	// -----------------
	struct poptOption burn_opt_table[] ={
					{ "serial",'\0',
					POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,&burnport,0,
					"Serial port the BURN1 is on",
					"/dev/ttyUSB0" },

					{"chip",'\0',
					POPT_ARG_INT,&burn_settings.chip_type,0,
					"Chip type. 0 = AT29C256, 1 = SST27SF512, 2 = CHIP_AM29F040, 3 = 2732A",
					NULL},

					{ "read",'\0',POPT_ARG_NONE,&opt_flags[0],0,
					"Read from PROM",NULL},

					{ "write",'\0',POPT_ARG_NONE,&opt_flags[1],0,
					"Write to PROM",NULL},

					{ "start",'\0',POPT_ARG_INT,&start_addr,0,
					"Start address. Must end on first byte of a block",
					"0x0000"},

					{ "end",'\0',POPT_ARG_INT,&end_addr,0,
					"End address. Must end on last byte of a block",
					"0x7FFF"},

					POPT_AUTOHELP

					{ NULL, 0, 0, NULL, 0, 0, NULL}
					};

	// popt context
	popt_burn = poptGetContext(NULL, argc, argv,burn_opt_table,0);
	poptSetOtherOptionHelp(popt_burn,"[OPTIONS]* <bin file>\nTo use GUI: linuxburn [-serial=/dev/ttyUSB0]");

	if (argc<2) { poptPrintUsage(popt_burn,stderr,0); return 1; }

	res = poptGetNextOpt(popt_burn); // parse the command line arguments

	binfile = poptGetArg(popt_burn);
	// if no bin file specified but a command line read or write operation is requested... 
	if ((binfile == NULL || !(poptPeekArg(popt_burn)==NULL)) && (opt_flags[READ] || opt_flags[WRITE]))
	{
		fprintf(stderr,"Specify a single .bin file to read/write, .e.g., BCFA1618.bin\n");
		return -1;
	}

	poptFreeContext(popt_burn); // free the popt context

	// Initialize the burn/read buffer
	// ===============================
	if (select_chip(burn_settings.chip_type) != 0)
	{
		return -1;
	}
	// zero out the bin buffer
	for (i=0; i<burn_settings.chip_size; i++)
		burn_settings.binbuffer[i] = 0;

	// Establish a connection to the BURN1
	// ===================================
	printf("Trying to connect to device on %s...\n",burnport);

	// connect to the device and set the serial port settings
	burn_settings.fburnport = serial_connect(burnport,O_RDWR | O_NOCTTY,BAUDRATE);
	if (burn_settings.fburnport == -1)
	{
		printf("Couldn't connect to %s\n",burnport);
		return -1;
	}
	
	// wake up the BURN1 / verify it is present
	if (verifyburn1()<0)
	{
		printf("BURN1 verification failure.\n");
		tcflush(burn_settings.fburnport, TCIOFLUSH);
		close(burn_settings.fburnport);
		return -1;
	}




	// read operation
	//===============
	if (opt_flags[READ])
	{	
		// Open the .bin file for read/write, create if it doesnt exist
		// ------------------------------------------------------------
		burn_settings.fbinfile = open(binfile,O_RDWR | O_CREAT, 0666);
		if (burn_settings.fbinfile == -1)
		{
			printf("Unable to open/create %s for writing.\n",binfile);
			return 0;	}
		printf("Opened %s for writing.\n",binfile);

		// Read from the chip
		// ------------------
		res = readchip(burn_settings.fbinfile,start_addr,end_addr);

		if (res==-1) printf("Fatal read error occured. Bin file may be corrupted.\n");
		else printf("Received %d bytes from device and wrote to file: %s.\n", res,binfile);
		
		close(burn_settings.fbinfile);
	}

	// burn/write chip operation
	//==========================
	else if (opt_flags[WRITE])
	{	burn_settings.fbinfile = open(binfile,O_RDONLY);
		if (burn_settings.fburnport == -1)
		{
			printf("Unable to open %s for reading.\n", binfile);
			printf("Check that the file exists.\n");
			return -1;
		}
		printf("Opened %s for burning to device.\n",binfile);

		// Write to the chip
		// -----------------
		res = burnfile(burn_settings.fbinfile,start_addr,end_addr);

		if (res==-1) printf("Fatal burn error occured. Bin file may be corrupted.\n");
		else printf("Burned %d bytes to device from file: %s.\n", res,binfile);
		
		close(burn_settings.fbinfile);

	}
	// GUI mode if no read/write command line argument 
	// ===============================================
	else
	{
		// GUI mode
		linuxburn_gui(argc, (char**)argv);
	}

	// discard any unwritten data
	tcflush(burn_settings.fburnport, TCIOFLUSH);

	// close the port
	close(burn_settings.fburnport);
	printf("Connection closed.\n"); 

	// free the bin buffer
	free(burn_settings.binbuffer);
	return 0;
}


// linuxburn general function definitions 
// ============================================================


// check if the specified chip is known, set the settings in burn_settings,
// initialize the bin buffer for the selected chip type, etc
int select_chip(linuxburn_chip_t chip_num)
{

	if (chip_num == CHIP_AT29C256)
	{
		burn_settings.chip_type = CHIP_AT29C256;
		burn_settings.read_block_size = AT29C256_READ_BLOCK_SIZE;
		burn_settings.burn_block_size = AT29C256_BURN_BLOCK_SIZE;
		burn_settings.chip_size = AT29C256_MAX_BYTES; // XXX double check this... does the BURN1
													// support burning this many bytes to it?
	}


	if (burn_settings.chip_size == 0)
	{
		printf("Chip type %d could not be selected.\n", chip_num);
		return -1;
	}

	burn_settings.binbuffer = malloc(burn_settings.chip_size);
	if (burn_settings.binbuffer == NULL)
	{
		printf("Out of memory.\n");
		return -1;
	}

	return 0;
}


// wake up / verify the BURN1
int verifyburn1()
{
	int res;
	unsigned int attempts = 0;
	unsigned char inbuffer[16];
 
	unsigned char outbuffer[8] = { 0x56,0x56 }; // XXX should dynamically allocate this?

	while (attempts < MAX_CONNECT_ATTEMPTS)
	{
		write(burn_settings.fburnport,&outbuffer,2); // write the command and burn data to the device
		tcdrain(burn_settings.fburnport); // wait for the bytes to be written
	
		// wait for input
		res = readwithtimeout(burn_settings.fburnport,inbuffer,3,1);
		if (res!=3 || inbuffer[0] != 5)
		{
			attempts++;
			continue;
		}
		
		break;
	}
	
	switch (inbuffer[0])
	{
	case 5: 	printf(" BURN1 found, firmware version %d!\n",inbuffer[1]);
				return 0;
	case 10 : 	printf(" Ostrich found.\n");
				return -1;
	case 2: 	printf(" AutoProm found.\n");
				return -1;
	default: 	printf(" Device not found after %d attempts.\n",MAX_CONNECT_ATTEMPTS);
				return -1;
	}
	
}

int burnfile(int fd, int start_addr, int end_addr){
	int res;
	unsigned int total_bytes=0, block_bytes_loaded=0;
	unsigned char* burnbuffer;

	burnbuffer=malloc(burn_settings.burn_block_size);

	while(1) // read from file until no more bytes left to read.
	{
		block_bytes_loaded = 0;
		while (block_bytes_loaded < burn_settings.burn_block_size)
		{
			res = read(fd,burnbuffer,burn_settings.burn_block_size);
			if (res==0) // end of file
			{
				break;
			}
			else if (res==-1)
			{
				printf("Error reading from file.\n");
				free(burnbuffer);
				return -1;
			}
			block_bytes_loaded+=res;
		}
		if (block_bytes_loaded == 0) break; // if end of file
		
		res = writeblock(start_addr,burnbuffer,burn_settings.burn_block_size);
		if (res==-1)
		{
			printf("Block burn failure.\n");
			free(burnbuffer);
			return -1;
		}
		total_bytes+=burn_settings.burn_block_size;
		start_addr+=burn_settings.burn_block_size;
	}
	free(burnbuffer);

	return total_bytes;
}


// read blocks from start_addr to end_addr from chip type chip_type
int readchip(int fd, int start_addr, int end_addr){
		unsigned char* readbuffer;
		int res;
		unsigned int total_bytes=0, block_attempts = 1;

		readbuffer = malloc(burn_settings.read_block_size);
		if (readbuffer == NULL)
		{
			printf("Out of memory.\n");
			return -1;
		}

		while(start_addr<end_addr)
		{
			res = readblock(start_addr,readbuffer,burn_settings.read_block_size);
			if ((unsigned)res != burn_settings.read_block_size)
			{
				if (block_attempts < MAX_BLOCK_ATTEMPTS)
				{
					printf("Error receiving block. Retrying.\n");
					block_attempts++;
					continue;
				}
				else
				{
					printf("Failed to receive block after %d retries. Aborting.\n",
							MAX_BLOCK_ATTEMPTS);
					free(readbuffer);					
					return -1;
				}
			}
			write(fd,readbuffer,res);
			total_bytes+=res;
			start_addr+=0x100;
			block_attempts=0;

		}

		free(readbuffer);

		return total_bytes;
}


// Burn a block of bytes from burnbuffer[] to the device starting at start_addr.
// Returns -1 if an error occured, otherwise returns the # of bytes burned
// to the device. size is the size of burnbuffer in bytes.
int writeblock(int start_addr, unsigned char* blockbuffer, unsigned int size)
{
	unsigned char* outbuffer;
	unsigned char reply;
	int res, attempts = 0, byteswritten = 0;

	if (burn_settings.chip_type == CHIP_AT29C256)
	{
		outbuffer = malloc(burn_settings.burn_block_size+6);
		if (outbuffer==NULL) return -1;
		outbuffer[0] = 0x32; outbuffer[1] = 0x57;
		outbuffer[2] = 0x00; outbuffer[3] = 0x00; outbuffer[4] = 0x00;

		// convert the integer start addr to big-endian 2-byte address format and
		// store in the output buffer at outbuffer[3] and outbuffer[4];
		intoToAddr(start_addr,outbuffer+3);
	}
	else
	{
		printf("Chip not recognized.\n");
		free(outbuffer);
		return -1;
	}

	if (size != burn_settings.burn_block_size)
	{
		printf("Burn buffer must be %d bytes.\n",burn_settings.burn_block_size);
		free(outbuffer);
		return -1;
	}

	if (start_addr > 0x7F00)
	{
			printf("Block burn address cannot start at greater than 0x7F00 for 29C256 chip.\n");
			free(outbuffer);
			return -1;
	}
	else if (start_addr < 0x0000)
	{
			printf("Block burn address cannot be negative.\n");
			free(outbuffer);
			return -1;
	}
	// verify that the start address is a multiple of the block size
	else if ( (start_addr%(burn_settings.burn_block_size)) )
	{
		printf("Burn address must be the start address of a ");
		printf("block. Burn block size: %d\n", burn_settings.burn_block_size);
		free(outbuffer);
		return -1;
	}


	// copy the burn buffer into the output buffer
	unsigned int i;
	for (i=0; i<burn_settings.burn_block_size; i++)
		outbuffer[5+i] = blockbuffer[i];

	// calculate and store the command checksum in the buffer.
	// note that the burn data is checksummed, not just the command!
	outbuffer[burn_settings.burn_block_size+5] = calc_checksum(outbuffer,burn_settings.burn_block_size+5);
	
	printf("Writing block starting at %d\n",start_addr);

	tcflush(burn_settings.fburnport,TCIOFLUSH);

	// write the command and burn data to the device
	write(burn_settings.fburnport,outbuffer,burn_settings.burn_block_size+6); 

	tcdrain(burn_settings.fburnport); // wait for the bytes to be written

	// wait for 0x4F from device to confirm burn is complete

	// wait for input
	res=readwithtimeout(burn_settings.fburnport,&reply,1,1);
	if (res!=1)
	{
		printf("Timed out waiting for reply from device.\n");
		free(outbuffer);
		return -1;
	}
	else if (reply!=0x4F)
	{
			printf("Block burn failed. Received %d\n",reply);
			free(outbuffer);
			return -1;
	}
	else byteswritten = burn_settings.burn_block_size;
	
	free(outbuffer);
	
	return byteswritten;
}


// Read a block of bytes into inbuffer[] from the device starting at start_addr.
// returns -1 if an error occured, otherwise returns the # of bytes written
// to inbuffer. size is the size of inbuffer in bytes.
int readblock(int start_addr, unsigned char* inbuffer, unsigned int size)
{
	unsigned char* outbuffer;
	unsigned int outbuffer_size;
	unsigned char temp_inbuffer[257]; // XXX need to dynamically allocate this
	int res;
	unsigned char checkval;

	if (burn_settings.chip_type == CHIP_AT29C256)
	{
		outbuffer_size = 6;
		outbuffer = malloc(outbuffer_size);
		if (outbuffer==NULL) return -1;
		outbuffer[0] = 0x32; outbuffer[1] = 0x52;
		outbuffer[2] = 0x00; outbuffer[3] = 0x00; outbuffer[4] = 0x00;
		outbuffer[5] = 0x84;

		// convert the integer start addr to big-endian 2-byte address format and
		// store in the output buffer at outbuffer[3] and outbuffer[4];
		intoToAddr(start_addr,outbuffer+3);

	}
	else
	{
		printf("Chip not recognized.\n");
		free(outbuffer);
		return -1;
	}

	if (size < burn_settings.read_block_size)
	{
		printf("Read buffer must be at least %d bytes.\n",burn_settings.read_block_size);
		free(outbuffer);		
		return -1;
	}

	if (start_addr > 0x7F00)
	{
			printf("Block read address cannot start at greater than 0x7F00 for 29C256 chip.\n");
			free(outbuffer);
			return -1;
	}
	else if (start_addr < 0x0000)
	{
			printf("Block read address cannot be negative.\n");
			free(outbuffer);
			return -1;
	}
	// verify that the start address is a multiple of the block size
	else if ( (start_addr%(burn_settings.read_block_size)) )
	{
		printf("Read address must be the start address of a ");
		printf("block. Read block size: %d\n", burn_settings.read_block_size);
		free(outbuffer);
		return -1;
	}
	


	// calculate and store the checksum in the buffer
	outbuffer[outbuffer_size-1] = calc_checksum(outbuffer,outbuffer_size-1);

	// write the command to the serial interface
	write(burn_settings.fburnport,outbuffer,outbuffer_size); 
	
	printf("Reading block starting at %d\n",start_addr);

	tcdrain(burn_settings.fburnport); // wait for the bytes to be written

	// wait for input
	res=readwithtimeout(burn_settings.fburnport,temp_inbuffer, 257,1);
	if (res!=257)
	{
		printf("Timed out waiting for reply from device. Received only %d bytes.\n",res);
		free(outbuffer);
		return -1;
	}

	// checksum
	unsigned char checksum = calc_checksum(temp_inbuffer,256);
	if (temp_inbuffer[256]!=checksum)
	{
		printf("Bad checksum. Received %d, should be %d.\n",temp_inbuffer[256],checksum);
		free(outbuffer);
		return -1;
	}
	
	unsigned int i;
	for (i=0; i<256; i++)
		inbuffer[i]=temp_inbuffer[i];

	free(outbuffer);

	return 256;
}


// calculates the single-byte checksum, summing from the start of buffer
// through len bytes. the checksum is calculated by adding each byte
// together and ignoring overflow
unsigned char calc_checksum(unsigned char* buffer, unsigned int len)
{
	unsigned char acc = 0x00;

	unsigned int i;
	for (i=0; i<len; i++)
	{
		//printf("%d,",buffer[i]);
		//if (!(i%16)) printf("\n");
		acc+=buffer[i];
	}
	//printf("Checksum: %d\n",acc);

	return acc;
}

// truncates input_int to a 16-bit unsigned integer (sign is ignored), and stores it in
// big-endian format in the first two bytes of buf. 
// e.g. -65532 is converted to 0xFFFC, so buf[0] = 0xFF, buf[1] = 0xFC
void intoToAddr(int integer, unsigned char* buf)
{
	//printf("Converted %d ",integer);
	unsigned int intsize = sizeof(int);
	// convert to unsigned
	if (integer < 0)
		integer = -integer;

	unsigned char lobyte = integer&(0xFF);
	unsigned char hibyte = (integer>>8)&(0xFF);

	buf[0] = hibyte;
	buf[1] = lobyte;
	
	//printf("to %d %d \n",hibyte,lobyte);
}
