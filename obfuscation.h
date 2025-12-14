#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <intrin.h>
#include <cstring>

#ifdef _WIN64
typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[21];
    PVOID Reserved3[2];
    PVOID Ldr;
    PVOID ProcessParameters;
} PEB, *PPEB;
#else
typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PVOID Ldr;
    PVOID ProcessParameters;
} PEB, *PPEB;
#endif

class AntiDebug {
public:
    static bool Check() {
        if (IsDebuggerPresent()) return true;
        if (CheckRemoteDebugger()) return true;
        if (CheckNtGlobalFlag()) return true;
        return false;
    }

private:
    static bool CheckRemoteDebugger() {
        BOOL debuggerPresent = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &debuggerPresent);
        return debuggerPresent == TRUE;
    }

    static bool CheckNtGlobalFlag() {
#ifdef _WIN64
        PPEB peb = (PPEB)__readgsqword(0x60);
        if (peb && peb->BeingDebugged) return true;
#else
        PPEB peb = (PPEB)__readfsdword(0x30);
        if (peb && peb->BeingDebugged) return true;
#endif
        return false;
    }
};

class ObfuscatedAPI {
private:
    HMODULE hModule;
    FARPROC pFunc;
    char key;

public:
    ObfuscatedAPI(const char* moduleName, const char* funcName, char xorKey) : key(xorKey), hModule(nullptr), pFunc(nullptr) {
        size_t modLen = strlen(moduleName);
        size_t funcLen = strlen(funcName);
        char* obfModule = new char[modLen + 1];
        char* obfFunc = new char[funcLen + 1];
        
        for (size_t i = 0; i < modLen; i++) obfModule[i] = moduleName[i] ^ key;
        obfModule[modLen] = 0;
        for (size_t i = 0; i < funcLen; i++) obfFunc[i] = funcName[i] ^ key;
        obfFunc[funcLen] = 0;
        
        hModule = LoadLibraryA(obfModule);
        if (hModule) {
            pFunc = GetProcAddress(hModule, obfFunc);
        }
        
        delete[] obfModule;
        delete[] obfFunc;
    }

    template<typename T>
    T Call() {
        if (pFunc) return reinterpret_cast<T>(pFunc)();
        return T();
    }

    template<typename T, typename... Args>
    T Call(Args... args) {
        if (pFunc) return reinterpret_cast<T(*)(Args...)>(pFunc)(args...);
        return T();
    }

    ~ObfuscatedAPI() {
        if (hModule) FreeLibrary(hModule);
    }
};

#define JUNK_CODE_1 \
    volatile int _junk1 = 0; \
    for (int _i = 0; _i < 3; _i++) { _junk1 += _i * 2; } \
    if (_junk1 > 100) { _junk1 = 0; }

#define JUNK_CODE_2 \
    volatile float _junk2 = 1.0f; \
    _junk2 *= 2.5f; \
    _junk2 /= 1.3f; \
    if (_junk2 < 0) { _junk2 = 0; }

#define JUNK_CODE_3 \
    volatile char _junk3[8] = {0}; \
    for (int _j = 0; _j < 8; _j++) { _junk3[_j] = (char)(_j ^ 0xAA); }

#define POLY_START \
    volatile int _poly_var = GetTickCount(); \
    if (_poly_var % 2 == 0) { _poly_var++; } else { _poly_var--; }

#define POLY_END \
    _poly_var = 0;

class ObfuscatedString {
private:
    char* data;
    size_t len;
    char key;

public:
    ObfuscatedString(const char* str, char xorKey) : key(xorKey) {
        len = strlen(str);
        data = new char[len + 1];
        for (size_t i = 0; i < len; i++) {
            data[i] = str[i] ^ key;
        }
        data[len] = 0;
    }

    ~ObfuscatedString() {
        delete[] data;
    }

    const char* Get() const {
        static thread_local char buffer[256];
        for (size_t i = 0; i < len && i < 255; i++) {
            buffer[i] = data[i] ^ key;
        }
        buffer[len < 255 ? len : 255] = 0;
        return buffer;
    }
};

