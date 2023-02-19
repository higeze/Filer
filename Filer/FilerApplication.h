#pragma once
#include "Application.h"
#include "FilerWnd.h"
#include "Debug.h"
#include "se_exception.h"

class CFilerApplication : public CApplication<CFilerApplication>
{
private:
	enum class json_path
	{
		main = 0,
		favorites = 1,
		launcher = 2,
		exeextension = 3,
	};

	std::tuple<std::string, std::string, std::string, std::string> GetJsonPaths();

	std::string GetJsonPath();

	std::unique_ptr<scoped_se_translator> m_pSETrans;
	std::unique_ptr<CCoInitializer> m_pCoinit;
	std::unique_ptr<COleInitializer> m_pOleinit;
	//RoInitialize(RO_INIT_SINGLETHREADED);

	std::unique_ptr<CFilerWnd> m_pWnd;


	template<typename TRect>
	void SerializeValue(const std::string& key, const TRect & value)
	{
		if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
			std::ifstream i(path);
			json j;
			i >> j;

			if (auto iter = j.find(key); iter != j.end()) {
				*iter = json{ {key, value} };
				std::ofstream o(path);
				o << std::setw(4) << j << std::endl;
			}
		}
		THROW_FILE_LINE_FUNC;
	}

	template<typename TRect>
	TRect DeserializeValue(const std::string& key)
	{
		if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
			std::ifstream i(path);
			json j;
			i >> j;

			if (auto iter = j.find(key); iter != j.end()) {
				return *iter;
			}
		}
		THROW_FILE_LINE_FUNC;

	}


public:

	virtual void Init() override;
	virtual void Term() override;
	virtual void Deserialize();
	virtual void Serialize();



	void SerializeLauncher(const std::shared_ptr<CLauncherProperty>& spProp);
	void SerializeFavorites(const std::shared_ptr<CFavoritesProperty>& spProp);
	void SerializeExeExtension(const std::shared_ptr<ExeExtensionProperty>& spProp);

	std::shared_ptr<CLauncherProperty> DeserializeLauncher();
	std::shared_ptr<CFavoritesProperty> DeserializeFavoirtes();
	std::shared_ptr<ExeExtensionProperty> DeserializeExeExtension();

};
