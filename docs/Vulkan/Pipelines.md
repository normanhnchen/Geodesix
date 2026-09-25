# Pipelines

A Vulkan pipeline (`VkPipeline`) is an immutable object that describes the entire process of how a GPU should convert data to final pixels.

## The Graphics Pipeline

A standard graphics pipeline is outlined as follows:

<div style="text-align: center;">
    <details>
        <summary><b>Input Assembler</b></summary>
        <p style="text-align: left;">Collects raw vertex data and assembles them into geometric primitives like points, lines, or triangles.</p>
    </details>
    ↓<br>
    <details>
        <summary><b>Vertex Shader</b></summary>
        <p style="text-align: left;"><b>Runs code on each vertex to perform transformations on them.</b></p>
    </details>
    ↓<br>
    <details>
        <summary><b>Tessellation (Optional)</b></summary>
        <p style="text-align: left;"><b>Allows subdivision of geometry.</b></p>
    </details>
    ↓<br>
    <details>
        <summary><b>Rasterization</b></summary>
        <p style="text-align: left;"><b>Converts geometric primitives to fragments (pixel candidates) overlapping the screen.</b></p>
    </details>
    ↓<br>
    <details>
        <summary><b>Fragment Shader</b></summary>
        <p style="text-align: left;"><b>Runs code on each fragment to calculate the final color, lighting, and textures.</b></p>
    </details>
    ↓<br>
    <details>
        <summary><b>Color Blending</b></summary>
        <p style="text-align: left;"><b>Mixes the final pixel values with the existing data inside the framebuffer (and handling transparency).</b></p>
    </details>
</div>


## References

https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/00_Introduction.html
