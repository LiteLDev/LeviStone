# LeviStone

[English](README.md) | 中文

一个用于在 [LeviLamina](https://github.com/LiteLDev/LeviLamina) 上运行 [Endstone](https://github.com/EndstoneMC/endstone) 插件的运行时。

## 安装

```bash
pip install levistone --target plugins/EndstoneRuntime
```
你也可以在 levistone 后加上 == 来安装指定版本。  
例如，安装 0.10.5 版本：
```bash
pip install levistone==0.10.5 --target plugins/EndstoneRuntime
```
你可以在 [PyPI](https://pypi.org/project/levistone/#history) 上找到所有版本。

## 常见问题

### 我可以运行 Endstone 的原生插件吗？

不可以，LeviStone 只支持用 Python 编写的插件。

### 为什么有些 Endstone 版本不受支持？

LeviLamina 只支持每个 Minecraft 协议版本中的一个版本，而 Endstone 支持多个版本。
