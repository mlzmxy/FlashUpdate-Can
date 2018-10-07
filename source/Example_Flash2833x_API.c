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

/*---- example include file ---------------------------------------------------*/
#include "Example_Flash2833x_API.h"

#include "CanProc.h"
#include "CRC16.h"

/*---- Standard headers -------------------------------------------------------*/
#include <stdio.h>

/*--- Callback function.  Function specified by defining Flash_CallbackPtr */
void MyCallbackFunction(void);
Uint32 MyCallbackCounter; // Just increment a counter in the callback function

/*--- Global variables used to interface to the flash routines */
FLASH_ST FlashStatus;

/*---------------------------------------------------------------------------
 Data/Program Buffer used for testing the flash API functions
 ---------------------------------------------------------------------------*/
#define  WORDS_IN_FLASH_BUFFER 0x200      // Programming data buffer, Words
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

enum FLOW
{
    handshake = 0x10,
    unlockCSM = 0x11,
    toggle = 0x12,
    version = 0x13,
    erase = 0x14,
    dataBlockSize = 0x15,
    receiveData = 0x16,
    checkSum = 0x17,
    program = 0x18,
    verify = 0x19,
    updateDone = 0x20
};
enum FLOW update_flow = handshake;

#pragma CODE_SECTION(FlashUpdate,"ramfuncs");
void FlashUpdate()
{
    Uint16 status;
    Uint16 *flash_ptr;     // Pointer to a location in flash
    Uint16 length;         // Number of 16-bit values to be programmed
    Uint16 versionHex;     // Version of the API in decimal encoded hex
    Uint16 checkCode;      // CRC16 check code
    Uint16 i;

    can_msg_data data;

    /*------------------------------------------------------------------
     Copy API Functions into SARAM

     The flash API functions MUST be run out of internal
     zero-waitstate SARAM memory.  This is required for
     the algos to execute at the proper CPU frequency.
     If the algos are already in SARAM then this step
     can be skipped.
     DO NOT run the algos from Flash
     DO NOT run the algos from external memory
     ------------------------------------------------------------------*/

    //Copy the Flash API functions to SARAM
    MemCopy(&Flash28_API_LoadStart, &Flash28_API_LoadEnd,
            &Flash28_API_RunStart);

    /*------------------------------------------------------------------
     Initalize Flash_CPUScaleFactor.

     Flash_CPUScaleFactor is a 32-bit global variable that the flash
     API functions use to scale software delays. This scale factor
     must be initalized to SCALE_FACTOR by the user's code prior
     to calling any of the Flash API functions. This initalization
     is VITAL to the proper operation of the flash API functions.

     SCALE_FACTOR is defined in Example_Flash2833x_API.h as
     #define SCALE_FACTOR  1048576.0L*( (200L/CPU_RATE) )
     
     This value is calculated during the compile based on the CPU
     rate, in nanoseconds, at which the algorithums will be run.
     ------------------------------------------------------------------*/

    Flash_CPUScaleFactor = SCALE_FACTOR;

    /*------------------------------------------------------------------
     Initalize Flash_CallbackPtr.

     Flash_CallbackPtr is a pointer to a function.  The API uses
     this pointer to invoke a callback function during the API operations.
     If this function is not going to be used, set the pointer to NULL
     NULL is defined in <stdio.h>.
     ------------------------------------------------------------------*/

    Flash_CallbackPtr = NULL;

    MyCallbackCounter = 0; // Increment this counter in the callback function

    // Jump to SARAM and call the Flash API functions
    //Example_CallFlashAPI();

    while (1)
    {
        if (receive_flag)
        {
            DINT; //关闭全局中断

            receive_flag = 0;
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
                data.byte.b1 = 0x55;  //握手成功
                break;
            case unlockCSM:
                /*------------------------------------------------------------------
                 Unlock the CSM.
                 If the API functions are going to run in unsecured RAM
                 then the CSM must be unlocked in order for the flash
                 API functions to access the flash.

                 If the flash API functions are executed from secure memory
                 (L0-L3) then this step is not required.
                 ------------------------------------------------------------------*/

                status = Example_CsmUnlock();
                if (status == STATUS_SUCCESS)
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                    //Example_Error(status);
                }

                break;
            case toggle:
                GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;
                GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;

                // Example: Toggle GPIO0
                Example_ToggleTest(0);

                data.byte.b1 = 0x55;
                break;
            case version:
                versionHex = Flash_APIVersionHex();
                data.byte.b2 = versionHex & 0xFF;
                data.byte.b1 = versionHex >> 8;
                if (versionHex == 0x0210)
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                    // Unexpected API version
                    // Make a decision based on this info.
                    //asm("    ESTOP0");
                }
                break;
            case erase:
                status = Flash_Erase((SECTORC|SECTORD|SECTORE), &FlashStatus);
                flash_ptr = Sector[2].StartAddr;
                if (status == STATUS_SUCCESS)
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                    //Example_Error(status);
                }
                break;
            case dataBlockSize:
                data.byte.b1 = 0x55;
                length = data.byte.b2 + (data.byte.b3 << 8);
                i = 0;
                break;
            case receiveData:
                if((data.byte.b2 + (data.byte.b3 << 8)) == i)
                {
                    if(i != length)
                    {
                        data.byte.b1 = 0x55;
                        buffer[i] = data.byte.b4 + (data.byte.b5 << 8);
                        buffer[i + 1] = data.byte.b6 + (data.byte.b7 << 8);
                        i += 2;
                    }
                    else
                    {
                        data.byte.b1 = 0x0;
                    }
                }
                else
                {
                    data.byte.b1 = 0x0;
                    data.byte.b2 = i & 0xFF;
                    data.byte.b3 = i >> 8;
                }
                break;
            case checkSum:
                checkCode = Crc16(buffer, length);
                if(checkCode == (data.byte.b2 + (data.byte.b3 << 8)))
                {
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                }
                break;
            case program:
                if ((flash_ptr + length) <= Sector[4].EndAddr)
                {
                    status = Flash_Program(flash_ptr, buffer, length,
                                           &FlashStatus);
                    flash_ptr += length;
                    if (status == STATUS_SUCCESS)
                    {
                        data.byte.b1 = 0x55;
                    }
                    else
                    {
                        data.byte.b1 = 0x0;
                        //Example_Error(Status);
                    }
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
                    data.byte.b1 = 0x55;
                }
                else
                {
                    data.byte.b1 = 0x0;
                    //Example_Error(Status);
                }
                break;
            case updateDone:
                data.byte.b1 = 0x55;
                EnableDog();  //使能看门狗
                break;
            default:
                data.byte.b1 = 0x0;
            }
            Cana_send_data(&data);
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

/*------------------------------------------------------------------
 Example_ToggleTest

 This function shows example calls to the ToggleTest.

 This test is used to Toggle a GPIO pin at a known rate and thus
 confirm the frequency configuration of the API functions.

 A pin should be selected based on the hardware being used.

 Return Value: The toggle test does not return.  It will loop
 forever and is used only for testing purposes.

 Notes:
 ----------------------------------------------------------------*/
void Example_ToggleTest(Uint16 PinNumber)
{
    Uint32 mask;

    // Before calling the Toggle Test, we must setup
    // the MUX and DIR registers.

    if (PinNumber > (Uint16) 34)
    {
        asm("    ESTOP0");
        // Stop here. Invalid option.
        for (;;)
            ;
    }

    // Pins GPIO16-GPIO31
    else if (PinNumber >= 32)
    {
        EALLOW;
        mask = ~(((Uint32) 1 << (PinNumber - 16) * 2)
                | ((Uint32) 1 << (PinNumber - 32) * 2 + 1));
        GpioCtrlRegs.GPBMUX1.all &= mask;
        GpioCtrlRegs.GPBDIR.all = GpioCtrlRegs.GPADIR.all
                | ((Uint32) 1 << (PinNumber - 32));
        Flash_ToggleTest(&GpioDataRegs.GPBTOGGLE.all,
                         ((Uint32 )1 << PinNumber));
        EDIS;
    }

    // Pins GPIO16-GPIO31
    else if (PinNumber >= 16)
    {
        EALLOW;
        mask = ~(((Uint32) 1 << (PinNumber - 16) * 2)
                | ((Uint32) 1 << (PinNumber - 16) * 2 + 1));
        GpioCtrlRegs.GPAMUX2.all &= mask;
        GpioCtrlRegs.GPADIR.all = GpioCtrlRegs.GPADIR.all
                | ((Uint32) 1 << PinNumber);
        Flash_ToggleTest(&GpioDataRegs.GPATOGGLE.all,
                         ((Uint32 )1 << PinNumber));
        EDIS;
    }

    // Pins GPIO0-GPIO15
    else
    {
        EALLOW;
        mask = ~(((Uint32) 1 << PinNumber * 2)
                | ((Uint32) 1 << PinNumber * 2 + 1));
        GpioCtrlRegs.GPAMUX1.all &= mask;
        GpioCtrlRegs.GPADIR.all = GpioCtrlRegs.GPADIR.all
                | ((Uint32) 1 << PinNumber);
        EDIS;
        Flash_ToggleTest(&GpioDataRegs.GPATOGGLE.all,
                         ((Uint32 )1 << PinNumber));
    }

}

/*------------------------------------------------------------------
 Simple memory copy routine to move code out of flash into SARAM
 -----------------------------------------------------------------*/

void Example_MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr,
                     Uint16* DestAddr)
{
    while (SourceAddr < SourceEndAddr)
    {
        *DestAddr++ = *SourceAddr++;
    }
    return;
}

/*------------------------------------------------------------------
 For this example, if an error is found just stop here
 -----------------------------------------------------------------*/
#pragma CODE_SECTION(Example_Error,"ramfuncs");
void Example_Error(Uint16 Status)
{

//  Error code will be in the AL register. 
    asm("    ESTOP0");
    asm("    SB 0, UNC");
}

/*------------------------------------------------------------------
 For this example, once we are done just stop here
 -----------------------------------------------------------------*/
#pragma CODE_SECTION(Example_Done,"ramfuncs");
void Example_Done(void)
{

    asm("    ESTOP0");
    asm("    SB 0, UNC");
}

/*------------------------------------------------------------------
 Callback function - must be executed from outside flash/OTP
 -----------------------------------------------------------------*/
#pragma CODE_SECTION(MyCallbackFunction,"ramfuncs");
void MyCallbackFunction(void)
{
    // Toggle pin, service external watchdog etc
    MyCallbackCounter++;
    asm("    NOP");

}

