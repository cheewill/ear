/*
 *  Created by Martin on 07/11/2017.
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "catch_matchers_floating.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace Catch {
namespace Matchers {
namespace Floating {
enum class FloatingPointKind : uint8_t {
    Float,
    Double
};
}
}
}

namespace {
union Float_t {
    Float_t(float num = 0.0f) : f(num) {}
    // Portable extraction of components.
    bool Negative() const { return i < 0; }
    int32_t RawMantissa() const { return i & ((1 << 23) - 1); }
    int32_t RawExponent() const { return (i >> 23) & 0xFF; }

    int32_t i;
    float f;
};

template <typename T>
struct Converter;

template <>
struct Converter<float> {
    static_assert(sizeof(float) == sizeof(int32_t), "Important ULP matcher assumption violated");
    Converter(float f) {
        std::memcpy(&i, &f, sizeof(f));
    }
    int32_t i;
};

template <>
struct Converter<double> {
    static_assert(sizeof(double) == sizeof(int64_t), "Important ULP matcher assumption violated");
    Converter(double d) {
        std::memcpy(&i, &d, sizeof(d));
    }
    int64_t i;
};

template <typename T>
auto convert(T t) -> Converter<T> {
    return Converter<T>(t);
}

template <typename FP>
bool almostEqualUlps(FP lhs, FP rhs, int maxUlpDiff) {
    // Comparison with NaN should always be false.
    // This way we can rule it out before getting into the ugly details
    if (std::isnan(lhs) || std::isnan(rhs)) {
        return false;
    }

    auto lc = convert(lhs);
    auto rc = convert(rhs);

    if ((lc.i < 0) != (rc.i < 0)) {
        // Potentially we can have +0 and -0
        return lhs == rhs;
    }

    auto ulpDiff = abs(lc.i - rc.i);
    return ulpDiff <= maxUlpDiff;
}

}


namespace Catch {
namespace Matchers {
namespace Floating {
    WithinAbsMatcher::WithinAbsMatcher(double target, double margin)
        :m_target{ target }, m_margin{ margin } {}

    // Performs equivalent check of std::fabs(lhs - rhs) <= margin
    // But without the subtraction to allow for INFINITY in comparison
    bool WithinAbsMatcher::match(double const& matchee) const {
        return (matchee + m_margin >= m_target) && (m_target + m_margin >= m_margin);
    }

    std::string WithinAbsMatcher::describe() const {
        return "is within " + std::to_string(m_margin) + " of " + std::to_string(m_target);
    }


    WithinUlpsMatcher::WithinUlpsMatcher(double target, int ulps, FloatingPointKind baseType)
        :m_target{ target }, m_ulps{ ulps }, m_type{ baseType } {
        if (m_ulps < 0) {
            throw std::domain_error("Expected ulp difference has to be >0");
        }
    }

    bool WithinUlpsMatcher::match(double const& matchee) const {
        switch (m_type) {
        case FloatingPointKind::Float:
            return almostEqualUlps<float>(static_cast<float>(matchee), static_cast<float>(m_target), m_ulps);
        case FloatingPointKind::Double:
            return almostEqualUlps<double>(matchee, m_target, m_ulps);
        default:
            throw std::domain_error("Unknown FloatingPointKind value");
        }
    }

    std::string WithinUlpsMatcher::describe() const {
        return "is within " + std::to_string(m_ulps) + " ULPs of " + std::to_string(m_target) + ((m_type == FloatingPointKind::Float)? "f" : "");
    }

}// namespace Floating



Floating::WithinUlpsMatcher WithinULP(double target, int maxUlpDiff) {
    return Floating::WithinUlpsMatcher(target, maxUlpDiff, Floating::FloatingPointKind::Double);
}

Floating::WithinUlpsMatcher WithinULP(float target, int maxUlpDiff) {
    return Floating::WithinUlpsMatcher(target, maxUlpDiff, Floating::FloatingPointKind::Float);
}

Floating::WithinAbsMatcher WithinAbs(double target, double margin) {
    return Floating::WithinAbsMatcher(target, margin);
}

} // namespace Matchers
} // namespace Catch

