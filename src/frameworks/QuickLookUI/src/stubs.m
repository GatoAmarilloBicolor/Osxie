/*
 QuickLookUI stub framework — provides the classes that real apps reference
 at load time (QLPreviewPanel, QLThumbnailGenerator, etc.).
*/

#import <Foundation/Foundation.h>

@interface QLPreviewPanel : NSObject
+ (BOOL)sharedPreviewPanelExists;
+ (QLPreviewPanel *)sharedPreviewPanel;
- (BOOL)sharedPreviewPanelExists;
- (QLPreviewPanel *)sharedPreviewPanel;
- (void)makeKeyAndOrderFront:(id)sender;
- (void)orderOut:(id)sender;
- (void)setDataSource:(id)dataSource;
- (void)setDelegate:(id)delegate;
- (void)updateController;
- (void)reloadData;
@end

@implementation QLPreviewPanel
+ (BOOL)sharedPreviewPanelExists {
    return NO;
}
+ (QLPreviewPanel *)sharedPreviewPanel {
    return nil;
}
- (BOOL)sharedPreviewPanelExists {
    return NO;
}
- (QLPreviewPanel *)sharedPreviewPanel {
    return nil;
}
- (void)makeKeyAndOrderFront:(id)sender {}
- (void)orderOut:(id)sender {}
- (void)setDataSource:(id)dataSource {}
- (void)setDelegate:(id)delegate {}
- (void)updateController {}
- (void)reloadData {}
- (BOOL)respondsToSelector:(SEL)aSelector {
    if (aSelector == @selector(sharedPreviewPanelExists) ||
        aSelector == @selector(sharedPreviewPanel) ||
        aSelector == @selector(makeKeyAndOrderFront:) ||
        aSelector == @selector(orderOut:) ||
        aSelector == @selector(setDataSource:) ||
        aSelector == @selector(setDelegate:) ||
        aSelector == @selector(updateController) ||
        aSelector == @selector(reloadData))
        return YES;
    return [super respondsToSelector:aSelector];
}
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes:"v@:"];
}
- (void)forwardInvocation:(NSInvocation *)anInvocation {
}
@end

@interface QLThumbnailGenerationRequest : NSObject
@end

@implementation QLThumbnailGenerationRequest
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes:"v@:"];
}
- (void)forwardInvocation:(NSInvocation *)anInvocation {
}
@end

@interface QLThumbnailGenerator : NSObject
+ (QLThumbnailGenerator *)sharedGenerator;
@end

@implementation QLThumbnailGenerator
+ (QLThumbnailGenerator *)sharedGenerator {
    static QLThumbnailGenerator *gen = nil;
    if (!gen) gen = [[QLThumbnailGenerator alloc] init];
    return gen;
}
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes:"v@:"];
}
- (void)forwardInvocation:(NSInvocation *)anInvocation {
}
@end
