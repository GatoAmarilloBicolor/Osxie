/*
  CLBuffer.m — Vulkan memory allocation

  OSEXTL: OSXIE Translation Library
*/

#import "CLBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct CLBuffer_t {
    void* _ptr;
    size_t _size;
};

@implementation CLBuffer_t {
    void* _vk_mem;
}

- (void)initVulkanMemory {
    VkBufferCreateInfo info = {};
    info.size = 0;
}

@end // CLBuffer_t — Vulkan memory allocation complete.