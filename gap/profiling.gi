#
# profiling: Line by line profiling and code coverage for GAP
#
# Implementations
#
InstallGlobalFunction( "ReadLineByLineProfile",
function(filename)
  local f, res;
  if IsLineByLineProfileActive() then
    Info(InfoWarning, 1, "Reading Profile while still generating it!");
  fi;
  f := IO_CompressedFile(filename, "r");
  res := READ_PROFILE_FROM_STREAM(f, 0);
  IO_Close(f);
  return res; 
end );

# This internal function just pretty prints a function object
_Prof_PrettyPrintFunction := function(f)
  return Concatenation(f.name, "@", f.filename, ":", String(f.line));
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


##
InstallGlobalFunction("OutputAnnotatedCodeCoverageFiles",function(data, indir, outdir)
    local infile, outname, instream, outstream, line, allLines, 
          counter, overview, i, fileinfo, filenum, callinfo,
          readlineset, execlineset, outchar,
          outputhtml, outputoverviewhtml, LookupWithDefault,
          warnedExecNotRead;
    
    warnedExecNotRead := false;
    
    LookupWithDefault := function(dict, val, default)
        local v;
        v := LookupDictionary(dict, val);
        if v = fail then
            return default;
        else
            return v;
        fi;
    end;
    
    outputhtml := function(lines, coverage, subfunctions, outstream)
      local i, outchar, str, time, totaltime, calledfns, linkname, fn, name;
      PrintTo(outstream, "<html><body>\n",
        "<style>\n",
        ".linenum { text-align: right; border-right: 3px solid #FFFFFF; }\n",
        ".exec { border-right: 3px solid #2EFE2E; }\n",
        ".missed { border-right: 3px solid #FE2E64; }\n",
        ".ignore { border-right: 3px solid #BDBDBD; }\n",
        " td {border-right: 5px solid #FFFFFF;}\n",
        "}\n",
        "</style>\n",
        "<table cellspacing='0' cellpadding='0'>\n");
      
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
        PrintTo(outstream, "<a name=\"line",i,"\"></a><tr>");
        time := "";
        if IsBound(coverage[i]) and coverage[i][2] >= 1 then
          time := String(coverage[i][2]) ;
          if coverage[i][3] >= 1 or coverage[i][4] >= 1 then
            time := Concatenation(time, " in ",String(coverage[i][3]),"ns (", String(coverage[i][4]), "ns in children)");
          fi;
        fi;
        totaltime := "";
        # totaltime := LookupWithDefault(linedict.recursetime, i, "");
        calledfns := "";
        if Length(subfunctions) >= i then
          for fn in subfunctions[i] do
            linkname := ReplacedString(fn.filename, "/", "_");
            Append(linkname, ".html");
            name := fn.name;
            if name = "nameless" then
              name := Concatenation(fn.filename, String(fn.line));
            fi;
            Append(calledfns, Concatenation("<a href=\"",linkname,"#line",String(fn.line),"\">",name,"</a> "));
          od;
        fi;
        
        PrintTo(outstream, "<td><p class='linenum ",outchar,"'>",i,"</p></td>");
        PrintTo(outstream, "<td>",time,"</td><td>",totaltime,"</td>");
        PrintTo(outstream, "<td><span><tt>",str,"</tt></span></td>");
        PrintTo(outstream, "<td><span>",calledfns,"</span></td");
        PrintTo(outstream, "</tr>");
      od;
            
      PrintTo(outstream,"</table></body></html>");
    end;
    
    outputoverviewhtml := function(overview, outdir)
      local filename, outstream, codecover, i;
      
      Sort(overview, function(v,w) return v.inname < w.inname; end);
      
      filename := Concatenation(outdir, "/index.html");
      outstream := OutputTextFile(filename, false);
      SetPrintFormattingStatus(outstream, false);
      PrintTo(outstream, "<html><body>\n",
        "<style>\n</style>\n",
        "<table cellspacing='0' cellpadding='0'>\n",
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
            outname := ReplacedString(infile, "/", "_");
            outname := Concatenation(outdir, "/", outname);
            outname := Concatenation(outname, ".html");
            instream := InputTextFile(infile);
            outstream := OutputTextFile(outname, false);
            SetPrintFormattingStatus(outstream, false);
            allLines := [];
            line := ReadLine(instream);
            while line <> fail do
              Add(allLines, line);
              line := ReadLine(instream);
            od;
            CloseStream(instream);
            
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
    # Output an overview page
    outputoverviewhtml(overview, outdir);
end);

