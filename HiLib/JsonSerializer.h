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
        auto base = static_pointer_cast<Base>(ptr);\
        auto derived = dynamic_pointer_cast<Derived>(base);\
        if(!derived){THROW_FILE_LINE_FUNC;}\
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

/**********/
/* get_to */
/**********/
template<typename T>
auto get_to(const json& j, const char* key, T& obj) noexcept
{
    try {
        if (j.find(key) != j.end()){
            return j[key].get_to(obj);
        } else {
            return obj;
        }
    }catch(...){
        return obj;
    }
}

#define NLOHMANN_JSON_TO(v1) nlohmann_json_j[#v1] = nlohmann_json_t.v1;
#define NLOHMANN_JSON_FROM_NOTHROW(v1) get_to(nlohmann_json_j, #v1, nlohmann_json_t.v1);
//nlohmann_json_j.at(#v1).get_to(nlohmann_json_t.v1);

#define NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(Type, ...)  \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_NOTHROW, __VA_ARGS__)) }

#define NLOHMANN_JSON_M_TO(v1) nlohmann_json_j[#v1] = nlohmann_json_t.m_##v1;
#define NLOHMANN_JSON_M_FROM_NOTHROW(v1) get_to(nlohmann_json_j, #v1, nlohmann_json_t.m_##v1);
//nlohmann_json_j.at(#v1).get_to(nlohmann_json_t.v1);

#define NLOHMANN_DEFINE_TYPE_M_INTRUSIVE_NOTHROW(Type, ...)  \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_M_TO, __VA_ARGS__)) } \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_M_FROM_NOTHROW, __VA_ARGS__)) }

/******************/
/* adl_serializer */
/******************/
namespace nlohmann {

   template<typename TRect>
   struct adl_serializer<std::unique_ptr<TRect>>
   {
      static void to_json(json& j, const std::unique_ptr<TRect>& value)
      {
         if (!value){
            j = nullptr;
         }
         else{
            j = *value;
         }
      }

      static void from_json(json const& j, std::unique_ptr<TRect>& value)
      {
         if (j.is_null()){
            value.reset();
         }
         else{
            value = std::make_unique<TRect>();
            json::json_serializer<TRect, void>::from_json(j, *value);
         }
      }
   };

    template <typename TRect>
    struct adl_serializer<std::shared_ptr<TRect>> {

        static void from_json(const json& j, std::shared_ptr<TRect>& ptr) {

            //typeinfoname
            std::string name;
            if (j.find("typeinfoname") != j.end()) {
                j["typeinfoname"].get_to(name);
            } else {
                name = typeid(TRect).name();
            }

            //make_shared
            if (!ptr) {
                auto iter_make_shared = json_make_shared_map.find(name);
                if (iter_make_shared != json_make_shared_map.end()) {
                    ptr = std::static_pointer_cast<TRect>(iter_make_shared->second());
                } else {
                    if constexpr (std::is_abstract<TRect>::value){
                        ptr = nullptr;
                    } else {
                        ptr = std::make_shared<TRect>();
                    }
                }
            }

            //from_json
            if (!j.is_null()) {
                //polymorphic
                auto iter = json_polymorphic_map.find(name);
                if (iter != json_polymorphic_map.end()) {
                    std::shared_ptr<void> p = std:: static_pointer_cast<void>(ptr);
                    iter->second.first(j, p);
                    ptr = std::static_pointer_cast<TRect>(p);
                } else {
                    json::json_serializer<TRect, void>::from_json(j, *ptr);
                }
            }
        }

        static void to_json(json& j, const std::shared_ptr<TRect>& ptr)
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
            o = utf8_to_wide(str);
        }
        static void to_json(json& j, const std::wstring& o)
        {
            j = wide_to_utf8(o);
        }

    };

}



