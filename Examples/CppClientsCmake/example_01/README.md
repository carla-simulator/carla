# About
The demo is representing the basic set ups in the carla scene:
*   synchronouse mode
*   switching scene
*   setting weather
*   spawning walkers and vehicles

The synchronose mode may be especially important for those who simulate cameras/sensors and may not skip recorded frames.

## Build
The **project is NOT integrated** into the upper level CmakeLists.txt!!! So the build will require running a separate cmake command AFTER the normal build of the Carla Cpp Client libraries. This means that the<br>
/path/to/carla/Build/LibCarla/cmake/client/libcarla_client.a and/or<br>
/path/to/carla/Build/LibCarla/cmake/client/libcarla_client_debug.a<br>
should already exist on the disk.<br>
Example to make debug build for the application:
1.  mkdir debug
2.  cd debug
3.  cmake .. -DCMAKE_BUILD_TYPE=Debug
4.  make

Similar for release, just -DCMAKE_BUILD_TYPE=Release instead<br>

For those who is using standard boost libraries (ex. 1.65.1 for Ubuntu 18) run cmake as follows: cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES=/path/to/boost/boost_1_7x. Ex.: if previously the "make rebuild" was started to rebuild the targets the boost 1.72 should have been downloaded into the Build folder and -DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES=../../../../Build/boost-1.72.0-c8-install/include

The above usage of the boost 1.72/1.73 is also applicable for building the Carla client libraries (i.e. carla_client.a and carla_client_debug.a). In this way you will not have to mix up the standard boost libraries with the ones required by Carla.

The two following bugs were found during setting up the current demo:
<pre>
diff --git a/LibCarla/source/carla/client/detail/Simulator.cpp b/LibCarla/source/carla/client/detail/Simulator.cpp
index 2b543c31..e018c7dc 100644
--- a/LibCarla/source/carla/client/detail/Simulator.cpp
+++ b/LibCarla/source/carla/client/detail/Simulator.cpp
@@ -89,6 +89,7 @@ namespace detail {
     for (auto i = 0u; i < number_of_attempts; ++i) {
       using namespace std::literals::chrono_literals;
       _episode->WaitForState(10ms);
+      _episode.reset();
       auto episode = GetCurrentEpisode();
       if (episode.GetId() != id && !_client.CheckIntermediateEpisode()) {
         return episode;
diff --git a/LibCarla/source/carla/nav/Navigation.cpp b/LibCarla/source/carla/nav/Navigation.cpp
index c0f435aa..8979eb8b 100644
--- a/LibCarla/source/carla/nav/Navigation.cpp
+++ b/LibCarla/source/carla/nav/Navigation.cpp
@@ -193,8 +193,6 @@ namespace nav {
       return;
     }
 
-    DEBUG_ASSERT(_crowd != nullptr);
     // create and init
     _crowd = dtAllocCrowd();
     // these radius should be the maximum size of the vehicles (CarlaCola for Carla)

</pre>

1.  Bug in the Simulator.cpp **when loading new scene** leads to the situation where the walkers were not "seen" by the cars causing too many hits of cars with walkers. The line "_episode.reset();" is needed to update the Episode id otherwise the IsAlive() for walkers returned false in LocalizationStage::Action() (~line 289)
2.  The "DEBUG_ASSERT(_crowd != nullptr);" should not be present since _crowd is initialized right after it and this causes unjustified termination in the debug mode.

Known issues: in the master branch there is eventually an error int the commit c45614c983fa72b971dde1964b85aeb15e5a4ac7 which causes the crash for the cpp client examples. The 0.9.9 tag however should work fine with the introduced changes.