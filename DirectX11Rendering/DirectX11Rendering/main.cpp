

#include "pch.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    Engine engine;
    if (!engine.Initialize())
        return ERRNO;

    return engine.Run();
}