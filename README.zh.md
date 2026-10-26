# LeviStone

<img src="docs/logo.png" alt="" width="128" align="right">

[English](README.md) | 中文

一个用于在 [LeviLamina](https://github.com/LiteLDev/LeviLamina) 上运行 [Endstone](https://github.com/EndstoneMC/endstone) 插件的运行时。

## 安装

在已安装 LeviLamina 的基岩版专用服务器目录中运行：

```bash
pip install levistone --target plugins/EndstoneRuntime
```
LeviStone 是一个名为 `EndstoneRuntime`（即其附带的 Endstone 运行时）的 LeviLamina 模组，因此安装到该模组的目录 `plugins/EndstoneRuntime`。

你也可以在 levistone 后加上 == 来安装指定版本。  
例如，安装 0.11.13 版本：
```bash
pip install levistone==0.11.13 --target plugins/EndstoneRuntime
```
你可以在 [PyPI](https://pypi.org/project/levistone/#history) 上找到所有版本。

你正在使用Docker?试试 [docker-levistone-server](https://github.com/LiteLDev/docker-levistone-server)

## 常见问题

### 我可以运行 Endstone 的原生插件吗？

不可以，LeviStone 只支持用 Python 编写的插件。

### 为什么有些 Endstone 版本不受支持？

LeviLamina 只支持每个 Minecraft 协议版本中的一个版本，而 Endstone 支持多个版本。

### 如何知道我应该安装哪个版本？

通过查看 [LeviLamina支持的版本](https://docs.levimc.org/LeviLamina/zh/versions/) 和 [Endstone的发布页面](https://github.com/EndstoneMC/endstone/releases)。

## 许可证

Apache-2.0。LeviStone 基于 The Endstone Project 的 [Endstone](https://github.com/EndstoneMC/endstone)。
