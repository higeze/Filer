#pragma once
#include "Cell.h"
#include "ShellFile.h"
#include <sigslot/signal.hpp>

class CFileIconCell:public CCell
{
protected:
	mutable sigslot::connection m_conDelayUpdateAction;
public:
	using CCell::CCell;
	virtual ~CFileIconCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile() = 0;

	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;

	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override;

	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override;

	virtual void OnMouseEnter(const MouseEnterEvent& e) override;

	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;

};