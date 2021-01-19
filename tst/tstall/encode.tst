gap> HTMLEncodeString("");
""
gap> HTMLEncodeString("abc");
"abc"
gap> HTMLEncodeString("<");
"&lt;"
gap> HTMLEncodeString("&");
"&amp;"
gap> HTMLEncodeString(";");
";"
gap> HTMLEncodeString("\"");
"\""
gap> HTMLEncodeString(" ");
"&nbsp;"
gap> HTMLEncodeString("     ");
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
gap> HTMLEncodeString("\t");
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
gap> HTMLEncodeString(" &<a &<b&< c");
"&nbsp;&amp;&lt;a&nbsp;&amp;&lt;b&amp;&lt;&nbsp;c"
