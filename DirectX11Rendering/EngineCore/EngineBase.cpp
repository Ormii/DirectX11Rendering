#include "pch.h"
#include "EngineBase.h"

EngineBase* g_EngineBase = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return g_EngineBase->MsgProc(hWnd, msg, wParam, lParam);
}

EngineBase::EngineBase()
	:m_screenWidth(MAIN_WINDOW_WIDTH), m_screenHeight(MAIN_WINDOW_HEIGHT), m_hwnd(0)
{
	g_EngineBase = this;
}

EngineBase::~EngineBase()
{
	g_EngineBase = nullptr;

    DestroyWindow(m_hwnd);
}

int EngineBase::Run()
{
    MSG msg = { 0 };
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Update();
            Render();
        }
    }

    return 0;
}

bool EngineBase::Initialize()
{
    if (!InitMainWindow())
        return false;

    return true;
}

LRESULT EngineBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

bool EngineBase::InitMainWindow()
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"Engine",
                     NULL };

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    RECT wr = { 0, 0, m_screenWidth, m_screenHeight };
    DWORD dwStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    AdjustWindowRect(&wr, dwStyle, false);

    m_hwnd = CreateWindow(wc.lpszClassName, L"Engine", dwStyle,
        300,                
        10,                
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL, NULL, wc.hInstance, NULL);

    if (!m_hwnd) {
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);

    return true;
}