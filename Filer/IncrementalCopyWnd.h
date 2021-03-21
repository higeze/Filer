#pragma once
#include "MyWnd.h"
#include "Direct2DWrite.h"
#include "FilerGridViewProperty.h"
#include "ShellFunction.h"
#include "DeadlineTimer.h"
#include "D2DWWindow.h"

class CProgressBar;
class CButton;

class CIDL;
class CCheckableFileGrid;

class CIncrementalCopyWnd :public CD2DWWindow
{
private:
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

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
	CIncrementalCopyWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	~CIncrementalCopyWnd();
	std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> GetRects();
	std::shared_ptr<CProgressBar>& GetProgressBarPtr() { return m_spProgressbar; }

	void OnCreate(const CreateEvt& e);
	void OnClose(const CloseEvent& e);
	void OnRect(const RectEvent& e);
	void OnIncrementMax();
	void OnIncrementValue();
	void OnAddItem(const CIDL& newIdl);
};