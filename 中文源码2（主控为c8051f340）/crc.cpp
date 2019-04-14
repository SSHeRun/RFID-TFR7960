/**
	* @file crc.h
	* @auther Sherry Hwang
	* @version V1.0
	* @date 26-December-2018
	* @breif The CRC caculation and check
	*/
#include "crc.h"
/**
  * @file   crc.c
	* @brief  Caculate frames's CRC
  * @param  str The array waiting to be caculated
	* @param  n   The array's length
  * @retval the CRC value of the frames
  */
unsigned char caculCRC(unsigned char *str,unsigned char n) 
{
    unsigned char i = 0;
    unsigned char sum = 0xaa;

    for(i = 0; i < n; i++) 
		{
        sum += str[i];
    }
    return sum;
}

/**
  * @file   crc.c
	* @brief  Check frames's CRC
  * @param  str The array waiting to be checked
	* @param  n   The array's length
  * @retval Whether the frames's CRC is right
  */
bit checkCRC(unsigned char *str,unsigned char n)
{
		//CRC frames[num-2]
		unsigned char i = 0;
    unsigned char sum = 0;

    for(i = 0; i < n - 2; i++) {
        sum += str[i];
    }

    if(sum == str[n - 2]) {
        return 1;
    }
    return 0;
}