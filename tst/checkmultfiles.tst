gap> START_TEST("checkmultfiles.tst");
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
gap> f(-1);
3
gap> Read(Filename(testdir, "testcode1.g"));
gap> f(2);
2
gap> UnprofileLineByLine();
true
gap> x := ReadLineByLineProfile(file);;
gap> SortedList(RecNames(x));
[ "info", "line_function_calls", "line_info", "stack_runtimes" ]
gap> filenames := List(x.line_info, y -> y[1]);;
gap> file := Filtered(filenames, x -> EndsWith(x, "testcode1.g"));;
gap> Length(file);
1
gap> datapos := PositionProperty(filenames, x -> EndsWith(x, "testcode1.g"));;
gap> data := x.line_info[datapos];;
gap> data[2] = [ [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ],
>                [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ],
>                [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ], 
>                [ 1, 0, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 3, 0, 0 ], [ 1, 2, 0, 0 ] ];
true
gap> STOP_TEST("checkmultfiles.tst", 1);
