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
	CDirect2DWrite* m_pDirect;
	std::unordered_set<std::wstring> m_excludeExtSet;
	std::unique_ptr<CD2DAtlasFixedSizeBitmap> m_pAtlasBitmap;

	CIcon GetDefaultIcon() const;
	CIcon GetIcon(const CIDL& absoluteIDL) const;
	CComPtr<ID2D1Bitmap> GetBitmapFromIcon(const CIcon& icon) const;

public:
	CD2DFileIconDrawer(CDirect2DWrite* pDirect);
	~CD2DFileIconDrawer() = default;

	void DrawFileIconBitmap(const CRectF& dstRect, const CIDL& absoluteIDL, const std::wstring& path, const std::wstring& ext, const DWORD& data, std::function<void()> funupd);
	void Clear();
};