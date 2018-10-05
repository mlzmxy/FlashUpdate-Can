/*
 * CanProc.h
 *
 *  Created on: 2018-7-19
 *      Author: wjg
 */

#ifndef CANPROC_H
#define CANPROC_H


// CAN ID
#define SEND_CANID 0x00000001     //CAN发送消息ID
#define RECEIVE_CANID 0x00000001  //CAN接收消息ID

#define EXTENDED_FRAME 0x80000000          // 扩展帧置位标志

//---------------------------------------------------------------------------
// CAN message structure
//
typedef union
{
	union
	{
		Uint32 u32[2];
		Uint16 u16[4];
	}word;

	struct
	{
		Uint16 b0:8;
		Uint16 b1:8;
		Uint16 b2:8;
		Uint16 b3:8;
		Uint16 b4:8;
		Uint16 b5:8;
		Uint16 b6:8;
		Uint16 b7:8;
	}byte;
}can_msg_data;  // can_msg.data

typedef union
{
	Uint32 u32;

	struct
	{
        Uint16 sa:8;  // 发送方
        Uint16 da:8;  // 接收方
        Uint16 pf:8;  // 功能码
        Uint16 dp:1;  // 数据页
        Uint16 r:1;   // 保留位
        Uint16 pri:3; // 优先级
        Uint16 res:3; // 头信息
	}seg;

	struct
	{
		Uint16 b0:8;
		Uint16 b1:8;
		Uint16 b2:8;
		Uint16 b3:8;
	}byte;
}can_msg_id;  // can_msg.id

typedef struct
{
	can_msg_id id;
	Uint32 dlc;
	can_msg_data data;
}can_msg;  // can msg



//---------------------------------------------------------------------------
// Extern Varibales
//


//---------------------------------------------------------------------------
// Prototype for Functions
//


/* prototype for interrupt functions */
interrupt void ecan0a_isr();


#endif /* CANPROC_H */
