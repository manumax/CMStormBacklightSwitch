/*
 *  MIT License
 *
 *  Copyright (c) 2016 Manuele Mion
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>

#define kCMStormDevastatorProductID 0x0001
#define kCMStormDevastatorVendorID 0x258a

static CFMutableDictionaryRef cm_CreateKeyboardMatchingCFDictRef()
{
    CFMutableDictionaryRef matchingCFDictRef = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                         0,
                                                                         &kCFTypeDictionaryKeyCallBacks,
                                                                         &kCFTypeDictionaryValueCallBacks);
    UInt32 productID = kCMStormDevastatorProductID;
    CFNumberRef productIDCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &productID);
    CFDictionarySetValue(matchingCFDictRef, CFSTR(kIOHIDProductIDKey), productIDCFNumberRef);
    CFRelease(productIDCFNumberRef);
    
    UInt32 vendorID = kCMStormDevastatorVendorID;
    CFNumberRef vendorIDCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendorID);
    CFDictionarySetValue(matchingCFDictRef, CFSTR(kIOHIDVendorIDKey), vendorIDCFNumberRef);
    CFRelease(vendorIDCFNumberRef);
    
    UInt32 usagePage = kHIDPage_GenericDesktop;
    CFNumberRef usagePageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usagePage);
    CFDictionarySetValue(matchingCFDictRef, CFSTR(kIOHIDDeviceUsagePageKey), usagePageCFNumberRef);
    CFRelease(usagePageCFNumberRef);
    
    UInt32 usage = kHIDUsage_GD_Keyboard;
    CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    CFDictionarySetValue(matchingCFDictRef, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
    CFRelease(usageCFNumberRef);
    
    return matchingCFDictRef;
}

static CFMutableDictionaryRef cm_CreateLEDsMatchingCFDictRef()
{
    CFMutableDictionaryRef matchingCFDictRef = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                         0,
                                                                         &kCFTypeDictionaryKeyCallBacks,
                                                                         &kCFTypeDictionaryValueCallBacks);
    
    UInt32 usagePage = kHIDPage_LEDs;
    CFNumberRef usagePageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usagePage);
    CFDictionarySetValue(matchingCFDictRef, CFSTR(kIOHIDElementUsagePageKey), usagePageCFNumberRef);
    CFRelease(usagePageCFNumberRef);
    
    return matchingCFDictRef;
}

static int cm_SwitchKeyboardBacklight(bool switchOn)
{
    IOHIDManagerRef tIOHIDManagerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    
    CFMutableDictionaryRef tCMStormDevastatorMatchingCFDictRef = cm_CreateKeyboardMatchingCFDictRef();
    IOHIDManagerSetDeviceMatching(tIOHIDManagerRef, tCMStormDevastatorMatchingCFDictRef);
    CFRelease(tCMStormDevastatorMatchingCFDictRef);
    
    /* IOReturn tIOReturn = */
    IOHIDManagerOpen(tIOHIDManagerRef, kIOHIDOptionsTypeNone);
    
    CFSetRef deviceCFSetRef = IOHIDManagerCopyDevices(tIOHIDManagerRef);
    CFIndex deviceIndex, deviceCount = CFSetGetCount(deviceCFSetRef);
    
    IOHIDDeviceRef *tIOHIDDeviceRefs = malloc(sizeof(IOHIDDeviceRef) * deviceCount);
    CFSetGetValues(deviceCFSetRef, (const void **)tIOHIDDeviceRefs );
    CFRelease(deviceCFSetRef);
    
    CFMutableDictionaryRef tLEDsMatchingCFDictRef = cm_CreateLEDsMatchingCFDictRef();
    for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
    {
        CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(tIOHIDDeviceRefs[deviceIndex],
                                                                       tLEDsMatchingCFDictRef,
                                                                       kIOHIDOptionsTypeNone);
        
        CFIndex elementIndex, elementCount = CFArrayGetCount(elementCFArrayRef);
        for (elementIndex = 0; elementIndex < elementCount; elementIndex++)
        {
            IOHIDElementRef tIOHIDElementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(elementCFArrayRef, elementIndex);
            
            CFIndex backlightModeCFIndex = (switchOn ? 0 : 1);
            IOHIDValueRef valueIOHIDValueRef;
            if (elementIndex != 2)
            {
                /* IOReturn tIOReturn = */
                IOHIDDeviceGetValue(tIOHIDDeviceRefs[deviceIndex], tIOHIDElementRef, &valueIOHIDValueRef);
            }
            else
            {
                valueIOHIDValueRef = IOHIDValueCreateWithIntegerValue(kCFAllocatorDefault, tIOHIDElementRef, 0, backlightModeCFIndex);
            }
            
            /* IOReturn tIOReturn = */
            IOHIDDeviceSetValue(tIOHIDDeviceRefs[deviceIndex], tIOHIDElementRef, valueIOHIDValueRef);
            CFRelease(valueIOHIDValueRef);
        }
        
        CFRelease(elementCFArrayRef);
    }
    
    CFRelease(tLEDsMatchingCFDictRef);
    free(tIOHIDDeviceRefs);
    CFRelease(tIOHIDManagerRef);
    
    return 0;
}

static void cm_ShowHelp()
{
    printf("A very simple tool to switch CMStorm Devastator keyboard backlight on/off on MacOS.\n");
    printf("\n");
    printf("Usage:\n");
    printf("  CMStormBacklightSwitch [On|Off]\n");
}

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        if (strcasecmp(argv[1], "on"))
        {
            return cm_SwitchKeyboardBacklight(true);
        }
        
        if (strcasecmp(argv[1], "off"))
        {
            return cm_SwitchKeyboardBacklight(false);
        }
    }
    
    cm_ShowHelp();
    return 0;
} /* main */
