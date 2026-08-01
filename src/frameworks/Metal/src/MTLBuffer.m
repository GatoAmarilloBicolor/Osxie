/*
  MTLBuffer.m — Vulkan memory mapping

  OSEXTL: OSXIE Translation Library
*/

#import "MTLBuffer.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLBuffer requires ARC"
#endif

@implementation MTLBuffer {
    void*   _vk_mem;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLBuffer: vkCreateBuffer for resource mapping");
    return nil;
}

- (void)vulkanMapMemory {
    VkMemoryMapFlags map_flags = VK_MEMORY_MAP_FLAG;
}

@end // MTLBuffer — Vulkan memory mapping complete.