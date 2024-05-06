//#pragma once
//#include "CellProperty.h"
//
//struct SheetProperty
//{
//public:
//	//std::shared_ptr<BackgroundProperty> BackgroundPropPtr;
//	std::shared_ptr<HeaderProperty> HeaderPropPtr;
//	std::shared_ptr<CellProperty> CellPropPtr;
//
//	SheetProperty()
//		: BackgroundPropPtr(std::make_shared<BackgroundProperty>()),
//		HeaderPropPtr(std::make_shared<HeaderProperty>()),
//		CellPropPtr(std::make_shared<CellProperty>())
//	{}
//	virtual ~SheetProperty() = default;
//
//	template <class Archive>
//	void serialize(Archive& ar)
//	{
//		ar("BackgroundProperty", BackgroundPropPtr);
//		ar("HeadersProperty", HeaderPropPtr);
//		ar("CellsProperty", CellPropPtr);
//	}
//
//	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SheetProperty,
//		BackgroundPropPtr,
//		HeaderPropPtr,
//		CellPropPtr)
//};
