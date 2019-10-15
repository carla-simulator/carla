// Copyright (c) 2019 Aptiv
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Paths.h"
#include "Regex.h"
#include <dlfcn.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <poll.h>

#include "Carla/Sensor/Filter.h"

#define MAX_EVENTS 1024
#define LEN_NAME 16
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))

class FilterLoader {
public:
  FilterLoader(FString filterNameToLoad) {
    filterName = filterNameToLoad;
    setupFiltersDirIfNeeded();
    inotifyDescriptor = inotify_init1(IN_NONBLOCK);
    watcher = inotify_add_watch(inotifyDescriptor, TCHAR_TO_UTF8(*FILTERS_DIR),
                                IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM);
    pollDescriptors = {inotifyDescriptor, POLLIN, 0};
    tryLoad();
  };

  ~FilterLoader() {
    inotify_rm_watch(inotifyDescriptor, watcher);
    close(inotifyDescriptor);
  };

  std::shared_ptr <Filter> getFilter() {
    int ret = poll(&pollDescriptors, 1, 0);
    if (ret < 0) {
      UE_LOG(LogCarla, Error, TEXT("Error polling inotify events: %s"), dlerror());
    } else if (ret > 0) {
      int length = read(inotifyDescriptor, buffer, BUF_LEN);
      bool reload = false;

      while (length > 0) {
        length = read(inotifyDescriptor, buffer, BUF_LEN);
        reload = true;
      }

      if (reload) {
        tryLoad();
      }
    }
    return std::shared_ptr<Filter>(filter);
  };

  FString filterName;

private:
  const FString FILTERS_FILES_EXT = ".dll";
  const FString FILTERS_DIR_PATH_POSTFIX = "Filters/";
  const FString FILTERS_DIR = FPaths::LaunchDir() + FILTERS_DIR_PATH_POSTFIX;

  std::shared_ptr <Filter> filter;

  int inotifyDescriptor;
  int watcher;
  char buffer[BUF_LEN];
  struct pollfd pollDescriptors;

  void tryLoad() {
    FString highestFilter = getHighestFilterPath();
    if (!highestFilter.IsEmpty()) {
      filter = load(highestFilter);
    } else {
      filter = std::shared_ptr<Filter>(nullptr);
    }
  }

  std::shared_ptr <Filter> load(FString filePath) const {
    void *handle = dlopen(toStdString(filePath).c_str(), RTLD_NOW);
    if (!handle) {
      UE_LOG(LogCarla, Error, TEXT("Could not load filter with path: %s and error: %s"), *filePath,
             dlerror());
      return std::shared_ptr<Filter>(nullptr);
    }
    Filter * (*create)();
    void (*destroy)(Filter *);

    create = (Filter *(*)()) dlsym(handle, "create_object");
    destroy = (void (*)(Filter *)) dlsym(handle, "destroy_object");

    return std::shared_ptr<Filter>((Filter *) create());
  }

  std::string toStdString(FString string) const {
    return std::string(TCHAR_TO_UTF8(*string));
  }

  void setupFiltersDirIfNeeded() {
    if (!getPlatformFile().DirectoryExists(*FILTERS_DIR)) {
      UE_LOG(LogCarla, Log, TEXT("Filters dir not found - creating it: %s"), *FILTERS_DIR);
      getPlatformFile().CreateDirectory(*FILTERS_DIR);
      if (!getPlatformFile().DirectoryExists(*FILTERS_DIR)) {
        UE_LOG(LogCarla, Error, TEXT("Couldn't create filters dir: %s"), *FILTERS_DIR);
      }
    }
  }

  FString getHighestFilterPath() {
    TArray<FString> files;
    getPlatformFile().FindFiles(files, *FILTERS_DIR, *FILTERS_FILES_EXT);
    int32 highestIndex = -1;
    FString fullName = "";
    FRegexPattern filterFileNamePattern(filterName + "_(\\d+)" + FILTERS_FILES_EXT);

    for (FString file: files) {
      FRegexMatcher fileMatcher(filterFileNamePattern, file);
      if (fileMatcher.FindNext()) {
        FString capturedNumber = fileMatcher.GetCaptureGroup(1);
        int32 index = FCString::Atoi(*fileMatcher.GetCaptureGroup(1));
        if (index > highestIndex) {
          highestIndex = index;
          fullName = file;
        }
      }
    }

    if (highestIndex > -1) {
      return fullName;
    }

    FString defaultFilterName = FILTERS_DIR + filterName + FILTERS_FILES_EXT;
    if (getPlatformFile().FileExists(*defaultFilterName)) {
      return defaultFilterName;
    }

    return "";
  }

  IPlatformFile &getPlatformFile() {
    return FPlatformFileManager::Get().GetPlatformFile();
  }
};
