#include "MyIcon.h"

CIcon::CIcon(HICON hIcon)
{
	m_pIcon=std::shared_ptr<std::remove_pointer<HICON>::type>(hIcon,/*[](HICON hIcon){*/::DestroyIcon/*(hIcon);}*/);
}
