# Fixed-Functions

Fixed-functions are non-programmable operations in a [pipeline](./Pipelines.md) whose behaviour can be pre-configured but are baked into an immutable object. In contrast, [dynamic states](./Dynamic_States.md) behave similarly to fixed-functions except a limited amount ot the objects' state can be configured during runtime.

## Vertex Input

The vertex input (`vk::PipelineVertexInputStateCreateInfo`) struct specifies the format of the vertex data passed to the vertex shader:

- **Bindings**: Defines the spacing of data in memory (stride) and whether or not the data should be per-vertex or [per-instance](https://en.wikipedia.org/wiki/Geometry_instancing).

- **Attributes**: Maps attributes in the vertex shader to specific formats and specifies the byte offset within a stride.

## Input Assembly

The input assembly (vk::PipelineInputAssemblyStateCreateInfo) struct specifies the construction of vertices into primitives. The topology of these primitives can be configured by choosing one fo the following values:

- **`vk::PrimitiveTopology::ePointList`**: Draws points from vertices.
- **`vk::PrimitiveTopology::eLineList`**: Draws lines between every two vertices (without reusing vertices).
- **`vk::PrimitiveTopology::eLineStrip`**: Draws lines where the end vertex of every line is used as the start vertex for the next line.
- **`vk::PrimitiveTopology::eTriangleList`**: Draws triangles from every three vertices (without reusing vertices).
- **`vk::PrimitiveTopology::eTriangleStrip`**: Draws triangles where every second and third vertex of every triangle are reused for the next triangle's first two vertices.

## The Viewport

The Vulkan viewport descrbies the framebuffer region where the final rendered graphics output is displayed. Note that the viewport can be specified as a [dynamic state](./Dynamic_States.md#the-viewport).

## Scissors

The Vulkan scissor is a rectangular region that defines a hard clipping mask for pixels. Any pixel outside of its scissor region is completely discarded by the rasterizer. Note that the scissor can be specified as a [dynamic state](./Dynamic_States.md#scissors).

## The Rasterizer

The rasterizer converts the transformed vertex data passed from the vertex shader to fragments depending on its configurable struct parameters. It also performs any depth testing, culling, or scissor testing. The struct `vk::PipelineRasterizationStateCreateInfo` include the following configurable parameters:

- **`depthClampEnable`**: Instead of discarding fragments outside the clip planes, they are clamped to the clip planes.

- **`rasterizerDiscardEnable`**: Disables output to the framebuffer.

- **`polygonMode`**
    - `vk::PolygonMode::eFill`: Fills primitives formed by vertices with fragments
    - `vk::PolygonMode::eLine`: Connects primitive vertices with lines.
    - `vk::PolygonMode::ePoint`: Draws vertices as points.

- **`cullMode`**: Face culling type (disabled, front-face culling, back-face culling, cull both front and back-faces)

- **`frontFace`**: Determines whtether or not the front-face of a primitive includes vertices that in clockwise or counter-clockwise order.

- **`depthBiasEnable`**: Allows biasing of depth values.

- **`lineWidth`**: Controls the thickness of lines (in units of fragments).

## Color Blending

Color blending is the process of mixing new fragment shader color outputs to the data already stored in the framebuffer.

## References

[https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html)
