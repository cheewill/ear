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

namespace juce
{

DrawableRectangle::DrawableRectangle() {}
DrawableRectangle::~DrawableRectangle() {}

DrawableRectangle::DrawableRectangle (const DrawableRectangle& other)
    : DrawableShape (other),
      bounds (other.bounds),
      cornerSize (other.cornerSize)
{
    rebuildPath();
}

std::unique_ptr<Drawable> DrawableRectangle::createCopy() const
{
    return std::make_unique<DrawableRectangle> (*this);
}

//==============================================================================
void DrawableRectangle::setRectangle (Parallelogram<float> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        rebuildPath();
    }
}

void DrawableRectangle::setCornerSize (Point<float> newSize)
{
    if (cornerSize != newSize)
    {
        cornerSize = newSize;
        rebuildPath();
    }
}

void DrawableRectangle::rebuildPath()
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    Path newPath;

    if (cornerSize.x > 0 && cornerSize.y > 0)
        newPath.addRoundedRectangle (0, 0, w, h, cornerSize.x, cornerSize.y);
    else
        newPath.addRectangle (0, 0, w, h);

    newPath.applyTransform (AffineTransform::fromTargetPoints (Point<float>(),       bounds.topLeft,
                                                               Point<float> (w, 0),  bounds.topRight,
                                                               Point<float> (0, h),  bounds.bottomLeft));

    if (path != newPath)
    {
        path.swapWithPath (newPath);
        pathChanged();
    }
}

} // namespace juce
