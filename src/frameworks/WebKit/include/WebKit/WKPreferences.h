#include <Foundation/Foundation.h>

@interface WKPreferences : NSObject {
    BOOL _javaScriptEnabled;
    BOOL _javaScriptCanOpenWindowsAutomatically;
}

@property (nonatomic) BOOL javaScriptEnabled;
@property (nonatomic) BOOL javaScriptCanOpenWindowsAutomatically;

@end