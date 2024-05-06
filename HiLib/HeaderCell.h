#pragma once
#include "TextCell.h"

class CColor;

class CHeaderCell:public CTextCell
{
public:
	virtual const SolidFill& GetNormalBackground() const
	{
		static const SolidFill value(239.f / 255.f, 239.f / 255, 232.f / 255.f, 1.0f); return value; //asbestos
	}
	virtual const SolidFill& GetFocusedBackground() const
	{
		static const SolidFill value(195.f / 255.f, 224.f / 255, 226.f / 255.f, 1.0f); return value;
	}
	virtual const SolidFill& GetSelectedOverlay() const
	{
		static const SolidFill value(195.f / 255.f, 224.f / 255, 226.f / 255.f, 0.3f); return value;
	}
	virtual const SolidFill& GetUnfocusSelectedOverlay() const 
	{
		static const SolidFill value(224.f / 255.f, 224.f / 255, 224.f / 255.f, 0.3f); return value;
	}

public:
	using CTextCell::CTextCell;
	virtual ~CHeaderCell(){}
//	virtual void PaintBackground(CDC* pDC,CRect rc);
	virtual bool IsComparable()const{return false;}

private:
//	void TwoColorGradientFill(CDC* pDC,CRect rc,CColor& cr1,CColor& cr2, ULONG ulMode);
};

