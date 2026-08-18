#import <Foundation/Foundation.h>
@interface MPMoviePlayerController : NSObject
@property (nonatomic, copy) NSURL *contentURL;
- (void)play;
@end
@implementation MPMoviePlayerController
- (void)play {}
@end
