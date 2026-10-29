# Building CARLA from source

!!! Note
    This build documentation refers to building CARLA 1.0 on Unreal Engine 5.8. The Unreal Engine 4.26 versions of CARLA have [their own documentation](https://carla.readthedocs.io/en/latest/).

Users can build CARLA from source code for development purposes. This is recommended if you want to add extra features or capabilities to CARLA or if you want to use the Unreal Editor to create assets or manipulate maps.

Build instructions are available for both Linux and Windows. You can also run CARLA in a Docker container for deployment in AWS, Azure or Google cloud services. Visit the [__CARLA GitHub__](https://github.com/carla-simulator/carla) and clone the repository.

CARLA requires a minimum of Windows 11 or Ubuntu 22.04, and also supports Ubuntu 24.04. If you do not need to build CARLA, you can download it instead.

The default way to build CARLA is to let your AI coding agent do it: it clones the code and the content, builds the Unreal Engine fork and CARLA, and checks the result, following procedures tested by the CARLA team.

!!! agent "Ask your agent"
    *"Clone CARLA from the ue58-dev branch, build it with Unreal Engine 5.8 and launch the editor."*

    Skills: `build-carla-ue58`, `run-carla-ue58-server`. [Connect your agent](agent_skills.md#connect-your-agent).

If you prefer to build CARLA by hand, follow the guide for your platform:

* [__Linux build__](build_linux_ue5.md)
* [__Windows build__](build_windows_ue5.md)
* [__Docker__](build_docker.md)
* [__Development container__](build_devcontainer.md)
* [__Download__](download.md)
* [__Build FAQ__](build_faq.md)
