#include "patcher.h"
#include "obfuscation.h"

MemoryPatcher::MemoryPatcher(MemoryReader& mem) 
    : memory(mem), patch1Address(0), patch2Address(0), patch1Applied(false), patch2Applied(false) {
}

bool MemoryPatcher::Initialize() {
    JUNK_CODE_1
    return FindPatchAddresses();
}

bool MemoryPatcher::FindPatchAddresses() {
    std::vector<BYTE> pattern1 = { 0xF3, 0x0F, 0x5D };
    std::vector<BYTE> mask1 =    { 0xFF, 0xFF, 0xFF };
    
    patch1Address = memory.FindPattern(pattern1, mask1);
    if (patch1Address == 0) return false;
    
    originalBytes1 = memory.ReadOriginalBytes(patch1Address, 3);
    if (originalBytes1.size() != 3) return false;
    
    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t address = 0;
    while (VirtualQueryEx(memory.processHandle, (LPCVOID)address, &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT && mbi.Protect == PAGE_EXECUTE_READWRITE) {
            for (uintptr_t addr = (uintptr_t)mbi.BaseAddress; 
                 addr < (uintptr_t)mbi.BaseAddress + mbi.RegionSize - 1; 
                 addr++) {
                BYTE byte = memory.Read<BYTE>(addr);
                if (byte == 0x00 || byte == 0x01) {
                    patch2Address = addr;
                    originalBytes2 = memory.ReadOriginalBytes(addr, 1);
                    return true;
                }
            }
        }
        address = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
    }
    
    return false;
}

void MemoryPatcher::ApplyPatch1(bool enable) {
    if (patch1Address == 0) return;
    
    if (enable && !patch1Applied) {
        std::vector<BYTE> nopBytes = { 0x90, 0x90, 0x90 };
        if (memory.WriteMemory(patch1Address, nopBytes.data(), 3)) {
            patch1Applied = true;
        }
    } else if (!enable && patch1Applied) {
        if (memory.WriteMemory(patch1Address, originalBytes1.data(), 3)) {
            patch1Applied = false;
        }
    }
}

void MemoryPatcher::ApplyPatch2(bool enable) {
    if (patch2Address == 0) return;
    
    if (enable && !patch2Applied) {
        BYTE newByte = 0x01;
        if (memory.WriteMemory(patch2Address, &newByte, 1)) {
            patch2Applied = true;
        }
    } else if (!enable && patch2Applied) {
        if (memory.WriteMemory(patch2Address, originalBytes2.data(), 1)) {
            patch2Applied = false;
        }
    }
}

void MemoryPatcher::RestoreAll() {
    ApplyPatch1(false);
    ApplyPatch2(false);
}





