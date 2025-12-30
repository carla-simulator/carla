# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# pylint: disable=W0401,import-self
from .libcarla import *

def __fix_ad_namespaces():
    """
    When using PythonAPI.rss the ad_rss subpackage is imported, however
    this package puts all its subpackages into sys.modules.
    This creates ambiguous modules and a wrong hierarchy.
    
    This is a # HACK to fix this issue.
    
    When updating the RSS version, check if this is correct or ad_rss has fixed
    the import behavior.
    """
    import sys
    import inspect
    ad_submodules = {}  # dict of modules
    def get_submodules(mod, parent):
       for v in vars(mod).values():
            if (not inspect.ismodule(v)
                or "." in v.__name__  # assume correct
                or v.__name__ in sys.builtin_module_names  # Later ad versions may contain os or sys
                # Make more robust with expected docstring
                or (v.__doc__ and ("Python binding of" not in v.__doc__ or "C++ code" not in v.__doc__))
            ):
                # NOTE: Check when updating RSS version
                # if inspect.ismodule(v):
                #    print("Skipping", v)
                continue
            if v in ad_submodules:
                # this could be problematic with sibling imports into subpackages
                # e.g. ad.map vs ad.rss.map
                # this should be fine for the current rss versions.
                # NOTE: Check when updating RSS version
                if v.__name__ != "map":
                    print(v, "already imported. This might create a wrong carla.ad import hierarchy")
                continue
            correct_path = parent+"."+v.__name__
            ad_submodules[v] = correct_path
            get_submodules(v, correct_path)
    get_submodules(ad, "carla.libcarla.ad")  # type: ignore
    if ad.__name__ != "carla.libcarla.ad" and "ad" in sys.modules and ad is sys.modules["ad"] and "carla.libcarla.ad" not in sys.modules:  # type: ignore
        sys.modules["carla.libcarla.ad"] = sys.modules.pop("ad")
    for mod, correct_name in ad_submodules.items():
        # NOTE: Check when updating RSS version
        #print("Correcting", mod.__name__, "to", correct_name)
        sys.modules[correct_name] = sys.modules.pop(mod.__name__)
        mod.__name__ = correct_name
    
# Only when using RSS build
if "ad" in locals():
    try:
        __fix_ad_namespaces()
    except Exception as e:
        print("Could not clean ad_rss namespace due to", e, "Please report this bug")
del __fix_ad_namespaces

# Allow from carla.command import ...
import sys
sys.modules["carla.command"] = command
del sys

