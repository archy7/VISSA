#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>>

#include "generalGL.h"
#include "Shader.h"
#include "Camera.h"
#include "Window.h"
#include "GeometricPrimitiveData.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


Renderer::Renderer() :
	m_fNearPlane(0.1f),
	m_fFarPlane(1000.0f)
{
	
}

Renderer::~Renderer()
{
	FreeGPUResources();
}

void Renderer::InitRenderer()
{
	LoadShaders();
	glAssert();
	LoadTextures();
	glAssert();
	LoadPrimitivesToGPU();
	glAssert();
	SetInitialOpenGLState();
	glAssert();
}

void Renderer::LoadShaders()
{
	// A simple color shader
	Shader tColorShader("resources/shaders/Color.vs", "resources/shaders/Color.frag");
	m_tColorShader = tColorShader;
	assert(m_tColorShader.IsInitialized());

	// A texture shader
	Shader tTextureShader("resources/shaders/FlatTexture.vs", "resources/shaders/FlatTexture.frag");
	m_tTextureShader = tTextureShader;
	assert(m_tTextureShader.IsInitialized());
}

void Renderer::LoadTextures()
{
	// texture 1
	// ---------
	GLuint& rTextureUnit1 = m_uiTexture1;
	glGenTextures(1, &rTextureUnit1);
	glBindTexture(GL_TEXTURE_2D, rTextureUnit1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load("resources/textures/container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


	// texture 2
	// ---------
	GLuint& rTextureUnit2 = m_uiTexture2;
	glGenTextures(1, &rTextureUnit2);
	glBindTexture(GL_TEXTURE_2D, rTextureUnit2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load("resources/textures/awesomeface.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void Renderer::LoadPrimitivesToGPU()
{
	GLuint &rTexturedCubeVBO = m_uiTexturedCubeVBO, &rTexturedCubeVAO = m_uiTexturedCubeVAO, &rTexturedCubeEBO = m_uiTexturedCubeEBO;
	glGenVertexArrays(1, &rTexturedCubeVAO);
	glGenBuffers(1, &rTexturedCubeVBO);
	glGenBuffers(1, &rTexturedCubeEBO);

	glBindVertexArray(rTexturedCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, rTexturedCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Cube::TexturedVertexData), Primitives::Cube::TexturedVertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedCubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Cube::TexturedIndexData), Primitives::Cube::TexturedIndexData, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Renderer::SetInitialOpenGLState()
{
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
}

void Renderer::FreeGPUResources()
{
	// Buffers
	glDeleteVertexArrays(1, &m_uiTexturedCubeVAO);
	glDeleteBuffers(1, &m_uiTexturedCubeVBO);
	glDeleteBuffers(1, &m_uiTexturedCubeEBO);

	// Textures
	glDeleteTextures(1, &m_uiTexture1);
	glDeleteTextures(2, &m_uiTexture2);
}

void Renderer::RenderFrame(const Camera & rCamera, Window & rWindow)
{
	glClearColor(rWindow.m_vec4fClearColor[0], rWindow.m_vec4fClearColor[1], rWindow.m_vec4fClearColor[2], rWindow.m_vec4fClearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Render3DScene(rCamera, rWindow);
	RenderGUI(rWindow);
}

void Renderer::Render3DScene(const Camera& rCamera, const Window& rWindow)
{
	glAssert();

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	m_tTextureShader.use();
	glAssert();
	m_tTextureShader.setInt("texture1", 0);
	m_tTextureShader.setInt("texture2", 1);

	glAssert();

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture2);

	glAssert();

	// calculate the model matrix for each object and pass it to shader before drawing
	glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	world = glm::translate(world, glm::vec3(0.0f, 0.0f, 0.0f));
	m_tTextureShader.setMat4("world", world);

	// camera/view transformation
	glm::mat4 view = rCamera.GetViewMatrix();
	m_tTextureShader.setMat4("view", view);

	// pass projection matrix to shader (note that in this case it could change every frame)
	glm::mat4 projection = glm::perspective(glm::radians(rCamera.Zoom), (float)rWindow.m_iWindowWidth / (float)rWindow.m_iWindowHeight, m_fNearPlane, m_fFarPlane);
	m_tTextureShader.setMat4("projection", projection);

	glAssert();	

	// render boxes
	glBindVertexArray(m_uiTexturedCubeVAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Cube::TexturedIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

	glAssert();
}

void Renderer::RenderGUI(Window& rWindow)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Capture Mouse", &rWindow.m_bMouseCaptured);				// Edit bools storing our window open/close state

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", rWindow.m_vec4fClearColor); // Edit 3 floats representing a color

		if (ImGui::Button("Toggle Mouse"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			rWindow.SetMouseCaptureMode(!rWindow.m_bMouseCaptured);
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
