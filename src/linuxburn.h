#ifndef LINUXBURN_H_INCLUDED
#define LINUXBURN_H_INCLUDED

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

#define AT29C256_BURN_BLOCK_SIZE 256
#define AT29C256_READ_BLOCK_SIZE 256
#define AT29C256_MAX_BYTES 32768

#define MAX_BLOCK_ATTEMPTS 4
#define MAX_CONNECT_ATTEMPTS 3

#define BAUDRATE B115200


typedef enum{ CHIP_AT29C256 = 0, CHIP_SST27SF512, CHIP_AM29F040, CHIP_2732A} linuxburn_chip_t;
// XXX only AT29C256 is currently supported

typedef struct _linuxburn_settings{
	int fburnport;
	char* binfilename;
	int fbinfile;
	int start_addr;
	int end_addr;

	linuxburn_chip_t chip_type;
	unsigned char* binbuffer; // XXX need to initialize this
	unsigned int chip_size; 
	unsigned int read_block_size;
	unsigned int burn_block_size;
} linuxburn_settings;

int select_chip(linuxburn_chip_t chip_num);
// check if the specified chip is known, set the settings in burn_settings,
// initialize the bin buffer for the selected chip type, etc
// returns 0 if successful, -1 if chip unknown

int verifyburn1();
// wake up / verify the BURN1

void intoToAddr(int integer, unsigned char* buf);
// truncates input_int to a 16-bit unsigned integer (sign is ignored), and stores it in
// big-endian format in the first two bytes of buf. 
// e.g. -65532 is converted to 0xFFFC, so buf[0] = 0xFF, buf[1] = 0xFC

unsigned char calc_checksum(unsigned char* buffer, unsigned int len);
// calculates the single-byte checksum, summing from the start of buffer
// through len bytes. the checksum is calculated by adding each byte
// together and ignoring overflow

int burnfile(int fd, int start_addr, int end_addr);
// writes data from file descriptor fd onto the chip starting at start_addr,
// and stopping at end_addr or when no more bytes are available from fd

int writeblock(int start_addr, unsigned char* burnbuffer, unsigned int size);
// Burn a block of bytes from burnbuffer[] to the device starting at start_addr.
// Returns -1 if an error occured, otherwise returns the # of bytes burned
// to the device. size is the size of burnbuffer in bytes.

int readchip(int fd, int start_addr, int end_addr);
// read blocks from start_addr to end_addr

int readblock(int start_addr, unsigned char* inbuffer, unsigned int size);
// Read a block of bytes into inbuffer[] from the device starting at start_addr.
// returns -1 if an error occured, otherwise returns the # of bytes written
// to inbuffer. size is the size of inbuffer in bytes.

#endif

