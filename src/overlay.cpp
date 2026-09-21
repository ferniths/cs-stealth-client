#include "overlay.h"
#define NOMINMAX
#include <dwmapi.h>
#include <dxgi.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static Overlay* g_overlay = nullptr;

static LRESULT CALLBACK overlay_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return 1;
    switch (msg) {
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_SIZE:
        if (g_overlay && wparam != SIZE_MINIMIZED) {
            g_overlay->width = LOWORD(lparam);
            g_overlay->height = HIWORD(lparam);
        }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

bool Overlay::create() {
    g_overlay = this;
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = overlay_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"StealthOverlay";
    RegisterClassExW(&wc);

    width = 1920; height = 1080;
    hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName, L"", WS_POPUP, 0, 0, width, height,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) return false;

    // STREAM-PROOF: invisible to OBS, Discord, Game Bar, etc.
    SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
    SetLayeredWindowAttributes(hwnd, RGB(10, 8, 16), 0, LWA_COLORKEY);

    // Click-through by default
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    style |= WS_EX_TRANSPARENT;
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, style);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width; sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate = {60, 1};
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc = {1, 0};
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        &fl, 1, D3D11_SDK_VERSION, &sd, &swap_chain, &device, nullptr, &context))) {
        destroy(); return false;
    }

    ID3D11Texture2D* bb = nullptr;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&bb));
    if (bb) { device->CreateRenderTargetView(bb, nullptr, &render_target); bb->Release(); }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
    ShowWindow(hwnd, SW_SHOW);
    return true;
}

void Overlay::set_open(bool v) { menu_open = v; update_click_through(); }

void Overlay::update_click_through() {
    LONG_PTR s = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (menu_open) s &= ~WS_EX_TRANSPARENT; else s |= WS_EX_TRANSPARENT;
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, s);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void Overlay::destroy() {
    ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
    if (render_target) { render_target->Release(); render_target = nullptr; }
    if (swap_chain) { swap_chain->Release(); swap_chain = nullptr; }
    if (context) { context->Release(); context = nullptr; }
    if (device) { device->Release(); device = nullptr; }
    if (hwnd) { DestroyWindow(hwnd); hwnd = nullptr; }
    g_overlay = nullptr;
}

void Overlay::begin_frame() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg); DispatchMessageW(&msg);
        if (msg.message == WM_QUIT) { menu_open = false; return; }
    }
    ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();
    const float cc[4] = { 10.0f/255, 8.0f/255, 16.0f/255, 1.0f };
    context->OMSetRenderTargets(1, &render_target, nullptr);
    context->ClearRenderTargetView(render_target, cc);
}

void Overlay::end_frame() {
    ImGui::Render(); ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swap_chain->Present(1, 0);
}
