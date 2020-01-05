gap> START_TEST("checknoreturn.tst");
gap> IsLineByLineProfileActive();
false
gap> LoadPackage("IO", false);
true
gap> LoadPackage("profiling", false);
true
gap> dir := DirectoryTemporary();;
gap> file := Filename(dir, "out.gz");;
gap> testdir:= DirectoriesPackageLibrary( "profiling", "tst" )[1];;
gap> Read(Filename(testdir, "testcodenoreturn.g"));
gap> ProfileLineByLine(file);
true
gap> f1(0);
1
gap> f2(1);
gap> UnprofileLineByLine();
true
gap> x := ReadLineByLineProfile(file);;
gap> SortedList(RecNames(x)) = 
> [ "info", "line_calling_function_calls", "line_function_calls", "line_info", "stack_runtimes" ];
true
gap> filenames := List(x.line_info, y -> y[1]);;
gap> file := Filtered(filenames, x -> EndsWith(x, "testcodenoreturn.g"));;
gap> Length(file);
1
gap> datapos := PositionProperty(filenames, x -> EndsWith(x, "testcodenoreturn.g"));;
gap> data := x.line_info[datapos][2];;
gap> sample := [ [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 1, 1, 0 ],
>  [ 0, 1, 1, 0 ], [ 0, 1, 21, 0 ], [ 0, 1, 2, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ],
>  [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 1, 2, 0 ], [ 0, 1, 7, 0 ], [ 0, 0, 0, 0 ],
>  [ 0, 1, 0, 0 ] ];;
gap> Filtered([1..Length(data)], x -> not (data[x]{[1..2]} = sample[x]{[1..2]}) );
[  ]
gap> STOP_TEST("checknoreturn.tst", 1);
