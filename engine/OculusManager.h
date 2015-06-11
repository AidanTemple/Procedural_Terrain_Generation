#pragma once
#include "OVR.h"
#include <d3d11.h>
#include <d3dx11tex.h>
#include "d3dclass.h"

class ApplicationClass;

struct ImageBuffer
{
    ID3D11Texture2D *            Tex;
    ID3D11ShaderResourceView *   TexSv;
    ID3D11RenderTargetView *     TexRtv;
    ID3D11DepthStencilView *     TexDsv;
    OVR::Sizei                   Size;

    ImageBuffer::ImageBuffer(bool rendertarget, ID3D11Device* device,ID3D11DeviceContext* context, bool depth, OVR::Sizei size, int mipLevels = 1,
                             unsigned char * data = NULL) : Size(size)
    {
        D3D11_TEXTURE2D_DESC dsDesc;
        dsDesc.Width     = size.w;
        dsDesc.Height    = size.h;
        dsDesc.MipLevels = mipLevels;
        dsDesc.ArraySize = 1;
        dsDesc.Format    = depth ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
        dsDesc.SampleDesc.Count = 1;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage     = D3D11_USAGE_DEFAULT;
        dsDesc.CPUAccessFlags = 0;
        dsDesc.MiscFlags      = 0;
        dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        if (rendertarget &&  depth) dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (rendertarget && !depth) dsDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        
		device->CreateTexture2D(&dsDesc, NULL, &Tex);
        device->CreateShaderResourceView(Tex, NULL, &TexSv);
        
        if (rendertarget &&  depth) device->CreateDepthStencilView(Tex, NULL, &TexDsv);
        if (rendertarget && !depth) device->CreateRenderTargetView(Tex, NULL, &TexRtv);
 
        if (data) // Note data is trashed, as is width and height
        {
            for (int level=0; level < mipLevels; level++)
            {
                context->UpdateSubresource(Tex, level, NULL, data, size.w * 4, size.h * 4);
                for(int j = 0; j < (size.h & ~1); j += 2)
                {
                    const uint8_t* psrc = data + (size.w * j * 4);
                    uint8_t*       pdest = data + ((size.w >> 1) * (j >> 1) * 4);
                    for(int i = 0; i < size.w >> 1; i++, psrc += 8, pdest += 4)
                    {
                        pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[size.w * 4 + 0] + psrc[size.w * 4 + 4]) >> 2;
                        pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[size.w * 4 + 1] + psrc[size.w * 4 + 5]) >> 2;
                        pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[size.w * 4 + 2] + psrc[size.w * 4 + 6]) >> 2;
                        pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[size.w * 4 + 3] + psrc[size.w * 4 + 7]) >> 2;
                    }
                }
                size.w >>= 1;  size.h >>= 1;
            }
        }
    }
};

class OculusManager
{
public:
	OculusManager();
	~OculusManager(void);

	bool Initialize(D3DClass* d3dManager);
	void Shutdown();

	void getEulerAngleRotation(float &yaw, float &pitch, float &roll);
	void render(D3DClass* d3dManager);

private:
	ovrHmd oculus;
	ImageBuffer    * eyeRenderTexture[2]; // Where the eye buffers will be rendered
	ImageBuffer    * eyeDepthBuffer[2];   // For the eye buffers to use when rendered
	ovrPosef         EyeRenderPose[2];     // Useful to remember where the rendered eye originated
	float            YawAtRender[2];       // Useful to remember where the rendered eye originated
	ovrRecti         eyeRenderViewport[2]; // Useful to remember when varying resolution
	ovrEyeRenderDesc EyeRenderDesc[2];     // Description of the VR.
	float            Yaw;       // Horizontal rotation of the player
	OVR::Vector3f         Pos; // Position of player

};

