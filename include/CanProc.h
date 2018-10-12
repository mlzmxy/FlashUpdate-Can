/*
 * CanProc.h
 *
 *  Created on: 2018-7-19
 *      Author: wjg
 */

#ifndef CANPROC_H
#define CANPROC_H


// CAN ID
#define SEND_CANID    0x01A2A3A1        //发送ID
#define RECEIVE_CANID 0x01A2A3A0      //接收ID

/*
#define UNLOCKCSM_CANID 0x05A5A111              //解锁CSM 发送ID
#define RECEIVE_UNLOCKCSM_CANID 0x05A5A011      //解锁CSM 接收ID

#define TOGGLE_CANID 0x05A5A112                 //toggle测试 发送ID
#define RECEIVE_TOGGLE_CANID 0x05A5A012         //toggle测试 接收ID

#define VERSION_CANID 0x05A5A113                //API版本 发送ID
#define RECEIVE_VERSION_CANID 0x05A5A013        //API版本 接收ID

#define ERASE_CANID 0x05A5A114                  //Flash擦除 发送ID
#define RECEIVE_ERASE_CANID 0x05A5A014          //Flash擦除 接收ID

#define DATABLOCKSIZE_CANID 0x05A5A115          //数据块大小 发送ID
#define RECEIVE_DATABLOCKSIZE_CANID 0x05A5A015  //数据块大小 接收ID

#define RECEICVEDATA_CANID 0x05A5A116           //接收数据 发送ID
#define RECEIVE_RECEICVEDATA_CANID 0x05A5A016   //接收数据 接收ID

#define CHECKSUM_CANID 0x05A5A117               //数据校验 发送ID
#define RECEIVE_CHECKSUM_CANID 0x05A5A017       //数据校验 接收ID

#define PROGRAM_CANID 0x05A5A118                //Flash Program 发送ID
#define RECEIVE_PROGRAM_CANID 0x05A5A018        //Flash Program 接收ID

#define VERIFY_CANID 0x05A5A119                 //Flash校验 发送ID
#define RECEIVE_VERIFY_CANID 0x05A5A019         //Flash校验 接收ID
*/

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
extern can_msg receive_msg;
extern Uint16 receive_flag;

//---------------------------------------------------------------------------
// Prototype for Functions
//
void Cana_send_data(can_msg_data* data);
void Canb_send_data(can_msg_data* data);

/* prototype for interrupt functions */
interrupt void ecan0a_isr();
interrupt void ecan0b_isr();


#endif /* CANPROC_H */
