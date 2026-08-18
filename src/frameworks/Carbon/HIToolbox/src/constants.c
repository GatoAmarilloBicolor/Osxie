#include <CoreFoundation/CoreFoundation.h>
#include <HIToolbox/TextInputSources.h>

/* HIToolbox.framework */
const CFStringRef kTISNotifySelectedKeyboardInputSourceChanged = CFSTR("com.apple.Carbon.TISNotifySelectedKeyboardInputSourceChanged");

const CFStringRef kTISPropertyInputSourceID = CFSTR("TISPropertyInputSourceID");
const CFStringRef kTISPropertyInputSourceIsFromSystem = CFSTR("TISPropertyInputSourceIsFromSystem");
const CFStringRef kTISPropertyInputSourceIsASCIICapable = CFSTR("TISPropertyInputSourceIsASCIICapable");

const CFStringRef kTISPropertyUnicodeKeyLayoutData = CFSTR("TISPropertyUnicodeKeyLayoutData");
const CFStringRef kTISCategoryKeyboardInputSource = CFSTR("TISCategoryKeyboardInputSource");
const CFStringRef kTISPropertyInputSourceCategory = CFSTR("TISPropertyInputSourceCategory");
const CFStringRef kTISPropertyInputSourceType = CFSTR("TISPropertyInputSourceType");
const CFStringRef kTISTypeKeyboardLayout = CFSTR("TISTypeKeyboardLayout");

const float kHIToolboxVersionNumber = 1163.0;

/* Secure event input (password fields). Osxie never enables it; report that
   secure input is off so apps (e.g. iTerm2) don't defer keystrokes. */
Boolean IsSecureEventInputEnabled(void)
{
	return false;
}

OSStatus EnableSecureEventInput(void)
{
	return noErr;
}

OSStatus DisableSecureEventInput(void)
{
	return noErr;
}
