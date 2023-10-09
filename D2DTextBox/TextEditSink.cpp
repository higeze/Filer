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
#include "TSFManager.h"
#include "DisplayAttribute.h"

DWORD CTextEditSink::TES_INVALID_COOKIE = ((DWORD)(-1));

CTextEditSink::CTextEditSink(CTextBox* pTextBox)
    :m_pEditor(pTextBox), m_dwEditCookie(TES_INVALID_COOKIE){}

HRESULT CTextEditSink::_Advise(const CComPtr<ITfContext>& pContext)
{
    CComPtr<ITfSource> pSource;

    FAILED_THROW(pContext->QueryInterface(IID_ITfSource, (void**)&pSource));
    FAILED_THROW(pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &m_dwEditCookie));

    m_pContext = CComPtr<ITfContext>(pContext);

    return S_OK;
}

HRESULT CTextEditSink::_Unadvise()
{
    CComPtr<ITfSource> pSource;

    FAILED_THROW(m_pContext->QueryInterface(IID_ITfSource, (void**)&pSource));
    FAILED_THROW(pSource->UnadviseSink(m_dwEditCookie));
    m_pContext.Release();

    return S_OK;
}

STDAPI CTextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    CDispAttrProps *pDispAttrProps = CTSFManager::GetInstance()->GetDispAttrProps();
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
                m_pEditor->ClearCompositionRenderInfo();

                // We read the display attribute for entire range.
                // It could be optimized by filtering the only delta with ITfEditRecord interface. 
                CComPtr<ITfRange> pRangeEntire = nullptr;
                CComPtr<ITfRange> pRangeEnd = nullptr;
                if (SUCCEEDED(pic->GetStart(ecReadOnly, &pRangeEntire)) &&
                    SUCCEEDED(pic->GetEnd(ecReadOnly, &pRangeEnd)) &&
                    SUCCEEDED(pRangeEntire->ShiftEndToRange(ecReadOnly, pRangeEnd, TF_ANCHOR_END)))
                {
                    CComPtr<ITfReadOnlyProperty> pProp;

                    CTSFManager::GetInstance()->GetDisplayAttributeTrackPropertyRange(ecReadOnly, pic, pRangeEntire, &pProp, pDispAttrProps);

                    if (CComPtr<IEnumTfRanges> pEnumRanges;
                        SUCCEEDED(pProp->EnumRanges(ecReadOnly, &pEnumRanges, pRangeEntire))){
                        CComPtr<ITfRange> pRange;
                        while (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
                        {
                            TF_DISPLAYATTRIBUTE DisplayAttribute;
                            TfGuidAtom guid;
                            if (CTSFManager::GetInstance()->GetDisplayAttributeData(ecReadOnly, pProp, pRange, &DisplayAttribute, &guid) == S_OK)
                            {
                                if(auto pRangeACP = CComQIPtr<ITfRangeACP>(pRange)){
                                    LONG start, end;
                                    pRangeACP->GetExtent(&start, &end);
									m_pEditor->AddCompositionRenderInfo(start, start + end, &DisplayAttribute);
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

    return S_OK;
}

