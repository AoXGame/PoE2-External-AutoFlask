#include <windows.h>
#include "memory.h"
#include "game.h"
#include "flask.h"
#include "patcher.h"
#include "gui.h"
#include "config.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    MemoryReader memory;
    bool processAttached = memory.Attach("PathOfExile2_x64Steam.exe") || memory.Attach("PathOfExile2_x64EGS.exe");
    
    Config config;
    GameDataReader* gameData = nullptr;
    FlaskManager* flaskManager = nullptr;
    MemoryPatcher* patcher = nullptr;
    
    if (processAttached) {
        gameData = new GameDataReader(memory);
        if (!gameData->Initialize()) {
            delete gameData;
            gameData = nullptr;
            processAttached = false;
        } else {
            flaskManager = new FlaskManager(*gameData, config);
            patcher = new MemoryPatcher(memory);
            patcher->Initialize();
        }
    }

    GUI gui;
    if (!gui.Create()) {
        MessageBoxA(nullptr, "Failed to create GUI window.\n\nPossible causes:\n- DirectX 11 not available\n- Insufficient permissions", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastTime);

    bool lastPatch1State = false;
    bool lastPatch2State = false;

    while (!gui.ShouldClose()) {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        float deltaTime = (float)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        lastTime = currentTime;
        float fps = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;

        VitalStruct hp, mp;
        float hpPercent = 0.0f;
        float mpPercent = 0.0f;

        if (processAttached && gameData) {
            if (gameData->GetPlayerHP(hp) && hp.total > 0) {
                hpPercent = (float)hp.current / (float)hp.total;
            }
            if (gameData->GetPlayerMP(mp) && mp.total > 0) {
                mpPercent = (float)mp.current / (float)mp.total;
            }
        }

        config.hpThreshold = gui.GetHPThreshold();
        config.mpThreshold = gui.GetMPThreshold();
        config.autoHP = gui.IsHPEnabled();
        config.autoMP = gui.IsMPEnabled();

        bool currentPatch1State = gui.IsPatch1Enabled();
        bool currentPatch2State = gui.IsPatch2Enabled();

        if (processAttached && patcher) {
            if (currentPatch1State != lastPatch1State) {
                patcher->ApplyPatch1(currentPatch1State);
                lastPatch1State = currentPatch1State;
            }

            if (currentPatch2State != lastPatch2State) {
                patcher->ApplyPatch2(currentPatch2State);
                lastPatch2State = currentPatch2State;
            }
        }

        if (processAttached && flaskManager) {
            flaskManager->Update();
        }
        gui.Render(config, hpPercent, mpPercent, fps);

        Sleep(10);
    }

    if (patcher) {
        patcher->RestoreAll();
        delete patcher;
    }
    if (flaskManager) {
        delete flaskManager;
    }
    if (gameData) {
        delete gameData;
    }
    gui.Shutdown();
    return 0;
}

