# Rendering Multiple Objects

We need to consider which resources should be:
1) Shared across all objects - to minimize memory usage and *state changes*
2) Duplicated for each object - to allow for *independent* positioning and appearence

## Shared Resources
- Vertex + Index buffers (when objects use the same mesh)
- Textures and Samplers (when objects use the same textures)
- Pipeline objects and pipeline layouts
- Render passes
- Command pools

## Per-object Resources
- Transformation matrices (position, rotation, scale)
- Uniform buffers containing those matrices
- Descriptor sets that reference those uniform buffers
- Push constants (for small, frequently changing data)

---

In the most basic scenario, drawing multiple objects may look like:

```cpp
vkCmdBeginRenderPass(commandBuffer, ...);

vkCmdBindVertexBuffers(commandBuffer, ...);
vkCmdBindIndexBuffer(commandBuffer, ...);
vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

for (auto& object: objects) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, &object.descriptorSet, 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, object.indexCount, 1, 0, object.indexBase, 0);
}

vkCmdEndRenderPass();
```

In this most basic case:
- Each object has its own descriptor set that includes the matrices + other shader parameters

As an optimization, we can split descriptor sets into two:
1) Containing per object model matrix
2) Containing global view and projection matrices.

```cpp
for (auto& object: objects) {
    std::array<VkDescriptorSet, 2> descriptorSets{};
    descriptorSets[0] = scene.descriptorSet;
    descriptorSets[1] = object.descriptorSet;
    vkCmdBindDescriptorSets(commanddBuffer, ...);
}
```
