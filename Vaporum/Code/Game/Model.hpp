#pragma once

#include "Engine/Core/CpuMesh.hpp"
#include "Engine/Core/GpuMesh.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Math/Mat44.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Model
{
public:
	Model();
	~Model();

	void Render( Mat44 const& modelMatrix, Rgba8 const& modelColor ) const;
	void ParseXmlData( std::string xmlFileName );

public:
	CpuMesh* m_cpuMesh	= nullptr;
	GpuMesh* m_gpuMesh	= nullptr;
	Shader*  m_shader	= nullptr;
};