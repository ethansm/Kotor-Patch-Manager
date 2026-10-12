# A1 MDLOps findings (KotOR 1 binary -> KotOR 2 binary, headless)

Primary source: https://deadlystream.com/files/file/779-mdlops/ (read as raw HTML via curl; the readme text is inline on this page). The readme zip returned HTTP 403, so nothing was downloaded or executed.

Q1 latest release: VERIFIED. "MDLOps 1.0.0", "Released December 23, 2017". Windows mdlops.exe plus uncompiled mdlops.pl (Perl). Newest version listed on the page; a "1.0.2" mention came from a search summary and was not found on any page read. https://deadlystream.com/files/file/779-mdlops/
Q1 GitHub: UNVERIFIED. PyKotor submodule vendor/mdlops points to th3w1zard1/mdlops, which redirects to oldrepublicwizard/MDLOps. Every GitHub and API path returned 404 (repo unreachable anonymously). https://api.github.com/repos/OpenKotOR/PyKotor/contents/vendor/mdlops
Q2 K2 switch: VERIFIED. Quoted: "mdlops.exe [-a] [-s] [-k1|-k2] c:\directory\model.mdl" and "-k2 will output a binary model in kotor 2 format". https://deadlystream.com/files/file/779-mdlops/
Q2 binary/ASCII mode: VERIFIED that no CLI switch exists. Quoted: "For the command line the following switches can be used:" followed only by -a, -s, -k1, -k2. Binary vs ASCII is chosen only in the GUI: "To quickly convert the model click 'Read and write model'". https://deadlystream.com/files/file/779-mdlops/
Q2 headless K1 binary -> K2 binary in one call: UNVERIFIED. The usage line suggests mdlops.exe -k2 c:\dir\model.mdl writes K2 binary, but the README never states what a binary input produces from the CLI. GUI output for a binary input is ASCII ("model-ascii.mdl"). Fallback is two passes: binary -> ASCII, then ASCII -> binary with -k2. Test on a scratch copy before trusting.
Q3 host app (3ds Max) at runtime: REFUTED. No line says 3ds Max, Gmax, or KOTORMax is required to run. Those are listed only as separate editing tools ("KOTORMax (to get models in and out of Gmax or Max)"). The stated runtime requirement is Perl: "Running uncompiled version requires perl 5.12+ and, on Windows, will". https://deadlystream.com/files/file/779-mdlops/
Q3 extra blocker: UNVERIFIED whether Tk, JSON, Win32::FileOp are needed for CLI on Linux. Readme lists "JSON, Win32::FileOp, Tk, Tk::Tree, Tk::Frame" "(and all required for Command Line)".
Q4 KotOR 1 .mdl samples: VERIFIED none in the release. Distributed files: GPL.txt, icon.bmp, KotOR Tutorial.txt, mdlops.exe, mdlops.pl, MDLOpsM.pm, Quick_tutorial.txt, readme_mdlops_1-0.txt, replacer_tutorial.txt. Repo samples: UNVERIFIED (repo unreachable).

Traps:
- Supermodel: "the super model or the original model must be in the same directory as model being imported" (readme note 5). https://deadlystream.com/files/file/779-mdlops/
- MDX: "For binary models you must have the .MDL and .MDX in the same directory" (readme note 4).
- Version conflict: a 2012 hilt guide says "MAKE SURE YOU USE MDLOPS 0.5 NOT 0.6!!!", and a later thread says "Do not use v0.5" and "Use MDLEdit or MDLOps v1.0". Both were read via summary, not raw. https://deadlystream.com/topic/1643-how-to-convert-a-hilt-model-between-kotor-and-tsl/ and https://deadlystream.com/topic/6822-what-am-i-doing-wrong-with-mdlops/
- Perl route: MDLOpsM.pm must be copied into a Perl lib dir ("you must first copy the MDLOpsM.pm file into your \perl\lib or \perl\site\lib directory"). Linux equivalent is inferred, not documented.
