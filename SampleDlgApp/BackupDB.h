#pragma once
#include "SQLiteRecordset.h"

class CBackupDB
{
public:
	CBackupDB(CSQLiteDatabase* pDatabase) : m_pDB(pDatabase) {}
	void CreateAll();
	void DropAll();
	void ImportAll(TxtFmt fmt = TxtFmt::standard, LPCTSTR pszExt = _T("txt"), char cSep = ';');
	void ExportAll(TxtFmt fmt, LPCTSTR pszExt = _T("txt"), char cSep = ';');

protected:
	CSQLiteDatabase* m_pDB;
};

