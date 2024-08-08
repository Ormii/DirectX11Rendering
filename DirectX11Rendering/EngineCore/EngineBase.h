#pragma once

class EngineBase
{
public: 
	EngineBase();
	virtual ~EngineBase();

public:
	int Run();

public:
	virtual bool Initialize();
	virtual void Update() = 0;
	virtual void Render() = 0;

public:
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	bool InitMainWindow();

protected:
	int m_screenWidth;
	int m_screenHeight;
	HWND m_hwnd;
};