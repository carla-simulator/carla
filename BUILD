genrule(
    name = "version_genrule",
    srcs = [],
    outs = ["Version.h"],
    cmd = """\
#!/bin/bash
cat >$@ <<EOL
#pragma once

namespace carla {

  constexpr const char *version() {
    return "`cat bazel-out/stable-status.txt | grep STABLE_GIT_TAG | awk '{print $$2}'`";
  }

} // namespace carla
EOL
""",
    stamp = True,
    visibility = ["//visibility:private"],
)

cc_library(
    name = "version",
    hdrs = ["Version.h"],
    include_prefix = "carla",
    data = [":version_genrule"],
    visibility = ["//visibility:public"],
)
