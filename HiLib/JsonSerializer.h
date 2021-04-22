#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

/**********/
/* GLOBAL */
/**********/

std::unordered_map<std::string,
    std::pair<
        std::function<void(const json&, std::shared_ptr<void>&)>,// from
        std::function<void(json&, const std::shared_ptr<const void>&)> //to
    >>  json_polymorphic_map;

/*********************/
/* POLYMORPHIC MACRO */
/*********************/
#define JSON_REGISTER_POLYMORPHIC_RELATION(Base, Derived)\
json_polymorphic_map.insert_or_assign(\
	typeid(Derived).name(),\
    std::make_pair(\
    [](const json& j, std::shared_ptr<void>& ptr) {\
        ptr = std::make_shared<Derived>();\
	    *(dynamic_pointer_cast<Derived>(static_pointer_cast<Base>(ptr))) = j;},\
    [](json& j, const std::shared_ptr<const void>& ptr) {\
	    j = *(dynamic_pointer_cast<const Derived>(static_pointer_cast<const Base>(ptr)));}\
    )\
)

#define JSON_CLEAR_POLYMORPHIC_RELATION \
	json_polymorphic_map.clear()

#define ENABLE_IF_ABSTRUCT std::enable_if_t<\
								std::is_abstract<T>::value,\
								std::nullptr_t> = nullptr

#define ENABLE_IF_NOTABSTRUCT std::enable_if_t<\
								!std::is_abstract<T>::value,\
								std::nullptr_t> = nullptr



namespace nlohmann {

    std::wstring str2wstr(const std::string& str)
    {
	    //Length
	    auto len = ::MultiByteToWideChar(CP_THREAD_ACP, 0, str.c_str(), -1, NULL,0);
	    std::vector<wchar_t> wstr(len+1,0);
	    //Convert
	    ::MultiByteToWideChar(CP_THREAD_ACP, 0, str.c_str(), -1, &wstr[0], len);

	    return &wstr[0];
    }

    std::string wstr2str(const std::wstring& wstr)
    {
	    //Length
	    auto len = ::WideCharToMultiByte(CP_THREAD_ACP, 0, wstr.c_str(), -1, NULL, 0 ,NULL ,NULL);
	    std::vector<char> str(len+1,'\0');
	    //Convert
	    ::WideCharToMultiByte(CP_THREAD_ACP, 0, wstr.c_str(),wstr.size(), &str[0], len, NULL, NULL);

	    return &str[0];
    }

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
    struct adl_serializer<
        std::shared_ptr<T>,
        typename std::enable_if<!std::is_abstract<T>::value>::type> {
        static void from_json(const json& j, std::shared_ptr<T>& ptr) {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                
                if (j.find("typeinfoname") != j.end()) {
                    std::string name;
                    j["typeinfoname"].get_to(name);
                    auto iter = json_polymorphic_map.find(name);
                    if (iter != json_polymorphic_map.end()) {
                        std::shared_ptr<void> p;
                        iter->second.first(j, p);
                        ptr = std::static_pointer_cast<T>(p);
                    } else {
                        ptr = std::make_shared<T>();
                        //from_json(j, *ptr);
                    }
                } else {
                    ptr = std::make_shared<T>();
                    //from_json(j, *ptr);
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


    template <typename T>
    struct adl_serializer<
        std::shared_ptr<T>,
        typename std::enable_if<std::is_abstract<T>::value>::type>
    {
        static void from_json(const json& j, std::shared_ptr<T>& ptr) {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                
                if (j.find("typeinfoname") != j.end()) {
                    std::string name;
                    j["typeinfoname"].get_to(name);
                    auto iter = json_polymorphic_map.find(name);
                    if (iter != json_polymorphic_map.end()) {
                        std::shared_ptr<void> p;
                        iter->second.first(j, p);
                        ptr = std::static_pointer_cast<T>(p);
                    }
                } else {
                    ptr = nullptr;
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
