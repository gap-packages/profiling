# This code is just some test code, for checking profiling

g := function(a)
	local x;
	x := 0;
	if a < 1 then
		x := 3;
	fi;
	if a > 1 then
		x := 2;
	fi;
	if a > 2 then
		x := 1;
	fi;
	return x;
end;

g(2);
g(-2);
