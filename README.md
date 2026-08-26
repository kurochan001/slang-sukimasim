> **Fork Notice**
> This repository is a fork of [MikePopoloski/slang](https://github.com/MikePopoloski/slang).
> Purpose: Experimentation and feature additions for developing a custom SystemVerilog simulator (sukimasim).
> Upstream sync: Regularly synced with upstream/master, with contributions back to upstream when possible.

slang-sukimasim - SystemVerilog Language Services (Fork)
=========================================================
![](https://github.com/MikePopoloski/slang/workflows/CI%20Build/badge.svg)
[![codecov](https://codecov.io/gh/MikePopoloski/slang/branch/master/graph/badge.svg)](https://codecov.io/gh/MikePopoloski/slang)
[![PyPI](https://img.shields.io/pypi/v/pyslang.svg)](https://pypi.org/project/pyslang/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/MikePopoloski/slang/blob/master/LICENSE)

slang-sukimasim is an experimental fork of the original slang, with additional features for developing a custom SystemVerilog simulator. While inheriting slang's fast and robust frontend capabilities, it includes unique improvements aimed at simulator implementation.

The original slang is a software library that provides various components for lexing, parsing, type checking, and elaborating SystemVerilog code.

slang is the fastest and most compliant SystemVerilog frontend (according to the open source [chipsalliance test suite](https://github.com/chipsalliance/sv-tests)).

Full documentation is available on the website: https://sv-lang.com

### Features
-   Fully parse, elaborate, and analyze all SystemVerilog features.
-   Be robust about compilation, no matter how broken the source text. This makes the compiler usable in editor highlighting and completion scenarios, where the code is likely to be broken because the user is still writing it.
-   The parse tree should round trip back to the original source, making it easy to write refactoring and code generation tools.
-   Provide great error messages, ala clang.
-   Be fast and robust in the face of production-scale projects.

### Use Cases
Some examples of things you can use slang for:
-   Very fast syntax checking and linting tool
-   Dumping the AST of your project to JSON
-   Source code introspection via included Python bindings
-   SystemVerilog code generation and refactoring
-   As the engine for an editor language server
-   As a fast and robust preprocessor that sits in front of downstream tools
-   As a frontend for a synthesis or simulation tool, by including slang as a library

### Fork Differences from Upstream

Main changes in this fork:
- **Phase 1**: DeclarationsAtStart improvements, IEEE 1800-2023 functions added
- **Phase 2**: Covergroup parsing improvements (reserved keyword issue fixes)
- **Experimental features**: Internal structure investigation and improvements for simulator implementation
- **Compatibility**: Full compatibility with upstream maintained (no breaking changes)

See documentation in the [docs/](docs/) directory for details.

### Getting Started

Pre-built binaries for Linux, macOS, and Windows are available on the [releases page](https://github.com/MikePopoloski/slang/releases).

Instructions on building slang from source are [here](https://sv-lang.com/building.html). The tl;dr is:
```
git clone https://github.com/kurochan001/slang-sukimasim.git
cd slang-sukimasim
cmake -B build
cmake --build build -j32
```

The slang binary can be run on your code right out of the box; check out the [user manual](https://sv-lang.com/user-manual.html) for more information about how it works.

If you're looking to use slang as a library, please read through the [developer guide](https://sv-lang.com/developer-guide.html).

The project also includes Python bindings for the library, which can be installed via PyPI:
```
pip install pyslang
```
or, to update your installed version to the latest release:
```
pip install -U pyslang
```
or, to checkout and install a local build:
```
git clone https://github.com/kurochan001/slang-sukimasim.git
cd slang-sukimasim
pip install .
```

### Try It Out

Experiment with parsing, type checking, and error detection live [on the web](https://sv-lang.com/explore/) (inspired by Matt Godbolt's excellent [Compiler Explorer](https://godbolt.org/)).

### Upstream Sync

```bash
# Set up upstream (first time only)
git remote add upstream https://github.com/MikePopoloski/slang.git

# Sync with upstream
git fetch upstream --prune --tags
git switch master && git merge upstream/master
git push origin master
```

Regularly synced with upstream/master. Generic improvements from this fork will be considered for upstream PRs.

### Contact & Support

- **Fork-specific issues**: File in this repository's Issues
- **Core slang issues**: File in the [upstream repository](https://github.com/MikePopoloski/slang)
- **Simulator implementation questions**: Use this repository's Discussions

Contributions are welcome, whether they be in the form of bug reports, comments, suggestions, documentation improvements, or full fledged new features via pull requests.

### License & Credits

This fork is provided under the same MIT license as the original slang.

#### Original Author
>   Copyright (c) 2015-2025 Michael Popoloski

#### Fork Modifications
>   Copyright (c) 2025 kurochan001 (sukimasim fork)

Special thanks to Michael Popoloski, the original author of slang, and all contributors to the slang community.

---

**Original License (MIT):**

>   Copyright (c) 2015-2026 Michael Popoloski
>
>   Permission is hereby granted, free of charge, to any person obtaining a copy
>   of this software and associated documentation files (the "Software"), to deal
>   in the Software without restriction, including without limitation the rights
>   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
>   copies of the Software, and to permit persons to whom the Software is
>   furnished to do so, subject to the following conditions:
>
>   The above copyright notice and this permission notice shall be included in
>   all copies or substantial portions of the Software.
>
>   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
>   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
>   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
>   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
>   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
>   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
>   THE SOFTWARE.
