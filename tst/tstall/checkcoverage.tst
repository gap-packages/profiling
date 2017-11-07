gap> START_TEST("genprof.tst");
gap> IsLineByLineProfileActive();
false
gap> LoadPackage("IO", false);
true
gap> LoadPackage("profiling", false);
true
gap> dir := DirectoryTemporary();;
gap> file := Filename(dir, "cheese.gz");;
gap> testdir:= DirectoriesPackageLibrary( "profiling", "tst" )[1];;
gap> Read(Filename(testdir, "testcode1.g"));
gap> CoverageLineByLine(file);
true
gap> Read(Filename(testdir, "testcode2.g"));
gap> f(1);;
gap> f(-1);;
gap> UnprofileLineByLine();
true
gap> OutputAnnotatedCodeCoverageFiles(file, Filename(dir, "outdir"));
# Warning: Some lines marked executed but not read. If you
# want to see which lines are NOT executed,
# use the --prof/--cover command line options
gap> IsReadableFile(Filename(dir, "outdir/index.html"));
true
gap> x := ReadLineByLineProfile(file);;
gap> OutputAnnotatedCodeCoverageFiles(x, Filename(dir, "outdir2"));
# Warning: Some lines marked executed but not read. If you
# want to see which lines are NOT executed,
# use the --prof/--cover command line options
gap> IsReadableFile(Filename(dir, "outdir2/index.html"));
true
gap> STOP_TEST("genprof.tst", 1);
