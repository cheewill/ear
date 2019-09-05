/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2017 - ROLI Ltd.

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JUCE_WINDOWS
 #undef _WIN32_WINNT
 #define _WIN32_WINNT 0x500
 #undef STRICT
 #define STRICT 1
 #include <windows.h>
 #include <float.h>
 #if JUCE_MSVC
  #pragma warning (disable : 4312 4355)
 #endif
 #ifdef __INTEL_COMPILER
  #pragma warning (disable : 1899)
 #endif
#elif JUCE_LINUX
 #include <float.h>
 #include <sys/time.h>
#elif JUCE_MAC || JUCE_IOS
 #if ! (defined (JUCE_SUPPORT_CARBON) || defined (__LP64__))
  #define JUCE_SUPPORT_CARBON 1
 #endif

 #ifdef __OBJC__
  #if JUCE_MAC
   #include <Cocoa/Cocoa.h>
  #elif JUCE_IOS
   #include <UIKit/UIKit.h>
  #else
   #error
  #endif
 #endif

 #if JUCE_SUPPORT_CARBON && (! JUCE_IOS)
  #include <Carbon/Carbon.h>
 #endif

 #include <objc/runtime.h>
 #include <objc/objc.h>
 #include <objc/message.h>
#endif
