#pragma once

#include "CellProperty.h"

class CGridViewProperty
{
public:
	std::shared_ptr<BackgroundProperty> m_spBackgroundProperty;
	std::shared_ptr<HeaderProperty> m_spPropHeader; 
	std::shared_ptr<CellProperty> m_spPropCell; 
	std::shared_ptr<int> m_spDeltaScroll;

	std::shared_ptr<HeaderProperty> GetHeaderProperty() { return m_spPropHeader; }


	CGridViewProperty()
		:m_spPropHeader(std::make_shared<HeaderProperty>()),
		m_spBackgroundProperty(std::make_shared<BackgroundProperty>()),
		m_spPropCell(std::make_shared<CellProperty>()),
		m_spDeltaScroll(std::make_shared<int>(30))
	{
	
	}

	FRIEND_SERIALIZER

    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("BackgroundProperty",m_spBackgroundProperty);
		ar("HeadersProperty",m_spPropHeader);
		ar("CellsProperty",m_spPropCell);
		ar("DeltaScroll",m_spDeltaScroll);
    }
};

