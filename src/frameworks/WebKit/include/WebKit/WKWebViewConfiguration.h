#include <Foundation/Foundation.h>

@class WKPreferences;
@class WKUserContentController;
@class WKWebsiteDataStore;

@interface WKWebViewConfiguration : NSObject {
    WKPreferences *_preferences;
    WKUserContentController *_userContentController;
    WKWebsiteDataStore *_websiteDataStore;
}

@property (nonatomic, readonly, strong) WKPreferences *preferences;
@property (nonatomic, readonly, strong) WKUserContentController *userContentController;
@property (nonatomic, readonly, strong) WKWebsiteDataStore *websiteDataStore;

@end