# Dynamic States

In contrast to [fixed-functions](./Fixed-Functions.md), dynamic states allow runtime configuration of a limitated amount of states in pipeline objects compared to fixed-functions' immutable states.

## The Viewport

The viewport dyanmic state lets you control the position and size of the viewport. Therefore, upon window resizing, the entire graphics pipeline doesn't need to be recreated.

## Scissors

The Vulkan scissor dyanmic state lets you control its clipping boundary. Therefore, upon window resizing or in UI systems, the entire graphics pipeline doesn't need to be recreated.

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html)  
