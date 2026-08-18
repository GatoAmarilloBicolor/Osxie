#import <Foundation/Foundation.h>
@interface UTType : NSObject
+ (UTType *)typeWithIdentifier:(NSString *)identifier;
@end
@implementation UTType
+ (UTType *)typeWithIdentifier:(NSString *)identifier { return [[UTType alloc] init]; }
@end
