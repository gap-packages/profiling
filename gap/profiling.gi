#
# profiling: Line by line profiling and code coverage for GAP
#
# Implementations
#
InstallGlobalFunction( "ReadLineByLineProfile",
function(filename)
  local res;
  if IsLineByLineProfileActive() then
    Info(InfoWarning, 1, "Reading Profile while still generating it!");
  fi;
  res := READ_PROFILE_FROM_STREAM(USER_HOME_EXPAND(filename), 0);
  return res;
end );

# This internal function just pretty prints a function object
_Prof_PrettyPrintFunction := function(f)
  return Concatenation(f.name, "@", f.filename, ":", String(f.line));
end;

# This just makes it easy to give dictionaries a default value 
_prof_LookupWithDefault := function(dict, val, default)
    local v;
    v := LookupDictionary(dict, val);
    if v = fail then
        return default;
    else
        return v;
    fi;
end;
    
#############################################################################
##
##
##  <#GAPDoc Label="OutputFlameGraph">
##  <ManSection>
##  <Func Name="OutputFlameGraph" Arg="cover, filename"/>
##
##  <Description>
##  <Ref Func="OutputFlameGraph"/> takes <A>cover</A> (an output of
##  <Ref Func="ReadLineByLineProfile"/>), and a file name. It translates
##  profiling information in <A>cover</A> into a suitable format to
##  generate flame graphs.
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
InstallGlobalFunction("OutputFlameGraph",function(data, filename)
  local outstream, trace, fun, firstpass;
  outstream := OutputTextFile(filename, false);
  if outstream = fail then
    ErrorMayQuit("Unable to write to file ", outstream);
  fi;

  if not(IsRecord(data)) then
    data := ReadLineByLineProfile(data);
  fi;

  SetPrintFormattingStatus(outstream, false);
  for trace in data.stack_runtimes do
    firstpass := true;
    for fun in trace[1] do
      if firstpass = true then
        firstpass := false;
      else
        PrintTo(outstream, ";");
      fi;
      PrintTo(outstream, _Prof_PrettyPrintFunction(fun));
    od;
    PrintTo(outstream, " ", String(trace[2]), "\n");
  od;
  CloseStream(outstream);
end);


# The CSS we want to inject into every page
_prof_CSS :=
"""<style>
table { border-collapse: collapse }
tr .linenum { text-align: right; }
tr:nth-child(odd)  { background-color: #EEE; }
tr:nth-child(even)  { background-color: #FFF; }
tr:nth-child(odd).exec  { background-color: #0F0; }
tr:nth-child(even).exec  { background-color: #3F3; }
tr:nth-child(odd).missed  { background-color: #F00; }
tr:nth-child(even).missed  { background-color: #F33; }
td, th {
    border: 1px solid #98bf21;
    padding: 3px 7px 2px 7px;
}
th {
    font-size: 1.1em;
    text-align: left;
    padding-top: 5px;
    padding-bottom: 4px;
    background-color: #A7C942;
    color: #ffffff;
}
table.sortable th:not(.sorttable_sorted):not(.sorttable_sorted_reverse):not(.sorttable_nosort):after {
    content: " \25B4\25BE"
}
</style>""";

##
InstallGlobalFunction("OutputAnnotatedCodeCoverageFiles",function(arg)
    local data, indir, outdir,
          infile, outname, instream, outstream, line, allLines,
          counter, overview, i, fileinfo, filenum, callinfo,
          readlineset, execlineset, outchar,
          outputhtml, outputoverviewhtml,
          warnedExecNotRead, filebuf;

    if Length(arg) < 2 or Length(arg) > 3 then
      ErrorMayQuit("Usage: OutputAnnotatedCodeCoverageFiles(data, [indir,] outdir)");
    fi;

    data := arg[1];
    if Length(arg) = 2 then
      indir := "";
      outdir := arg[2];
    else
      indir := arg[2];
      outdir := arg[3];
    fi;

    if IsDirectory(indir) then
      indir := indir![1];
    fi;

    if IsDirectory(outdir) then
      outdir := outdir![1];
    fi;

    indir := USER_HOME_EXPAND(indir);
    outdir := USER_HOME_EXPAND(outdir);

    # Try to make directory (might already exist)
    IO_mkdir(outdir, IO.S_IRUSR+IO.S_IWUSR+IO.S_IXUSR+
                                IO.S_IRGRP+IO.S_IXGRP+
                                IO.S_IROTH+IO.S_IXOTH);

    if IO_opendir(outdir) = fail then
      ErrorMayQuit("Unable to access directory ", outdir);
    fi;

    IO_closedir();

    if not(IsRecord(data)) then
      data := ReadLineByLineProfile(data);
    fi;

    warnedExecNotRead := false;

    # Don't bother warning about missing 'read' lines if we are just profiling
    if data.info.is_cover = false then
      warnedExecNotRead := true;
    fi;



    outputhtml := function(lines, coverage, subfunctions, outstream)
      local i, outchar, str, time, calls, calledfns, linkname, fn, name, filebuf;
      PrintTo(outstream, "<!DOCTYPE html><script src=\"sorttable.js\"></script><html><body>\n");
      PrintTo(outstream, _prof_CSS);

      PrintTo(outstream, "<table class=\"sortable\">\n");
      PrintTo(outstream, "<tr><th>Line</th><th>Execs</th><th>Time</th><th>Time+Childs</th><th>Code</th><th>Called Functions</th><tr>\n");
      for i in [1..Length(lines)] do
        if not(IsBound(coverage[i])) or (coverage[i] = [0,0,0,0]) then
          outchar := "ignore";
        elif coverage[i][2] >= 1 then
          outchar := "exec";
        elif coverage[i][1] >= 1 then
          outchar := "missed";
        else
          Error("Invalid profile - there were lines which were not executed, but took time!");
        fi;

        str := List(lines[i]);
        str := ReplacedString(str, "&", "&amp;");
        str := ReplacedString(str, "<", "&lt;");
        str := ReplacedString(str, " ", "&nbsp;");
        PrintTo(outstream, "<tr class='", outchar,"'>");
        time := "<td></td><td></td><td></td>";
        if IsBound(coverage[i]) and coverage[i][2] >= 1 then
          calls := coverage[i][2];
          if data.info.is_cover and calls > 1 then
            calls := 0;
          fi;

          if coverage[i][3] >= 1 or coverage[i][4] >= 1 then
            time := Concatenation("<td>",String(calls), "</td><td>",
                                  String(coverage[i][3]),"</td><td>",
                                  String(coverage[i][4]+coverage[i][3]), "</td>");
          else
            time := Concatenation("<td>",String(calls),"</td><td></td><td></td>");
          fi;
        fi;
        # totaltime := LookupWithDefault(linedict.recursetime, i, "");
        calledfns := "";
        if Length(subfunctions) >= i then
          for fn in subfunctions[i] do
            linkname := ReplacedString(fn.filename, "/", "_");
            Append(linkname, ".html");
            name := fn.name;
            if name = "nameless" then
              name := Concatenation(fn.filename, ":", String(fn.line));
            fi;
            Append(calledfns, Concatenation("<a href=\"",linkname,"#line",String(fn.line),"\">",name,"</a> "));
          od;
        fi;

        PrintTo(outstream, "<td><a name=\"line",i,"\"></a><div class='linenum'>",i,"</div></td>");
        PrintTo(outstream, time);
        PrintTo(outstream, "<td><span><tt>",str,"</tt></span></td>");
        PrintTo(outstream, "<td><span>",calledfns,"</span></td>");
        PrintTo(outstream, "</tr>\n");
      od;

      PrintTo(outstream,"</table></body></html>");
    end;

    outputoverviewhtml := function(overview, outdir)
      local filename, outstream, codecover, i;

      Sort(overview, function(v,w) return v.inname < w.inname; end);

      filename := Concatenation(outdir, "/index.html");
      outstream := OutputTextFile(filename, false);
      SetPrintFormattingStatus(outstream, false);
      PrintTo(outstream, "<!DOCTYPE html><script src=\"sorttable.js\"></script><html><body>\n");
      PrintTo(outstream, _prof_CSS);
      PrintTo(outstream, "<table cellspacing='0' cellpadding='0' class=\"sortable\">\n",
        "<tr><th>File</th><th>Coverage%</th><th>Coverage Lines</th><th>Time</th><th>Statements</th></tr>\n"
        );

      for i in overview do
        PrintTo(outstream, "<tr>");
        PrintTo(outstream, "<td><a href='",
           Remove(SplitString(i.outname,"/")),
           "'>",i.inname,"</a></td>");

        codecover := 1 - (i.readnotexeclines / (i.execlines + i.readnotexeclines));
        # We have to do a slightly horrible thing to get the formatting we want
        codecover := String(Floor(codecover*100.0));
        PrintTo(outstream, "<td>",codecover{[1..Length(codecover)-1]},"</td>");
        PrintTo(outstream, "<td>", i.execlines,"/",i.execlines + i.readnotexeclines,"</td>");
        PrintTo(outstream, "<td>",i.filetime, "</td><td>",i.fileexec,"</td>");
        PrintTo(outstream, "</tr>");
      od;

      PrintTo(outstream,"</table></body></html>");
      CloseStream(outstream);
    end;

    overview := [];
    for filenum in [1..Length(data.line_info)] do
        fileinfo := data.line_info[filenum];
        callinfo := data.line_function_calls[filenum];
        infile := fileinfo[1];
        if Length(indir) <= Length(infile)
                and indir = infile{[1..Length(indir)]} then
            # Make a nicer output filename, handling the input being in
            # directories, or having *s in the name.
            outname := infile;
            outname := ReplacedString(outname, "/", "_");
            outname := ReplacedString(outname, "*", "_");
            outname := Concatenation(outdir, "/", outname);
            outname := Concatenation(outname, ".html");
            outstream := OutputTextFile(outname, false);
            SetPrintFormattingStatus(outstream, false);

            # Check file exists. This also handles us accidentally trying
            # to open files like *stdin*
            if IsExistingFile(infile) then
              instream := InputTextFile(infile);
              allLines := [];
              line := ReadLine(instream);
              while line <> fail do
                Add(allLines, line);
                line := ReadLine(instream);
              od;
              CloseStream(instream);
            else
              allLines := List([1..Length(fileinfo[2])], x -> "<missing file>");
            fi;

            # Check for lines which are executed, but not read

            if ForAny(fileinfo[2], x -> (x[1] = 0 and x[2] > 0) and not warnedExecNotRead) then
              Print("Warning: There are statements in ", fileinfo[1],"\n",
                    "which are marked executed but not marked as read. Your profile may not\n",
                    "show lines which were read but not executed.\n",
                    "Please call ProfileLineByLine before loading any files you wish to profile.\n",
                    "You can use the --prof/--cover command line option to begin profiling\n",
                    "before GAP starts to profile library code.\n",
                    "(This warning will only be printed once.)\n");
              warnedExecNotRead := true;
            fi;

            Add(overview, rec(outname := outname, inname := infile,
            filetime := Sum(fileinfo[2], x -> x[3]),
            fileexec := Sum(fileinfo[2], x -> x[2]),
            execlines := Length(Filtered(fileinfo[2], x -> (x[2] >= 1))),
            readnotexeclines := Length(Filtered(fileinfo[2], x -> (x[1] >= 1 and x[2] = 0)))));
            outputhtml(allLines, fileinfo[2], callinfo[2], outstream);

            CloseStream(outstream);
        fi;
    od;

    filebuf := ReadAll(InputTextFile(Filename(DirectoriesPackageLibrary( "profiling", "data"), "sorttable.js")));
    outstream := OutputTextFile(Concatenation(outdir, "/sorttable.js"), false);
    SetPrintFormattingStatus(outstream, false);
    PrintTo(outstream, filebuf);
    CloseStream(outstream);

    # Output an overview page
    outputoverviewhtml(overview, outdir);
end);
