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

#pragma once


//==============================================================================
namespace CodeHelpers
{
    String indent (const String& code, int numSpaces, bool indentFirstLine);
    String unindent (const String& code, int numSpaces);

    String createIncludeStatement (const File& includedFile, const File& targetFile);
    String createIncludeStatement (const String& includePath);
    String createIncludePathIncludeStatement (const String& includedFilename);

    String stringLiteral (const String& text, int maxLineLength = -1);
    String floatLiteral (double value, int numDecPlaces);
    String boolLiteral (bool value);

    String colourToCode (Colour);
    String justificationToCode (Justification);

    String alignFunctionCallParams (const String& call, const StringArray& parameters, int maxLineLength);

    String getLeadingWhitespace (String line);
    int getBraceCount (String::CharPointerType line);
    bool getIndentForCurrentBlock (CodeDocument::Position pos, const String& tab,
                                   String& blockIndent, String& lastLineIndent);
}
