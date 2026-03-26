import re
import os
import sys
import shutil
import platform
import subprocess
from setuptools import setup, find_packages
from setuptools.command.bdist_wheel import bdist_wheel
from glob import glob


data_path = "bin/EndstoneRuntime"


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

if not os.path.exists(data_path):
    build_native()

setup(
    packages=find_packages(data_path) + ["."],
    package_dir={"": data_path},
    package_data={"": ["*"]},
    setup_requires=["setuptools_scm"],
    use_scm_version={
        "root": "endstone",
        "relative_to": __file__,
        "version_scheme": "only-version",
        "local_scheme": "no-local-version",
    },
    cmdclass={"bdist_wheel": TagProvider},
)
