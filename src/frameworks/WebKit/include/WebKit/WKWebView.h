#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

@class WKWebViewConfiguration;
@protocol WKNavigationDelegate;

@interface WKWebView : NSView {
    NSURL *_currentURL;
    id<WKNavigationDelegate> _navigationDelegate;
    WKWebViewConfiguration *_configuration;
}

@property (nonatomic, readonly, copy) NSURL *URL;
@property (nonatomic, readonly, copy) NSString *title;
@property (nonatomic, readonly, getter=isLoading) BOOL loading;
@property (nonatomic, readonly, getter=canGoBack) BOOL canGoBack;
@property (nonatomic, readonly, getter=canGoForward) BOOL canGoForward;
@property (nonatomic, assign) id<WKNavigationDelegate> navigationDelegate;
@property (nonatomic, readonly, strong) WKWebViewConfiguration *configuration;

- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration *)configuration;
- (void)loadRequest:(NSURLRequest *)request;
- (void)loadHTMLString:(NSString *)string baseURL:(NSURL *)baseURL;
- (void)reload;
- (void)stopLoading;
- (void)goBack;
- (void)goForward;

@end