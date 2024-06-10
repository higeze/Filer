#include "FilerApplication.h"
#include "Textbox.h"
#include "EditorProperty.h"
#include "JsonSerializer.h"
#include "PDFDoc.h"
#include "TSFManager.h"
#include "FilerWnd.h"

std::string CFilerApplication::GetJsonPath()
{
	std::string dir = GetModuleDirPath<char>();

	std::string path;
	::PathCombineA(::GetBuffer(path, MAX_PATH), dir.c_str(), "Filer.json");
	::ReleaseBuffer(path);
	return path;
}

std::tuple<std::string, std::string, std::string, std::string> CFilerApplication::GetJsonPaths()
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

void CFilerApplication::Deserialize()
{
	if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
		//std::stringstream stream;
		//std::ifstream inputFile(GetJsonPath(), std::ios::in);
		//stream << inputFile.rdbuf();
		//cereal::JSONInputArchive jsonInputArchive(stream);
		//jsonInputArchive(cereal::make_nvp("FilerWnd", m_pWnd));
		std::ifstream file(GetJsonPath(), std::ios::in);
		cereal::JSONInputArchive archive(file);
		archive(cereal::make_nvp("FilerWnd", m_pWnd));
	}



	if(!m_pWnd){
		m_pWnd = std::make_shared<CFilerWnd>();
	}
	//if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
	//	std::ifstream i(path);
	//	json j;
	//	i >> j;
	//	j.get_to(m_pWnd);
	//} else {
	//	m_pWnd = std::make_shared<CFilerWnd>();
	//}
}

void CFilerApplication::Serialize()
{
	//std::stringstream stream;
	//cereal::JSONOutputArchive jsonOutArchive(stream);
	//jsonOutArchive(cereal::make_nvp("FilerWnd", m_pWnd));
	//std::ofstream outputFile(GetJsonPath(), std::ios::out);
	//outputFile << stream.str();
	//outputFile.close();
	//stream.clear();

	std::ofstream file(GetJsonPath(), std::ios::out);
	cereal::JSONOutputArchive archive(file);
	archive(cereal::make_nvp("FilerWnd", m_pWnd));


	//json j = m_pWnd;
	//auto path = GetJsonPath();
	//std::ofstream o(path);
	//o << std::setw(4) << j << std::endl;
}


//std::shared_ptr<CLauncherProperty> CFilerApplication::DeserializeLauncher()
//{
//	return DeserializeValue<std::shared_ptr<CLauncherProperty>>("LauncherProperty");
//}
//
//std::shared_ptr<CFavoritesProperty> CFilerApplication::DeserializeFavoirtes()
//{
//	return DeserializeValue<std::shared_ptr<CFavoritesProperty>>("FavoritesProperty");
//}
//
//std::shared_ptr<ExeExtensionProperty> CFilerApplication::DeserializeExeExtension()
//{
//	return DeserializeValue<std::shared_ptr<ExeExtensionProperty>>("ExeExtensionProperty");
//}

//void CFilerApplication::SerializeLauncher(const std::shared_ptr<CLauncherProperty>& spProp)
//{
//	SerializeValue("LauncherProperty", spProp);
//}
//
//void CFilerApplication::SerializeFavorites(const std::shared_ptr<CFavoritesProperty>& spProp)
//{
//	SerializeValue("FavoritesProperty", spProp);
//}

//void CFilerApplication::SerializeExeExtension(const std::shared_ptr<ExeExtensionProperty>& spProp)
//{
//	SerializeValue("ExeExtensionProperty", spProp);
//}
//#include "PreviewHandlerFrame.h"
//#include <propkey.h>
//
//CPreviewHandlerFrame test;
//CComPtr<IPreviewHandler> handler;
//
//HRESULT Open(HWND hWnd, const std::wstring& path)
//{
//	CComPtr<IShellItem> item1;
//	::SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(&item1));
//	CComQIPtr<IShellItem2> item2(item1);
//
//	CComHeapPtr<wchar_t> ext;
//	FAILED_THROW(item2->GetString(PKEY_ItemType, &ext));
//
//	CComHeapPtr<wchar_t> parsingName;
//	FAILED_THROW(item2->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEPARSING, &parsingName));
//
//	CComPtr<IQueryAssociations> assoc;
//	FAILED_THROW(item2->BindToHandler(NULL, BHID_AssociationArray, IID_PPV_ARGS(&assoc)));
//
//	WCHAR sclsid[48] = { 0 };
//	DWORD size = 48;
//	FAILED_THROW(assoc->GetString(ASSOCF_INIT_DEFAULTTOSTAR, ASSOCSTR_SHELLEXTENSION, L"{8895b1c6-b41f-4c1c-a562-0d564250836f}", sclsid, &size));
//
//	CLSID clsid;
//	SHCLSIDFromString(sclsid, &clsid);
//	FAILED_THROW(handler.CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER));
//
//	CComPtr<IInitializeWithItem> iitem;
//	if (SUCCEEDED(handler->QueryInterface(&iitem)))
//	{
//		FAILED_THROW(iitem->Initialize(item2, STGM_READ));
//	}
//	else
//	{
//		CComPtr<IInitializeWithFile> ifile;
//		if (SUCCEEDED(handler->QueryInterface(&ifile)))
//		{
//			FAILED_THROW(ifile->Initialize(parsingName, STGM_READ));
//		}
//		else
//		{
//			CComPtr<IInitializeWithStream> istream;
//			FAILED_THROW(handler->QueryInterface(&istream));
//
//			CComPtr<IStream> stream;
//			FAILED_THROW(SHCreateStreamOnFile(parsingName, STGM_READ, &stream));
//			FAILED_THROW(istream->Initialize(stream, STGM_READ));
//		}
//	}
//
//	CComPtr<IObjectWithSite> site;
//	if (SUCCEEDED(handler->QueryInterface(&site)))
//	{
//		site->SetSite(&test);
//	}
//
//	RECT rc;
//	GetClientRect(hWnd, &rc);
//	rc.right = (rc.left + rc.right) / 2;
//
//	FAILED_THROW(handler->SetWindow(hWnd, &rc));
//	FAILED_THROW(handler->DoPreview());
//
//	return 0;
//}



void CFilerApplication::Init()
{
	//SEH
	m_pSETrans = std::make_unique<scoped_se_translator>();
	//Logger
	//auto loggersink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("FilerLog.txt", 1024*1024, 5, false);
	//auto logger = std::make_shared<spdlog::async_logger>("logger", loggersink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	auto logger = spdlog::rotating_logger_mt("logger", "filer.log", 1024 * 1024, 3, false);
	spdlog::set_default_logger(logger);
	spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e\t%P\t%t\t%l\t%v");
	LOG_1("***Application Start***");

	//COM, OLE
	m_pCoinit = std::make_unique<CCoInitializer>();
	m_pOleinit = std::make_unique<COleInitializer>();

	//TSF
	CTSFManager::GetInstance()->Init();

	//PDFium
	CPDFDoc::Init();

	//Deserialize
	Deserialize();

	//Create
	m_pWnd->Create(NULL);
	CTSFManager::GetInstance()->GetKeyTraceEventSinkPtr()->SetHwnd(m_pWnd->m_hWnd);

	//Show & Update
	m_pWnd->ShowWindow(SW_SHOW);
	m_pWnd->UpdateWindow();

	//Hook
	//SetHook(m_pWnd->m_hWnd);

	//Open(m_pWnd->m_hWnd, LR"(C:\Users\kuuna\Downloads\VersyPDF (1).pdf)");
}

//int CFilerApplication::Run()
//{
//	BOOL fResult;
//	//BOOL fEaten;
//	MSG msg;
//        BOOL focus;
//
//	while (SUCCEEDED(CTSFManager::GetInstance()->GetMessagePumpPtr()->GetMessageW(&msg, NULL, 0, 0, &fResult))) {
//        //CTSFManager::GetInstance()->GetThreadMgrPtr()->IsThreadFocus(&focus);
//        //OutputDebugStringA(focus ? "" : "~");
//		if (fResult == 0 || fResult == -1) {
//			break;
//		} else /*if (msg.message == WM_KEYDOWN) {
//			if (CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
//				CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten) {
//				continue;
//			}
//
//        } else if (msg.message == WM_KEYUP){
//			if (CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
//				CTSFManager::GetInstance()->GetKeystrokeMgrPtr()->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten) {
//				continue;
//			}
//		}*/
//			
//		if ((m_hDlgModeless == (HWND)NULL || !IsDialogMessage(m_hDlgModeless, &msg))) {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//	}
//	return msg.wParam;
//}

void CFilerApplication::Term()
{
	//COM, OLE
	m_pCoinit.release();
	m_pOleinit.release();

	//Serialize
	Serialize();

	//Window
	m_pWnd.reset();

	//TSF
	CTSFManager::GetInstance()->Term();

	//PDFium to be called after Window destructed
	CPDFDoc::Term();

	//Logger
	LOG_1("***Application Finish***");
	spdlog::drop_all();
	//SEH
	m_pSETrans.release();

	//Hook
	//ResetHook();
}





