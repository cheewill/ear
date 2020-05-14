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
namespace dsp
{

template <typename FloatType>
static FloatType ncos (size_t order, size_t i, size_t size) noexcept
{
    return std::cos (static_cast<FloatType> (order * i)
                      * MathConstants<FloatType>::pi / static_cast<FloatType> (size - 1));
}

template <typename FloatType>
WindowingFunction<FloatType>::WindowingFunction (size_t size, WindowingMethod type, bool normalise, FloatType beta)
{
    fillWindowingTables (size, type, normalise, beta);
}

template <typename FloatType>
void WindowingFunction<FloatType>::fillWindowingTables (size_t size, WindowingMethod type,
                                                        bool normalise, FloatType beta) noexcept
{
    windowTable.resize (static_cast<int> (size));
    fillWindowingTables (windowTable.getRawDataPointer(), size, type, normalise, beta);
}

template <typename FloatType>
void WindowingFunction<FloatType>::fillWindowingTables (FloatType* samples, size_t size,
                                                        WindowingMethod type, bool normalise,
                                                        FloatType beta) noexcept
{
    switch (type)
    {
        case rectangular:
        {
            for (size_t i = 0; i < size; ++i)
                samples[i] = static_cast<FloatType> (1);
        }
        break;

        case triangular:
        {
            auto halfSlots = static_cast<FloatType> (0.5) * static_cast<FloatType> (size - 1);

            for (size_t i = 0; i < size; ++i)
                samples[i] = static_cast<FloatType> (1.0) - std::abs ((static_cast<FloatType> (i) - halfSlots) / halfSlots);
        }
        break;

        case hann:
        {
            for (size_t i = 0; i < size; ++i)
            {
                auto cos2 = ncos<FloatType> (2, i, size);
                samples[i] = static_cast<FloatType> (0.5 - 0.5 * cos2);
            }
        }
        break;

        case hamming:
        {
            for (size_t i = 0; i < size; ++i)
            {
                auto cos2 = ncos<FloatType> (2, i, size);
                samples[i] = static_cast<FloatType> (0.54 - 0.46 * cos2);
            }
        }
        break;

        case blackman:
        {
            constexpr FloatType alpha = 0.16f;

            for (size_t i = 0; i < size; ++i)
            {
                auto cos2 = ncos<FloatType> (2, i, size);
                auto cos4 = ncos<FloatType> (4, i, size);

                samples[i] = static_cast<FloatType> (0.5 * (1 - alpha) - 0.5 * cos2 + 0.5 * alpha * cos4);
            }
        }
        break;

        case blackmanHarris:
        {
            for (size_t i = 0; i < size; ++i)
            {
                auto cos2 = ncos<FloatType> (2, i, size);
                auto cos4 = ncos<FloatType> (4, i, size);
                auto cos6 = ncos<FloatType> (6, i, size);

                samples[i] = static_cast<FloatType> (0.35875 - 0.48829 * cos2 + 0.14128 * cos4 - 0.01168 * cos6);
            }
        }
        break;

        case flatTop:
        {
            for (size_t i = 0; i < size; ++i)
            {
                auto cos2 = ncos<FloatType> (2, i, size);
                auto cos4 = ncos<FloatType> (4, i, size);
                auto cos6 = ncos<FloatType> (6, i, size);
                auto cos8 = ncos<FloatType> (8, i, size);

                samples[i] = static_cast<FloatType> (1.0 - 1.93 * cos2 + 1.29 * cos4 - 0.388 * cos6 + 0.028 * cos8);
            }
        }
        break;

        case kaiser:
        {
            const double factor = 1.0 / SpecialFunctions::besselI0 (beta);

            for (size_t i = 0; i < size; ++i)
                samples[i] = static_cast<FloatType> (SpecialFunctions::besselI0 (beta * std::sqrt (1.0 - std::pow ((i - 0.5 * (size - 1.0))
                                                                                                                     / ( 0.5 * (size - 1.0)), 2.0)))
                                                      * factor);
        }
        break;

        case numWindowingMethods:
        default:
            jassertfalse;
            break;
    }

    // DC frequency amplitude must be one
    if (normalise)
    {
        FloatType sum (0);

        for (size_t i = 0; i < size; ++i)
            sum += samples[i];

        auto factor = static_cast<FloatType> (size) / sum;

        FloatVectorOperations::multiply (samples, factor, static_cast<int> (size));
    }
}

template <typename FloatType>
void WindowingFunction<FloatType>::multiplyWithWindowingTable (FloatType* samples, size_t size) noexcept
{
    FloatVectorOperations::multiply (samples, windowTable.getRawDataPointer(), jmin (static_cast<int> (size), windowTable.size()));
}

template <typename FloatType>
const char* WindowingFunction<FloatType>::getWindowingMethodName (WindowingMethod type) noexcept
{
    switch (type)
    {
        case rectangular:          return "Rectangular";
        case triangular:           return "Triangular";
        case hann:                 return "Hann";
        case hamming:              return "Hamming";
        case blackman:             return "Blackman";
        case blackmanHarris:       return "Blackman-Harris";
        case flatTop:              return "Flat Top";
        case kaiser:               return "Kaiser";
        case numWindowingMethods:
        default: jassertfalse;     return "";
    }
}

template class WindowingFunction<float>;
template class WindowingFunction<double>;

} // namespace dsp
} // namespace juce