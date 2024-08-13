#pragma once
#include "D2DAtlasBitmap.h"
#include "MyIcon.h"

enum class FileIconType
{
	Default,
	Common,
	Unique
};

class CD2DFileIconDrawer
{
private:
	bool m_isWin10;
	std::unordered_set<std::wstring> m_excludeExtSet;
	std::unique_ptr<CD2DAtlasBitmap<std::wstring>> m_pAtlasBitmap;
	future_group<void> m_futureGroup;

	CIcon GetDefaultIcon() const;
	CIcon GetIcon(const CIDL& absoluteIDL) const;
	CComPtr<IWICBitmapSource> GetBitmapFromIcon(const CIcon& icon) const;

public:
	CD2DFileIconDrawer();
	~CD2DFileIconDrawer() = default;
	bool DrawDefaultIconBitmap(
		const CDirect2DWrite* pDirect,
		const CPointF& dstPoint);

	bool DrawFileIconBitmap(
		const CDirect2DWrite* pDirect,
		const CPointF& dstPoint,
		const CShellFile* pFile,
		std::function<void()>&& callback);
	void Clear();
private:
	bool DrawFileIconBitmap(
		const CDirect2DWrite* pDirect,
		const CPointF& dstPoint,
		const CIDL& absoluteIDL,
		const std::wstring& path,
		const std::wstring& ext,
		const DWORD& data,
		std::function<void()>&& callback);
};