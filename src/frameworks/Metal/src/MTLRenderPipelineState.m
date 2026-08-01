/*
  MTLRenderPipelineState.m — Vulkan graphics pipeline mapping

  OSEXTL: OSXIE Translation Library
*/

#import "MTLRenderPipelineState.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLRenderPipelineState requires ARC"
#endif

@implementation MTLRenderPipelineState {
    VkPipeline  _vk_pipeline;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLRenderPipelineState: Creating pipeline via VkShaderStageCreateInfo");
    return nil;
}

- (void)createWithVkPipeline:(VkPipeline)*vk_pipe {
    _vk_pipeline = vk_pipe;
}

@end // MTLRenderPipelineState — Vulkan pipeline complete.