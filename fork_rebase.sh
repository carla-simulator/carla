#!/usr/bin/env bash
# ==============================================================================
# fork_rebase.sh - Generic rebase and sync script for CARLA forks
#
# Usage:
#   ./fork_rebase.sh [OPTIONS]
#
# Options:
#   -u, --upstream-remote <name>   Upstream remote name (default: upstream)
#   -b, --upstream-branch <name>   Upstream branch name (default: ue58-dev)
#   -l, --local-branch    <name>   Local branch to rebase (default: current branch)
#   --skip-content                 Skip syncing the CARLA Content repository
#   --skip-fetch                   Skip fetching from remotes
#   --dry-run                      Print what would be executed without doing it
#   -h, --help                     Show this help message
# ==============================================================================

set -euo pipefail

# ANSI colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m' # No Color

log_info()  { echo -e "${BLUE}[INFO]${NC} $*"; }
log_ok()    { echo -e "${GREEN}[OK]${NC} $*"; }
log_warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# Default configuration
UPSTREAM_REMOTE="upstream"
UPSTREAM_BRANCH="ue58-dev"
LOCAL_BRANCH=""
SKIP_CONTENT=0
SKIP_FETCH=0
DRY_RUN=0

# Determine repository root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  log_error "Not inside a git repository."
  exit 1
fi

show_help() {
  sed -n '2,17p' "$0" | sed 's/^# \?//'
}

# Parse command line options
while [[ $# -gt 0 ]]; do
  case "$1" in
    -u|--upstream-remote)
      UPSTREAM_REMOTE="$2"
      shift 2
      ;;
    -b|--upstream-branch)
      UPSTREAM_BRANCH="$2"
      shift 2
      ;;
    -l|--local-branch)
      LOCAL_BRANCH="$2"
      shift 2
      ;;
    --skip-content)
      SKIP_CONTENT=1
      shift
      ;;
    --skip-fetch)
      SKIP_FETCH=1
      shift
      ;;
    --dry-run)
      DRY_RUN=1
      shift
      ;;
    -h|--help)
      show_help
      exit 0
      ;;
    *)
      log_error "Unknown option: $1"
      show_help
      exit 1
      ;;
  esac
done

# Resolve local branch
CURRENT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
if [ -z "${LOCAL_BRANCH}" ]; then
  LOCAL_BRANCH="${CURRENT_BRANCH}"
fi

echo -e "${BOLD}=== CARLA Fork Rebase & Sync Utility ===${NC}"
echo "Local Branch:    ${LOCAL_BRANCH}"
echo "Upstream Remote: ${UPSTREAM_REMOTE}"
echo "Upstream Branch: ${UPSTREAM_BRANCH}"
echo "Sync Content:    $([ $SKIP_CONTENT -eq 0 ] && echo 'Yes' || echo 'No')"
echo "Dry Run:         $([ $DRY_RUN -eq 0 ] && echo 'No' || echo 'Yes')"
echo "----------------------------------------"

# 1. Pre-flight checks
DIRTY_CHANGES="$(git status --porcelain | grep -v 'fork_rebase\.sh' || true)"
if [ -n "${DIRTY_CHANGES}" ]; then
  log_error "Working directory has uncommitted changes. Please commit or stash them first."
  echo "${DIRTY_CHANGES}"
  exit 1
fi

if ! git remote get-url "${UPSTREAM_REMOTE}" >/dev/null 2>&1; then
  log_error "Upstream remote '${UPSTREAM_REMOTE}' not found."
  echo "You can add it via: git remote add ${UPSTREAM_REMOTE} https://github.com/carla-simulator/carla.git"
  exit 1
fi

# Switch branch if requested and different from current
if [ "${CURRENT_BRANCH}" != "${LOCAL_BRANCH}" ]; then
  log_info "Switching to branch '${LOCAL_BRANCH}'..."
  if [ $DRY_RUN -eq 0 ]; then
    git checkout "${LOCAL_BRANCH}"
  fi
fi

# 2. Fetch updates
if [ $SKIP_FETCH -eq 0 ]; then
  log_info "Fetching latest changes from ${UPSTREAM_REMOTE}/${UPSTREAM_BRANCH}..."
  if [ $DRY_RUN -eq 0 ]; then
    git fetch "${UPSTREAM_REMOTE}" "${UPSTREAM_BRANCH}"
  fi
else
  log_info "Skipping remote fetch."
fi

UPSTREAM_REF="${UPSTREAM_REMOTE}/${UPSTREAM_BRANCH}"

# Check if upstream ref exists
if ! git rev-parse --verify "${UPSTREAM_REF}" >/dev/null 2>&1; then
  log_error "Could not find upstream reference '${UPSTREAM_REF}'. Ensure it has been fetched."
  exit 1
fi

# 3. Find base and commits
FORK_BASE="$(git merge-base "${LOCAL_BRANCH}" "${UPSTREAM_REF}" 2>/dev/null || true)"
if [ -z "${FORK_BASE}" ]; then
  log_error "Could not find a common merge base between ${LOCAL_BRANCH} and ${UPSTREAM_REF}."
  exit 1
fi

UPSTREAM_HEAD="$(git rev-parse "${UPSTREAM_REF}")"
LOCAL_HEAD="$(git rev-parse "${LOCAL_BRANCH}")"

if [ "${FORK_BASE}" = "${UPSTREAM_HEAD}" ]; then
  log_ok "Your branch '${LOCAL_BRANCH}' is already up-to-date with ${UPSTREAM_REF}. No rebase needed!"
else
  AHEAD_COUNT="$(git rev-list --count "${FORK_BASE}..${LOCAL_BRANCH}")"
  BEHIND_COUNT="$(git rev-list --count "${FORK_BASE}..${UPSTREAM_REF}")"
  log_info "Branch '${LOCAL_BRANCH}' has ${AHEAD_COUNT} local commits on top of base $(git rev-parse --short "${FORK_BASE}")."
  log_info "Upstream '${UPSTREAM_REF}' has ${BEHIND_COUNT} new commits to catch up with."

  # 4. Create safety backup branch
  TIMESTAMP="$(date +"%Y%m%d_%H%M%S")"
  BACKUP_BRANCH="backup/${LOCAL_BRANCH}_pre_rebase_${TIMESTAMP}"
  log_info "Creating safety backup branch: ${BACKUP_BRANCH}"
  if [ $DRY_RUN -eq 0 ]; then
    git branch "${BACKUP_BRANCH}" "${LOCAL_BRANCH}"
    log_ok "Backup branch created at ${LOCAL_HEAD}."
  fi

  # 5. Perform the rebase
  # Using --onto <newbase> <oldbase> <branch> for robustness with shallow/grafted histories
  log_info "Running: git rebase --onto ${UPSTREAM_REF} ${FORK_BASE} ${LOCAL_BRANCH}"
  if [ $DRY_RUN -eq 0 ]; then
    if git rebase --onto "${UPSTREAM_REF}" "${FORK_BASE}" "${LOCAL_BRANCH}"; then
      log_ok "Rebase completed successfully!"
    else
      log_error "Rebase encountered conflicts."
      echo -e "${YELLOW}To resolve:${NC}"
      echo "  1. Fix conflicts in conflicting files"
      echo "  2. git add <resolved-files>"
      echo "  3. git rebase --continue"
      echo -e "${YELLOW}To abort and restore:${NC}"
      echo "  git rebase --abort"
      echo "  (or checkout backup branch '${BACKUP_BRANCH}')"
      exit 1
    fi
  else
    log_info "[DRY-RUN] Would rebase ${LOCAL_BRANCH} onto ${UPSTREAM_REF} from base ${FORK_BASE}."
  fi
fi

# 6. Synchronize Content Repository (if present)
CONTENT_DIR="Unreal/CarlaUnreal/Content/Carla"
if [ $SKIP_CONTENT -eq 0 ]; then
  if [ -d "${CONTENT_DIR}/.git" ]; then
    log_info "Found CARLA Content repository at '${CONTENT_DIR}'."
    CONTENT_BRANCH="$(git -C "${CONTENT_DIR}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo '')"
    CONTENT_REMOTE="$(git -C "${CONTENT_DIR}" remote 2>/dev/null | head -n 1 || echo '')"
    
    if [ -n "${CONTENT_BRANCH}" ] && [ -n "${CONTENT_REMOTE}" ]; then
      log_info "Syncing Content repository (branch: ${CONTENT_BRANCH}, remote: ${CONTENT_REMOTE})..."
      if [ $DRY_RUN -eq 0 ]; then
        if [ -n "$(git -C "${CONTENT_DIR}" status --porcelain)" ]; then
          log_warn "Content repository has uncommitted changes. Skipping pull to avoid conflicts."
        else
          git -C "${CONTENT_DIR}" fetch "${CONTENT_REMOTE}" "${CONTENT_BRANCH}"
          git -C "${CONTENT_DIR}" pull --ff-only "${CONTENT_REMOTE}" "${CONTENT_BRANCH}" || {
            log_warn "Could not fast-forward Content repository. Please inspect manually."
          }
          log_ok "Content repository synced."
        fi
      else
        log_info "[DRY-RUN] Would sync Content repo at ${CONTENT_DIR}."
      fi
    else
      log_warn "Could not determine branch or remote for Content repo at '${CONTENT_DIR}'."
    fi
  else
    log_info "No Content repository found at '${CONTENT_DIR}' (or not a git checkout). Skipping content sync."
  fi
else
  log_info "Skipping Content repository sync as requested."
fi

echo "----------------------------------------"
log_ok "Upgrade procedure completed!"
echo -e "${BOLD}Next steps (when ready to test/build):${NC}"
echo "  1. Rebuild LibCarla & Unreal plugin: cmake --build Build"
echo "  2. Rebuild & install Python API:     cmake --build Build --target carla-python-api-install"
echo "  3. Push to your fork remote:         git push --force-with-lease origin ${LOCAL_BRANCH}"
