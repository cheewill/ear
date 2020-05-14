/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Project/jucer_Project.h"
class JucerDocument;

//==============================================================================
/**
    A class for collecting the various snippets of c++ that will be assembled into
    the final cpp and header files.
*/
class GeneratedCode
{
public:
    GeneratedCode (const JucerDocument*);
    ~GeneratedCode();

    //==============================================================================
    void applyToCode (String& code, const File& targetFile, const String& oldFileWithUserData) const;
    int getUniqueSuffix();

    //==============================================================================
    const JucerDocument* const document;

    String className;
    String componentName;
    String parentClassInitialiser;  // optional parent class initialiser to go before the items in the initialisers list
    StringArray initialisers; // (a list of the member variables that need initialising after the constructor declaration)
    String parentClasses;
    String constructorParams;
    String privateMemberDeclarations;
    String publicMemberDeclarations;
    Array<File> includeFilesH, includeFilesCPP;
    String constructorCode;
    String destructorCode;
    String staticMemberDefinitions;
    String jucerMetadata;

    struct CallbackMethod
    {
        String requiredParentClass;
        String returnType;
        String prototype;
        String content;
        bool hasPrePostUserSections;
    };

    OwnedArray<CallbackMethod> callbacks;

    String& getCallbackCode (const String& requiredParentClass,
                             const String& returnType,
                             const String& prototype,
                             const bool hasPrePostUserSections);

    void removeCallback (const String& returnType, const String& prototype);

    void addImageResourceLoader (const String& imageMemberName, const String& resourceName);

    String getCallbackDeclarations() const;
    String getCallbackDefinitions() const;
    StringArray getExtraParentClasses() const;

    bool shouldUseTransMacro() const noexcept;

private:
    String getClassDeclaration() const;
    String getInitialiserList() const;
    int suffix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratedCode)
};