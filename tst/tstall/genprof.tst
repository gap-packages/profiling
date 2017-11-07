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
gap> OutputFlameGraph(file, Filename(dir, "flame"));
gap> IsReadableFile(Filename(dir, "flame"));
true
gap> x := ReadLineByLineProfile(file);;
gap> OutputAnnotatedCodeCoverageFiles(x, Filename(dir, "outdir2"));
gap> IsReadableFile(Filename(dir, "outdir2/index.html"));
true
gap> OutputFlameGraph(x, Filename(dir, "flame2"));
gap> IsReadableFile(Filename(dir, "flame2"));
true
gap> STOP_TEST("genprof.tst", 1);
