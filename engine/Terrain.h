#ifndef TERRAIN_H
#define TERRAIN_H

const int TEXTURE_REPEAT = 32;

#include <d3d11.h>
#include <d3dx10math.h>
#include <stdio.h>
#include "Texture.h"

class Terrain
{
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	    D3DXVECTOR3 normal;
	};

	struct HeightMapType 
	{ 
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType 
	{ 
		float x, y, z;
	};

public:
	Terrain();
	Terrain(const Terrain&);
	~Terrain();

	bool Initialize(ID3D11Device*, char*, WCHAR*);
	
	// Initialise with Perlin noise
	bool InitTerrain(ID3D11Device*, int, int, WCHAR*, WCHAR*, WCHAR*);

	// Generate height map data using Perlin noise
	bool GenerateHeightMap(ID3D11Device*);

	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	int GetVertexCount();
	int GetIndexCount();

	void GetTerrainSize(int&, int&);

	void CopyVertexArray(void*);
	void Render(ID3D11DeviceContext*);

	ID3D11ShaderResourceView* GetGrassTexture();
	ID3D11ShaderResourceView* GetSlopeTexture();
	ID3D11ShaderResourceView* GetRockTexture();

private:
	bool LoadHeightMap(char*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	void CalculateTextureCoordinates();
	bool LoadTexture(ID3D11Device*, WCHAR*, WCHAR*, WCHAR*);
	void ReleaseTexture();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();

	//Renders the terrain
	void RenderBuffers(ID3D11DeviceContext*);
	
private:
	int m_terrainWidth, m_terrainHeight;
	HeightMapType* m_heightMap;
	Texture* m_Texture;
	int m_vertexCount, m_indexCount;
	VertexType* m_vertices;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;

	Texture *m_GrassTexture, *m_SlopeTexture, *m_RockTexture;
};

#endif