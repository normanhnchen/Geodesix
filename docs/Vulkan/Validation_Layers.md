# Validation Layers

Because Vulkan is designed to be as memory efficient and performant as possible, the one thing *on defafult* the API critically lacks are error checks and validations. Thus, bugs in the code will often cause silent errors or unexpected crashes. Therefore, Vulkan added its own system called **validation layers**. 

Validation layers can be enabled with Vulkan's SDK `VK_LAYER_KHRONOS_validation`. To access its debug messenger callback features that actually print out the details we use the object `vk::raii::DebugUtilsMessengerEXT`.
