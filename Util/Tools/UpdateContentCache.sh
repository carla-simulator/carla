#! /bin/bash

set -eo pipefail

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

REPO_URL="https://bitbucket.org/carla-simulator/carla-content.git"
REPO_DIR=
BRANCH=

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Create or update a shallow clone of the CARLA content repository to be used as a CI cache.
WARNING: any local change, untracked file or build artifact in PATH is deleted."

USAGE_STRING="Usage: $0 [-h|--help] --path PATH --branch BRANCH [--url URL]"

OPTS=`getopt -o h --long help,path:,branch:,url: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --path )
      REPO_DIR="$2";
      shift 2 ;;
    --branch )
      BRANCH="$2";
      shift 2 ;;
    --url )
      REPO_URL="$2";
      shift 2 ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done

source $(dirname "$0")/Environment.sh

# Both arguments are required on purpose, so the script never wipes a directory by default
if [[ -z "$REPO_DIR" || -z "$BRANCH" ]]; then
  echo "$USAGE_STRING"
  fatal_error "--path and --branch are required"
fi

# ==============================================================================
# -- Initial set up ------------------------------------------------------------
# ==============================================================================

if [[ ! -d "$REPO_DIR/.git" ]]; then

  # Never touch a non-empty directory that is not a Git repository
  if [[ -d "$REPO_DIR" && -n "$(ls -A "$REPO_DIR")" ]]; then
    fatal_error "$REPO_DIR exists, is not empty and is not a Git repository"
  fi

  log "Cache not found, initializing ${REPO_DIR}."

  # Initialize in place instead of cloning, so it also works when PATH is a mount point.
  # The content is downloaded by the fetch below; if it is interrupted,
  # the next run simply fetches again.
  git init -q "$REPO_DIR"
  git -C "$REPO_DIR" remote add -t "$BRANCH" origin "$REPO_URL"

  # Prevent automatic garbage collection in the middle of a fetch
  git -C "$REPO_DIR" config gc.auto 0

fi

cd "$REPO_DIR"

# Make sure PATH really is the content repository before deleting anything in it
if [[ "$(git remote get-url origin)" != "$REPO_URL" ]]; then
  fatal_error "$REPO_DIR is not a clone of ${REPO_URL}"
fi

# ==============================================================================
# -- Update --------------------------------------------------------------------
# ==============================================================================

# Remove stale lock files left behind by a cancelled job
rm -f .git/*.lock

log "Fetching latest revision of ${BRANCH}."

# Fetch the latest revision and make the working tree identical to it
git fetch --depth 1 --no-tags origin "$BRANCH"
git reset --hard FETCH_HEAD
git clean -ffdxq

# ==============================================================================
# -- Clean up old revisions ----------------------------------------------------
# ==============================================================================

log "Removing objects from previous revisions."

# The reflog must be expired first, otherwise gc still considers
# old commits reachable and frees nothing
git reflog expire --expire=now --all
git gc --prune=now

# ==============================================================================
# -- Verify --------------------------------------------------------------------
# ==============================================================================

# Fail if anything other than the exact origin revision remains
if [[ -n "$(git status --porcelain --ignored)" ]]; then
  git status --short --ignored
  fatal_error "Cache is not clean"
fi

log "Cache at revision $(git rev-parse --short HEAD)."
du -sh .
du -sh .git
df -h .

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
