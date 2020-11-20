
#ifndef _grcasm_h_PERFECTPLUG_
#define _grcasm_h_PERFECTPLUG_
#include <Windows.h>

#define JUMP_TYPE_MEMORY_ADDR			0X1
#define JUMP_TYPE_TABLE					0X2
 

typedef struct _OpInfo
{
	int iJumpType ;									//��ת����
	char szTOIp[32] ;									//��תĿ��ASMָ��
	char szEip[32] ;									//��ת��ǰASMָ��
	DWORD dwEip ;									//��ת��ǰ��ַ
	DWORD dwTOIp ;									//��תĿ���ַ
	DWORD dwJMPCount ;							//��ת��ʶ
	DWORD dwJumpTableFirstLabel ;				//��ת��һ����ǩ
	DWORD dwJumpTableSecondLabel ;			//��ת�������ǩ
}OPINFO ,*POPINFO;

typedef struct _JumpTable 
{
	DWORD dwEip ;									//��ת��ǰ��ַ
	DWORD dwTOIp ;									//��תĿ���ַ
	DWORD dwJumpTableFirstLabel ;				//��ת��һ����ǩ
	DWORD dwJumpTableSecondLabel ;			//��ת�������ǩ
}JUMPTABLE ,*PJUMPTABLE;

//////////////////////////////////////////////////////////////////////////
//��������fGetRunableASMCodeToFile
//����˵������ȡָ����Χ�ķ������������溯������ʽ�������ı�
//������
// ulstart (ulong) ����ȡ�������뿪ʼ��ַ
//	ulend (ulong) ����ȡ�������������ַ
//����ֵ��
//	BOOL�����óɹ�����TRUE ������ʧ�ܷ���FALSE
BOOL fGetRunableASMCodeToFile(char szFileDirectory[] ,long  ulstart ,long  ulend ,int iCallCount )   ;


#define TYPE_GET_BYTE_ARRAY		1
#define TYPE_GET_WORD_ARRAY	2
#define TYPE_GET_DWORD_ARRAY	4
void fGetArrayHEXToFile(int itype ,long ulstart ,long ulend);



void fuck(DWORD dwNextAddr,DWORD dwSaveAddr) ;

#endif