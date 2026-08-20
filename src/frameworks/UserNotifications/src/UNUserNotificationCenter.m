/*
 This file is part of Osxie.

 Copyright (C) 2021 Lubos Dolezel

 Osxie is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Osxie is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Osxie.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <UserNotifications/UNUserNotificationCenter.h>
#import <UserNotifications/UNNotificationRequest.h>
#import <UserNotifications/UNNotificationContent.h>
#import <UserNotifications/UNNotificationSettings.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSProcessInfo.h>

extern int osxie_dbus_portal_notify(void *portal, const char *app_name,
                                    const char *summary, const char *body,
                                    const char *icon_name);

@implementation UNUserNotificationCenter

static UNUserNotificationCenter *_instance = nil;

+ (UNUserNotificationCenter *)currentNotificationCenter {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _instance = [[UNUserNotificationCenter alloc] init];
    });
    return _instance;
}

- (void)requestAuthorizationWithOptions:(UNAuthorizationOptions)options
                      completionHandler:(void (^)(BOOL granted, NSError *error))handler {
    if (handler) {
        handler(YES, nil);
    }
}

- (void)addNotificationRequest:(UNNotificationRequest *)request
       withCompletionHandler:(void (^)(NSError *error))handler {
    if (request && request.content) {
        const char *appName = [[[NSBundle mainBundle] infoDictionary][@"CFBundleName"] UTF8String];
        if (!appName) appName = [[[NSProcessInfo processInfo] processName] UTF8String];
        const char *title = [request.content.title UTF8String];
        const char *body = [request.content.body UTF8String];

        if (osxie_dbus_portal_notify) {
            osxie_dbus_portal_notify(NULL, appName,
                title ? title : "",
                body ? body : "",
                "");
        }
    }
    if (handler) {
        handler(nil);
    }
}

- (void)getNotificationSettingsWithCompletionHandler:(void (^)(UNNotificationSettings *settings))handler {
    UNNotificationSettings *settings = [[UNNotificationSettings alloc] init];
    settings->_authorizationStatus = 2; /* UNAuthorizationStatusAuthorized */
    if (handler) {
        handler(settings);
    }
}

@end
