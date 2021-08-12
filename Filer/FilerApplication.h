#pragma once
#include "Application.h"
#include "FilerWnd.h"

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

	std::unique_ptr<CCoInitializer> m_pCoinit;
	std::unique_ptr<COleInitializer> m_pOleinit;
	//RoInitialize(RO_INIT_SINGLETHREADED);

	std::unique_ptr<CFilerWnd> m_pWnd;

	template<int TEnum, typename T>
	void SerializeValue(const T& value)
	{
		std::string path = std::get<static_cast<int>(TEnum)>(GetJsonPaths());
		SerializeValue(path, value);
	}

	template<typename T>
	void SerializeValue(const std::string& path, const T & value)
	{
		json js = value;
		std::ofstream o(path);
		o << std::setw(4) << js << std::endl;
	}

	template<int TEnum, typename T>
	T DeserializeValue()
	{
		std::string path = std::get<static_cast<int>(TEnum)>(GetJsonPaths());
		return DeserializeValue<T>(path);
	}

	template<typename T>
	T DeserializeValue(const std::string& path)
	{
		json j;
		std::ifstream i(path);
		i >> j;
		return j;
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
