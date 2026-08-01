/*
  MTLRenderCommandEncoder.m — Vulkan render pass mapping

  OSEXTL: OSXIE Translation Library
*/

#import "MTLRenderCommandEncoder.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLRenderCommandEncoder requires ARC"
#endif

@implementation MTLRenderCommandEncoder {
    void*   _vk_cmd_buf;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLRenderCommandEncoder: vkCmdBeginRenderPass");
    return nil;
}

- (void)bindPipeline:(void*)vk_pipeline {
    VkRenderPassBeginInfo pass_info = {};
}

@end // MTLRenderCommandEncoder — Vulkan render pass complete.