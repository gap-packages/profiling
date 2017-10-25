gap> START_TEST("colour.tst");
gap> LoadPackage("profiling", false);
true
gap> f := function(x) local w; if x < 0 then w := 3; else w := 4; fi; end;
function( x ) ... end
gap> Print(f,"\n");
function ( x )
    local w;
    if x < 0 then
        w := 3;
    else
        w := 4;
    fi;
    return;
end
gap> ActivateProfileColour(true);
[0mtrue
gap> Print(f,"\n");
function ( x )
    local w;
    [31m[31mif [31mx < 0[31m then
        [31mw := 3;[31m
    else
        [31mw := 4;[31m
    fi;[31m
    [31mreturn;[31m[0m
end
gap> f(0);;
gap> Print(f,"\n");
function ( x )
    local w;
    [31m[31mif [31mx < 0[31m then
        [31mw := 3;[31m
    else
        [31mw := 4;[31m
    fi;[31m
    [31mreturn;[31m[0m
end
gap> f(1);;
gap> Print(f,"\n");
function ( x )
    local w;
    [31m[31mif [31mx < 0[31m then
        [31mw := 3;[31m
    else
        [31mw := 4;[31m
    fi;[31m
    [31mreturn;[31m[0m
end
gap> f := function(x) local w; if x < 0 then w := 3; else w := 4; fi; end;
function( x ) ... end
gap> Print(f,"\n");
function ( x )
    local w;
    [31m[31mif [31mx < 0[31m then
        [31mw := 3;[31m
    else
        [31mw := 4;[31m
    fi;[31m
    [31mreturn;[31m[0m
end
gap> ActivateProfileColour(false);
[0mtrue
gap> Print(f,"\n");
function ( x )
    local w;
    if x < 0 then
        w := 3;
    else
        w := 4;
    fi;
    return;
end
gap> STOP_TEST("colour.tst", 1);
