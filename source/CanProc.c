/*
 * CanProc.c
 *
 *  Created on: 2018-7-19
 *      Author: wjg
 */

#include "DSP2833x_Device.h"     // DSP28 Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP28 Examples Include File

#include "CanProc.h"

//---------------------------------------------------------------------------
// Const Variables
//
const can_msg can_msg_0 = {{0}, 0, {0, 0}};  //can消息初始值

//---------------------------------------------------------------------------
// Variables
//


//---------------------------------------------------------------------------
// prototype for functions
//


/* CAN Interrupt Function */
interrupt void ecan0a_isr()
{
	//can_msg msg = can_msg_0;

	DINT;

	//邮箱包含接收信息,RMP[31:0]接收消息挂起位，若邮箱n接收到消息，则n置位
	if(0x01 == ECanaRegs.CANRMP.bit.RMP31)
	{

		ECanaRegs.CANRMP.bit.RMP31 = 1;  //写1清除
	}

	PieCtrlRegs.PIEACK.bit.ACK9 = 1;

	EINT;
}

