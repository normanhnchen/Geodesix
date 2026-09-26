# Shaders

## The Vertex Shader

The vertex shader inputs raw vertex data including its position, color, and texture coordinates and transforms each vertex. The output is the final [clip position](#clip-position) and any needed per-vertex data. The final positions, after the clip positions undergo [perspective division](#perspective-division) and the [viewport transform](#the-viewport-transform), are then sent to the rasterizer to convert the primitives the vertices form into pixel fragments.

### Clip Position

The **clip position**, or **clip coordinate** is the final 4D transformed vertex position calculated in the vertex shader. These clip positions are found after the GPU *clips* all vertices that sit outside the camera's view.

### Perspective Division

*Normalized Device Coordinates (NDC)* are 3D coordinates found through the **perspective division** step, where clip positions are divided by its 4th dimension (divided by $w$, where the vector is defined as $(x, y, z, w)$). In turn, the framebuffer is mapped to $[-1, -1]$ by $[1, 1]$.

### The Viewport Transform

In the **viewport transform**, NDC coordinates are converted to screen coordinates that exactly match the framebuffer resolution. The final 2D screen coordinates are then returned along with the corresponding per-vertex depth data to be sent to the rasterizer.

## The Fragment Shader

The fragment shader runs on every fragment returned by the rasterizer. It computes the final color and visual properties of an individual pixel before displaying on screen.

## Shader Compiling

Vulkan expects all shaders to be in SPIR-V bytecode. SPIR-V is a binary intermediate representation (a universal, pre-compiled bytecode format) that provides universal optimization and cross-platform compatability, especially with extensions. Because SPIR-V is a pre-compiled format, it can be compiled from high-level shader languages such as GLSl or HLSL.

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/01_Shader_modules.html)  
[https://docs.vulkan.org/guide/latest/what_is_spirv.html](https://docs.vulkan.org/guide/latest/what_is_spirv.html)  
[https://learnopengl.com/Getting-started/Coordinate-Systems](https://learnopengl.com/Getting-started/Coordinate-Systems)
