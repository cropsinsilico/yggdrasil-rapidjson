#!/bin/sh
#
# Update the version for YggdrasilRapidJSON.
#
# This script:
#   1. Computes the next version by incrementing the last number
#      of the most recent version tag (e.g. v1.1.0.7 -> v1.1.0.8)
#      unless a version is provided with --version.
#   2. Updates the version in pixi.toml, CMakeLists.txt and
#      conda.recipe/recipe.yaml. CHANGELOG.md will be uploaded as
#      will if --changelog is provided.
#   5. Optionally commits the version bump (--commit)
#
# Usage: bump_version.sh [--commit] [--changelog] [--version VERSION]
#
#   --commit  Stage and commit the updated version files.
#   --changelog  Update the changelog using git-cliff
#   --version VERSION  Use VERSION instead of the next semantic version
#                      (e.g. --version 1.1.0.8). May also be passed as
#                      --version=VERSION.

set -e

RELEASE_BRANCH="yggdrasil"
FILES="pixi.toml CMakeLists.txt conda.recipe/recipe.yaml"

do_changelog=0
do_commit=0
requested_version=""

while [ $# -gt 0 ]; do
    case "$1" in
        --commit)
            do_commit=1
            ;;
        --changelog)
            do_changelog=1
            ;;
        --version)
            if [ $# -lt 2 ]; then
                echo "bump-version: --version requires a value." >&2
                exit 1
            fi
            requested_version="$2"
            shift
            ;;
        --version=*)
            requested_version="${1#--version=}"
            ;;
        -h|--help)
            sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "bump-version: unknown argument '$1'." >&2
            echo "Usage: bump_version.sh [--commit] [--changelog] [--version VERSION]" >&2
            exit 1
            ;;
    esac
    shift
done

# Ensure we are in the repository root.
cd "$(git rev-parse --show-toplevel)"

# 1. Find the most recent version tag.
last_tag="$(git tag --list 'v[0-9]*' --sort=-v:refname | head -n 1)"
if [ -z "$last_tag" ]; then
    echo "bump-version: no version tags found." >&2
    exit 1
fi

# 2. Determine the version to use. If one was provided on the command line,
#    validate and use it. Otherwise compute the next version by incrementing
#    the last numeric component.
if [ -n "$requested_version" ]; then
    if ! printf '%s' "$requested_version" | grep -Eq '^[0-9]+(\.[0-9]+)+$'; then
        echo "bump-version: invalid --version '$requested_version'; expected a numeric version like 1.1.0.8." >&2
        exit 1
    fi
    new_version="$requested_version"
    new_tag="v${new_version}"
else
    last_version="${last_tag#v}"
    case "$last_version" in
        *.*.*.*)
            prefix="${last_version%.*}"
            exten="${last_version##*.}"
            ;;
        *)
            echo "bump-version: last tag '$last_tag' does not match the expected MAJOR.MINOR.PATCH.EXTEN format." >&2
            exit 1
            ;;
    esac
    if ! printf '%s' "$exten" | grep -Eq '^[0-9]+$'; then
        echo "bump-version: last tag '$last_tag' does not end in a numeric component." >&2
        exit 1
    fi
    new_version="${prefix}.$((exten + 1))"
    new_tag="v${new_version}"
fi

if git rev-parse -q --verify "refs/tags/${new_tag}" >/dev/null; then
    echo "bump-version: tag '$new_tag' already exists." >&2
    exit 1
fi

echo "bump-version: last tag:  $last_tag"
echo "bump-version: next tag:  $new_tag"

# 3. Update the version in pixi.toml, CMakeLists.txt and conda.recipe/recipe.yaml.
if ! grep -Eq '^version = "' pixi.toml; then
    echo "bump-version: could not find the version in pixi.toml." >&2
    exit 1
fi
sed -i.bak -E "s/^version = \".*\"/version = \"${new_version}\"/" pixi.toml
rm -f pixi.toml.bak

if ! grep -Eq 'DEFAULT_YGGDRASIL_RAPIDJSON_VERSION "[^"]*"' CMakeLists.txt; then
    echo "bump-version: could not find the version in CMakeLists.txt." >&2
    exit 1
fi
sed -i.bak -E "s/(DEFAULT_YGGDRASIL_RAPIDJSON_VERSION \")[^\"]*(\")/\1${new_version}\2/" CMakeLists.txt
rm -f CMakeLists.txt.bak

if ! grep -Eq '^  version: "[^"]*"' conda.recipe/recipe.yaml; then
    echo "bump-version: could not find the version in conda.recipe/recipe.yaml." >&2
    exit 1
fi
sed -i.bak -E "s/^  version: \".*\"/  version: \"${new_version}\"/" conda.recipe/recipe.yaml
rm -f conda.recipe/recipe.yaml.bak
if [ "$do_changelog" -eq 1 ]; then
    if !git rev-parse -q --verify "refs/tags/${new_tag}" >/dev/null; then
        git-cliff --unreleased --tag "${new_tag}" --prepend
        FILES="${FILES} CHANGELOG.md"
    else
        echo "bump-version: the tag already exists so CHANGELOG.md was not updated" >&2
    fi
fi

echo "bump-version: updated version to ${new_version} in ${FILES}"

# Determine whether the version files were actually changed.
files_changed=1
if git diff --quiet -- $FILES; then
    files_changed=0
fi

# 5. Optionally commit the version bump.
if [ "$do_commit" -eq 1 ]; then
    if [ "$files_changed" -eq 1 ]; then
        git add $FILES
        git commit -m "chore(release): prepare for ${new_tag}"
        echo "bump-version: committed version bump for ${new_tag}"
    else
        echo "bump-version: version files already up to date, nothing to commit."
    fi
fi
