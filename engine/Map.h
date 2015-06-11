#ifndef MAP_H
#define MAP_H

#include "Bitmap.h"
#include "TextureShader.h"

class Map
{
public:
	Map();
	Map(const Map&);
	~Map();

	bool Initialize(ID3D11Device*, HWND, int, int, D3DXMATRIX, float, float);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, TextureShader*);
	
	void PositionUpdate(float, float);

private:
	int m_mapLocationX, m_mapLocationY, m_pointLocationX, m_pointLocationY;
	float m_mapSizeX, m_mapSizeY, m_terrainWidth, m_terrainHeight;
	D3DXMATRIX m_viewMatrix;

	Bitmap *m_MiniMapBitmap, *m_Border, *m_Point;
};

#endif