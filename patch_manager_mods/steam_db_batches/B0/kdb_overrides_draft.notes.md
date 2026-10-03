# kdb_overrides draft: decisions that need NO row (keep the KDB name)

persist skips these upserts with `CONFLICT: trusted holder ... kept`; the KDB name is an equivalent naming of the same
function, so no override row is proposed (the GOG name is still shipped through functions.csv / the branch DB):

| Steam | KDB name (trusted) | B0 final name | decision |
|---|---|---|---|
| 0x733570 | CExoString::CExoString_char | CExoString::Constructor_2 | keep: both name the char* constructor (126 B, strlen + new) |
| 0x733780 | CExoString::~CExoString | CExoString::Destructor | keep: same destructor (60 B, free) |
| 0x7110d0 | CRes::CRes | CRes::Constructor | keep: same base constructor (151 B) |

To use the proposed rows: review, then copy `kdb_overrides_draft.tsv` to `kdb_overrides.tsv` (the Opus step); `persist` applies it before
the upserts. Rename rows keep the old name/source/confidence in the evidence text.
