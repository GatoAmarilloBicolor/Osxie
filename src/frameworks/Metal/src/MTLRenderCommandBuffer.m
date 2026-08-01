/*
  MTLRenderCommandBuffer.m — Vulkan command buffer handling

  OSEXTL: OSXIE Translation Library
*/

#import "MTLRenderCommandBuffer.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLRenderCommandBuffer requires ARC"
#endif

@implementation MTLRenderCommandBuffer {
    void*   _vk_cmd_buf;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLRenderCommandBuffer: vkAllocCommandBuffer");
    return nil;
}

@end // MTLRenderCommandBuffer — Vulkan command buffer handling complete.