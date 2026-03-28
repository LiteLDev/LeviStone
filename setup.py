import os
import sys
import platform
import subprocess
from setuptools import setup
from setuptools.command.bdist_wheel import bdist_wheel

bin_path = "bin/EndstoneRuntime"

class TagProvider(bdist_wheel):
    def get_tag(self):
        python_version = f"cp{sys.version_info[0]}{sys.version_info[1]}"

        if platform.system() != "Windows":
            raise Exception("unsupported system")
        if platform.architecture() != ("64bit", "WindowsPE"):
            raise Exception("unsupported architecture")
        return python_version, python_version, "win_amd64"


def build_native():
    python_version = f"{sys.version_info[0]}.{sys.version_info[1]}.x"
    env = os.environ.copy()
    env["PYTHON_VERSION"] = python_version
    proj_dir = os.path.dirname(__file__)
    subprocess.run(
        ["xmake", "f", "-p", "windows", "-a", "x64", "-m", "release", "-y"],
        env=env,
        cwd=proj_dir,
        check=True,
    )
    subprocess.run(
        ["xmake", "-v", "-y"],
        env=env,
        cwd=proj_dir,
        check=True,
    )

if not os.path.exists(bin_path):
    build_native()

setup(
    cmdclass={"bdist_wheel": TagProvider},
)
