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

HWND g_hDlgModeless=NULL;
//std::unique_ptr<ThreadPool> g_pThreadPool;

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

int cppFun(int x)
{
	std::cout << "Inside C++" << std::endl;
	return x + 3;
}

template<class T>
boost::python::list std_vector_to_py_list(const std::vector<T>& v)
{
	boost::python::object get_iter = boost::python::iterator<std::vector<T> >();
	boost::python::object iter = get_iter(v);
	boost::python::list l(iter);
	return l;
}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	//Initialize COM
	CCoInitializer coinit;
	COleInitializer oleinit;

	//Locale
	//std::locale::global(std::locale("en_US.UTF-8"));

	//Detect memory leaks

	//Catch SEH exception as CEH
	_set_se_translator (CSEHException::TransferSEHtoCEH);
	//Deserialize
	auto spWnd = DeserializeProperty();
	g_spApplicationProperty = spWnd->GetApplicationProperty();
	//g_spApplicationProperty->m_bDebug = true;

	//Console
	FILE* fp=NULL;
	if(g_spApplicationProperty->m_bDebug){
		::AllocConsole();
		::_wfreopen_s(&fp,L"CON",L"r",stdin);
		::_wfreopen_s(&fp,L"CON",L"w",stdout);
	}
	try {
		//using namespace boost::python;
		//Py_Initialize();
		//object global_ns = import("__main__").attr("__dict__");

		//std::string script = CFile::ReadAllString<char>(L"C:\\Users\\kuuna\\AppData\\Local\\Programs\\Python\\Python37-32\\pdf.py");

		//exec(script.c_str(), global_ns, global_ns);
		//object pythonFun = global_ns["pdf_split"];

		//boost::python::list li;
		//li.append("C:\\Users\\kuuna\\Desktop\\FanHub_Op500_OI_10244754-01.pdf");
		//pythonFun(li);

		//std::cout << extract<int>(v) << std::endl;
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	//g_pThreadPool.reset(new ThreadPool(3, 100));

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
	SerializeProperty(spWnd);

	return uMsg.wParam;
}