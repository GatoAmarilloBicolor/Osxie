/*
 This file is part of Osxie.

 Copyright (C) 2019 Lubos Dolezel

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

#import <QuickLookUI/QLPreviewPanel.h>

@implementation QLPreviewPanel

+ (BOOL)sharedPreviewPanelExists
{
    return NO;
}

+ (QLPreviewPanel *)sharedPreviewPanel
{
    return nil;
}

- (BOOL)sharedPreviewPanelExists
{
    return NO;
}

- (QLPreviewPanel *)sharedPreviewPanel
{
    return nil;
}

- (void)makeKeyAndOrderFront:(id)sender
{
}

- (void)orderOut:(id)sender
{
}

- (void)setDataSource:(id)dataSource
{
}

- (void)setDelegate:(id)delegate
{
}

- (void)updateController
{
}

- (void)reloadData
{
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
    if (aSelector == @selector(sharedPreviewPanelExists) ||
        aSelector == @selector(sharedPreviewPanel) ||
        aSelector == @selector(makeKeyAndOrderFront:) ||
        aSelector == @selector(orderOut:) ||
        aSelector == @selector(setDataSource:) ||
        aSelector == @selector(setDelegate:) ||
        aSelector == @selector(updateController) ||
        aSelector == @selector(reloadData))
        return YES;
    return [super respondsToSelector:aSelector];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes:"v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
}

@end
