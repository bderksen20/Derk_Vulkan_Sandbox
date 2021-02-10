/* Vulkan Window Implementation */

#include "vke_window.hpp"
#include <stdexcept>

namespace vke {

	// Constructor Imp: member initializer list
	VkeWindow::VkeWindow(int w, int h, std::string name) : width{ w }, height{ h }, window_name{ name }{
		initWindow();
	}
	
	// Init window imp.
	void VkeWindow::initWindow() {

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// glfw base is in context of opengl: tell it not to do that
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		// disable window resizing after creation

		// Create window pointer
		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
	}

	void VkeWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {

		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}

	}

	// Destructor: destroy window + end glfw
	VkeWindow::~VkeWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}