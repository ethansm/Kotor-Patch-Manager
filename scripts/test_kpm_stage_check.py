"""Tests for kpm_stage_check.py (run: python3 test_kpm_stage_check.py, cwd=scripts)."""
import hashlib, json, os, shutil, subprocess, sys, tempfile, unittest, zipfile

SCRIPT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'kpm_stage_check.py')


def cfg(entries):
    """entries: [(id, [hook function names])] -> patch_config.toml text."""
    out = ['target_version_sha = "AB"']
    for pid, funcs in entries:
        out += ['[[patches]]', f'id = "{pid}"', f'dll = "patches/{pid}.dll"']
        for n, f in enumerate(funcs):
            out += ['[[patches.hooks]]', f'address = {1000 + n}', 'original_bytes = [1, 2]',
                    'type = "detour"', f'function = "{f}"']
    return '\n'.join(out) + '\n'


def put(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb' if isinstance(data, bytes) else 'w') as f:
        f.write(data)


def kpatch(stage, pid, dll):
    with zipfile.ZipFile(os.path.join(stage, pid + '.kpatch'), 'w') as z:
        z.writestr('manifest.toml', f'id = "{pid}"\n')
        z.writestr('binaries/windows_x86.dll', dll)


def run(*args):
    return subprocess.run([sys.executable, SCRIPT, *args], capture_output=True, text=True)


class Base(unittest.TestCase):
    IDS = ['alpha', 'beta']

    def setUp(self):
        self.root = tempfile.mkdtemp(prefix='kpmchk_')
        self.addCleanup(shutil.rmtree, self.root, True)
        self.game, self.bak, self.stage = (os.path.join(self.root, d) for d in ('game', 'bak', 'stage'))
        os.makedirs(self.stage)
        self.populate(self.game)
        self.populate(self.bak)
        for i in self.IDS:
            kpatch(self.stage, i, i.encode() + b'-dll')

    def populate(self, d, ids=None):
        ids = self.IDS if ids is None else ids
        for i in ids:
            put(os.path.join(d, 'patches', i + '.dll'), i.encode() + b'-dll')
        put(os.path.join(d, 'patches', 'alpha.dll.bak_1'), b'old')
        put(os.path.join(d, 'patch_config.toml'), cfg([(i, ['F_' + i]) for i in ids]))
        put(os.path.join(d, 'kpm_install_state.json'), json.dumps({'InstalledPatches': ids}))


class DefaultMode(Base):
    def test_output_compat(self):
        r = run(self.stage, self.game)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn('installed ids: 2', r.stdout)
        self.assertIn('DIFFS 0', r.stdout)
        self.assertNotIn('WARN', r.stdout)

    def test_orphan_dll_warning(self):
        put(os.path.join(self.game, 'patches', 'gamma.dll'), b'x')
        r = run(self.stage, self.game)
        self.assertEqual(r.returncode, 0)
        self.assertIn('WARN orphan dll gamma.dll: not in InstalledPatches, the installer will delete it', r.stdout)
        self.assertLess(r.stdout.index('DIFFS 0'), r.stdout.index('WARN orphan'))

    def test_config_id_without_kpatch_warning(self):
        os.remove(os.path.join(self.stage, 'beta.kpatch'))
        r = run(self.stage, self.game)
        self.assertEqual(r.returncode, 0)
        self.assertIn('WARN config id beta has no kpatch in the stage: the installer will drop it', r.stdout)


class PostMode(Base):
    def post(self, *extra):
        return run('--post', self.bak, self.game, *extra)

    def test_identical(self):
        r = self.post()
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn('POST DIFFS 0 (allowed 0)', r.stdout)

    def test_deleted_non_installed_dll(self):
        os.remove(os.path.join(self.game, 'patches', 'alpha.dll.bak_1'))
        r = self.post()
        self.assertEqual(r.returncode, 1)
        self.assertIn('MISSING patches/alpha.dll.bak_1', r.stdout)
        self.assertIn('POST DIFFS 1 (allowed 0)', r.stdout)

    def test_new_file(self):
        put(os.path.join(self.game, 'patches', 'zeta.dll'), b'z')
        r = self.post()
        self.assertEqual(r.returncode, 1)
        self.assertIn('NEW patches/zeta.dll', r.stdout)

    def test_changed_md5_allowed(self):
        put(os.path.join(self.game, 'patches', 'alpha.dll'), b'changed')
        r = self.post('--allow', 'alpha')
        self.assertEqual(r.returncode, 0, r.stdout)
        self.assertIn('ALLOWED DIFF patches/alpha.dll md5', r.stdout)
        self.assertIn('POST DIFFS 0 (allowed 1)', r.stdout)

    def test_changed_md5_not_allowed(self):
        put(os.path.join(self.game, 'patches', 'beta.dll'), b'changed')
        r = self.post('--allow', 'alpha')
        self.assertEqual(r.returncode, 1)
        a, b = hashlib.md5(b'beta-dll').hexdigest(), hashlib.md5(b'changed').hexdigest()
        self.assertIn(f'DIFF patches/beta.dll md5 {a}->{b}', r.stdout)

    def test_config_entry_replaced(self):
        put(os.path.join(self.game, 'patch_config.toml'), cfg([('alpha', ['F_alpha']), ('gamma', ['F_gamma'])]))
        r = self.post()
        self.assertEqual(r.returncode, 1)
        self.assertIn('CONFIG beta removed', r.stdout)
        self.assertIn('CONFIG gamma added', r.stdout)

    def test_config_hook_changed(self):
        put(os.path.join(self.game, 'patch_config.toml'), cfg([('alpha', ['F_other']), ('beta', ['F_beta'])]))
        r = self.post()
        self.assertEqual(r.returncode, 1)
        self.assertIn('CONFIG alpha changed', r.stdout)

    def test_config_changed_allowed(self):
        put(os.path.join(self.game, 'patch_config.toml'), cfg([('alpha', ['F_other']), ('beta', ['F_beta'])]))
        r = self.post('--allow', 'alpha')
        self.assertEqual(r.returncode, 0)
        self.assertIn('ALLOWED CONFIG alpha changed', r.stdout)

    def test_config_reordered_only(self):
        put(os.path.join(self.game, 'patch_config.toml'), cfg([('beta', ['F_beta']), ('alpha', ['F_alpha'])]))
        r = self.post()
        self.assertEqual(r.returncode, 0, r.stdout)
        self.assertIn('POST DIFFS 0 (allowed 0)', r.stdout)

    def test_backup_without_config(self):
        os.remove(os.path.join(self.bak, 'patch_config.toml'))
        r = self.post()
        self.assertEqual(r.returncode, 1)
        self.assertIn('WARN backup has no patch_config.toml', r.stdout)


if __name__ == '__main__':
    unittest.main()
