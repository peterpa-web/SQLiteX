#include "pch.h"
#include "SQLiteTypes.h"

CSQLiteTypes::CTypeInt CSQLiteTypes::s_aTypes[s_nTypes] = {
	{ 1, "long",	"long",	"RFX_Long",	FX_NN | FX_PK | FX_AN | FX_UN },
//	{ 1, "__int64", "__int64",			"RFX_Int64",	FX_NN | FX_PK | FX_AN | FX_UN },
	{ 1, "bool",	"bool",				"RFX_Bool",	0 },
	{ 1, "CTime", "CTime >= 1970",		"RFX_Time", FX_NN | FX_UN },
	{ 1, "CDateLong", "CDateLong (YYYYMMDD)", "RFX_Date", FX_NN | FX_UN },
	{ 1, "CEuro",	"CEuro (Cents)",	"RFX_Euro",	FX_NN | FX_UN },
	{ 2, "double",	"double", 			"RFX_Double",	FX_NN | FX_UN },
	{ 2, "COleDateTime", "COleDataTime", "RFX_DateTime", FX_NN | FX_UN },
	{ 3, "CString",	"CString",			"RFX_Text",	FX_NN | FX_UN },
	{ 4, "CBlob",	"CBlob",			"RFX_Blob",	FX_NN }
};

WCHAR* CSQLiteTypes::s_aszSqlTypes[5] = { L"-", L"integer", L"real", L"text", L"blob" };

void CSQLiteTypes::FillCombo(CComboBox& combo, int nSqlType, int nFktType)
{
	combo.ResetContent();
	for (int t = 0; t < s_nTypes; t++)
	{
		if (s_aTypes[t].m_nSqlType != nSqlType)
			continue;
		CString str(s_aTypes[t].m_pszDescr);
		int n = combo.AddString(str);
		combo.SetItemData(n, t);
		if (t == nFktType)
			combo.SetCurSel(n);
	}
}

CString CSQLiteTypes::GetDeclLine(int nType, const CString& strVarName)
{
	CString str(s_aTypes[nType].m_pszCppType);
	str += ' ';
	str += strVarName;
	if (islower(str[0]))
		str += " = 0";
	str += ';';
	return str;
}

CString CSQLiteTypes::GetFktLine(int nType, const CString& strSqlName, const CString& strVarName, DWORD dwFlags)
{
//	RFX_Long(pFX, _T("[CompID]"), m_CompID, FX_PK);
	CString str(s_aTypes[nType].m_pszFunction);
	str += L"(pFX, _T(\"" + strSqlName + L"\"), " + strVarName;
	dwFlags &= GetFlags(nType);
	if (dwFlags != 0)
	{
		CString strFlags;
		if ((dwFlags & FX_NN) != 0)
			strFlags = L"FX_NN";
		if ((dwFlags & FX_PK) != 0)
			strFlags += strFlags.IsEmpty() ? L"FX_PK" : L" | FX_PK";
		if ((dwFlags & FX_AN) != 0)
			strFlags += strFlags.IsEmpty() ? L"FX_AN" : L" | FX_AN";
		if ((dwFlags & FX_UN) != 0)
			strFlags += strFlags.IsEmpty() ? L"FX_UN" : L" | FX_UN";
		str += L", " + strFlags;
	}
	str += L");";
	return str;
}

int CSQLiteTypes::GetDefaultType(int nSqlType)
{
	for (int i = 0; i < s_nTypes; i++)
	{
		if (s_aTypes[i].m_nSqlType == nSqlType)
			return i;
	}
	return -1;
}

int CSQLiteTypes::GetSqlType(const CString& strType)
{
	if (strType.IsEmpty())
		return 3;		// text

	CString s = strType;
	s = s.MakeLower();
	if (s.Compare(L"boolean") == 0)
		return 1;		// integer

	if (s.Compare(L"float") == 0)
		return 2;		// real

	if (s.Left(7).Compare(L"varchar") == 0)
		return 3;		// text

	for (int i = 1; i < 5; i++)
	{
		if (s.Compare(s_aszSqlTypes[i]) == 0)
			return i;
	}
	ASSERT(FALSE);
	return 0;
}

CString CSQLiteTypes::GetSqlType(int nType)
{
	return CString(s_aszSqlTypes[nType]);
}
