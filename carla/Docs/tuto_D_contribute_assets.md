# How to upgrade content

Our content resides on a separate [Git LFS repository][contentrepolink]. As part
of our build system, we generate and upload a package containing the latest
version of this content tagged with the current date and commit. Regularly, we
upgrade the [CARLA repository][carlarepolink] with a link to the latest version
of the content package. This document contains the manual steps necessary to
update this link to the latest version.

1. **Copy the tag of the content package you wish to link.**<br>
   This tag can be found by looking at the package name generated in the
   artifacts section of the latest [Jenkins build][jenkinslink], e.g.,
   `20190617_086f97f.tar.gz`.

2. **Paste the tag in ContentVersions.txt.**<br>
   [Edit ContentVersions.txt][cvlink] by pasting the tag at the end of the file,
   e.g. `Latest: 20190617_086f97f` (without the `.tar.gz` part).

3. **Open a Pull Request.**<br>
   Commit the changes and open a new Pull Request.

[contentrepolink]: https://bitbucket.org/carla-simulator/carla-content
[carlarepolink]: https://github.com/carla-simulator/carla
[jenkinslink]: http://35.181.165.160:8080/blue/organizations/jenkins/carla-content/activity
[cvlink]: https://github.com/carla-simulator/carla/edit/master/Util/ContentVersions.txt
