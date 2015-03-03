#
# profiling: Line by line profiling and code coverage for GAP
#
# This file is a script which compiles the package manual.
#
if fail = LoadPackage("AutoDoc", ">= 2014.03.27") then
    Error("AutoDoc version 2014.03.27 is required.");
fi;

AutoDoc( "profiling" : scaffold := true, autodoc := true );

PrintTo("VERSION", PackageInfo("profiling")[1].Version);

QUIT;
