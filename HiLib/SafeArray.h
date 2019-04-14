#pragma once
#include "StdAfx.h"
      
#include <stdlib.h>   
#include <crtdbg.h>
  
#include <tchar.h>   
#include <ole2.h>   
#include <atlbase.h> 
#include <atlcom.h>
#include <comutil.h>
#include <comdef.h> 
 

VARIANT Create2DSafeArray(long i,long j);
VARIANT Get2DSafeArrayElement(VARIANT* varArray,long i,long j);

  

