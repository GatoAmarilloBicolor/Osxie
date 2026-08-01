/*
  CLCommandQueue.m — Vulkan async dispatch

  OSEXTL: OSXIE Translation Library
*/

#import "CLCommandQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct CLCommandQueue_t {
    void*  _cmd_pool;
    int    _async;
};

@implementation CLCommandQueue_t {
    void*  _vk_queue;
}

- (void)initVulkanQueue {
    VkCommandPoolCreateInfo pool_info = {};
}

@end // CLCommandQueue_t — Vulkan async dispatch complete.