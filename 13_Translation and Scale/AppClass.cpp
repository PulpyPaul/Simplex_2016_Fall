#include "AppClass.h"
#include <vector>

using namespace std;

void Application::InitVariables(void)
{
	////Change this to your name and email
	m_sProgrammer = "Paul DeSimone - pjd3949@rit.edu";

	////Alberto needed this at this position for software recording.
	//m_pWindow->setPosition(sf::Vector2i(710, 0));
	
	// Generates cubes
	for (int i = 0; i < 46; i++) {
		meshVectors.push_back(new MyMesh());
		meshVectors[i]->GenerateCube(1.0f, C_BLACK);
	}
}

void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	// Matrices needed for rendering
	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	static float value = 0.0f;
	matrix4 m4Translate = glm::translate(IDENTITY_M4, vector3(value, 2.0f, 3.0f));
	
	// Amount of change per frame
	value += 0.0001f;

	//matrix4 m4Model = m4Translate * m4Scale;
	matrix4 m4Model = m4Scale * m4Translate;

	// Render middle row of squares
	for (int i = 0; i < 46; i++) {
		translateVectors[i] = vector3(translateVectors[i].x + value, translateVectors[i].y, translateVectors[i].z);
		m4Translate = glm::translate(IDENTITY_M4, translateVectors[i]);
		m4Model = m4Scale * m4Translate;
		meshVectors[i]->Render(m4Projection, m4View, m4Model);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//SafeDelete(m_pMesh);

	for (int i = 0; i < meshVectors.size(); i++) {
		SafeDelete(meshVectors[i]);
	}

	//release GUI
	ShutdownGUI();
}