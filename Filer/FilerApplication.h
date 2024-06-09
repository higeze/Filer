#pragma once
#include <cereal/cereal.hpp>
#include <cereal/macros.hpp>
#include <cereal/specialize.hpp>
#include <cereal/archives/json.hpp>
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

	std::shared_ptr<CFilerWnd> m_pWnd;


	template<typename T>
	void SerializeValue(const std::string& key, const T & value)
	{
		//if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
		//	std::ifstream i(path);
		//	json j;
		//	i >> j;

		//	if (auto iter = j.find(key); iter != j.end()) {
		//		*iter = json{ {key, value} };
		//		std::ofstream o(path);
		//		o << std::setw(4) << j << std::endl;
		//	}
		//}
		//THROW_FILE_LINE_FUNC;
		if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
			std::stringstream stream;
			cereal::JSONOutputArchive jsonOutArchive(stream);
			jsonOutArchive(cereal::make_nvp(key, value));
			std::ofstream outputFile(path, std::ios::out);
			outputFile << stream.str();
			outputFile.close();
			stream.clear();
		}
		THROW_FILE_LINE_FUNC;
	}

	template<typename T>
	T DeserializeValue(const std::string& key)
	{
		//if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
		//	std::ifstream i(path);
		//	json j;
		//	i >> j;

		//	if (auto iter = j.find(key); iter != j.end()) {
		//		return *iter;
		//	}
		//}
		//THROW_FILE_LINE_FUNC;
		if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
			std::stringstream stream;
			std::ifstream inputFile(path, std::ios::in);
			stream << inputFile.rdbuf();
			cereal::JSONInputArchive jsonInputArchive(stream);
			T value;
			jsonInputArchive(cereal::make_nvp(key, value));
			return value;
		}
		THROW_FILE_LINE_FUNC;
	}


public:

	virtual void Init() override;
	//virtual int Run() override;
	virtual void Term() override;
	virtual void Deserialize();
	virtual void Serialize();
	virtual HWND GetHWnd() { return m_pWnd->m_hWnd; }



	//void SerializeLauncher(const std::shared_ptr<CLauncherProperty>& spProp);
	//void SerializeFavorites(const std::shared_ptr<CFavoritesProperty>& spProp);
	//void SerializeExeExtension(const std::shared_ptr<ExeExtensionProperty>& spProp);

	//std::shared_ptr<CLauncherProperty> DeserializeLauncher();
	//std::shared_ptr<CFavoritesProperty> DeserializeFavoirtes();
	//std::shared_ptr<ExeExtensionProperty> DeserializeExeExtension();

};

namespace cereal
{
	template<class A>
	std::string CEREAL_SAVE_MINIMAL_FUNCTION_NAME( A const &, const std::wstring &in )
	{
		return wstr2str( in );
	}

	template<class A>
	void CEREAL_LOAD_MINIMAL_FUNCTION_NAME( A const &, std::wstring &out, const std::string &in )
	{
		out = str2wstr( in );
	}
}

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( std::wstring, cereal::specialization::non_member_load_save_minimal );

//CEREAL_REGISTER_TYPE(CFilerWnd);
