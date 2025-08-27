# Engine Architecture


## `src/core/`
Contains core engine logic, such as top level application module, memory subsystems, logger and assertion units.

### `src/core/memory/`
Contains logic required for memory allocators and memory handling of the engine in general.
TODO: Support various memory allocation techniques such as destack allocator, pool allocator, etc. to avoid fragmentation.

## `src/platform/`
Contains platform specific logic to abstract away platform dependencies. Currently supported platforms are `platform_glcw`.
TODO: Support the following platforms in their platform-native implementations:
- Linux (xlib)
- MacOS (metal)
- Windows (WinAPI)

## `src/renderer/`
Contains renderer specific logic, from renderer backends (found in `src/renderer/backend/`) to frontend rendering techniques. Currently supported renderer backend is `vulkan`.
TODO: Support the following renderer backends:
- OpenGL
- DirectX 12



# Code Cleanness
1) Instead of C-style platform implementations (including platform.hpp and implementing details with a simple macro definition), utilize composition/polymorphism.
2) Care to optimize for RVO/NRVO, convert single parameter constructors to explicit.
3) Look at move/copy constructors and their operators.
4) Abstract away Vulkan backend as much as possible, in the most generic way. Use Vulkan RAII if possible.
5) Either have singletons for engine subsystems or initialize them in the required order as described in "Game Engine Architecture"

