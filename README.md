[![Build Status](https://github.com/gap-packages/profiling/workflows/CI/badge.svg?branch=master)](https://github.com/gap-packages/profiling/actions?query=workflow%3ACI+branch%3Amaster)
[![Code Coverage](https://codecov.io/github/gap-packages/profiling/coverage.svg?branch=master&token=)](https://codecov.io/gh/gap-packages/profiling)

# The GAP 4 package `profiling'

This package provides line-by-line profiling of GAP, allowing
both discovering which lines of code take the most time, and
which lines of code are even executed.

The main function provided by this package is
`OutputAnnotatedCodeCoverageFiles`, which takes a previously
generated profile (using `ProfileLineByLine` or `CoverageLineByLine`,
both provided by the GAP library), and outputs human-readable
HTML files.

There is also `OutputFlameGraph`, which outputs a graphical diagram
showing which functions took the most time during exection.

## Building

### Requirements

This package requires the 'IO' package is installed and compiled.
Check it can be loaded by running `LoadPackage("io");` before trying
to compile and run 'profiling'.

This package also requires a C++ compiler (typically clang++ or g++)

### Build Instructions For Release

The package should be built using the commands:

    ./configure
    make

Optionally, if this package is not within GAP's pkg directory,
you can use the following notation to tell configure where GAP
is located.

    ./configure <location of GAP root>
    make
