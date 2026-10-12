# T8 headless tooling recon (K1 import). Fetched 2026-10-11.

Q1 pykotor: VERIFIED | https://pypi.org/pypi/PyKotor/json ; https://github.com/OpenKotOR/PyKotor
  - PyPI name "pykotor", latest 2.3.12, uploaded 2026-05-02, license LGPL-3.0-or-later, requires >=3.8.
  - PyPI HTML page failed to load; the JSON API was used. One search snippet said MIT: stale, ignore.
  - CLI exists: entry points "pykotor" and "pykotorcli" (GitHub README).
  - "2da2csv" is real: argparser.py L161 lists it; dispatch.py L163 calls cmd_2da2csv (format_convert.py L255).
  - README example: uvx --refresh pykotor 2da2csv "path/to/file.2da".
  - MDL read/write for K1 and K2: see Q4 caveat.

Q2 xoreos-tools: VERIFIED | https://github.com/xoreos/xoreos-tools/releases/tag/v0.0.6 ; https://www.xoreos.org/blog/2020/08/03/xoreos-0-dot-0-6-elanee-released/
  - Latest release: v0.0.6 "Elanee", 2020-08-03 (GitHub API releases/latest).
  - Tag v0.0.6 tree has all four: src/unerf.cpp, src/unrim.cpp, src/gff2xml.cpp, src/xml2gff.cpp (plus rim.cpp).
  - Blog: xml2gff is new in 0.0.6 ("counterpart to gff2xml").
  - No release after 2020. Untagged HEAD commit is dated 2026-08-28; not checked for renames.

Q3 headless K1-to-K2 MDL (not MDLEdit): VERIFIED | https://deadlystream.com/files/file/779-mdlops/
  - MDLOps 1.0.0, released 2017-12-23. Syntax: "mdlops.exe [-a] [-s] [-k1|-k2] model.mdl" or "perl mdlops.pl ...".
  - Quoted: "-k2 will output a binary model in kotor 2 format".
  - NOT tested: whether the -k2 output loads in K2. Forum posts say a K2 supermodel is needed (unverified).
  - pykotor "model-convert" is MDL<->ASCII only, no game flag (argparser.py L956-960). Not a K1->K2 path.

Q4 pykotor MDL ASCII API: VERIFIED | https://raw.githubusercontent.com/OpenKotOR/PyKotor/HEAD/Libraries/PyKotor/src/pykotor/resource/formats/mdl/__init__.py
  - Module path: pykotor.resource.formats.mdl.
  - Exports (mdl/__init__.py L81-87): MDLAsciiReader, MDLAsciiWriter, MDLBinaryReader, MDLBinaryWriter, read_mdl, write_mdl, read_mdl_fast, bytes_mdl.
  - ASCII classes: io_mdl_ascii.py MDLAsciiWriter L313, MDLAsciiReader L993.
  - read_mdl / write_mdl take file_format (MDL or MDL_ASCII): mdl_auto.py L262, L443.
  - K1/K2 binary layouts: io_mdl.py L227-235. Reader sets game from file (L2369, L2374).
  - Caveat: MDLBinaryWriter has NO game parameter. self.game defaults to K1 and flips to K2 only if K2-only dirt/hologram fields are set (io_mdl.py L3212-3261).
  - MDX: io_mdl.py imports bioware_kaitai_formats.mdx (L44) and handles MDX offset/size fields (L181-216). Not run on real K1/K2 files.

Gemini claim (c) .utc/.2da/compile: PARTIAL | https://github.com/OpenKotOR/PyKotor/tree/HEAD/Libraries/PyKotor/src/pykotor
  - Modules exist: resource/generics/utc.py, resource/formats/twoda/io_twoda.py, resource/formats/ncs/compilers.py.
  - CLI: "compile" (project targets, argparser.py L429) and "nwnnsscomp" (L890).
  - Edit and compile behavior not executed.

Unresolved:
  - Does pykotor writer output or MDLOps -k2 output load in K2 in-game without a supermodel or other fixes? Untested.
  - No round-trip run on a real K1 model.
