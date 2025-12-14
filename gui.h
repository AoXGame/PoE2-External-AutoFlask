#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "config.h"

class GUI {
private:
    HWND hwnd;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    bool initialized;
    
    float hpThreshold;
    float mpThreshold;
    bool hpEnabled;
    bool mpEnabled;
    bool patch1Enabled;
    bool patch2Enabled;
    bool shouldClose;

public:
    GUI();
    ~GUI();
    
    bool Create();
    void Render(Config& config, float hpPercent, float mpPercent, float fps);
    void Shutdown();
    bool ShouldClose() const { return shouldClose; }
    
    float GetHPThreshold() const { return hpThreshold; }
    float GetMPThreshold() const { return mpThreshold; }
    bool IsHPEnabled() const { return hpEnabled; }
    bool IsMPEnabled() const { return mpEnabled; }
    bool IsPatch1Enabled() const { return patch1Enabled; }
    bool IsPatch2Enabled() const { return patch2Enabled; }
    
    void SetPatch1Enabled(bool enabled) { patch1Enabled = enabled; }
    void SetPatch2Enabled(bool enabled) { patch2Enabled = enabled; }

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool InitD3D();
    void CleanupD3D();
};

