# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import platform
import sys

if platform.system() == 'Darwin':
	mac_version = '.'.join(platform.mac_ver()[0].split('.')[:2])
	sys.path.append(
    	'../dist/carla-0.9.0-py%d.%d-macosx-%s-x86_64.egg' % (sys.version_info.major,
	                                                          sys.version_info.minor,
	                                                          mac_version))
else:
	sys.path.append(
    	'../dist/carla-0.9.0-py%d.%d-linux-x86_64.egg' % (sys.version_info.major,
	                                                      sys.version_info.minor))
