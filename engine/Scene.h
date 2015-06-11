#ifndef SCENE_H
#define SCENE_H

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

#pragma region Includes

#include "InputHandler.h"
#include "Direct3D.h"
#include "Camera.h"
#include "Terrain.h"
#include "Timer.h"
#include "Position.h"
#include "FrameRate.h"
#include "CPU.h"
#include "FontShader.h"
#include "Text.h"
#include "TerrainShader.h"
#include "Light.h"
#include "Frustum.h"
#include "QuadTree.h"
#include "SkyDome.h"
#include "SkyDomeShader.h"
#include "skyplane.h"
#include "skyplaneshader.h"
#include "TextureShader.h"
#include "Map.h"
#include "RenderTexture.h"
#include "HorizontalBlurShader.h"
#include "VerticalBlurShader.h"
#include "RenderTexture.h"
#include "OrthographicWindow.h"

#pragma endregion

class Scene
{
public:
	Scene();
	Scene(const Scene&);
	~Scene();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	bool HandleInput(float);
	bool DownSampleTexture();
	bool UpSampleTexture();
	bool RenderHorizontalBlurToTexture();
	bool RenderVerticalBlurToTexture();
	bool RenderSceneToTexture();
	bool Render2DTextureScene();
	bool Render();

private:
	InputHandler* m_Input;
	Direct3D* m_Direct3D;
	Camera* m_Camera;
	Camera* m_BlurSceneCamera;
	Terrain* m_Terrain;
	Timer* m_Timer;
	Position* m_Position;
	FrameRate* m_Fps;
	CPU* m_Cpu;
	FontShader* m_FontShader;
	Text* m_Text;
	TerrainShader* m_TerrainShader;
	Light* m_Light;
	Frustum* m_Frustum;
	QuadTree* m_QuadTree;
	SkyDome* m_SkyDome;
	SkyDomeShader* m_SkyDomeShader;
	SkyPlane *m_SkyPlane;
	SkyPlaneShader* m_SkyPlaneShader;
	TextureShader* m_TextureShader;
	Map* m_MiniMap;

	HorizontalBlurShader* m_HorizontalBlurShader;
	VerticalBlurShader* m_VerticalBlurShader;
	RenderTexture *m_RenderTexture, *m_DownSampleTexure, *m_HorizontalBlurTexture, *m_VerticalBlurTexture, *m_UpSampleTexure;
	OrthographicWindow *m_SmallWindow, *m_FullScreenWindow;

	bool m_IsBlurEnabled;
};

#endif