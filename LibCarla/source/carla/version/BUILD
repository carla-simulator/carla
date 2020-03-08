genrule(
    name = "version_cpp_genrule",
    srcs = [],
    outs = ["Version.cpp"],
    cmd_bash = """\
#!/bin/bash
cat >$@ <<EOL
#include "carla/Version.h"

namespace carla {

  const char *version() {
    return "`cat bazel-out/stable-status.txt | grep STABLE_GIT_TAG | awk '{print $$2}'`"
#ifndef NDEBUG
           "-dbg"
#endif // NDEBUG
    ;
  }

} // namespace carla
EOL
""",
    cmd_bat = "", # @todo
    stamp = True,
    visibility = ["//visibility:private"],
)

cc_library(
    name = "version",
    hdrs = ["Version.h"],
    srcs = ["Version.cpp"],
    include_prefix = "carla",
    data = [":version_cpp_genrule"],
    visibility = ["//visibility:public"],
)
