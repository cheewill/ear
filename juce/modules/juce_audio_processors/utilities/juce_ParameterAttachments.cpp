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

ParameterAttachment::ParameterAttachment (RangedAudioParameter& param,
                                          std::function<void (float)> parameterChangedCallback,
                                          UndoManager* um)
    : parameter (param),
      undoManager (um),
      setValue (std::move (parameterChangedCallback))
{
    parameter.addListener (this);
}

ParameterAttachment::~ParameterAttachment()
{
    parameter.removeListener (this);
    cancelPendingUpdate();
}

void ParameterAttachment::sendInitialUpdate()
{
    parameterValueChanged ({}, parameter.getValue());
}

void ParameterAttachment::setValueAsCompleteGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (float f)
    {
        beginGesture();
        parameter.setValueNotifyingHost (f);
        endGesture();
    });
}

void ParameterAttachment::beginGesture()
{
    if (undoManager != nullptr)
        undoManager->beginNewTransaction();

    parameter.beginChangeGesture();
}

void ParameterAttachment::setValueAsPartOfGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (float f)
    {
        parameter.setValueNotifyingHost (f);
    });
}

void ParameterAttachment::endGesture()
{
    parameter.endChangeGesture();
}

template <typename Callback>
void ParameterAttachment::callIfParameterValueChanged (float newDenormalisedValue,
                                                       Callback&& callback)
{
    const auto newValue = normalise (newDenormalisedValue);

    if (parameter.getValue() != newValue)
        callback (newValue);
}

void ParameterAttachment::parameterValueChanged (int, float newValue)
{
    lastValue = newValue;

    if (MessageManager::getInstance()->isThisTheMessageThread())
    {
        cancelPendingUpdate();
        handleAsyncUpdate();
    }
    else
    {
        triggerAsyncUpdate();
    }
}

void ParameterAttachment::handleAsyncUpdate()
{
    if (setValue != nullptr)
        setValue (parameter.convertFrom0to1 (lastValue));
}

//==============================================================================
SliderParameterAttachment::SliderParameterAttachment (RangedAudioParameter& param,
                                                      Slider& s,
                                                      UndoManager* um)
    : slider (s),
      attachment (param, [this] (float f) { setValue (f); }, um)
{
    slider.valueFromTextFunction = [&param] (const String& text) { return (double) param.convertFrom0to1 (param.getValueForText (text)); };
    slider.textFromValueFunction = [&param] (double value) { return param.getText (param.convertTo0to1 ((float) value), 0); };
    slider.setDoubleClickReturnValue (true, param.convertFrom0to1 (param.getDefaultValue()));

    auto range = param.getNormalisableRange();

    auto convertFrom0To1Function = [range] (double currentRangeStart,
                                            double currentRangeEnd,
                                            double normalisedValue) mutable
    {
        range.start = (float) currentRangeStart;
        range.end = (float) currentRangeEnd;
        return (double) range.convertFrom0to1 ((float) normalisedValue);
    };

    auto convertTo0To1Function = [range] (double currentRangeStart,
                                          double currentRangeEnd,
                                          double mappedValue) mutable
    {
        range.start = (float) currentRangeStart;
        range.end = (float) currentRangeEnd;
        return (double) range.convertTo0to1 ((float) mappedValue);
    };

    auto snapToLegalValueFunction = [range] (double currentRangeStart,
                                             double currentRangeEnd,
                                             double mappedValue) mutable
    {
        range.start = (float) currentRangeStart;
        range.end = (float) currentRangeEnd;
        return (double) range.snapToLegalValue ((float) mappedValue);
    };

    NormalisableRange<double> newRange { (double) range.start,
                                         (double) range.end,
                                         std::move (convertFrom0To1Function),
                                         std::move (convertTo0To1Function),
                                         std::move (snapToLegalValueFunction) };
    newRange.interval = range.interval;
    newRange.skew = range.skew;
    newRange.symmetricSkew = range.symmetricSkew;

    slider.setNormalisableRange (newRange);

    sendInitialUpdate();
    slider.valueChanged();
    slider.addListener (this);
}

SliderParameterAttachment::~SliderParameterAttachment()
{
    slider.removeListener (this);
}

void SliderParameterAttachment::sendInitialUpdate() { attachment.sendInitialUpdate(); }

void SliderParameterAttachment::setValue (float newValue)
{
    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    slider.setValue (newValue, sendNotificationSync);
}

void SliderParameterAttachment::sliderValueChanged (Slider*)
{
    if (ignoreCallbacks || ModifierKeys::currentModifiers.isRightButtonDown())
        return;

    attachment.setValueAsPartOfGesture ((float) slider.getValue());
}

//==============================================================================
ComboBoxParameterAttachment::ComboBoxParameterAttachment (RangedAudioParameter& param,
                                                          ComboBox& c,
                                                          UndoManager* um)
    : comboBox (c),
      attachment (param, [this] (float f) { setValue (f); }, um)
{
    sendInitialUpdate();
    comboBox.addListener (this);
}

ComboBoxParameterAttachment::~ComboBoxParameterAttachment()
{
    comboBox.removeListener (this);
}

void ComboBoxParameterAttachment::sendInitialUpdate()
{
    attachment.sendInitialUpdate();
}

void ComboBoxParameterAttachment::setValue (float newValue)
{
    const auto index = roundToInt (newValue);

    if (index == comboBox.getSelectedItemIndex())
        return;

    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    comboBox.setSelectedItemIndex (index, sendNotificationSync);
}

void ComboBoxParameterAttachment::comboBoxChanged (ComboBox*)
{
    if (ignoreCallbacks)
        return;

    attachment.setValueAsCompleteGesture ((float) comboBox.getSelectedItemIndex());
}

//==============================================================================
ButtonParameterAttachment::ButtonParameterAttachment (RangedAudioParameter& param,
                                                      Button& b,
                                                      UndoManager* um)
    : button (b),
      attachment (param, [this] (float f) { setValue (f); }, um)
{
    sendInitialUpdate();
    button.addListener (this);
}

ButtonParameterAttachment::~ButtonParameterAttachment()
{
    button.removeListener (this);
}

void ButtonParameterAttachment::sendInitialUpdate()
{
    attachment.sendInitialUpdate();
}

void ButtonParameterAttachment::setValue (float newValue)
{
    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    button.setToggleState (newValue >= 0.5f, sendNotificationSync);
}

void ButtonParameterAttachment::buttonClicked (Button*)
{
    if (ignoreCallbacks)
        return;

    attachment.setValueAsCompleteGesture (button.getToggleState() ? 1.0f : 0.0f);
}

} // namespace juce
