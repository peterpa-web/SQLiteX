This Visual Studio solution contains a single MFC lib project SQLiteX providing 
a lightweight c++ wrapper for SQLite that is similar to the MFC database 
classes CDatabase & CRecordset without using any ODBC driver. 
Currently the most relevant features are implemented.

Please add manually the required original SQLite source files sqlite3.c and sqlite3.h
to the project folder SQLiteX by downloading the amalgamation source zip from 
https://sqlite.org/download.html

Additionally some helper classes are implementing Euro currency as long type and Date as
long type where its decimal representation directly shows a readable date.

The provided MFC project SampleDlgApp shows most features of the wrapper. 
Please feel free to extend it.

The licence for this solution is the same as for SQLite.

Please mention the author Peter Pagel, Glinde, Germany