#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
// https://qiita.com/javacommons/items/9ea0c8fd43b61b01a8da#stdstring-wide_to_utf8const-stdwstring-s
#include <strconv.h>

using json = nlohmann::json;


//shared_ptr
//possible null
//copy ptr
// 
//unique_ptr
//possible null
//copy ptr 
// 
//ptr
//possible null
//copy ptr
//
//polymophic ptr 
//
//   
//normal
//not possible null
//copy constructor

/**********/
/* get_to */
/**********/
template<typename T>
auto json_safe_from(const json& j, const char* key, T& obj) noexcept
{
    try {
        if (j.find(key) != j.end()){
            return j[key].get_to(obj);
        } else {
            return obj;
        }
    } catch(...){
        return obj;
    }
}

template<typename T>
auto json_safe_to(json& j, const char* key, const T& obj) noexcept
{
    try {
        return j[key] = obj;
    } catch (...) {
        return j;
    }
}

/**************/
/* JSON_MACRO */
/**************/

#define NLOHMANN_JSON_TO_NOTHROW(v1) json_safe_to(nlohmann_json_j, #v1, nlohmann_json_t.v1);
#define NLOHMANN_JSON_FROM_NOTHROW(v1) json_safe_from(nlohmann_json_j, #v1, nlohmann_json_t.v1);

#define NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(Type, ...)  \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO_NOTHROW, __VA_ARGS__)) } \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_NOTHROW, __VA_ARGS__)) }

#define NLOHMANN_JSON_M_TO_NOTHROW(v1) json_safe_from_to(nlohmann_json_j, #v1, nlohmann_json_t.m_##v1);
#define NLOHMANN_JSON_M_FROM_NOTHROW(v1) json_safe_from(nlohmann_json_j, #v1, nlohmann_json_t.m_##v1);

#define NLOHMANN_DEFINE_TYPE_M_INTRUSIVE_NOTHROW(Type, ...)  \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_M_TO_NOTHROW, __VA_ARGS__)) } \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_M_FROM_NOTHROW, __VA_ARGS__)) }


/***************/
/* polymophism */
/***************/
template <class T>
class static_object
{
private:
	static T& create()
	{
		static T t;
		return t;
	}

	static_object(static_object const& /*other*/) {}

public:
	static T& get_instance()
	{
		return create();
	}
};

struct json_polymophism_serializer
{
    template<typename T, typename U, typename V>
    json_polymophism_serializer(T&& t, U&& u, V&& v)
        :from_json(std::forward<T>(t)), to_json(std::forward<U>(u)), make_shared(std::forward<V>(v)){}
	std::function<void(const json&, std::shared_ptr<void>&)> from_json;
	std::function<void(json&, const std::shared_ptr<const void>&)> to_json;
	std::function<std::shared_ptr<void>()> make_shared;

};

struct json_polymophism
{
	std::unordered_map<std::string, json_polymophism_serializer> map;
};

template<typename _Base, typename _Derived>
struct json_polymophism_init;

template<typename _Base, typename _Derived>
struct json_polymophism_entry
{
	json_polymophism_entry() {}

	json_polymophism_entry<_Base, _Derived> entry()
	{
        static_object<json_polymophism>::get_instance().map.insert_or_assign(
            typeid(_Derived).name(),
            json_polymophism_serializer(
            [](const json& j, std::shared_ptr<void>& ptr) {
                json::json_serializer<_Derived, void>::from_json(j, *(dynamic_pointer_cast<_Derived>(static_pointer_cast<_Base>(ptr))));
            },
            [](json& j, const std::shared_ptr<const void>& ptr) {
                json::json_serializer<_Derived, void>::to_json(j, *(dynamic_pointer_cast<const _Derived>(static_pointer_cast<const _Base>(ptr))));
                //j = *(dynamic_pointer_cast<const _Derived>(static_pointer_cast<const _Base>(ptr)));
            },
            []() { return std::make_shared<_Derived>(); }
            ));

		return *this;
	}
};

#define JSON_ENTRY_TYPE(...)\
template<>\
struct json_polymophism_init<__VA_ARGS__>\
{\
	static inline json_polymophism_entry<__VA_ARGS__> const b = static_object<json_polymophism_entry<__VA_ARGS__>>::get_instance().entry();\
};

/******************/
/* adl_serializer */
/******************/
namespace nlohmann {

   template<typename T>
   struct adl_serializer<std::unique_ptr<T>>
   {
      static void to_json(json& j, const std::unique_ptr<T>& value)
      {
         if (!value){
            j = nullptr;
         }
         else{
            j = *value;
         }
      }

      static void from_json(json const& j, std::unique_ptr<T>& value)
      {
         if (j.is_null()){
            value.reset();
         }
         else{
            value = std::make_unique<T>();
            json::json_serializer<T, void>::from_json(j, *value);
         }
      }
   };

    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {

        static void from_json(const json& j, std::shared_ptr<T>& sp)
        {

            //if constexpr (std::is_abstract<T>::value){
            //    sp = nullptr;
            //    return;
            //} else {

                //typeinfoname
                std::string name;
                if (j.find("typeinfoname") != j.end()) {
                    j["typeinfoname"].get_to(name);
                } else {
                    name = typeid(T).name();
                }

                //polymophism
                if (auto iter = static_object<json_polymophism>::get_instance().map.find(name);
                    iter != static_object<json_polymophism>::get_instance().map.end()) {
                    //make_shared
                    if (!sp) {
                        sp = std::static_pointer_cast<T>(iter->second.make_shared());
                    }
                    //from_json
                    if (!j.is_null()) {
                        std::shared_ptr<void> spv = std::static_pointer_cast<void>(sp);
                        iter->second.from_json(j, spv);
                        sp = std::static_pointer_cast<T>(spv);
                    }

                } else {
                    if constexpr (std::is_abstract<T>::value) {
                        sp = nullptr;
                    } else {
                        //make_shared
                        if (!sp) {
                            sp = std::make_shared<T>();
                        }
                        //from_json
                        if (!j.is_null()) {
                            json::json_serializer<T, void>::from_json(j, *sp);
                        }
                    }

                }
            //}
        }

        static void to_json(json& j, const std::shared_ptr<T>& sp)
        {
            if (!sp.get()) { return; }
            std::string name = typeid(*sp).name();
            if (auto iter = static_object<json_polymophism>::get_instance().map.find(name);
                iter != static_object<json_polymophism>::get_instance().map.end()) {
                iter->second.to_json(j, sp);
                j["typeinfoname"] = name;
            } else {
                j = *sp;
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
            o = utf8_to_wide(str);
        }
        static void to_json(json& j, const std::wstring& o)
        {
            j = wide_to_utf8(o);
        }

    };

}



