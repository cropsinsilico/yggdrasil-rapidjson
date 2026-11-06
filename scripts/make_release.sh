#!/bin/sh
#
# Create a release tag for YggdrasilRapidJSON.
#
# This script:
#   1. Verifies the current branch is "yggdrasil".
#   2. Verifies there are commits that are not part of the last tag.
#   3. Computes the next version by incrementing the last number of the most
#      recent version tag (e.g. v1.1.0.7 -> v1.1.0.8).
#   4. Updates the version in pixi.toml, CMakeLists.txt and
#      conda.recipe/recipe.yaml.
#   5. Commits the version bump.
#   6. Creates that release tag with notes based on git-cliff.
#   7. Pushes the tag to github.
#   8. Create the release from the tag.
#
# Usage: make_release.sh
#

set -e

RELEASE_BRANCH="yggdrasil"
FILES="pixi.toml CMakeLists.txt conda.recipe/recipe.yaml"

while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "make-release: unknown argument '$1'." >&2
            echo "Usage: make_release.sh [--commit] [--tag]" >&2
            exit 1
            ;;
    esac
    shift
done

# Ensure we are in the repository root.
cd "$(git rev-parse --show-toplevel)"

# 1. Check the current branch.
current_branch="$(git rev-parse --abbrev-ref HEAD)"
if [ "$current_branch" != "$RELEASE_BRANCH" ]; then
    echo "make-release: releases must be created from the '$RELEASE_BRANCH' branch (currently on '$current_branch')." >&2
    exit 1
fi

# 2. Find the most recent version tag.
last_tag="$(git tag --list 'v[0-9]*' --sort=-v:refname | head -n 1)"
if [ -z "$last_tag" ]; then
    echo "make-release: no version tags found." >&2
    exit 1
fi

# Check for commits not part of the last tag.
n_commits="$(git rev-list "${last_tag}..HEAD" --count)"
if [ "$n_commits" -eq 0 ]; then
    echo "make-release: no commits since the last tag ($last_tag). Nothing to release." >&2
    exit 1
fi

# 3. Compute the next version by incrementing the last numeric component.
last_version="${last_tag#v}"
case "$last_version" in
    *.*.*.*)
        prefix="${last_version%.*}"
        exten="${last_version##*.}"
        ;;
    *)
        echo "make-release: last tag '$last_tag' does not match the expected MAJOR.MINOR.PATCH.EXTEN format." >&2
        exit 1
        ;;
esac
if ! printf '%s' "$exten" | grep -Eq '^[0-9]+$'; then
    echo "make-release: last tag '$last_tag' does not end in a numeric component." >&2
    exit 1
fi
new_version="${prefix}.$((exten + 1))"
new_tag="v${new_version}"

if git rev-parse -q --verify "refs/tags/${new_tag}" >/dev/null; then
    echo "make-release: tag '$new_tag' already exists." >&2
    exit 1
fi

echo "make-release: last tag:  $last_tag"
echo "make-release: next tag:  $new_tag ($n_commits commit(s) since last tag)"

# 4. Update the version in pixi.toml, CMakeLists.txt and conda.recipe/recipe.yaml.
./scripts/bump_version.sh --commit --changelog --version "${new_version}"

# 5. Create the release tag.
tag_msg="Release ${new_tag}"
notes_file="$(mktemp)"
if git-cliff --unreleased --tag "${new_tag}" --strip all -o "${notes_file}" && [ -s "${notes_file}" ]; then
    tag_msg_file="${notes_file}"
else
    rm -f "${notes_file}"
    tag_msg_file=""
fi
if [ -n "${tag_msg_file}" ]; then
    git tag -a "${new_tag}" --cleanup=verbatim -F "${tag_msg_file}"
    rm -f "${tag_msg_file}"
else
    echo "make-release: git-cliff unavailable or produced no notes." >&2
    exit 1
    # Allow default tag message without notes?
    # git tag -a "${new_tag}" -m "${tag_msg}"
fi

# 5. Push the tag
git push origin "${new_tag}"

# 6. Create the release
gh release create "${new_tag}" --notes-from-tag
