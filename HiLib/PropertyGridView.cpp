#include "PropertyGridView.h"
#include "GridViewProperty.h"
#include "ResourceIDFactory.h"

CPropertyGridView::CPropertyGridView(std::shared_ptr<CGridViewProperty> spGridViewProperty)
	:CGridView(spGridViewProperty)
{
	//RegisterArgs and CreateArgs
	RegisterClassExArgument()
		.lpszClassName(_T("CPropertyGridView"))
		.style(CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)(COLOR_3DFACE+1));
	CreateWindowExArgument()
		.lpszClassName(_T("CPropertyGridView"))
		.lpszWindowName(_T("PropertyGridView"))
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"PropertyGridView"));
}


