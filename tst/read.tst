gap> START_TEST("read.tst");
gap> LoadPackage("profiling", false);
true
gap> ReadLineByLineProfile(6);
Error, Filename must be a string
gap> ReadLineByLineProfile("filethatdoesnotexist.cheese");
Error, Unable to open file filethatdoesnotexist.cheese
gap> ReadLineByLineProfile("/");
fail
gap> STOP_TEST("read.tst", 1);