#include "pch.h"
#include "SQLiteTypes.h"

#include "OldClass.h"

bool COldClass::Read(const CString& strPath, const CString strFile)
{
	m_strFileName.Empty();
	m_strPathName.Empty();
	m_strClassName.Empty();
	CString strPathName = strPath + L"\\" + strFile + L".cpp";
	TRACE1("Read %s\n", strPathName);
	CFileStatus fs;
	if (!CFile::GetStatus(strPathName, fs) || (fs.m_attribute & CFile::directory) != 0)
		return false;
	m_strFileName = strFile;
	CStdioFile file(strPathName, CFile::modeRead);
	m_listData.RemoveAll();
	CString strLine;
	while (file.ReadString(strLine))
	{
		strLine = strLine.Trim();
		if (strLine.Left(4) == L"RFX_")
		{
			int p = strLine.Find('(');
			CString strRfx = strLine.Left(p).TrimRight();
			CFldData d;
			d.m_nType = CSQLiteTypes::GetTypeByRfx(strRfx);
			if (d.m_nType >= 0)
			{
				d.m_nSqlType = CSQLiteTypes::GetSqlTypeNo(d.m_nType);
				CString str = strLine.Tokenize(L",", p);
				d.m_strSql = strLine.Tokenize(L",", p);
				d.m_strSql = d.m_strSql.Trim();
				d.m_strSql = d.m_strSql.Mid(2, d.m_strSql.GetLength() - 3); // remove L"..."
				d.m_strVar = strLine.Tokenize(L",)", p);
				d.m_strVar = d.m_strVar.Trim();
				d.m_strFlags = strLine.Tokenize(L")", p);
				d.m_strFlags = d.m_strFlags.Trim();
				m_listData.AddTail(d);
			}
		}
		else
		{
			int p = strLine.Find(L"::DoFieldExchange(");
			if (p > 0)
			{
				m_strClassName = strLine.Mid(5, p - 5);
			}
		}
	}
	m_strPathName = strPathName;

	strPathName = strPath + L"\\" + strFile + L"2.cpp";
	m_bDerived = CFile::GetStatus(strPathName, fs) && (fs.m_attribute & CFile::directory) == 0;
	return true;
}

const COldClass::CFldData* COldClass::GetFldBySql(const CString& strSql)
{
	POSITION pos = m_listData.GetHeadPosition();
	while (pos != NULL)
	{
		const CFldData* pd = &m_listData.GetNext(pos);
		if (pd->m_strSql == strSql)
			return pd;
	}
	return nullptr;
}
/*
int COldClass::GetTypeBySql(const CString& strSql, int nSqlType)
{
	POSITION pos = m_listData.GetHeadPosition();
	while (pos != NULL)
	{
		CFldData& d = m_listData.GetNext(pos);
		if (d.m_strSql == strSql && d.m_nSqlType == nSqlType)
			return d.m_nType;
	}
	return -1;
}

CString COldClass::GetFlagsBySql(const CString& strSql)
{
	POSITION pos = m_listData.GetHeadPosition();
	while (pos != NULL)
	{
		CFldData& d = m_listData.GetNext(pos);
		if (d.m_strSql == strSql)
			return d.m_strFlags;
	}
	return CString();
}

CString COldClass::GetVarNameBySql(const CString& strSql)
{
	POSITION pos = m_listData.GetHeadPosition();
	while (pos != NULL)
	{
		CFldData& d = m_listData.GetNext(pos);
		if (d.m_strSql == strSql)
			return d.m_strVar;
	}
	return CString();
}
*/