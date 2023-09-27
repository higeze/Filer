#include "TSFManager.h"
#include "Debug.h"

const CComPtr<ITfThreadMgr2>& CTSFManager::GetThreadMgrPtr() const
{
	if (!m_pThreadMgr) {
		FAILED_THROW(::CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr2, (void**)&m_pThreadMgr));
	}
	return m_pThreadMgr;
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

const CComPtr<CTextEditSink>& CTSFManager::GetTextEditSinkPtr() const
{
	if (!m_pTextEditSink) {
		m_pTextEditSink.Attach(new CTextEditSink(nullptr));//TODOTODO
	}
	return m_pTextEditSink;
}
const CComPtr<CTextStore>& CTSFManager::GetTextStorePtr() const
{
	if (!m_pTextStore) {
		m_pTextStore.Attach(new CTextStore(nullptr));//TODO
	}
	return m_pTextStore;
}
const CComPtr<ITfDocumentMgr>& CTSFManager::GetDocumentMgrPtr() const
{
	if (!m_pDocumentMgr) {
		FAILED_THROW(GetThreadMgrPtr()->CreateDocumentMgr(&m_pDocumentMgr))
	}
	return m_pDocumentMgr;

}
const CComPtr<ITfContext>& CTSFManager::GetContextPtr() const
{
	if (!m_pContext) {
		FAILED_THROW(GetDocumentMgrPtr()->CreateContext(m_tfClientId, 0, static_cast<ITextStoreACP*>(GetTextStorePtr().p), &m_pContext, &m_editCookie));
	}
	return m_pContext;
}


CTSFManager::CTSFManager()
{
	FAILED_THROW(GetThreadMgrPtr()->Activate(&m_tfClientId));

	FAILED_THROW(GetDocumentMgrPtr()->Push(GetContextPtr()));
	FAILED_THROW(GetThreadMgrPtr()->SetFocus(GetDocumentMgrPtr()));
	FAILED_THROW(GetTextEditSinkPtr()->_Advise(GetContextPtr()));
}

CTSFManager::~CTSFManager()
{
	FAILED_THROW(GetTextEditSinkPtr()->_Unadvise());
	FAILED_THROW(GetDocumentMgrPtr()->Pop(TF_POPF_ALL));

	FAILED_THROW(GetThreadMgrPtr()->Deactivate());
}
