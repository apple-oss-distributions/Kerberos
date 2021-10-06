#ifdef __NMAKE__

# NMAKE portion.
# Build with : nmake /f custom.cpp
# Clean with : nmake /f custom.cpp clean

# Builds custom.dll

OUTPATH = .

# program name macros
CC = cl /nologo

LINK = link /nologo

RM = del

DLLFILE = $(OUTPATH)\custom.dll

DLLEXPORTS =\
    -EXPORT:EnableAllowTgtSessionKey \
    -EXPORT:RevertAllowTgtSessionKey \
    -EXPORT:AbortMsiImmediate \
    -EXPORT:UninstallNsisInstallation \
    -EXPORT:KillRunningProcesses \
    -EXPORT:ListRunningProcesses

$(DLLFILE): $(OUTPATH)\custom.obj
    $(LINK) /OUT:$@ /DLL $** $(DLLEXPORTS)

$(OUTPATH)\custom.obj: custom.cpp custom.h
    $(CC) /c /Fo$@ custom.cpp

all: $(DLLFILE)

clean:
    $(RM) $(DLLFILE)
    $(RM) $(OUTPATH)\custom.obj
    $(RM) $(OUTPATH)\custom.exp

!IFDEF __C_TEXT__
#else
/*

Copyright 2004 by the Massachusetts Institute of Technology

All rights reserved.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the Massachusetts
Institute of Technology (M.I.T.) not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

/**************************************************************
* custom.cpp : Dll implementing custom action to install Kerberos for Windows
*
*         The functions in this file are for use as entry points
*         for calls from MSI only. The specific MSI parameters
*         are noted in the comments section of each of the
*         functions.
*
* rcsid: $Id: custom.cpp 16675 2004-08-20 23:42:59Z jaltman $
**************************************************************/

#pragma unmanaged

// Only works for Win2k and above
#define _WIN32_WINNT 0x500
#include "custom.h"

// linker stuff
#pragma comment(lib, "msi")
#pragma comment(lib, "advapi32")


void ShowMsiError( MSIHANDLE hInstall, DWORD errcode, DWORD param ){
	MSIHANDLE hRecord;

	hRecord = MsiCreateRecord(3);
	MsiRecordClearData(hRecord);
	MsiRecordSetInteger(hRecord, 1, errcode);
	MsiRecordSetInteger(hRecord, 2, param);

	MsiProcessMessage( hInstall, INSTALLMESSAGE_ERROR, hRecord );
	
	MsiCloseHandle( hRecord );
}

#define LSA_KERBEROS_KEY "SYSTEM\\CurrentControlSet\\Control\\Lsa\\Kerberos"
#define LSA_KERBEROS_PARM_KEY "SYSTEM\\CurrentControlSet\\Control\\Lsa\\Kerberos\\Parameters"
#define KFW_CLIENT_KEY "SOFTWARE\\MIT\\Kerberos\\Client\\"
#define SESSKEY_VALUE_NAME "AllowTGTSessionKey"

#define SESSBACKUP_VALUE_NAME "AllowTGTSessionKeyBackup"
#define SESSXPBACKUP_VALUE_NAME "AllowTGTSessionKeyBackupXP"


/* Set the AllowTGTSessionKey registry keys on install.  Called as a deferred custom action. */
MSIDLLEXPORT EnableAllowTgtSessionKey( MSIHANDLE hInstall ) {
    return SetAllowTgtSessionKey( hInstall, TRUE );
}

/* Unset the AllowTGTSessionKey registry keys on uninstall.  Called as a deferred custom action. */
MSIDLLEXPORT RevertAllowTgtSessionKey( MSIHANDLE hInstall ) {
    return SetAllowTgtSessionKey( hInstall, FALSE );
}

UINT SetAllowTgtSessionKey( MSIHANDLE hInstall, BOOL pInstall ) {
    TCHAR tchVersionString[1024];
    TCHAR tchVersionKey[2048];
    DWORD size;
    DWORD type;
    DWORD value;
    HKEY hkKfwClient = NULL;
    HKEY hkLsaKerberos = NULL;
    HKEY hkLsaKerberosParm = NULL;
    UINT rv;
    DWORD phase = 0;

    // construct the backup key path
    size = sizeof(tchVersionString) / sizeof(TCHAR);
    rv = MsiGetProperty( hInstall, _T("CustomActionData"), tchVersionString, &size );
    if(rv != ERROR_SUCCESS) {
        if(pInstall) {
            ShowMsiError( hInstall, ERR_CUSTACTDATA, rv );
            return rv;
        } else {
            return ERROR_SUCCESS;
        }
    }

    _tcscpy( tchVersionKey, _T( KFW_CLIENT_KEY ) );
    _tcscat( tchVersionKey, tchVersionString );

    phase = 1;

    rv = RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchVersionKey, 0, ((pInstall)?KEY_WRITE:KEY_READ), &hkKfwClient );
    if(rv != ERROR_SUCCESS)
        goto cleanup;

    phase = 2;

    rv = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T( LSA_KERBEROS_KEY ), 0, KEY_READ | KEY_WRITE, &hkLsaKerberos );
    if(rv != ERROR_SUCCESS) 
        goto cleanup;

    phase = 3;

    rv = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T( LSA_KERBEROS_PARM_KEY ), 0, KEY_READ | KEY_WRITE, &hkLsaKerberosParm );
    if(rv != ERROR_SUCCESS) {
        hkLsaKerberosParm = NULL;
    }

    if(pInstall) {
        // backup the existing values
        if(hkLsaKerberosParm) {
            phase = 4;

            size = sizeof(value);
            rv = RegQueryValueEx( hkLsaKerberosParm, _T( SESSKEY_VALUE_NAME ), NULL, &type, (LPBYTE) &value, &size );
            if(rv != ERROR_SUCCESS)
                value = 0;

            phase = 5;
            rv = RegSetValueEx( hkKfwClient, _T( SESSBACKUP_VALUE_NAME ), 0, REG_DWORD, (LPBYTE) &value, sizeof(value));
            if(rv != ERROR_SUCCESS)
                goto cleanup;
        }

        phase = 6;
        size = sizeof(value);
        rv = RegQueryValueEx( hkLsaKerberos, _T( SESSKEY_VALUE_NAME ), NULL, &type, (LPBYTE) &value, &size );
        if(rv != ERROR_SUCCESS)
            value = 0;

        phase = 7;
        rv = RegSetValueEx( hkKfwClient, _T( SESSXPBACKUP_VALUE_NAME ), 0, REG_DWORD, (LPBYTE) &value, sizeof(value));
        if(rv != ERROR_SUCCESS)
            goto cleanup;

        // and now write the actual values
        phase = 8;
        value = 1;
        rv = RegSetValueEx( hkLsaKerberos, _T( SESSKEY_VALUE_NAME ), 0, REG_DWORD, (LPBYTE) &value, sizeof(value));
        if(rv != ERROR_SUCCESS)
            goto cleanup;

        if(hkLsaKerberosParm) {
            phase = 9;
            value = 1;
            rv = RegSetValueEx( hkLsaKerberosParm, _T( SESSKEY_VALUE_NAME ), 0, REG_DWORD, (LPBYTE) &value, sizeof(value));
            if(rv != ERROR_SUCCESS)
                goto cleanup;
        }

    } else { // uninstalling
        // Don't fail no matter what goes wrong.  This is also a rollback action.
        if(hkLsaKerberosParm) {
            size = sizeof(value);
            rv = RegQueryValueEx( hkKfwClient, _T( SESSBACKUP_VALUE_NAME ), NULL, &type, (LPBYTE) &value, &size );
            if(rv != ERROR_SUCCESS)
                value = 0;

            RegSetValueEx( hkLsaKerberosParm, _T( SESSKEY_VALUE_NAME ), 0, REG_DWORD, (LPBYTE) &value, sizeof(value));
        }

        size = sizeof(value);
        rv = RegQueryValueEx( hkKfwClient, _T( SESSXPBACKUP_VALUE_NAME ), NULL, &type, (LPBYTE) &value, &size );
        if(rv != ERROR_SUCCESS)
            value = 0;

        RegSetValueEx( hkLsaKerberos, _T( SESSKEY_VALUE_NAME ), 0, REG_DWORD, (LPBYTE) &value, sizeof(value));

        RegDeleteValue( hkKfwClient, _T( SESSXPBACKUP_VALUE_NAME ) );
        RegDeleteValue( hkKfwClient, _T( SESSBACKUP_VALUE_NAME ) );
    }

    // all done
    rv = ERROR_SUCCESS;

cleanup:
    if(rv != ERROR_SUCCESS && pInstall) {
        ShowMsiError(hInstall, 4005, phase);
    }
    if(hkKfwClient) RegCloseKey( hkKfwClient );
    if(hkLsaKerberos) RegCloseKey( hkLsaKerberos );
    if(hkLsaKerberosParm) RegCloseKey( hkLsaKerberosParm );

    return rv;
}

/* Abort the installation (called as an immediate custom action) */
MSIDLLEXPORT AbortMsiImmediate( MSIHANDLE hInstall ) {
    DWORD rv;
	DWORD dwSize = 0;
	LPTSTR sReason = NULL;
	LPTSTR sFormatted = NULL;
	MSIHANDLE hRecord = NULL;
	LPTSTR cAbortReason = _T("ABORTREASON");

	rv = MsiGetProperty( hInstall, cAbortReason, _T(""), &dwSize );
	if(rv != ERROR_MORE_DATA) goto _cleanup;

	sReason = new TCHAR[ ++dwSize ];

	rv = MsiGetProperty( hInstall, cAbortReason, sReason, &dwSize );

	if(rv != ERROR_SUCCESS) goto _cleanup;

    hRecord = MsiCreateRecord(3);
	MsiRecordClearData(hRecord);
	MsiRecordSetString(hRecord, 0, sReason);

	dwSize = 0;

	rv = MsiFormatRecord(hInstall, hRecord, "", &dwSize);
	if(rv != ERROR_MORE_DATA) goto _cleanup;

	sFormatted = new TCHAR[ ++dwSize ];

	rv = MsiFormatRecord(hInstall, hRecord, sFormatted, &dwSize);

	if(rv != ERROR_SUCCESS) goto _cleanup;

	MsiCloseHandle(hRecord);

	hRecord = MsiCreateRecord(3);
	MsiRecordClearData(hRecord);
	MsiRecordSetInteger(hRecord, 1, ERR_ABORT);
	MsiRecordSetString(hRecord,2, sFormatted);
	MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecord);

_cleanup:
	if(sFormatted) delete sFormatted;
	if(hRecord) MsiCloseHandle( hRecord );
	if(sReason) delete sReason;

	return ~ERROR_SUCCESS;
}

/* Kill specified processes that are running on the system */
/* Uses the custom table KillProcess.  Called as an immediate action. */

#define MAX_KILL_PROCESSES 255
#define FIELD_SIZE 256

struct _KillProc {
    TCHAR * image;
    TCHAR * desc;
    BOOL    found;
    DWORD   pid;
};

#define RV_BAIL if(rv != ERROR_SUCCESS) goto _cleanup

MSIDLLEXPORT KillRunningProcesses( MSIHANDLE hInstall ) {
    return KillRunningProcessesSlave( hInstall, TRUE );
}

/* When listing running processes, we populate the ListBox table with
   values associated with the property 'KillableProcesses'.  If we
   actually find any processes worth killing, then we also set the
   'FoundProcceses' property to '1'.  Otherwise we set it to ''.
*/

MSIDLLEXPORT ListRunningProcesses( MSIHANDLE hInstall ) {
    return KillRunningProcessesSlave( hInstall, FALSE );
}

UINT KillRunningProcessesSlave( MSIHANDLE hInstall, BOOL bKill )
{
    UINT rv = ERROR_SUCCESS;
    _KillProc * kpList;
    int nKpList = 0;
    int i;
    int rowNum = 1;
    DWORD size;
    BOOL found = FALSE;

    MSIHANDLE hDatabase = NULL;
    MSIHANDLE hView = NULL;
    MSIHANDLE hViewInsert = NULL;
    MSIHANDLE hRecord = NULL;
    MSIHANDLE hRecordInsert = NULL;

    HANDLE hSnapshot = NULL;

    PROCESSENTRY32 pe;

    kpList = new _KillProc[MAX_KILL_PROCESSES];
    memset(kpList, 0, sizeof(*kpList) * MAX_KILL_PROCESSES);

    hDatabase = MsiGetActiveDatabase( hInstall );
    if( hDatabase == NULL ) {
        rv = GetLastError();
        goto _cleanup;
    }

    // If we are only going to list out the processes, delete all the existing
    // entries first.

    if(!bKill) {

        rv = MsiDatabaseOpenView( hDatabase,
            _T( "DELETE FROM `ListBox` WHERE `ListBox`.`Property` = 'KillableProcesses'" ),
            &hView); RV_BAIL;

        rv = MsiViewExecute( hView, NULL ); RV_BAIL;

        MsiCloseHandle( hView );

        hView = NULL;
        
        rv = MsiDatabaseOpenView( hDatabase,
              _T( "SELECT * FROM `ListBox` WHERE `Property` = 'KillableProcesses'" ),
            &hViewInsert); RV_BAIL;

        MsiViewExecute(hViewInsert, NULL);

        hRecordInsert = MsiCreateRecord(4);

        if(hRecordInsert == NULL) {
            rv = GetLastError();
            goto _cleanup;
        }
    }

    // Open a view
    rv = MsiDatabaseOpenView( hDatabase, 
        _T( "SELECT `Image`,`Desc` FROM `KillProcess`" ),
        &hView); RV_BAIL;

    rv = MsiViewExecute( hView, NULL ); RV_BAIL;

    do {
        rv = MsiViewFetch( hView, &hRecord );
        if(rv != ERROR_SUCCESS) {
            if(hRecord) 
                MsiCloseHandle(hRecord);
            hRecord = NULL;
            break;
        }

        kpList[nKpList].image = new TCHAR[ FIELD_SIZE ]; kpList[nKpList].image[0] = _T('\0');
        kpList[nKpList].desc = new TCHAR[ FIELD_SIZE ];  kpList[nKpList].desc[0] = _T('\0');
        nKpList++;

        size = FIELD_SIZE;
        rv = MsiRecordGetString(hRecord, 1, kpList[nKpList-1].image, &size); RV_BAIL;

        size = FIELD_SIZE;
        rv = MsiRecordGetString(hRecord, 2, kpList[nKpList-1].desc, &size); RV_BAIL;

        MsiCloseHandle(hRecord);
    } while(nKpList < MAX_KILL_PROCESSES);

    hRecord = NULL;

    // now we have all the processes in the array.  Check if they are running.
    
    hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if(hSnapshot == INVALID_HANDLE_VALUE) {
        rv = GetLastError();
        goto _cleanup;
    }

    pe.dwSize = sizeof( PROCESSENTRY32 );

    if(!Process32First( hSnapshot, &pe )) {
        // technically we should at least find the MSI process, but we let this pass
        rv = ERROR_SUCCESS;
        goto _cleanup;
    }

    do {
        for(i=0; i<nKpList; i++) {
            if(!_tcsicmp( kpList[i].image, pe.szExeFile )) {
                // got one
                if(bKill) {
                    // try to kill the process
                    HANDLE hProcess = NULL;

                    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if(hProcess == NULL) {
                        rv = GetLastError();
                        goto _cleanup;
                    }

                    if(!TerminateProcess(hProcess, 0)) {
                        rv = GetLastError();
                        CloseHandle(hProcess);
                        goto _cleanup;
                    }

                    CloseHandle(hProcess);

                } else {
                    TCHAR buf[256];

                    // we are supposed to just list out the processes
                    rv = MsiRecordClearData( hRecordInsert ); RV_BAIL;
                    rv = MsiRecordSetString( hRecordInsert, 1, _T("KillableProcesses"));
                    rv = MsiRecordSetInteger( hRecordInsert, 2, rowNum++ ); RV_BAIL;
                    _itot( rowNum, buf, 10 );
                    rv = MsiRecordSetString( hRecordInsert, 3, buf ); RV_BAIL;
                    if(_tcslen(kpList[i].desc)) {
                        rv = MsiRecordSetString( hRecordInsert, 4, kpList[i].desc ); RV_BAIL;
                    } else {
                        rv = MsiRecordSetString( hRecordInsert, 4, kpList[i].image ); RV_BAIL;
                    }
                    MsiViewModify(hViewInsert, MSIMODIFY_INSERT_TEMPORARY, hRecordInsert); RV_BAIL;

                    found = TRUE;
                }
                break;
            }
        }
   } while( Process32Next( hSnapshot, &pe ) );

    if(!bKill) {
        // set the 'FoundProcceses' property
        if(found) {
            MsiSetProperty( hInstall, _T("FoundProcesses"), _T("1"));
        } else {
            MsiSetProperty( hInstall, _T("FoundProcesses"), _T(""));
        }
    }

    // Finally:
    rv = ERROR_SUCCESS;

_cleanup:

    if(hRecordInsert) MsiCloseHandle(hRecordInsert);
    if(hViewInsert) MsiCloseHandle(hView);

    if(hSnapshot && hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);

    while(nKpList) {
        nKpList--;
        delete kpList[nKpList].image;
        delete kpList[nKpList].desc;
    }
    delete kpList;

    if(hRecord) MsiCloseHandle(hRecord);
    if(hView) MsiCloseHandle(hView);

    if(hDatabase) MsiCloseHandle(hDatabase);

    if(rv != ERROR_SUCCESS) {
        ShowMsiError(hInstall, ERR_PROC_LIST, rv);
    }

    return rv;
}

/* Uninstall NSIS */
MSIDLLEXPORT UninstallNsisInstallation( MSIHANDLE hInstall )
{
	DWORD rv = ERROR_SUCCESS;
	// lookup the NSISUNINSTALL property value
	LPTSTR cNsisUninstall = _T("UPGRADENSIS");
	HANDLE hIo = NULL;
	DWORD dwSize = 0;
	LPTSTR strPathUninst = NULL;
	HANDLE hJob = NULL;
	STARTUPINFO sInfo;
	PROCESS_INFORMATION pInfo;

	pInfo.hProcess = NULL;
	pInfo.hThread = NULL;

	rv = MsiGetProperty( hInstall, cNsisUninstall, _T(""), &dwSize );
	if(rv != ERROR_MORE_DATA) goto _cleanup;

	strPathUninst = new TCHAR[ ++dwSize ];

	rv = MsiGetProperty( hInstall, cNsisUninstall, strPathUninst, &dwSize );
	if(rv != ERROR_SUCCESS) goto _cleanup;

	// Create a process for the uninstaller
	sInfo.cb = sizeof(sInfo);
	sInfo.lpReserved = NULL;
	sInfo.lpDesktop = _T("");
	sInfo.lpTitle = _T("Foo");
	sInfo.dwX = 0;
	sInfo.dwY = 0;
	sInfo.dwXSize = 0;
	sInfo.dwYSize = 0;
	sInfo.dwXCountChars = 0;
	sInfo.dwYCountChars = 0;
	sInfo.dwFillAttribute = 0;
	sInfo.dwFlags = 0;
	sInfo.wShowWindow = 0;
	sInfo.cbReserved2 = 0;
	sInfo.lpReserved2 = 0;
	sInfo.hStdInput = 0;
	sInfo.hStdOutput = 0;
	sInfo.hStdError = 0;

	if(!CreateProcess( 
		strPathUninst,
		_T("Uninstall /S"),
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		NULL,
		&sInfo,
		&pInfo)) {
			pInfo.hProcess = NULL;
			pInfo.hThread = NULL;
			rv = 40;
			goto _cleanup;
		};

	// Create a job object to contain the NSIS uninstall process tree

	JOBOBJECT_ASSOCIATE_COMPLETION_PORT acp;

	acp.CompletionKey = 0;

	hJob = CreateJobObject(NULL, _T("NSISUninstallObject"));
	if(!hJob) {
		rv = 41;
		goto _cleanup;
	}

	hIo = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
	if(!hIo) {
		rv = 42;
		goto _cleanup;
	}

	acp.CompletionPort = hIo;

	SetInformationJobObject( hJob, JobObjectAssociateCompletionPortInformation, &acp, sizeof(acp));

	AssignProcessToJobObject( hJob, pInfo.hProcess );

	ResumeThread( pInfo.hThread );

	DWORD a,b,c;
	for(;;) {
		if(!GetQueuedCompletionStatus(hIo, &a, (PULONG_PTR) &b, (LPOVERLAPPED *) &c, INFINITE)) {
			Sleep(1000);
			continue;
		}
		if(a == JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO) {
			break;
		}
	}

	rv = ERROR_SUCCESS;
    
_cleanup:
	if(hIo) CloseHandle(hIo);
	if(pInfo.hProcess)	CloseHandle( pInfo.hProcess );
	if(pInfo.hThread) 	CloseHandle( pInfo.hThread );
	if(hJob) CloseHandle(hJob);
	if(strPathUninst) delete strPathUninst;

	if(rv != ERROR_SUCCESS) {
        ShowMsiError( hInstall, ERR_NSS_FAILED, rv );
	}
	return rv;
}
#endif
#ifdef __NMAKE__
!ENDIF
#endif