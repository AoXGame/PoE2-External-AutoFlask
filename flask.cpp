#include "flask.h"

void FlaskManager::UseFlask(int flaskNumber) {
    INPUT inputs[2] = {};
    
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_F1 + flaskNumber - 1;
    inputs[0].ki.dwFlags = 0;
    
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_F1 + flaskNumber - 1;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    
    SendInput(2, inputs, sizeof(INPUT));
}

void FlaskManager::Update() {
    DWORD currentTime = GetTickCount();
    
    if (config.autoHP) {
        VitalStruct hp;
        if (gameData.GetPlayerHP(hp) && hp.total > 0) {
            float hpPercent = (float)hp.current / (float)hp.total;
            
            if (hpPercent <= config.hpThreshold && (currentTime - lastHPFlaskTime) > FLASK_COOLDOWN) {
                UseFlask(1);
                lastHPFlaskTime = currentTime;
            }
        }
    }
    
    if (config.autoMP) {
        VitalStruct mp;
        if (gameData.GetPlayerMP(mp) && mp.total > 0) {
            float mpPercent = (float)mp.current / (float)mp.total;
            
            if (mpPercent <= config.mpThreshold && (currentTime - lastMPFlaskTime) > FLASK_COOLDOWN) {
                UseFlask(2);
                lastMPFlaskTime = currentTime;
            }
        }
    }
}

