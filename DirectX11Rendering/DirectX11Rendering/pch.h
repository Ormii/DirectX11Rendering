#pragma once
// add headers that you want to pre-compile here
#include "CorePch.h"


#define MAX_LIGHTS 16

#include "Material.h"
#include "Light.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "ProxyModel.h"
#include "CubeMap.h"
#include "ImageFilter.h"

#include "Engine.h"


#ifdef _DEBUG
#pragma comment(lib, "Debug/EngineCore.lib")
#else
#pragma comment(lib, "Release/EngineCore.lib")
#endif
