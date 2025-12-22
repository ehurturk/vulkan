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
