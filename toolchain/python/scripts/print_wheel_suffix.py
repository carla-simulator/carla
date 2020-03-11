from setuptools import Distribution


class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True


dist = BinaryDistribution()
bdist_wheel_cmd = dist.get_command_obj('bdist_wheel')
bdist_wheel_cmd.ensure_finalized()

tag = '-'.join(bdist_wheel_cmd.get_tag())
print('{}.whl'.format(tag))
