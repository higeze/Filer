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
		std::ifstream i(path);
		json j;
		i >> j;
		j.get_to(m_pWnd);
	}

	if(!m_pWnd){
		m_pWnd = std::make_shared<CFilerWnd>();
	}
}

void CFilerApplication::Serialize()
{
	json j = m_pWnd;
	auto path = GetJsonPath();
	std::ofstream o(path);
	o << std::setw(4) << j << std::endl;
}

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
	m_pWnd->Close.subscribe([this]() {Serialize(); }, m_pWnd);

	//Create
	m_pWnd->Create(NULL);
	CTSFManager::GetInstance()->GetKeyTraceEventSinkPtr()->SetHwnd(m_pWnd->m_hWnd);

	//Show & Update
	m_pWnd->ShowWindow(SW_SHOW);
	m_pWnd->UpdateWindow();

	//Hook
	//SetHook(m_pWnd->m_hWnd);
}
#include "ThreadPool.h"
void CFilerApplication::Term()
{
	//COM, OLE
	m_pCoinit.release();
	m_pOleinit.release();

	//Serialize
	//Serialize();

	//Window
	m_pWnd.reset();

	::OutputDebugStringW(CThreadPool::GetInstance()->OutputString().c_str());

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




