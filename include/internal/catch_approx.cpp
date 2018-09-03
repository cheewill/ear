/*
 *  Created by Martin on 19/07/2017.
 *  Copyright 2017 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "catch_approx.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

// Performs equivalent check of std::fabs(lhs - rhs) <= margin
// But without the subtraction to allow for INFINITY in comparison
bool marginComparison(double lhs, double rhs, double margin) {
    return (lhs + margin >= rhs) && (rhs + margin >= lhs);
}

}

namespace Catch {
namespace Detail {

    Approx::Approx ( double value )
    :   m_epsilon( std::numeric_limits<float>::epsilon()*100 ),
        m_margin( 0.0 ),
        m_scale( 0.0 ),
        m_value( value )
    {}

    Approx Approx::custom() {
        return Approx( 0 );
    }

    Approx Approx::operator-() const {
        auto temp(*this);
        temp.m_value = -temp.m_value;
        return temp;
    }


    std::string Approx::toString() const {
        ReusableStringStream rss;
        rss << "Approx( " << ::Catch::Detail::stringify( m_value ) << " )";
        return rss.str();
    }

    bool Approx::equalityComparisonImpl(const double other) const {
        // First try with fixed margin, then compute margin based on epsilon, scale and Approx's value
        // Thanks to Richard Harris for his help refining the scaled margin value
        return marginComparison(m_value, other, m_margin) || marginComparison(m_value, other, m_epsilon * (m_scale + std::fabs(m_value)));
    }

    void Approx::setMargin(double margin) {
        if (margin < 0) {
            throw std::domain_error
            ("Invalid Approx::margin: " +
             Catch::Detail::stringify(margin) +
             ", Approx::Margin has to be non-negative.");

        }
        m_margin = margin;
    }

    void Approx::setEpsilon(double epsilon) {
        if (epsilon < 0 || epsilon > 1.0) {
            throw std::domain_error
            ("Invalid Approx::epsilon: " +
             Catch::Detail::stringify(epsilon) +
             ", Approx::epsilon has to be between 0 and 1");
        }
        m_epsilon = epsilon;
    }

} // end namespace Detail

namespace literals {
    Detail::Approx operator "" _a(long double val) {
        return Detail::Approx(val);
    }
    Detail::Approx operator "" _a(unsigned long long val) {
        return Detail::Approx(val);
    }
} // end namespace literals

std::string StringMaker<Catch::Detail::Approx>::convert(Catch::Detail::Approx const& value) {
    return value.toString();
}

} // end namespace Catch
