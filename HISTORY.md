2.6.2
=====

* Internal changes to prepare for future GAP versions

2.6.1
=====

* Internal changes to support new versions of gcc

2.6.0
=====

* Improve output formatting
* Overhaul CI

2.5.4
=====

* Quick release just to fix incorrect release date

2.5.3
=====

* Improve error messages when profiles are corrupted
* Remove 32bit testing
* Fix testing for latest GAP kernel

2.5.2
=====

* Support latest changes to GAP kernel (4.13)

2.5.1
=====

* Support latest changes to GAP kernel (4.12)

2.5.0
=====

* Improve WSL support
* Overhaul build system

2.4.1
=====

* Support LineByLineProfileFunction in WSL

2.4.0
=====

* Provide multiple flamegraphs, to give different viewpoints
* Improve documentation and tutorial
* Support Lcov format output

2.3.0
=====

* Fix rare crash
* Update build system

2.2.1
=====

* Fix typo which broke compiling

2.2.0
=====

* Use better MD5 code
* Clean up building
* Add tutorial


2.0.1
=====

* Further performance improvements
* Fix memory leak which tended to cause crashes with very large profiles
* Update how we find GMP (should not effect users)

2.0.0
=====

* Improved performance
* Show where functions were called from
* Output a general overview page

1.3.0
=====

* Fix running in GAP 4.8

1.2.0
=====

* Fix 32-bit builds of profiling package

1.1.0
=====

* Tweak how perl programs are run (again)


0.6.2
=====

* Improve formatting for files with only coverage

0.6.1
=====

* Handle executable perl scripts

0.6.0
=====

Bug Fixes

* Don't assume python is 0.6.0
* Handle bad filenames

New functionality

* Add 'LineByLineProfileFunction', a quick way to profile a function call.
* Add commas to numbers and align nicer
* Add support for codecov.io JSON output

0.5.1
=====

Several small tweaks to improve HTML output quality

0.5.0
=====

New features:

* Add MergeLineByLineProfiles, a function to merge multiple profiles

Bug Fixes:

* FlameGraph generation was broken, due to a hard-wired directory.

Minor improvements:

* Do not print filetime and file executed statements, when they are defined for no file


0.4.0
=====

Bugs
----

* Fix occasional crash (Thanks to Horvath Gabor for report and test case)
