/*
 * main.c
 *
 *  Created on: 2018-10-05
 *      Author: mlzmxy
 */

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

#include "Flash2833x_API_Config.h"    // F2833x Flash User Settings
#include "Flash2833x_API_Library.h"   // Flash API include file
#include "Example_Flash2833x_API.h"   // example include file

#include "CanProc.h"

#include <stdio.h>   // Standard headers

void (*p)() = (void (*)())0x328000;

void main(void)
{
    // Initialize System Control
    InitSysCtrl();

    // Initialize GPIO
    InitECanGpio();

    // Disable CPU interrupts
    DINT;

    // Initialize the PIE control registers to their default state
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    InitPieVectTable();

    MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
    InitFlash();

    EALLOW;
    PieVectTable.ECAN0INTA = &ecan0a_isr;
    PieVectTable.ECAN0INTB = &ecan0b_isr;
    EDIS;

    // Initialize all the Device Peripherals
    InitECan();

    // Enable INT in PIE
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  //enable PIE
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;  //ECAN0INTA  eCANA
    PieCtrlRegs.PIEIER9.bit.INTx7 = 1;  //ECAN0INTB  eCANB

    IER |= M_INT9;  // Enable CPU Interrupt 9 - CAN

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    EALLOW;
       GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;
       GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
       GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;
       GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
       GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;
       GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
       GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;
       GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;
       EDIS;

       GpioDataRegs.GPASET.bit.GPIO0;
       DELAY_US(10);
       GpioDataRegs.GPASET.bit.GPIO1;
       DELAY_US(10);
       GpioDataRegs.GPASET.bit.GPIO6;
       DELAY_US(10);
       GpioDataRegs.GPASET.bit.GPIO7;
       DELAY_US(10);

    while (1)
    {
//        if(0x5A5A == (*(volatile Uint16*)(0x33FF7F)))
//        {
//            FlashUpdate();
//        }
//        else
//        {
    DELAY_US(1000000);
//            asm(" LB 0x328000");
    (*p)();
//        }
    }

}



