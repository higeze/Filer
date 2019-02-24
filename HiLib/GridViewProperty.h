#pragma once
#include "CellProperty.h"

struct GridViewProperty
{
public:
	std::shared_ptr<BackgroundProperty> BackgroundPropPtr;
	std::shared_ptr<HeaderProperty> HeaderPropPtr; 
	std::shared_ptr<CellProperty> CellPropPtr; 
	std::shared_ptr<int> DeltaScrollPtr;

	GridViewProperty()
		:HeaderPropPtr(std::make_shared<HeaderProperty>()),
		BackgroundPropPtr(std::make_shared<BackgroundProperty>()),
		CellPropPtr(std::make_shared<CellProperty>()),
		DeltaScrollPtr(std::make_shared<int>(30)){}
	virtual ~GridViewProperty(){}

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("BackgroundProperty", BackgroundPropPtr);
		ar("HeadersProperty",HeaderPropPtr);
		ar("CellsProperty",CellPropPtr);
		ar("DeltaScroll",DeltaScrollPtr);
    }
};

