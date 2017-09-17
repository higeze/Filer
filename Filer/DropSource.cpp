#include "DropSource.h"

CDropSource::CDropSource():CUnknown<IDropSource>()
{}

CDropSource::~CDropSource(){}

STDMETHODIMP CDropSource::QueryInterface(REFIID riid, void **ppvObject)
{
	*ppvObject = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropSource)){
		*ppvObject = static_cast<IDropSource *>(this);
	}else{
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}

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

