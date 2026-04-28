// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaDeviceProfileSelectorModule.h"

#include "HAL/PlatformProperties.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

IMPLEMENT_MODULE(FCarlaDeviceProfileSelectorModule, CarlaDeviceProfileSelector);

void FCarlaDeviceProfileSelectorModule::StartupModule()
{
}

void FCarlaDeviceProfileSelectorModule::ShutdownModule()
{
}

namespace
{

// Append our tier-specific exec commands to the cached command line. If the
// user already passed -execcmds=..., merge our commands with theirs in a
// single -execcmds= token (UE's FParse::Value returns the FIRST match and
// stops, so two separate -execcmds= tokens would silently drop the second).
// User-supplied commands win in eval order: ours run first, theirs run last,
// so any CVar a user explicitly sets via -execcmds= still ends with their
// chosen value.
void AppendOrMergeExecCmds(const FString &TierExecCmds)
{
  if (TierExecCmds.IsEmpty())
  {
    return;
  }

  const FString OriginalCommandLine = FCommandLine::Get();

  FString ExistingValue;
  if (!FParse::Value(*OriginalCommandLine, TEXT("-execcmds="), ExistingValue))
  {
    // No user-supplied -execcmds=; append a fresh one.
    const FString Appendage = FString::Printf(TEXT(" -execcmds=\"%s\""), *TierExecCmds);
    FCommandLine::Append(*Appendage);
    return;
  }

  // Locate the existing -execcmds= token in the command line so we can
  // splice our commands into the same value rather than add a duplicate
  // token UE would ignore.
  const FString TokenKey = TEXT("-execcmds=");
  const int32 TokenStart = OriginalCommandLine.Find(TokenKey, ESearchCase::IgnoreCase);
  if (TokenStart == INDEX_NONE)
  {
    // Defensive: FParse::Value matched but Find did not (case mismatch
    // unlikely with IgnoreCase). Fall back to appending a separate token --
    // not ideal but better than dropping the tier commands entirely.
    const FString Appendage = FString::Printf(TEXT(" -execcmds=\"%s\""), *TierExecCmds);
    FCommandLine::Append(*Appendage);
    return;
  }

  const int32 ValueStart = TokenStart + TokenKey.Len();
  int32 TokenEnd = OriginalCommandLine.Len();
  if (ValueStart < OriginalCommandLine.Len() && OriginalCommandLine[ValueStart] == TEXT('"'))
  {
    // Quoted form: -execcmds="cmd1,cmd2". End is the matching close quote.
    const int32 CloseQuote = OriginalCommandLine.Find(
        TEXT("\""),
        ESearchCase::IgnoreCase,
        ESearchDir::FromStart,
        ValueStart + 1);
    TokenEnd = (CloseQuote != INDEX_NONE) ? (CloseQuote + 1) : OriginalCommandLine.Len();
  }
  else
  {
    // Unquoted form: -execcmds=cmd1,cmd2. End is the next whitespace.
    for (int32 i = ValueStart; i < OriginalCommandLine.Len(); ++i)
    {
      if (FChar::IsWhitespace(OriginalCommandLine[i]))
      {
        TokenEnd = i;
        break;
      }
    }
  }

  // Tier commands first so the user's commands are last and take final
  // effect when UE replays them. Both lists are comma-separated.
  const FString MergedValue = TierExecCmds + TEXT(",") + ExistingValue;

  FString Replacement = OriginalCommandLine.Left(TokenStart);
  Replacement += FString::Printf(TEXT("-execcmds=\"%s\""), *MergedValue);
  Replacement += OriginalCommandLine.Mid(TokenEnd);

  FCommandLine::Set(*Replacement);
}

}  // namespace

const FString FCarlaDeviceProfileSelectorModule::GetRuntimeDeviceProfileName()
{
  // Translate -quality-level=<Tier> into a CarlaQuality_<Tier> DeviceProfile
  // and merge the tier-specific -execcmds= payload that overrides
  // [GroupName@N] bucket members at SetByConsole priority. -execcmds= is
  // applied by UE during FEngineLoop::Init, after the DeviceProfile CVars
  // have been applied -- so axis CVars survive the bucket apply.
  FString QualityLevel;
  if (FParse::Value(FCommandLine::Get(), TEXT("-quality-level="), QualityLevel))
  {
    const FString Normalised = QualityLevel.ToLower();
    const TCHAR *Profile = nullptr;
    FString TierExecCmds;

    if (Normalised == TEXT("low"))
    {
      Profile = TEXT("CarlaQuality_Low");
      // [ReflectionQuality@0] forces r.SSR.Quality=0; bring it back to 2 so
      // Low car paint still picks up nearby buildings/sky via cheap SSR.
      TierExecCmds = TEXT("r.SSR.Quality 2");
    }
    else if (Normalised == TEXT("medium"))
    {
      Profile = TEXT("CarlaQuality_Medium");
    }
    else if (Normalised == TEXT("high"))
    {
      Profile = TEXT("CarlaQuality_High");
      // High mirrors Epic except for these six axes that override
      // [ViewDistanceQuality@3] / [TextureQuality@3] / [ReflectionQuality@3]
      // bucket members. See CarlaQuality_High in DefaultDeviceProfiles.ini
      // for the rest of the per-tier values.
      TierExecCmds = TEXT("r.ViewDistanceScale 0.8,")
                     TEXT("r.SkeletalMeshLODBias 1,")
                     TEXT("r.MaxAnisotropy 4,")
                     TEXT("r.VT.MaxAnisotropy 4,")
                     TEXT("r.Lumen.Reflections.DownsampleFactor 2,")
                     TEXT("r.Lumen.Reflections.MaxBounces 1");
    }
    else if (Normalised == TEXT("epic"))
    {
      Profile = TEXT("CarlaQuality_Epic");
    }

    if (Profile != nullptr)
    {
      AppendOrMergeExecCmds(TierExecCmds);
      return Profile;
    }
  }

  // -quality-level= absent or unrecognised: fall back to UE's default
  // platform-driven selection so callers that do not set the flag see no
  // behavioural change.
  return FPlatformProperties::PlatformName();
}
