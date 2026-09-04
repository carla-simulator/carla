# How to upgrade content

Our content resides on a separate [Git LFS repository][contentrepolink], which
users clone directly. Regularly, we upgrade the [CARLA repository][carlarepolink]
to pin the latest commit of the content repository. This document contains the
manual steps necessary to update this pin to the latest version.

1. **Copy the commit of the content you wish to pin.**<br>
   Take the date and the abbreviated commit hash of the content repository
   commit you want to pin, e.g., `20190617_086f97f`.

2. **Paste the tag in ContentVersions.txt.**<br>
   [Edit ContentVersions.txt][cvlink] by pasting the tag at the end of the file,
   e.g. `Latest: 20190617_086f97f`.

3. **Open a Pull Request.**<br>
   Commit the changes and open a new Pull Request.

[contentrepolink]: https://bitbucket.org/carla-simulator/carla-content/src/master/
[carlarepolink]: https://github.com/carla-simulator/carla
[cvlink]: https://github.com/carla-simulator/carla/edit/master/Util/ContentVersions.txt
