#include "Game/Model.hpp"

#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

extern Renderer* g_theRenderer;

//----------------------------------------------------------------------------------------------------------------------
Model::Model()
{
}

//----------------------------------------------------------------------------------------------------------------------
Model::~Model()
{
	delete m_gpuMesh;	
	delete m_cpuMesh;	

	m_cpuMesh	= nullptr;
	m_gpuMesh	= nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void Model::Render( Mat44 const& modelMatrix, Rgba8 const& modelColor ) const
{
	Vec3 sunDir				= Vec3( 0.5, 0.5, -1.0 );
	float sunIntensity		= 0.9f;
	float ambientIntensity	= 0.1f;

	g_theRenderer->SetModelConstants( modelMatrix, modelColor );
	g_theRenderer->BindShader( m_shader );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetLightingConstants( sunDir, sunIntensity, ambientIntensity );
	m_gpuMesh->Render();

	// Resetting binded objects
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );
}


//----------------------------------------------------------------------------------------------------------------------
// Oak Version
//----------------------------------------------------------------------------------------------------------------------
void Model::ParseXmlData( std::string xmlFileName )
{
	XmlDocument xmlData;
	XmlResult result = xmlData.LoadFile(xmlFileName.c_str());
	if (result == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{
		return;
	}
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required xml file \" % s\"", xmlFileName.c_str()));

	XmlElement* rootElement = xmlData.RootElement();
	GUARANTEE_OR_DIE(rootElement, Stringf("Failed to fetch required root element"));

	std::string modelName = ParseXmlAttribute(*rootElement, "name", "INVALID");
	GUARANTEE_OR_DIE(modelName != "INVALID", Stringf("Invalid Model Name: %s, Provide a valid model name!!!", modelName.c_str()));

	std::string modelPath = ParseXmlAttribute(*rootElement, "path", "INVALID_PATH");
	GUARANTEE_OR_DIE(modelPath != "INVALID_PATH", Stringf("Invalid Model Path: %s, Provide a valid model path!!!", modelPath.c_str()));

	std::string modelShaderName = ParseXmlAttribute(*rootElement, "shader", "INVALID_SHADER");
	GUARANTEE_OR_DIE(modelShaderName != "INVALID_SHADER", Stringf("Invalid Shader: %s, Provide a valid shader!!!", modelPath.c_str()));
	m_shader = g_theRenderer->CreateOrGetShaderByName(modelShaderName.c_str(), VertexType::VERTEX_PCUTBN);

	if (m_cpuMesh)
	{
		delete m_cpuMesh;
		m_cpuMesh = nullptr;
	}
	if (m_gpuMesh)
	{
		delete m_gpuMesh;
		m_gpuMesh = nullptr;
	}

	XmlElement* transformElement = rootElement->FirstChildElement("Transform");
	GUARANTEE_OR_DIE(transformElement, Stringf("Failed to fetch required transform element"));

	Vec3  iBasis		= ParseXmlAttribute( *transformElement, "x", Vec3(1.f, 0.f, 0.f));
	Vec3  jBasis		= ParseXmlAttribute( *transformElement, "y", Vec3(0.f, 1.f, 0.f));
	Vec3  kBasis		= ParseXmlAttribute( *transformElement, "z", Vec3(0.f, 0.f, 1.f));
	Vec3  translation	= ParseXmlAttribute( *transformElement, "t", Vec3(0.f, 0.f, 0.f));
	float scale			= ParseXmlAttribute( *transformElement, "scale", 1.f);

	Mat44 transformFixUpMat;
	transformFixUpMat.SetIJKT3D(iBasis, jBasis, kBasis, translation);
	transformFixUpMat.AppendScaleUniform3D(scale);

	m_cpuMesh = new CpuMesh();
	m_gpuMesh = new GpuMesh();

	OBJLoader::LoadOBJByFileName( modelPath.c_str(), transformFixUpMat, m_cpuMesh->m_vertexes, m_cpuMesh->m_indexes );
//	OBJLoader::LoadObjFile( modelPath, m_cpuMesh->m_vertexes, m_cpuMesh->m_indexes, transformFixUpMat );
	m_gpuMesh->CopyCpuToGpu(m_cpuMesh);
}
