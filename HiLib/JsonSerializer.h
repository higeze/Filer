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


//extern bool json_create_shared_ptr;

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

//#define ENABLE_IF_ABSTRUCT std::enable_if_t<\
//								std::is_abstract<T>::value,\
//								std::nullptr_t> = nullptr
//
//#define ENABLE_IF_NOTABSTRUCT std::enable_if_t<\
//								!std::is_abstract<T>::value,\
//								std::nullptr_t> = nullptr
//
//struct has_create_make_shared_impl {
//  template <class T>
//  static auto check(T*) -> decltype(
//    create_make_shared(std::declval<std::shared_ptr<T>&>()),
//    std::true_type());
//
//  // óvåèÇñûÇΩÇµÇƒÇ¢Ç»ÇØÇÍÇŒfalse_typeå^Çï‘Ç∑
//  template <class T>
//  static auto check(...) -> std::false_type;
//};
//
//template <class T>
//struct has_create_make_shared
//  : decltype(has_create_make_shared_impl::check<T>(nullptr)) {};
//
//
//template<typename T, typename... TArgs>
//class get_to_t
//{
//public:
//    std::tuple<TArgs...> m_tuple;
//    std::shared_ptr<T>& m_ptr;
//    get_to_t(std::shared_ptr<T>& ptr, TArgs... args)
//        :m_ptr(ptr), m_tuple(args...){}
//    auto operator()(const json& j)
//    {
//        std::string name(typeid(T).name());
//        std::shared_ptr<T> new_ptr = m_ptr ? m_ptr : std::apply([](auto&&... args)
//        {
//            return std::make_shared<T>(args...);
//        }, m_tuple);
//        json_make_shared_map.insert_or_assign(name, [new_ptr]{ return new_ptr; });
//        auto ret = j.get_to(m_ptr);
//        json_make_shared_map.erase(name);
//        return ret;
//    }
//};
//
//template<typename T, typename... TArgs >
//inline get_to_t<T, TArgs...> get_to(std::shared_ptr<T>& ptr, TArgs... args)
//{
//    return get_to_t<T, TArgs...>(ptr, args...);
//}

//template<typename T, typename... TArgs>
//class overload_to_t
//{
//public:
//    std::shared_ptr<T> m_new_ptr;
//    std::shared_ptr<T>& m_ptr;
//    get_to_t(std::shared_ptr<T>& ptr, TArgs... args)
//        :m_ptr(ptr), m_new_ptr(ptr){}
//    auto operator()(const json& j)
//    {
//        std::string name(typeid(T).name());
//        json_make_shared_map.insert_or_assign(name, [new_ptr = m_new_ptr]() { return new_ptr; });
//        auto ret = j.get_to(m_ptr);
//        json_make_shared_map.erase(name);
//        return ret;
//    }
//};
//
//template<typename T, typename... TArgs >
//inline overload_to_t<T, TArgs...> get_to(std::shared_ptr<T>& ptr, TArgs... args)
//{
//    return overload_to_t<T, TArgs...>(ptr, args...);
//}



//template<typename T, typename... TArgs >
//inline typename std::shared_ptr<T> operator|(const json& j, get_to_t<T, TArgs...> func)
//{
//    return func(j);
//}
//
//template<typename T>
//auto get_to_nothrow(const json& j, const char* key, T& o)
//{
//    try {
//        return j.at(key).get_to(o);
//    }catch(...){
//        return o;
//    }
//    
//}
//
//template<typename T, typename... TArgs>
//auto get_to_nothrow(const json& j, const char* key, std::shared_ptr<T>& o, TArgs... args)
//{
//    try {
//        return j.at(key) | get_to(o, args...);
//    }catch(std::exception e){
//        return o = std::make_shared<T>(args...);
//    }
//}



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
//template<typename T>
//auto get_to(const json& j, const char* key, std::shared_ptr<T>& obj)
//{
//    try {
//        if (j.find(key) != j.end()){
//            return j[key].get_to(obj);
//        } else {
//            return obj;
//        }
//    }catch(...){
//        return obj;
//    }
//}
//
//template<typename T>
//auto get(const json& j, const char* key)
//{
//    try {
//        if (j.find(key) != j.end()){
//            return j[key].get<T>();
//        } else {
//            return T();
//        }
//    }catch(...){
//        return T();
//    }
//}
//
//template<typename T>
//auto get_shared_ptr(const json& j, const char* key)
//{
//    try {
//        if (j.find(key) != j.end()){
//            return j[key].get<T>();
//        } else {
//            return std::make_shared<T>();
//        }
//    }catch(...){
//        return std::make_shared<T>();
//    }
//}

namespace nlohmann {


    //template <typename T>
    //struct adl_serializer<std::unique_ptr<T>> {
    //    static void from_json(const json& j, std::unique_ptr<T>&& ptr) {
    //        if (j.is_null()) {
    //            ptr = nullptr;
    //        } else {
    //            ptr = std::make_unique<T>();
    //            json::json_serializer<T, void>::from_json(j, *ptr);
    //        }
    //    }
    //    static void to_json(json& j, const std::unique_ptr<T>& opt) {
    //        if (opt.get()) {
    //            j = *opt;
    //        } else {
    //            j = nullptr;
    //        }
    //    }
    //};

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

        //template<typename U, typename std::enable_if_t<!std::is_abstract<U>::value, std::nullptr_t> = nullptr>
        //static void create_shared_ptr(const std::string name, std::shared_ptr<U>&ptr)
        //{
        //    //make_shared
        //    auto iter_make_shared = json_make_shared_map.find(name);
        //    if (iter_make_shared != json_make_shared_map.end()) {
        //        ptr = std::static_pointer_cast<T>(iter_make_shared->second());
        //    } else {
        //        ptr = std::make_shared<T>();
        //    }
        //}

        //template<typename U, typename std::enable_if_t<std::is_abstract<U>::value, std::nullptr_t> = nullptr>
        //static void create_shared_ptr(const std::string name, std::shared_ptr<U>&ptr)
        //{
        //    //make_shared
        //    auto iter_make_shared = json_make_shared_map.find(name);
        //    if (iter_make_shared != json_make_shared_map.end()) {
        //        ptr = std::static_pointer_cast<T>(iter_make_shared->second());
        //    } else {
        //        ptr = nullptr;
        //    }
        //}

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



