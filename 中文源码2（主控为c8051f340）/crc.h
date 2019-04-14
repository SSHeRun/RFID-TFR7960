#ifndef __CRC_H__
#define __CRC_H__
/**
  * @file   crc.h
	* @brief  Caculate frames's CRC
  * @param  str The array waiting to be caculated
	* @param  n   The array's length
  * @retval the CRC value of the frames
  */
unsigned char caculCRC(unsigned char *str,unsigned char n);

/**
  * @file   crc.h
	* @brief  Check frames's CRC
  * @param  str The array waiting to be checked
	* @param  n   The array's length
  * @retval Whether the frames's CRC is right
  */
bit checkCRC(unsigned char *str,unsigned char n);
#endif