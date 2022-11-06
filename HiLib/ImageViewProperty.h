#pragma once
#include "Direct2DWrite.h"
#include "ScrollProperty.h"

struct ImageViewProperty
{
public:
	std::shared_ptr<SolidLine> FocusedLine;
	std::shared_ptr<SolidFill> NormalFill;

	std::shared_ptr<CRectF> Padding;
	std::shared_ptr<ScrollProperty> VScrollPropPtr;
	std::shared_ptr<ScrollProperty> HScrollPropPtr;


public:
	ImageViewProperty()
		:FocusedLine(std::make_shared<SolidLine>(22.f/255.f, 160.f/255.f, 133.f/255.f, 1.0f, 1.0f)),
		NormalFill(std::make_shared<SolidFill>(246.f/255.f, 246.f/255.f, 246.f/255.f, 1.0f)),
		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)),
		VScrollPropPtr(std::make_shared<ScrollProperty>()),
		HScrollPropPtr(std::make_shared<ScrollProperty>()){};

	virtual ~ImageViewProperty() = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ImageViewProperty,
		FocusedLine,
		NormalFill,
		Padding,
		VScrollPropPtr,
		HScrollPropPtr)
};

