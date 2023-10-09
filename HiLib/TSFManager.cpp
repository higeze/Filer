#include "TSFManager.h"
#include "Debug.h"


UINT CTSFManager::WM_TSF_STARTCOMPOSITION = ::RegisterWindowMessage(L"WM_TSF_STARTCOMPOSITION");

const CComPtr<ITfThreadMgr2>& CTSFManager::GetThreadMgrPtr() const
{
	if (!m_pThreadMgr) {
		FAILED_THROW(::CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr2, (void**)&m_pThreadMgr));
	}
	return m_pThreadMgr;
}

const CComPtr<ITfMessagePump>& CTSFManager::GetMessagePumpPtr() const
{
    if (!m_pMessagePump) {
        FAILED_THROW(GetThreadMgrPtr()->QueryInterface(IID_ITfMessagePump, (void**)&m_pMessagePump));
    }
    return m_pMessagePump;
}

const CComPtr<ITfKeystrokeMgr>& CTSFManager::GetKeystrokeMgrPtr() const
{
	if (!m_pKeystrokeMgr) {
		FAILED_THROW(GetThreadMgrPtr()->QueryInterface(IID_ITfKeystrokeMgr, (void**)&m_pKeystrokeMgr));
	}
	return m_pKeystrokeMgr;
}

const CComPtr<ITfDisplayAttributeMgr>& CTSFManager::GetDisplayAttributeMgrPtr() const
{
	if (!m_pDisplayAttributeMgr) {
		FAILED_THROW(::CoCreateInstance(
			CLSID_TF_DisplayAttributeMgr,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITfDisplayAttributeMgr,
			(void**)&m_pDisplayAttributeMgr));
	}
	return m_pDisplayAttributeMgr;
}

const CComPtr<ITfCategoryMgr>& CTSFManager::GetCategoryMgrPtr() const
{
	if (!m_pCategoryMgr) {
		FAILED_THROW(::CoCreateInstance(
			CLSID_TF_CategoryMgr,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITfCategoryMgr,
			(void**)&m_pCategoryMgr));
	}
	return m_pCategoryMgr;
}

const CComPtr<CKeyTraceEventSink>& CTSFManager::GetKeyTraceEventSinkPtr() const
{
	if (!m_pKeyTraceEventSink) {
		m_pKeyTraceEventSink.Attach(new CKeyTraceEventSink());
	}
	return m_pKeyTraceEventSink;
}

CTSFManager::CTSFManager() {}

CTSFManager::~CTSFManager() {}

void CTSFManager::Init()
{
	FAILED_THROW(GetThreadMgrPtr()->Activate(&m_tfClientId));
	FAILED_THROW(GetKeyTraceEventSinkPtr()->_Advise());
}

void CTSFManager::Term()
{
	FAILED_THROW(GetKeyTraceEventSinkPtr()->_Unadvise());
	FAILED_THROW(GetThreadMgrPtr()->Deactivate());
}

CDispAttrProps* CTSFManager::GetDispAttrProps()
{
    CComPtr<IEnumGUID> pEnumProp;
    CDispAttrProps *pProps = NULL;
	FAILED_THROW(GetCategoryMgrPtr()->EnumItemsInCategory(GUID_TFCAT_DISPLAYATTRIBUTEPROPERTY, &pEnumProp));

    // Make a database for Display Attribute Properties.
    GUID guidProp;
    pProps = new CDispAttrProps;

    // Add System Display Attribute first.
    // So no other Display Attribute property overwrite it.
    pProps->Add(GUID_PROP_ATTRIBUTE);
	// SUCCEEDED:>=0
	// FAILED:<0
	// S_OK:0
	// S_FALSE:1
    while(pEnumProp->Next(1, &guidProp, NULL) == S_OK)
    {
		if (!::IsEqualGUID(guidProp, GUID_PROP_ATTRIBUTE)) {
			pProps->Add(guidProp);
		}
    }

	return pProps;
}

HRESULT CTSFManager::GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfReadOnlyProperty **ppProp, CDispAttrProps *pDispAttrProps)
{
    ITfReadOnlyProperty *pProp = NULL;
    GUID  *pguidProp = NULL;
    const GUID **ppguidProp;
    ULONG ulNumProp = 0;
    ULONG i;
 
    pguidProp = pDispAttrProps->GetPropTable();

    ulNumProp = pDispAttrProps->Count();

    // TrackProperties wants an array of GUID *'s
    //if ((ppguidProp = (const GUID **)LocalAlloc(LMEM_ZEROINIT, sizeof(GUID *)*ulNumProp)) == NULL)
	if ((ppguidProp = (const GUID **)new byte[sizeof(GUID* )*ulNumProp]) == NULL)
        return E_OUTOFMEMORY;

    for (i=0; i<ulNumProp; i++)
    {
        ppguidProp[i] = pguidProp++;
    }
    
    if (SUCCEEDED(pic->TrackProperties(ppguidProp, ulNumProp, 0, NULL, &pProp))){
        *ppProp = pProp;
    }

	delete [] (byte*)ppguidProp;
    //LocalFree(ppguidProp);
	return S_OK;
}

HRESULT CTSFManager::GetDisplayAttributeData(TfEditCookie ec, ITfReadOnlyProperty *pProp, ITfRange *pRange, TF_DISPLAYATTRIBUTE *pda, TfGuidAtom *pguid)
{
    VARIANT var;
    IEnumTfPropertyValue *pEnumPropertyVal;
    TF_PROPERTYVAL tfPropVal;
    GUID guid;
    TfGuidAtom gaVal;
    ITfDisplayAttributeInfo *pDAI;

    HRESULT hr = E_FAIL;

    hr = S_FALSE;
    if (SUCCEEDED(pProp->GetValue(ec, pRange, &var)))
    {
        if (SUCCEEDED(var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, 
                                                  (void **)&pEnumPropertyVal)))
        {
            while (pEnumPropertyVal->Next(1, &tfPropVal, NULL) == S_OK)
            {
                if (tfPropVal.varValue.vt == VT_EMPTY)
                    continue; // prop has no value over this span

                gaVal = (TfGuidAtom)tfPropVal.varValue.lVal;

                GetCategoryMgrPtr()->GetGUID(gaVal, &guid);

                if (SUCCEEDED(GetDisplayAttributeMgrPtr()->GetDisplayAttributeInfo(guid, &pDAI, NULL)))
                {
                    //
                    // Issue: for simple apps.
                    // 
                    // Small apps can not show multi underline. So
                    // this helper function returns only one 
                    // DISPLAYATTRIBUTE structure.
                    //
                    if (pda)
                    {
                        pDAI->GetAttributeInfo(pda);
                    }

                    if (pguid)
                    {
                        *pguid = gaVal;
                    }

                    pDAI->Release();
                    hr = S_OK;
                    break;
                }
            }
            pEnumPropertyVal->Release();
        }
        VariantClear(&var);
    }

    return hr;
}



