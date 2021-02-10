#include "app_ctrl.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {

	// Init an app instance (which has a window)
	vke::VkeApplication app{};

	// Try launch. If error thrown, spit to console.
	try {
		app.vke_app_run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
	
}
