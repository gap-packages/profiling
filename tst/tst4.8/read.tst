gap> START_TEST("read.tst");
gap> LoadPackage("profiling", false);
true
gap> ReadLineByLineProfile(6);
Error, no method found! For debugging hints type ?Recovery from NoMethodFound
Error, no 1st choice method found for `Length' on 1 arguments
gap> ReadLineByLineProfile("filethatdoesnotexist.cheese");
Error, Unable to open file filethatdoesnotexist.cheese
gap> ReadLineByLineProfile("/");
fail
gap> STOP_TEST("read.tst", 1);
