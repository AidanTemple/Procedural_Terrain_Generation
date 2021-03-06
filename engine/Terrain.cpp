#include "Terrain.h"
#include "PerlinNoise.h"

Terrain::Terrain()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_heightMap = 0;
	m_Texture = 0;
	m_vertices = 0;

	m_terrainWidth = 0;
	m_terrainHeight = 0;

	m_GrassTexture = 0;
	m_SlopeTexture = 0;
	m_RockTexture = 0;
}

Terrain::Terrain(const Terrain& other)
{

}

Terrain::~Terrain()
{

}

bool Terrain::InitTerrain(ID3D11Device* device, int terrainWidth, int terrainHeight, WCHAR* grassTexture, 
	WCHAR* slopeTexture, WCHAR* rockTexture)
{
	int index;
	float height = 0.0f;

	bool result;

	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	// Create new height map which will hold terrain data
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];

	if(!m_heightMap)
	{
		return false;
	}

	// Initialise terrain data
	for(int z = 0; z < m_terrainHeight; z++)
	{
		for(int x = 0; x < m_terrainWidth; x++)
		{
			index = (m_terrainHeight * z) + x;

			m_heightMap[index].x = (float)x;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)z;
		}
	}

	// Calculate terrain normals
	result = CalculateNormals();

	if(!result)
	{
		return false;
	}

	// Calculate the texture coordinates.
	CalculateTextureCoordinates();

	// Load terraint texture(s)
	result = LoadTexture(device, grassTexture, slopeTexture, rockTexture);

	if(!result)
	{
		return false;
	}

	// Initialize vertex and index geometry buffers
	result = InitializeBuffers(device);

	if(!result)
	{
		return false;
	}

	return true;
}

void Terrain::Shutdown()
{
	// Release the texture.
	ReleaseTexture();

	// Release the vertex array.
	ShutdownBuffers();

	// Release the height map data.
	ShutdownHeightMap();

	return;
}

void Terrain::Render(ID3D11DeviceContext* device)
{
	RenderBuffers(device);

	return;
}

int Terrain::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* Terrain::GetTexture()
{
	return m_Texture->GetTexture();
}

ID3D11ShaderResourceView* Terrain::GetGrassTexture()
{
	return m_GrassTexture->GetTexture();
}

ID3D11ShaderResourceView* Terrain::GetSlopeTexture()
{
	return m_SlopeTexture->GetTexture();
}

ID3D11ShaderResourceView* Terrain::GetRockTexture()
{
	return m_RockTexture->GetTexture();
}

bool Terrain::GenerateHeightMap(ID3D11Device* device)
{
	bool result;

	int index;
	int k = 1;

	float height = 0.0;
	float upperLimit = 15.0f;
	float pass[3];

	PerlinNoise pNoise;

	for(int z = 0; z < m_terrainHeight; z++)
	{
		for(int x = 0; x < m_terrainWidth; x++)
		{
			index = (m_terrainHeight * z) + x;

			pass[0] = (float)x / 30;
			pass[1] = (float)0.25f;
			pass[2] = (float)z / 30;

			m_heightMap[index].x = (float)x;
			m_heightMap[index].y = (float)pNoise.noise3(pass) * 10.0f;
			m_heightMap[index].z = (float)z;
		}
	}

	result = CalculateNormals();

	if(!result)
	{
		return false;
	}

	result = InitializeBuffers(device);

	if(!result)
	{
		return false;
	}

	return true;
}

void Terrain::NormalizeHeightMap()
{
	int i, j;

	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			m_heightMap[(m_terrainHeight * j) + i].y /= 15.0f;
		}
	}

	return;
}

bool Terrain::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	VectorType* normals;

	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new VectorType[(m_terrainHeight-1) * (m_terrainWidth-1)];
	if(!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for(j=0; j<(m_terrainHeight-1); j++)
	{
		for(i=0; i<(m_terrainWidth-1); i++)
		{
			index1 = (j * m_terrainHeight) + i;
			index2 = (j * m_terrainHeight) + (i+1);
			index3 = ((j+1) * m_terrainHeight) + i;

			// Get three vertices from the face.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;
		
			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;
		
			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainHeight-1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if(((i-1) >= 0) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if((i < (m_terrainWidth-1)) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if(((i-1) >= 0) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if((i < (m_terrainWidth-1)) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}
			
			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));
			
			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete [] normals;
	normals = 0;

	return true;
}

void Terrain::ShutdownHeightMap()
{
	if(m_heightMap)
	{
		delete [] m_heightMap;
		m_heightMap = 0;
	}

	return;
}

void Terrain::CalculateTextureCoordinates()
{
	int incrementCount, i, j, tuCount, tvCount;
	float incrementValue, tuCoordinate, tvCoordinate;


	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)m_terrainWidth;

	// Calculate how many times to repeat the texture.
	incrementCount = m_terrainWidth / TEXTURE_REPEAT;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			// Store the texture coordinate in the height map.
			m_heightMap[(m_terrainHeight * j) + i].tu = tuCoordinate;
			m_heightMap[(m_terrainHeight * j) + i].tv = tvCoordinate;

			// Increment the tu texture coordinate by the increment value and increment the index by one.
			tuCoordinate += incrementValue;
			tuCount++;

			// Check if at the far right end of the texture and if so then start at the beginning again.
			if(tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		// Increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		// Check if at the top of the texture and if so then start at the bottom again.
		if(tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}

	return;
}

bool Terrain::LoadTexture(ID3D11Device* device, WCHAR* grassTexture, 
	WCHAR* slopeTexture, WCHAR* rockTexture)
{
	bool result;

	// Create the grass texture object.
	m_GrassTexture = new Texture;
	if(!m_GrassTexture)
	{
		return false;
	}

	// Initialize the grass texture object.
	result = m_GrassTexture->Initialize(device, grassTexture);
	if(!result)
	{
		return false;
	}

	// Create the slope texture object.
	m_SlopeTexture = new Texture;
	if(!m_SlopeTexture)
	{
		return false;
	}

	// Initialize the slope texture object.
	result = m_SlopeTexture->Initialize(device, slopeTexture);
	if(!result)
	{
		return false;
	}

	// Create the rock texture object.
	m_RockTexture = new Texture;
	if(!m_RockTexture)
	{
		return false;
	}

	// Initialize the rock texture object.
	result = m_RockTexture->Initialize(device, rockTexture);
	if(!result)
	{
		return false;
	}

	return true;
}

void Terrain::ReleaseTexture()
{
	// Release the texture objects.
	if(m_GrassTexture)
	{
		m_GrassTexture->Shutdown();
		delete m_GrassTexture;
		m_GrassTexture = 0;
	}

	if(m_SlopeTexture)
	{
		m_SlopeTexture->Shutdown();
		delete m_SlopeTexture;
		m_SlopeTexture = 0;
	}

	if(m_RockTexture)
	{
		m_RockTexture->Shutdown();
		delete m_RockTexture;
		m_RockTexture = 0;
	}

	return;
}

//Initialises the terrains buffers, sets up the vertices and indices, position, texture coordinates and normals
bool Terrain::InitializeBuffers(ID3D11Device* device)
{
	//VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index1, index2, index3, index4;
	float tu, tv;

	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	m_vertices = new VertexType[m_vertexCount];
	if(!m_vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;

	// Load the vertex and index array with the terrain data.
	for(j=0; j<(m_terrainHeight-1); j++)
	{
		for(i=0; i<(m_terrainWidth-1); i++)
		{
			index1 = (m_terrainHeight * j) + i;          // Bottom left.
			index2 = (m_terrainHeight * j) + (i+1);      // Bottom right.
			index3 = (m_terrainHeight * (j+1)) + i;      // Upper left.
			index4 = (m_terrainHeight * (j+1)) + (i+1);  // Upper right.

			// Upper left.
			tv = m_heightMap[index3].tv;

			// Modify the texture coordinates to cover the top edge.
			if(tv == 1.0f) { tv = 0.0f; }

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			m_vertices[index].texture = D3DXVECTOR2(m_heightMap[index3].tu, tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
			indices[index] = index;
			index++;

			// Upper right.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// Modify the texture coordinates to cover the top and right edge.
			if(tu == 0.0f) { tu = 1.0f; }
			if(tv == 1.0f) { tv = 0.0f; }

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			m_vertices[index].texture = D3DXVECTOR2(tu, tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// Bottom left.
			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			m_vertices[index].texture = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// Bottom left.
			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			m_vertices[index].texture = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// Upper right.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// Modify the texture coordinates to cover the top and right edge.
			if(tu == 0.0f) { tu = 1.0f; }
			if(tv == 1.0f) { tv = 0.0f; }

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			m_vertices[index].texture = D3DXVECTOR2(tu, tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// Bottom right.
			tu = m_heightMap[index2].tu;

			// Modify the texture coordinates to cover the right edge.
			if(tu == 0.0f) { tu = 1.0f; }

			m_vertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			m_vertices[index].texture = D3DXVECTOR2(tu, m_heightMap[index2].tv);
			m_vertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
			indices[index] = index;
			index++;
		}
	}

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	//// Release the arrays now that the buffers have been created and loaded.
	//delete [] m_vertices;
	//m_vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

//Renders the terrain
void Terrain::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

//Cleans up the buffers
void Terrain::ShutdownBuffers()
{
	if(m_vertices)
	{
		delete [] m_vertices;
		m_vertices = 0;
	}

	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

int Terrain::GetVertexCount()
{
	return m_vertexCount;
}

void Terrain::CopyVertexArray(void* vertexList)
{
	memcpy(vertexList, m_vertices, sizeof(VertexType) * m_vertexCount);
	return;
}

void Terrain::GetTerrainSize(int& width, int& height)
{
	// Return the width and height of the terrain.
	width = m_terrainWidth;
	height = m_terrainHeight;

	return;
}