// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#if defined _M_IX86
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "FilerWnd.h"
#include "MyString.h"
//#include "FilerProperty.h"
#include "MyXmlSerializer.h"
#include "Resource.h"
#include "ApplicationProperty.h"
#include "MyCom.h"
#include "Ole2.h"
#include "SEHException.h"
#include "ThreadPool.h"
#include "MyFile.h"
#include "CellProperty.h"
#include "Console.h"
#include "Textbox.h"

HWND g_hDlgModeless = NULL;
CConsole g_console;

std::shared_ptr<CApplicationProperty> g_spApplicationProperty(std::make_shared<CApplicationProperty>());

std::shared_ptr<CFilerWnd> DeserializeProperty()
{
	//Get XML Path
	std::wstring wstrXMLPath;
	::GetModuleFileName(NULL,GetBuffer(wstrXMLPath,MAX_PATH),MAX_PATH);
	ReleaseBuffer(wstrXMLPath);
	std::wregex rx(REGEX_FILENAME_W);
	wstrXMLPath=std::regex_replace(wstrXMLPath,rx,std::wstring(L"Filer.xml"));
	//Deserialize
	std::shared_ptr<CFilerWnd> spProp;
	bool bSerialized(false);
	if(::GetFileAttributes(wstrXMLPath.c_str())==0xffffffff){ 
	}else{
		try{
			//Serialize
			CXMLSerializer<std::shared_ptr<CFilerWnd>> serializer;
			serializer.Deserialize(wstrXMLPath.c_str(),L"Filer",spProp);	
			bSerialized=true;
		}catch(/*_com_error &e*/...){
		
		}
	}

	if(!bSerialized)spProp=std::make_shared<CFilerWnd>();

	return spProp;
}

void SerializeProperty(CFilerWnd* pProp)
{
	//Get XML Path
	std::wstring wstrXMLPath;
	::GetModuleFileName(NULL,GetBuffer(wstrXMLPath,MAX_PATH),MAX_PATH);
	ReleaseBuffer(wstrXMLPath);
	std::wregex rx(REGEX_FILENAME_W);
	wstrXMLPath=std::regex_replace(wstrXMLPath,rx,std::wstring(L"Filer.xml"));
	//Serialize
	try{
		//Serialize
		CXMLSerializer<CFilerWnd*> serializer;
		serializer.Serialize(wstrXMLPath.c_str(),L"Filer", pProp);
	}catch(/*_com_error &e*/...){
	}
}

void SerializeProperty(std::shared_ptr<CFilerWnd> spProp)
{
	SerializeProperty(spProp.get());
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	//Logger
//	auto loggersink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("FilerLog.txt", 1024*1024, 5, false);
//	auto logger = std::make_shared<spdlog::async_logger>("logger", loggersink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	auto logger = spdlog::rotating_logger_mt("logger", "filer.log", 1024 * 1024, 3, false);
	spdlog::set_default_logger(logger);
	spdlog::info("Application Start");

	MSG uMsg;
	{
		//Initialize COM
		CCoInitializer coinit;
		COleInitializer oleinit;

		//Locale
		//std::locale::global(std::locale("en_US.UTF-8"));

		//Detect memory leaks

		//Catch SEH exception as CEH
		_set_se_translator(CSEHException::TransferSEHtoCEH);
		//Deserialize
		auto spWnd = DeserializeProperty();
		g_spApplicationProperty = spWnd->GetApplicationProperty();

		D2DTextbox::AppTSFInit();

		spWnd->Create(NULL);
		spWnd->ShowWindow(SW_SHOW);
		spWnd->UpdateWindow();

		BOOL bRet;
		while ((bRet = GetMessage(&uMsg, NULL, 0, 0)) != 0) {
			if (bRet == -1) {
				break;
			} else if ((g_hDlgModeless == (HWND)NULL || !IsDialogMessage(g_hDlgModeless, &uMsg))) {// &&
				TranslateMessage(&uMsg);
				DispatchMessage(&uMsg);
			}
		}

		D2DTextbox::AppTSFExit();
		//Serialize
		SerializeProperty(spWnd);
	}
	spdlog::info("Application Finish");

	spdlog::drop_all();
	return uMsg.wParam;
}