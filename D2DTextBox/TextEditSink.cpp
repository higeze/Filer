#include "D2DWWindow.h"
#include "Textbox.h"
#include "TextEditSink.h"
#include "CellProperty.h"
#include "MyClipboard.h"
#include "IBridgeTSFInterface.h"
#include "TextStoreACP.h"
#include "CellProperty.h"
#include "Direct2DWrite.h"
#include "Debug.h"

/**/
//
// CTextEditSink
//
/**/
#include "DisplayAttribute.h"
#pragma region IUnknown
//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextEditSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTextEditSink::Release()
{
    long cr;

    cr = --_cRef;

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}
#pragma endregion
//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------
#define TES_INVALID_COOKIE  ((DWORD)(-1))

CTextEditSink::CTextEditSink(CTextBox *pEditor)
{
    _cRef = 1;
    _dwEditCookie = TES_INVALID_COOKIE;
    _pEditor = pEditor;
}

//+---------------------------------------------------------------------------
//
// EndEdit
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    CDispAttrProps *pDispAttrProps = _pEditor->GetDispAttrProps();
    if (pDispAttrProps)
    {
        
        if (CComPtr<IEnumTfRanges> pEnum;
            SUCCEEDED(pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT,
                                                             pDispAttrProps->GetPropTablePointer(),
                                                             pDispAttrProps->Count(),
                                                             &pEnum)) && pEnum)
        {
            // Test if there is range
            if (CComPtr<ITfRange> pTestRange;
                pEnum->Next(1, &pTestRange, NULL) == S_OK)
            {
                _pEditor->ClearCompositionRenderInfo();

                // We read the display attribute for entire range.
                // It could be optimized by filtering the only delta with ITfEditRecord interface. 
                CComPtr<ITfRange> pRangeEntire = nullptr;
                CComPtr<ITfRange> pRangeEnd = nullptr;
                if (SUCCEEDED(pic->GetStart(ecReadOnly, &pRangeEntire)) &&
                    SUCCEEDED(pic->GetEnd(ecReadOnly, &pRangeEnd)) &&
                    SUCCEEDED(pRangeEntire->ShiftEndToRange(ecReadOnly, pRangeEnd, TF_ANCHOR_END)))
                {
                    CComPtr<ITfReadOnlyProperty> pProp;

                    _pEditor->GetDisplayAttributeTrackPropertyRange(ecReadOnly, pic, pRangeEntire, &pProp, pDispAttrProps);

                    if (CComPtr<IEnumTfRanges> pEnumRanges;
                        SUCCEEDED(pProp->EnumRanges(ecReadOnly, &pEnumRanges, pRangeEntire))){
                        CComPtr<ITfRange> pRange;
                        while (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
                        {
                            TF_DISPLAYATTRIBUTE DisplayAttribute;
                            TfGuidAtom guid;
                            if (_pEditor->GetDisplayAttributeData(ecReadOnly, pProp, pRange, &DisplayAttribute, &guid) == S_OK)
                            {
                                if(auto pRangeACP = CComQIPtr<ITfRangeACP>(pRange)){
                                    LONG start, end;
                                    pRangeACP->GetExtent(&start, &end);
									_pEditor->AddCompositionRenderInfo(start, start + end, &DisplayAttribute);
                                }
                            }
                            pRange.Release();
                        }
                    }
                } 
            }
        }

        delete pDispAttrProps;
    }

	if ( OnChanged_ )
		OnChanged_();

	

    return S_OK;
}

#pragma region Advise_Unadvice
//+---------------------------------------------------------------------------
//
// CTextEditSink::Advise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Advise(ITfContext *pic)
{
    HRESULT hr;
    ITfSource *source = NULL;

    _pic = NULL;
    hr = E_FAIL;

    if (FAILED(pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwEditCookie)))
        goto Exit;

    _pic = pic;
    _pic->AddRef();

    hr = S_OK;

Exit:
    if (source)
        source->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// CTextEditSink::Unadvise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Unadvise()
{
    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_pic == NULL)
        goto Exit;

    if (FAILED(_pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->UnadviseSink(_dwEditCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    if (source)
        source->Release();

    if (_pic)
    {
        _pic->Release();
        _pic = NULL;
    }

    return hr;
}
#pragma endregion