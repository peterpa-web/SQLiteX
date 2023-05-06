#pragma once
#include "SQLiteRecordset.h"

class CSQLiteTypes
{
public:
	static const int s_nTypes = 9;
	static void FillCombo(CComboBox& combo, int nSqlType, int nFktType);
	static LPCSTR GetInclude(int nType) { return s_aTypes[nType].m_pszInclude; }
	static CString GetDeclLine(int nType, const CString& strVarName);
	static CString GetFktLine(int nType, const CString& strSqlName, const CString& strVarName, DWORD dwFlags);
	static int GetDefaultType(int nSqlType);
	static int GetSqlType(const CString& strType);
	static CString GetSqlType(int nType);
	static LPCSTR GetDescr(int nType) { return s_aTypes[nType].m_pszDescr; }
	static DWORD GetFlags(int nType) { return s_aTypes[nType].m_dwFlags; }

protected:
	typedef struct {
		int m_nSqlType;
		LPCSTR m_pszCppType;
		LPCSTR m_pszDescr;
		LPCSTR m_pszInclude;
		LPCSTR m_pszFunction;
		DWORD m_dwFlags;	// supported flags
	} CTypeInt;
	static CTypeInt s_aTypes[s_nTypes];
	static WCHAR* s_aszSqlTypes[5];
};

