# Vulkan Project
A Vulkan game engine designed to aid my understanding of game engines, computer architectures, platform abstractions, optimizations (through vector instructions such as SIMD), and build a fully-fledged architecture and system to experiment with things.

## Roadmap
- ~Set up development environment with CMake~
- ~Extract engine source from playground source, making engine a static library (TODO: Change to dynamic library in the future)~
- Platform independence layer
  - ~Platform detection~
  - Collections & Iterators
  - File system
  - Vulkan/OpenGL Wrapper
- Core Systems
  - Start-up & Shut-down logic
  - Assertions
  - Memory Allocators (stack, doubly stack, pool allocators) (TODO: Avoid fragmentation)
  - Math library (SIMD optimized?)
  - Strings & String IDs
  - Logging
  - Parsers (Schema, CSV, Json)
  - Engine config
  - Async File IO
- Resource
  - Resource Manager
  - Texture resource
  - 3D Model resource
  - Material resource
  - Font resource
  - etc.
- Renderer Backend
  - Materials & Textures & Shaders (SPIR-V)
  - Cameras & Projections
  - Lighting (static & dynamic)
  - Primitive submissions to GPU
- Renderer Frontend
  - Light mapping & Dynamic Cascaded Shadows
  - PBR lighting
  - Post effects
  - Deferred rendering
  - Clustered rendering
