#include "TaskCheckBoxCell.h"
#include "TaskRow.h"

void CTaskCheckBoxCell::PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rc)
{
	return static_cast<CTaskRow*>(m_pRow)->RenderBackground(pDirect, rc);
}