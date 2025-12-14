#include "gui.h"
#include <d3d11.h>
#include <dxgi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

GUI::GUI() : hwnd(nullptr), device(nullptr), context(nullptr), swapChain(nullptr), 
             renderTargetView(nullptr), initialized(false), hpThreshold(0.6f), 
             mpThreshold(0.6f), hpEnabled(true), mpEnabled(true), 
             patch1Enabled(false), patch2Enabled(false), shouldClose(false) {
}

GUI::~GUI() {
    Shutdown();
}

LRESULT CALLBACK GUI::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GUI* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (GUI*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (GUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (pThis) {
        return pThis->WndProc(hwnd, msg, wParam, lParam);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT GUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (device && wParam != SIZE_MINIMIZED) {
            if (renderTargetView) {
                renderTargetView->Release();
                renderTargetView = nullptr;
            }
            swapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* pBackBuffer;
            swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
            pBackBuffer->Release();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        shouldClose = true;
        PostQuitMessage(0);
        return 0;
    case WM_NCHITTEST: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ScreenToClient(hwnd, &pt);
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (pt.y < 30 && pt.x < rect.right - 100) {
            return HTCAPTION;
        }
        break;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool GUI::Create() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"POE2AutoFlask";
    RegisterClassEx(&wc);

    hwnd = CreateWindowEx(0, L"POE2AutoFlask", L"POE2 Auto Flask",
        WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX,
        100, 100, 400, 500, nullptr, nullptr, wc.hInstance, this);

    if (!hwnd) return false;

    if (!InitD3D()) {
        CleanupD3D();
        return false;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);

    initialized = true;
    return true;
}

bool GUI::InitD3D() {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context) != S_OK)
        return false;

    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
    pBackBuffer->Release();

    return true;
}

void GUI::CleanupD3D() {
    if (renderTargetView) { renderTargetView->Release(); renderTargetView = nullptr; }
    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
    if (context) { context->Release(); context = nullptr; }
    if (device) { device->Release(); device = nullptr; }
}

void GUI::Render(Config& config, float hpPercent, float mpPercent, float fps) {
    if (!initialized) return;

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text("POE2 Auto Flask");
    ImGui::Separator();

    ImGui::Text("HP: %.0f%%", hpPercent * 100.0f);
    ImGui::SameLine();
    ImGui::ProgressBar(hpPercent, ImVec2(200, 20));
    ImGui::SameLine();
    if (ImGui::Button(hpEnabled ? "ON" : "OFF", ImVec2(50, 20))) {
        hpEnabled = !hpEnabled;
    }

    ImGui::SliderFloat("HP Threshold", &hpThreshold, 0.0f, 1.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp);
    config.hpThreshold = hpThreshold;
    config.autoHP = hpEnabled;

    ImGui::Spacing();

    ImGui::Text("MP: %.0f%%", mpPercent * 100.0f);
    ImGui::SameLine();
    ImGui::ProgressBar(mpPercent, ImVec2(200, 20));
    ImGui::SameLine();
    if (ImGui::Button(mpEnabled ? "ON" : "OFF", ImVec2(50, 20))) {
        mpEnabled = !mpEnabled;
    }

    ImGui::SliderFloat("MP Threshold", &mpThreshold, 0.0f, 1.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp);
    config.mpThreshold = mpThreshold;
    config.autoMP = mpEnabled;

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Checkbox("Zoomhack ( NOT WORKING )", &patch1Enabled);
    config.patch1Enabled = patch1Enabled;

    ImGui::Checkbox("Map Reveal ( NOT WORKING )", &patch2Enabled);
    config.patch2Enabled = patch2Enabled;

    ImGui::Spacing();
    ImGui::Text("FPS: %.1f", fps);

    ImGui::End();

    ImGui::Render();
    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);
    context->ClearRenderTargetView(renderTargetView, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    swapChain->Present(1, 0);
}

void GUI::Shutdown() {
    if (initialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupD3D();
        if (hwnd) {
            DestroyWindow(hwnd);
            hwnd = nullptr;
        }
        initialized = false;
    }
}

