#!/usr/bin/env python3

import json
import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_DIR = Path(__file__).resolve().parent / "scripts"


def run_checked(cmd, *, cwd=REPO_ROOT, env=None, input_text=None):
	result = subprocess.run(
		cmd,
		cwd=cwd,
		env=env,
		input=input_text,
		text=True,
		capture_output=True,
	)
	if result.returncode != 0:
		raise RuntimeError(
			f"command failed: {' '.join(cmd)}\n"
			f"stdout:\n{result.stdout}\n"
			f"stderr:\n{result.stderr}"
		)
	return result


class PythonPluginIntegrationTests(unittest.TestCase):
	@classmethod
	def setUpClass(cls):
		cls.tmpdir = Path(tempfile.mkdtemp(prefix="r2lang-python-tests-"))
		cls.home = cls.tmpdir / "home"
		cls.xdg_data_home = cls.tmpdir / "share"
		cls.plugins_dir = cls.xdg_data_home / "radare2" / "plugins"
		cls.home.mkdir(parents=True, exist_ok=True)
		cls.plugins_dir.mkdir(parents=True, exist_ok=True)
		cls.env = os.environ.copy()
		cls.env.update(
			{
				"HOME": str(cls.home),
				"XDG_DATA_HOME": str(cls.xdg_data_home),
				"R2_COLOR": "0",
			}
		)
		run_checked(["make", "-C", "python", "py"], env=cls.env)
		run_checked(
			[
				"make",
				"-C",
				"python",
				"python-install",
				f"R2PM_PLUGDIR={cls.plugins_dir}",
			],
			env=cls.env,
		)

	@classmethod
	def tearDownClass(cls):
		shutil.rmtree(cls.tmpdir, ignore_errors=True)

	def run_r2(self, *args):
		result = subprocess.run(
			["r2", *args],
			cwd=REPO_ROOT,
			env=self.env,
			text=True,
			capture_output=True,
		)
		self.assertEqual(
			result.returncode,
			0,
			msg=(
				f"r2 failed: {' '.join(args)}\n"
				f"stdout:\n{result.stdout}\n"
				f"stderr:\n{result.stderr}"
			),
		)
		return result

	def test_python_lang_plugin_is_visible(self):
		result = self.run_r2("-N", "-q", "-c", "Llj", "-")
		plugins = json.loads(result.stdout)
		self.assertIn("python", {plugin["name"] for plugin in plugins})

	def test_basic_script_runs_and_can_issue_commands(self):
		result = self.run_r2("-N", "-q", "-i", str(SCRIPT_DIR / "basic.py"), "-")
		self.assertEqual(result.stdout.strip(), "5")

	def test_io_plugin_can_open_and_read(self):
		result = self.run_r2(
			"-N",
			"-q",
			"-I",
			str(SCRIPT_DIR / "io_plugin.py"),
			"-c",
			"p8 4",
			"pytestio://33",
		)
		self.assertEqual(result.stdout.strip(), "41414141")

	def test_arch_plugin_can_decode_and_encode(self):
		result = self.run_r2(
			"-N",
			"-q",
			"-i",
			str(SCRIPT_DIR / "arch_plugin.py"),
			"-c",
			"wx 031234; e asm.arch=pyarch; e asm.bits=32; pdj 1; pa nop; pa mov r2, 0x33",
			"malloc://3",
		)
		lines = [line.strip() for line in result.stdout.splitlines() if line.strip()]
		self.assertGreaterEqual(len(lines), 3, msg=result.stdout)
		op = json.loads(lines[0])[0]
		self.assertEqual(op["opcode"], "jne 0x1234")
		self.assertEqual(op["jump"], 0x1234)
		self.assertEqual(op["fail"], 3)
		self.assertEqual(lines[1], "00")
		self.assertEqual(lines[2], "010233")


if __name__ == "__main__":
	unittest.main()
