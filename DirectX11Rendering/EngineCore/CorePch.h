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
#include <intsafe.h>
#include <thread>
#include <future>
#include <mutex>
#include <functional>
#include <filesystem>
#include <cstring>
#include <unordered_map>
#include <initializer_list>
#include <tuple>

#include <vector>

using namespace std;

/*---------------------------
*		  DirectX11
-----------------------------*/
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <SimpleMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")

using DirectX::SimpleMath::Matrix;

/*---------------------------
*		   ImGui
-----------------------------*/
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>


using Microsoft::WRL::ComPtr;
using std::vector;
using std::wstring;


/*---------------------------
*		custom header
-----------------------------*/
#include "Types.h"
#include "CoreMacro.h"
#include "Lock.h"
#include "ThreadInfo.h"
#include "MeshData.h"
#include "MemoryManager.h"
#include "ResourceManager.h"
#include "ThreadManager.h"
#include "ModelLoader.h"
#include "EngineUtility.h"
#include "EngineBase.h"
#include "GeometryGenerator.h"
#include "SamplerGenerator.h"
#include "Container.h"


#define MAIN_WINDOW_WIDTH	1280
#define MAIN_WINDOW_HEIGHT	960

#define ERRNO	-1


//#define LOD_TEST
//#define FRUSTOM_CULLING_TEST
//#define RESOURCE_SHARE_TEST_CREATE_TEXTURE
//#define THREAD_POOL_TEST