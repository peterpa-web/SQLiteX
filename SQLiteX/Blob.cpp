#include "pch.h"
#include "Blob.h"

CBlob::CBlob(const CBlob& b)
{
	if (m_dwSize != b.m_dwSize)
	{
		delete m_pData;
		m_dwSize = b.m_dwSize;
		m_pData = new BYTE[m_dwSize];
	}
	memcpy(m_pData, b.m_pData, m_dwSize);
}

CBlob::CBlob(DWORD dwSize, const BYTE* pData)
{
	m_dwSize = dwSize;
	m_pData = new BYTE[dwSize];
}

void CBlob::SetData(DWORD dwSize, const void* pb)
{
	if (m_dwSize != dwSize)
	{
		delete m_pData;
		m_dwSize = dwSize;
		m_pData = new BYTE[m_dwSize];
	}
	memcpy(m_pData, pb, m_dwSize);
}

bool CBlob::operator==(const CBlob& b) const
{
	if (m_dwSize != b.m_dwSize)
		return false;

	return (memcmp(m_pData, b.m_pData, m_dwSize) == 0);
}
