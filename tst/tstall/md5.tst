gap> START_TEST("md5.tst");
gap> MD5File(fail);
Error, MD5File: <filename> must be a string
gap> MD5File("DOES_NOT_EXIST");
Error, MD5File: failed to open file DOES_NOT_EXIST
gap> filename := Filename(DirectoriesPackageLibrary("profiling", "tst/tstall"), "md5.sample");;
gap> MD5File(filename);
"91785c4eeb49934bdaef739a6e2a2710"
gap> STOP_TEST("md5.tst", 1);
