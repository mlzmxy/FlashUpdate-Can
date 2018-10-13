/*
 * project.h
 *
 *  Created on: 2018年10月13日
 *      Author: wfyjs001
 */

#ifndef PROJECT_H_
#define PROJECT_H_

#define  WORDS_IN_FLASH_BUFFER 0x200      // Programming data buffer, Words

//---------------------------------------------------------------------------
// extern variables
//
extern Uint16 receive_cmd_flag;
extern Uint16 start_data_flag;
extern Uint16 data_num;
extern Uint16 buffer[WORDS_IN_FLASH_BUFFER];  //Data/Program Buffer

#endif /* PROJECT_H_ */
