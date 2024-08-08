#pragma once

#include <SDKDDKVer.h>

// C RunTime Header Files
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <windows.h>
#include <wrl.h>

#include <vector>

/*---------------------------
*		  DirectX11
-----------------------------*/
#include <d3d11.h>
#include <d3dcompiler.h>


/*---------------------------
*		   ImGui
-----------------------------*/
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

/*---------------------------
*		custom header
-----------------------------*/
#include "Types.h"
#include "CoreMacro.h"

#include "EngineBase.h"


using Microsoft::WRL::ComPtr;
using std::vector;
using std::wstring;


#define MAIN_WINDOW_WIDTH	1280
#define MAIN_WINDOW_HEIGHT	960



#define ERRNO	-1