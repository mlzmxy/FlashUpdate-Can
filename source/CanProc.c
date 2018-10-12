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
can_msg receive_msg = {{0}, 0, {0, 0}};  //接收的CAN消息
Uint16 receive_flag = 0;  //接收消息标志

//---------------------------------------------------------------------------
// prototype for functions
//


/* CAN Interrupt Function */
interrupt void ecan0a_isr()
{
	DINT;

	//邮箱包含接收信息,RMP[31:0]接收消息挂起位，若邮箱n接收到消息，则n置位
	if(0x01 == ECanaRegs.CANRMP.bit.RMP31)
	{
	    receive_msg.id.u32 = ECanaMboxes.MBOX31.MSGID.all & 0x1fffffff; //29位的ID
	    receive_msg.dlc = ECanaMboxes.MBOX31.MSGCTRL.bit.DLC;
	    receive_msg.data.byte.b0 = ECanaMboxes.MBOX31.MDL.byte.BYTE0;
	    receive_msg.data.byte.b1 = ECanaMboxes.MBOX31.MDL.byte.BYTE1;
	    receive_msg.data.byte.b2 = ECanaMboxes.MBOX31.MDL.byte.BYTE2;
	    receive_msg.data.byte.b3 = ECanaMboxes.MBOX31.MDL.byte.BYTE3;
	    receive_msg.data.byte.b4 = ECanaMboxes.MBOX31.MDH.byte.BYTE4;
	    receive_msg.data.byte.b5 = ECanaMboxes.MBOX31.MDH.byte.BYTE5;
	    receive_msg.data.byte.b6 = ECanaMboxes.MBOX31.MDH.byte.BYTE6;
	    receive_msg.data.byte.b7 = ECanaMboxes.MBOX31.MDH.byte.BYTE7;

        receive_flag = 1;

		ECanaRegs.CANRMP.bit.RMP31 = 1;  //写1清除
	}

	PieCtrlRegs.PIEACK.bit.ACK9 = 1;

	EINT;
}

/**
 * Cana_send_data  发送CAN消息
 */
void Cana_send_data(can_msg_data* data)
{
    Uint16 i = 0;
    EALLOW;
    ECanaRegs.CANME.all = 0;  //DISABLE MAILBOX

    ECanaMboxes.MBOX0.MSGID.all = SEND_CANID | EXTENDED_FRAME;
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX0.MDL.byte.BYTE0 = data->byte.b0;
    ECanaMboxes.MBOX0.MDL.byte.BYTE1 = data->byte.b1;
    ECanaMboxes.MBOX0.MDL.byte.BYTE2 = data->byte.b2;
    ECanaMboxes.MBOX0.MDL.byte.BYTE3 = data->byte.b3;
    ECanaMboxes.MBOX0.MDH.byte.BYTE4 = data->byte.b4;
    ECanaMboxes.MBOX0.MDH.byte.BYTE5 = data->byte.b5;
    ECanaMboxes.MBOX0.MDH.byte.BYTE6 = data->byte.b6;
    ECanaMboxes.MBOX0.MDH.byte.BYTE7 = data->byte.b7;

    ECanaRegs.CANME.all = 0x80000001;
    ECanaRegs.CANTRS.bit.TRS0 = 0x01;   //写1发送，当发送成功或终止时，被复位

    while(ECanaRegs.CANTA.bit.TA0 == 0)
    {
        if(0xFFF0 == ++i)  //循环65520次 ，退出强制退出
        {
            break;
        }
    }

    ECanaRegs.CANTA.bit.TA0 = 1;
    EDIS;
}

/* CAN Interrupt Function */
interrupt void ecan0b_isr()
{
    DINT;

    //邮箱包含接收信息,RMP[31:0]接收消息挂起位，若邮箱n接收到消息，则n置位
    if(0x01 == ECanbRegs.CANRMP.bit.RMP31)
    {
        receive_msg.id.u32 = ECanbMboxes.MBOX31.MSGID.all & 0x1fffffff; //29位的ID
        receive_msg.dlc = ECanbMboxes.MBOX31.MSGCTRL.bit.DLC;
        receive_msg.data.byte.b0 = ECanbMboxes.MBOX31.MDL.byte.BYTE0;
        receive_msg.data.byte.b1 = ECanbMboxes.MBOX31.MDL.byte.BYTE1;
        receive_msg.data.byte.b2 = ECanbMboxes.MBOX31.MDL.byte.BYTE2;
        receive_msg.data.byte.b3 = ECanbMboxes.MBOX31.MDL.byte.BYTE3;
        receive_msg.data.byte.b4 = ECanbMboxes.MBOX31.MDH.byte.BYTE4;
        receive_msg.data.byte.b5 = ECanbMboxes.MBOX31.MDH.byte.BYTE5;
        receive_msg.data.byte.b6 = ECanbMboxes.MBOX31.MDH.byte.BYTE6;
        receive_msg.data.byte.b7 = ECanbMboxes.MBOX31.MDH.byte.BYTE7;

        receive_flag = 1;

        ECanbRegs.CANRMP.bit.RMP31 = 1;  //写1清除
    }

    PieCtrlRegs.PIEACK.bit.ACK9 = 1;

    EINT;
}

/**
 * Cana_send_data  发送CAN消息
 */
void Canb_send_data(can_msg_data* data)
{
    Uint16 i = 0;
    EALLOW;
    ECanbRegs.CANME.all = 0;  //DISABLE MAILBOX

    ECanbMboxes.MBOX0.MSGID.all = SEND_CANID | EXTENDED_FRAME;
    ECanbMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanbMboxes.MBOX0.MDL.byte.BYTE0 = data->byte.b0;
    ECanbMboxes.MBOX0.MDL.byte.BYTE1 = data->byte.b1;
    ECanbMboxes.MBOX0.MDL.byte.BYTE2 = data->byte.b2;
    ECanbMboxes.MBOX0.MDL.byte.BYTE3 = data->byte.b3;
    ECanbMboxes.MBOX0.MDH.byte.BYTE4 = data->byte.b4;
    ECanbMboxes.MBOX0.MDH.byte.BYTE5 = data->byte.b5;
    ECanbMboxes.MBOX0.MDH.byte.BYTE6 = data->byte.b6;
    ECanbMboxes.MBOX0.MDH.byte.BYTE7 = data->byte.b7;

    ECanbRegs.CANME.all = 0x80000001;
    ECanbRegs.CANTRS.bit.TRS0 = 0x01;   //写1发送，当发送成功或终止时，被复位

    while(ECanbRegs.CANTA.bit.TA0 == 0)
    {
        if(0xFFF0 == ++i)  //循环65520次 ，退出强制退出
        {
            break;
        }
    }

    ECanbRegs.CANTA.bit.TA0 = 1;
    EDIS;
}

