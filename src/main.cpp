#include "application.h"

#include <iostream>
#include <stdexcept>


int main() {
    Application app;
    
    try {
        app.Initialize();

        while (app.IsRunning()) {
            app.MainLoop();
        }

        app.Terminate();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}