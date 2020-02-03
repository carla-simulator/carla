import argparse
import os
import sys
import sysconfig

major,minor = sys.version_info[:2]
paths = sysconfig.get_paths()

contents = """
# This file was generated
using python : {major}.{minor}
   : {command}
   : {include}
   : {lib}
   ;
""".format(major=major, minor=minor, 
	       command=sys.executable, 
	       include=paths['include'], 
	       lib=paths['stdlib'])

parser = argparse.ArgumentParser()
parser.add_argument('output_file', nargs='?', default=sys.stdout)
parsed = parser.parse_args()

if parsed.output_file == sys.stdout:
	print(contents)
else:
	with open(parsed.output_file, 'w') as out:
		out.write(contents)
		print('Generated ' + os.path.abspath(parsed.output_file))
