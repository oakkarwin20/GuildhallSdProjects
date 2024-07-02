#pragma once

#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/CpuMesh.hpp"
#include "Engine/Core/GpuMesh.hpp"
#include "Engine/Renderer/Shader.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Material;
class Game;


//----------------------------------------------------------------------------------------------------------------------
class Model
{
public:
	Model( Game* game );
	~Model();

	void Render();
	void ParseXmlData( std::string xmlFileName );

public:
	CpuMesh* m_cpuMesh	= nullptr;
	GpuMesh* m_gpuMesh	= nullptr;
	Shader*  m_shader	= nullptr;

	// Materials
	Material*	m_material	  = nullptr;
	EulerAngles m_orientation = EulerAngles();
	Game*		m_game		  = nullptr; 
};