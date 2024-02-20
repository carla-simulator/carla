// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using EpicGames.Core;

public class CarlaTools :
  ModuleRules
{
  [CommandLine("-verbose")]
  bool Verbose = false;

  [CommandLine("-houdini")]
  bool EnableHoudini = false;

  [CommandLine("-nv-omniverse")]
  bool EnableNVIDIAOmniverse = false;

  [CommandLine("-osm2odr")]
  bool EnableOSM2ODR = false;



  public CarlaTools(ReadOnlyTargetRules Target) :
    base(Target)
  {
    bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    bEnableExceptions = true;
    
    Action<bool, string, string> TestOptionalFeature = (enable, name, definition) =>
    {
      if (enable)
        PrivateDefinitions.Add(name);
      Console.WriteLine(string.Format("{0} is {1}.", name, enable ? "enabled" : "disabled"));
    };
    
    Action<string> AddIncludeDirectories = (str) =>
    {
      if (str.Length == 0)
        return;
      var paths = str.Split(';');
      if (paths.Length == 0)
        return;
      PublicIncludePaths.AddRange(paths);
    };

    TestOptionalFeature(EnableOSM2ODR, "OSM2ODR support", "WITH_OSM2ODR");

    PublicDependencyModuleNames.AddRange(new string[]
    {
      "Core",
      "ProceduralMeshComponent",
      "MeshDescription",
      "RawMesh",
      "AssetTools",
    });

    PrivateDependencyModuleNames.AddRange(new string[]
    {
      "CoreUObject",
      "Engine",
      "Slate",
      "SlateCore",
      "UnrealEd",
      "Blutility",
      "UMG",
      "EditorScriptingUtilities",
      "Landscape",
      "Foliage",
      "FoliageEdit",
      "MeshMergeUtilities",
      "Carla",
      "StaticMeshDescription",
      "Json",
      "JsonUtilities",
      "Networking",
      "Sockets",
      "HTTP",
      "RHI",
      "RenderCore",
      "MeshMergeUtilities",
      "StreetMapImporting",
      "StreetMapRuntime",
      "Chaos",
      "ChaosVehicles"
    });

    if (EnableHoudini)
    {
      PrivateDependencyModuleNames.AddRange(new string[]
      {
        "HoudiniEngine",
        "HoudiniEngineEditor",
        "HoudiniEngineRuntime"
      });
    }

    if (EnableNVIDIAOmniverse)
    {
      PrivateDefinitions.Add("WITH_OMNIVERSE");
      PrivateDefinitions.Add("WITH_OMNIVERSE");

      PrivateDependencyModuleNames.AddRange(new string[]
      {
        "OmniverseUSD",
        "OmniverseRuntime"
      });
    }

    if (IsWindows)
    {
      PrivateDefinitions.Add("NOMINMAX");
      PrivateDefinitions.Add("VC_EXTRALEAN");
      PrivateDefinitions.Add("WIN32_LEAN_AND_MEAN");
    }

    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
    PublicDefinitions.Add("BOOST_NO_RTTI");
    PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
    PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
    PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
    PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");

    if (EnableOSM2ODR)
    {
      // @TODO
      PublicAdditionalLibraries.Add("");
    }
    
    PublicIncludePaths.Add(ModuleDirectory);
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/boost-src/libs/asio/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/align/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/config/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/core/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/static_assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/throw_exception/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/context/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mp11/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/pool/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/integer/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/type_traits/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/winapi/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/predef/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/smart_ptr/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/move/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/coroutine/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/exception/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/tuple/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/system/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/variant2/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/utility/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/io/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/preprocessor/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/date_time/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/algorithm/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/array/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/bind/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/concept_check/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/iterator/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/detail/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function_types/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mpl/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/fusion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container_hash/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/describe/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/typeof/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/functional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/optional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/range/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/conversion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/regex/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/unordered/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/lexical_cast/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/intrusive/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/tokenizer/include");
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/boost-src/libs/python/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/align/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/config/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/core/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/static_assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/throw_exception/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/bind/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/conversion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/smart_ptr/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/move/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/type_traits/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/detail/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/preprocessor/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/foreach/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/iterator/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/concept_check/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function_types/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mpl/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/predef/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/utility/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/io/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/fusion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container_hash/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/describe/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mp11/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/tuple/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/typeof/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/functional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/optional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/range/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/array/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/regex/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/lexical_cast/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/intrusive/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/integer/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/numeric/conversion/include;C:/Users/Marce/AppData/Local/Programs/Python/Python38/include");
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/boost-src/libs/python/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/config/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/core/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/static_assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/throw_exception/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/detail/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/preprocessor/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/type_traits/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mpl/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/predef/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/utility/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/io/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/python/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/align/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/bind/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/conversion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/smart_ptr/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/move/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/foreach/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/iterator/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/concept_check/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function_types/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/fusion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container_hash/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/describe/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mp11/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/tuple/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/typeof/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/functional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/optional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/range/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/array/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/regex/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/lexical_cast/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/intrusive/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/integer/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/numeric/conversion/include;C:/Users/Marce/AppData/Local/Programs/Python/Python38/include;C:/Users/Marce/AppData/Local/Programs/Python/Python38/Lib/site-packages/numpy/core/include");
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/boost-src/libs/geometry/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/algorithm/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/array/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/config/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/core/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/static_assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/throw_exception/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/bind/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/concept_check/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/preprocessor/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/type_traits/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/exception/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/smart_ptr/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/move/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/tuple/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/iterator/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/detail/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function_types/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mpl/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/predef/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/utility/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/io/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/fusion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container_hash/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/describe/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mp11/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/typeof/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/functional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/optional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/range/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/conversion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/regex/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/unordered/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/any/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/type_index/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/intrusive/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/endian/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/integer/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/lexical_cast/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/math/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/random/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/dynamic_bitset/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/system/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/variant2/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/winapi/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/multiprecision/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/polygon/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/qvm/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/rational/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/serialization/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/spirit/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/phoenix/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/proto/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/pool/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/thread/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/atomic/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/chrono/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/ratio/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/date_time/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/tokenizer/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/variant/include");
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/boost-src/libs/gil/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/config/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/concept_check/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/preprocessor/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/static_assert/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/type_traits/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/container_hash/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/describe/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mp11/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/core/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/throw_exception/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/filesystem/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/detail/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/io/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/iterator/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function_types/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/mpl/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/predef/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/utility/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/fusion/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/tuple/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/typeof/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/functional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/function/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/bind/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/optional/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/move/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/smart_ptr/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/system/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/variant2/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/winapi/include;F:/Carla/out/build/x64-Release/_deps/boost-src/libs/integer/include");
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/eigen-src");
      
    PublicAdditionalLibraries.Add("F:/Carla/out/build/x64-Release/LibCarla/carla-server.lib");
    AddIncludeDirectories("F:/Carla/LibCarla/source");

    PublicAdditionalLibraries.Add("F:/Carla/out/build/x64-Release/_deps/zlib-build/zlibstatic.lib");
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/zlib-build;F:/Carla/out/build/x64-Release/_deps/zlib-src");

    PublicAdditionalLibraries.Add("F:/Carla/out/build/x64-Release/_deps/libpng-build/libpng16_static.lib");
    AddIncludeDirectories("");

    PublicAdditionalLibraries.Add("F:/Carla/out/build/x64-Release/_deps/rpclib-build/rpc.lib");
    AddIncludeDirectories("F:/Carla/out/build/x64-Release/_deps/rpclib-src/include");

    PublicAdditionalLibraries.Add("F:/Carla/out/build/x64-Release/libsqlite3.lib");
    PublicIncludePaths.Add("F:/Carla/out/build/x64-Release/_deps/sqlite3-src");

    PublicDefinitions.AddRange(new string[]
    {
      "ASIO_NO_EXCEPTIONS",
      "BOOST_NO_EXCEPTIONS",
      "LIBCARLA_NO_EXCEPTIONS",
      "PUGIXML_NO_EXCEPTIONS",
      "BOOST_DISABLE_ABI_HEADERS",
      "BOOST_NO_RTTI",
      "BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY",
    });

    if (IsWindows)
    {
      PrivateDefinitions.Add("NOMINMAX");
      PrivateDefinitions.Add("VC_EXTRALEAN");
      PrivateDefinitions.Add("WIN32_LEAN_AND_MEAN");
    }
  }
}
