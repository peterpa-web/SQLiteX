#include "pch.h"
#include "CompanyRec.h"
#include "EmployeRec.h"
#include "EmployeFull.h"

#include "BackupDB.h"

void CBackupDB::CreateAll()
{
	{ CCompanyRec rec(m_pDB); rec.Create(); }
	{ CEmployeRec rec(m_pDB); rec.Create(); }
	{ CEmployeFull rec(m_pDB); rec.Create(); }
}

void CBackupDB::DropAll()
{
	{ CCompanyRec rec(m_pDB); rec.Drop(); }
	{ CEmployeRec rec(m_pDB); rec.Drop(); }
	{ CEmployeFull rec(m_pDB); rec.Drop(); }
}

void CBackupDB::ImportAll(TxtFmt fmt, LPCTSTR pszExt, char cSep)
{
	{ CCompanyRec rec(m_pDB); rec.Import(fmt, pszExt, cSep); }
	{ CEmployeRec rec(m_pDB); rec.Import(fmt, pszExt, cSep); }
}

void CBackupDB::ExportAll(TxtFmt fmt, LPCTSTR pszExt, char cSep)
{
	{ CCompanyRec rec(m_pDB); rec.Export(fmt, pszExt, cSep); }
	{ CEmployeRec rec(m_pDB); rec.Export(fmt, pszExt, cSep); }
}
