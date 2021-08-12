#include "FilerApplication.h"
#include "Textbox.h"
#include "JsonSerializer.h"



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
	auto [path, favorites_path, launcher_path, exeextension_path] = GetJsonPaths();

	if (::PathFileExistsA(path.c_str())) {
		m_pWnd = DeserializeValue<std::unique_ptr<CFilerWnd>>(path);
	} else {
		m_pWnd = std::make_unique<CFilerWnd>();
	}
	if (::PathFileExistsA(favorites_path.c_str())) {
		m_pWnd->SetFavoritesPropPtr(DeserializeValue<std::shared_ptr<CFavoritesProperty>>(favorites_path));
	} else {
		m_pWnd->SetFavoritesPropPtr(std::make_shared<CFavoritesProperty>());
	}
	if (::PathFileExistsA(launcher_path.c_str())) {
		m_pWnd->SetLauncherPropPtr(DeserializeValue<std::shared_ptr<CLauncherProperty>>(launcher_path));
	} else {
		m_pWnd->SetLauncherPropPtr(std::make_shared<CLauncherProperty>());
	}
	if (::PathFileExistsA(exeextension_path.c_str())) {
		m_pWnd->SetExeExtensionPropPtr(DeserializeValue<std::shared_ptr<ExeExtensionProperty>>(exeextension_path));
	} else {
		m_pWnd->SetExeExtensionPropPtr(std::make_shared<ExeExtensionProperty>());
	}
}

void CFilerApplication::Serialize()
{
	auto [path, favorites_path, launcher_path, exeextension_path] = GetJsonPaths();

	SerializeValue(path, m_pWnd);
	SerializeValue(favorites_path, m_pWnd->GetFavoritesPropPtr());
	SerializeValue(launcher_path, m_pWnd->GetLauncherPropPtr());
	SerializeValue(exeextension_path, m_pWnd->GetExeExtensionPropPtr());
}


std::shared_ptr<CLauncherProperty> CFilerApplication::DeserializeLauncher()
{
	return DeserializeValue<static_cast<int>(json_path::launcher), std::shared_ptr<CLauncherProperty>>();
}

std::shared_ptr<CFavoritesProperty> CFilerApplication::DeserializeFavoirtes()
{
	return DeserializeValue<static_cast<int>(json_path::favorites), std::shared_ptr<CFavoritesProperty>>();
}

std::shared_ptr<ExeExtensionProperty> CFilerApplication::DeserializeExeExtension()
{
	return DeserializeValue<static_cast<int>(json_path::exeextension), std::shared_ptr<ExeExtensionProperty>>();
}

void CFilerApplication::SerializeLauncher(const std::shared_ptr<CLauncherProperty>& spProp)
{
	SerializeValue<static_cast<int>(json_path::favorites)>(spProp);
}

void CFilerApplication::SerializeFavorites(const std::shared_ptr<CFavoritesProperty>& spProp)
{
	SerializeValue<static_cast<int>(json_path::favorites)>(spProp);
}

void CFilerApplication::SerializeExeExtension(const std::shared_ptr<ExeExtensionProperty>& spProp)
{
	SerializeValue<static_cast<int>(json_path::exeextension)>(spProp);
}


void CFilerApplication::Init()
{
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
	CTextBox::AppTSFInit();

	//Deserialize
	Deserialize();

	//Window
	m_pWnd->Create(NULL);
	m_pWnd->ShowWindow(SW_SHOW);
	m_pWnd->UpdateWindow();
}

void CFilerApplication::Term()
{
	//COM, OLE
	m_pCoinit.release();
	m_pOleinit.release();

	//TSF
	CTextBox::AppTSFExit();

	//Serialize
	Serialize();

	//Window
	m_pWnd.reset();

	//Logger
	LOG_1("***Application Finish***");
	spdlog::drop_all();

}





