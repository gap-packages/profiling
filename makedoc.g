#
# profiling: Line by line profiling and code coverage for GAP
#
# This file is a script which compiles the package manual.
#
if fail = LoadPackage("AutoDoc", ">= 2016.01.21") then
    Error("AutoDoc 2016.01.21 or newer is required");
fi;

AutoDoc( rec( scaffold := true,
            autodoc := rec( files := [ "doc/tutorial.autodoc" ] ),
) );

QUIT;
