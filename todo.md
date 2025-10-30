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

- Cmake:

- [x] Resolve the DEBUG/RELEASE issue
- [x] Debug/Release type for both playground & src cmake?

- Allocators:

- [ ] PoolAllocator & PoolAdapter -> `std::list` compatibility??
- [ ] Create the `Allocator` superclass, make other allocators implement it. The `Allocator` superclass defaults to
  standard allocator `std::allocator`.
- [ ] Is `MemoryAllocator` a singleton?

- Windows
- [ ] Finish GLFWWindow & HeadlessWindow
- [ ] Wrap platform abstraction into AppPlatform
- [ ] Test loading and opening windows

- PlatformEntryPoint
- [ ] Add Windows platform creation inside the entry point

- PlatformContext:
- [ ] In entry point: why are we creating a unique pointer and returning it?

- GLFWWindow:
- [ ] Fullscreen/windowed mode (in glfwGetPrimaryMonitor)

4) Platform
- [ ] Try to think of better ways of handling platform abstraction
- [ ] Consider using spdlog for logging
- [ ] Filesystem & ResourceManager classes
- [ ] Parse window properties from platformcontext arguments
