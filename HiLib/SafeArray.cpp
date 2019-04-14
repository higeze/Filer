#include "SafeArray.h"

VARIANT Create2DSafeArray(long i,long j)
{
	_variant_t varArray;
	SAFEARRAYBOUND sab[2];
	sab[0].lLbound=0;
	sab[0].cElements=i;
	sab[1].lLbound=0;
	sab[1].cElements=j;
										  
	varArray.vt=VT_ARRAY | VT_VARIANT;
	varArray.parray=::SafeArrayCreate(VT_VARIANT,2,sab);

	return varArray;
}

VARIANT Get2DSafeArrayElement(VARIANT* pVarArray,long i,long j)
{
	_variant_t varRtn;	
	long lIndex[2];
	lIndex[0]=i;
	lIndex[1]=j;
	::SafeArrayGetElement(pVarArray->parray,lIndex,(void*)(&varRtn));
	return varRtn;
}


  

