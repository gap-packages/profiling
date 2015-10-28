gap> START_TEST("genprof.tst");
gap> IsLineByLineProfileActive();
false
gap> LoadPackage("IO", false);
true
gap> dir := DirectoryTemporary();;
gap> file := Filename(dir, "cheese.gz");;
gap> ProfileLineByLine(file);
true
gap> Intersection(Group((1,2),(3,4)), Group((1,2,3)));
Group(())
gap> UnprofileLineByLine();
true
gap> OutputAnnotatedCodeCoverageFiles(file, Filename(dir, "outdir"));
gap> IsReadableFile(Filename(dir, "outdir/index.html"));
true
gap> STOP_TEST("genprof.tst", 1);
