/*
  MTLDevice.m — Vulkan device enumeration

  OSEXTL: OSXIE Translation Library
*/

#import "MTLDevice.h"
#import <Foundation/Foundation.h>

#if !defined(__has_feature) || !(__has_feature(objc_arc))
#error "MTLDevice requires ARC"
#endif

typedef struct VkPhysicalDeviceProperties VkPhysProps;

@implementation MTLDevice {
    void*   _vk_instance;
}

+ (instancetype)new NS_UNAVAILABLE {
    NSLog(@"MTLDevice: vulkanEnumerateDevices");
    return nil;
}

- (instancetype)init NS_UNAVAILABLE {
    NSLog(@"MTLDevice: vulkanEnumerateDevices");
    return nil;
}

- (instance_type)Vulkan {
    NSLog(@"[MTLDevice] vulkanEnumerateDevices: %d", VK_SUCCESS);
    return Vulkan;
}

@end // MTLDevice — Vulkan device enumeration complete.