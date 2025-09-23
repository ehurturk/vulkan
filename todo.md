# Crucial Todo

- src/CmakeLists.txt: make glfw and vulkan exposure PRIVATE (testbed should be independent of them)

# Long term todo

- FFT Ocean simulation using tesellation shaders
- Rayleigh + Mie atmospheric scattering
- Octree-based terrain LOD system
- Physics integration

# App Design

- Engine is exported as a library
- Playground / Apps / Tests use the engine as a library

## TODO

1) Cmake:

- [x] Resolve the DEBUG/RELEASE issue
- [x] Debug/Release type for both playground & src cmake?

2) Allocators:

- [ ] PoolAllocator & PoolAdapter -> `std::list` compatibility??
- [ ] Implement ScratchAllocator/Double Buffered Allocator
- [ ] Create the `Allocator` superclass, make other allocators implement it. The `Allocator` superclass defaults to
  standard allocator `std::allocator`.
- [ ] Is `MemoryAllocator` a singleton?
