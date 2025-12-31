# Vulkan Resource Guide


## Resource Sharing Mode
Buffer and image objects are created with a sharing mode controlling how they can be accessed from **queues**.

The supported sharing modes are:

```c
typedef enum VkSharingMode {
    VK_SHARING_MODE_EXCLUSIVE = 0,
    VK_SHARING_MODE_CONCURRENT = 1,
} VkSharingMode;
```

- `VK_SHARING_MODE_EXCLUSIVE`: Specifies that access to any range or image subresource of the object will be exclusive to a single queue family at a time.
- `VK_SHARING_MODE_CONCURRENT`: Specifies that concurrent access to any range or image subresource of the object from multiple queue families is supported.

A note: `VK_SHARING_MODE_CONCURRENT` amy result in lower performance access to the buffer / image than the exclusive sharing mode.

Buffer / image objects creating using `VK_SHARING_MODE_EXCLUSIVE` must only be accessed by queues in the queue family that has **ownership** of the resource. Upon creation, such objects are not owned by any queue family.
- Ownership is acquired upon first use within a queue.

Once a resource with exclusive sharing mode is owned by a queue family, the application **must** perform a **queue family ownership transfer** if it wishes to make the memory contents of the resource accessable to a different queue family.

For images, before being used on the first queue, they still require a **layout transition** from:
- `VK_IMAGE_LAYOUT_UNDEFINED`
- `VK_IMAGE_LAYOUT_PREINITIALIZED`
- `VK_IMAGE_LAYOUT_ZERO_INITIALIZED_EXT`

## Buffers

### Vertex & Index Buffers
bla bla bla

### Staging Buffers
If we just create a buffer with a memory type that allows us to access it from the CPU (via the `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`), it may not be the most optimal memory type for the GPU to read from.

The most optimal memory type has the `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT` flag and this optimal memory is not accessible by the CPU on dedicated GPUs (hence the name).

Therefore, we are going to mitigate this by creating 2 buffers and transfer data between the two. The first buffer (called the staging buffer) is going to be in CPU accessible memory (thus is going to have `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`) to map the vertex data to its memory, and the second buffer is going to be in the GPU memory (therefore its memory type will have `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`). 

Then, a buffer copy command is going to be used to move the data from staging buffer to the actual buffer.

#### Staging Buffer for Vertex Buffers
To use a staging buffer for vertex buffer creation, we will set the staging buffer's buffer usage flags as `VK_BUFFER_USAGE_TRANSFER_SRC_BIT`. The buffer's memory is going to have a the `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` flags since we want to write to the buffer from the CPU, and we want the graphics driver to be aware of our writes to the buffer (this doesn't mean the changes are actually visible on the GPU yet, this transfer of data to the GPU happens in the background).

We then have to copy the data into the staging buffer's mapped memory.

We then create the actual vertex buffer with buffer usage flags as `VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT` since the vertex buffer is the destination buffer of a transfer operation between 2 buffers. The vertex buffer will have a memory with property `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT` which will be the most optimal memory for GPU to read.

We then copy the data in the staging buffer to the vertex buffer.

In a high level overview, a vertex buffer creation looks like:

```cpp
// Create a staging buffer to use it as a source in memory transfer operation
// It is in HOST_VISIBLE_BIT and HOST_COHERENT_BIT states.
VkBuffer stagingBuffer;
VkDeviceMemory stagingBufferMemory;
create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
              stagingBuffer, stagingBufferMemory);

void* data;
vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
vkUnmapMemory(device, stagingBufferMemory);

VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
// Make the vertex buffer a transfer destination for the memory transfer
// Memory is most optimal for the GPU: DEVICE_LOCAL_BIT
create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
              vertexBuffer, vertexBufferMemory);

copy_buffer(stagingBuffer, m_VertexBuffer, bufferSize);

vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
```

#### Transfer Queue
The buffer copy command requires a queue family that supports transfer operations (therefore the queue family has to have the `VK_QUEUE_TRANSFER_BIT` in its flags).

Memory transfer operations are executed using command buffers (like drawing commands). This operation's command buffer is going to be short-lived (i.e. is going to be a single time executed command buffer). Therefore, it is advised to create another command pool for memory transfer operations because the implementation may be able to apply memory allocation optimizations. Therefore, if another command pool is going to be created for short-lived command buffers, the `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT` flag should be used during command pool generation.

This time, we are only going to use the command buffer once and wait with returning from the function until the transfer operation has finished executing, therefore we are going to use the `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT` flag in begin info:

```cpp
VkCommandBufferBeginInfo beginInfo{};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
```

The command to copy the buffer is called `vkCmdCopyBuffer` which takes the source and the destination buffers.

In a high overview, copying buffers looks like:

```cpp
VkCommandBufferAllocateInfo allocInfo{}; // ...

VkCommandBuffer commandBuffer;
vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

VkCommandBufferBeginInfo beginInfo{};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

vkEndCommandBuffer(commandBuffer);

VkSubmitInfo submitInfo; // ... 

vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
// NOTE: A fence should be used to possibly have better optimization for 
// waiting on multiple transfer operations.
vkQueueWaitIdle(graphicsQueue); 
```

**NOTE**: Instead of calling `vkAllocateMemory` for every individual buffer, the `offset` field to utilize a single allocation for many different buffers. This can easily be achieved by using *VMA (VulkanMemoryAllocator)*.

## Images

A Vulkan image can be created using these descriptors:
- Width
- Height
- MipLevels
- Format (`VkFormat`)
- Tiling (`VkImageTiling`)
- Image Usage Fags (`VkImageUsageFlags`)


#### 1) Image Creation

We also set the image's sharing mode to `SHARING_MODE_EXCLUSIVE` because we want the image to be used for the graphics queue. The image is also going to be used for transfer operations. But since queue families that support graphics queues also have implicit support for transfer queues, we don't have to set the sharing mode to concurrent since both used queues will be in the same family. 

##### Image Usage Flags
These flags describe the intended usage of an image. The ones that are commonly used in a Vulkan renderer are:

- **`VK_IMAGE_USAGE_TRANSFER_SRC_BIT`**: This flag is used to specify that the image can be used as the source of a transfer command 
- **`VK_IMAGE_USAGE_TRANSFER_DST_BIT`**: This flag is used to specify that the image can be used as the destination of a transfer command
- **`VK_IMAGE_USAGE_SAMPLED_BIT`**: This flag is used to specify that the image can be used to create a `VkImageView` suitable for occupying a `VkDescriptorSet` slot of type `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER`.
- **`VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT`**: This flag is used to specify that the image can be used to create a `VkImageView` suitable for use as a depth/stencil attachment in a `VkFramebuffer`.
- **`VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`**: This flag is used to specify that the image can be used to create a `VkImageView` suitable for use as a color attachment in a `VkFramebuffer`.



#### 2) Staging Buffer Creation:

Similar to creating buffers, we have to utilize a staging buffer when creating an image. It is possible to use a staging *image* as the staging resource, but Vulkan allows us to have copy operations between a `VkBuffer` and a `VkImage` as well, which is proven to be faster on some hardware. Therefore similar to vertex/index buffer creations, our staging resource is going to be a buffer.

```cpp
VkBuffer stagingBuffer;
VkDeviceMemory stagingBufferMemory;
create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
              stagingBuffer, stagingBufferMemory);
void *data;
vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &pixels);
memcpy(data, pixels, imageSize); // map pixels to staging buffer memory
vkUnmapMemory(device, stagingBufferMemory);
```


#### 2) Create an Image to Copy the Pixels
```cpp
// Create the actual image in device local memory
create_image(texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB,
             VK_IMAGE_TILING_OPTIMAL,
             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                             | VK_IMAGE_USAGE_SAMPLED_BIT,
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
             textureImage, textureMemory);

```
We create the image (now with empty data and with an image layout of UNDEFINED) with the necessary properties (width, height, mip levels, imageformat, tiling, image usage flags and memory property flags).

Some of the non-trivial and explanation-worthy properties are:

Image Usage Flags: `TRANSFER_SRC_BIT | TRANSFER_DST_BIT | SAMPLED_BIT`
- We specify that the image is going to be used **both** as a source and destination in transfer operations. The image is the destination image when receiving pixel data from the staging buffer. In addition, the image is the source image when generating mipmaps. We also use the image to sample in a shader using a combined image sampler.

Memory Property Flags: `DEVICE_LOCAL_BIT`
- The image memory has property `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT` since it's memory should be optimized for the GPU to read from.

#### 3) Copy Staging Buffer To Image

##### Image Layout Transition
To copy the staging buffer to the image we have, we have to first change the image layout. We created the image with `VK_IMAGE_LAYOUT_UNDEFINED`, indicating we do not care about the contents of the image (yet). Now, the image is going to be used as the destination image that is receiving data from the staging buffer. Therefore, its new layout should be `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`. We 

We now want to perform a layout transition from `UNDEFINED` layout to `TRANSFER_DST_OPTIMAL` layout. To do so, we have to use a **image memory barrier**.

Executing a image memory barrier requires recording of a command buffer, so we have to record a single-time command buffer.

The image memory barries can be configured as such:
```cpp
VkImageMemoryBarrier barrier {};
barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
barrier.oldLayout = oldLayout;
barrier.newLayout = newLayout;
barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
barrier.image = image;
barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
barrier.subresourceRange.baseMipLevel = 0;
barrier.subresourceRange.levelCount = mipLevels;
barrier.subresourceRange.baseArrayLayer = 0;
barrier.subresourceRange.layerCount = 1;
barrier.srcAccessMask = 0; // todo: filled later
barrier.dstAccessMask = 0; // todo: filled later
```

- Layout transition is specified using `oldLayout` and `newLayout` parameters.
- Both the `srcQueueFamilyIndex` and `dstQueueFamilyIndex` parameters are ignored since we are not making a queue family ownership transfer.

Then a `vkCmdPipelineBarrier(commandBuffer, srcStage, 0, dstStage, 0, 0, nullptr, 0 , nullptr, 1, &barrier);` command can be recorded into the single time command buffer.

Since barriers are used for synchronization, we must specify the types of operations that involve the resource that must happen before the barrier and the operations that involve the resource that must wait on the barrier using the `srcAccessMask` and `dstAccessMask` parameters. These flags depend on the old and new layouts.

We have 2 transitions that are happening:
- UNDEFINED -> TRANSFER_DST_OPTIMAL: transfer writes that don't need to wait on anything
- TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL: shader reading. Shader reads should wait on transfer writes (specially the fragment shader).

```cpp
VkPipelineStageFlags sourceStage;
VkPipelineStageFlags destinationStage;

if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
} 
else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
}
```

Specifying these correclty effectively says: "Pause the Destination Stage until the Source Stage has finished completely, and while you are pausing, transform this image layout (via information in `VkImageMemoryBarrier`)."

For example, if the transition is between `UNDEFINED` and `TRANSFER_DST_OPTIMAL`, then the `srcAccessMask` can effectively be 0 since we don't care about previous data, but the `dstAccessMask` should be `VK_ACCESS_TRANSFER_WRITE_BIT`. Similarly, we should set `sourceStage` as `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT` to tell the GPU to signal start this layout change immediately. The `destinationStage` should be `VK_PIPELINE_STAGE_TRANSFER_BIT` since we want the transfer stage to wait for the layout transition to finish.

Similarly, if the transition is between `TRANSFER_DST_OPTIMAL` and `SHADER_READ_ONLY_OPTIMAL`, then `srcAccessMask` should be `VK_ACCESS_TRANSFER_WRITE_BIT` and `dstAccessMask` should be `VK_ACCESS_SHADER_READ_BIT`. Similarly, `sourceStage` is the `VK_PIPELINE_STAGE_TRANSFER_BIT` (since we want this layout change to start, hence block, at the transfer stage) and `destinationStage` is the `VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT` (since we want the resource to be available at this stage, so wait for transfer stage to finish at the fragment stage).

##### Copying Data

We now have to (again) record a command buffer to execute 
```cpp
vkCmdCopyBufferToImage(
    commandBuffer,
    buffer,
    image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
);
```
The fourth parameter is the layout the image is currently using, and since we performed a layout transition to change the image layout to `TRANSFER_DST_OPTIMAL` we are using it to describe the image layout.

#### 4) Preparing the Image
After performing an image layout transition and performing the data copy, we have to transition the image's layout to `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` to prepare it for shader access. The previous layout was `TRANSFER_DST_OPTIMAL`.