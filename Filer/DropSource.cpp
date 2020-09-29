#include "DropSource.h"

CDropSource::CDropSource():CUnknown<IDropSource>()
{}

CDropSource::~CDropSource() = default;

STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed)
		return DRAGDROP_S_CANCEL;

	if ((grfKeyState & MK_LBUTTON) == 0)
		return DRAGDROP_S_DROP;

	return S_OK;
}

STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

