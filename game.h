#pragma once
#include "memory.h"
#include <cstdint>
#include <vector>
#include <algorithm>

namespace Offsets {
    constexpr uintptr_t Character_CurrentHP = 0x360;
    constexpr uintptr_t Character_MaxHP = 0x364;
    constexpr uintptr_t Character_CurrentES = 0x36C;
    constexpr uintptr_t Character_MaxES = 0x370;
    constexpr uintptr_t Character_CurrentMana = 0x760;
    constexpr uintptr_t Character_MaxMana = 0x758;
}

struct VitalStruct {
    int current;
    int total;
    int reservedFlat;
    int reservedPercent;
};

class GameDataReader {
private:
    MemoryReader& memory;
    uintptr_t characterAddress;

public:
    GameDataReader(MemoryReader& mem) : memory(mem), characterAddress(0) {}

    bool Initialize();
    bool GetPlayerHP(VitalStruct& hp);
    bool GetPlayerMP(VitalStruct& mp);
    bool FindCharacterAddress();
};

