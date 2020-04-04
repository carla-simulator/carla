#!/usr/bin/env python

import sys

from subprocess import check_output

out = check_output(['git', 'describe', '--tags', '--dirty', '--always', ])
if sys.version_info > (3, 0):
    out = out.decode('utf8')
print('STABLE_GIT_TAG {}'.format(out))
