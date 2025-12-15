#include "memory.h"
#include <cstring>
#include <cstdlib>

bool MemoryReader::EnableDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;

    LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bool result = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0) != FALSE;
    CloseHandle(hToken);
    return result;
}

DWORD MemoryReader::GetProcessIdByName(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    size_t processNameLen = strlen(processName);
    wchar_t* wProcessName = new wchar_t[processNameLen + 1];
    size_t converted = 0;
    mbstowcs_s(&converted, wProcessName, processNameLen + 1, processName, _TRUNCATE);

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, wProcessName) == 0) {
                delete[] wProcessName;
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    delete[] wProcessName;
    CloseHandle(snapshot);
    return 0;
}

bool MemoryReader::Attach(const char* processName) {
    EnableDebugPrivilege();
    processId = GetProcessIdByName(processName);
    if (processId == 0) return false;

    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    return processHandle != nullptr;
}

bool MemoryReader::ReadMemory(uintptr_t address, void* buffer, size_t size) {
    if (!processHandle) return false;
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(processHandle, (LPCVOID)address, buffer, size, &bytesRead) && bytesRead == size;
}

bool MemoryReader::WriteMemory(uintptr_t address, const void* buffer, size_t size) {
    if (!processHandle) return false;
    SIZE_T bytesWritten = 0;
    DWORD oldProtect;
    if (!VirtualProtectEx(processHandle, (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;
    bool result = WriteProcessMemory(processHandle, (LPVOID)address, buffer, size, &bytesWritten) && bytesWritten == size;
    VirtualProtectEx(processHandle, (LPVOID)address, size, oldProtect, &oldProtect);
    return result;
}

uintptr_t MemoryReader::FindPattern(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask) {
    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t address = 0;

    while (VirtualQueryEx(processHandle, (LPCVOID)address, &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_EXECUTE_READ || mbi.Protect == PAGE_EXECUTE_READWRITE || mbi.Protect == PAGE_READONLY || mbi.Protect == PAGE_READWRITE)) {
            std::vector<BYTE> buffer(mbi.RegionSize);
            SIZE_T bytesRead = 0;
            if (ReadProcessMemory(processHandle, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
                for (size_t i = 0; i <= bytesRead - pattern.size(); i++) {
                    bool match = true;
                    for (size_t j = 0; j < pattern.size(); j++) {
                        if (mask[j] == 0xFF && buffer[i + j] != pattern[j]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        return (uintptr_t)mbi.BaseAddress + i;
                    }
                }
            }
        }
        address = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
    }
    return 0;
}

std::vector<BYTE> MemoryReader::ReadOriginalBytes(uintptr_t address, size_t size) {
    std::vector<BYTE> bytes(size);
    SIZE_T bytesRead = 0;
    ReadProcessMemory(processHandle, (LPCVOID)address, bytes.data(), size, &bytesRead);
    return bytes;
}

