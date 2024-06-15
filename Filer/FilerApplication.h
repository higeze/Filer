#pragma once
#include <cereal/cereal.hpp>
#include <cereal/macros.hpp>
#include <cereal/specialize.hpp>
#include <cereal/archives/json.hpp>
#include "Application.h"
#include "FilerWnd.h"
#include "Debug.h"
#include "se_exception.h"
#include "JsonSerializer.h"

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
		//if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
		//	std::stringstream stream;
		//	cereal::JSONOutputArchive jsonOutArchive(stream);
		//	jsonOutArchive(cereal::make_nvp(key, value));
		//	std::ofstream outputFile(path, std::ios::out);
		//	outputFile << stream.str();
		//	outputFile.close();
		//	stream.clear();
		//}
		//THROW_FILE_LINE_FUNC;
	}

	template<typename T>
	T DeserializeValue(const std::string& key)
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
		//if (auto path = GetJsonPath(); ::PathFileExistsA(path.c_str())) {
		//	std::stringstream stream;
		//	std::ifstream inputFile(path, std::ios::in);
		//	stream << inputFile.rdbuf();
		//	cereal::JSONInputArchive jsonInputArchive(stream);
		//	T value;
		//	jsonInputArchive(cereal::make_nvp(key, value));
		//	return value;
		//}
		//THROW_FILE_LINE_FUNC;
	}


public:

	virtual void Init() override;
	//virtual int Run() override;
	virtual void Term() override;
	virtual void Deserialize();
	virtual void Serialize();
	virtual HWND GetHWnd() { return m_pWnd->m_hWnd; }

	void Test();



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


class Base
{
public:
	Base() {}
	virtual ~Base() = default;
	int base = 1;
	friend void to_json(json& j, const Base& o)
	{
		j["base"] = o.base;
	}
	friend void from_json(const json& j, Base& o)
	{
		j.at("base").get_to(o.base);
	}

};

class Derived:public Base
{
public:
	Derived() {}
	virtual~Derived() = default;
	int derived = 2;
	friend void to_json(json& j, const Derived& o)
	{
		to_json(j, static_cast<const Base&>(o));

		j["derived"] = o.derived;
	}
	friend void from_json(const json& j, Derived& o)
	{
		from_json(j, static_cast<Base&>(o));

		j.at("derived").get_to(o.derived);
	}
};

JSON_ENTRY_TYPE(Base, Derived)


//
//template <class T>
//struct init_binding;
//
//template <class T>
//struct bind_to_archives
//{
//	//! Binding for non abstract types
//	void bind(std::false_type) const
//	{
//		//instantiate_polymorphic_binding(static_cast<T*>(nullptr), 0, Tag{}, adl_tag{});
//	}
//
//	//! Binding for abstract types
//	void bind(std::true_type) const
//	{}
//
//	//! Binds the type T to all registered archives
//	/*! If T is abstract, we will not serialize it and thus
//		do not need to make a binding */
//	bind_to_archives const& bind() const
//	{
//		static_assert(std::is_polymorphic<T>::value,
//			"Attempting to register non polymorphic type");
//		bind(std::is_abstract<T>());
//		return *this;
//	}
//};
//
//

//
//namespace aaa{
//namespace detail{
//
//template <class T>
//class static_object
//{
//	private:
//
//	static T& create()
//	{
//		static T t;
//		return t;
//	}
//
//	static_object(static_object const& /*other*/) {}
//
//	public:
//	static T& getInstance()
//	{
//		return create();
//	}
//
//};
//}}
//
//namespace aaa{
//namespace detail{
//
//template <class T>
//struct bind_to_archives
//{
//
//	bind_to_archives()
//	{
//		auto a = 1;
//	}
//	//! Binding for non abstract types
//	void bind(std::false_type) const
//	{
//		//instantiate_polymorphic_binding(static_cast<T*>(nullptr), 0, Tag{}, adl_tag{});
//	}
//
//	//! Binding for abstract types
//	void bind(std::true_type) const
//	{}
//
//	//! Binds the type T to all registered archives
//	/*! If T is abstract, we will not serialize it and thus
//		do not need to make a binding */
//	bind_to_archives const& bind() const
//	{
//		static_assert(std::is_polymorphic<T>::value,
//			"Attempting to register non polymorphic type");
//		bind(std::is_abstract<T>());
//		return *this;
//	}
//};
//}}
//
//namespace aaa{
//namespace detail{
//
//template <class T>
//struct init_binding;
//}}
//
//namespace aaa{
//namespace detail{
//
//template<>
//struct init_binding<Derived>
//{
//
//	static inline bind_to_archives<Derived> const b = bind_to_archives<Derived>();
//};

//bind_to_archives<Derived> const& init_binding<Derived>::b = ::aaa::detail::static_object<bind_to_archives<Derived>>::getInstance().bind();

//}}




//template <>                                                            \
//struct binding_name<Derived>                                       \
//{                                                                      \
//CEREAL_STATIC_CONSTEXPR char const * name() { return #__VA_ARGS__; } \
//};                                                                     \
//} } /* end namespaces */                                               \
//CEREAL_BIND_TO_ARCHIVES(__VA_ARGS__)


//class reg
//{
//public:
//	static int const& b;
//};
//
//int const & reg::b = 5;

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( std::wstring, cereal::specialization::non_member_load_save_minimal );

CEREAL_REGISTER_TYPE(CFilerWnd);
