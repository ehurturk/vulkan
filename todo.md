# TODO

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


- Rewrite the application extend logic, i think this is a bit confusing, idk.

- I think all the renderer, window, and camera and input stuff should be in the base application
 class? 
 
- Rethink design choices:
   - Does platform own application?
   - Does platform own window?
   - Who owns inupt manager? should it even be an instance, or can it be singleton (note
     thread-safeness)?

- Make delta time and frame time (and other AppStats) public and accesible via a singleton?
    - no thread concern here, as they will be read only probably? -> return const

- Add the ability to configure window size, title, mode (windowed vs fullscreen etc.) here.

- Platform create input seems funny, change it to something reasonable.
      -> maybe the platform owns the input system? seems more reasonable.

- Decouple the renderer here, maybe do not expose the renderer at all!

- implement glfw window missing functionalities

- Resource path -> maybe embedded inside the platform's paths/filesystem?
  - For loading assets / resources

- Platform initialization -> args

- Multiple objects -> multiple Assimp aiScene*'s?
  - Or a scene graph?

# Roadmap

Currently in phase: Integrating VMA into vulkan renderer

1) VMA:
- [x] Depth images use VMA
- [ ] Texture images use VMA
- [ ] Vertex & Index buffers use VMA + unified buffer for vertex and index buffers
- [ ] All other memory allocations should use VMA

2) Vulkan Abstraction
- [ ] Abstract VKShaderModule + Vkshaders into a ShaderGPU class
- [ ] Abstract VkBuffer + VmaAllocation into a general BufferGPU class (do NOT create staging buffers, instead let VMA handle it via `HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT` + vmaCopyMemoryToAllocation)
- [ ] Abstract VkImage + VmaAllocation + necessary components into a TextureGPU class (handle layouts, transitions, transfers as well.)
- [ ] Abstract VkPipeline...
- [ ] Better abstract Model & Mesh & Material classes

3) Create proper resource management system
- [ ] Finish away ResourcePool, ResourceStore and individual resources
- [ ] Create a path-independent resource loading API (look into platform absraction paths i.e. UnixPlatform & WindowsPlatform)

4) Integrate ImGui for a GUI
- [ ] Integrate ImGui to the engine
- [ ] Select a good ImGui style
- [ ] Add certain parameters to the ImGui
- [ ] Display hardware information, FPS, etc.
- [ ] Display graphs & profiler information in ImGui.

5) Migrate to Vulkan 1.3
- [ ] Use Dynamic Rendering
- [ ] Use Bindless Descriptors (i.e. buffer device addresses)

6) Multithreading for Resource Loading (Streaming)
- [ ] Use worker threads & job system to load model resources concurrently.

7) Multithreading for Command Buffer Recording
- [ ] Use both primary buffers and secondary buffers for multithreaded command buffer rendering
