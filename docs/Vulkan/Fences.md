# Fences

Vulkan fences are used to for GPU-CPU synchronization, allowing the host (CPU side) to check and wait for completion of GPU work.

Similar to [binary semaphores](./Semaphores.md#binary-semaphores), fences have two states: signaled and unsignaled. Unlike semaphores, the CPU can completely block and wait for a fence. Fences must be manually updated to change states.

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html)  
