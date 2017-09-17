#pragma once
#include "MyFriendSerializer.h"
#include "ApplicationProperty.h"
#include "FilerWnd.h"


class CFilerProperty
{
private:
	std::shared_ptr<CFilerWnd> m_spFilerWnd;
	std::shared_ptr<CApplicationProperty> m_spApplicationProperty;
public:
	CFilerProperty()
		:m_spFilerWnd(std::make_shared<CFilerWnd>()),
		 m_spApplicationProperty(std::make_shared<CApplicationProperty>()){}
	~CFilerProperty(){}

	std::shared_ptr<CFilerWnd> GetFilerWndPtr(){return m_spFilerWnd;}
	std::shared_ptr<CApplicationProperty> GetApplicationPropertyPtr(){return m_spApplicationProperty;}


public:
	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("FilerWnd", m_spFilerWnd);
		ar("ApplicationProperty",m_spApplicationProperty);
    }
};

//class CFilerProperty
//{
//public:
//	std::shared_ptr<CApplicationProperty> m_spApplicationProperty;
//	std::shared_ptr<CHeaderProperty> m_spPropHeader; 
//	std::shared_ptr<CCellProperty> m_spPropFilter; 
//	std::shared_ptr<CCellProperty> m_spPropCell; 
//	std::shared_ptr<int> m_spDeltaScroll;
//	//CellsPropertyPtr m_spPropTableHdrs; 
//	//CellsPropertyPtr m_spPropTableCells; 
//
//	CFilerProperty()
//		:m_spApplicationProperty(std::make_shared<CApplicationProperty>()),
//		m_spPropHeader(std::make_shared<CHeaderProperty>()),
//		m_spPropFilter(std::make_shared<CCellProperty>()),
//		m_spPropCell(std::make_shared<CCellProperty>()),
//		m_spDeltaScroll(std::make_shared<int>(30)){}
//
//	std::shared_ptr<CApplicationProperty> GetApplicationPropertyPtr(){return m_spApplicationProperty;}	
//
//	friend class CSerializer;friend class CDeserializer;friend class CCellSerializer;friend CCellDeserializer;
//    template <class Archive>
//    void serialize(Archive& ar)
//    {
//		ar("ApplicationProperty",m_spApplicationProperty);
//		ar("HeadersProperty",m_spPropHeader);
//		ar("FiltersProperty",m_spPropFilter);
//		ar("CellsProperty",m_spPropCell);
//		ar("DeltaScroll",m_spDeltaScroll);
//
//    }
//};
//
//typedef std::shared_ptr<CFilerProperty> FilerPropertyPtr;
