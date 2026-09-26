# Frames In Flight

To maximize performance in Vulkan, frames in flight are used. This method allows the CPU and GPU to be busy while waiting for the previous frame. Therefore, no device is idle while waiting for another process to finish executing. They can concurrently be working for a specified number of frames in flight.

Because the CPU and GPU are simultaneously working on different frames, resources cannot be shared across frames. Therefore, for the specified number of frames in flight, resources that must be accessed and modfied must be duplicated.

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/03_Frames_in_flight.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/03_Frames_in_flight.html)  
