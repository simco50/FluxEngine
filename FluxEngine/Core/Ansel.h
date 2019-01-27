#pragma once
#include "Subsystem.h"
#include "Rendering/Camera/Camera.h"

class Renderer;

namespace ansel
{
	struct SessionConfiguration;
	struct CaptureConfiguration;
}

class Ansel : public Subsystem
{
	FLUX_OBJECT(Ansel, Subsystem)

public:
	Ansel(Context* pContext);

	void Update();
	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
	bool IsInSession() const { return m_InSession; }

private:
	int SessionStarted(ansel::SessionConfiguration& configuration);
	void SessionStopped();
	void CaptureStarted(const ansel::CaptureConfiguration& configuration);
	void CaptureStopped();

	bool m_IsAvailable = false;
	Renderer* m_pRenderer = nullptr;
	Camera* m_pCamera = nullptr;
	bool m_InSession = false;

	bool m_WasPostprocessingEnabled = false;
	View m_View;
	float m_FoV;
	Vector3 m_Position;
	float m_Width;
	float m_Height;
	Quaternion m_Rotation;
};