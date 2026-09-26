# The Swap Chain

Because Vulkan provides no default framebuffer that automatically refreshes, we must explicitly manage, clear, and synchronize our own framebuffer to present something on screen. As such, the Vulkan **swap chain** is used. The Vulkan swap chain is an explicit queue of renderable framebuffers that will be presented to screen while synchronized to the refresh rate of the screen. In other words, it swaps rendered images in its queue to the window surface.

## Presentation Modes

The presentation mode a surface uses describes how images should be swapped to the presentable screen.

### `vk::PresentModeKHR::eImmediate`

In this mode, swap chain images are displayed immediately the moment when the GPU finishes a new frame. Because the GPU doesn't necessarily wait for a frame to completely finish, it may result in screen tearing. This mode results in the lowest latency out of all presentation modes.

### `vk::PresentModeKHR::eFifo`

In this mode (where FIFO means First In, First Out), the swap chain is a queue of images where the screen displays images refreshed from the FIFO queue. When the queue is full, it blocks the application. This mode is also known as *vertical sync*, or *V-Sync*, where waiting for the frame to completely finish eliminates screen tearing. This mode results in low latency, although slightly more than `vk::PresentModeKHR::eImmediate` due to the GPU blocking when the queue is full.

### `vk::PresentModeKHR::eFifoRelaxed`

Similarly to `vk::PresentModeKHR::eFifo`, the swap chain in `vk::PresentModeKHR::eFifoRelaxed` is a queue of images where image refreshes are blocked until processes are finished. However, if the GPU is late to a frame, that frame is swapped to the window surface immediately. Because it doesn't wait for any screen processes to finish, it may cause visible screen tearing. This mode results in lower latency than `vk::PresentModeKHR::eFifo` in situations when the GPU lags behind the monitor.

### `vk::PresentModeKHR::eMailbox`

This mode is another variation of the `vk::PresentModeKHR::eFifo` except when the queue is full, images already in the queue are replaced with newer frames. Commonly known as *triple buffering*, this mode usually uses more energy while avoiding screen tearing. It result in lower latency than `vk::PresentModeKHR::eFifo` because the GPU is never blocked. 

## The Swap Extent

The swap extent, or the resolution of the swap chain images has the exact same dimensions as the resolution of the window surface. Because Vulkan uses pixels by convention, the swap chain image resolutions must also be in pixels.

For GLFW, we must use `glfwGetFramebufferSize` to get the exact window resolution *in pixels*, or otherwise screen coordinates used by GLFW by default will result in a resolution mismatch (in high DPI displays).

## Surface Formatting

The format of a surface defines the displayed color space and its color precision (color depth) or color formatting.

### Color Spaces

Common color spaces used for surface formatting include `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR` or sRGB formats such as `VK_FORMAT_B8G8R8A8_SRGB` or `VK_FORMAT_R8G8B8A8_SRGB` with automatic [gamma correction](https://en.wikipedia.org/wiki/Gamma_correction).

Note: *In sRGB formats, it is important to note that the final image before displaying **must contain linear sRGB values**!*

(More on color spaces can be found in the [official documentation](https://docs.vulkan.org/spec/latest/chapters/formats.html).)

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html)  
