/*
 * main.c
 *
 *  Created on: 2018-09-27
 *      Author: mlzmxy
 */

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

void main(void)
{
    // Initialize System Control
    InitSysCtrl();

    // Initialize GPIO
//    InitECanGpio();
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

//    MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
//    InitFlash();

    EALLOW;
//    PieVectTable.TINT0 = &cpu_timer0_isr;
//    PieVectTable.XINT13 = &cpu_timer1_isr;
//    PieVectTable.ADCINT = &adc_isr;
//    PieVectTable.ECAN0INTA = &ecan0a_isr;
//    PieVectTable.ECAN0INTB = &ecan0b_isr;
    //PieVectTable.EPWM1_INT = &epwm1_isr;
    EDIS;

    // Initialize all the Device Peripherals
//    InitAdc();
//    ConfigAdc();
//    InitEPwm();
//    InitECan();

    // Init and configue CpuTimer0 and CpuTimer1
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 150, 120000);  //100ms定时器
    ConfigCpuTimer(&CpuTimer1, 150, 1000000);  //1s计时器


    // Enable INT in PIE
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  //enable PIE
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  //CpuTimer0
    PieCtrlRegs.PIEIER1.bit.INTx6 = 1;  //ADC
    //PieCtrlRegs.PIEIER3.bit.INTx1 = 1;  //ePWM1 INT
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;  //ECAN0INTA  eCANA
    PieCtrlRegs.PIEIER9.bit.INTx7 = 1;  //ECAN0INTB  eCANB

    IER |= M_INT1;  // Enable CPU Interrupt 1 - ADC CpuTimer0
    //IER |= M_INT3;  // Enable CPU Interrupt 3 - EPWM
    IER |= M_INT9;  // Enable CPU Interrupt 9 - CAN
    IER |= M_INT13; // Enable CPU Interrupt 1 - CpuTimer1

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    StartCpuTimer0();

    while (1)
    {
    }
}


