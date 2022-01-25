# ProfileFile
gap> tmpdir := DirectoryTemporary();;
gap> quicktest := Filename(tmpdir, "quicktest.tst");;
gap> runquicktest := Filename(tmpdir, "runquicktest.g");;
gap> IsPosInt(FileString(quicktest, "gap> Length([1,2,3]);\n3\n"));
true
gap> IsPosInt(FileString(runquicktest, StringFormatted("Test(\"{}\");\n", quicktest)));
true
gap> Test(quicktest);
true
gap> Read(runquicktest);
gap> str := ProfileFile(quicktest, rec(showOutput := false));;
gap> EndsWith(str, ".html");
true
gap> str := ProfileFile(runquicktest);;
gap> EndsWith(str, ".html");
true

# ProfilePackage
gap> str := ProfilePackage("transgrp", rec(indir := "", showOutput := false));;
gap> EndsWith(str, ".html");
true
gap> ProfilePackage("io", 2, 3);
Error, ProfilePackage: takes 1 or 2 arguments, but 3 were given
