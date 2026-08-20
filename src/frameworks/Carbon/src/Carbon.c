/*
 This file is part of Osxie.

 Copyright (C) 2017 Lubos Dolezel

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


#include <Carbon/Carbon.h>

#include <osxie_stub_log.h>

OSXIE_STUB_INIT




// These stubs should prob be moved elsewhere

OSErr ActivateTSMDocument(TSMDocumentID a)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSErr DeactivateTSMDocument(TSMDocumentID a)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus CreateStandardAlert(AlertType a, CFStringRef b, CFStringRef c, const AlertStdCFStringAlertParamRec * d, DialogRef * e)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSErr UseInputWindow(TSMDocumentID a, Boolean b)
{
    OSXIE_STUB_CALLED;
	return 0;
}

void FlushEvents(EventMask a, EventMask b)
{
    OSXIE_STUB_CALLED;
}

EventTargetRef GetApplicationEventTarget(void)
{
    OSXIE_STUB_CALLED;
	return (EventTargetRef)0;
}

OSStatus GetEventDispatcherTarget()
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus GetScrapByName(CFStringRef a, OptionBits b, ScrapRef * c)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus GetScrapFlavorData(ScrapRef a, ScrapFlavorType b, Size * c, void * d)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus GetScrapFlavorSize(ScrapRef a, ScrapFlavorType b, Size * c)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus GetStandardAlertDefaultParams(AlertStdCFStringAlertParamPtr a, UInt32 b)
{
    OSXIE_STUB_CALLED;
	return 0;
}

void HideMenuBar(void)
{
    OSXIE_STUB_CALLED;
}

OSErr NMInstall(NMRecPtr a)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSErr NewTSMDocument(short a, InterfaceTypeList b, TSMDocumentID * c, long d)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus ProcessHICommand(const HICommand * a)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus PutScrapFlavor(ScrapRef a, ScrapFlavorType b, ScrapFlavorFlags c, Size d, const void * e)
{
    OSXIE_STUB_CALLED;
	return 0;
}

void RunApplicationEventLoop(void)
{
    OSXIE_STUB_CALLED;
}

OSStatus RunStandardAlert(DialogRef a, ModalFilterUPP b, DialogItemIndex * c)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus SetEventMask(EventMask a)
{
    OSXIE_STUB_CALLED;
	return 0;
}

OSStatus SetEventParameter(EventRef a, EventParamName b, EventParamType c, UInt32 d, const void * e)
{
    OSXIE_STUB_CALLED;
	return 0;
}

void ShowMenuBar(void)
{
    OSXIE_STUB_CALLED;
}

void GetKeys (KeyMap theKeys)
{
    OSXIE_STUB_CALLED;
}


