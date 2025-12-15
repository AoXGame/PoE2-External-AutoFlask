#pragma once
#include "memory.h"
#include <vector>
#include <cstdint>

class MemoryPatcher {
private:
    MemoryReader& memory;
    uintptr_t patch1Address;
    uintptr_t patch2Address;
    std::vector<BYTE> originalBytes1;
    std::vector<BYTE> originalBytes2;
    bool patch1Applied;
    bool patch2Applied;

public:
    MemoryPatcher(MemoryReader& mem);
    bool Initialize();
    void ApplyPatch1(bool enable);
    void ApplyPatch2(bool enable);
    void RestoreAll();

private:
    bool FindPatchAddresses();
};





