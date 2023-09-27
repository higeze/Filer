#pragma once
#include <msctf.h>
#include <atlcomcli.h>
#include "getter_macro.h"
#include "TextEditSink.h"
#include "TextStoreACP.h"

class CTSFManager
{
private:
	TfClientId m_tfClientId = TF_CLIENTID_NULL;
	mutable TfEditCookie m_editCookie;
	DECLARE_LAZY_COMPTR_GETTER(ITfThreadMgr2, ThreadMgr)
	DECLARE_LAZY_COMPTR_GETTER(ITfKeystrokeMgr, KeystrokeMgr)
	DECLARE_LAZY_COMPTR_GETTER(ITfDisplayAttributeMgr, DisplayAttributeMgr)
	DECLARE_LAZY_COMPTR_GETTER(ITfCategoryMgr, CategoryMgr)

	DECLARE_LAZY_COMPTR_GETTER(CTextEditSink, TextEditSink)
	DECLARE_LAZY_COMPTR_GETTER(CTextStore, TextStore)
	DECLARE_LAZY_COMPTR_GETTER(ITfDocumentMgr, DocumentMgr)
	DECLARE_LAZY_COMPTR_GETTER(ITfContext, Context)


public:
	CTSFManager();
	~CTSFManager();

	static class CTSFManager* GetInstance()
	{
		static CTSFManager manager;
		return &manager;
	}


};
