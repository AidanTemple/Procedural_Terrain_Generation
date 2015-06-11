#include "OculusManager.h"
#include<Windows.h>
#define  OVR_D3D_VERSION 11
#include <OVR_CAPI_D3D.h>
#include "applicationclass.h"

OculusManager::OculusManager()
	:Yaw(3.141592f),Pos(0.0f,1.6f,-5.0f){}


OculusManager::~OculusManager(void)
{
}

bool OculusManager::Initialize(D3DClass* d3dManager)
{
	// Init the library
	bool result = ovr_Initialize();

	if (!result)
	{
		return false;
	}

	// take the first oculus detected
	oculus = ovrHmd_Create(0);

	if (!oculus)
	{
		int res = MessageBoxA(0, "Oculus Rift not detected. Continue?", "Oculus Rift Detection",
			MB_OKCANCEL|MB_ICONWARNING);
		if (res == IDCANCEL)
		{
			exit(0);
		}
		// create a virtual device, basing on the development kit 2
		oculus = ovrHmd_CreateDebug(ovrHmd_DK2);
	}

	// initialize the sensors with orientation, yaw correction, and position tracking
	result =  ovrHmd_ConfigureTracking(oculus,  
										ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position,0);

	// Stereo Settings
	OVR::Sizei defaultLeftEyeTextureSize = ovrHmd_GetFovTextureSize( oculus,ovrEye_Left,oculus->DefaultEyeFov[0],1.f);
	OVR::Sizei defaultRightEyeTextureSize = ovrHmd_GetFovTextureSize( oculus,ovrEye_Right,oculus->DefaultEyeFov[1],1.f);

	OVR::Sizei renderTargetSize;
	renderTargetSize.w = defaultLeftEyeTextureSize.w + defaultRightEyeTextureSize.w;
	renderTargetSize.h = max(defaultLeftEyeTextureSize.h, defaultRightEyeTextureSize.h);

	const int eyeRenderMultisample = 1; 
	for (int i = 0; i < 2; i++)
	{
		OVR::Sizei idealSize     = ovrHmd_GetFovTextureSize(oculus, (ovrEyeType)i,oculus->DefaultEyeFov[i], 1.0f);
		eyeRenderTexture[i]      = new ImageBuffer(true,d3dManager->GetDevice(),d3dManager->GetDeviceContext(), false, idealSize);
		eyeDepthBuffer[i]        = new ImageBuffer(true,d3dManager->GetDevice(),d3dManager->GetDeviceContext(), true, eyeRenderTexture[i]->Size);

	}

	// Rendering Settings
	ovrD3D11Config d3d11cfg;
    d3d11cfg.D3D11.Header.API            = ovrRenderAPI_D3D11;
    d3d11cfg.D3D11.Header.BackBufferSize = OVR::Sizei(oculus->Resolution.w, oculus->Resolution.h);
    d3d11cfg.D3D11.Header.Multisample    = 1;
	d3d11cfg.D3D11.pDevice               = d3dManager->GetDevice();
    d3d11cfg.D3D11.pDeviceContext        = d3dManager->GetDeviceContext();
    d3d11cfg.D3D11.pBackBufferRT         = d3dManager->GetRenderTargetView();
    d3d11cfg.D3D11.pSwapChain            = d3dManager->GetSwapChain();

    if (!ovrHmd_ConfigureRendering(oculus, &d3d11cfg.Config,
                                   ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette |
                                   ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive,
                                   oculus->DefaultEyeFov, EyeRenderDesc))
        return(1);

	return result;
}

void OculusManager::Shutdown()
{
	// delete img buffer
	for (int i = 0; i < 2; i++)
	{
		if (eyeRenderTexture[i] != 0)
		{
			delete eyeRenderTexture[i];
			eyeRenderTexture[i] = 0;
		}

		if (eyeDepthBuffer[i] != 0)
		{
			delete eyeDepthBuffer[i];
			eyeDepthBuffer[i] = 0;
		}
	}

	// destroy the object and shut down the library
	ovrHmd_Destroy(oculus);
	ovr_Shutdown();
}

void OculusManager::render(D3DClass* d3dManager)
{
	ovrVector3f useHmdToEyeViewOffset[2] = {EyeRenderDesc[0].HmdToEyeViewOffset,
			                                    EyeRenderDesc[1].HmdToEyeViewOffset};
	ovrHmd_BeginFrame(oculus, 0);
	// Get both eye poses simultaneously, with IPD offset already included. 
	ovrPosef temp_EyeRenderPose[2];
	ovrHmd_GetEyePoses(oculus, 0, useHmdToEyeViewOffset, temp_EyeRenderPose, NULL);

	 // Render the two undistorted eye views into their render buffers.  
	for (int i = 0; i < 2; i++)
	{
		ImageBuffer * useBuffer      = eyeRenderTexture[i];  
		ovrPosef    * useEyePose     = &EyeRenderPose[i];
        float       * useYaw         = &YawAtRender[i];
        bool          clearEyeImage  = true;
        bool          updateEyeImage = true;

		// clear and set the rendering target -- TODO: should be in a function pheraps
		float black[] = {0, 0, 0, 1}; 
		OVR::Recti viewport (eyeRenderViewport[i]);
		d3dManager->GetDeviceContext()->OMSetRenderTargets(1, &(useBuffer->TexRtv),eyeDepthBuffer[i]->TexDsv);
		d3dManager->GetDeviceContext()->ClearRenderTargetView(useBuffer->TexRtv,black);
		d3dManager->GetDeviceContext()->ClearDepthStencilView(eyeDepthBuffer[i]->TexDsv,D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL ,1,0);
		D3D11_VIEWPORT D3Dvp;
		D3Dvp.Width    = (float)viewport.w;    D3Dvp.Height   = (float)viewport.h;
		D3Dvp.MinDepth = 0;              D3Dvp.MaxDepth = 1;
		D3Dvp.TopLeftX = (float)viewport.x;    D3Dvp.TopLeftY = (float)viewport.y;    
		d3dManager->GetDeviceContext()->RSSetViewports(1, &D3Dvp);
		
		// Write in values actually used (becomes significant in Example features)
		*useEyePose = temp_EyeRenderPose[i];
		*useYaw     = Yaw;

        // Get view and projection matrices (note near Z to reduce eye strain)
		OVR::Matrix4f rollPitchYaw       = OVR::Matrix4f::RotationY(Yaw);
		OVR::Matrix4f finalRollPitchYaw  = rollPitchYaw * OVR::Matrix4f(useEyePose->Orientation);
		OVR::Vector3f finalUp            = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
		OVR::Vector3f finalForward       = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
		OVR::Vector3f shiftedEyePos      = Pos + rollPitchYaw.Transform(useEyePose->Position);
		
		OVR::Matrix4f view = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
		OVR::Matrix4f proj = ovrMatrix4f_Projection(EyeRenderDesc[i].Fov, 0.2f, 1000.0f, true); 
		


	   }
}

void OculusManager::getEulerAngleRotation(float &yaw, float &pitch, float &roll)
{
	// Get the current tracking state
	ovrTrackingState trackingState = ovrHmd_GetTrackingState(oculus,ovr_GetTimeInSeconds());
	// get the current head pose
	OVR::Posef pose = trackingState.HeadPose.ThePose;
	pose.Rotation.GetEulerAngles<OVR::Axis_Y,OVR::Axis_X,OVR::Axis_Z>(&yaw,&pitch,&roll);
	
}


