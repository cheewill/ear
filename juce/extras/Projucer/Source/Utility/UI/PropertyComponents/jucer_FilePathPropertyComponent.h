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


//==============================================================================
/** A PropertyComponent for selecting files or folders.

    The user may drag files over the property box, enter the path manually and/or click
    the '...' button to open a file selection dialog box.
*/
class FilePathPropertyComponent    : public PropertyComponent,
                                     public FileDragAndDropTarget,
                                     private Value::Listener
{
public:
    FilePathPropertyComponent (Value valueToControl, const String& propertyName, bool isDir, bool thisOS = true,
                               const String& wildcardsToUse = "*", const File& relativeRoot = File())
        : PropertyComponent (propertyName),
          text (valueToControl, propertyName, 1024, false),
          isDirectory (isDir), isThisOS (thisOS), wildcards (wildcardsToUse), root (relativeRoot)
    {
        textValue.referTo (valueToControl);
        init();
    }

    /** Displays a default value when no value is specified by the user. */
    FilePathPropertyComponent (ValueWithDefault& valueToControl, const String& propertyName, bool isDir, bool thisOS = true,
                               const String& wildcardsToUse = "*", const File& relativeRoot = File())
       : PropertyComponent (propertyName),
         text (valueToControl, propertyName, 1024, false),
         isDirectory (isDir), isThisOS (thisOS), wildcards (wildcardsToUse), root (relativeRoot)
    {
        textValue = valueToControl.getPropertyAsValue();
        init();
    }

    //==============================================================================
    void refresh() override {}

    void resized() override
    {
        auto bounds = getLocalBounds();

        text.setBounds (bounds.removeFromLeft (jmax (400, bounds.getWidth() - 55)));
        bounds.removeFromLeft (5);
        browseButton.setBounds (bounds);
    }

    void paintOverChildren (Graphics& g) override
    {
        if (highlightForDragAndDrop)
        {
            g.setColour (findColour (defaultHighlightColourId).withAlpha (0.5f));
            g.fillRect (getLookAndFeel().getPropertyComponentContentPosition (text));
        }
    }

    //==============================================================================
    bool isInterestedInFileDrag (const StringArray&) override     { return true; }
    void fileDragEnter (const StringArray&, int, int) override    { highlightForDragAndDrop = true;  repaint(); }
    void fileDragExit (const StringArray&) override               { highlightForDragAndDrop = false; repaint(); }

    void filesDropped (const StringArray& selectedFiles, int, int) override
    {
        setTo (selectedFiles[0]);

        highlightForDragAndDrop = false;
        repaint();
    }

private:
    //==============================================================================
    void init()
    {
        textValue.addListener (this);

        text.setInterestedInFileDrag (false);
        addAndMakeVisible (text);

        browseButton.onClick = [this] { browse(); };
        addAndMakeVisible (browseButton);

        lookAndFeelChanged();
    }

    void setTo (File f)
    {
        if (isDirectory && ! f.isDirectory())
            f = f.getParentDirectory();

        auto pathName = (root == File()) ? f.getFullPathName()
                                         : f.getRelativePathFrom (root);

        text.setText (pathName);
        updateEditorColour();
    }

    void browse()
    {
        File currentFile = {};

        if (text.getText().isNotEmpty())
            currentFile = root.getChildFile (text.getText());

        if (isDirectory)
        {
            FileChooser chooser ("Select directory", currentFile);

            if (chooser.browseForDirectory())
                setTo (chooser.getResult());
        }
        else
        {
            FileChooser chooser ("Select file", currentFile, wildcards);

            if (chooser.browseForFileToOpen())
                setTo (chooser.getResult());
        }
    }

    void updateEditorColour()
    {
        if (isThisOS)
        {
            text.setColour (TextPropertyComponent::textColourId, findColour (widgetTextColourId));

            auto pathToCheck = text.getText();

            if (pathToCheck.isNotEmpty())
            {
                pathToCheck.replace ("${user.home}", "~");

               #if JUCE_WINDOWS
                if (pathToCheck.startsWith ("~"))
                    pathToCheck = pathToCheck.replace ("~", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
               #endif

                if (! root.getChildFile (pathToCheck).exists())
                    text.setColour (TextPropertyComponent::textColourId, Colours::red);
            }
        }
    }

    void valueChanged (Value&) override
    {
        updateEditorColour();
    }

    void lookAndFeelChanged() override
    {
        browseButton.setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
        browseButton.setColour (TextButton::textColourOffId, Colours::white);

        updateEditorColour();
    }

    //==============================================================================
    Value textValue;

    TextPropertyComponent text;
    TextButton browseButton { "..." };

    bool isDirectory, isThisOS, highlightForDragAndDrop = false;
    String wildcards;
    File root;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePathPropertyComponent)
};