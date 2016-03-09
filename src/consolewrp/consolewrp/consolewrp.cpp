//

#include "stdafx.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <Windows.h>
#include <stdio.h>
#include <sstream>
#include <io.h>



using namespace std;

wstring getPath() {
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	wstring ws(path);


	ws = ws.substr(0, ws.find_last_of('/'));
	ws = ws.substr(0, ws.find_last_of('\\'));

	return ws;
}


HANDLE  hJob = NULL;


int run(wstring cmdl, BOOL usestd) {
	/*
	* Create a job object.
	*/


	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

	if (!hJob) {
		hJob = CreateJobObject(NULL, NULL);

		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
	}





	STARTUPINFO si;
	PROCESS_INFORMATION pi;


	ZeroMemory(&si, sizeof(si));
	

	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcessW(NULL,   // No module name (use command line)
		&cmdl[0],        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		TRUE,          // Set handle inheritance to true
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		int err = GetLastError();
		//printf("CreateProcess failed (%d).\n", GetLastError());
		return err;
	}

	AssignProcessToJobObject(hJob, pi.hProcess); // Does not work if without CREATE_BREAKAWAY_FROM_JOB

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);


	DWORD exit_code = 0;


	GetExitCodeProcess(pi.hProcess, &exit_code);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);


	return exit_code;
}

DWORD _tmain(int argc, _TCHAR* argv[])
{
	wstring command = L"";
	wifstream infile(getPath() + L"/consolewrp.txt");

	if (infile.good())
	{
		getline(infile, command);
	}
	infile.close();

	if (command != L"") {
		command = L"\"" + getPath() + L"\\" + command + L"\"";
		for (int i = 1; i < argc; i++) {
			command += wstring(L" \"") + wstring(argv[i]) + wstring(L"\"");
		}
		if (run(L"console --silent --test \" \" ", false) == 0) {
			command = wstring(L"console.exe ") + command;
		}

		//command = "\"" + command + "\"";

		

		return run(command, true);
	}
	else
	{
		return 1;
	}


}
