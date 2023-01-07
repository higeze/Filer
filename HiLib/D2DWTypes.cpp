#include "D2DWTypes.h"

CRectF CRectU2CRectF(const CRectU& rc)
{
	return CRectF(static_cast<FLOAT>(rc.left), static_cast<FLOAT>(rc.top), static_cast<FLOAT>(rc.right), static_cast<FLOAT>(rc.bottom));
}

CSizeU CSizeF2CSizeU(const CSizeF& sz)
{
	return CSizeU(static_cast<UINT32>(sz.width), static_cast<UINT32>(sz.height));
}