#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/SkeletalSystem/IK_Joint3D.hpp"

#include <vector>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
class  IK_Joint3D;
class  IK_Chain3D;
class  Renderer;
class  Texture;
struct Shader;


//----------------------------------------------------------------------------------------------------------------------
enum MeshType
{
	MESH_TYPE_CYLINDER,
	MESH_TYPE_CYLINDER_TAPERED,
	MESH_TYPE_SPHERE,
	MESH_TYPE_ARROW,
	MESH_TYPE_NONE,
};


//----------------------------------------------------------------------------------------------------------------------
struct Raycast
{
	RaycastResult3D	m_raycastResult	= RaycastResult3D();
	Vec3			m_rayEndPos		= Vec3::ZERO;
};


//----------------------------------------------------------------------------------------------------------------------
class CreatureBase
{
public:
	CreatureBase( Vec3 const& rootStartPos = Vec3::ZERO, float length = 1.0f );
	~CreatureBase();

	void Update( float deltaSeconds );
	void RenderMesh( Renderer* g_theRenderer, Texture* texture = nullptr, Shader* const shader = nullptr, Rgba8 const& color = Rgba8::WHITE ) const;
	void Render( std::vector<Vertex_PCU>& verts, Rgba8 const& limbColor, Rgba8 const& jointColor, bool renderDebugJointBasis = false, bool const& renderDebugCurrentPos_EE = false ) const;

	// Initialization Functions
	IK_Chain3D* CreateChildChain	(	std::string		const&  name, 
										Vec3			const&  position_WS					= Vec3::ZERO, 
										IK_Joint3D*				ownerSkeletonFirstJoint		= nullptr, 
										bool					shouldReachInsteadOfDrag	= true 
									);
	void InitJointMesh( IK_Chain3D* const ikChain, float radius, float radiusDecrement, MeshType meshType = MESH_TYPE_CYLINDER );
	void InitJointMeshes_Ellipsoid( IK_Chain3D* const ikChain, Vec3 const& start, Vec3 const& end, float startRadius, float endRadius, Vec2 const& startRadiusScalarXY, Vec2 const& endRadiusScalarXY, Rgba8 const& color = Rgba8::WHITE );
	void InitMesh_2PerSegment( IK_Chain3D* const ikChain, float radius, float lengthPerSegment, float gapDistPerSegment );

	// Util Functions
	IK_Chain3D*	GetSkeletonByName			( std::string const& nameOfSkeletalSystem );
	bool		IsLimbTooFarFromRoot		( std::string const& nameOfSkeletalSystem, float maxDist = -1.0f );
	bool		IsLimbTooFarFromPos			( IK_Chain3D* const currentLimb, Vec3 const& refPosition, float maxDist );
	void		ToggleIsControlled();
	void		SetAllChainsCurTargetPosToTargetGoal();
	// Debug Render Functions
	void DebugDrawAnchorStates ( std::vector<Vertex_PCU>& verts, bool renderAtGoalOrientation, float radius ) const;

public:
	std::vector<IK_Chain3D*> m_skeletalList;
	IK_Joint3D*	m_modelRoot = nullptr;
	bool m_isControlled		= false;
	Vec3 m_avgFeetPos		= Vec3::ZERO; 
};