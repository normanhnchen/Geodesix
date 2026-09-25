#include "header_inclusions/vulkan.hpp"
#include "header_inclusions/glfw.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "application.hpp"


/**
 * Runs the application.
 * 
 * @see https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 */
int main() {
    try {
        Application app;
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
