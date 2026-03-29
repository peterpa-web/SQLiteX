#pragma once
class COldClass
{
public:
	class CFldData
	{
	public:
		CString m_strSql;
		CString m_strVar;
		int m_nSqlType = -1;
		int m_nType = -1;
		CString m_strFlags;
	};

	bool IsEmpty() { return m_strPathName.IsEmpty(); }
	void Reset() { m_strPathName.Empty(); }
	bool Read(const CString &strPath, const CString strFile);
	const CFldData* GetFldBySql(const CString& strSql);
//	int GetTypeBySql(const CString& strSql, int nSqlType);
//	CString GetFlagsBySql(const CString& strSql);
//	CString GetVarNameBySql(const CString& strSql);
	bool HasDerived() { return m_bDerived; }
	CString GetOldClassName() { return m_strClassName; }
	CString GetOldFileName() { return m_strFileName; }
	int GetFieldCount() { return m_listData.GetCount(); }
	void SetDerived() { m_bDerived = true; }
//	bool RenameToBak();

protected:
	CString m_strFileName;
	CString m_strPathName;
	CString m_strClassName;
	CList<CFldData> m_listData;
	bool m_bDerived = false;
};

