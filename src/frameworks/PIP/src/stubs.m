/*
 PIP.framework stub — private framework referenced by IINA.
*/

#import <Foundation/Foundation.h>
#import <AppKit/NSViewController.h>

@interface PIPViewController : NSViewController
@end

@implementation PIPViewController
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes:"v@:"];
}
- (void)forwardInvocation:(NSInvocation *)anInvocation {
}
@end
