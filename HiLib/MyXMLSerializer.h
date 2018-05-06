#pragma once

#include <type_traits>
#include <utility>
#include <map>
#include "MyCom.h"
#include "MyMPL.h"
#include "MySerializer.h"
#include "CDataString.h"
#include "MyString.h"
#include "SerializerEnableIf.h"

class CSerializer{
private:
	MSXML2::IXMLDOMDocumentPtr m_pDoc;
	MSXML2::IXMLDOMElementPtr m_pElem;
	CSerializer(){}

public:
	CSerializer(MSXML2::IXMLDOMDocumentPtr pDoc,
		MSXML2::IXMLDOMElementPtr pElem):m_pDoc(pDoc),m_pElem(pElem){}
	virtual ~CSerializer(){}

	static std::map<std::string, std::function<void(CSerializer*, MSXML2::IXMLDOMElementPtr, void*)>> s_dynamicSerializeMap;

	template<class char_type,class T>
	void operator()(const char_type* lpszName, T& tValue)
	{
		MSXML2::IXMLDOMElementPtr pElem = m_pDoc->createElement(_bstr_t(lpszName));
		if (m_pElem) {
			m_pElem->appendChild(pElem);
		} else {
			m_pDoc->appendChild(pElem);
		}
		SerializeValue(tValue, pElem);
	}

	//For base
	template<class T>
	void SerializeValue(T& tValue,MSXML2::IXMLDOMElementPtr pElem,ENABLE_IF_DEFAULT)
	{
		pElem->Puttext(_bstr_t(boost::lexical_cast<std::wstring>(tValue).c_str()));
	}
	
	//For enum
	template<class T>
	void SerializeValue(T& tValue,MSXML2::IXMLDOMElementPtr pElem,ENABLE_IF_ENUM)
	{
		pElem->Puttext(_bstr_t(boost::lexical_cast<std::wstring>(static_cast<int>(tValue)).c_str()));
	}

	//For ptr
	template<class T>
	void SerializeValue(T& tValue, MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_PTR)
	{
		SerializeValue(*tValue, pElem);
	}
	
	//For serialize
	template<class T>
	void SerializeValue(T& tValue,MSXML2::IXMLDOMElementPtr pElem,ENABLE_IF_SERIALIZE)
	{
		tValue.serialize(CSerializer(m_pDoc,pElem));
	}

	//For save load
	template<class T>
	void SerializeValue(T& tValue, MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_SAVE_LOAD)
	{
		tValue.save(CSerializer(m_pDoc, pElem));
	}

	//For vector
	template<class T>
	void SerializeValue(std::vector<T>& tValue,MSXML2::IXMLDOMElementPtr pElem)
	{
		for(auto iter=tValue.begin(),end=tValue.end();iter!=end;++iter){
			MSXML2::IXMLDOMElementPtr pItemElem=m_pDoc->createElement(_bstr_t("item"));
			pElem->appendChild(pItemElem);
			SerializeValue(*iter,pItemElem);
		}
	}

	//For shared_ptr
	template<class T>
	void SerializeValue(std::shared_ptr<T>& tValue, MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_ABSTRUCT)
	{
		auto iter = s_dynamicSerializeMap.find(typeid(*tValue).name());
		if (iter != s_dynamicSerializeMap.end()) {
			pElem->setAttribute(_bstr_t(L"typeinfoname"), _bstr_t(typeid(*tValue).name()));
			return iter->second(this, pElem, tValue.get());
		}
		throw std::exception("No registration for abstruct class");
	}

	template<class T>
	void SerializeValue(std::shared_ptr<T>& tValue, MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_NOTABSTRUCT)
	{
		auto iter = s_dynamicSerializeMap.find(typeid(*tValue).name());
		if (iter != s_dynamicSerializeMap.end()) {
			pElem->setAttribute(_bstr_t(L"typeinfoname"), _bstr_t(typeid(*tValue).name()));
			iter->second(this, pElem, tValue.get());
		} else {
			SerializeValue(*tValue, pElem);
		}
	}
};

class CDeserializer{
private:
	MSXML2::IXMLDOMDocumentPtr m_pDoc;
	MSXML2::IXMLDOMElementPtr m_pElem;
	CDeserializer(){}

public:
	CDeserializer(MSXML2::IXMLDOMDocumentPtr pDoc,
		MSXML2::IXMLDOMElementPtr pElem):m_pDoc(pDoc),m_pElem(pElem){}
	virtual ~CDeserializer(){}

	static std::map<std::string, std::function<void(CDeserializer*, MSXML2::IXMLDOMElementPtr, void*)>> s_dynamicDeserializeMap;
	static std::map<std::string, std::function<std::shared_ptr<void>()>> s_dynamicMakeSharedMap;

	//operator()
	template<class char_type,class T>
	void operator()(const char_type* lpszName,T& tValue)
	{
		std::vector<MSXML2::IXMLDOMElementPtr> vpElem;
		if(m_pElem){
			vpElem=GetChildElementsByTagName(m_pElem,lpszName);
		}else{
			vpElem=GetChildElementsByTagName(m_pDoc,lpszName);		
		}
		if(vpElem.size()==1){
			MSXML2::IXMLDOMElementPtr pElem=vpElem[0];
			DeserializeElement(tValue,pElem);
		}
	}

	template<class char_type, class T, class U>
	void operator()(const char_type* lpszName, T& tValue, U uValue)
	{
		std::vector<MSXML2::IXMLDOMElementPtr> vpElem;
		if (m_pElem) {
			vpElem = GetChildElementsByTagName(m_pElem, lpszName);
		}
		else {
			vpElem = GetChildElementsByTagName(m_pDoc, lpszName);
		}
		if (vpElem.size() == 1) {
			MSXML2::IXMLDOMElementPtr pElem = vpElem[0];
			DeserializeElement(tValue, pElem, uValue);
		}
	}
	
	//For base
	template<class T>
	void DeserializeElement(T& tValue,MSXML2::IXMLDOMElementPtr pElem,ENABLE_IF_DEFAULT)
	{
		tValue=boost::lexical_cast<T>(std::wstring(pElem->Gettext()));
	}

	//For enum
	template<class T>
	void DeserializeElement(T& tValue,MSXML2::IXMLDOMElementPtr pElem,ENABLE_IF_ENUM)
	{
		tValue=static_cast<T>(boost::lexical_cast<int>(std::wstring(pElem->Gettext())));
	}

	//For serialize
	template<class T>
	void DeserializeElement(T& tValue,MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_SERIALIZE)
	{
		tValue.serialize(CDeserializer(m_pDoc,pElem));
	}

	template<class T, class U>
	void DeserializeElement(T& tValue, MSXML2::IXMLDOMElementPtr pElem, U& uValue, ENABLE_IF_SERIALIZE)
	{
		tValue.serialize(CDeserializer(m_pDoc, pElem), uValue);
	}

	//For save load
	template<class T>
	void DeserializeElement(T& tValue, MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_SAVE_LOAD)
	{
		tValue.load(CDeserializer(m_pDoc, pElem));
	}

	template<class T, class U>
	void DeserializeElement(T& tValue, MSXML2::IXMLDOMElementPtr pElem, U& uValue, ENABLE_IF_SAVE_LOAD)
	{
		tValue.load(CDeserializer(m_pDoc, pElem), uValue);
	}

	//For vector
	template<class T>
	void DeserializeElement(std::vector<T>& tValue, MSXML2::IXMLDOMElementPtr pElem)
	{
		std::vector<MSXML2::IXMLDOMElementPtr> vpItemElem=GetChildElementsByTagName(pElem,"item");
		if(!vpItemElem.empty()){
			tValue.resize(vpItemElem.size());
			for(UINT n=0,nSize=vpItemElem.size();n<nSize;++n){
				DeserializeElement(tValue[n],vpItemElem[n]);
			}
		}
	}

	//For shared_ptr
	template<class T>
	void DeserializeElement(std::shared_ptr<T>& tValue, MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_ABSTRUCT)
	{
		_variant_t var = pElem->getAttribute(_bstr_t(L"typeinfoname"));
		if (var.vt != VT_NULL) {

			std::string key;
			::strcpy_s(::GetBuffer(key, 32), 32 * sizeof(char) / sizeof(byte), (_bstr_t)var);
			::ReleaseBuffer(key);

			//Dynamic Make shared
			auto msIter = s_dynamicMakeSharedMap.find(key);
			if (msIter != s_dynamicMakeSharedMap.end()) {

				if (!tValue) {
					tValue = std::static_pointer_cast<T>(msIter->second());
				}

				//Dynamic Deserialize
				auto deIter = s_dynamicDeserializeMap.find(key);
				if (deIter != s_dynamicDeserializeMap.end()) {
					return deIter->second(this, pElem, tValue.get());
				}
			}
		}
		throw std::exception("No registration for abstruct class");
	}

	template<class T>
	void DeserializeElement(std::shared_ptr<T>& tValue, MSXML2::IXMLDOMElementPtr pElem, ENABLE_IF_NOTABSTRUCT)
	{
		_variant_t var = pElem->getAttribute(_bstr_t(L"typeinfoname"));
		if (var.vt != VT_NULL) {

			std::string key;
			::strcpy_s(::GetBuffer(key, 32), 32 * sizeof(char) / sizeof(byte), (_bstr_t)var);
			::ReleaseBuffer(key);

			//Dynamic Make shared
			auto msIter = s_dynamicMakeSharedMap.find(key);
			if (msIter != s_dynamicMakeSharedMap.end()) {

				if (!tValue) {
					tValue = std::static_pointer_cast<T>(msIter->second());
				}

				//Dynamic Deserialize
				auto deIter = s_dynamicDeserializeMap.find(key);
				if (deIter != s_dynamicDeserializeMap.end()) {
					return deIter->second(this, pElem, tValue.get());
				}
			}
		}

		if (!tValue) {
			tValue = std::make_shared<T>();
		}
		DeserializeElement(*tValue, pElem);
	}

	template<class T, class U>
	void DeserializeElement(std::shared_ptr<T>& tValue, MSXML2::IXMLDOMElementPtr pElem, U uValue)
	{
		if (!tValue) {
			tValue = std::make_shared<T>(uValue);
		}
		DeserializeElement(*tValue, pElem);
	}
};

template<class T>
class CXMLSerializer
{
public:
	CXMLSerializer(){}
	virtual ~CXMLSerializer(){}

	template<class char_type>
	void Serialize(const char_type* lpszFilePath,
					const char_type* lpszRootName,
					T& tObj)
	{
		CCoInitializer coinit;
		MSXML2::IXMLDOMDocumentPtr pDoc;
		throw_if_failed(pDoc.CreateInstance("Msxml2.DOMDocument.6.0"));
		throw_if_failed(pDoc->put_async(VARIANT_FALSE));
	
		MSXML2::IXMLDOMProcessingInstructionPtr pProcInst=pDoc->createProcessingInstruction("xml", " version='1.0' encoding='UTF-8'");
		pDoc->appendChild(pProcInst);

		//Serialize
		CSerializer serializer(pDoc,nullptr);
		serializer(lpszRootName,tObj);	
		//Save
		throw_if_failed(pDoc->save(_variant_t(lpszFilePath)));
	}

	template<class char_type>
	void Deserialize(const char_type* lpszFilePath,
					const char_type* lpszRootName,
					T& tObj)
	{
		CCoInitializer coinit;
		MSXML2::IXMLDOMDocumentPtr pDoc;
		throw_if_failed(pDoc.CreateInstance("Msxml2.DOMDocument.6.0"));
		throw_if_failed(pDoc->put_async(VARIANT_FALSE));
		VARIANT_BOOL vtbSuccess=pDoc->load(_variant_t(lpszFilePath));
		throw_if_false(vtbSuccess==VARIANT_TRUE);

		//Deserialize
		CDeserializer deserializer(pDoc,nullptr);
		deserializer(lpszRootName,tObj);		
	}

	template<class char_type>
	MSXML2::IXMLDOMDocumentPtr Serialize(const char_type* lpszRootName, T& tObj)
	{
		CCoInitializer coinit;
		MSXML2::IXMLDOMDocumentPtr pDoc;
		throw_if_failed(pDoc.CreateInstance("Msxml2.DOMDocument.6.0"));
		throw_if_failed(pDoc->put_async(VARIANT_FALSE));
	
		MSXML2::IXMLDOMProcessingInstructionPtr pProcInst=pDoc->createProcessingInstruction("xml", " version='1.0' encoding='UTF-8'");
		pDoc->appendChild(pProcInst);

		//Serialize
		CSerializer serializer(pDoc,nullptr);
		serializer(lpszRootName,tObj);	
		//Save
		return pDoc;
	}

	//template<class char_type>
	//MSXML2::IXMLDOMElementPtr Serialize(MSXML2::IXMLDOMDocumentPtr pDoc,const char_type* lpszRootName, T& tObj)
	//{
	//	CCoInitializer coinit;
	//	MSXML2::IXMLDOMElementPtr pElem=pDoc->createElement(_bstr_t(lpszRootName));
	//	//Serialize
	//	CSerializer serializer(pDoc,pElem);
	//	serializer(lpszRootName,tObj);	
	//	//Save
	//	return pDoc;
	//}

	template<class char_type>
	void Deserialize(MSXML2::IXMLDOMDocumentPtr pDoc, const char_type* lpszRootName, T& tObj)
	{
		//Deserialize
		CDeserializer deserializer(pDoc,nullptr);
		deserializer(lpszRootName,tObj);		
	}
};