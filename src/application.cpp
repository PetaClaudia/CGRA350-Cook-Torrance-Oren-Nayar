
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;

int latDivisions = 15;
int lonDivisions = 15;

 mesh_builder Application::sphere_latlong() {
     vector<vec3> positionsVec;
     vector<vec3> normalsVec;
     vector<vec2> uvsVec;
     vector<unsigned int> indices;
     float radius = 1.0f;
     auto pi = glm::pi<float>();
     mesh_builder mb;
     float x, y, z, xy;                              // vertex position
     float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
     float s, t;                                     // vertex texCoord
     int k1, k2;
     float latStep = (2 * pi) / latDivisions;
     float lonStep = pi / lonDivisions;
     float azimuthAngle, elevationAngle;
     
     //iterate through all longitude divisions
     for (int i = 0; i <= lonDivisions; ++i){
         elevationAngle = pi / 2 - i * lonStep;
         xy = radius * cos(elevationAngle);
         z = radius * sin(elevationAngle);
         //iterate through all latitude divisions
         for (int j = 0; j <= latDivisions; ++j){
            azimuthAngle = j * latStep;           // starting from 0 to 2pi
             
             // vertex position (x, y, z)
             x = xy * cosf(azimuthAngle);             // r * cos(u) * cos(v)
             y = xy * sinf(azimuthAngle);             // r * cos(u) * sin(v)
             
             positionsVec.push_back(vec3(x, y, z));
             
             // normalized vertex normal (nx, ny, nz)
             nx = x * lengthInv;
             ny = y * lengthInv;
             nz = z * lengthInv;
             
             normalsVec.push_back(vec3(nx, ny, nz));
             
             // vertex tex coord (s, t) range between [0, 1]
             s = (float)j / latDivisions;
             t = (float)i / lonDivisions;
             
             uvsVec.push_back(vec2(s, t));
         }
     }
     for (int i = 0; i < lonDivisions; ++i){
         k1 = i * (latDivisions + 1);
         k2 = k1 + latDivisions + 1;    
         
         for (int j = 0; j < latDivisions; ++j, ++k1, ++k2)
         {
             //top left triangle
             if (i != 0)    {
                 indices.push_back(k1);
                 indices.push_back(k2);
                 indices.push_back(k1 + 1);
             }
             //bottom right triangle
             if (i != (lonDivisions - 1)){
                 indices.push_back(k1 + 1);
                 indices.push_back(k2);
                 indices.push_back(k2 + 1);
             }
         }
     }
     for (unsigned int k = 0; k < indices.size(); ++k) {
             mb.push_index(k);
             mb.push_vertex(mesh_vertex{
                 positionsVec[indices[k]],
                 normalsVec[indices[k]],
                 uvsVec[indices[k]]
             });
         }
         return mb;
 }


void basic_model::draw(const glm::mat4 &view, const glm::mat4 proj) {
	mat4 modelview = view * modelTransform;
	
	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	mesh.draw(); // draw
}


Application::Application(GLFWwindow *window) : m_window(window) {
	
	shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
    m_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_model.color = vec3(1, 0, 0);
}


void Application::render() {
	
	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height); 

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw,   vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);


	// draw the model
	m_model.draw(view, proj);
}


void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);
    if (ImGui::Button("Sphere"))
        m_model.mesh = sphere_latlong().build();
        m_model.color = vec3(1, 0, 0);
    if(ImGui::SliderInt("Latitude Divisions", &latDivisions, 5, 100)){
        m_model.mesh = sphere_latlong().build();
        m_model.color = vec3(1, 0, 0);
    }
    if(ImGui::SliderInt("Longitude Divisions", &lonDivisions, 5, 100)){
        m_model.mesh = sphere_latlong().build();
        m_model.color = vec3(1, 0, 0);
    }
    if (ImGui::Button("Cook-Torrance")){
        shader_builder sb;
        sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
        sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//cook_torrance.glsl"));
        GLuint shader = sb.build();
        m_model.shader = shader;
    }
    if (ImGui::Button("Oren-Nayar")){
           shader_builder sb;
           sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
           sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//oren_nayar.glsl"));
           GLuint shader = sb.build();
           m_model.shader = shader;
       }

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	
	ImGui::Separator();

	// example of how to use input boxes
	static float exampleInput;
	if (ImGui::InputFloat("example input", &exampleInput)) {
		cout << "example input changed to " << exampleInput << endl;
	}

	// finish creating window
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
