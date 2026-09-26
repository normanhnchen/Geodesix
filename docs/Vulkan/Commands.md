# Commands

In Vulkan, GPU execution commands must be recorded through a **command buffer** before being sent to the GPU.

## Command Pools

Command pools manage the CPU memory used to store command buffers.

## Command Buffers

Command buffers are allocated through a command pool. The `vk::CommandBufferAllocateInfo` struct requires specifying whether the command buffer is a *primary command buffer* or *secondary command buffer*,

- **Primary command buffer**: These can be submitted directly to a [queue](./Queues.md).
- **Secondary command buffers**: These must be executed through a primary command buffer and cannot be submitted directly to a [queue](./Queues.md).

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html)  
