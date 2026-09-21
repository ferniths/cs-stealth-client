#pragma once
#include <Windows.h>
#include <d3d11.h>

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

class Overlay {
public:
    bool create();
    void destroy();
    void begin_frame();
    void end_frame();
    void set_open(bool v);
    void update_click_through();

    HWND hwnd = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* render_target = nullptr;
    int width = 0, height = 0;
    bool menu_open = false;
    bool is_open() const { return menu_open; }
};
