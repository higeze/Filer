#pragma once
#include "Unknown.h"

class CDropSource : public CUnknown<IDropSource>
{
public:
	CDropSource();
	~CDropSource();
	
	STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;
	STDMETHODIMP GiveFeedback(DWORD dwEffect) override;
};