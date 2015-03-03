#
# profiling: Line by line profiling and code coverage for GAP
#
# Reading the declaration part of the package.
#
_PATH_SO:=Filename(DirectoriesPackagePrograms("profiling"), "profiling.so");
if _PATH_SO <> fail then
    LoadDynamicModule(_PATH_SO);
fi;
Unbind(_PATH_SO);

ReadPackage( "profiling", "gap/profiling.gd");
