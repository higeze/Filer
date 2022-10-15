#include "D2DWTypes.h"

CRectF CRectU2CRectF(const CRectU& rc)
{
	return CRectF(static_cast<FLOAT>(rc.left), static_cast<FLOAT>(rc.top), static_cast<FLOAT>(rc.right), static_cast<FLOAT>(rc.bottom));
}
