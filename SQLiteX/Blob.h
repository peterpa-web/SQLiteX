#pragma once
class CBlob
{
public:
	CBlob() {}
	CBlob(const CBlob& b);
	CBlob(DWORD dwSize, const BYTE* pData);
	virtual ~CBlob() { Empty(); }
	void Empty() { delete m_pData; m_pData = nullptr; m_dwSize = 0; }
	bool IsEmpty() { return m_dwSize == 0; }
	void SetData(DWORD dwSize, const void* pb);
	DWORD GetSize() const { return m_dwSize; }
	BYTE* GetData() { return m_pData; }
	const BYTE* GetData() const { return m_pData; }
	bool operator==(const CBlob& b) const;

protected:
	DWORD m_dwSize = 0;
	BYTE* m_pData = nullptr;
};

