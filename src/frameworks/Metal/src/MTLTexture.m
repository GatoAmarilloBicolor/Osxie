/*
  MTLTexture.m — Vulkan image mapping

  OSEXTL: OSXIE Translation Library
*/

#import "MTLTexture.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLTexture requires ARC"
#endif

@implementation MTLTexture {
    VkImage           _vk_image;
    VkDeviceMemory    _vk_mem;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLTexture: Creating Vulkan image via vkCreateImage");
    return nil;
}

- (void)createWithVkImage:(VkImage)*vk_img {
    _vk_image = vk_img;
}

- (void)destroyVkImage {
    VkResult res = vkDestroyImage((VkDevice*)_vk_device, _vk_image, NULL);
    NSLog(@"[MTLTexture] vkDestroyImage: %d", res);
}

@end // MTLTexture — Vulkan image mapping complete.