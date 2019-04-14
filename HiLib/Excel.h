#pragma once
#include "StdAfx.h"
  
//#define _CRTDBG_MAP_ALLOC   
#include <stdlib.h>   
#include <crtdbg.h>
  
#include <tchar.h>   
#include <ole2.h>   
#include <atlbase.h> 
#include <atlcom.h> 

#include "SafeArray.h" 


//#include <atlsafe.h>   


class CExcel
{
private:
	CComDispatchDriver m_pApp;
	CComDispatchDriver m_pBooks;
	CComDispatchDriver m_pBook;
   	CComDispatchDriver m_pSheets;
	CComDispatchDriver m_pSheet;

	BOOL GetA1ReferenceStyle(long lRow, long lCol, LPWSTR lpszRet);
	BOOL GetA1ReferenceStyle(long lRow1, long lCol1,long lRow2, long lCol2, LPWSTR lpszRet);

	BOOL GetRangePtr(LPWSTR szA1,CComDispatchDriver& pRange);
	long GetSheetCount();

public:

	CExcel();
	~CExcel();
	
	BOOL OpenBook(LPWSTR lpszFilePath);
	BOOL AddBook();
	BOOL SaveBook();
	BOOL SaveBookAs(LPWSTR lpszFileName);
	BOOL ActivateSheet(LPWSTR lpszSheetName);
	BOOL ActivateSheet(long lSheetNum);
	BOOL Close();
	BOOL Read(long lRow,long lCol,VARIANT* pVar);
	BOOL Read(long lRow1,long lCol1,long lRow2,long lCol2,VARIANT* pVar);
	BOOL Write(long lRow,long lCol,VARIANT* pVar);
	BOOL Write(long lRow1,long lCol1,long lRow2,long lCol2,VARIANT* pVar);


};

