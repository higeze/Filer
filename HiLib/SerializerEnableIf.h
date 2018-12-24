#pragma once
#include <type_traits>
#include <utility>
#include <map>
#include "MyCom.h"
#include "MyMPL.h"
#include "MySerializer.h"

class CSerializer;

#ifndef SERIALIZER_ENABLE_IF
#define SERIALIZER_ENABLE_IF

#define ENABLE_IF_DEFAULT std::enable_if_t<\
								!has_serialize<T,CSerializer>::value &&\
								!has_save<T,CSerializer>::value &&\
								!std::is_enum<T>::value &&\
								!std::is_pointer<T>::value,\
								std::nullptr_t> = nullptr

#define ENABLE_IF_ENUM std::enable_if_t<\
								!has_serialize<T,CSerializer>::value &&\
								!has_save<T,CSerializer>::value &&\
									std::is_enum<T>::value &&\
								!std::is_pointer<T>::value,\
								std::nullptr_t> =  nullptr

#define ENABLE_IF_SERIALIZE std::enable_if_t<\
									has_serialize<T,CSerializer>::value &&\
								!has_save<T,CSerializer>::value &&\
								!std::is_enum<T>::value &&\
								!std::is_pointer<T>::value,\
								std::nullptr_t> = nullptr

#define ENABLE_IF_PTR std::enable_if_t<\
								!has_serialize<T,CSerializer>::value &&\
								!has_save<T,CSerializer>::value &&\
								!std::is_enum<T>::value &&\
									std::is_pointer<T>::value,\
								std::nullptr_t> = nullptr

#define ENABLE_IF_SAVE_LOAD std::enable_if_t<\
								!has_serialize<T,CSerializer>::value &&\
									has_save<T,CSerializer>::value &&\
								!std::is_enum<T>::value &&\
								!std::is_pointer<T>::value,\
								std::nullptr_t> = nullptr

#define ENABLE_IF_ABSTRUCT std::enable_if_t<\
								std::is_abstract<T>::value,\
								std::nullptr_t> = nullptr

#define ENABLE_IF_NOTABSTRUCT std::enable_if_t<\
								!std::is_abstract<T>::value,\
								std::nullptr_t> = nullptr

#endif
