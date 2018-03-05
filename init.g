#
# profiling: Line by line profiling and code coverage for GAP
#
# Reading the declaration part of the package.
#

# load a method for UserHomeExpand if necessary
if not IsBound(UserHomeExpand) then
  BindGlobal("UserHomeExpand", USER_HOME_EXPAND);
fi;

# load kernel function if it is installed:
_PATH_SO:=Filename(DirectoriesPackagePrograms("profiling"), "profiling.so");
if _PATH_SO <> fail then
    LoadDynamicModule(_PATH_SO);
fi;
Unbind(_PATH_SO);

ReadPackage( "profiling", "gap/profiling.gd");
