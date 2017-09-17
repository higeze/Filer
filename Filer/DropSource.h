#pragma once
#include "Unknown.h"

class CDropSource : public CUnknown<IDropSource>
{
public:
	CDropSource();
	~CDropSource();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject);
	
	STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHODIMP GiveFeedback(DWORD dwEffect);
};