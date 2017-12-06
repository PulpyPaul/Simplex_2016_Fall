#include "Octant.h"
using namespace Simplex;

uint Octant::m_uOctantCount;
uint Octant::m_uLeafCount;
uint Octant::m_uMaxLevel;
uint Octant::m_uIdealEntityCount;

// Constructor used ONLY for the root octant
Simplex::Octant::Octant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	// Gets singleton instances and creates an ID
	Init();

	// Make this object the root octant
	m_pRoot = this;

	// Assigns constructor parameters
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;

	// Gets the current count of entities
	m_uEntityCount = m_pEntityMngr->GetEntityCount();

	// Sets the Mix/Max to the global center for the root
	m_v3Max = m_pEntityMngr->GetRigidBody()->GetCenterGlobal();
	m_v3Min = m_pEntityMngr->GetRigidBody()->GetCenterGlobal();

	// Loops through all entity indexes, adds them to the Octant Entity list, sets the mix/max/size, then subdivides to create children
	for (uint i = 0; i < m_uEntityCount; ++i) {

		// Adds the index of the entity
		m_EntityList.push_back(i);

		// Gets the min/max of the given entity
		vector3 entity_min = m_pEntityMngr->GetRigidBody(i)->GetMinGlobal();
		vector3 entity_max = m_pEntityMngr->GetRigidBody(i)->GetMaxGlobal();

		// Sets the min/mix based on this octant

		// X values
		if (entity_min.x < m_v3Min.x) { m_v3Min.x = entity_min.x; }
		if (entity_max.x > m_v3Max.x) { m_v3Max.x = entity_max.x; }

		// Y Values
		if (entity_min.y < m_v3Min.y) { m_v3Min.y = entity_min.y; }
		if (entity_max.y > m_v3Max.y) { m_v3Max.y = entity_max.y; }

		// Z Values
		if (entity_min.z < m_v3Min.z) { m_v3Min.z = entity_min.z; }
		if (entity_max.z > m_v3Max.z) { m_v3Max.z = entity_max.z; }
	}

		// Gets center/size of the octant
		m_v3Center = (m_v3Min + m_v3Max) / 2.0f;
		m_v3Size = m_v3Max - m_v3Min;

		// Subdivide the octant into child octants
		Subdivide();

		// Add dimensions to the leafs
		CreateLeafDimensions();
}

// Constructor used to make the branches/leafs
Simplex::Octant::Octant(vector3 a_v3Center, vector3 a_v3Size)
{
	// Get references
	Init();

	// Assigns parameters
	m_v3Center = a_v3Center;
	m_v3Size = a_v3Size;

	// Gets mix/max
	m_v3Max = m_v3Center + m_v3Size / 2.0f;
	m_v3Min = m_v3Center - m_v3Size / 2.0f;
}

// Copy Constructor
Simplex::Octant::Octant(Octant const & other)
{
	// Gets references
	Init();

	// Assigns data from other object to this object
	m_pRoot = other.m_pRoot;
	m_pParent = other.m_pParent;
	m_uChildren = other.m_uChildren;
	m_uLevel = other.m_uLevel;
	m_v3Size = other.m_v3Size;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;
	m_uEntityCount = other.m_uEntityCount;

	// Creates the children octants
	for (uint i = 0; i < m_uChildren; ++i) {
		m_pChild[i] = new Octant(*other.m_pChild[i]);
	}

	// Copies over the entity list
	for (uint i = 0; i < m_uEntityCount; ++i) {
		m_EntityList.push_back(other.m_EntityList[i]);
	}

	// Copies over the child list if this Octant is the root
	if (this == m_pRoot) {
		for (uint i = 0; i < other.m_lChild.size(); ++i) {
			m_lChild.push_back(other.m_lChild[i]);
		}
	}
}

// Copy Assignment operator
Octant & Simplex::Octant::operator=(Octant const & other)
{
	// Returns this object if called on itself
	if (&other == this) {
		return *this;
	}

	Release();
	Init();

	// Assigns data from other object to this object
	m_pRoot = other.m_pRoot;
	m_pParent = other.m_pParent;
	m_uChildren = other.m_uChildren;
	m_uLevel = other.m_uLevel;
	m_v3Size = other.m_v3Size;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;
	m_uEntityCount = other.m_uEntityCount;

	// Creates the children octants
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i] = new Octant(*other.m_pChild[i]);
	}

	// Copies over the entity list
	for (uint i = 0; i < m_uEntityCount; i++) {
		m_EntityList.push_back(other.m_EntityList[i]);
	}

	// Copies over the child list if this Octant is the root
	if (this == m_pRoot) {
		for (uint i = 0; i < other.m_lChild.size(); ++i) {
			m_lChild.push_back(other.m_lChild[i]);
		}
	}

	// Returns this octant
	return *this;
}

// Destructor
Simplex::Octant::~Octant(void)
{
	Release();
}

void Simplex::Octant::Swap(Octant & other)
{
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pChild, other.m_pChild);
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);
	std::swap(m_v3Size, other.m_v3Size);
	std::swap(m_EntityList, other.m_EntityList);
	std::swap(m_uEntityCount, other.m_uEntityCount);
}

vector3 Simplex::Octant::GetSize(void)
{
	return m_v3Size;
}

vector3 Simplex::Octant::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 Simplex::Octant::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 Simplex::Octant::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool Simplex::Octant::IsColliding(uint a_uRBIndex)
{
	// Gets the Rigid body of the given index
	MyRigidBody* rigidBody = m_pEntityMngr->GetRigidBody(a_uRBIndex);

	// Gets the mix/max of the rigid body
	vector3 body_min = rigidBody->GetMinGlobal();
	vector3 body_max = rigidBody->GetMaxGlobal();

	// AABB Test
	if (m_v3Max.x < body_min.x) //this to the right of other
		return false;
	if (m_v3Min.x > body_max.x) //this to the left of other
		return false;
	if (m_v3Max.y < body_min.y) //this below of other
		return false;
	if (m_v3Min.y > body_max.y) //this above of other
		return false;
	if (m_v3Max.z < body_min.z) //this behind of other
		return false;
	if (m_v3Min.z > body_max.z) //this in front of other
		return false;

	// If no test fails, must be colliding
	return true;
}

void Simplex::Octant::Display(uint a_nIndex, vector3 a_v3Color)
{
	// Displays all octants if the index is equal to or greater than the count
	if (a_nIndex >= m_uOctantCount) {
		DisplayLeafs(a_v3Color);
	}
	else {
		m_lChild[a_nIndex]->Display(a_v3Color);
	}
}

// Displays a single octant given a color
void Simplex::Octant::Display(vector3 a_v3Color)
{
	// Creates a wire cube based on the center location and scale
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(IDENTITY_M4, m_v3Size), a_v3Color);
}

// Display a leaf if the current octant is a leaf, recursively displays all other leafs
void Simplex::Octant::DisplayLeafs(vector3 a_v3Color)
{
	if (IsLeaf()) {
		Display(a_v3Color);
	}
	else {
		for (uint i = 0; i < m_uChildren; i++) {
			m_pChild[i]->DisplayLeafs(a_v3Color);
		}
	}
}

// Loops through all the entity lists and clears them
void Simplex::Octant::ClearEntityList(void)
{
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ClearEntityList();
	}

	m_EntityList.clear();
}

// Subdivides the Octant
void Simplex::Octant::Subdivide(void)
{
	// Return if the maximum number is it hit or if the ideal entity count reached
	if (ContainsMoreThan(m_uIdealEntityCount) == false || m_uLevel >= m_uMaxLevel) {
		m_pRoot->m_lChild.push_back(this);
		m_uLeafCount++;
		return;
	}

	// Means octant was already subdivided
	if (m_uChildren == 8) {
		return;
	}

	// Creates the 8 Octants as children of the current Octant
	ConstructList();

	// Initializes all the children in the list
	for (uint i = 0; i < m_uChildren; ++i) {
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel++;
		m_pChild[i]->m_pRoot = m_pRoot;

		// If the child octant is colliding with an entity, add that entity index to the childs list of entities
		for (uint j = 0; j < m_uEntityCount; ++j) {
			if (m_pChild[i]->IsColliding(m_EntityList[j])) {
				m_pChild[i]->m_EntityList.push_back(m_EntityList[j]);
			}
		}

		// Gets the propery entity count based on entity list size
		m_pChild[i]->m_uEntityCount = m_pChild[i]->m_EntityList.size();

		// Calls this function recursively
		m_pChild[i]->Subdivide();
	}
}

Octant * Simplex::Octant::GetChild(uint a_nChild)
{
	// If there a no children, return a null pointer
	if (m_uChildren == 0) {
		return nullptr;
	}
	else {
		return m_pChild[a_nChild];
	}
}

Octant * Simplex::Octant::GetParent(void)
{
	return m_pParent;
}

bool Simplex::Octant::IsLeaf(void)
{
	// If the octant is a leaf, it has no children
	return m_uChildren == 0;
}

bool Simplex::Octant::ContainsMoreThan(uint a_nEntities)
{
	if (m_uEntityCount > a_nEntities) {
		return true;
	}
	else {
		return false;
	}
}

// Deletes everything but root Octant
void Simplex::Octant::KillBranches(void)
{
	// If this octant is a leaf return
	if (IsLeaf()) {
		return;
	}
	// Otherwise continue to delete the branches recursively
	else {
		for (uint i = 0; i < m_uChildren; ++i) {
			m_pChild[i]->KillBranches();
			SafeDelete(m_pChild[i]);
		}
	}
}

void Simplex::Octant::ConstructTree(uint a_nMaxLevel)
{
}

void Simplex::Octant::AssignIDtoEntity(void)
{
}

uint Simplex::Octant::GetLeafCount(void)
{
	return m_uLeafCount;
}

uint Simplex::Octant::GetOctantCount(void)
{
	return m_uOctantCount;
}

// Create the Entity Dimensions for each leaf
void Simplex::Octant::CreateLeafDimensions(void)
{
	// If this Octant is a leaf, add the entity dimension, otherwise call the function recursively
	if (IsLeaf()) {
		for (uint i = 0; i < m_uEntityCount; ++i) {
			m_pEntityMngr->AddDimension(m_EntityList[i], m_uID);
		}
	}
	else {
		for (uint i = 0; i < m_uChildren; ++i) {
			m_pChild[i]->CreateLeafDimensions();
		}
	}
}

void Simplex::Octant::Release(void)
{
	// If this octant is the root, kill the branches
	if (m_pRoot == this) {
		KillBranches();
	}
}

// Assigns the singletons and gets an ID based on octant count
void Simplex::Octant::Init(void)
{
	// Gets instances of the singletons
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();

	// Assigns the ID of the octant based on the count
	m_uID = m_uOctantCount;
	m_uOctantCount++;
}

// Creates the octants in the child list
void Simplex::Octant::ConstructList(void)
{
	// Right Bottom back
	m_pChild[0] = new Octant(m_v3Center + vector3(m_v3Size.x / 4, -m_v3Size.y / 4, -m_v3Size.z / 4), m_v3Size / 2.f);

	// Right bottom front
	m_pChild[1] = new Octant(m_v3Center + vector3(m_v3Size.x / 4, -m_v3Size.y / 4, m_v3Size.z / 4), m_v3Size / 2.f);

	// Right top back
	m_pChild[2] = new Octant(m_v3Center + vector3(m_v3Size.x / 4, m_v3Size.y / 4, -m_v3Size.z / 4), m_v3Size / 2.f);

	// Right top front
	m_pChild[3] = new Octant(m_v3Center + vector3(m_v3Size.x / 4, m_v3Size.y / 4, m_v3Size.z / 4), m_v3Size / 2.f);

	// Left Top back
	m_pChild[4] = new Octant(m_v3Center + vector3(-m_v3Size.x / 4, m_v3Size.y / 4, -m_v3Size.z / 4), m_v3Size / 2.f);

	// Left Top front
	m_pChild[5] = new Octant(m_v3Center + vector3(-m_v3Size.x / 4, m_v3Size.y / 4, m_v3Size.z / 4), m_v3Size / 2.f);

	// Left Bottom back
	m_pChild[6] = new Octant(m_v3Center + vector3(-m_v3Size.x / 4, -m_v3Size.y / 4, -m_v3Size.z / 4), m_v3Size / 2.f);

	// Left Bottom front
	m_pChild[7] = new Octant(m_v3Center + vector3(-m_v3Size.x / 4, -m_v3Size.y / 4, m_v3Size.z / 4), m_v3Size / 2.f);

	// Number of children
	m_uChildren = 8;
}
