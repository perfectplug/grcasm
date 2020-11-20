// grcasm.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
// erasing from list
#include <iostream>
#include <list>

std::list<DWORD> listCall;
std::list<DWORD>::iterator itCall ;

BOOL fGetRunableASMCodeToFile(char szFileDirectory[] ,long  ulstart ,long  ulend ,int iCallCount ) 
{
	char szTmpBuf[2048*10] ={0} ,szTmpBuf1[MAX_PATH] ={0} ,szTmpBuf2[MAX_PATH] ={0} ,szTmpBuf3[MAX_PATH] ={0};
	DWORD dwArrayJumpTableJumpMaxNum[100] ={0} ; 
	std::list<OPINFO> listJump;
	std::list<OPINFO>::iterator itJump ;
	std::list<JUMPTABLE> listJumpTable;
	std::list<JUMPTABLE>::iterator itJumpTable ;
	OPINFO struct_op_Jump ;
	JUMPTABLE struct_jt_JumpTable ;
	char szTmpJmpBuf[MAX_PATH] ={0} ;
	t_memory		*pmem = NULL;
	t_disasm		da ;
	ulong			cmdsize =MAXCMDSIZE ,uldisasmlen = 0 ,ulpocBYTE=0;
	uchar			cmd[MAXCMDSIZE],*pdecode;
	ulong			decodesize;
	DWORD dwWrittenRet =0 ,dwJMPCount = 0;
	int i =0 ;

	if (iCallCount == 1)
	{
		listCall.clear() ;
	}
	//打开文件
	sprintf(szTmpBuf ,"%s\\%d-%08X.txt",szFileDirectory ,iCallCount ,ulstart) ;
	_Addtolist(0X0 ,1 ,"%s" ,szTmpBuf) ;
	DeleteFile(szTmpBuf) ;
	HANDLE hCurFile = CreateFile(szTmpBuf , GENERIC_READ | GENERIC_WRITE ,
		FILE_SHARE_READ |FILE_SHARE_WRITE ,
		NULL ,OPEN_ALWAYS ,FILE_ATTRIBUTE_NORMAL ,NULL) ;
	if (hCurFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("fGetRunableASMCodeToFile hCurFile == INVALID_HANDLE_VALUE \r\n") ;
		return FALSE ;
	}
	//写入头部
	sprintf(szTmpBuf ,"void __declspec (naked) SUB_%08X()\r\n{\r\n	__asm{\r\n" ,ulstart) ;
	SetFilePointer(hCurFile ,NULL ,NULL ,FILE_END) ;
	WriteFile(hCurFile , szTmpBuf ,strlen(szTmpBuf) , &dwWrittenRet ,NULL) ;

	char seps[] = " ";
	char seps1[] = ".>";
	char seps2[] = "," ;
	char seps3[] ="[*" ;
	char *token;
	BOOL bResolve = FALSE ;
	DWORD dwAddrJumpTable =0 ;
	DWORD dwOffsetJumpTable = 0 ;
	int iJumpTableFirstLabel =0 ,iJumpTableSecondLabel = 0;
	BOOL bInListJumpTable = FALSE ,bInListJump = FALSE ;
	
	//////////////////////////////////////////////////////////////////////////
	//第一遍扫描跳转指令，保存至list中
	ulpocBYTE = 0 ;
	dwJMPCount = 0 ;
	listJump.clear() ;
	while(ulend>=(ulstart+ulpocBYTE))
	{
		_Readmemory(cmd,ulstart+ulpocBYTE,cmdsize,MM_RESTORE|MM_SILENT);
		pmem=_Findmemory(ulstart+ulpocBYTE);
		pdecode =_Finddecode(ulstart+ulpocBYTE,&decodesize);
		//反汇编该指令
		uldisasmlen =_Disasm(cmd,cmdsize,ulstart+ulpocBYTE,pdecode,&da, DISASM_SIZE | DISASM_DATA | DISASM_CODE ,0);

		switch(da.cmdtype)
		{
		case C_JMP:
			//JMP ADDR
			if (da.memtype == 0x4 && da.indexed == 0x4)
			//if(1 ==0 )
			{
				dwOffsetJumpTable = 0 ;
				iJumpTableSecondLabel =0 ;
				while(TRUE)
				{
					_Readmemory(&dwAddrJumpTable ,da.jmptable + dwOffsetJumpTable ,sizeof(DWORD) ,MM_RESTORE|MM_SILENT) ;
					if (dwAddrJumpTable < ulstart || dwAddrJumpTable >ulend)
					{
						break ;
					}
					dwOffsetJumpTable +=4 ;
					iJumpTableSecondLabel ++ ;
				}
				dwArrayJumpTableJumpMaxNum[iJumpTableFirstLabel] = iJumpTableSecondLabel ;
				iJumpTableFirstLabel ++ ;
				break ;
			}
		case C_JMC:
			//JE ADDR  ,JNZ ADDR
		case 0X68:
			//JPO ADDR
			if (da.jmpaddr > 0)
			{
				struct_op_Jump.dwJMPCount = dwJMPCount ;
				struct_op_Jump.dwTOIp = da.jmpaddr ;
				struct_op_Jump.iJumpType = JUMP_TYPE_MEMORY_ADDR ;
				listJump.push_back(struct_op_Jump) ;
				dwJMPCount ++ ;
			}
			break ;
		default:
			break ;
		}
		ulpocBYTE+= uldisasmlen ;
	}
	//////////////////////////////////////////////////////////////////////////
	//计算每个switch 的case个数
	for (i =0 ;i < iJumpTableFirstLabel-1 ; i++)
	{
		dwArrayJumpTableJumpMaxNum[i] -= dwArrayJumpTableJumpMaxNum[i+1] ;
	}
	//第二遍跳转表信息保存至list
	ulpocBYTE = 0 ;
	iJumpTableFirstLabel= 0 ;
	while(ulend>=(ulstart+ulpocBYTE))
	{
		_Readmemory(cmd,ulstart+ulpocBYTE,cmdsize,MM_RESTORE|MM_SILENT);
		pmem=_Findmemory(ulstart+ulpocBYTE);
		pdecode =_Finddecode(ulstart+ulpocBYTE,&decodesize);
		//反汇编该指令
		uldisasmlen =_Disasm(cmd,cmdsize,ulstart+ulpocBYTE,pdecode,&da, DISASM_SIZE | DISASM_DATA | DISASM_CODE ,0);
		switch(da.cmdtype)
		{
		case C_JMP:
			//JMP ADDR
			if (da.memtype == 0x4 && da.indexed == 0x4)
			{
				//////////////////////////////////////////////////////////////////////////
				dwOffsetJumpTable = 0 ;
				iJumpTableSecondLabel =0 ;
				for (i=0 ;i< dwArrayJumpTableJumpMaxNum[iJumpTableFirstLabel] ;i++)
				{
					_Readmemory(&dwAddrJumpTable ,da.jmptable + dwOffsetJumpTable ,sizeof(DWORD) ,MM_RESTORE|MM_SILENT) ;
					if (dwAddrJumpTable < ulstart || dwAddrJumpTable >ulend)
					{
						break ;
					}
					//保存跳转到目的节点
					struct_op_Jump.dwJMPCount = 0 ;
					struct_op_Jump.dwEip = da.ip ;
					struct_op_Jump.dwTOIp = dwAddrJumpTable ;
					struct_op_Jump.dwJumpTableFirstLabel = iJumpTableFirstLabel ;
					struct_op_Jump.dwJumpTableSecondLabel = iJumpTableSecondLabel ;
					struct_op_Jump.iJumpType = JUMP_TYPE_TABLE ;
					listJump.push_back(struct_op_Jump) ;
					//保存跳转表节点
					struct_jt_JumpTable.dwEip = da.ip ;
					struct_jt_JumpTable.dwTOIp = dwAddrJumpTable ;
					struct_jt_JumpTable.dwJumpTableFirstLabel = iJumpTableFirstLabel ;
					struct_jt_JumpTable.dwJumpTableSecondLabel = iJumpTableSecondLabel ;
					listJumpTable.push_back(struct_jt_JumpTable) ;

					dwOffsetJumpTable +=4 ;
					iJumpTableSecondLabel ++ ;
				}
				iJumpTableFirstLabel ++ ;
				//////////////////////////////////////////////////////////////////////////
			}
		default:
			break ;
		}
		ulpocBYTE+= uldisasmlen ;
	}
	//////////////////////////////////////////////////////////////////////////
	//第三遍扫描指令写入文件
	ulpocBYTE = 0 ;
	dwJMPCount = 0 ;
	iJumpTableFirstLabel = 0 ;
	while(ulend>=(ulstart+ulpocBYTE))
	{
		_Readmemory(cmd,ulstart+ulpocBYTE,cmdsize,MM_RESTORE|MM_SILENT);
		pmem=_Findmemory(ulstart+ulpocBYTE);
		pdecode =_Finddecode(ulstart+ulpocBYTE,&decodesize);
		//反汇编该指令
		uldisasmlen =_Disasm(cmd,cmdsize,ulstart+ulpocBYTE,pdecode,&da,
			DISASM_SIZE | DISASM_DATA | DISASM_CODE ,0);

		bResolve = FALSE ;
		switch(da.cmdtype)
		{
		case 0X0:
			//MOV REG,ADDR 
		case 0X1:
			//MOV [REG],ADDR
			if (da.immconst>0)
			{
				sprintf(szTmpBuf1 ,"%s" ,da.result) ;
				token = strtok( szTmpBuf1 , seps2 );
				if (token!=NULL)
				{
					bResolve = TRUE ;
					if (strcmp(da.comment,"")!=NULL)
					{
						sprintf(szTmpBuf ,"		%s, 0X%08X ;%s\r\n",token ,da.immconst ,da.comment) ;
					}
					else
					{
						sprintf(szTmpBuf ,"		%s, 0X%08X \r\n" ,token ,da.immconst ) ;
					}
				}
			}
			break ;
		case C_PSH:
			//PUSH 
			if (da.zeroconst == 0 && da.immconst != 0)
			{
				bResolve = TRUE ;
				if (strcmp(da.comment,"")!=NULL)
				{
					sprintf(szTmpBuf ,"		PUSH 0X%08X ;%s\r\n" ,da.immconst ,da.comment) ;
				}
				else
				{
					sprintf(szTmpBuf ,"		PUSH 0X%08X\r\n" ,da.immconst ) ;
				}
			}
			break ;
		case C_JMP:
			//JMP ADDR
			if (da.memtype == 0x4 && da.indexed == 0x4)
			{
				sprintf(szTmpBuf1 ,"%s" ,da.result) ;
				token = strtok( szTmpBuf1 , seps3 );
				if (token != NULL)
				{
					token = strtok(NULL ,seps3) ;
					memset(szTmpBuf ,0 ,2048) ;
					for (itJumpTable = listJumpTable.begin() ;itJumpTable != listJumpTable.end() ;)
					{
						if (itJumpTable->dwEip == da.ip)
						{
							bResolve = TRUE ;
							sprintf(szTmpBuf3 ,"		CMP %s, 0X%08X\r\n		JE SWITCH_CASE_%08X_%08X\r\n" ,token ,
								itJumpTable->dwJumpTableSecondLabel ,
								itJumpTable->dwJumpTableFirstLabel ,
								itJumpTable->dwJumpTableSecondLabel) ;
							strcat(szTmpBuf ,szTmpBuf3) ;
							itJumpTable = listJumpTable.erase(itJumpTable) ;
						}
						else
						{
							++itJumpTable ;
						}
					}
				}
				break ;
			}
		case C_JMC:
			//JE ADDR  ,JNZ ADDR
		case 0X68:
			//JPO ADDR
			if (da.jmpaddr >0)
			{
				sprintf(szTmpJmpBuf,"%s" ,da.result) ;
				token = strtok( szTmpJmpBuf , seps );
				bResolve = TRUE ;
				sprintf(szTmpBuf ,"		%s LABEL_%08X\r\n" ,token , dwJMPCount ) ;
				dwJMPCount ++ ;
			}
			break ;

		case C_CAL:
			//CALL 
			switch(da.memtype)
			{
			case 0X0:
				if (da.jmpaddr >= dwCodeSegStart && da.jmpaddr <= dwCodeSegEnd)
				{
					BOOL bInlistCall = FALSE ;
					//排除重复的
					for (itCall = listCall.begin() ;itCall != listCall.end() ;itCall ++)
					{
						if (*itCall == da.jmpaddr)
						{
							bInlistCall = TRUE ;
							break ;
						}
					}
					if (!bInlistCall)
					{
						//压入listcall中
						listCall.push_back(da.jmpaddr) ;
						//如果为CALL指令跳转到代码段处
						DWORD dwNextstart = da.jmpaddr ;
						DWORD dwNextlend = _Findprocend(da.jmpaddr) ;
						if ( dwNextlend == 0)
						{
							dwNextlend = _Findnextproc(da.jmpaddr) -1 ;
						}
						//////////////////////////////////////////////////////////////////////////
						//////////////////////////////////////////////////////////////////////////
						fGetRunableASMCodeToFile(szFileDirectory ,dwNextstart ,dwNextlend ,iCallCount + 1) ;
						//////////////////////////////////////////////////////////////////////////
						//////////////////////////////////////////////////////////////////////////
					}
					bResolve = TRUE ;
					if (strcmp(da.comment,"")!=NULL)
					{
						sprintf(szTmpBuf ,"		CALL SUB_%08X ;%s\r\n" ,da.jmpaddr ,da.comment) ;
					}
					else
					{
						sprintf(szTmpBuf ,"		CALL SUB_%08X\r\n" ,da.jmpaddr ) ;
					}
				}
				break ;
			case 0X4:
				if (da.jmpaddr>0X70000000)
				{
					
					sprintf(szTmpBuf1 ,"%s" ,da.result) ;
					token = strtok( szTmpBuf1 , seps1 );
					if (token != NULL)
					{
						token = strtok(NULL ,seps1) ;
						bResolve = TRUE ;
						if (strcmp(da.comment,"")!=NULL)
						{
							sprintf(szTmpBuf ,"		CALL DWORD PTR [%s] ;%s\r\n" ,token ,da.comment) ;
						}
						else
						{
							sprintf(szTmpBuf ,"		CALL DWORD PTR [%s] \r\n" ,token ) ;
						}
					}
				}
				break ;
			default:
				break ;
			}

			break ;
		default:
			break ;
		}
		if (!bResolve)
		{
			if (strcmp(da.comment,"")!=NULL)
			{
				sprintf(szTmpBuf ,"		%s	;%s\r\n" ,da.result ,da.comment) ;
			}
			else
			{
				sprintf(szTmpBuf ,"		%s\r\n" ,da.result ) ;
			}
		}
		
		for (itJump=listJump.begin(); itJump!=listJump.end();)
		{
			if (da.ip == itJump->dwTOIp)
			{
				if (itJump->iJumpType == JUMP_TYPE_MEMORY_ADDR)
				{
					sprintf(itJump->szTOIp ,"LABEL_%08X:\r\n" ,itJump->dwJMPCount) ;
				}
				else if (itJump->iJumpType == JUMP_TYPE_TABLE)
				{
					sprintf(itJump->szTOIp ,"SWITCH_CASE_%08X_%08X:\r\n" ,itJump->dwJumpTableFirstLabel ,itJump->dwJumpTableSecondLabel) ;
				}
				//写入跳转标签
				SetFilePointer(hCurFile ,NULL ,NULL ,FILE_END) ;
				WriteFile(hCurFile , itJump->szTOIp ,strlen(itJump->szTOIp) , &dwWrittenRet ,NULL) ;
				//删除已经写入到文件的标签
				itJump = listJump.erase(itJump) ;
			}
			else
			{
				++itJump ;
			}
		}
		//写入当前指令
		SetFilePointer(hCurFile ,NULL ,NULL ,FILE_END) ;
		WriteFile(hCurFile , szTmpBuf ,strlen(szTmpBuf) , &dwWrittenRet ,NULL) ;

		ulpocBYTE+= uldisasmlen ;
	}
	//写入尾部
	memset(szTmpBuf ,0 ,MAX_PATH) ;
	strcpy(szTmpBuf ,"	}\r\n}\r\n") ;
	SetFilePointer(hCurFile ,NULL ,NULL ,FILE_END) ;
	WriteFile(hCurFile , szTmpBuf ,strlen(szTmpBuf) , &dwWrittenRet ,NULL) ;
	CloseHandle(hCurFile) ;
	return TRUE ;
}

void fGetArrayHEXToFile(int itype ,long ulstart ,long ulend)
{
	char szTmpOneBuf[20] ={0} ;
	char szFilePath[MAX_PATH] ={0} ;
	sprintf(szFilePath ,"%s" ,"C:\\ARRAY_HEX_.txt") ;
	DeleteFile(szFilePath) ;
	HANDLE hCurFile = CreateFile(szFilePath , GENERIC_READ | GENERIC_WRITE ,
		FILE_SHARE_READ |FILE_SHARE_WRITE ,
		NULL ,OPEN_ALWAYS ,FILE_ATTRIBUTE_NORMAL ,NULL) ;
	if (hCurFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString("fGetArrayHEXToFile hCurFile == INVALID_HANDLE_VALUE \r\n") ;
		return  ;
	}
	long lBufLen = ulend - ulstart ;
	BYTE *pBuf = new BYTE[lBufLen] ;
	memset(pBuf ,0 ,lBufLen) ;
	_Readmemory(pBuf ,ulstart ,lBufLen ,MM_RESTORE|MM_SILENT);
	DWORD dwWrittenRet =0 ;
	if (itype == TYPE_GET_BYTE_ARRAY)
	{
		for (int i = 0  ;i < ulend - ulstart ;i =i +itype)
		{
			if (i == 0)
			{
				sprintf(szTmpOneBuf ,"0X%02X" , *(BYTE*)((DWORD)pBuf + i) ) ;
			}
			else
			{
				if ( i%16 ==0 )
				{
					sprintf(szTmpOneBuf ,",0X%02X\r\n" ,*(BYTE*)((DWORD)pBuf + i) ) ;
				}
				else
				{
					sprintf(szTmpOneBuf ,",0X%02X" ,*(BYTE*)((DWORD)pBuf + i) ) ;
				}
			}
			SetFilePointer(hCurFile ,NULL ,NULL ,FILE_END) ;
			WriteFile(hCurFile , szTmpOneBuf ,strlen(szTmpOneBuf) , &dwWrittenRet ,NULL) ;
		}
	}
	else if (itype == TYPE_GET_WORD_ARRAY)
	{
		for (int i =0  ;i < ulend -ulstart ;i =i +itype)
		{
			if (i == 0)
			{
				sprintf(szTmpOneBuf ,"0X%04X" ,*(WORD*)((DWORD)pBuf + i)) ;
			}
			else
			{
				if ( i%16 ==0 )
				{
					sprintf(szTmpOneBuf ,",0X%04X\r\n" ,*(WORD*)((DWORD)pBuf + i) ) ;
				}
				else
				{
					sprintf(szTmpOneBuf ,",0X%04X" ,*(WORD*)((DWORD)pBuf + i) ) ;
				}
			}
			SetFilePointer(hCurFile ,NULL ,NULL ,FILE_END) ;
			WriteFile(hCurFile , szTmpOneBuf ,strlen(szTmpOneBuf) , &dwWrittenRet ,NULL) ;
		}
	}
	else if (itype == TYPE_GET_DWORD_ARRAY )
	{
		for (int i =0  ;i <ulend -ulstart ;i =i +itype)
		{
			if (i == 0)
			{
				sprintf(szTmpOneBuf ,"0X%08X" ,*(DWORD*)((DWORD)pBuf + i) ) ;
			}
			else
			{
				if ( i%16 ==0 )
				{
					sprintf(szTmpOneBuf ,",0X%08X\r\n" ,*(DWORD*)((DWORD)pBuf + i) ) ;
				}
				else
				{
					sprintf(szTmpOneBuf ,",0X%08X" ,*(DWORD*)((DWORD)pBuf + i) ) ;
				}
			}
			SetFilePointer(hCurFile ,NULL ,NULL ,FILE_END) ;
			WriteFile(hCurFile , szTmpOneBuf ,strlen(szTmpOneBuf) , &dwWrittenRet ,NULL) ;
		}
	}
	delete []pBuf ;
	pBuf = NULL ;
	CloseHandle(hCurFile) ;
}
