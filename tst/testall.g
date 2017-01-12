#
# profiling: Line by line profiling and code coverage for GAP
#
# This file runs package tests. It is also referenced in the package
# metadata in PackageInfo.g.
#
# Profiling changes the global state of GAP in various ways. Therefore
# we must use IO_fork to run each test in a separate instance of GAP.

LoadPackage( "profiling" );
LoadPackage( "io" );

IO_InstallSIGCHLDHandler();

failedtest := false;

dir := DirectoriesPackageLibrary( "profiling", "tst" )[1];
files := DirectoryContents(dir);
files := List(files, x -> Filename(dir, x));
tstfiles := Filtered(files, x -> EndsWith(x,".tst"));
for t in tstfiles do
    fork := IO_fork();
    if fork = fail then
        Print("Fork failed. Emergency exit\n");
        FORCE_QUIT_GAP(1);
    fi;

    if fork = 0 then
        TestDirectory([t], rec(exitGAP := true));
    else
        ret := IO_WaitPid(fork, true);
        if ret.status <> 0 then
            failedtest := true;
        fi;
    fi;
od;

if failedtest then
    Print("A test failed!\n");
    FORCE_QUIT_GAP(1);
else
    FORCE_QUIT_GAP(0);
fi;
