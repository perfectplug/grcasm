
#ifndef _grcasm_h_PERFECTPLUG_
#define _grcasm_h_PERFECTPLUG_
#include <Windows.h>

#define JUMP_TYPE_MEMORY_ADDR			0X1
#define JUMP_TYPE_TABLE					0X2
 

typedef struct _OpInfo
{
	int iJumpType ;									//跳转类型
	char szTOIp[32] ;									//跳转目标ASM指令
	char szEip[32] ;									//跳转当前ASM指令
	DWORD dwEip ;									//跳转当前地址
	DWORD dwTOIp ;									//跳转目标地址
	DWORD dwJMPCount ;							//跳转标识
	DWORD dwJumpTableFirstLabel ;				//跳转表一级标签
	DWORD dwJumpTableSecondLabel ;			//跳转表二级标签
}OPINFO ,*POPINFO;

typedef struct _JumpTable 
{
	DWORD dwEip ;									//跳转表当前地址
	DWORD dwTOIp ;									//跳转目标地址
	DWORD dwJumpTableFirstLabel ;				//跳转表一级标签
	DWORD dwJumpTableSecondLabel ;			//跳转表二级标签
}JUMPTABLE ,*PJUMPTABLE;

//////////////////////////////////////////////////////////////////////////
//函数名：fGetRunableASMCodeToFile
//功能说明：获取指定范围的反汇编代码以裸替函数的形式保存至文本
//参数：
// ulstart (ulong) ，获取反汇编代码开始地址
//	ulend (ulong) ，获取反汇编代码结束地址
//返回值：
//	BOOL，调用成功返回TRUE ，调用失败返回FALSE
BOOL fGetRunableASMCodeToFile(char szFileDirectory[] ,long  ulstart ,long  ulend ,int iCallCount )   ;


#define TYPE_GET_BYTE_ARRAY		1
#define TYPE_GET_WORD_ARRAY	2
#define TYPE_GET_DWORD_ARRAY	4
void fGetArrayHEXToFile(int itype ,long ulstart ,long ulend);



void fuck(DWORD dwNextAddr,DWORD dwSaveAddr) ;

#endif