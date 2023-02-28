
// SampleDlgApp.h: Hauptheaderdatei für die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'pch.h' vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"		// Hauptsymbole


// CSampleDlgApp:
// Siehe SampleDlgApp.cpp für die Implementierung dieser Klasse
//

class CSampleDlgApp : public CWinApp
{
public:
	CSampleDlgApp();

// Überschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CSampleDlgApp theApp;
