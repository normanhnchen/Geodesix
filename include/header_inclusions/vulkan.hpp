// Used to enable C++20 designated initializers
// (e.g. .member = value)
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
// To prevent vk::Result::eErrorOutOfDateKHR (that triggers an exception) when the swap chain
// becomes incompatible with the window surface, we define this macro to prevent the runtime error
// because it is not a fatal program crash but rather an expected runtime state
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

// Both Vulkan header includes import the exact same things
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
/* Default header inclusion (fallback) */
// VS Code's IntelliSense engine or `USE_CPP20_MODULES` isn't defined by CMake, the `import`
// statement is not recognized as valid code and will crash
#include <vulkan/vulkan_raii.hpp>
#else
/* Faster compiling */
// If CMake's `USE_CPP20_MODULES` macro is defined, we use the supported `import` statement
import vulkan_hpp;
#endif
