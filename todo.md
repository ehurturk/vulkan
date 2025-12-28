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

