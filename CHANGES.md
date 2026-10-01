## 2.6.3 (2026-07-29)

* Remove unneeded dependancy on GAPDoc
* Many changes to CI (does not effect usage)

## 2.6.2 (2025-06-21)

* Internal changes to prepare for future GAP versions

## 2.6.1 (2025-06-19)

* Internal changes to support new versions of gcc

## 2.6.0 (2024-08-27)

* Improve output formatting
* Overhaul CI

## 2.5.4 (2023-06-29)

* Quick release just to fix incorrect release date

## 2.5.3 (2023-06-28)

* Improve error messages when profiles are corrupted
* Remove 32bit testing
* Fix testing for latest GAP kernel

## 2.5.2 (2022-12-20)

* Support latest changes to GAP kernel (4.13)

## 2.5.1 (2022-10-10)

* Support latest changes to GAP kernel (4.12)

## 2.5.0 (2022-02-22)

* Improve WSL support
* Overhaul build system

## 2.4.1 (2021-02-14)

* Support LineByLineProfileFunction in WSL

## 2.4.0

* Provide multiple flamegraphs, to give different viewpoints
* Improve documentation and tutorial
* Support Lcov format output

## 2.3.0

* Fix rare crash
* Update build system

## 2.2.1 (2019-03-15)

* Fix typo which broke compiling

## 2.2.0

* Use better MD5 code
* Clean up building
* Add tutorial

## 2.0.1 (2018-05-06)

* Further performance improvements
* Fix memory leak which tended to cause crashes with very large profiles
* Update how we find GMP (should not effect users)

## 2.0.0 (2018-03-29)

* Improved performance
* Show where functions were called from
* Output a general overview page

## 1.3.0 (2017-02-23)

* Fix running in GAP 4.8

## 1.2.0 (2017-01-11)

* Fix 32-bit builds of profiling package

## 1.1.0 (2016-11-01)

* Tweak how perl programs are run (again)

## 0.6.2 (2016-10-18)

* Improve formatting for files with only coverage

## 0.6.1 (2016-10-10)

* Handle executable perl scripts

## 0.6.0 (2016-10-10)

Bug Fixes

* Don't assume python is 0.6.0
* Handle bad filenames

New functionality

* Add 'LineByLineProfileFunction', a quick way to profile a function call.
* Add commas to numbers and align nicer
* Add support for codecov.io JSON output

## 0.5.1 (2016-02-24)

Several small tweaks to improve HTML output quality

## 0.5.0 (2016-02-03)

New features:

* Add MergeLineByLineProfiles, a function to merge multiple profiles

Bug Fixes:

* FlameGraph generation was broken, due to a hard-wired directory.

Minor improvements:

* Do not print filetime and file executed statements, when they are defined for no file

## 0.4.0 (2016-01-19)

Bugs

* Fix occasional crash (Thanks to Horvath Gabor for report and test case)
