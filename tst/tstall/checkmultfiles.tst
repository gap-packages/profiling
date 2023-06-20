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
gap> SortedList(RecNames(x)) =
> [ "info", "line_calling_function_calls", "line_function_calls", "line_info", "stack_runtimes" ];
true
gap> filenames := List(x.line_info, y -> y[1]);;
gap> file := Filtered(filenames, x -> EndsWith(x, "testcode1.g"));;
gap> Length(file);
1
gap> datapos := PositionProperty(filenames, x -> EndsWith(x, "testcode1.g"));;
gap> data := x.line_info[datapos];;
gap> kernelVer := 0;;
gap> if IsBound(GAPInfo.KernelInfo.KERNEL_API_VERSION) then
> kernelVer := GAPInfo.KernelInfo.KERNEL_API_VERSION;
> fi;;
gap> if kernelVer >= 9000 then
> profData := [ [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 1, 1, 0, 0 ],
>  [ 0, 0, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ],
>  [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ],
>  [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ],
>  [ 1, 0, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 3, 0, 0 ],
>  [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ] ];
> elif kernelVer >= 8001 then
> profData := [ [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 1, 1, 0, 0 ],
>  [ 0, 0, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 1, 0, 0 ],
>  [ 1, 1, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 1, 0, 0 ],
>  [ 1, 1, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 1, 0, 0 ],
>  [ 1, 0, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 1, 0, 0 ],
>  [ 1, 1, 0, 0 ], [ 1, 1, 0, 0 ] ];
> elif kernelVer >= 6000 then
> profData := [ [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 1, 1, 0, 0 ], [ 0, 0, 0, 0 ],
>  [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ],
>  [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ],
>  [ 1, 0, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 3, 0, 0 ], [ 1, 2, 0, 0 ],
>  [ 1, 1, 0, 0 ] ];
> elif kernelVer >= 4000 then
>  profData := [ [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ],
>                [ 1, 3, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ],
>                [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ], 
>                [ 1, 0, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 3, 0, 0 ], [ 1, 2, 0, 0 ],
>                [ 1, 1, 0, 0 ] ];
> else
>  profData := [ [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ], [ 0, 0, 0, 0 ],
>                [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ],
>                [ 1, 2, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 2, 0, 0 ],
>                [ 1, 0, 0, 0 ], [ 1, 2, 0, 0 ], [ 1, 3, 0, 0 ], [ 1, 2, 0, 0 ] ];
> fi;;
gap> data[2] = profData;
true
gap> STOP_TEST("checkmultfiles.tst", 1);
