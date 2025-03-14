   This is a port of BOBCAT (Lynx for DOS) to ELKS (and Linux). To compile,
   run "make -f Makefile.elks" from inside "src" directory. For Linux, use
   the "Makefile" to compile.

   Binary releases are available in "Releases" page.

   Follows the original README.


                             LYNX FOR DOS (BOBCAT)
                                       
                                WHAT IS BOBCAT?
                                       
   Bobcat aspires to be a text WWW browser.
   
   In the big picture, Bobcat is a project that I dreamed up to amuse
   myself. I like to dabble with code as a hobby. I'm not very good, so
   things can get pretty messy.
   
   Before Bobcat I played with altering DOSLYNX, but there came a point
   in which it started frustrating me more than being fun. At that time I
   knew it was time to try something new. I have always liked Lynx. It is
   a great web browser! I really wanted to see it work in plain DOS, even
   though I knew there would be some painful limitations. I also knew
   there were other people looking into making 386 and Windows versions
   so I figured I would give it a try.
   
   Bobcat, by definition, means a small Lynx. The name was picked to
   imply that it is a less mature sibling to the Lynx browser. Bobcat
   started life as Lynx2-4-2. The reason 2-4-2 was chosen is because
   starting at 2-5, many files in the Lynx distribution became too big to
   compile out of the package targeted towards a plain DOS version. There
   have been many changes since 2-4-2, however, it does support many
   things that DOSLYNX didn't. I have also made many modifications to
   Bobcat's original behavior to make it more functional.
   
   Taking the 2-4-2 base, I then added PDCurses2.2, WATTCP, SPAWNO and
   bits of DOSLYNX and Lynx2-6 code. I then mixed vigorously in Borland
   C++ 3.1 until it actually compiled. The first step, getting it to
   compile into an EXE, was deceptively simple. However, as many hackers
   know, you can get a LOT of code to compile and still not have it do
   anything.
   
   After that I made personal decisions on what to support. Although it
   may disappoint some people, news support is not in Bobcat. It never
   worked correctly in DOSLYNX and I didn't feel it was a priority.
   Gopher and Ftp should hopefully work to some degree, but they are not
   the top priority either. The important thing, as far as I am
   concerned, is good web support. This is the reason for cutting corners
   on the other features. There is a new feature added to Bobcat that
   will allow people to define external programs for certain URLs. This
   product uses the SPAWNO routines by Ralf Brown to minimize memory use
   while shelling to DOS and running other programs. Hopefully this will
   help compensate for Bobcat's shortcomings.
   
   This new Lynx for DOS is a big switch from DOSLYNX. It is Curses
   based, so it looks like the real Lynx. It does not have pull down
   menus or a multiple document interface. I prefer the real Lynx
   interface which is partially why I made the switch in development.
   Other good reasons for starting with a new code base are support for
   forms, numeric links, view source, history list and other features
   which were not available in DOSLYNX.
   
   The first version of Bobcat has severe memory constraints because of
   the way the original Lynx was designed. In Un*x, programs assume they
   have unlimited memory and the programs are coded as such. Bobcat has
   yet to have memory checking added to it, so when you run out of the
   tiny amount of conventional memory available it will very
   unceremoniously drop you to the DOS prompt.
   
   Bobcat is NOT a Lynx port. It is a work based on Lynx. This means that
   it can not keep up with the current Lynx development nor can Lynx
   patches be added to Bobcat. Hopefully, some of the forthcoming 386 and
   Windows ports will have this ability.
   
   I am pretty excited about having a new variant of Lynx to play with. I
   hope other people will be receptive. If you have comments or
   suggestions you can send them to bobcat@fdisk.com. Please remember
   that this is a hobby and I do it solely for fun. Flames will be
   publicly ridiculed. No payment has been given for the work done so far
   and none is expected for continued use of the program. Obscene,
   no-strings-attached, gratuitous donations of cash will, however, be
   gleefully accepted *grin* but if you really have nothing better to do
   with your money then donate some to a thoughtfully chosen charity.
   
   Big thanks to John Lewis for the pre release definition of
   requirements, hours of debugging, the help files, the program
   extract.exe and all the batch files which integrate the package. He
   truly helped make Bobcat a package rather than just a program!
   
   Licence:

   The pieces of code that Bobcat is based on all have their own licence
   agreements and my arrangement of them should in no way revoke any of
   their individual licences. On top of that, the Bobcat program itself
   and my contributing code is copyright 1997 Wayne Buttles to be
   distributed freely and without charge in source and executable form.
   
   The program can be modified and repackaged, but it must continue to be
   free except for nominal copying charges. Part of this "restriction" is
   placed on Bobcat by the original Lynx licence on which much of the
   code is based.
   
   Bobcat comes with no warranty of fitness for any purpose. By using
   this software you are agreeing to take responsibility for any adverse
   consequences.
   
   Portions copyrighted by:
   (c)1996,1997 Wayne Buttles (The Bobcat entity)
   (c)GNU General Public License (Lynx base code)
   (c)1990,1991,1992 Ralf Brown (SPAWNO v4.10)
   (c)1990,1991,1992,1993 Erick Engelke and others (WATTCP)
   Other copyrights held for the PDcurses, WATTCP and WWWLIB
   libraries can be found in the source distribution.
   
