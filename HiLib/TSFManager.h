#pragma once
#include <msctf.h>
#include <atlcomcli.h>
#include "getter_macro.h"
#include "TextEditSink.h"
#include "KeyTraceEventSink.h"
#include "TextStoreACP.h"
#include "DisplayAttribute.h"


class CTSFManager
{
public:
public:
	static UINT WM_TSF_STARTCOMPOSITION;
private:
	TfClientId m_tfClientId = TF_CLIENTID_NULL;
	DECLARE_LAZY_COMPTR_GETTER(ITfThreadMgr2, ThreadMgr)
	DECLARE_LAZY_COMPTR_GETTER(ITfMessagePump, MessagePump)
	DECLARE_LAZY_COMPTR_GETTER(ITfKeystrokeMgr, KeystrokeMgr)
	DECLARE_LAZY_COMPTR_GETTER(CKeyTraceEventSink, KeyTraceEventSink)

	DECLARE_LAZY_COMPTR_GETTER(ITfDisplayAttributeMgr, DisplayAttributeMgr)
	DECLARE_LAZY_COMPTR_GETTER(ITfCategoryMgr, CategoryMgr)

public:
	CTSFManager();
	~CTSFManager();

	static class CTSFManager* GetInstance()
	{
		static CTSFManager manager;
		return &manager;
	}

	void Init();
	void Term();
	TfClientId GetID()const { return m_tfClientId; }
	CDispAttrProps* GetDispAttrProps();
	HRESULT GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext* pic, ITfRange* pRange, ITfReadOnlyProperty** ppProp, CDispAttrProps* pDispAttrProps);
	HRESULT GetDisplayAttributeData(TfEditCookie ec, ITfReadOnlyProperty* pProp, ITfRange* pRange, TF_DISPLAYATTRIBUTE* pda, TfGuidAtom* pguid);



};
