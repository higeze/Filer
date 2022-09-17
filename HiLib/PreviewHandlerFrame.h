#pragma once
#include "Unknown.h"

class CPreviewHandlerFrame : public CUnknown<IPreviewHandlerFrame>
{
public:
	CPreviewHandlerFrame() : CUnknown() { }

	IFACEMETHODIMP GetWindowContext(PREVIEWHANDLERFRAMEINFO* pinfo)
	{
		return S_OK;
	}

	IFACEMETHODIMP TranslateAccelerator(MSG* pmsg)
	{
		return S_OK;
	}
};

