// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Misc/ConfigCacheIni.h"
#include <util/ue-header-guard-end.h>

#include <limits>

/// Wrapper around Unreal's INI file. In get functions, @a Target value is only
/// set if it was present in the INI file, otherwise it keeps its value.
class CARLA_API FIniFile : private NonCopyable
{
private:

  template <typename TARGET, typename SOURCE>
  static void SafeCastTo(SOURCE source, TARGET &target)
  {
    if ((source >= std::numeric_limits<TARGET>::lowest()) &&
        (source <= std::numeric_limits<TARGET>::max())) {
      target = static_cast<TARGET>(source);
    } else {
      UE_LOG(LogCarla, Error, TEXT("FIniFile: Type cast failed"));
    }
  }

public:

  // ===========================================================================
  /// @name Constructor
  // ===========================================================================
  /// @{

  FIniFile() = default;

  explicit FIniFile(const FString &FileName)
  {
    ConfigFile.Read(FileName);
  }

  /// @}
  // ===========================================================================
  /// @name Other functions
  // ===========================================================================
  /// @{

  bool Combine(const FString &FileName)
  {
    return ConfigFile.Combine(FileName);
  }

  void ProcessInputFileContents(const FString &INIFileContents, const FString &Hint)
  {
    ConfigFile.ProcessInputFileContents(INIFileContents, Hint);
  }

  bool HasSection(const FString &Section) const
  {
    return (ConfigFile.Num() > 0) && (ConfigFile.FindSection(Section) != nullptr);
  }

  void AddSectionIfMissing(const FString &Section)
  {
    if (!HasSection(Section)) {
      ConfigFile.Add(Section, FConfigSection());
    }
  }

  /// Write contents to disk.
  bool Write(const FString &Filename)
  {
    return ConfigFile.Write(Filename);
  }

  /// Retrieve Unreal's FConfigFile.
  const FConfigFile &GetFConfigFile() const
  {
    return ConfigFile;
  }

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{

  template <typename T>
  void GetInt(const TCHAR* Section, const TCHAR* Key, T &Target) const
  {
    int64 Value;
    if (ConfigFile.GetInt64(Section, Key, Value)) {
      SafeCastTo<T>(Value, Target);
    }
  }

  void GetString(const TCHAR* Section, const TCHAR* Key, FString &Target) const
  {
    FString Value;
    if (ConfigFile.GetString(Section, Key, Value)) {
      Target = Value;
    }
  }

  void GetBool(const TCHAR* Section, const TCHAR* Key, bool &Target) const
  {
    bool Value;
    if (ConfigFile.GetBool(Section, Key, Value)) {
      Target = Value;
    }
  }

  void GetFloat(const TCHAR* Section, const TCHAR* Key, float &Target, const float Factor = 1.0f) const
  {
    FString Value;
    if (ConfigFile.GetString(Section, Key, Value)) {
      Target = Factor * FCString::Atof(*Value);
    }
  }

  void GetLinearColor(const TCHAR* Section, const TCHAR* Key, FLinearColor &Target) const
  {
    FString Value;
    if (ConfigFile.GetString(Section, Key, Value)) {
      Target.InitFromString(Value);
    }
  }

  /// @}
  // ===========================================================================
  /// @name Set functions
  // ===========================================================================
  /// @{

  void SetInt(const TCHAR* Section, const TCHAR* Key, const int64 Value)
  {
    ConfigFile.SetInt64(Section, Key, Value);
  }

  void SetString(const TCHAR* Section, const TCHAR* Key, const TCHAR* Value)
  {
    ConfigFile.SetString(Section, Key, Value);
  }

  void SetString(const TCHAR* Section, const TCHAR* Key, const FString &Value)
  {
    SetString(Section, Key, *Value);
  }

  void SetBool(const TCHAR* Section, const TCHAR* Key, const bool Value)
  {
    SetString(Section, Key, Value ? TEXT("True") : TEXT("False"));
  }

  void SetFloat(const TCHAR* Section, const TCHAR* Key, const float Value)
  {
    SetString(Section, Key, FText::AsNumber(Value).ToString());
  }

  void SetLinearColor(const TCHAR* Section, const TCHAR* Key, const FLinearColor &Value)
  {
    SetString(Section, Key, Value.ToString());
  }

  /// @}

private:

  FConfigFile ConfigFile;
};
