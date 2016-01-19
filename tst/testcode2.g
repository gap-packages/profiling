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
	Intersection(AlternatingGroup(8), AlternatingGroup(8)*(1,2));
	return x;
end;

g(2);
g(-2);
