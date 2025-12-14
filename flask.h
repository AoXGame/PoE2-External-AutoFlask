#pragma once
#include "game.h"
#include "config.h"
#include <windows.h>

class FlaskManager {
private:
    GameDataReader& gameData;
    Config& config;
    DWORD lastHPFlaskTime;
    DWORD lastMPFlaskTime;
    const DWORD FLASK_COOLDOWN = 100;

public:
    FlaskManager(GameDataReader& gd, Config& cfg) 
        : gameData(gd), config(cfg), lastHPFlaskTime(0), lastMPFlaskTime(0) {}

    void Update();
    void UseFlask(int flaskNumber);
};

