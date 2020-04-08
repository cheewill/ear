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

#include "../../Application/jucer_Headers.h"
#include "jucer_JucerTreeViewBase.h"
#include "../../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
void TreePanelBase::setRoot (JucerTreeViewBase* root)
{
    rootItem.reset (root);
    tree.setRootItem (root);
    tree.getRootItem()->setOpen (true);

    if (project != nullptr)
    {
        if (auto treeOpenness = project->getStoredProperties().getXmlValue (opennessStateKey))
        {
            tree.restoreOpennessState (*treeOpenness, true);

            for (int i = tree.getNumSelectedItems(); --i >= 0;)
                if (auto item = dynamic_cast<JucerTreeViewBase*> (tree.getSelectedItem (i)))
                    item->cancelDelayedSelectionTimer();
        }
    }
}

void TreePanelBase::saveOpenness()
{
    if (project != nullptr)
    {
        std::unique_ptr<XmlElement> opennessState (tree.getOpennessState (true));

        if (opennessState != nullptr)
            project->getStoredProperties().setValue (opennessStateKey, opennessState.get());
        else
            project->getStoredProperties().removeValue (opennessStateKey);
    }
}

//==============================================================================
JucerTreeViewBase::JucerTreeViewBase()  : textX (0)
{
    setLinesDrawnForSubItems (false);
    setDrawsInLeftMargin (true);
}

JucerTreeViewBase::~JucerTreeViewBase()
{
}

void JucerTreeViewBase::refreshSubItems()
{
    WholeTreeOpennessRestorer wtor (*this);
    clearSubItems();
    addSubItems();
}

Font JucerTreeViewBase::getFont() const
{
    return Font (getItemHeight() * 0.6f);
}

void JucerTreeViewBase::paintOpenCloseButton (Graphics& g, const Rectangle<float>& area, Colour /*backgroundColour*/, bool isMouseOver)
{
    g.setColour (getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId : treeIconColourId));
    TreeViewItem::paintOpenCloseButton (g, area, getOwnerView()->findColour (defaultIconColourId), isMouseOver);
}

void JucerTreeViewBase::paintIcon (Graphics& g, Rectangle<float> area)
{
    g.setColour (getContentColour (true));
    getIcon().draw (g, area, isIconCrossedOut());
    textX = roundToInt (area.getRight()) + 7;
}

void JucerTreeViewBase::paintItem (Graphics& g, int width, int height)
{
    ignoreUnused (width, height);

    auto bounds = g.getClipBounds().withY (0).withHeight (height).toFloat();

    g.setColour (getOwnerView()->findColour (treeIconColourId).withMultipliedAlpha (0.4f));
    g.fillRect (bounds.removeFromBottom (0.5f).reduced (5, 0));
}

Colour JucerTreeViewBase::getContentColour (bool isIcon) const
{
    if (isMissing())      return Colours::red;
    if (isSelected())     return getOwnerView()->findColour (defaultHighlightedTextColourId);
    if (hasWarnings())    return getOwnerView()->findColour (defaultHighlightColourId);

    return getOwnerView()->findColour (isIcon ? treeIconColourId : defaultTextColourId);
}

void JucerTreeViewBase::paintContent (Graphics& g, Rectangle<int> area)
{
    g.setFont (getFont());
    g.setColour (getContentColour (false));

    g.drawFittedText (getDisplayName(), area, Justification::centredLeft, 1, 1.0f);
}

Component* JucerTreeViewBase::createItemComponent()
{
    return new TreeItemComponent (*this);
}

//==============================================================================
class RenameTreeItemCallback  : public ModalComponentManager::Callback
{
public:
    RenameTreeItemCallback (JucerTreeViewBase& ti, Component& parent, const Rectangle<int>& bounds)
        : item (ti)
    {
        ed.setMultiLine (false, false);
        ed.setPopupMenuEnabled (false);
        ed.setSelectAllWhenFocused (true);
        ed.setFont (item.getFont());
        ed.onReturnKey = [this] { ed.exitModalState (1); };
        ed.onEscapeKey = [this] { ed.exitModalState (0); };
        ed.onFocusLost = [this] { ed.exitModalState (0); };
        ed.setText (item.getRenamingName());
        ed.setBounds (bounds);

        parent.addAndMakeVisible (ed);
        ed.enterModalState (true, this);
    }

    void modalStateFinished (int resultCode) override
    {
        if (resultCode != 0)
            item.setName (ed.getText());
    }

private:
    struct RenameEditor   : public TextEditor
    {
        void inputAttemptWhenModal() override   { exitModalState (0); }
    };

    RenameEditor ed;
    JucerTreeViewBase& item;

    JUCE_DECLARE_NON_COPYABLE (RenameTreeItemCallback)
};

void JucerTreeViewBase::showRenameBox()
{
    Rectangle<int> r (getItemPosition (true));
    r.setLeft (r.getX() + textX);
    r.setHeight (getItemHeight());

    new RenameTreeItemCallback (*this, *getOwnerView(), r);
}

void JucerTreeViewBase::itemClicked (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        if (getOwnerView()->getNumSelectedItems() > 1)
            showMultiSelectionPopupMenu();
        else
            showPopupMenu();
    }
    else if (isSelected())
    {
        itemSelectionChanged (true);
    }
}

void JucerTreeViewBase::deleteItem()    {}
void JucerTreeViewBase::deleteAllSelectedItems() {}
void JucerTreeViewBase::showDocument()  {}
void JucerTreeViewBase::showPopupMenu() {}
void JucerTreeViewBase::showAddMenu()  {}
void JucerTreeViewBase::showMultiSelectionPopupMenu() {}

static void treeViewMenuItemChosen (int resultCode, WeakReference<JucerTreeViewBase> item)
{
    if (item != nullptr)
        item->handlePopupMenuResult (resultCode);
}

void JucerTreeViewBase::launchPopupMenu (PopupMenu& m)
{
    m.showMenuAsync (PopupMenu::Options(),
                     ModalCallbackFunction::create (treeViewMenuItemChosen, WeakReference<JucerTreeViewBase> (this)));
}

void JucerTreeViewBase::handlePopupMenuResult (int)
{
}

ProjectContentComponent* JucerTreeViewBase::getProjectContentComponent() const
{
    for (Component* c = getOwnerView(); c != nullptr; c = c->getParentComponent())
        if (ProjectContentComponent* pcc = dynamic_cast<ProjectContentComponent*> (c))
            return pcc;

    return nullptr;
}

//==============================================================================
class JucerTreeViewBase::ItemSelectionTimer  : public Timer
{
public:
    ItemSelectionTimer (JucerTreeViewBase& tvb)  : owner (tvb) {}

    void timerCallback() override    { owner.invokeShowDocument(); }

private:
    JucerTreeViewBase& owner;
    JUCE_DECLARE_NON_COPYABLE (ItemSelectionTimer)
};

void JucerTreeViewBase::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
    {
        delayedSelectionTimer.reset (new ItemSelectionTimer (*this));
        delayedSelectionTimer->startTimer (getMillisecsAllowedForDragGesture());
    }
    else
    {
        cancelDelayedSelectionTimer();
    }
}

void JucerTreeViewBase::invokeShowDocument()
{
    cancelDelayedSelectionTimer();
    showDocument();
}

void JucerTreeViewBase::itemDoubleClicked (const MouseEvent&)
{
    invokeShowDocument();
}

void JucerTreeViewBase::cancelDelayedSelectionTimer()
{
    delayedSelectionTimer.reset();
}
