#include "pch.h"
#include "Euro.h"

CEuro::CEuro(double e)
{
    double d = e * 100;
    d += (d >= 0) ? 0.5 : -0.5;
    m_nCents = (long)d;
}

CString CEuro::ToString()
{
    CString str;
    str.Format(_T("%d,%2.2d"), m_nCents / 100, (m_nCents < 0 ? -m_nCents : m_nCents) % 100);
    return str;
}

void CEuro::FromString(const CStringA& str)
{
    
    int p = str.FindOneOf(".,");
    if (p >= 0)
    {
        CStringA s(str);
        if (s[p] == ',')
            s.Replace(',', '.');
        double d = atof(s) * 100;
        d += (d >= 0) ? 0.5 : -0.5;
        m_nCents = (long)d;
    }
    else
        m_nCents = atol(str);
}

