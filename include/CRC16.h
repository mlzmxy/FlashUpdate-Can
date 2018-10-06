/*
 * CRC16.h
 *
 * CRC16: 多项式x16+x12+x5+1（0x1021）,初始值0x0000,低位在前,高位在后,结果与0x0000异或
 *
 *  Created on: 2018.10.6
 *      Author: mlzmxy
 */

#ifndef INCLUDE_CRC16_H_
#define INCLUDE_CRC16_H_

void CRC16(unsigned int* pDataIn, int iLenIn, unsigned int* pCRCOut);

#endif /* INCLUDE_CRC16_H_ */
