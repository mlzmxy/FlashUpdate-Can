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
#include "UpgradeFlash.h"
#include "CanProc.h"

#include <stdio.h>   // Standard headers

interrupt void cpu_timer0_isr();  // CpuTimer0 interrupt function

Uint16 upgrade_flag = 0;  //升级标志

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

    EALLOW;
    PieVectTable.TINT0 = &cpu_timer0_isr;
    PieVectTable.ECAN0INTA = &ecan0a_isr;
    PieVectTable.ECAN0INTB = &ecan0b_isr;
    EDIS;

    // Initialize all the Device Peripherals
    InitECan();
    // Init and configue CpuTimer0
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 150, 200000);  //200ms定时器

    // Enable INT in PIE
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  //enable PIE
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  //CpuTimer0
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;  //ECAN0INTA  eCANA
    //PieCtrlRegs.PIEIER9.bit.INTx7 = 1;  //ECAN0INTB  eCANB

    IER |= M_INT1;  // Enable CPU Interrupt 1 - CpuTimer0
    IER |= M_INT9;  // Enable CPU Interrupt 9 - CAN

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    StartCpuTimer0();

    FlashUpdate();
}

//---------------------------------------------------------------------------
// CpuTimer0 interrupt function
//---------------------------------------------------------------------------
// 500ms定时器中断
//
interrupt void cpu_timer0_isr()
{
    if(0 == upgrade_flag)
    {
        asm(" LB 0x318000");
    }

    StopCpuTimer0();
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
    CpuTimer0Regs.TCR.bit.TIF = 1;  //清除cpu定时器中断标志位
    CpuTimer0Regs.TCR.bit.TRB = 1;  //定时器重载
}






