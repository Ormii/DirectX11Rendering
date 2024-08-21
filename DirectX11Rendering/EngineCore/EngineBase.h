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
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;

	virtual void UpdateGUI() = 0;

public:
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	bool InitMainWindow();
	bool InitDirectX();
	bool InitGUI();

protected:
	virtual void OnMouseMove(WPARAM wParam, int mouseX, int mouseY);
	virtual void KeyBeginPress(WPARAM wParam) = 0;
	virtual void KeyEndPress(WPARAM wParam) = 0;

public:
	float GetAspectRatio();

protected:
	bool CreateRenderTargetView();
	bool CreateRasterizerState();

	void SetViewport();

protected:
	int m_screenWidth;
	int m_screenHeight;
	HWND m_hwnd;

public:
	ComPtr<ID3D11Device> GetDevice() { return m_device; }
	ComPtr<ID3D11DeviceContext> GetContext() { return m_context; }

	ComPtr<ID3D11RasterizerState> GetSolidRasterizerState() { return m_solidRasterizerState; }
	ComPtr<ID3D11RasterizerState> GetWiredRasterizerState() { return m_wireRasterizerState; }

	D3D11_VIEWPORT* GetViewport() { return &m_viewport; }

	ComPtr<ID3D11ShaderResourceView> GetShaderResourceView() { return m_shaderResourceView; }

protected:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11RasterizerState> m_solidRasterizerState;
	ComPtr<ID3D11RasterizerState> m_wireRasterizerState;

	ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	D3D11_VIEWPORT m_viewport;

	UINT m_numQualityLevels;

	ComPtr<ID3D11Texture2D> m_tempTexture;
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;

protected:
	bool m_keyPressed[256] = { false, };

	int m_mouseCursorX;
	int m_mouseCursorY;
	float m_mouseCursorNdcX;
	float m_mouseCursorNdxY;

	float m_mousePrevCurserNdcX;
	float m_mousePrevCurserNdcY;

	bool m_leftButtonDown;
	bool m_rightButtonDowm;
};

extern EngineBase* g_EngineBase;
extern ResourceManager* g_ResourceManager;
extern MemoryManager* g_MemoryManager;
extern ThreadManager* g_ThreadManager;