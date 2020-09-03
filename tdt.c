// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include "parser.h" 
#include <stdlib.h>
#include <time.h>

//     int BytesSent, nlen;
int i;
int timer;
uint8_t frame[255];
char tbuffer[33];
bool fileopen = 0;
bool fileerr = 0;
FILE *fp = NULL;
char temm[200] = {0};

time_t rawtime; // = time(NULL);
struct tm *ptm;// = gmtime(&rawtime);
struct tm *ltm;// = localtime(&rawtime);
uint8_t in[40] = {0x73, 0x70, 0x1A, 0xE6, 0xD6, 0x11, 0x08, 0x12, 0xF0, 0x0F, 0x58, 0x0D, 0x44, 0x45, 0x55, 0x02, 0x02, 0x00, 0xE7, 0x0B, 0x01, 0x00, 0x00, 0x01, 0x00};
uint32_t result;



unsigned int crc32mpeg(unsigned char *message, size_t l)
{
   size_t i, j;
   unsigned int crc, msb;

   crc = 0xFFFFFFFF;
   for(i = 0; i < l; i++) {
      // xor next byte to upper bits of crc
      crc ^= (((unsigned int)message[i])<<24);
      for (j = 0; j < 8; j++) {    // Do eight times.
            msb = crc>>31;
            crc <<= 1;
            crc ^= (0 - msb) & 0x04C11DB7;
      }
   }
   return crc;         // don't complement crc on output
}


uint16_t DateToMjd (int Year, int Month, int Day)
{
    return
        367 * Year
        - 7 * (Year + (Month + 9) / 12) / 4
        - 3 * ((Year + (Month - 9) / 7) / 100 + 1) / 4
        + 275 * Month / 9
        + Day
        + 1721028
        - 2400000;
}


void get_utc(void){
    rawtime = time(NULL);
    ptm = gmtime(&rawtime);
}

void get_ltc(void){
    rawtime = time(NULL);
    ltm = localtime(&rawtime);
}

uint32_t dec2bcd_r(uint16_t dec)
{
    return (dec) ? ((dec2bcd_r( dec / 10 ) << 4) + (dec % 10)) : 0;
}

void gentime(void){
    get_utc();

    result = DateToMjd(ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday);
    //fprintf(stderr,"MJD:\t%04X\n\n",result);
    in[0] = 0x73;
    in[3] = result >> 8 & 0xff;
    in[4] = result & 0xff;
    in[5] = dec2bcd_r(ptm->tm_hour);
    in[6] = dec2bcd_r(ptm->tm_min);
    in[7] = dec2bcd_r(ptm->tm_sec);

    printf("The UTCtime is: %02d.%02d.%04d %02d:%02d:%02d\n", ptm->tm_mday, ptm->tm_mon +1 , ptm->tm_year + 1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    get_ltc();

    in[16] = 1 + ltm->tm_isdst;
    in[23] = (in[16] % 2) + 1;

    //printf("The Local-time: %02d.%02d.%04d %02d:%02d:%02d ", ltm->tm_mday, ltm->tm_mon +1 , ltm->tm_year + 1900, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    //ltm->tm_isdst ? printf("DST\n") : printf("\n");

if(ltm->tm_isdst){
    ltm->tm_mon = 9;
    ltm->tm_mday = 31;
    ltm->tm_hour = 5;
    ltm->tm_min = 0;
    ltm->tm_sec = 0;
    rawtime = mktime(ltm);
    ltm = localtime(&rawtime);
    while(ltm->tm_isdst == 0){
        ltm->tm_mday--;
        rawtime = mktime(ltm);
        ltm = localtime(&rawtime);
        while(ltm->tm_isdst == 0){
            ltm->tm_hour--;
            rawtime = mktime(ltm);
            ltm = localtime(&rawtime);
        }
    }
} else {
    ltm->tm_year++;
    ltm->tm_mon = 3;
    ltm->tm_mday = 31;
    ltm->tm_hour = 5;
    ltm->tm_min = 0;
    ltm->tm_sec = 0;
    rawtime = mktime(ltm);
    ltm = localtime(&rawtime);
    while(ltm->tm_isdst == 1){
        ltm->tm_mday--;
        rawtime = mktime(ltm);
        ltm = localtime(&rawtime);
        while(ltm->tm_isdst == 1){
            ltm->tm_hour--;
            rawtime = mktime(ltm);
            ltm = localtime(&rawtime);
        }

    }
}

    //printf("Zeitumstellung am: %02d.%02d.%04d um %02d:%02d:%02d UTC\n", ltm->tm_mday, ltm->tm_mon +1 , ltm->tm_year + 1900, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

    result = DateToMjd(ltm->tm_year+1900,ltm->tm_mon+1,ltm->tm_mday);
    //fprintf(stderr,"MJD:\t%04X\n\n",result);

    in[18] = result >> 8 & 0xff;
    in[19] = result & 0xff;

    result = crc32mpeg(in,25);
    in[25] = result >> 24 & 0xFF;
    in[26] = result >> 16 & 0xFF;
    in[27] = result >> 8 & 0xFF;
    in[28] = result & 0xFF;

    for(int i = 0;i<29;i++){
        printf("%02X ", in[i]);
    }

    printf("\n\n");
    in[0] &= 0xF0;
}


int main(int argc, char const *argv[]) 
{ 
    unsigned int port;
    if(argc != 2){
        fprintf(stderr,"No Port defined! Exiting...\n");
        return -1;
    }

    port = atoi(argv[1]);
    int sock = 0, BytesSent; 
    struct sockaddr_in serv_addr; 
    char buffer[1024] = {0}; 
    char sendbuf[1024];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

     setup(0,sendbuf);
     send(sock, sendbuf, ((sendbuf[4]+5) &0xff ), 0);
     recv(sock , buffer, 1024,0);
     printf("\n");
     printf("Incoming Message\n");
	 for(i =0;i < buffer[4]+5;i++){printf("%02X ",buffer[i]&0xff);}
     printf("\n");

     setup(1,sendbuf);
     send(sock, sendbuf, ((sendbuf[4]+5) &0xff ), 0);
     recv(sock , buffer, 1024,0);
     printf("\n");
     printf("Incoming Message\n");
	 for(i =0;i < buffer[4]+5;i++){printf("%02X ",buffer[i]&0xff);}
     printf("\n");

     setup(2,sendbuf);
     send(sock, sendbuf, ((sendbuf[4]+5) &0xff ), 0);
     recv(sock, buffer, 1024,0);
     printf("\n");
     printf("Incoming Message\n");
	 for(i =0;i < buffer[4]+5;i++){printf("%02X ",buffer[i]&0xff);}
     printf("\n");


while(true){
	genframe(sendbuf);
	gentime();
	
	memcpy(&sendbuf[40],&in[0],8);
	sendbuf[42] = 0x05;
	BytesSent = send(sock, sendbuf, ((sendbuf[4]+5) &0xff ), 0);
	memcpy(&sendbuf[40],&in[0],29);
	sendbuf[40] = 0x73;
	BytesSent = send(sock, sendbuf, ((sendbuf[4]+5) &0xff ), 0);

	
	if(BytesSent == SO_ERROR){
          printf("Client: send() error .\n");
	} else {
          //printf("Client: send() is OK - bytes sent: %d\n", BytesSent);
    }
sleep(4);
usleep(500);
}
    return 0; 
} 
