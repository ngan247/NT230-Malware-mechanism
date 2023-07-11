// Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <strsafe.h>
#include <tchar.h>

// Get the process ID of desired App
int getProcId(const wchar_t* procName) {
    DWORD pID = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    // Take snapshots of everything
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    do {
        if (wcscmp(pe32.szExeFile, procName) == 0) {
            CloseHandle(hSnapshot);
            pID = pe32.th32ParentProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));
    CloseHandle(hSnapshot);
    return pID;
}

inline bool exist(const std::string& name)
{
    std::ifstream file(name);
    if (!file)            // If the file was not found, then file is 0, i.e. !file=1 or true.
        return false;    // The file was not found.
    else                 // If the file was found, then file is non-0.
        return true;     // The file was found.
}

void ErrorExit(const wchar_t* lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}



int main()
{
    HANDLE hProcess;
    LPVOID pszLibFileRemote = NULL;
    HANDLE handleThread;
    const wchar_t* process = L"EMPIRESX.EXE";
    int pID = getProcId(process);

    char dll[] = "AOEResourceHack.dll";
    if (!exist(dll)) {
        std::cout << "debuginfo: Invalid DLL path!" << std::endl;
    }
    char dllPath[MAX_PATH] = { 0 }; // full path of DLL
    GetFullPathNameA(dll, MAX_PATH, dllPath, NULL);
    std::cout << "debuginfo: Full DLL path: " << dllPath << std::endl;


    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |PROCESS_VM_WRITE, 0, pID);

    if (hProcess) {
        pszLibFileRemote = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }
    else {
        std::cout << "error: Could not open process handle with id:" << pID << std::endl;
    }
    if (pszLibFileRemote == NULL) std::cout << "error: Cannot allocate memory" << std::endl;

    int isWriteOK = WriteProcessMemory(hProcess, pszLibFileRemote, dllPath, strlen(dllPath) + 1, NULL);
    if (!isWriteOK) std::cout << "error: Failed to write" << std::endl;

    handleThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA, pszLibFileRemote, NULL, NULL);
    if (handleThread == NULL) {
        std::cout << "error: Failed to create thread" << std::endl;
        ErrorExit(_T("CreateRemoteThread"));
    }

    WaitForSingleObject(handleThread, INFINITE);
    CloseHandle(handleThread);
    VirtualFreeEx(hProcess, dllPath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return 0;
    
}
