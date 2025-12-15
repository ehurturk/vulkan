# Learning Vulkan

## Outline of Vulkan Structures

### The Hardware Layer
- *Physical device (`VkPhysicalDevice`)*: The actual graphics card in the PC
  - We query the physical device to ask certain features (does it support Vulkan, memory size, etc.)
- *Logical Device (`VkDevice`)*: Application's view of the GPU, acts as the interface.
  - Physical device is used to check capabilities, but logical device is used to interface with Vulkan and make function calls.
- *Queues*: A GPU is a concurrent device, and there are different queues of jobs that are executed in the GPU. A single queue is a line that feeds job into the GPU. 
  - The commands aren't executed instantly when the function is called, rather, they are submitted into a queue and the GPU executes them when it can.
- *Queue Families*: There are different queue families in the GPU, each with assigned to different tasks.
  - The different queue families are:
    - *Graphics Family*: Handles drawing of triangles
    - *Transfer Familt*: Optimized for moving data (e.g. for staging buffers)
    - *Compute Family*: Optimized for calculations
- *Queue Indices*: 
  - *Queue Family Index*: Describes the type of the queue (i.e. We want the graphics family, which is ID 0)
  - *Queue Index*: Describes the specific queue in the specified *Queue Family Index* (usually set to index 0)

### The Command Layer
- *Command Buffer*: A todo list for the GPU. Compared to OpenGL (in which the GPU drew immediately when a draw call was made), Vulkan's draw command adds a new command to a command buffer. The command buffer is filled, and then the whole buffer is handed over to the Queue at once.
- *Command Pool*: Allocating memory is slow, and creating a new *Command Buffer* every frame from scratch is expensive. Therefore, a *Command Pool* is a big chunk of pre-reserved memory for "instantiating" new *Command Buffers*.
  - *Command Pools* are tied to *Queue Families* as well, therefore we need a specific command pool to make a related command buffer (i.e. we need a graphics command pool to make a graphics command buffer).
  
### The Presentation Layer
- *Surface*: Platform-agnostic window. 
  - In Windows, it is `HWND`, in Linux it is `xcb`/`Wayland`, in MacOS it is `Cocoa`. 
  - Vulkan wraps these into a `VkSurfaceKHR` so handling of window is platform-agnostic.
- *Swapchain*: The infrastructure that owns the screen buffers (essentially like a waiting line of images).
- *Swapchain Image*: The actual raw data arrays (pixels) inside the *Swapchain* (we usually have 2/3 of swapchain images for double/triple buffering).

### Data View Layer
- *Image (`VkImage`)*: Blob of raw data in memory
- *ImageView (`VKImageView`)*: Metadata that tells the driver how to look at that blob (i.e. 2D texture, use RGB format, etc.)
  - Why? We might want to treat the same *Image* as a 2D color texture in one pass, but as a depth map in another. We can use 1 `VkImage` for it and 2 different `VkImageView`s.
- *Framebuffer*: A wrapper that links the *Render Pass* to the specific *Image Views* we are drawing to right now
  - The *Render Pass* says: I output to a color attachment
  - The *Framebuffer* says: That color attachment is specifically this *Image View* from the *Swapchain*.

### The Pipeline Layer
- *Graphics Pipeline*: Monolithic object that freezes the state of the GPU. Everything (Shaders, Blending, Depth, Rasterizes) is baked into the `VkPipeline` object. If shaders want to be changed, the pipeline should be changed as well.
- *Pipeline Layout*: This is a function signature for the shader:
  - SPIR-V is the body of the function
  - The *Pipeline Layout* tells Vulkan what a pipeline expects (i.e. 2 Matrices and 1 Texture as input)
  - The pipeline layout is needed because the graphics driver need to know how to map data (Descriptor Sets) to hardware before shader is ran.
- *Render Pass*: Structure of a rendering task (i.e. use 1 Color attachment, 1 Depth attachment, clear color to black, at the end store the result, etc.)
- *Subpass*: A step inside a *Render Pass*
  - For _Deferred Rendering_, subpass 1 draws geometry/normals, and subpass 2 reads geometry and applies lighting.
  

## Outline of a Frame
1. Wait for the previous frame to finish
2. Acquire an image from the swap chain
3. Record a command buffer which draws the scene onto that image
4. Submit the recorded command buffer
5. Present the swap chain image

## Synchronization
- Synchronization of execution on the GPU is explicit

### Semaphores
Semaphore is used to add order between queue operations
    - Queue operations refer to the work we submit to a queue (command buffer / within a function)
    - Graphics queue / presentation queue / etc...

Semaphores are used both to order work inside the same queue and between different queues.

There are binary and timeline semaphores in Vulkan.

Binary semaphores are:
```cpp
VkCommandBuffer A, B = ...  // record command buffers
VkSemaphore S = ... // create a semaphore

// enqueue A, signal S when done -> starts executing immediately
vkQueueSubmit(work: A, signal: S, wait: nullptr);

// enqueue B, wait on S to start
vkQueueSubmit(work: B, signal: nullptr, wait: S)
```

Both calls to `vkQueueSubmit` return immediately - blocking happens in the GPU and the CPU is not blocked.

### Fences
Fences are used for ordering the execution on the CPU (host). If host needs to know when the GPU has finished something, use a fence.

Example: Taking a screenshot:
- GPU does necessary work
- Need to transfer image from the GPU to the host and then save the memory to a file. 
- Command buffer A executes the transfer, and we have fence F.
- Submit command buffer with fence F, then tell the host to wait for F to signal (making host blocked).

```cpp
VkCommandBuffer A = ... // record command buffer with the transfer
VkFence F = ... // create the fence

// enqueue A, start work immediately, signal F when done
vkQueueSubmit(work: A, fence: F);

vkWaitForFence(F) // blocks host execution until A has finished executing

save_screenshot_to_disk() // can't run until transfer has finished
```
