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

#include "Filer.h"
#include "FilerWnd.h"
#include "MyString.h"
//#include "FilerProperty.h"
#include "MyXmlSerializer.h"
#include "Resource.h"
#include "ApplicationProperty.h"
#include "MyCom.h"
#include "Ole2.h"
#include "SEHException.h"
#include "MyFile.h"
#include "CellProperty.h"
#include "Console.h"
#include "Textbox.h"
#include "JsonSerializer.h"


HWND g_hDlgModeless = NULL;
CConsole g_console;

std::shared_ptr<CApplicationProperty> g_spApplicationProperty(std::make_shared<CApplicationProperty>());

std::string GetJsonPath()
{
	std::string path;
	::GetModuleFileNameA(NULL,::GetBuffer(path, MAX_PATH),MAX_PATH);
	::PathRemoveFileSpecA(path.data());
	::PathCombineA(path.data(), path.c_str(), "Filer.json");
	::ReleaseBuffer(path);
	return path;
}

std::tuple<std::string, std::string, std::string, std::string> GetJsonPaths()
{
	std::string dir = GetModuleDirPath<char>();
	
	std::string path;
	::PathCombineA(::GetBuffer(path, MAX_PATH), dir.c_str(), "Filer.json");
	::ReleaseBuffer(path);

	std::string favorite_path;
	::PathCombineA(::GetBuffer(favorite_path, MAX_PATH), dir.c_str(), "filer_favorites.json");
	::ReleaseBuffer(favorite_path);

	std::string launcher_path;
	::PathCombineA(::GetBuffer(launcher_path, MAX_PATH), dir.c_str(), "filer_launcher.json");
	::ReleaseBuffer(launcher_path);

	std::string exeextension_path;
	::PathCombineA(::GetBuffer(exeextension_path, MAX_PATH), dir.c_str(), "filer_exeextension.json");
	::ReleaseBuffer(exeextension_path);

	return { path, favorite_path, launcher_path, exeextension_path };
}



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
	spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e\t%P\t%t\t%l\t%v");
	LOG_1("***Application Start***");

	MSG uMsg;
	{
		//Initialize COM
		CCoInitializer coinit;
		COleInitializer oleinit;

		//RoInitialize(RO_INIT_SINGLETHREADED);

		//Locale
		//std::locale::global(std::locale("en_US.UTF-8"));

		//Catch SEH exception as CEH
		_set_se_translator(CSEHException::TransferSEHtoCEH);
		//TSF
		CTextBox::AppTSFInit();

		//Deserialize
		std::shared_ptr<CFilerWnd> spWnd;
		auto [path, favorites_path, launcher_path, exeextension_path] = GetJsonPaths();

		if (::PathFileExistsA(path.c_str())) {
			json jd;
			std::ifstream i(path);
			i >> jd;
			spWnd = jd; 
		} else {
			spWnd = DeserializeProperty();
		}

		g_spApplicationProperty = spWnd->GetApplicationProperty();

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

		//TSF
		CTextBox::AppTSFExit();

		//Serialize
		//SerializeProperty(spWnd);

		//Serialize to JSON
		auto serialize = [](const std::string& path, const auto& value)
		{
			json js = value;
			std::ofstream o(path);
			o << std::setw(4) << js << std::endl;
		};

		serialize(path, spWnd);
		serialize(favorites_path, spWnd->GetFavoritesPropPtr());
		serialize(launcher_path, spWnd->GetLauncherPropPtr());
		serialize(exeextension_path, spWnd->GetExeExtensionPropPtr());

	}
	LOG_1("***Application Finish***");

	spdlog::drop_all();
	return uMsg.wParam;
}