#pragma once
#include "D2DWDialog.h"
#include "FilerGridViewProperty.h"
#include "ShellFunction.h"
#include "DeadlineTimer.h"
#include "D2DWWindow.h"

class CProgressBar;
class CButton;

class CIDL;
class CCheckableFileGrid;

class CIncrementalCopyDlg :public CD2DWDialog
{
private:
	std::shared_ptr<CProgressBar> m_spProgressbar;
	std::shared_ptr<CCheckableFileGrid> m_spFileGrid;

	std::shared_ptr<CButton> m_spButtonDo;
	std::shared_ptr<CButton> m_spButtonCancel;
	std::shared_ptr<CButton> m_spButtonClose;


	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

	CIDL m_srcIDL;
	std::vector<CIDL> m_srcChildIDLs;
	CIDL m_destIDL;

	std::future<void> m_doFuture;
	std::future<void> m_compFuture;
	//CDeadlineTimer m_periodicTimer;

	std::unordered_map<CIDL, std::vector<CIDL>,
		shell::IdlHash, shell::IdlEqual> m_idlMap;

public:
	CIncrementalCopyDlg(
		CD2DWControl* pParentControl,
		const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	~CIncrementalCopyDlg();
	std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> GetRects();
	std::shared_ptr<CProgressBar>& GetProgressBarPtr() { return m_spProgressbar; }

	virtual void ArrangeOverride(const CRectF& e) override;

	virtual void OnCreate(const CreateEvt& e);
	virtual void OnIncrementMax();
	virtual void OnIncrementValue();
	virtual void OnAddItem(const CIDL& newIdl);
};