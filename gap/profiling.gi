#
# profiling: Line by line profiling and code coverage for GAP
#
# Implementations
#
InstallGlobalFunction( "ReadLineByLineProfile",
function(filename)
  local f, res;
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

#############################################################################
##
##
##  <#GAPDoc Label="OutputAnnotatedCodeCoverageFiles">
##  <ManSection>
##  <Func Name="OutputAnnotatedCodeCoverageFiles" Arg="cover, indir, outdir"/>
##
##  <Description>
##  <Ref Func="OutputAnnotatedCodeCoverageFiles"/> takes <A>cover</A> (an output of
##  <Ref Func="ReadLineByLineProfile"/>), and two directory names. It outputs a copy
##  of each file in <A>cover</A> which is contained in <A>indir</A>
##  into <A>outdir</A>, annotated with which lines were executed.
##  <A>indir</A> may also be the name of a single file, in which case
##  only code coverage for that file is produced.
##  </Description>
##  </ManSection>
##  <#/GAPDoc>
##
BIND_GLOBAL("OutputAnnotatedCodeCoverageFilesNEW",function(data, indir, outdir)
    local infile, outname, instream, outstream, line, allLines, 
          counter, overview, i, fileinfo,
          readlineset, execlineset, outchar,
          outputhtml, outputoverviewhtml, LookupWithDefault;
    
    LookupWithDefault := function(dict, val, default)
        local v;
        v := LookupDictionary(dict, val);
        if v = fail then
            return default;
        else
            return v;
        fi;
    end;
    
    outputhtml := function(lines, coverage, outstream)
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
        if not(IsBound(coverage[i])) or (coverage[i] = [0,0,0]) then
          outchar := "ignore";
        elif coverage[i][2] = 1 then
          outchar := "exec";
        elif coverage[i][1] = 1 then
          outchar := "missed";
        else
          Error("Internal error");
        fi;
        
        str := List(lines[i]);
        str := ReplacedString(str, "&", "&amp;");
        str := ReplacedString(str, "<", "&lt;");
        str := ReplacedString(str, " ", "&nbsp;");
        PrintTo(outstream, "<a name=\"line",i,"\"></a><tr>");
        time := 0;
        if IsBound(coverage[i]) and coverage[i][2] = 1 then
          time := String(coverage[i][3]);
        fi;
        totaltime := "";
        # totaltime := LookupWithDefault(linedict.recursetime, i, "");
        calledfns := "";
#        for fn in LookupWithDefault(linedict.calledfuncs, i, []) do
#          linkname := ReplacedString(fn.file, "/", "_");
#          Append(linkname, ".html");
#          name := fn.shortname;
#          if name = "nameless" then
#            name := fn.longname;
#          fi;
#          Append(calledfns, Concatenation("<a href=\"",linkname,"#line",fn.line,"\">",name,"</a> "));
#        od;
        
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
        "<tr><td valign='top'>\n");
      
      for i in [1..Length(overview)] do
        PrintTo(outstream, "<p><a href='",
           Remove(SplitString(overview[i].outname,"/")),
           "'>",overview[i].inname,"</a></p>");
      od;
      
      PrintTo(outstream, "</td><td class='text' valign='top'>");
      
      for i in overview do
        codecover := 1 - (i.readnotexeclines / (i.execlines + i.readnotexeclines));
        # We have to do a slightly horrible thing to get the formatting we want
        codecover := String(Floor(codecover*100.0));
        PrintTo(outstream, "<p>",codecover{[1..Length(codecover)-1]},"% (",
          i.execlines,"/",i.execlines + i.readnotexeclines,")</p>");
      od;
      
      PrintTo(outstream,"</td></tr></table></body></html>");
      CloseStream(outstream);
    end;
    
    overview := [];
    for fileinfo in data.line_info do
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
            
            
            Add(overview, rec(outname := outname, inname := infile,
            execlines := Length(Filtered(fileinfo[2], x -> (x[2] = 1))),
            readnotexeclines := Length(Filtered(fileinfo[2], x -> (x[1] = 1 or x[2] = 1)))));
            outputhtml(allLines, fileinfo[2], outstream);

            CloseStream(outstream);
        fi;
    od;    
    # Output an overview page
    outputoverviewhtml(overview, outdir);
end);

