#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WKWebView.h>
#import <WebKit/WKWebViewConfiguration.h>
#import <WebKit/WKNavigationDelegate.h>
#import <WebKit/WKWebsiteDataStore.h>
#import <WebKit/WKUserContentController.h>
#import <WebKit/WKPreferences.h>
#import <WebKit/WKUserScript.h>

@implementation WKWebView

- (instancetype)initWithFrame:(NSRect)frame configuration:(WKWebViewConfiguration *)configuration {
    self = [super initWithFrame:frame];
    if (self) {
        _configuration = configuration ?: [[WKWebViewConfiguration alloc] init];
        _currentURL = nil;
        NSLog(@"[OSXIE WebKit] WKWebView initialized with frame: %@", NSStringFromRect(frame));
    }
    return self;
}

- (void)loadRequest:(NSURLRequest *)request {
    _currentURL = request.URL;
    NSLog(@"[OSXIE WebKit] Loading URL: %@", _currentURL);
    if ([_navigationDelegate respondsToSelector:@selector(webView:didStartProvisionalNavigation:)]) {
        [_navigationDelegate webView:self didStartProvisionalNavigation:nil];
    }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if ([self->_navigationDelegate respondsToSelector:@selector(webView:didFinishNavigation:)]) {
            [self->_navigationDelegate webView:self didFinishNavigation:nil];
        }
    });
}

- (void)loadHTMLString:(NSString *)string baseURL:(NSURL *)baseURL {
    _currentURL = baseURL;
    NSLog(@"[OSXIE WebKit] Loading HTML content (length: %lu)", (unsigned long)string.length);
    if ([_navigationDelegate respondsToSelector:@selector(webView:didStartProvisionalNavigation:)]) {
        [_navigationDelegate webView:self didStartProvisionalNavigation:nil];
    }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if ([self->_navigationDelegate respondsToSelector:@selector(webView:didFinishNavigation:)]) {
            [self->_navigationDelegate webView:self didFinishNavigation:nil];
        }
    });
}

- (void)reload {
    NSLog(@"[OSXIE WebKit] Reloading current page");
    if (_currentURL) {
        [self loadRequest:[NSURLRequest requestWithURL:_currentURL]];
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
    return _currentURL;
}

- (NSString *)title {
    return @"Osxie WebView";
}

- (BOOL)isLoading {
    return NO;
}

- (void)setNavigationDelegate:(id<WKNavigationDelegate>)navigationDelegate {
    _navigationDelegate = navigationDelegate;
}

- (id<WKNavigationDelegate>)navigationDelegate {
    return _navigationDelegate;
}

- (WKWebViewConfiguration *)configuration {
    return _configuration;
}

@end

@implementation WKWebViewConfiguration

- (instancetype)init {
    self = [super init];
    if (self) {
        _preferences = [[WKPreferences alloc] init];
        _userContentController = [[WKUserContentController alloc] init];
        _websiteDataStore = [WKWebsiteDataStore defaultDataStore];
        NSLog(@"[OSXIE WebKit] WKWebViewConfiguration initialized");
    }
    return self;
}

- (WKPreferences *)preferences {
    return _preferences;
}

- (void)setPreferences:(WKPreferences *)preferences {
    _preferences = preferences;
}

- (WKUserContentController *)userContentController {
    return _userContentController;
}

- (void)setUserContentController:(WKUserContentController *)userContentController {
    _userContentController = userContentController;
}

- (WKWebsiteDataStore *)websiteDataStore {
    return _websiteDataStore;
}

- (void)setWebsiteDataStore:(WKWebsiteDataStore *)websiteDataStore {
    _websiteDataStore = websiteDataStore;
}

@end

@implementation WKPreferences

- (instancetype)init {
    self = [super init];
    if (self) {
        _javaScriptEnabled = YES;
        _javaScriptCanOpenWindowsAutomatically = NO;
    }
    return self;
}

- (BOOL)javaScriptEnabled {
    return _javaScriptEnabled;
}

- (void)setJavaScriptEnabled:(BOOL)javaScriptEnabled {
    _javaScriptEnabled = javaScriptEnabled;
}

- (BOOL)javaScriptCanOpenWindowsAutomatically {
    return _javaScriptCanOpenWindowsAutomatically;
}

- (void)setJavaScriptCanOpenWindowsAutomatically:(BOOL)javaScriptCanOpenWindowsAutomatically {
    _javaScriptCanOpenWindowsAutomatically = javaScriptCanOpenWindowsAutomatically;
}

@end

@implementation WKUserContentController

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

@implementation WKWebsiteDataStore

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