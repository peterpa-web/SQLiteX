
// SQLiteClassGen.h: Hauptheaderdatei für die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'pch.h' vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"		// Hauptsymbole


// CSQLiteClassGenApp:
// Siehe SQLiteClassGen.cpp für die Implementierung dieser Klasse
//

class CSQLiteClassGenApp : public CWinApp
{
public:
	CSQLiteClassGenApp();

// Überschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CSQLiteClassGenApp theApp;
