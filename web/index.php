<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <title>SourceForge Boomerang project</title>
  <meta http-equiv="content-type"
 content="text/html; charset=ISO-8859-1">
</head>
<body>
<table cellpadding="2" cellspacing="2" border="0" width="100%">
  <tbody>
    <tr>
      <td valign="top" width="60%">
      <h1 align="center"><br>
      </h1>
      <h1 align="center">Boomerang</h1>
      <h2 align="center">An attempt at a general, open source,
retargetable decompiler of binary files</h2>
      <a href="http://sourceforge.net/projects/boomerang"> Sourceforge
Project home page</a><br>
      <br>
General decompilation Wiki pages:
      <ul>
        <li>Main <a
 href="http://www.program-transformation.org/twiki/bin/view/Transform/DeCompilation">Decompilation</a>
page<br>
        </li>
        <li><a
 href="http://www.program-transformation.org/twiki/bin/view/Transform/DecompilationPossible">Is
it Possible?</a> (for those in denial)</li>
        <li><a
 href="http://www.program-transformation.org/twiki/bin/view/Transform/WhyDecompilation">Why
Decompilation?</a><br>
        </li>
        <li>The <a
 href="http://www.program-transformation.org/twiki/bin/view/Transform/EthicsOfDecompilation">
Ethics</a> of decompilation<br>
        </li>
      </ul>
      </td>
      <td valign="top"><img src="boomerangs_small.jpg" alt="boomerangs"
 width="300" height="317" align="right"> <a href="#News"><br>
      </a> </td>
    </tr>
  </tbody>
</table>
<h1 align="center"></h1>
<h2 align="center"></h2>
<a href="#News">News</a><br>
<a href="#Intro">Introduction</a><br>
<a href="#making">Making Boomerang</a><br>
<a href="FAQ.html">FAQ</a><br>
<h2><a name="News"></a>News</h2>
<b>1 Oct 03: </b>We now require the Boehm garbage collector.
Over time, we will remove code for freeing objects. Some of this can
get arduous.<br>
<br>
<b>12/Aug/03: </b>Boomerang is working well enough now to correctly
decompile most of the simple test programs in the test/ directory
(including the frustrating recursive fibonacci programs). To those that
have been waiting for Boomerang to settle down and become usable again,
the long wait is over. It's still being changed rapidly, but the design
should remain fairly stable now. There is still a long way to go before
Boomerang is useful for real-world programs.<br>
<br>
<b>5/Aug/03:</b> The combination of global dataflow analysis and SSA
didn't work out. (That paper was not accepted.) We've decided that SSA
by itself has enough power to do what we need, at least in terms of
dataflow analysis. And it doesn't need global analysis, saving memory
requirements. The old dataflow code is gone, as is the "implicit" SSA,
replaced by more standard SSA code (using dominance frontiers and all
that). The global optimisation (see comments for 30/May) therefore no
longer happens.<br>
There has also been a redesign. The multiple inheritance (e.g. HLCall
from Statement and RTL) has gone. Now, an RTL is a list of Statements
(previously, a list of expressions (class Exp)). Assignments are no
longer expressions, but statements. This has cleaned up a lot of code
that iterates through statements. A lot of old commented out code has
been removed as well.<br>
We also have a theorem prover now. This is powerful enough to prove
whether a register is saved, even in the presence of recursion.<br>
It is expected that parameters and return location(s) will be working
fairly well soon. BoolStatements (e.g. created from the Pentium <span
 style="font-family: monospace;">setz</span>
instruction) work now.<br>
<br>
<b>30/May/03: </b> There has been a lot of development behind the
scenes.
Boomerang actually works slightly worse than it did in February, but
that's
just temporary while we experiment with the best design. The latest
idea
is to combine global dataflow analysis with Static Single Assignment
(SSA) form.
We've come across an interesting way to represent SSA implicitly. It's
so
neat that we're writing a paper on its implementation.
<p>In the meantime, Boomerang is not all that usable.
One interesting result is for the
SPARC twoproc program (see below for source).
</p>
<pre>void main()<br>{<br>    proc1();<br>    printf("%i\n", 7);<br>    return ;<br>}<br><br>int proc1()<br>{<br>    return %o0;<br>}<br></pre>
As you can see, return locations are broken; they are not a priority at
present.
Parameters are also gone, but not for long. Note how the global
dataflow has
actually propagated the entire semantics for proc1() into the printf
statement
in main()! Some may argue that this is not a "faithful" decompilation.
This
shows a fundamental misunderstanding of our goals and therefore needs
to be addressed. It is often stated that the goal of a decompiler is to
reconstruct
the original source code of the program that is compiled into a given
binary.
This may well be the case, but it is not the goal of this project. We
are interested in finding the simplest possible program that has
equivilent functionality to a given binary. In doing this we ignore
things that are not
explicitly represented in the high level language, like runtime and
memory usage. The above is an example of this fundamental difference of
opinion on
the goals of a decompiler. <a href="loop.html">Here</a>'s another one.
So let's hear no more about what the original program did. Who cares?
What's important is that the decompiler has best utilized the semantics
of the output language to present the simplest possible program.
<p> <b>3/February/03: </b>The very slow dataflow is now just somewhat
slow;
speeded up by at least an order of magnitude. With a couple of other
changes,
it can now translate twoproc.c (pentium or sparc):<br>
<tt>int proc1(int a, int b) {<br>
&nbsp;&nbsp;&nbsp; return a + b;<br>
}<br>
int main() {<br>
&nbsp;&nbsp;&nbsp; printf("%i\n", proc1(3, 4));<br>
}<br>
</tt><br>
to this:<br>
<tt><br>
int main() {<br>
int local0;<br>
&nbsp;&nbsp;&nbsp; local0 = proc1(3, 4) ;<br>
&nbsp;&nbsp;&nbsp; local0 = printf("%i\n", local0) ;<br>
&nbsp;&nbsp;&nbsp; return local0;<br>
}<br>
int proc1(int arg1, int arg2) {<br>
&nbsp;&nbsp;&nbsp; return arg1+arg2;<br>
}</tt><br>
There is a lot of debug output; pipe to less and look at the end.<br>
<br>
<b>4/December/02</b>: In response to university being finished for
another year, my girlfriend being overseas for a few weeks and me
coming down with a flu, I've managed to get some work done. Boomerang
can now decompile <i>hello world</i> on both Pentium and Sparc
architectures. A detailed account of this achievement is available <a
 href="helloworld.html">here</a>. The techniques used are general,
therefore, this achievement represents a subset of the possible
programs that Boomerang can now decompile. Investigations into
branching
and multi-procedure programs is next on the agenda. <br>
<b><br>
8/August/02</b>: New code has been checked in, including a GUI written
in
MFC. Obviously this is for windows only (use wine, whatever), it's for
prototyping
purposes and will eventually be replaced/complemented with
cross-platform guis. If you check out the new code on unix, please
ensure that you <i>./configure</i> before trying to make as I doubt
you are building on the same kind of Sparc box as I am.<br>
<b>8/August/02</b>: A design document outlining the (existing) internal
representation has been added. You can look at it <a
 href="internal.png">here</a> or grab the source <a
 href="internal1.dia">here</a>. It was created using the <a
 href="http://www.lysator.liu.se/%7Ealla/dia/">Dia</a> drawing tool.
Some other design documents would be nice but they dont exist at
present and some of us can only tolerate so much UML before we start
banging our heads into our keyboards uncontrollably.<br>
<b><br>
27/June/02</b>: Nothing more booked in, but there is quiet progress
behind the scenes. One of the developers has some GUI code going;
unfortunately, with our current lack of knowledge of toolkits, this is
Windows only code for now. We may have a sort of console mode
decompiler (same features, no GUI) that will compile on Unices.<br>
<b><br>
31/May/02</b>: UQBT is finally released! (See <a
 href="http://www.experimentalstuff.com/Technologies/uqbt/">this page</a>
if you are interested). See <a href="#whereAt">where the code is at</a>.<br>
<b><br>
17/May/02</b>: Frustratingly, the delay continues. We have the frontend
of UQBT modified and updated for boomerang, and it is decoding
instructions now. The next stage is removing source machine features
like delay slots.<br>
<b><br>
23/Apr/02</b>: There is a very little code booked into CVS. Code from <a
 href="http://www.itee.uq.edu.au/%7Ecristina/uqbt.html"> UQBT</a>,
which should provide a good part of at least the frontend of the
decompiler framework, was delayed but should be released shortly.<br>
<br>
</p>
<h2><a name="Intro"></a>Introduction</h2>
This project is an attempt to develop a real decompiler through the
open source community. A decompiler takes as input a binary file, and
attempts to create a high level, compilable, even maintainable source
file that does the same thing. It is therefore the opposite of a
compiler, which
of course takes a source file and makes a binary. It won't of course
recreate the original source file; probably nothing like it.<br>
<br>
The intent is to create a retargetable decompiler (i.e. one that can
work with different types of input binary files with modest effort,
e.g. X86-windows, sparc-solaris, etc). It will be highly modular, so
that different parts of the decompiler can be replaced with
experimental
modules. It will be interactive, a la <a
 href="http://www.datarescue.com/idabase/ida.htm"> IDA Pro</a>, because
some things (not just variable names and comments, though these are
obviously very important) require expert intervention.<br>
<br>
<br>
<h2><a name="whereAt"></a>31/May/2002: Where the code was at</h2>
You certainly can't decompile anything; that will be the case for
probably months. You won't be able to use boomerang to recover your
lost source code for years, if ever. There isn't even a connection
between the gui code
and the main part of the code. Hell, it won't even compile on Windows
at
present.<br>
<br>
What we do have is the UQBT instruction decoder; it can parse SPARC
or Pentium ELF binaries (Solaris or Linux; NOT Windows as yet, though
that may not be far away), and it produces a control flow graph
populated with RTLs. These RTLs are not (as yet) as high level as GCC
style RTLs. As it comes, nothing is visible (not even a disassembly)
except for some dots coming
out of cppunit (the init test system), and if all goes well, an OK
message.
If you want more, either wait, or help us write the code.<br>
<br>
Most of the code is pretty ugly. UQBT was in need of some major
refactoring for several years, but the time was never found. We have
some design documents for how we'd like the code to evolve to; email us
if you are interested.<br>
<br>
So the only way to make this code at present is on Solaris or Linux.
<h2><a name="making"></a>Making Boomerang</h2>
<p>You can make boomerang as a command line tool under Unix (Linux,
Solaris, others should work), or as a GUI tool under Windows. To make
on Unix, you need the following tools:<br>
</p>
<ul>
  <li><b>gcc version 3.1</b> (3.0 might work; 2.96 has a chance).
Because of the strstream / stringstream issue, it's not worth
attempting to compile it on older versions.</li>
  <li><b>CVS</b>, obviously.<br>
  </li>
  <li><b>cppunit</b> (from <a href="http://cppunit.sourceforge.net">cppunit.sourceforge.net</a>).
This is needed for testing. I suppose you could make boomerang without
it, but it's integral to develoopment.</li>
  <li>The Boehm garbage collector; see <a
 href="http://www.hpl.hp.com/personal/Hans_Boehm/gc/">http://www.hpl.hp.com/personal/Hans_Boehm/gc/</a>.
  </li>
  <li><font color="#999999"><b>wxWindows</b> (from <a
 href="http://wxwindows.org">wxwindows.org</a>). It's not absoluitely
needed right now, but (at present), this is the toolkit that we will
use for gui code.<font color="#000000"> (The GUI version only works on
Windows now.)</font></font></li>
  <li><font color="#ff0000">Optional</font>. These are so that you can
create generated files. If you don't have these (and only a small
number of developers will need them; they are only needed for
generating new front ends, or fixing major bugs in existing front
ends), you can just "make remote" and this will touch the generated
files. (Then just "make test" as usual).</li>
</ul>
<blockquote>
  <ul>
    <li><b>bison++</b> and <b>flex++</b> (from many places, just
search.)</li>
    <li>The ML version of the <b>New Jersey Machine Code Toolkit</b>.
From Norman Ramsey's <a
 href="http://eecs.harvard.edu/%7Enr/toolkit/ml.html">page</a>.</li>
    <ul>
      <li>If you make the toolkit and run into a memory explosion, see <a
 href="http://www.itee.uq.edu.au/%7Eemmerik/toolkitml.html#explosion">this
page</a>.<br>
      </li>
    </ul>
  </ul>
</blockquote>
<font color="#999999">To make the GUI version, you need<br>
</font>
<ul>
  <li><font color="#999999">The <b>Windows</b> OS</font></li>
  <li><font color="#999999">A recent version of <b>MSVC</b> (project
and workspace files are booked in to CVS)</font></li>
  <li><font color="#999999">Some way of accessing <b>CVS</b> files
through Windows, using SSH</font></li>
  <li><font color="#999999"><font color="#000000">For the time being,
the Windows version is not being supported. We are concentrating on the
core technology:
analyses, etc.</font><br>
    </font></li>
</ul>
<br>
<br>
<br>
<br>
<div align="center"><img src="boomerangs.jpg" alt="boomerangs"
 width="769" height="812"> <a href="http://sourceforge.net"><img
 src="sflogo.png" width="125" height="37" border="0"
 alt="SourceForge.net Logo"> </a><br>
<div align="left"><small>Last modified: 1/Oct/03: News</small><br>
</div>
</div>
<br>
<br>
<br>
<br>
<br>
<br>
</body>
</html>
