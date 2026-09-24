# Overview

[Vulkan](https://www.vulkan.org/) is an API developed by the [Khronos Group](https://www.khronos.org/) that includes cross-platform support and is heavily optimized for performance.

Boilerplate behind Vulkan is extremely verbose, requiring managing every detail related to memory, setup, hardware, and even explicitly controlling communication between the Vulkan API and the OS window library to draw things on screen.

## Setup

To include Vulkan C++ headers and its [RAII bindings](Resource_Acquisition_Is_Initialization.md), we use the following code.

```
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
```

VS Code’s IntelliSense engine flags `import vulkan_hpp` as an error so we default to using `#include <vulkan/vulkan_raii.hpp>`. When CMake `USE_CPP20_MODULES` is defined, we used `import vulkan_hpp` for a compiling speed up. Otherwise, we default to `#include <vulkan/vulkan_raii.hpp>` once again.

## References 

https://docs.vulkan.org/tutorial/latest/00_Introduction.html
https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
