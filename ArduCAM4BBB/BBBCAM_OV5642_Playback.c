/*
 ============================================================================
 Name        : BBBCAM_OV5642_Playback.c
 Author      : Lee
 Version     : V1.0
 Copyright   : ArduCAM demo (C)2015 Lee
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "BBBCAM.h"

#define BOOL int
#define TRUE 1
#define FALSE 0

void setup()
{
  uint8_t vid,pid;
  uint8_t temp;

  ArduCAM(OV5642);

  printf("ArduCAM Start!\n");

  //Check if the ArduCAM SPI bus is OK
  write_reg(ARDUCHIP_TEST1, 0x55);
  temp = read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
  	printf("SPI interface Error!");
  	while(1);
  }

  //Change MCU mode
  //write_reg(ARDUCHIP_MODE, 0x00);
  set_format(JPEG);
  InitCAM();
  
  write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  clear_fifo_flag();
  write_reg(0x01,0);	
}

int main()
{
  setup();

  while(1){
	uint8_t filePath[] = "./temp.jpg";
	GrabImage(filePath);
	while(1);
  }
}
  



int GrabImage(char* str)
{
	int nmemb = 1;
	//Open the new file
	fp = fopen(str,"w");

	flush_fifo();
	clear_fifo_flag();
	capture();

	printf("Start Capture\n");
	while(!(read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK));
	printf("Capture Done!\n");	
	uint8_t buf[256];
	uint16_t i = 0;
	
	uint8_t temp, temp_last;
	temp = read_fifo();
	printf("First byte: %d\n", temp);
	//Write first image data to buffer
	buf[i++] = temp;
	//Read JPEG data from FIFO
	uint32_t byte_count = 0;
	while( (temp != 0xD9) | (temp_last != 0xFF) )
	{
		temp_last = temp;
		temp = read_fifo();
		buf[i++] = temp;
		//Write image data to buffer if not full
		if(i == 256){
			//Write 256 bytes of image data to file
			fwrite(buf,1,256,fp);
			i = 0;
			byte_count++;
		}
	}
	//Write the remain uint8_ts in the buffer
	if(i > 0)	fwrite(buf,1,i,fp);
	byte_count*=256+i;
	printf("\nBytes in image: %d\n", byte_count);
	//Close the file
	fclose(fp);
	printf("File closed\n");
	//Clear the capture done flag
	clear_fifo_flag();

	return 1;
}
