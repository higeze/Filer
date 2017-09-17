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
#include "FilerProperty.h"
#include "MyXmlSerializer.h"
#include "Resource.h"
#include "ApplicationProperty.h"
#include "MyCom.h"
#include "Ole2.h"
#include "SEHException.h"
HWND g_hDlgModeless=NULL;

std::shared_ptr<CApplicationProperty> g_spApplicationProperty(std::make_shared<CApplicationProperty>());

std::shared_ptr<CFilerProperty> DeserializeProperty()
{
	//Get XML Path
	std::wstring wstrXMLPath;
	::GetModuleFileName(NULL,GetBuffer(wstrXMLPath,MAX_PATH),MAX_PATH);
	ReleaseBuffer(wstrXMLPath);
	std::wregex rx(REGEX_FILENAME_W);
	wstrXMLPath=std::regex_replace(wstrXMLPath,rx,std::wstring(L"Filer.xml"));
	//Deserialize
	std::shared_ptr<CFilerProperty> spProp;
	bool bSerialized(false);
	if(::GetFileAttributes(wstrXMLPath.c_str())==0xffffffff){ 
	}else{
		try{
			//Serialize
			CXMLSerializer<std::shared_ptr<CFilerProperty>> serializer;
			serializer.Deserialize(wstrXMLPath.c_str(),L"Filer",spProp);	
			bSerialized=true;
		}catch(/*_com_error &e*/...){
		
		}
	}

	if(!bSerialized)spProp=std::make_shared<CFilerProperty>();

	return spProp;
}

void SerializeProperty(std::shared_ptr<CFilerProperty> spProp)
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
		CXMLSerializer<std::shared_ptr<CFilerProperty>> serializer;
		serializer.Serialize(wstrXMLPath.c_str(),L"Filer",spProp);	
	}catch(/*_com_error &e*/...){
	}
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	//Initialize COM
	CCoInitializer coinit;
	COleInitializer oleinit;

	//Detect memory leaks

	//Catch SEH exception as CEH
	_set_se_translator (CSEHException::TransferSEHtoCEH);
	//Deserialize
	auto spProp = DeserializeProperty();
	g_spApplicationProperty = spProp->GetApplicationPropertyPtr();
	//g_spApplicationProperty->m_bDebug = true;

	//Console
	FILE* fp=NULL;
	if(g_spApplicationProperty->m_bDebug){
		::AllocConsole();
		::freopen_s(&fp,"CON","r",stdin);
		::freopen_s(&fp,"CON","w",stdout);
	}

	std::shared_ptr<CFilerWnd> spWnd = spProp->GetFilerWndPtr();
	HWND hWnd=spWnd->Create(NULL);
	spWnd->ShowWindow(SW_SHOW);
	spWnd->UpdateWindow();

	//Accelerator
	//HACCEL hAccel=::LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCEL_FILER));

	MSG uMsg;
	BOOL bRet;
	while((bRet=GetMessage(&uMsg,NULL,0,0))!=0){
		if(bRet==-1){
			break;
		}else if ((g_hDlgModeless == (HWND) NULL || !IsDialogMessage(g_hDlgModeless, &uMsg))){// &&
		//	((hWnd==uMsg.hwnd)?!TranslateAccelerator(hWnd, hAccel, &uMsg):true)) {

		//}else if(!TranslateAccelerator(hWnd,hAccel,&uMsg) /*|| g_hDlgModeless == NULL || !IsDialogMessage(g_hDlgModeless,&uMsg)*/){
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
		}
	}

	//Serialize
	SerializeProperty(spProp);

	return uMsg.wParam;
}