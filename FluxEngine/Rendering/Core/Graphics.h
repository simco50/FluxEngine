#pragma once
#include "Core\Subsystem.h"

class RenderTarget;
class ShaderVariation;
class Texture;
class ConstantBuffer;
class Shader;
class Texture2D;
class GraphicsImpl;
class GraphicsCommandContext;
struct SDL_Window;

struct AdapterInfo
{
	std::string Name;
	size_t VideoMemory = 0;
	size_t SystemMemory = 0;
	unsigned int DeviceId = 0;
	unsigned int VendorId = 0;
};

struct GraphicsCreateInfo
{
	std::string Title = "FluxEngine";
	int WindowWidth = 1240;
	int WindowHeight = 720;
	WindowType WindowType = WindowType::WINDOWED;
	bool Resizable = true;
	bool VSync = true;
	int MultiSample = 1;
	int RefreshRate = 60;
};

class Graphics : public Subsystem
{
	FLUX_OBJECT(Graphics, Subsystem)

public:
	explicit Graphics(Context* pContext);
	~Graphics();

	DELETE_COPY(Graphics)

		bool SetMode(const GraphicsCreateInfo& createInfo);

	ConstantBuffer* GetOrCreateConstantBuffer(unsigned int index, unsigned int size);
	Shader* GetShader(const std::string& filePath);
	ShaderVariation* GetShader(const std::string& filePath, ShaderType type, const std::string& defines = "");

	GraphicsCommandContext* GetGraphicsCommandContext();

	void OnResize(int width, int height);
	void BeginFrame();
	void EndFrame();

	void TakeScreenshot();
	void TakeScreenshot(OutputStream& outputStream);

	//Getters
	const std::string& GetWindowTitle() const { return m_WindowTitle; }
	HWND GetWindow() const;
	SDL_Window* GetSDLWindow() const { return m_pWindow; }
	int GetWindowWidth() const { return m_WindowWidth; }
	int GetWindowHeight() const { return m_WindowHeight; }
	int GetMultisample() const { return m_Multisample; }

	bool GetAdapterInfo(AdapterInfo& adapterInfo);

	GraphicsImpl* GetImpl() const { return m_pImpl.get(); }

	Texture2D* GetDefaultRenderTarget() const { return m_pDefaultRenderTarget.get(); }
	Texture2D* GetDefaultDepthStencil() const { return m_pDefaultDepthStencil.get(); }

private:
	bool OpenWindow();
	bool EnumerateAdapters();
	bool CreateDevice(int windowWidth, int windowHeight);
	void UpdateSwapchain(int windowWidth, int windowHeight);

	static const int RENDERTARGET_FORMAT;
	static const int DEPTHSTENCIL_FORMAT;

	DelegateHandle m_WindowSizeChangedHandle;
	SDL_Window* m_pWindow = nullptr;
	int m_WindowWidth = 0;
	int m_WindowHeight = 0;
	bool m_Resizable = true;
	WindowType m_WindowType;
	std::string m_WindowTitle;
	bool m_Vsync = true;
	int m_Multisample = 1;
	int m_RefreshRate = 60;

	std::unique_ptr<GraphicsImpl> m_pImpl;

	//All ConstantBuffers
	std::map<size_t, std::unique_ptr<ConstantBuffer>> m_ConstantBuffers;
	static std::string m_ShaderExtension;

	std::unique_ptr<Texture2D> m_pDefaultRenderTarget;
	std::unique_ptr<Texture2D> m_pDefaultDepthStencil;
};