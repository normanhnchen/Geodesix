# The Physical Device

The Vulkan physical device bridges the gap between the Vulkan API and physical hardware, or GPUs.

To select a physical device, we test if it is compatible with the Vulkan API, any extensions, [queue families](./Queues.md#queue-families), and any other required features or properties. We can also rank GPUs by "score" depending if it is most suitable and most performant for our needs. 

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html)  
