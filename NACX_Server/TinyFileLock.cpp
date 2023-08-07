#include "TinyFileLock.h"

#include <Windows.h>
#include <tlhelp32.h>
#include <tchar.h> 

TinyFileLock::TinyFileLock() {}

bool TinyFileLock::running()
{
    int count = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    BOOL bMore = ::Process32First(hProcessSnap, &pe32);
    while (bMore) {
        int len = WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, wcslen(pe32.szExeFile),
            NULL, 0, NULL, NULL);
        char* m_char = new char[len + 1];
        WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, wcslen(pe32.szExeFile),
            m_char, len, NULL, NULL);
        m_char[len] = '\0';

        if (strcmp("NACX_Server.exe", m_char) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                count++;
            }
        }
        bMore = ::Process32Next(hProcessSnap, &pe32);
        delete[] m_char;
    }
    return count > 1 ? true : false;;
}

//bool TinyFileLock::running()
//{
//    HANDLE hMutex = CreateMutex(NULL, FALSE, _T("TestAppName"));
//    if (GetLastError() == ERROR_ALREADY_EXISTS)
//    {
//        CloseHandle(hMutex);
//        return FALSE;
//    }
//}
