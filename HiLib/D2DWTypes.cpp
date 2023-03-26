#include "D2DWTypes.h"

CRectF CRectU2CRectF(const CRectU& rc)
{
	return CRectF(static_cast<FLOAT>(rc.left), static_cast<FLOAT>(rc.top), static_cast<FLOAT>(rc.right), static_cast<FLOAT>(rc.bottom));
}

CRectU CRectF2CRectU(const CRectF& rc)
{
	return CRectU(static_cast<UINT32>(std::round(rc.left)),
		static_cast<UINT32>(std::round(rc.top)),
		static_cast<UINT32>(std::round(rc.right)),
		static_cast<UINT32>(std::round(rc.bottom)));
}

CSizeU CSizeF2CSizeU(const CSizeF& sz)
{
	return CSizeU(static_cast<UINT32>(std::round(sz.width)), static_cast<UINT32>(std::round(sz.height)));
}

CSizeF CSizeU2CSizeF(const CSizeU& sz)
{
	return CSizeF(static_cast<FLOAT>(sz.width), static_cast<FLOAT>(sz.height));
}