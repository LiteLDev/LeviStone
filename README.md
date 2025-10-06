# LeviStone

English | [中文](README.zh.md)

A runtime for running [Endstone](https://github.com/EndstoneMC/endstone) plugins on [LeviLamina](https://github.com/LiteLDev/LeviLamina)

## Installation

```bash
pip install levistone --target plugins/EndstoneRuntime
```
You can also add == after levistone to install a specific version.  
For example, to install version 0.10.5:
```bash
pip install levistone==0.10.5 --target plugins/EndstoneRuntime
```
You can find versions on [PyPI](https://pypi.org/project/levistone/#history).

## FAQ

### Can I run Endstone native plugins?

No, LeviStone only supports plugins written in Python.

### Why are some Endstone versions not supported?

LeviLamina only supports one version from each Minecraft protocol version, while Endstone supports multiple versions.

### How do I know which version I should install?

Check the [LeviLamina supported versions](https://lamina.levimc.org/versions/) and the [Endstone release page](https://github.com/EndstoneMC/endstone/releases).