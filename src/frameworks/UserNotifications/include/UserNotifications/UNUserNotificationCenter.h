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

#include <Foundation/Foundation.h>

typedef NS_OPTIONS(NSUInteger, UNAuthorizationOptions) {
    UNAuthorizationOptionBadge   = (1 << 0),
    UNAuthorizationOptionSound   = (1 << 1),
    UNAuthorizationOptionAlert   = (1 << 2),
    UNAuthorizationOptionCarPlay = (1 << 3),
};

typedef NS_ENUM(NSUInteger, UNAuthorizationStatus) {
    UNAuthorizationStatusNotDetermined = 0,
    UNAuthorizationStatusDenied        = 1,
    UNAuthorizationStatusAuthorized     = 2,
    UNAuthorizationStatusProvisional    = 3,
};

@class UNNotificationSettings;
@class UNNotificationRequest;

@interface UNUserNotificationCenter : NSObject

@property(weak) id delegate;

+ (UNUserNotificationCenter *)currentNotificationCenter;

- (void)requestAuthorizationWithOptions:(UNAuthorizationOptions)options
                      completionHandler:(void (^)(BOOL granted, NSError *error))handler;

- (void)addNotificationRequest:(UNNotificationRequest *)request
       withCompletionHandler:(void (^)(NSError *error))handler;

- (void)getNotificationSettingsWithCompletionHandler:(void (^)(UNNotificationSettings *settings))handler;

@end
