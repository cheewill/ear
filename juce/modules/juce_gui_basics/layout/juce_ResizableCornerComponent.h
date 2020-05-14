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

namespace juce
{

//==============================================================================
/** A component that resizes a parent component when dragged.

    This is the small triangular stripey resizer component you get in the bottom-right
    of windows (more commonly on the Mac than Windows). Put one in the corner of
    a larger component and it will automatically resize its parent when it gets dragged
    around.

    @see ResizableBorderComponent

    @tags{GUI}
*/
class JUCE_API  ResizableCornerComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a resizer.

        Pass in the target component which you want to be resized when this one is
        dragged.

        The target component will usually be a parent of the resizer component, but this
        isn't mandatory.

        Remember that when the target component is resized, it'll need to move and
        resize this component to keep it in place, as this won't happen automatically.

        If a constrainer object is provided, then this object will be used to enforce
        limits on the size and position that the component can be stretched to. Make sure
        that the constrainer isn't deleted while still in use by this object. If you
        pass a nullptr in here, no limits will be put on the sizes it can be stretched to.

        @see ComponentBoundsConstrainer
    */
    ResizableCornerComponent (Component* componentToResize,
                              ComponentBoundsConstrainer* constrainer);

    /** Destructor. */
    ~ResizableCornerComponent() override;


protected:
    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    bool hitTest (int x, int y) override;

private:
    //==============================================================================
    WeakReference<Component> component;
    ComponentBoundsConstrainer* constrainer;
    Rectangle<int> originalBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizableCornerComponent)
};

} // namespace juce