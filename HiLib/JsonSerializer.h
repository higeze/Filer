#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "MyString.h"

//std::wstring str2wstr(const std::string& str)
//{
//	//Length
//	auto len = ::MultiByteToWideChar(CP_THREAD_ACP, 0, str.c_str(), -1, NULL,0);
//	std::vector<wchar_t> wstr(len+1,0);
//	//Convert
//	::MultiByteToWideChar(CP_THREAD_ACP, 0, str.c_str(), -1, &wstr[0], len);
//
//	return &wstr[0];
//}
//
//std::string wstr2str(const std::wstring& wstr)
//{
//	//Length
//	auto len = ::WideCharToMultiByte(CP_THREAD_ACP, 0, wstr.c_str(), -1, NULL, 0 ,NULL ,NULL);
//	std::vector<char> str(len+1,'\0');
//	//Convert
//	::WideCharToMultiByte(CP_THREAD_ACP, 0, wstr.c_str(),wstr.size(), &str[0], len, NULL, NULL);
//
//	return &str[0];
//}


using json = nlohmann::json;

/**********/
/* GLOBAL */
/**********/

extern std::unordered_map<std::string,
    std::pair<
        std::function<void(const json&, std::shared_ptr<void>&)>,// from
        std::function<void(json&, const std::shared_ptr<const void>&)> //to
    >>  json_polymorphic_map;

extern std::unordered_map <
    std::string,
    std::function<std::shared_ptr<void>()>> json_make_shared_map;

/*********************/
/* POLYMORPHIC MACRO */
/*********************/
#define JSON_REGISTER_POLYMORPHIC_RELATION(Base, Derived)\
json_polymorphic_map.insert_or_assign(\
	typeid(Derived).name(),\
    std::make_pair(\
    [](const json& j, std::shared_ptr<void>& ptr) {\
	    json::json_serializer<Derived, void>::from_json(j, *(dynamic_pointer_cast<Derived>(static_pointer_cast<Base>(ptr))));},\
    [](json& j, const std::shared_ptr<const void>& ptr) {\
	    j = *(dynamic_pointer_cast<const Derived>(static_pointer_cast<const Base>(ptr)));}\
    )\
);\
json_make_shared_map.insert_or_assign(\
	typeid(Derived).name(),\
    [](){ return std::make_shared<Derived>();}\
)

#define JSON_CLEAR_POLYMORPHIC_RELATION \
	json_polymorphic_map.clear()

#define ENABLE_IF_ABSTRUCT std::enable_if_t<\
								std::is_abstract<T>::value,\
								std::nullptr_t> = nullptr

#define ENABLE_IF_NOTABSTRUCT std::enable_if_t<\
								!std::is_abstract<T>::value,\
								std::nullptr_t> = nullptr

struct has_create_make_shared_impl {
  template <class T>
  static auto check(T*) -> decltype(
    create_make_shared(std::declval<std::shared_ptr<T>&>()),
    std::true_type());

  // óvåèÇñûÇΩÇµÇƒÇ¢Ç»ÇØÇÍÇŒfalse_typeå^Çï‘Ç∑
  template <class T>
  static auto check(...) -> std::false_type;
};

template <class T>
struct has_create_make_shared
  : decltype(has_create_make_shared_impl::check<T>(nullptr)) {};


template<typename T, typename... TArgs>
class get_to_t
{
public:
    std::shared_ptr<T> m_new_ptr;
    std::shared_ptr<T>& m_ptr;
    get_to_t(std::shared_ptr<T>& ptr, TArgs... args)
        :m_ptr(ptr), m_new_ptr(std::make_shared<T>(args...)){}
    auto operator()(const json& j)
    {
        std::string name(typeid(T).name());
        json_make_shared_map.insert_or_assign(name, [new_ptr = m_new_ptr]() { return new_ptr; });
        auto ret = j.get_to(m_ptr);
        json_make_shared_map.erase(name);
        return ret;
    }
};

template<typename T, typename... TArgs >
inline get_to_t<T, TArgs...> get_to(std::shared_ptr<T>& ptr, TArgs... args)
{
    return get_to_t<T, TArgs...>(ptr, args...);
}


template<typename T, typename... TArgs >
inline typename std::shared_ptr<T> operator|(const json& j, get_to_t<T, TArgs...> func)
{
    return func(j);
}

template<typename T>
auto from_json_nothrow(const json& j, const char* key, T& o)
{
    try {
        return j.at(key).get_to(o);
    }catch(...){
        return o;
    }
    
}

template<typename T, typename... TArgs>
auto from_json_nothrow(const json& j, const char* key, std::shared_ptr<T>& o, TArgs... args)
{
    try {
        return j.at(key) | get_to(o, args...);
    }catch(...){
        return o = std::make_shared<T>(args...);
    }
}


namespace nlohmann {


    template <typename T>
    struct adl_serializer<std::unique_ptr<T>> {
        static void from_json(const json& j, std::unique_ptr<T>& ptr) {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                ptr = std::make_unique<T>();
                *ptr = j;
            }
        }
        static void to_json(json& j, const std::unique_ptr<T>& opt) {
            if (opt.get()) {
                j = *opt;
            } else {
                j = nullptr;
            }
        }
    };


    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {

        template<typename U, typename std::enable_if_t<!std::is_abstract<U>::value, std::nullptr_t> = nullptr>
        static void create_shared_ptr(const std::string name, std::shared_ptr<U>&ptr)
        {
            //make_shared
            auto iter_make_shared = json_make_shared_map.find(name);
            if (iter_make_shared != json_make_shared_map.end()) {
                ptr = std::static_pointer_cast<T>(iter_make_shared->second());
            } else {
                ptr = std::make_shared<T>();
            }
        }

        template<typename U, typename std::enable_if_t<std::is_abstract<U>::value, std::nullptr_t> = nullptr>
        static void create_shared_ptr(const std::string name, std::shared_ptr<U>&ptr)
        {
            //make_shared
            auto iter_make_shared = json_make_shared_map.find(name);
            if (iter_make_shared != json_make_shared_map.end()) {
                ptr = std::static_pointer_cast<T>(iter_make_shared->second());
            } else {
                ptr = nullptr;
            }
        }

        static void from_json(const json& j, std::shared_ptr<T>& ptr) {
            //typeinfoname
            std::string name;
            if (j.find("typeinfoname") != j.end()) {
                j["typeinfoname"].get_to(name);
            } else {
                name = typeid(T).name();
            }
            //make_shared
            create_shared_ptr(name, ptr);

            //from_json
            if (!j.is_null()) {
                //polymorphic
                auto iter = json_polymorphic_map.find(name);
                if (iter != json_polymorphic_map.end()) {
                    std::shared_ptr<void> p = std:: static_pointer_cast<void>(ptr);
                    iter->second.first(j, p);
                    ptr = std::static_pointer_cast<T>(p);
                } else {
                    json::json_serializer<T, void>::from_json(j, *ptr);
                }
            }
        }

        static void to_json(json& j, const std::shared_ptr<T>& ptr)
        {

            if (ptr.get()) {
                std::string name = typeid(*ptr).name();
                auto iter = json_polymorphic_map.find(name);
                if (iter != json_polymorphic_map.end()) {
                    iter->second.second(j, ptr);
                    j["typeinfoname"] = name;
                } else {
                    j = *ptr;
                }
            } else {
                j = nullptr;
            }
        }
    };


    template<>
    struct adl_serializer<std::wstring>
    {
        static void from_json(const json& j, std::wstring& o)
        {
            std::string str;
            j.get_to(str);
            o = str2wstr(str);
        }
        static void to_json(json& j, const std::wstring& o)
        {
            j = wstr2str(o);
        }

    };

}
