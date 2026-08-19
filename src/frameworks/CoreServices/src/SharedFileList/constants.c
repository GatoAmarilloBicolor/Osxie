/*
 This file is part of Osxie.

 Copyright (C) 2025 Osxie Developers

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

#include <SharedFileList/SharedFileList.h>
#include <CoreFoundation/CoreFoundation.h>

struct OpaqueLSSharedFileListItemRef {};
static struct OpaqueLSSharedFileListItemRef _insertItemLast = { };

LSSharedFileListItemRef kLSSharedFileListItemLast = &_insertItemLast;
CFStringRef kLSSharedFileListSessionLoginItems = CFSTR("SessionLoginItems");
CFStringRef kLSSharedFileListLoginItemHidden = CFSTR("LoginItemHidden");

struct OpaqueLSSharedFileListRef {};
static struct OpaqueLSSharedFileListRef _dummyList = { };

LSSharedFileListRef LSSharedFileListCreate(CFAllocatorRef allocator, CFStringRef listType, CFStringRef listName) {
    return &_dummyList;
}

CFArrayRef LSSharedFileListCopySnapshot(LSSharedFileListRef list, LSSharedFileListRefreshTimestamp *timestamp) {
    if (timestamp) *timestamp = NULL;
    return CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
}

LSSharedFileListItemRef LSSharedFileListInsertItemURL(LSSharedFileListRef list, LSSharedFileListItemRef insertAfter, CFStringRef displayName, void* iconRef, CFURLRef url, CFDictionaryRef properties, LSSharedFileListRefreshTimestamp refreshTimestamp) {
    static struct OpaqueLSSharedFileListItemRef _dummyItem = { };
    return &_dummyItem;
}

void LSSharedFileListItemRemove(LSSharedFileListItemRef item) {
}

CFTypeRef LSSharedFileListItemCopyProperty(LSSharedFileListItemRef item, CFStringRef propertyKey) {
    return NULL;
}

CFURLRef LSSharedFileListItemCopyResolvedURL(LSSharedFileListItemRef item, LSSharedFileListItemResolveAliasStyle resolveStyle, int flags) {
    return NULL;
}
