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
  if LoadKernelExtension("profiling") = false then
      Error("failed to load profiling kernel extension");
  fi;


ReadPackage( "profiling", "gap/profiling.gd");
