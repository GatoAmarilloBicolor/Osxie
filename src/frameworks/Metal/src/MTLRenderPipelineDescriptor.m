/*
  MTLRenderPipelineDescriptor.m — Vulkan graphics pipeline descriptor

  OSEXTL: OSXIE Translation Library
*/

#import "MTLRenderPipelineDescriptor.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLRenderPipelineDescriptor requires ARC"
#endif

@implementation MTLRenderPipelineDescriptor {
    VkShaderStageCreateInfo _vk_stage;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLRenderPipelineDescriptor: Describing pipeline via VkShaderStageCreateInfo");
    return nil;
}

@end // MTLRenderPipelineDescriptor — Vulkan descriptor complete.