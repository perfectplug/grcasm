// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"


HINSTANCE        hinst;                // DLL instance
HWND             hwmain;               // Handle of main OllyDbg window
char             grcasmwinclass[32]; // Name of grcasm window class
BOOL bAnalysecode = FALSE ;
PBYTE gpbyteFuckArea =NULL ;

// Entry point into a plugin DLL. Many system calls require DLL instance
// which is passed to DllEntryPoint() as one of parameters. Remember it.
// Preferrable way is to place initializations into ODBG_Plugininit() and
// cleanup in ODBG_Plugindestroy().
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hinst = hModule ;
		break ;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

// ODBG_Plugindata() is a "must" for valid OllyDbg plugin. It must fill in
// plugin name and return version of plugin interface. If function is absent,
// or version is not compatible, plugin will be not installed. Short name
// identifies it in the Plugins menu. This name is max. 31 alphanumerical
// characters or spaces + terminating '\0' long. To keep life easy for users,
// this name should be descriptive and correlate with the name of DLL.
extc int _export cdecl ODBG_Plugindata(char shortname[32]) {
	strcpy(shortname,"grcasm");       // Name of plugin
	return PLUGIN_VERSION;
};

// OllyDbg calls this obligatory function once during startup. Place all
// one-time initializations here. If all resources are successfully allocated,
// function must return 0. On error, it must free partially allocated resources
// and return -1, in this case plugin will be removed. Parameter ollydbgversion
// is the version of OllyDbg, use it to assure that it is compatible with your
// plugin; hw is the handle of main OllyDbg window, keep it if necessary.
// Parameter features is reserved for future extentions, do not use it.
extc int _export cdecl ODBG_Plugininit(
	int ollydbgversion,HWND hw,ulong *features) 
{
   // This plugin uses all the newest features, check that version of OllyDbg is
   // correct. I will try to keep backward compatibility at least to v1.99.
   if (ollydbgversion<PLUGIN_VERSION)
	   return -1;
   // Keep handle of main OllyDbg window. This handle is necessary, for example,
   // to display message box.
   hwmain=hw;

   // Plugin successfully initialized. Now is the best time to report this fact
   // to the log window. To conform OllyDbg look and feel, please use two lines.
   // The first, in black, should describe plugin, the second, gray and indented
   // by two characters, bears copyright notice.
   _Addtolist(0,0,"grcasm plugin v1.0.0");
   _Addtolist(0,-1,"  Copyright (C) 2008-2013 完美辅游");

   return 0;
};

// Each window class needs its own window procedure. Both standard and custom
// OllyDbg windows must pass some system and OllyDbg-defined messages to
// Tablefunction(). See description of Tablefunction() for more details.
LRESULT CALLBACK Bookmarkwinproc(HWND hw,UINT msg,WPARAM wp,LPARAM lp) {
	int i,shiftkey,controlkey;
	HMENU menu;
	switch (msg) 
	{
		// Standard messages. You can process them, but - unless absolutely sure -
		// always pass them to Tablefunction().
	
	default: 
		break;
	};
	return DefMDIChildProc(hw,msg,wp,lp);
};

// If you define ODBG_Pluginmainloop, this function will be called each time
// from the main Windows loop in OllyDbg. If there is some debug event from
// the debugged application, debugevent points to it, otherwise it is NULL. Do
// not declare this function unnecessarily, as this may negatively influence
// the overall speed!
extc void _export cdecl ODBG_Pluginmainloop(DEBUG_EVENT *debugevent) 
{
};

// Record types must be unique among OllyDbg and all plugins. The best way to
// assure this is to register record type by OllDbg (Oleh Yuschuk). Registration
// is absolutely free of charge, except for email costs :)
#define TAG_BOOKMARK   0x236D420AL     // Bookmark record type in .udd file

// Time to save data to .udd file! This is done by calling Pluginsaverecord()
// for each data item that must be saved. Global, process-oriented data must
// be saved in main .udd file (named by .exe); module-relevant data must be
// saved in module files. Don't forget to save all addresses relative to
// module's base, so that data will be restored correctly even when module is
// relocated.
extc void _export cdecl ODBG_Pluginsaveudd(t_module *pmod,int ismainmodule) 
{

};

// OllyDbg restores data from .udd file. If record belongs to plugin, it must
// process record and return 1, otherwise it must return 0 to pass record to
// other plugins. Note that module descriptor pointed to by pmod can be
// incomplete, i.e. does not necessarily contain all informations, especially
// that from .udd file.
extc int _export cdecl ODBG_Pluginuddrecord(t_module *pmod,int ismainmodule,
	ulong tag,ulong size,void *data) 
{
		return 1;                            // Record processed
};

// Function adds items either to main OllyDbg menu (origin=PM_MAIN) or to popup
// menu in one of standard OllyDbg windows. When plugin wants to add own menu
// items, it gathers menu pattern in data and returns 1, otherwise it must
// return 0. Except for static main menu, plugin must not add inactive items.
// Item indices must range in 0..63. Duplicated indices are explicitly allowed.
extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item) 
{
	switch (origin) {
		// Menu creation is very simple. You just fill in data with menu pattern.
		// Some examples:
		// 0 Aaa,2 Bbb|3 Ccc|,,  - linear menu with 3items, relative IDs 0, 2 and 3,
		//                         separator between second and third item, last
		//                         separator and commas are ignored;
		// #A{0Aaa,B{1Bbb|2Ccc}} - unconditional separator, followed by popup menu
		//                         A with two elements, second is popup with two
		//                         elements and separator inbetween.
	case PM_MAIN:                      // Plugin menu in main window
		strcpy(data,"0 &option |1 &about");
		// If your plugin is more than trivial, I also recommend to include Help.
		return 1;
	case PM_DISASM:
		strcpy(data,"0 &get runable ASM code to file , 1 &get BYTE Array (HEX) to file , 2 &get WORD Array (HEX) to file , 3 &get DWORD Array (HEX) to file |");
		// If your plugin is more than trivial, I also recommend to include Help.
		return 1 ;
	case PM_CPUDUMP:
		strcpy(data,"0 &get BYTE Array (HEX) to file , 1 &get WORD Array (HEX) to file , 2 &get DWORD Array (HEX) to file ");
		return 1;
	default: 
		break;                    // Any other window
	};
	return 0;                            // Window not supported by plugin
};

// This optional function receives commands from plugin menu in window of type
// origin. Argument action is menu identifier from ODBG_Pluginmenu(). If user
// activates automatically created entry in main menu, action is 0.
extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item) 
{
	t_dump *pd =NULL;
	t_module *pm = NULL;
	int iret = 0 ;
	ulong ulSelectStart = 0 ,ulSelectEnd = 0 ;
	if (origin==PM_MAIN) 
	{
		//主菜单
		switch (action) 
		{
		  case 0:
			  //设置
			  MessageBox(hwmain,
				  "set opiton..." ,
				  "grcasm plugin",MB_OK|MB_ICONINFORMATION);
			  break;
		  case 1:
			  //关于
			  MessageBox(hwmain,
				  "grcasm plugin v1.1.3 扣扣更健康\n"
				  "Copyright (C) 2008-2013 完美辅游",
				  "grcasm plugin",MB_OK|MB_ICONINFORMATION);
			  break;
		  default: 
			  break;
		}; 
	}
	else if (origin==PM_DISASM) 
	{
		//反汇编窗口
		ulong ulstart = 0 ,ulend = 0 ,ulprev = 0 ,ulnext = 0 ;
		pd = (t_dump *)item ;
		if (pd)
		{
			switch (action) 
			{
			case 0:
				//抠取代码存放至文件
				if (bAnalysecode == FALSE)
				{
					//分析代码
					pm = _Findmodule(pd->sel0) ;
					iret = _Analysecode(pm) ;
					_Addtolist(pd->sel0 , 0 ,"analysecode result is %d\r\n" ,iret) ;
					bAnalysecode = TRUE ;
				}
				//设置代码起始地址和结束地址
				dwCodeSegStart = pd->base ;
				dwCodeSegEnd = dwCodeSegStart + pd->size ;
				//获取选中函数信息
				ulstart = _Findprocbegin(pd->sel0) ;
				ulend = _Findprocend(pd->sel0) ;
				ulprev = _Findprevproc(pd->sel0) ;
				ulnext = _Findnextproc(pd->sel0) ;
				_Addtolist(pd->sel0 , 0 ,
					"base:%X ,size:%X \r\n" ,pd->base ,pd->size) ;
				//反汇编显示选中指令
				fGetRunableASMCodeToFile("c:\\" ,pd->sel0 ,pd->sel1-1 ,1) ;
				break;

			case 1:

				//抠取BYTE ARRAY至文件
				fGetArrayHEXToFile(TYPE_GET_BYTE_ARRAY ,pd->sel0 ,pd->sel1) ;
				break ;
			case 2:
				//抠取WORD ARRAY至文件
				fGetArrayHEXToFile(TYPE_GET_WORD_ARRAY ,pd->sel0 ,pd->sel1) ;
				break ;
			case 3:
				//抠取DWORD ARRAY至文件
				fGetArrayHEXToFile(TYPE_GET_DWORD_ARRAY ,pd->sel0 ,pd->sel1) ;
				break ;
			default: 
				break;
			}; 
		}
	}
	else if (origin == PM_CPUDUMP)
	{
		//CPU DUMP窗口
		pd = (t_dump *)item ;
		if (pd)
		{
			switch(action)
			{
			case 0:
				//抠取BYTE ARRAY至文件
				fGetArrayHEXToFile(TYPE_GET_BYTE_ARRAY ,pd->sel0 ,pd->sel1) ;
				break ;
			case 1:
				//抠取WORD ARRAY至文件
				fGetArrayHEXToFile(TYPE_GET_WORD_ARRAY ,pd->sel0 ,pd->sel1) ;
				break ;
			case 2:
				//抠取DWORD ARRAY至文件
				fGetArrayHEXToFile(TYPE_GET_DWORD_ARRAY ,pd->sel0 ,pd->sel1) ;
				break ;
			default: 
				break;
			}

		}
	}
};

// This function receives possible keyboard shortcuts from standard OllyDbg
// windows. If it recognizes shortcut, it must process it and return 1,
// otherwise it returns 0.
extc int _export cdecl ODBG_Pluginshortcut(
	int origin,int ctrl,int alt,int shift,int key,void *item) 
{
	// Plugin accepts shortcuts in form Alt+x or Shift+Alt+x, where x is a key
	// '0'..'9'. Shifted shortcut sets bookmark (only in Disassembler),
	// non-shifted jumps to bookmark from everywhere.
	if (ctrl==0 && alt!=0 && key>='0' && key<='9') 
	{
		if (shift!=0 && origin==PM_DISASM && item!=NULL) 
		{
			// Set new or replace existing bookmark.
			return 1; 
		}                      // Shortcut recognized
		else if (shift==0) 
		{
			// Jump to existing bookmark (from any window).
			return 1;                        // Shortcut recognized
		};
	};
	return 0;                            // Shortcut not recognized
};

// Function is called when user opens new or restarts current application.
// Plugin should reset internal variables and data structures to initial state.
extc void _export cdecl ODBG_Pluginreset(void) 
{
	bAnalysecode = FALSE ;
};

// OllyDbg calls this optional function when user wants to terminate OllyDbg.
// All MDI windows created by plugins still exist. Function must return 0 if
// it is safe to terminate. Any non-zero return will stop closing sequence. Do
// not misuse this possibility! Always inform user about the reasons why
// termination is not good and ask for his decision!
extc int _export cdecl ODBG_Pluginclose(void) {
	// For automatical restoring of open windows, mark in .ini file whether

	return 0;
};

// OllyDbg calls this optional function once on exit. At this moment, all MDI
// windows created by plugin are already destroyed (and received WM_DESTROY
// messages). Function must free all internally allocated resources, like
// window classes, files, memory and so on.
extc void _export cdecl ODBG_Plugindestroy(void) 
{

};

