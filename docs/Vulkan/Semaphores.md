# Semaphores

A Vulkan sempahore is an object used to synchronize GPU queue operations.

## Binary Semaphores

A binary semaphore is a type of Vulkan sempahore that has only two states: signaled and unsignaled. Starting off unsignaled, the binary semaphore orders executions through passing the semaphore as a "signal" semaphore in one queue and as a "wait" semaphore in another. Therefore, when the first queue finishes executing, the semaphore toggles to "signaled" and so the next queue can be executed.

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html)  
