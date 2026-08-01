/*
  MTLComputeCommandEncoder.m — Vulkan compute dispatch mapping

  OSEXTL: OSXIE Translation Library
*/

#import "MTLComputeCommandEncoder.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLComputeCommandEncoder requires ARC"
#endif

@implementation MTLComputeCommandEncoder {
    VkCommandBuffer  _vk_cmd_buf;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLComputeCommandEncoder: Dispatch via vkCmdDispatch");
    return nil;
}

- (void)dispatchComputePipeline:(VkPipeline)*vk_pipeline {
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
}

@end // MTLComputeCommandEncoder — Vulkan compute dispatch complete.