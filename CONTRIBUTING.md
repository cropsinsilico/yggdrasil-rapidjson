# Contributing to YggdrasilRapidJSON

Thanks for your interest in contributing to YggdrasilRapidJSON! This document
describes how to set up a development environment, build and test the project,
and submit changes.

## Getting started

YggdrasilRapidJSON is developed with [pixi](https://pixi.sh), which provides all
of the build and test dependencies in reproducible environments. Install pixi
following the [official instructions](https://pixi.sh/latest/#installation),
then clone the repository and initialize the submodules:

```shell
git clone https://github.com/cropsinsilico/yggdrasil-rapidjson.git
cd yggdrasil-rapidjson
git submodule update --init --recursive
```

## Development environments

The environments and tasks used for development are defined in `pixi.toml`.
You do not need to activate them manually; `pixi run` installs and uses the
correct environment on demand.

| Environment  | Purpose                                             |
| ------------ | --------------------------------------------------- |
| `build`      | Configure and build the library, tests and examples |
| `test`       | Run the unit and performance tests                  |
| `docs`       | Build the API documentation with Doxygen            |
| `dev`        | Git tooling (git-cliff) and git hooks               |
| `build-conda`| Build the conda package with `pixi publish`         |
| `test-py311` ... `test-py315` | Test against a specific Python version |

## Building and testing

Configure the build:

```shell
pixi run configure
```

Build the tests (and the library):

```shell
pixi run build
```

Run the unit tests:

```shell
pixi run test
```

Run the full test suite (all ctest tests):

```shell
pixi run all-tests
```

Run the performance tests:

```shell
pixi run perftest
```

To test against a specific Python version, use the corresponding environment:

```shell
pixi run --environment test-py312 test
```

### Documentation

Build the API documentation with Doxygen:

```shell
pixi run docs
```

The generated HTML will be available under `build-doc/`.

### Installing

Install the built library to a prefix (defaults to the system prefix):

```shell
pixi run install
# or specify a prefix
pixi run install -- --prefix /path/to/prefix
```

## Git hooks

This repository uses a [Conventional Commits](https://www.conventionalcommits.org)
commit message format, enforced by a `commit-msg` git hook tracked in the
`githooks/` directory. Enable it once after cloning:

```shell
pixi run setup-hooks
```

This runs `git config core.hooksPath githooks`. Commit messages must follow the
form:

```
<type>[optional scope][!]: <description>
```

where `<type>` is one of `feat`, `fix`, `docs`, `style`, `refactor`, `perf`,
`test`, `build`, `ci`, `chore` or `revert`. For example:

```
feat(parser): add support for anchors
fix: handle empty input without crashing
docs(readme): update build instructions
refactor!: drop deprecated API
```

Conventional commits are used to generate `CHANGELOG.md` with
[git-cliff](https://git-cliff.org). To update the changelog:

```shell
pixi run changelog
```

## Building the conda package

The conda recipe lives in `conda.recipe/`. Build the package into a local
directory with:

```shell
pixi run build-conda
# or specify the output directory
pixi run build-conda /path/to/output
```

The `conda.recipe/conda_build_config.yaml` file is automatically picked up as a
build variant configuration file by `pixi publish`.

## Issues

Feel free to submit issues and enhancement requests.

Please help us by providing **minimal reproducible examples**, because source
code is easier to let other people understand what happens. For crash problems
on certain platforms, please include stack dump content along with the details
of the OS, compiler, etc. Please try breakpoint debugging first, tell us what
you found, and see if we can start exploring based on more information that has
been prepared.

## Workflow

In general, we follow the "fork-and-pull" git workflow.

1. **Fork** the repo on GitHub
2. **Clone** the project to your own machine
3. **Checkout** a new branch on your fork and start developing on the branch
4. **Test** the change before committing. Make sure the changes pass all the
   tests, including `unittest` and `perftest`, and add a test case for each new
   feature or bug fix if needed.
5. **Commit** changes to your own branch, following the conventional commit
   format described above
6. **Push** your work back up to your fork
7. Submit a **Pull request** so that we can review your changes

NOTE: Be sure to merge the latest from "upstream" before making a pull request!

## Copyright and Licensing

YggdrasilRapidJSON is distributed under the MIT license (see `license.txt`).
By contributing, you agree that your contributions are licensed under the same
terms. RapidJSON itself is provided by Tencent under the MIT license:

```
Tencent is pleased to support the open source community by making RapidJSON available.

Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.

Licensed under the MIT License (the "License"); you may not use this file except
in compliance with the License. You may obtain a copy of the License at

http://opensource.org/licenses/MIT

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
```
