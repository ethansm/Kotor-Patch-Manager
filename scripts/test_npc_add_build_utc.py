#!/usr/bin/env python3
"""Tests for scripts/npc_add_build_utc.py. Run: python3 scripts/run_tests.py npc_add_build_utc
Needs pykotor (re-runs under the repo .venv when the system python lacks it). The clone tests use an in-memory
vanilla-like n_telf01 template, so they run without the game. Only the real-archive test reads the game dir
(read only) and skips when 202tel.mod is absent. Every CLI test writes to a temp dir, never to the repo mirror."""
import contextlib
import hashlib
import io
import os
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

try:
    import pykotor  # noqa: F401
except ImportError:  # scripts/run_tests.py runs test_*.py with the system python: re-run under the repo venv
    _venv = Path(__file__).resolve().parent.parent / ".venv" / "bin" / "python3"
    if _venv.exists() and Path(sys.prefix).resolve() != _venv.parent.parent.resolve():
        os.execv(str(_venv), [str(_venv), __file__] + sys.argv[1:])
    print("SKIP: pykotor not importable and no repo .venv")
    sys.exit(0)

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_add_build_utc as B  # noqa: E402
from pykotor.common.language import Gender, Language, LocalizedString  # noqa: E402
from pykotor.common.misc import ResRef  # noqa: E402
from pykotor.resource.formats.erf import ERF, ERFType, bytes_erf, read_erf  # noqa: E402
from pykotor.resource.generics.utc import UTC, InventoryItem, UTCClass, bytes_utc, read_utc  # noqa: E402
from pykotor.resource.generics.utc import EquipmentSlot  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

VANILLA_MOD = B._common.GAME_DIR / "Modules" / "202tel.mod"
ROW_CANARY = B.Row(slug="twilekf", appearance=47, race=6, gender=1, behaviour="stand", canary=True)
CANARY_SLUGS = {"twilekf", "wookm", "rodian", "bith"}


def vanilla_like_template() -> bytes:
    """In-memory n_telf01 (202tel) with the vanilla commoner fields from research P4 table 1."""
    u = UTC()
    u.resref = ResRef("n_telf01")
    u.tag = "TelF01"
    u.conversation = ResRef("200comf")
    u.first_name = LocalizedString(127356)
    u.last_name = LocalizedString.from_invalid()
    u.race_id, u.subrace_id, u.gender_id, u.appearance_id, u.soundset_id = 6, 0, 1, 610, 35
    u.faction_id = 5
    u.interruptable = True
    u.disarmable = False
    u.perception_id, u.walkrate_id = 11, 7
    u.blindspot, u.challenge_rating, u.alignment = 120.0, 1.0, 60
    u.hp = u.current_hp = u.max_hp = 8
    u.body_variation, u.texture_variation, u.multiplier_set = 1, 1, 3
    u.awareness = 2
    u.classes = [UTCClass(0, 1)]
    u.feats = [93]
    u.equipment = {EquipmentSlot.ARMOR: InventoryItem(ResRef("g_a_clothes010"))}
    u.on_spawn = ResRef("k_202_commoner")
    u.on_heartbeat = ResRef("k_def_heartbt01")
    u.on_notice = ResRef("k_def_percept01")
    u.on_spell = ResRef("k_def_spellat01")
    u.on_attacked = ResRef("k_def_attacked01")
    u.on_damaged = ResRef("k_def_damage01")
    u.on_disturbed = ResRef("k_def_disturb01")
    u.on_end_round = ResRef("k_def_combend01")
    u.on_dialog = ResRef("k_def_dialogue01")
    u.on_end_dialog = ResRef("k_def_endconv")
    u.on_death = ResRef("k_def_death01")
    u.on_user_defined = ResRef("k_def_userdef01")
    u.on_blocked = ResRef("k_def_blocked01")
    return bytes_utc(u)


def mutated_template(**fields) -> bytes:
    u = read_utc(vanilla_like_template())
    for name, value in fields.items():
        setattr(u, name, value)
    return bytes_utc(u)


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def write_table(path: Path, rows: list[dict]) -> None:
    lines = []
    for r in rows:
        lines.append("[[species]]")
        for k, v in r.items():
            lines.append(f'{k} = "{v}"' if isinstance(v, str) else f"{k} = {str(v).lower() if isinstance(v, bool) else v}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def make_mod(path: Path, template: bytes, extra: dict[str, bytes] | None = None) -> None:
    erf = ERF(ERFType.MOD)
    erf.set_data("n_telf01", ResourceType.UTC, template)
    for resref, data in (extra or {}).items():
        erf.set_data(resref, ResourceType.UTC, data)
    path.write_bytes(bytes_erf(erf, ResourceType.MOD))


class TableTests(unittest.TestCase):
    def test_shipped_table_has_18_unique_rows_with_canary_set(self):
        rows = B.load_rows()
        self.assertEqual(len(rows), 18)
        self.assertEqual(len({r.slug for r in rows}), 18)
        self.assertEqual({r.slug for r in rows if r.canary}, CANARY_SLUGS)

    def test_every_resref_within_16_chars(self):
        for r in B.load_rows():
            self.assertLessEqual(len(r.resref), B.MAX_RESREF, r.resref)
            self.assertTrue(r.resref.startswith("npcadd_"))

    def test_longest_resref_is_sullustan_at_16(self):
        self.assertIn("npcadd_sullustan", [r.resref for r in B.load_rows()])
        self.assertEqual(len("npcadd_sullustan"), B.MAX_RESREF)

    def test_rejects_slug_that_overflows_resref(self):
        with self.assertRaises(B.BuildError):
            B.validate_row(B.Row(slug="sullustanx", appearance=601, race=6, gender=0, behaviour="stand"))

    def test_rejects_bad_behaviour_gender_and_slug(self):
        for bad in (B.Row("wookm", 271, 6, 0, "patrol"), B.Row("wookm", 271, 6, 2, "stand"),
                    B.Row("Wook M", 271, 6, 0, "stand")):
            with self.assertRaises(B.BuildError, msg=repr(bad)):
                B.validate_row(bad)

    def test_rejects_duplicate_slug_in_table(self):
        with tempfile.TemporaryDirectory() as td:
            t = Path(td) / "species.toml"
            write_table(t, [{"slug": "bith", "appearance": 12, "race": 6, "gender": 0, "behaviour": "idle"},
                            {"slug": "bith", "appearance": 12, "race": 6, "gender": 0, "behaviour": "stand"}])
            with self.assertRaises(B.BuildError):
                B.load_rows(t)


class CloneTests(unittest.TestCase):
    def setUp(self):
        self.template = vanilla_like_template()

    def test_clone_keeps_vanilla_commoner_fields(self):
        u = read_utc(B.build_utc(self.template, ROW_CANARY))
        self.assertEqual(u.faction_id, 5)
        self.assertFalse(u.plot)
        self.assertFalse(u.min1_hp)
        self.assertFalse(u.no_perm_death)
        self.assertFalse(u.is_pc)
        self.assertTrue(u.interruptable)
        self.assertFalse(u.disarmable)
        self.assertEqual(u.hp, 8)
        self.assertEqual(u.walkrate_id, 7)
        self.assertEqual(u.perception_id, 11)
        self.assertEqual([(c.class_id, c.class_level) for c in u.classes], [(0, 1)])
        self.assertEqual(u.feats, [93])
        self.assertEqual(u.awareness, 2)
        self.assertEqual([i.resref.get() for i in u.equipment.values()], ["g_a_clothes010"])
        self.assertEqual(u.on_dialog.get(), "k_def_dialogue01")
        self.assertEqual(u.on_end_dialog.get(), "k_def_endconv")

    def test_overrides_identity_appearance_sound_and_conversation(self):
        u = read_utc(B.build_utc(self.template, ROW_CANARY))
        self.assertEqual(u.resref.get(), "npcadd_twilekf")
        self.assertEqual(u.tag, "npcadd_twilekf")
        self.assertEqual(u.first_name.get(Language.ENGLISH, Gender.MALE), "Telos Resident")
        self.assertEqual(u.first_name.get(Language.ENGLISH, Gender.FEMALE), "Telos Resident")
        self.assertEqual(u.first_name.stringref, -1)
        self.assertEqual(u.last_name.stringref, -1)
        self.assertEqual(u.last_name.get(Language.ENGLISH, Gender.MALE), None)
        self.assertEqual((u.race_id, u.gender_id, u.appearance_id), (6, 1, 47))
        self.assertEqual(u.soundset_id, 35)
        self.assertEqual(u.conversation.get(), "")

    def test_behaviour_scripts(self):
        expect = {"stand": ("", ""), "walk": ("k_def_spawn01", ""), "idle": ("k_def_ambient", "k_def_heartbt01")}
        for behaviour, (spawn, heartbeat) in expect.items():
            row = B.Row("bith", 12, 6, 0, behaviour)
            u = read_utc(B.build_utc(self.template, row))
            self.assertEqual((u.on_spawn.get(), u.on_heartbeat.get()), (spawn, heartbeat), behaviour)

    def test_build_is_byte_stable(self):
        rows = [ROW_CANARY, B.Row("bith", 12, 6, 0, "idle")]
        first = B.build_all(self.template, rows)
        second = B.build_all(self.template, rows)
        self.assertEqual({k: sha(v) for k, v in first.items()}, {k: sha(v) for k, v in second.items()})
        self.assertEqual(list(first), ["npcadd_twilekf.utc", "npcadd_bith.utc"])

    def test_refuses_forbidden_clone_values(self):
        cases = (({"plot": True}, "Plot=1"), ({"min1_hp": True}, "Min1HP=1"),
                 ({"no_perm_death": True}, "NoPermDeath=1"), ({"faction_id": 1}, "FactionID 1"),
                 ({"is_pc": True}, "IsPC=1"), ({"interruptable": False}, "Interruptable=0"))
        for fields, expected in cases:
            with self.subTest(expected), self.assertRaises(B.BuildError) as ctx:
                B.build_utc(mutated_template(**fields), ROW_CANARY)
            self.assertIn(expected, str(ctx.exception))

    def test_refuses_non_chitin_script_in_a_kept_slot(self):
        # on_spawn and on_heartbeat are overwritten per behaviour; a non-k_def script in any other slot must fail
        t = mutated_template(on_notice=ResRef("k_202_commoner"))
        with self.assertRaises(B.BuildError) as ctx:
            B.build_utc(t, ROW_CANARY)
        self.assertIn("on_notice='k_202_commoner'", str(ctx.exception))

    def test_collision_with_taken_name_is_refused(self):
        with self.assertRaises(B.BuildError):
            B.build_all(self.template, [ROW_CANARY], taken={"npcadd_twilekf"})

    def test_mirror_in_repo_passes_guardrails_without_game(self):
        for row in B.load_rows():
            path = B.OUT_DIR / f"{row.resref}.utc"
            self.assertTrue(path.is_file(), path)
            self.assertEqual(B.utc_problems(read_utc(path.read_bytes()), row), [], row.resref)

    @unittest.skipUnless(VANILLA_MOD.is_file(), f"vanilla archive absent: {VANILLA_MOD}")
    def test_real_archive_clone_keeps_vanilla_fields(self):
        real = B.read_template(VANILLA_MOD)
        vanilla = read_utc(real)
        clone = read_utc(B.build_utc(real, ROW_CANARY))
        for field in ("faction_id", "plot", "min1_hp", "no_perm_death", "is_pc", "interruptable", "disarmable",
                      "hp", "walkrate_id", "perception_id", "feats", "awareness", "blindspot", "multiplier_set"):
            self.assertEqual(getattr(clone, field), getattr(vanilla, field), field)
        self.assertEqual(clone.on_heartbeat.get(), "")  # stand row blanks the heartbeat
        self.assertEqual(clone.on_dialog.get(), vanilla.on_dialog.get())


class NameScanTests(unittest.TestCase):
    def test_taken_names_cover_override_and_module_archives(self):
        with tempfile.TemporaryDirectory() as td:
            game = Path(td) / "game"
            (game / "Override").mkdir(parents=True)
            (game / "Override" / "npcadd_ovr.utc").write_bytes(b"x")
            (game / "Modules").mkdir()
            make_mod(game / "Modules" / "203tel.mod", vanilla_like_template(), {"npcadd_mod": b"y"})
            taken = B.taken_utc_names(game)
        self.assertIn("npcadd_ovr", taken)
        self.assertIn("npcadd_mod", taken)
        self.assertIn("n_telf01", taken)


class CliTests(unittest.TestCase):
    def setUp(self):
        self.td = Path(tempfile.mkdtemp(prefix="npcadd_cli_"))
        self.addCleanup(shutil.rmtree, self.td, ignore_errors=True)
        self.mod = self.td / "202tel.mod"
        make_mod(self.mod, vanilla_like_template())
        self.table = self.td / "species.toml"
        write_table(self.table, [
            {"slug": "bith", "appearance": 12, "race": 6, "gender": 0, "behaviour": "idle"},
            {"slug": "rodian", "appearance": 39, "race": 6, "gender": 0, "behaviour": "walk"},
        ])
        self.out = self.td / "override"
        self.nogame = self.td / "no_such_game"

    def run_cli(self, *extra) -> tuple[int, str, str]:
        out, err = io.StringIO(), io.StringIO()
        argv = ["--vanilla-mod", str(self.mod), "--table", str(self.table), "--out-dir", str(self.out),
                "--game-dir", str(self.nogame), *extra]
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = B.main(argv)
        return code, out.getvalue(), err.getvalue()

    def test_write_is_byte_stable_and_check_detects_drift(self):
        code, _, err = self.run_cli()
        self.assertEqual(code, 0, err)
        first = {p.name: sha(p.read_bytes()) for p in self.out.glob("*.utc")}
        self.assertEqual(sorted(first), ["npcadd_bith.utc", "npcadd_rodian.utc"])
        self.assertEqual(self.run_cli()[0], 0)
        self.assertEqual({p.name: sha(p.read_bytes()) for p in self.out.glob("*.utc")}, first)

        self.assertEqual(self.run_cli("--check")[0], 0)

        (self.out / "npcadd_bith.utc").write_bytes(b"tampered")
        code, out, _ = self.run_cli("--check")
        self.assertEqual(code, 1)
        self.assertIn("changed npcadd_bith.utc", out)

        self.run_cli()
        (self.out / "npcadd_rodian.utc").unlink()
        code, out, _ = self.run_cli("--check")
        self.assertEqual(code, 1)
        self.assertIn("missing npcadd_rodian.utc", out)

        self.run_cli()
        (self.out / "npcadd_zzz_stale.utc").write_bytes(b"old")
        code, out, _ = self.run_cli("--check")
        self.assertEqual(code, 1)
        self.assertIn("stale npcadd_zzz_stale.utc", out)

    def test_check_writes_nothing(self):
        self.run_cli()
        (self.out / "npcadd_bith.utc").write_bytes(b"tampered")
        self.run_cli("--check")
        self.assertEqual((self.out / "npcadd_bith.utc").read_bytes(), b"tampered")

    def test_missing_template_archive_exits_2(self):
        self.mod.unlink()
        code, _, err = self.run_cli()
        self.assertEqual(code, 2)
        self.assertIn("vanilla template archive not found", err)

    def test_template_without_n_telf01_exits_2(self):
        erf = ERF(ERFType.MOD)
        erf.set_data("other", ResourceType.UTC, vanilla_like_template())
        self.mod.write_bytes(bytes_erf(erf, ResourceType.MOD))
        code, _, err = self.run_cli()
        self.assertEqual(code, 2)
        self.assertIn("n_telf01", err)

    def test_collision_with_game_module_exits_2(self):
        game = self.td / "game"
        (game / "Override").mkdir(parents=True)
        (game / "Override" / "npcadd_bith.utc").write_bytes(b"live")
        out, err = io.StringIO(), io.StringIO()
        argv = ["--vanilla-mod", str(self.mod), "--table", str(self.table), "--out-dir", str(self.out),
                "--game-dir", str(game)]
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = B.main(argv)
        self.assertEqual(code, 2)
        self.assertIn("npcadd_bith", err.getvalue())


if __name__ == "__main__":
    unittest.main(verbosity=1)
