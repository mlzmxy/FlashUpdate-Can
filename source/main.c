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


void main(void)
{
    // Initialize System Control
    InitSysCtrl();

    // Initialize GPIO
    InitECanGpio();
//    ConfigGPIO();

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
//    PieVectTable.TINT0 = &cpu_timer0_isr;
//    PieVectTable.XINT13 = &cpu_timer1_isr;
    PieVectTable.ECAN0INTA = &ecan0a_isr;
    EDIS;

    // Initialize all the Device Peripherals
    InitECan();

    // Init and configue CpuTimer0 and CpuTimer1
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 150, 120000);  //100ms定时器
    ConfigCpuTimer(&CpuTimer1, 150, 1000000);  //1s计时器


    // Enable INT in PIE
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  //enable PIE
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  //CpuTimer0
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;  //ECAN0INTA  eCANA

    IER |= M_INT1;  // Enable CPU Interrupt 1 - ADC CpuTimer0
    IER |= M_INT9;  // Enable CPU Interrupt 9 - CAN
    IER |= M_INT13; // Enable CPU Interrupt 1 - CpuTimer1

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    while (1)
    {
        if(0x5A5A == (*(volatile Uint16*)(0x33FF7F)))
        {
            FlashUpdate();
        }
        else
        {

        }
    }
}



