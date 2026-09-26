# Window Surface

Vulkan is platform agnostic, meaning it was designed as an open, industry-wide standard without it being tied to any specific operating system or company. Therefore, to connect the Vulkan API to an operating system's window, we will use Window System Integration (WSI) extensions.

To include GLFW, we can use the GLFW's Vulkan compatibility extensions with the macro `GLFW_INCLUDE_MACRO` along with `glfwCreateWindowSurface` to actually create the Vulkan surface.

```
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/00_Window_surface.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/00_Window_surface.html)  
