#include "pch.h"
#include "SQLiteTypes.h"
#include "SQLiteSchema.h"

//    CREATE TABLE sqlite_schema(
//        type text,
//        name text,
//        tbl_name text,
//        rootpage integer,
//        sql text
//    );

CSQLiteSchema::~CSQLiteSchema()
{
	ResetTables();
}

void CSQLiteSchema::ReadAll(const CString& strDbPath)
{
	ResetTables();
	CSQLiteDatabase db;
	bool b = db.Open(strDbPath);
	CSQLiteSchemaInt schemaInt(&db);
	schemaInt.m_strFilter = L"type=\"table\"";
	schemaInt.Open();
	while (!schemaInt.IsEOF())
	{
		CSQLiteTable* pT = new CSQLiteTable;
		pT->m_TblName = schemaInt.m_TblName;
		CString strName = pT->m_TblName;
		if (strName[0] == '[')
			strName = strName.Mid(1, strName.GetLength() - 2);
		pT->m_FileName = strName.Left(1).MakeUpper() + strName.Mid(1);
		pT->m_ClassName = 'C' + pT->m_FileName;
		CString strSql = schemaInt.m_Sql;
		int p = 0;
		CString strStart = strSql.Tokenize(L"(", p);
		if (p > 0)
		{
			CString strFields = strSql.Tokenize(L")", p);
			pT->ParseFields(strFields);
		}
		m_tables.AddTail(pT);
		schemaInt.MoveNext();
	}
	schemaInt.Close();

}

void CSQLiteSchema::ResetTables()
{
	POSITION pos = m_tables.GetTailPosition();
	while (pos != NULL)
	{
		POSITION p = pos;
		CSQLiteTable* pTable = m_tables.GetPrev(pos);
		delete pTable;
		m_tables.RemoveAt(p);
	}
}



CSQLiteSchemaInt::CSQLiteSchemaInt(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
}

CString CSQLiteSchemaInt::GetDefaultSQL()
{
	return _T("sqlite_schema");
}

void CSQLiteSchemaInt::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Text(pFX, _T("type"), m_Type);
	RFX_Text(pFX, _T("name"), m_Name);
	RFX_Text(pFX, _T("tbl_name"), m_TblName);
	RFX_Long(pFX, _T("rootpage"), m_RootPage);
	RFX_Text(pFX, _T("sql"), m_Sql, FX_NN);
}

void CSQLiteTable::ParseFields(const CString& strFields)
{
	int p = 0;
	CString strField = strFields.Tokenize(L",", p);
	while (!strField.IsEmpty())
	{
		strField = strField.Trim();
		if (strField.Left(11).CompareNoCase(L"PRIMARY KEY") == 0)
			return;
		if (strField.Left(11).CompareNoCase(L"FOREIGN KEY") == 0)
			return;

		CSQLiteField field;
		int q = 0;
		field.m_SqlName = strField.Tokenize(L" ", q);
		field.m_SqlTypeRaw = strField.Tokenize(L" ", q);
		field.m_nSqlType = CSQLiteTypes::GetSqlType(field.m_SqlTypeRaw);
		field.SetDefaultType();
		CString strName = field.m_SqlName;
		if (strName[0] == '[')
			strName = strName.Mid(1, strName.GetLength() - 2);
		field.m_strVarName = L"m_" + strName;
		if (q > 0)
		{
			CString strFlags = strField.Tokenize(L")", q);
			field.SetFlags(strFlags);
		}
		m_fields.AddTail(field);
		strField = strFields.Tokenize(L",", p);
	}
}

void CSQLiteTable::FillList(CListCtrl& list)
{
	list.DeleteAllItems();
	POSITION pos = m_fields.GetHeadPosition();
	int i = 0;
	while (pos != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(pos);
		int n = list.InsertItem(i, f.m_SqlName);
		list.SetItemText(i, 1, f.m_SqlTypeRaw);
		list.SetItemText(i, 2, f.GetDescr());
		list.SetItemData(i++, (DWORD_PTR) &f);
	}
}

void CSQLiteTable::GetIncludes(CStringList& list)
{
	POSITION posF = m_fields.GetHeadPosition();
	while (posF != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(posF);
		LPCSTR psz = CSQLiteTypes::GetInclude(f.m_nFktType);
		if (psz == nullptr)
			continue;

		CString strI(psz);
		POSITION posI = list.GetHeadPosition();
		while (posI != NULL)
		{
			CString s = list.GetNext(posI);
			if (s == strI)
				break;
		}
		if (posI == NULL)	// not found
			list.AddTail(strI);
	}
}

void CSQLiteTable::GetDefs(CStringList& list)
{
	POSITION posF = m_fields.GetHeadPosition();
	while (posF != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(posF);
		CString s = f.GetDef();
		list.AddTail(s);
	}
}

void CSQLiteTable::GetFunctions(CStringList& list)
{
	POSITION posF = m_fields.GetHeadPosition();
	while (posF != NULL)
	{
		CSQLiteField& f = m_fields.GetNext(posF);
		CString s = f.GetFunction();
		list.AddTail(s);
	}
}

void CSQLiteField::SetDefaultType()
{
	m_nFktType = CSQLiteTypes::GetDefaultType(m_nSqlType);
}

void CSQLiteField::SetFlags(const CString& strFlags)
{
	if (strFlags.IsEmpty())
		return;

	CString strF = strFlags;
	strF.MakeLower();

	if (strF.Find(L"not null") >= 0)
		m_dwFlags |= FX_NN;
	if (strF.Find(L"primary key") >= 0)
		m_dwFlags |= FX_PK;
	if (strF.Find(L"autoincrement") >= 0)
		m_dwFlags |= FX_AN;
	if (strF.Find(L"unique") >= 0)
		m_dwFlags |= FX_UN;
}

CString CSQLiteField::GetDef()
{
	return CSQLiteTypes::GetDeclLine(m_nFktType, m_strVarName);
}

CString CSQLiteField::GetFunction()
{
	return CSQLiteTypes::GetFktLine(m_nFktType, m_SqlName, m_strVarName, m_dwFlags);
}
