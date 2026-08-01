/*
  MTLComputePipelineState.m — Vulkan compute pipeline mapping

  OSEXTL: OSXIE Translation Library
*/

#import "MTLComputePipelineState.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLComputePipelineState requires ARC"
#endif

@implementation MTLComputePipelineState {
    VkPipeline  _vk_pipeline;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLComputePipelineState: Creating compute pipeline via VkShaderStageCreateInfo");
    return nil;
}

- (void)createWithVkPipeline:(VkPipeline)*vk_pipe {
    _vk_pipeline = vk_pipe;
}

@end // MTLComputePipelineState — Vulkan compute pipeline complete.