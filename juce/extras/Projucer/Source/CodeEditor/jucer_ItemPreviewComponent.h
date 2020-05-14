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
class ItemPreviewComponent  : public Component
{
public:
    ItemPreviewComponent (const File& f)  : file (f)
    {
        setOpaque (true);
        tryToLoadImage();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));

        if (drawable != nullptr)
        {
            auto contentBounds = drawable->getDrawableBounds();

            if (auto* dc = dynamic_cast<DrawableComposite*> (drawable.get()))
            {
                auto r = dc->getContentArea();

                if (! r.isEmpty())
                    contentBounds = r;
            }

            auto area = RectanglePlacement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize)
                            .appliedTo (contentBounds, Rectangle<float> (4.0f, 22.0f, getWidth() - 8.0f, getHeight() - 26.0f));

            Path p;
            p.addRectangle (area);
            DropShadow (Colours::black.withAlpha (0.5f), 6, Point<int> (0, 1)).drawForPath (g, p);

            g.fillCheckerBoard (area, 24.0f, 24.0f, Colour (0xffffffff), Colour (0xffeeeeee));

            drawable->draw (g, 1.0f, RectanglePlacement (RectanglePlacement::stretchToFit)
                                        .getTransformToFit (contentBounds, area.toFloat()));
        }

        g.setFont (Font (14.0f, Font::bold));
        g.setColour (findColour (defaultTextColourId));
        g.drawMultiLineText (facts.joinIntoString ("\n"), 10, 15, getWidth() - 16);
    }

private:
    StringArray facts;
    File file;
    std::unique_ptr<Drawable> drawable;

    void tryToLoadImage()
    {
        facts.clear();
        facts.add (file.getFullPathName());
        drawable.reset();

        if (auto input = std::unique_ptr<FileInputStream> (file.createInputStream()))
        {
            auto totalSize = input->getTotalLength();
            String formatName;

            if (auto* format = ImageFileFormat::findImageFormatForStream (*input))
                formatName = " " + format->getFormatName();

            input.reset();

            auto image = ImageCache::getFromFile (file);

            if (image.isValid())
            {
                auto* d = new DrawableImage();
                d->setImage (image);
                drawable.reset (d);

                facts.add (String (image.getWidth()) + " x " + String (image.getHeight()) + formatName);
            }

            if (totalSize > 0)
                facts.add (File::descriptionOfSizeInBytes (totalSize));
        }

        if (drawable == nullptr)
            if (auto svg = parseXML (file))
                drawable = Drawable::createFromSVG (*svg);

        facts.removeEmptyStrings (true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemPreviewComponent)
};