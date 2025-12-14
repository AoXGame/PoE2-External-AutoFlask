#include "game.h"
#include <windows.h>

bool GameDataReader::Initialize() {
    std::vector<BYTE> pattern = { 0x45, 0x8B, 0xF8, 0x45, 0x2B, 0xF9, 0x40, 0x38 };
    std::vector<BYTE> mask =    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    uintptr_t patternAddr = memory.FindPattern(pattern, mask);
    if (patternAddr == 0) {
        characterAddress = 0;
        return true;
    }
    
    characterAddress = 0;
    return true;
}

bool GameDataReader::GetPlayerHP(VitalStruct& hp) {
    if (characterAddress == 0) {
        if (!FindCharacterAddress()) return false;
    }
    
    hp.current = memory.Read<int>(characterAddress + Offsets::Character_CurrentHP);
    hp.total = memory.Read<int>(characterAddress + Offsets::Character_MaxHP);
    hp.reservedFlat = 0;
    hp.reservedPercent = 0;
    
    return (hp.total > 0 && hp.total < 100000);
}

bool GameDataReader::FindCharacterAddress() {
    struct Candidate {
        uintptr_t address;
        int hp;
        int maxHP;
        int mana;
        int maxMana;
        int score;
    };
    
    std::vector<Candidate> candidates;
    
    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t address = 0;
    
    while (VirtualQueryEx(memory.processHandle, (LPCVOID)address, &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT && 
            mbi.Type == MEM_PRIVATE &&
            mbi.Protect == PAGE_READWRITE &&
            mbi.RegionSize > 0x10000) {
            
            for (uintptr_t addr = (uintptr_t)mbi.BaseAddress; 
                 addr < (uintptr_t)mbi.BaseAddress + mbi.RegionSize - 0x1000; 
                 addr += 0x1000) {
                
                int currentHP = memory.Read<int>(addr + Offsets::Character_CurrentHP);
                int maxHP = memory.Read<int>(addr + Offsets::Character_MaxHP);
                int currentMana = memory.Read<int>(addr + Offsets::Character_CurrentMana);
                int maxMana = memory.Read<int>(addr + Offsets::Character_MaxMana);
                
                if (currentHP > 0 && currentHP <= maxHP && 
                    maxHP >= 50 && maxHP <= 5000 &&
                    currentMana >= 0 && currentMana <= maxMana &&
                    maxMana >= 30 && maxMana <= 5000) {
                    
                    int score = maxHP;
                    float hpManaRatio = (maxMana > 0) ? (float)maxHP / (float)maxMana : 999.0f;
                    if (hpManaRatio > 10.0f || hpManaRatio < 0.1f) {
                        score += 10000;
                    }
                    
                    Candidate c;
                    c.address = addr;
                    c.hp = currentHP;
                    c.maxHP = maxHP;
                    c.mana = currentMana;
                    c.maxMana = maxMana;
                    c.score = score;
                    candidates.push_back(c);
                }
            }
        }
        
        address = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
    }
    
    if (candidates.empty()) {
        return false;
    }
    
    Sleep(2000);
    
    std::vector<Candidate> active;
    
    for (const auto& c : candidates) {
        int newHP = memory.Read<int>(c.address + Offsets::Character_CurrentHP);
        int newMana = memory.Read<int>(c.address + Offsets::Character_CurrentMana);
        int newMaxHP = memory.Read<int>(c.address + Offsets::Character_MaxHP);
        int newMaxMana = memory.Read<int>(c.address + Offsets::Character_MaxMana);
        
        if (newMaxHP == c.maxHP && newMaxMana == c.maxMana &&
            newHP > 0 && newHP <= newMaxHP &&
            newMana >= 0 && newMana <= newMaxMana) {
            
            bool valuesChanged = (newHP != c.hp || newMana != c.mana);
            
            Candidate updated;
            updated.address = c.address;
            updated.hp = newHP;
            updated.maxHP = newMaxHP;
            updated.mana = newMana;
            updated.maxMana = newMaxMana;
            updated.score = c.score;
            if (valuesChanged) {
                updated.score -= 1000;
            }
            active.push_back(updated);
        }
    }
    
    if (active.empty()) {
        return false;
    }
    
    std::sort(active.begin(), active.end(), 
        [](const Candidate& a, const Candidate& b) { return a.score < b.score; });
    
    characterAddress = active[0].address;
    return true;
}

bool GameDataReader::GetPlayerMP(VitalStruct& mp) {
    if (characterAddress == 0) {
        if (!FindCharacterAddress()) return false;
    }
    
    mp.current = memory.Read<int>(characterAddress + Offsets::Character_CurrentMana);
    mp.total = memory.Read<int>(characterAddress + Offsets::Character_MaxMana);
    mp.reservedFlat = 0;
    mp.reservedPercent = 0;
    
    return (mp.total > 0 && mp.total < 100000);
}

