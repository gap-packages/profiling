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
gap> OutputFlameGraph(file, Filename(dir, "flame3"), rec(type := "reverse"));
gap> IsReadableFile(Filename(dir, "flame3"));
true
gap> OutputFlameGraph(file, Filename(dir, "flame4"), rec(squash := true));
gap> IsReadableFile(Filename(dir, "flame4"));
true
gap> OutputFlameGraph(file, Filename(dir, "flame5"), rec(type := "reverse", squash := true));
gap> IsReadableFile(Filename(dir, "flame5"));
true
gap> x := ReadLineByLineProfile(file);;
gap> OutputAnnotatedCodeCoverageFiles(x, Filename(dir, "outdir2"));
gap> IsReadableFile(Filename(dir, "outdir2/index.html"));
true
gap> OutputAnnotatedCodeCoverageFiles(x, Filename(dir, "outdir3"), rec(title := "mytitle"));
gap> IsReadableFile(Filename(dir, "outdir3/index.html"));
true
gap> PositionSublist(StringFile(Filename(dir, "outdir3/index.html")), "mytitle") <> fail;
true
gap> OutputFlameGraph(x, Filename(dir, "flame2"));
gap> IsReadableFile(Filename(dir, "flame2"));
true
gap> STOP_TEST("genprof.tst", 1);
