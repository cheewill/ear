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
namespace build_tools
{
    String EntitlementOptions::getEntitlementsFileContent() const
    {
        String content =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n";

        const auto entitlements = getEntitlements();

        for (auto& key : entitlements.getAllKeys())
            content += "\t<key>" + key + "</key>\n\t" + entitlements[key] + "\n";

        return content + "</dict>\n</plist>\n";
    }

    StringPairArray EntitlementOptions::getEntitlements() const
    {
        StringPairArray entitlements;

        if (isiOS)
        {
            if (isAudioPluginProject && shouldEnableIAA)
                entitlements.set ("inter-app-audio", "<true/>");

            if (isiCloudPermissionsEnabled)
            {
                entitlements.set ("com.apple.developer.icloud-container-identifiers",
                                  "<array>\n"
                                  "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                                  "    </array>");

                entitlements.set ("com.apple.developer.icloud-services",
                                  "<array>\n"
                                  "        <string>CloudDocuments</string>\n"
                                  "    </array>");

                entitlements.set ("com.apple.developer.ubiquity-container-identifiers",
                                  "<array>\n"
                                  "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                                  "    </array>");
            }
        }

        if (isPushNotificationsEnabled)
            entitlements.set (isiOS ? "aps-environment"
                                    : "com.apple.developer.aps-environment",
                              "<string>development</string>");

        if (isAppGroupsEnabled)
        {
            auto appGroups = StringArray::fromTokens (appGroupIdString, ";", {});
            auto groups = String ("<array>");

            for (auto group : appGroups)
                groups += "\n\t\t<string>" + group.trim() + "</string>";

            groups += "\n\t</array>";

            entitlements.set ("com.apple.security.application-groups", groups);
        }

        if (isHardenedRuntimeEnabled)
            for (auto& option : hardenedRuntimeOptions)
                entitlements.set (option, "<true/>");

        if (isAppSandboxEnabled || (! isiOS && isAudioPluginProject && type == ProjectType::Target::AudioUnitv3PlugIn))
        {
            entitlements.set ("com.apple.security.app-sandbox", "<true/>");

            if (isAppSandboxInhertianceEnabled)
            {
                // no other sandbox options can be specified if sandbox inheritance is enabled!
                jassert (appSandboxOptions.isEmpty());

                entitlements.set ("com.apple.security.inherit", "<true/>");
            }

            if (isAppSandboxEnabled)
                for (auto& option : appSandboxOptions)
                    entitlements.set (option, "<true/>");
        }

        return entitlements;
    }
}
}
