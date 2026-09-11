# The staging manifests land under <archive>/<Platform>/, not the archive
# root, so glob for them wherever they are.
file(GLOB_RECURSE CARLA_STAGING_MANIFESTS ${CARLA_PACKAGE_ARCHIVE_PATH}/Manifest_*Files_*.txt)
if(CARLA_STAGING_MANIFESTS)
  file(REMOVE ${CARLA_STAGING_MANIFESTS})
endif()

# The archive step copies over a previous archive without deleting, so a
# ue.projectstore left behind by an earlier Zen (non -pak) stage would make the
# packaged binary try to stream content over the network and fail to start.
file(GLOB_RECURSE CARLA_STALE_PROJECT_STORES ${CARLA_PACKAGE_ARCHIVE_PATH}/ue.projectstore)
if(CARLA_STALE_PROJECT_STORES)
  file(REMOVE ${CARLA_STALE_PROJECT_STORES})
endif()
