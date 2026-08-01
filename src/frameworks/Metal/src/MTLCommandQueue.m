/*
  MTLCommandQueue.m — Vulkan command buffer allocation

  OSEXTL: OSXIE Translation Library
*/

#import "MTLCommandQueue.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLCommandQueue requires ARC"
#endif

@implementation MTLCommandQueue {
    VkCommandPool _vk_command_pool; // Command pool for buffer allocation
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLCommandQueue: Initializing via MTLDevice");
    return nil;
}

- (void)allocateCommandBuffer:(VkCommandPool)*vk_pool {
    VkResult res = vkAllocateCommandPool((VkDevice*)vk_pool, VK_COMMAND_BUFFER_CREATEFlags, &vk_alloc);
    NSLog(@"[MTLCommandQueue] vkAllocateCommandPool: %d", res);
}

- (void)dispatchCompute {
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    VkCommandBuffer cmd_buf;
    vkAllocateCommandBuffers(vk_alloc, &alloc_info, 1, &cmd_buf);
}

@end // MTLCommandQueue — Vulkan command buffer allocation complete.