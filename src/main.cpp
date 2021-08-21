#include <Windows.h>
#include <TCHAR.h>

#include <string>
#include <variant>

HRESULT PrepareStartupInformation(HPCON hpc, STARTUPINFOEX* psi) {
    STARTUPINFOEX si;
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEX);

    // Discover the size required for the list
    size_t bytesRequired;
    InitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);

    // Allocate memory to represent the list
    si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, bytesRequired);
    if (!si.lpAttributeList) {
        return E_OUTOFMEMORY;
    }

    // Initialize the list memory location
    if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &bytesRequired)) {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Set the pseudoconsole information into the list
    if (!UpdateProcThreadAttribute(si.lpAttributeList,
                                   0,
                                   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                   hpc,
                                   sizeof(hpc),
                                   NULL,
                                   NULL)) {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *psi = si;
    return S_OK;
}

bool SetupPtyMode() {
     DWORD consoleMode = 0;
     const HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
     if (!GetConsoleMode(stdoutHandle, &consoleMode)) {
       return false;
     }
     if (!SetConsoleMode(stdoutHandle, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN)) {
         return false;
     }

     return true;
}

struct console_context {
    HPCON consoleHandle;
};

std::variant<HRESULT, console_context> SetUpPseudoConsole(const COORD size) {
    HANDLE inputReadSide, outputWriteSide;
    HANDLE outputReadSide, inputWriteSide;

    if (!CreatePipe(&inputReadSide, &inputWriteSide, NULL, 0)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!CreatePipe(&outputReadSide, &outputWriteSide, NULL, 0)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HPCON consoleHandle;
    const HRESULT createConsoleResult = CreatePseudoConsole(size, inputReadSide, outputWriteSide, 0, &consoleHandle);
    if (FAILED(createConsoleResult)) {
        return createConsoleResult;
    }

    PCTSTR childApplication = L"C:\\windows\\system32\\cmd.exe";

    const size_t charsRequired = _tcslen(childApplication) + 1; // +1 null terminator
    PTSTR cmdLineMutable = (PTSTR)HeapAlloc(GetProcessHeap(), 0, sizeof(TCHAR) * charsRequired);

    if (!cmdLineMutable) {
        return E_OUTOFMEMORY;
    }

    _tcscpy_s(cmdLineMutable, charsRequired, childApplication);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    STARTUPINFOEX siEx;
    const HRESULT sixOk = PrepareStartupInformation(consoleHandle, &siEx);
    if (FAILED(sixOk)) {
        return sixOk;
    }

    // Call CreateProcess
    if (!CreateProcess(NULL,
                       cmdLineMutable,
                       NULL,
                       NULL,
                       FALSE,
                       EXTENDED_STARTUPINFO_PRESENT,
                       NULL,
                       NULL,
                       &siEx.StartupInfo,
                       &pi)) {
        HeapFree(GetProcessHeap(), 0, cmdLineMutable);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const console_context ctx { consoleHandle };
    return ctx;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return 0;
}
