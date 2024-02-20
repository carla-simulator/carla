// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using EpicGames.Core;

public class Carla :
  ModuleRules
{
  [CommandLine("-verbose")]
  bool Verbose = false;

  [CommandLine("-carsim")]
  bool EnableCarSim = false;

  [CommandLine("-chrono")]
  bool EnableChrono = false;

  [CommandLine("-pytorch")]
  bool EnablePytorch = false;

  [CommandLine("-ros2")]
  bool EnableRos2 = false;

  [CommandLine("-osm2odr")]
  bool EnableOSM2ODR = false;



  public Carla(ReadOnlyTargetRules Target) :
    base(Target)
  {
    bool IsWindows = Target.Platform == UnrealTargetPlatform.Win64;

    PrivatePCHHeaderFile = "Carla.h";
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

    TestOptionalFeature(EnableCarSim, "CarSim support", "WITH_CARSIM");
    TestOptionalFeature(EnableChrono, "Chrono support", "WITH_CHRONO");
    TestOptionalFeature(EnablePytorch, "PyTorch support", "WITH_PYTORCH");
    TestOptionalFeature(EnableRos2, "ROS2 support", "WITH_ROS2");
    TestOptionalFeature(EnableOSM2ODR, "OSM2ODR support", "WITH_OSM2ODR");

    PrivateDependencyModuleNames.AddRange(new string[]
    {
      "AIModule",
      "AssetRegistry",
      "CoreUObject",
      "Engine",
      "Foliage",
      "HTTP",
      "StaticMeshDescription",
      "ImageWriteQueue",
      "Json",
      "JsonUtilities",
      "Landscape",
      "Slate",
      "SlateCore",
      "PhysicsCore",
      "Chaos",
      "ChaosVehicles"
    });

    if (EnableCarSim)
    {
      PrivateDependencyModuleNames.Add("CarSim");
      PrivateIncludePathModuleNames.Add("CarSim");
    }

    PublicDependencyModuleNames.AddRange(new string[]
    {
      "Core",
      "RenderCore",
      "RHI",
      "Renderer",
      "ProceduralMeshComponent",
      "MeshDescription"
    });

    if (EnableCarSim)
      PublicDependencyModuleNames.Add("CarSim");

    if (Target.Type == TargetType.Editor)
      PublicDependencyModuleNames.Add("UnrealEd");
      
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

    if (EnableOSM2ODR)
    {
      // @TODO
      PublicAdditionalLibraries.Add("");
    }

    if (EnableChrono)
    {
      // @TODO
      var ChronoLibraryNames = new string[]
      {
        "ChronoEngine",
        "ChronoEngine_vehicle",
        "ChronoModels_vehicle",
        "ChronoModels_robot",
      };
    }

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
      PublicDefinitions.AddRange(new string[]
      {
        "NOMINMAX",
        "VC_EXTRALEAN",
        "WIN32_LEAN_AND_MEAN",
      });
    }
  }
}
