#pragma once
#include "Unknown.h"

class CTextBox;

class CTextEditSink : public CUnknown<ITfTextEditSink>
{
public:
	static DWORD TES_INVALID_COOKIE;
private:
	CComPtr<ITfContext> m_pContext;
	DWORD m_dwEditCookie;
	CTextBox *m_pEditor;
public:
	CTextEditSink(CTextBox* pTextBox);

	HRESULT _Advise(const CComPtr<ITfContext>& pContext);
	HRESULT _Unadvise();
	// ITfTextEditSink
	STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);


};


