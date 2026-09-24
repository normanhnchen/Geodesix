# Resource Acquisition Is Initialization

**Resource Acquisition Is Initialization (RAII)**, is a programming concept where a resource's lifecycle is tied to a local variable's lifetime. The constructor first acquires the resource, then after it is automatically destroyed and the resources are cleaned up.

In Vulkan, RAII is used very commonly used throughout an application to solve the challenge of manual resource management in such a verbose API in C++. Therefore, we use Vulkan's official RAII bindings.

Vulkan RAII objects can be retrieved with the `vk::raii` namespace containing RAII wrapper classes.
