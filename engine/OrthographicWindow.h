#ifndef ORTHOGRAPHICWINDOW_H
#define ORTHOGRAPHICWINDOW_H

#include <d3d11.h>
#include <d3dx10math.h>

class OrthographicWindow
{
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	OrthographicWindow();
	OrthographicWindow(const OrthographicWindow&);
	~OrthographicWindow();

	bool Initialize(ID3D11Device*, int, int);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*, int, int);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
};

#endif