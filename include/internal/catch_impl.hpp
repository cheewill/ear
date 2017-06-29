/*
 *  Created by Phil on 5/8/2012.
 *  Copyright 2012 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef TWOBLUECUBES_CATCH_IMPL_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_IMPL_HPP_INCLUDED

// Collect all the implementation files together here
// These are the equivalent of what would usually be cpp files

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
#endif

#include "../catch_session.hpp"
#include "catch_registry_hub.hpp"
#include "catch_notimplemented_exception.hpp"
#include "catch_context_impl.hpp"
#include "catch_console_colour_impl.hpp"
#include "catch_assertionresult.hpp"
#include "catch_test_case_info.hpp"
#include "catch_test_spec.hpp"
#include "catch_version.hpp"
#include "catch_message.hpp"
#include "catch_timer.hpp"
#include "catch_common.hpp"
#include "catch_section.hpp"
#include "catch_debugger.hpp"
#include "catch_tostring.hpp"
#include "catch_result_builder.hpp"
#include "catch_tag_alias_registry.hpp"
#include "catch_test_case_tracker.hpp"
#include "catch_matchers_string.hpp"
#include "catch_startup_exception_registry.hpp"

// These files are not included in the full (not single include) project
// as they are compiled as proper cpp files
#ifndef CATCH_CONFIG_FULL_PROJECT
#   include "catch_stringref.cpp"
#   include "catch_string.cpp"
#   include "catch_stringbuilder.cpp"
#   include "catch_stringdata.cpp"
#endif

#include "../reporters/catch_reporter_multi.hpp"
#include "../reporters/catch_reporter_xml.hpp"
#include "../reporters/catch_reporter_junit.hpp"
#include "../reporters/catch_reporter_console.hpp"
#include "../reporters/catch_reporter_compact.hpp"

namespace Catch {
    // These are all here to avoid warnings about not having any out of line
    // virtual methods
    NonCopyable::~NonCopyable() {}
    IStream::~IStream() noexcept {}
    FileStream::~FileStream() noexcept {}
    CoutStream::~CoutStream() noexcept {}
    DebugOutStream::~DebugOutStream() noexcept {}
    StreamBufBase::~StreamBufBase() noexcept {}
    IContext::~IContext() {}
    IResultCapture::~IResultCapture() {}
    ITestCase::~ITestCase() {}
    ITestCaseRegistry::~ITestCaseRegistry() {}
    IRegistryHub::~IRegistryHub() {}
    IMutableRegistryHub::~IMutableRegistryHub() {}
    IExceptionTranslator::~IExceptionTranslator() {}
    IExceptionTranslatorRegistry::~IExceptionTranslatorRegistry() {}
    IReporterFactory::~IReporterFactory() {}
    IReporterRegistry::~IReporterRegistry() {}
    IStreamingReporter::~IStreamingReporter() {}
    AssertionStats::~AssertionStats() {}
    SectionStats::~SectionStats() {}
    TestCaseStats::~TestCaseStats() {}
    TestGroupStats::~TestGroupStats() {}
    TestRunStats::~TestRunStats() {}
    CumulativeReporterBase::SectionNode::~SectionNode() {}
    CumulativeReporterBase::~CumulativeReporterBase() {}

    StreamingReporterBase::~StreamingReporterBase() {}
    ConsoleReporter::~ConsoleReporter() {}
    CompactReporter::~CompactReporter() {}
    IRunner::~IRunner() {}
    IMutableContext::~IMutableContext() {}
    IConfig::~IConfig() {}
    XmlReporter::~XmlReporter() {}
    JunitReporter::~JunitReporter() {}
    TestRegistry::~TestRegistry() {}
    FreeFunctionTestCase::~FreeFunctionTestCase() {}
    WildcardPattern::~WildcardPattern() {}
    TestSpec::Pattern::~Pattern() {}
    TestSpec::NamePattern::~NamePattern() {}
    TestSpec::TagPattern::~TagPattern() {}
    TestSpec::ExcludedPattern::~ExcludedPattern() {}
    Matchers::Impl::MatcherUntypedBase::~MatcherUntypedBase() {}

    void Config::dummy() {}

    namespace TestCaseTracking {
        ITracker::~ITracker() {}
        TrackerBase::~TrackerBase() {}
        SectionTracker::~SectionTracker() {}
        IndexTracker::~IndexTracker() {}
    }
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // TWOBLUECUBES_CATCH_IMPL_HPP_INCLUDED
