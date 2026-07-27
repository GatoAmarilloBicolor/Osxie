#import <Foundation/Foundation.h>
#import <WebKit/WKWebView.h>
#import <WebKit/WKWebViewConfiguration.h>
#import <WebKit/WKNavigationDelegate.h>
#import <WebKit/WKWebsiteDataStore.h>
#import <WebKit/WKUserContentController.h>
#import <WebKit/WKPreferences.h>
#import <WebKit/WKUserScript.h>
#import <objc/runtime.h>

@interface WKWebView (OsxieImpl)
@end

@interface WKWebViewConfiguration (OsxieImpl)
@end

@interface WKPreferences (OsxieImpl)
@end

@interface WKUserContentController (OsxieImpl)
@end

@interface WKWebsiteDataStore (OsxieImpl)
@end

@implementation WKWebView (OsxieImpl)

- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration *)configuration {
    self = [super init];
    if (self) {
        objc_setAssociatedObject(self, @selector(configuration), configuration ?: [[WKWebViewConfiguration alloc] init], OBJC_ASSOCIATION_RETAIN);
        objc_setAssociatedObject(self, @selector(URL), nil, OBJC_ASSOCIATION_RETAIN);
        NSLog(@"[OSXIE WebKit] WKWebView initialized");
    }
    return self;
}

- (void)loadRequest:(NSURLRequest *)request {
    objc_setAssociatedObject(self, @selector(URL), request.URL, OBJC_ASSOCIATION_RETAIN);
    NSLog(@"[OSXIE WebKit] Loading URL: %@", request.URL);
    id delegate = objc_getAssociatedObject(self, @selector(navigationDelegate));
    if ([delegate respondsToSelector:@selector(webView:didStartProvisionalNavigation:)]) {
        [delegate webView:self didStartProvisionalNavigation:nil];
    }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        id del = objc_getAssociatedObject(self, @selector(navigationDelegate));
        if ([del respondsToSelector:@selector(webView:didFinishNavigation:)]) {
            [del webView:self didFinishNavigation:nil];
        }
    });
}

- (void)loadHTMLString:(NSString *)string baseURL:(NSURL *)baseURL {
    objc_setAssociatedObject(self, @selector(URL), baseURL, OBJC_ASSOCIATION_RETAIN);
    NSLog(@"[OSXIE WebKit] Loading HTML content (length: %lu)", (unsigned long)string.length);
    id delegate = objc_getAssociatedObject(self, @selector(navigationDelegate));
    if ([delegate respondsToSelector:@selector(webView:didStartProvisionalNavigation:)]) {
        [delegate webView:self didStartProvisionalNavigation:nil];
    }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        id del = objc_getAssociatedObject(self, @selector(navigationDelegate));
        if ([del respondsToSelector:@selector(webView:didFinishNavigation:)]) {
            [del webView:self didFinishNavigation:nil];
        }
    });
}

- (void)reload {
    NSLog(@"[OSXIE WebKit] Reloading current page");
    NSURL *url = objc_getAssociatedObject(self, @selector(URL));
    if (url) {
        [self loadRequest:[NSURLRequest requestWithURL:url]];
    }
}

- (void)stopLoading {
    NSLog(@"[OSXIE WebKit] Stop loading");
}

- (void)goBack {
    NSLog(@"[OSXIE WebKit] Go back");
}

- (void)goForward {
    NSLog(@"[OSXIE WebKit] Go forward");
}

- (BOOL)canGoBack {
    return NO;
}

- (BOOL)canGoForward {
    return NO;
}

- (NSURL *)URL {
    return objc_getAssociatedObject(self, @selector(URL));
}

- (NSString *)title {
    return @"Osxie WebView";
}

- (BOOL)isLoading {
    return NO;
}

- (void)setNavigationDelegate:(id<WKNavigationDelegate>)navigationDelegate {
    objc_setAssociatedObject(self, @selector(navigationDelegate), navigationDelegate, OBJC_ASSOCIATION_ASSIGN);
}

- (id<WKNavigationDelegate>)navigationDelegate {
    return objc_getAssociatedObject(self, @selector(navigationDelegate));
}

- (WKWebViewConfiguration *)configuration {
    return objc_getAssociatedObject(self, @selector(configuration));
}

@end

@implementation WKWebViewConfiguration (OsxieImpl)

- (instancetype)init {
    self = [super init];
    if (self) {
        objc_setAssociatedObject(self, @selector(preferences), [[WKPreferences alloc] init], OBJC_ASSOCIATION_RETAIN);
        objc_setAssociatedObject(self, @selector(userContentController), [[WKUserContentController alloc] init], OBJC_ASSOCIATION_RETAIN);
        objc_setAssociatedObject(self, @selector(websiteDataStore), [WKWebsiteDataStore defaultDataStore], OBJC_ASSOCIATION_RETAIN);
        NSLog(@"[OSXIE WebKit] WKWebViewConfiguration initialized");
    }
    return self;
}

- (WKPreferences *)preferences {
    return objc_getAssociatedObject(self, @selector(preferences));
}

- (void)setPreferences:(WKPreferences *)preferences {
    objc_setAssociatedObject(self, @selector(preferences), preferences, OBJC_ASSOCIATION_RETAIN);
}

- (WKUserContentController *)userContentController {
    return objc_getAssociatedObject(self, @selector(userContentController));
}

- (void)setUserContentController:(WKUserContentController *)userContentController {
    objc_setAssociatedObject(self, @selector(userContentController), userContentController, OBJC_ASSOCIATION_RETAIN);
}

- (WKWebsiteDataStore *)websiteDataStore {
    return objc_getAssociatedObject(self, @selector(websiteDataStore));
}

- (void)setWebsiteDataStore:(WKWebsiteDataStore *)websiteDataStore {
    objc_setAssociatedObject(self, @selector(websiteDataStore), websiteDataStore, OBJC_ASSOCIATION_RETAIN);
}

@end

@implementation WKPreferences (OsxieImpl)

- (instancetype)init {
    self = [super init];
    if (self) {
        objc_setAssociatedObject(self, @selector(javaScriptEnabled), @YES, OBJC_ASSOCIATION_RETAIN);
        objc_setAssociatedObject(self, @selector(javaScriptCanOpenWindowsAutomatically), @NO, OBJC_ASSOCIATION_RETAIN);
    }
    return self;
}

- (BOOL)javaScriptEnabled {
    return [objc_getAssociatedObject(self, @selector(javaScriptEnabled)) boolValue];
}

- (void)setJavaScriptEnabled:(BOOL)javaScriptEnabled {
    objc_setAssociatedObject(self, @selector(javaScriptEnabled), @(javaScriptEnabled), OBJC_ASSOCIATION_RETAIN);
}

- (BOOL)javaScriptCanOpenWindowsAutomatically {
    return [objc_getAssociatedObject(self, @selector(javaScriptCanOpenWindowsAutomatically)) boolValue];
}

- (void)setJavaScriptCanOpenWindowsAutomatically:(BOOL)javaScriptCanOpenWindowsAutomatically {
    objc_setAssociatedObject(self, @selector(javaScriptCanOpenWindowsAutomatically), @(javaScriptCanOpenWindowsAutomatically), OBJC_ASSOCIATION_RETAIN);
}

@end

@implementation WKUserContentController (OsxieImpl)

- (instancetype)init {
    self = [super init];
    if (self) {
        NSLog(@"[OSXIE WebKit] WKUserContentController initialized");
    }
    return self;
}

- (void)addUserScript:(WKUserScript *)userScript {
    NSLog(@"[OSXIE WebKit] Adding user script");
}

- (void)removeAllUserScripts {
    NSLog(@"[OSXIE WebKit] Removing all user scripts");
}

@end

@implementation WKWebsiteDataStore (OsxieImpl)

static WKWebsiteDataStore *_defaultDataStore = nil;

+ (WKWebsiteDataStore *)defaultDataStore {
    if (!_defaultDataStore) {
        _defaultDataStore = [[WKWebsiteDataStore alloc] init];
    }
    return _defaultDataStore;
}

+ (WKWebsiteDataStore *)nonPersistentDataStore {
    return [[WKWebsiteDataStore alloc] init];
}

- (instancetype)init {
    self = [super init];
    if (self) {
        NSLog(@"[OSXIE WebKit] WKWebsiteDataStore initialized");
    }
    return self;
}

@end