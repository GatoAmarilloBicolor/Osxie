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

#import <UserNotifications/UNNotificationRequest.h>

@implementation UNNotificationRequest

@synthesize identifier = _identifier;
@synthesize content = _content;

+ (instancetype)requestWithIdentifier:(NSString *)identifier
                              content:(UNNotificationContent *)content
                              trigger:(id)trigger
{
    UNNotificationRequest *req = [[UNNotificationRequest alloc] init];
    req->_identifier = [identifier copy];
    req->_content = [content copy];
    return req;
}

@end
