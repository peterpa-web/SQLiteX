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
	schemaInt.m_strFilter = L"type=\"table\" OR type=\"view\"";
	schemaInt.Open();
	while (!schemaInt.IsEOF())
	{
		CSQLiteTable* pT = new CSQLiteTable;
		pT->m_TblName = schemaInt.m_TblName;
		CString strName = pT->m_TblName;
		if (strName[0] == '[')
			strName = strName.Mid(1, strName.GetLength() - 2);
		pT->m_FileName = L"Rec" + strName.Left(1).MakeUpper() + strName.Mid(1);
		pT->m_ClassName = 'C' + pT->m_FileName;
		CString strSql = schemaInt.m_Sql;
		int p = 0;
		if (schemaInt.m_Type == L"table")
		{
			CString strStart = strSql.Tokenize(L"(", p);
			if (p > 0)
			{
				CString strFields = strSql.Mid(p, strSql.GetLength() - p - 1);
				pT->ParseFields(strFields);
			}
		}
		else if (schemaInt.m_Type == L"view")
		{
			pT->m_bView = true;
			pT->m_Sql = schemaInt.m_Sql;
			CSQLiteTableInfo ti(&db);
			ti.OpenTable(schemaInt.m_TblName);
			while (!ti.IsEOF())
			{
				pT->AddField(ti);
				ti.MoveNext();
			}
		}
		m_tables.AddTail(pT);
		schemaInt.MoveNext();

		CSQLiteSchemaInt schemaInt2(&db);
		schemaInt2.m_strFilter = L"type=\"index\" AND tbl_name=$tbl_name";
		schemaInt2.m_TblName = pT->m_TblName;
		schemaInt2.Open();
		while (!schemaInt2.IsEOF())
		{
			CSQLiteIndex si;
			si.m_Name = schemaInt2.m_Name;
			si.m_Sql = schemaInt2.m_Sql;
			pT->m_idx.AddTail(si);
			schemaInt2.MoveNext();
		}
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
	return L"sqlite_schema";
}

void CSQLiteSchemaInt::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Text(pFX, L"type", m_Type);
	RFX_Text(pFX, L"name", m_Name);
	RFX_Text(pFX, L"tbl_name", m_TblName);
	RFX_Long(pFX, L"rootpage", m_RootPage);
	RFX_Text(pFX, L"sql", m_Sql, FX_NN);
}

void CSQLiteTable::ParseFields(const CString& strFields)
{
	int p = 0;
	CString strField;;
	while (!(strField = strFields.Tokenize(L",(", p)).IsEmpty())
	{
		strField = strField.Trim();
		if (strField.Left(11).CompareNoCase(L"PRIMARY KEY") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			if (strField2.Find(',') < 0)
			{
				CSQLiteField* pF = FindField(strField2);
				if (pF != nullptr)
				{
					pF->m_dwFlags |= FX_PK;
					continue;
				}
			}
			AddContraints(strField + '(' + strField2 + ')');
			continue;
		}
		if (strField.Left(6).CompareNoCase(L"UNIQUE") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			CSQLiteField* pF = FindField(strField2);
			if (pF != nullptr)
			{
				pF->m_dwFlags |= FX_UN;
				continue;
			}
			AddContraints(strField + '(' + strField2 + ')');
			continue;
		}
		if (strField.Left(11).CompareNoCase(L"FOREIGN KEY") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			CString strField3 = strFields.Tokenize(L")", p);
			AddContraints(strField + '(' + strField2 + ')' + strField3 + ')');
			continue;
		}
		if (strField.Left(5).CompareNoCase(L"CHECK") == 0)
		{
			CString strField2 = strFields.Tokenize(L")", p);
			AddContraints(strField + '(' + strField2 + ')');
			continue;
		}

		CSQLiteField field;
		int q = 0;
		if (strField[0] == '"')
		{
			q = strField.Find('"', 1);
			field.m_SqlName = strField.Mid(1, q - 1);
			while (strField[++q] == ' ' || strField[q] == '\t');
		}
		else
			field.m_SqlName = strField.Tokenize(L" \t", q);
		field.m_SqlTypeRaw = strField.Tokenize(L" ", q);
		field.m_nSqlType = CSQLiteTypes::GetSqlType(field.m_SqlTypeRaw);
		field.SetDefaultType();
		field.m_SqlName = StripDeco(field.m_SqlName);
		CString strName = field.m_SqlName;
		strName.Replace(' ', '_');
		field.m_strVarName = L"m_" + strName;
		if (q > 0)
		{
			CString strFlags = strField.Tokenize(L")", q);
			field.SetFlags(strFlags);
		}
		m_fields.AddTail(field);
	}
}

void CSQLiteTable::AddField(const CSQLiteTableInfo& ti)
{
	CSQLiteField field;
	field.m_SqlName = ti.m_Name;
	field.m_SqlTypeRaw = ti.m_Type;
	field.m_nSqlType = CSQLiteTypes::GetSqlType(field.m_SqlTypeRaw);
	field.SetDefaultType();
	field.m_SqlName = StripDeco(field.m_SqlName);
	CString strName = field.m_SqlName;
	strName.Replace(' ', '_');
	field.m_strVarName = L"m_" + strName;
	if (ti.m_NotNull)
		field.m_dwFlags |= FX_NN;
	if (ti.m_Pk)
		field.m_dwFlags |= FX_PK;
	m_fields.AddTail(field);
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

CString CSQLiteTable::StripDeco(CString strName)
{
	if (strName[0] == '[' || strName[0] == '\"')
		strName = strName.Mid(1, strName.GetLength() - 2);
	return strName;
}

CSQLiteField* CSQLiteTable::FindField(const CString& strName)
{
	CString strField = StripDeco(strName);
	POSITION pos = m_fields.GetHeadPosition();
	while (pos != NULL)
	{
		CSQLiteField* pF = &m_fields.GetNext(pos);
		if (strName.CompareNoCase(pF->m_SqlName) == 0)
			return pF;
	}
	return nullptr;
}

void CSQLiteTable::AddContraints(CString str)
{
	if (!m_strConstraintsQuoted.IsEmpty())
		m_strConstraintsQuoted += L",\")\n\t\t_T(\"";
	str.Replace(L"\"", L"\\\"");
	m_strConstraintsQuoted += str;
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
	ASSERT(m_strVarName.GetLength() > 2);
	return CSQLiteTypes::GetDeclLine(m_nFktType, m_strVarName);
}

CString CSQLiteField::GetFunction()
{
	return CSQLiteTypes::GetFktLine(m_nFktType, m_SqlName, m_strVarName, m_dwFlags);
}

CSQLiteTableInfo::CSQLiteTableInfo(CSQLiteDatabase* pdb)
	: CSQLiteRecordset(pdb)
{
}

CString CSQLiteTableInfo::GetDefaultSQL()
{
	return L"(table_info)";
}

void CSQLiteTableInfo::DoFieldExchange(CFieldExchange* pFX)
{
	RFX_Long(pFX, L"cid", m_Cid);
	RFX_Text(pFX, L"name", m_Name);
	RFX_Text(pFX, L"type", m_Type);
	RFX_Bool(pFX, L"notnull", m_NotNull);
	RFX_Text(pFX, L"dflt_value", m_Dflt_Value);
	RFX_Bool(pFX, L"pk", m_Pk);
}
