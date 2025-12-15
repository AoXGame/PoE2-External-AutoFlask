#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <vector>

class MemoryReader {
public:
    HANDLE processHandle;
    DWORD processId;

    MemoryReader() : processHandle(nullptr), processId(0) {}
    ~MemoryReader() {
        if (processHandle) CloseHandle(processHandle);
    }

    bool Attach(const char* processName);
    bool ReadMemory(uintptr_t address, void* buffer, size_t size);
    template<typename T> T Read(uintptr_t address);
    bool WriteMemory(uintptr_t address, const void* buffer, size_t size);
    template<typename T> bool Write(uintptr_t address, const T& value);
    uintptr_t FindPattern(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask);
    std::vector<BYTE> ReadOriginalBytes(uintptr_t address, size_t size);

private:
    bool EnableDebugPrivilege();
    DWORD GetProcessIdByName(const char* processName);
};

template<typename T>
T MemoryReader::Read(uintptr_t address) {
    T value = T();
    ReadMemory(address, &value, sizeof(T));
    return value;
}

template<typename T>
bool MemoryReader::Write(uintptr_t address, const T& value) {
    return WriteMemory(address, &value, sizeof(T));
}





