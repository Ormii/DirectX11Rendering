#pragma once
#include "EngineBase.h"

class Engine : public EngineBase
{
public:
	Engine();
	virtual ~Engine();

public:
	virtual void Update() override;
	virtual void Render() override;
};

