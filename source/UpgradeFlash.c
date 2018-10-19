//###########################################################################
//
// FILE:  Example_Flash2833x_API.c	
//
// TITLE: F2833x Flash API Example
//
// NOTE:  This example runs from Flash.  First program the example
//        into flash.  The code will then copy the API's to RAM and 
//        modify the flash. 
//
//
//###########################################################################
// $TI Release: F28335 API Release V1.0 $
// $Release Date: September 14, 2007 $
//###########################################################################

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

/*---- Flash API include file -------------------------------------------------*/
#include "Flash2833x_API_Library.h"

#include "UpgradeFlash.h"

/*---- example include file ---------------------------------------------------*/
#include "CanProc.h"
#include "CRC16.h"
#include "project.h"

/*---- Standard headers -------------------------------------------------------*/
#include <stdio.h>

/*--- Global variables used to interface to the flash routines */
FLASH_ST FlashStatus;

/*---------------------------------------------------------------------------
 Data/Program Buffer used for flash data
 ---------------------------------------------------------------------------*/
Uint16 buffer[WORDS_IN_FLASH_BUFFER];

/*---------------------------------------------------------------------------
 Sector address info
 ---------------------------------------------------------------------------*/
typedef struct
{
    Uint16 *StartAddr;
    Uint16 *EndAddr;
} SECTOR;

#define OTP_START_ADDR  0x380400
#define OTP_END_ADDR    0x3807FF

#define FLASH_START_ADDR  0x300000
#define FLASH_END_ADDR    0x33FFFF

SECTOR Sector[8] = { (Uint16 *) 0x338000, (Uint16 *) 0x33FFFF,
                     (Uint16 *) 0x330000, (Uint16 *) 0x337FFF,
                     (Uint16 *) 0x328000, (Uint16 *) 0x32FFFF,
                     (Uint16 *) 0x320000, (Uint16 *) 0x327FFF,
                     (Uint16 *) 0x318000, (Uint16 *) 0x31FFFF,
                     (Uint16 *) 0x310000, (Uint16 *) 0x317FFF,
                     (Uint16 *) 0x308000, (Uint16 *) 0x30FFFF,
                     (Uint16 *) 0x300000, (Uint16 *) 0x307FFF };

extern Uint32 Flash_CPUScaleFactor;
extern Uint16 upgrade_flag;

enum FLOW
{
    handshake = 0x10,
    unlockCSM = 0x11,
    version = 0x12,
    erase = 0x13,
    dataBlockInfo = 0x14,
    flashData = 0x15,
    checkSum = 0x16,
    program = 0x17,
    verify = 0x18,
    resetDSP = 0x19
};
enum FLOW update_flow = handshake;

Uint16 receive_cmd_flag = 0;   //接收命令标志
Uint16 start_data_flag = 0;  //允许接收消息标志
Uint16 data_num = 0;   //接收的数据个数

void FlashUpdate()
{
    Uint16 status;
    Uint16 *flash_ptr;     // Pointer to a location in flash
    Uint16 length;         // Number of 16-bit values to be programmed
    Uint16 versionHex;     // Version of the API in decimal encoded hex
    Uint16 checkCode;      // CRC16 check code

    Uint16 *addr;    //Uint32转换为地址
    Uint32 addr_32;  //32位地址变量

    can_msg_data data;
    data.word.u32[0] = 0;
    data.word.u32[1] = 0;

    Flash_CPUScaleFactor = SCALE_FACTOR;
    Flash_CallbackPtr = NULL;

    data.byte.b0 = handshake;
    Can_send_data(&data);  //主动发送握手帧

    while (1)
    {
        if (receive_cmd_flag)
        {
            DINT;  //关闭全局中断

            receive_cmd_flag = 0;
            status = 1;
            data.byte.b0 = receive_msg.data.byte.b0;
            data.byte.b1 = receive_msg.data.byte.b1;
            data.byte.b2 = receive_msg.data.byte.b2;
            data.byte.b3 = receive_msg.data.byte.b3;
            data.byte.b4 = receive_msg.data.byte.b4;
            data.byte.b5 = receive_msg.data.byte.b5;
            data.byte.b6 = receive_msg.data.byte.b6;
            data.byte.b7 = receive_msg.data.byte.b7;

            switch (receive_msg.data.byte.b0)
            {
            case handshake:
                upgrade_flag = 1;  //置升级标志
                data.byte.b1 = 0x55;  //握手成功
                break;
            case unlockCSM:
                status = Example_CsmUnlock();
                if (status == STATUS_SUCCESS)
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                }

                break;
            case version:
                versionHex = Flash_APIVersionHex();
                data.byte.b1 = versionHex & 0x00FF;
                data.byte.b2 = versionHex >> 8;
                break;
            case erase:
                if(0 == data.byte.b2)
                {
                    status = Flash_Erase((SECTORC|SECTORD|SECTORE),
                                         &FlashStatus);
                    flash_ptr = Sector[4].StartAddr;
                }
                else if(1 == data.byte.b2)
                {
                    status = Flash_Erase(SECTORA, &FlashStatus);
                    flash_ptr = Sector[0].StartAddr;
                }
                if (status == STATUS_SUCCESS)
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                }
                break;
            case dataBlockInfo:
                length = data.byte.b6 + (data.byte.b7 << 8);
                addr_32 = (data.byte.b5 << 8) | (data.byte.b4 & 0x00FF);
                addr_32 = (addr_32 << 16) | (((data.byte.b3 << 8) | (data.byte.b2 & 0x00FF)) & 0x0000FFFF);
                addr = (Uint16*) addr_32;
                if (addr == flash_ptr)
                {
                    data.byte.b1 = 0x55;
                    start_data_flag = 1;  //允许接收数据
                    data_num = 0;  //接收数据个数
                }
                else
                {
                    data.byte.b1 = 0x0;
                }
                break;
            case flashData:
                data.byte.b1 = data_num & 0x00FF;
                data.byte.b2 = (data_num >> 8) & 0x00FF;
                break;
            case checkSum:
                checkCode = Crc16(buffer, length);
                if (checkCode == (data.byte.b2 + (data.byte.b3 << 8)))
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                }
                break;
            case program:
                status = Flash_Program(flash_ptr, buffer, length, &FlashStatus);
                if (status == STATUS_SUCCESS)
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                }
                break;
            case verify:
                status = Flash_Verify(flash_ptr, buffer, length, &FlashStatus);
                if (status == STATUS_SUCCESS)
                {
                    flash_ptr += length;
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                }
                break;
            case resetDSP:
                data.byte.b1 = 0x55;
                EnableDog();  //使能看门狗
                break;
            }
            Can_send_data(&data);
            EINT;  //开全局中断
        }
    }

}

/*------------------------------------------------------------------
 Example_CsmUnlock

 Unlock the code security module (CSM)

 Parameters:

 Return Value:

 STATUS_SUCCESS         CSM is unlocked
 STATUS_FAIL_UNLOCK     CSM did not unlock
 
 Notes:
 
 -----------------------------------------------------------------*/
Uint16 Example_CsmUnlock()
{
    volatile Uint16 temp;

// Load the key registers with the current password
// These are defined in Example_Flash2833x_CsmKeys.asm

    EALLOW;
    CsmRegs.KEY0 = PRG_key0;
    CsmRegs.KEY1 = PRG_key1;
    CsmRegs.KEY2 = PRG_key2;
    CsmRegs.KEY3 = PRG_key3;
    CsmRegs.KEY4 = PRG_key4;
    CsmRegs.KEY5 = PRG_key5;
    CsmRegs.KEY6 = PRG_key6;
    CsmRegs.KEY7 = PRG_key7;
    EDIS;

// Perform a dummy read of the password locations
// if they match the key values, the CSM will unlock

    temp = CsmPwl.PSWD0;
    temp = CsmPwl.PSWD1;
    temp = CsmPwl.PSWD2;
    temp = CsmPwl.PSWD3;
    temp = CsmPwl.PSWD4;
    temp = CsmPwl.PSWD5;
    temp = CsmPwl.PSWD6;
    temp = CsmPwl.PSWD7;

// If the CSM unlocked, return succes, otherwise return
// failure.
    if ((CsmRegs.CSMSCR.all & 0x0001) == 0)
        return STATUS_SUCCESS;
    else
        return STATUS_FAIL_CSM_LOCKED;

}

