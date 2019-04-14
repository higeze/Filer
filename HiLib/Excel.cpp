#include "Excel.h"
\
CExcel::CExcel()
{
	::CoInitialize(NULL);
	//Get CLSID for our server...
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);
	//Start server and get IDispatch...
	IDispatch *pXl;
	hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&pXl);

	//Application 
	m_pApp=pXl;
 
	//Work Books
	CComVariant varRet;
	m_pApp.GetPropertyByName(L"Workbooks", &varRet);
	m_pBooks = varRet.pdispVal;
}

CExcel::~CExcel()
{
	m_pApp.Invoke0(L"Quit");
	m_pApp.Release();
	m_pBooks.Release();
	m_pBook.Release();
   	m_pSheets.Release();
	m_pSheet.Release();
	::CoUninitialize();

}

	
BOOL CExcel::OpenBook(LPWSTR lpszFilePath)
{
//	USES_CONVERSION;
	//Work Book
	CComVariant varRet;
	CComVariant varParam(lpszFilePath);
	m_pBooks.Invoke1(L"Open",&varParam,&varRet);
	m_pBook = varRet.pdispVal;
	//Work Sheets
	m_pApp.GetPropertyByName(L"Worksheets",&varRet);
	m_pSheets=varRet.pdispVal;
	//Work Sheet
	m_pBook.GetPropertyByName(L"ActiveSheet",&varRet);
	m_pSheet=varRet.pdispVal;

	return TRUE;
}

BOOL CExcel::AddBook()
{
	//Work Book
	CComVariant varRet;
	m_pBooks.Invoke0(L"Add",&varRet);
	m_pBook = varRet.pdispVal;

	//Work Sheets
	m_pApp.GetPropertyByName(L"Worksheets",&varRet);
	m_pSheets=varRet.pdispVal;

	//Work Sheet
	m_pBook.GetPropertyByName(L"ActiveSheet",&varRet);
	m_pSheet=varRet.pdispVal;

	return TRUE;
}

BOOL CExcel::SaveBook()
{
	m_pBook.Invoke0(L"Save");
	return TRUE;
}

BOOL CExcel::SaveBookAs(LPWSTR lpszFileName)
{
	CComVariant varParam(lpszFileName);
	m_pBook.Invoke1(L"SaveAs",&varParam);
	return TRUE;
}
BOOL CExcel::ActivateSheet(LPWSTR lpszSheetName)
{
	long lSheetCount=GetSheetCount();
	for(long l=1;l<=lSheetCount;l++){
		CComVariant varParam(l);
		CComVariant varRet;

		DISPID dispid = NULL;   
		DISPPARAMS  dp = { &varParam, NULL, 1, 0 };   
		m_pSheets.GetIDOfName(L"Item", &dispid);   
		m_pSheets->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,   
					DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dp, &varRet, NULL, NULL);
		CComDispatchDriver pSheet=varRet.pdispVal;
		pSheet.GetPropertyByName(L"Name",&varRet);

		if(wcscmp(lpszSheetName,(LPWSTR)varRet.pbstrVal)==0){
			pSheet.Invoke1(L"Select",&varParam);
			m_pBook.GetPropertyByName(L"ActiveSheet",&varRet);
			m_pSheet=varRet.pdispVal;

			return TRUE;
		}   
	}
	
	return FALSE;
}

long CExcel::GetSheetCount()
{
	CComVariant varRet;
	m_pSheets.GetPropertyByName(L"Count",&varRet);

	return varRet.lVal;
}

BOOL CExcel::ActivateSheet(long lSheetNum)
{
	if(lSheetNum>=1 || lSheetNum<=GetSheetCount()){
		CComVariant varParam(lSheetNum);
		CComVariant varRet;
 
		DISPID dispid = NULL;   
		DISPPARAMS  dp = { &varParam, NULL, 1, 0 };   
		m_pSheets.GetIDOfName(L"Item", &dispid);   
		m_pSheets->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,   
					DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dp, &varRet, NULL, NULL);
		CComDispatchDriver pSheet=varRet.pdispVal;
 
		pSheet.Invoke1(L"Select",&varParam);
		m_pBook.GetPropertyByName(L"ActiveSheet",&varRet);
		m_pSheet=varRet.pdispVal;
   
		return TRUE;

	}else{
		return FALSE;
	}
}

BOOL CExcel::Close()
{
	m_pBook.Invoke0(L"Close");
	return TRUE;
}

BOOL CExcel::Read(long lRow,long lCol,VARIANT* pVar)
{
	CComDispatchDriver pRange;
	WCHAR szA1[20];
	GetA1ReferenceStyle(lRow,lCol,szA1);
	GetRangePtr(szA1,pRange);

	CComVariant varRet;
	pRange.GetPropertyByName(L"Value2",pVar);

	return TRUE;

}
BOOL CExcel::Read(long lRow1,long lCol1,long lRow2,long lCol2,VARIANT* pVar)
{
	CComDispatchDriver pRange;
	WCHAR szA1[20];
	GetA1ReferenceStyle(lRow1,lCol1,lRow2,lCol2,szA1);
	GetRangePtr(szA1,pRange);

	pRange.GetPropertyByName(L"Value2",pVar);

	return TRUE;
}

BOOL CExcel::Write(long lRow,long lCol,VARIANT* pVar)
{													   
	CComDispatchDriver pRange;
	WCHAR szA1[20];
	GetA1ReferenceStyle(lRow,lCol,szA1);
	GetRangePtr(szA1,pRange);

	pRange.PutPropertyByName(L"Value2",pVar);

	return TRUE;
}

BOOL CExcel::Write(long lRow1,long lCol1,long lRow2,long lCol2,VARIANT* pVar)
{
	CComVariant aa;
	CComDispatchDriver pRange;
	WCHAR szA1[20];
	GetA1ReferenceStyle(lRow1,lCol1,lRow2,lCol2,szA1);
	GetRangePtr(szA1,pRange);

/*	SAFEARRAYBOUND sab[2];
	sab[0].lLbound=0;
	sab[0].cElements=(lRow2-lRow1+1);
	sab[1].lLbound=0;
	sab[1].cElements=(lCol2-lCol1+1);

	CComVariant varArray;
	varArray.vt=VT_ARRAY | VT_VARIANT;
	varArray.parray=::SafeArrayCreate(VT_VARIANT,2,sab);
	long index[2];
	int k=0;
	for(int i=0;i<(lRow2-lRow1+1);i++){
		index[0]=i;
		for(int j=0;j<(lCol2-lCol1+14);j++){
			index[1]=j;
			CComVariant varTmp(i*j);
			::SafeArrayPutElement(varArray.parray,index,&varTmp);
		}
	}
*/
	pRange.PutPropertyByName(L"Value2",pVar);

	return TRUE;
}

BOOL CExcel::GetA1ReferenceStyle(long lRow1, long lCol1,long lRow2, long lCol2, LPWSTR lpszRet)
{
	WCHAR szA1[20];
	WCHAR szA2[10];
	GetA1ReferenceStyle(lRow1,lCol1,szA1);
	GetA1ReferenceStyle(lRow2,lCol2,szA2);
	wcscpy(lpszRet,szA1);
	wcscat(lpszRet,L":");
	wcscat(lpszRet,szA2);
	return TRUE;
}

BOOL CExcel::GetA1ReferenceStyle(long lRow, long lCol, LPWSTR lpszRet)
{
	if(lRow>=1 && lRow<=65536 && lCol>=1 && lCol<=256){
		WCHAR szCol[3];
			if(int(lCol/26)==0){
				szCol[0]='A'+lCol-1;
				szCol[1]=NULL;
			}else{
				szCol[0]='A'+int(lCol/26)-1;
				szCol[1]='A'+int(lCol%26)-1;
				szCol[2]=NULL;
			}

		WCHAR szRow[6];
		_ltow(lRow,szRow,10);
		wcscpy(lpszRet,szCol);
		wcscat(lpszRet,szRow);
		return TRUE;
	}else{
		return FALSE;
	}
	
}

BOOL CExcel::GetRangePtr(LPWSTR lpszA1, CComDispatchDriver& pRange)
{

	CComVariant varParam(lpszA1);
	CComVariant varRet;

	DISPID dispid = NULL;   
	DISPPARAMS  dp = { &varParam, NULL, 1, 0 };   
	m_pSheet.GetIDOfName(L"Range", &dispid);   
	m_pSheet->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,   
                DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dp, &varRet, NULL, NULL);   

	pRange=varRet.pdispVal;


	return TRUE;
}
