add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("libbase64")
add_requires("boost 1.86.0")
add_requires("moodycamelconqueue")
add_requires("cpptrace 0.7.5")
add_requires("date 3.0.4")
add_requires("toml++ v3.4.0")
add_requires("microsoft-detours ea6c4ae7f3f1b1772b8a7cda4199230b932f5a50")
add_requires("funchook v1.1.3")

add_requires("levilamina 1.6.0")
add_requires("levibuildscript")

python_version = "3.12.x"
if os.getenv("PYTHON_VERSION") then
    python_version = os.getenv("PYTHON_VERSION")
end
local get_python_libname = function (version)
    local major, minor = version:match("(%d+)%.(%d+)")
    return "python" .. major .. minor
end
local python_libname = get_python_libname(python_version)
add_requires("python " .. python_version)
add_requires("pybind11 3.0.1")

-- Define common packages to avoid repetition
local common_packages = {
    "libbase64", "boost", "moodycamelconqueue",
    "cpptrace", "date", "toml++", "levilamina", "microsoft-detours"
}

local get_version = function (oss)
        oss.cd("endstone")
        local tags = oss.iorun("git describe --tags --long")
        oss.cd("..")
        local tag, num_commits, commit_hash = tags:match("v?(%S+)-(%d+)-g([a-f0-9]+)")
        if tonumber(num_commits) > 0 then
            tag = tag:match("%d+.%d+.%d+")
            tag = tag.."-dev"..num_commits
        end
        return tag
    end

set_project("endstone")
set_languages("c++20")
set_runtimes("MD")
add_defines("PYBIND11_DETAILED_ERROR_MESSAGES")

target("endstone")
    set_kind("headeronly")
    set_languages("c++20")
    add_includedirs("endstone/include", {public = true})
    add_headerfiles("endstone/include/(**.h)")
    on_load(function (target)
        os.cd("$(projectdir)/endstone")
        os.runv("git", {"restore", "."})
        os.cd("$(projectdir)")
        os.runv("git", {"apply", "patches/feat__add_support_for_BDS_v1_21_113.patch"})
        local toml = import("scripts.toml")
        local symbols = toml.parse(io.readfile( "endstone/src/bedrock/symbol_generator/symbols.toml"))[target:plat()]
        local count = 0
        for _ in pairs(symbols) do count = count + 1 end
        local file = assert(io.open("$(builddir)/generated/bedrock_symbols.generated.h", "w"), "Failed to open symbol file")
        file:write("#pragma once\n\n")
        file:write("// clang-format off\n")
        file:write("#include <array>\n")
        file:write("#include <string_view>\n\n")
        file:write("static constexpr std::array<std::pair<std::string_view, std::size_t>, ")
        file:write(count)
        file:write("> symbols = {{\n")
        for k, v in pairs(symbols) do
            file:print("    { \"%s\", %d },", k, v)
        end
        file:write("}};\n")
        file:write("// clang-format on\n")
        file:close()
    end)

target("endstone_python")
    add_rules("python.module")
    add_files("src/levistone/memory_operators.cpp")
    add_files("endstone/src/endstone/python/**.cpp")
    add_deps("endstone")
    add_packages(common_packages)
    add_packages("python", "pybind11", {links = python_libname})
    set_symbols("debug")
    after_build(function (target)
        local file = assert(io.open("endstone/endstone/_internal/version.py", "w"))
        file:write("TYPE_CHECKING = False\n")
        file:write("if TYPE_CHECKING:\n")
        file:write("    from typing import Tuple, Union\n")
        file:write("    VERSION_TUPLE = Tuple[Union[int, str], ...]\n")
        file:write("else:\n")
        file:write("    VERSION_TUPLE = object\n\n")
        file:write("version: str\n")
        file:write("__version__: str\n")
        file:write("__version_tuple__: VERSION_TUPLE\n")
        file:write("version_tuple: VERSION_TUPLE\n\n")
        local version = get_version(os)
        file:print("__version__ = version = '%s'", version)
        local major, minor, patch, suffix = version:match("(%d+)%.(%d+)%.(%d+)(.*)")
        file:printf("__version_tuple__ = version_tuple = (%d, %d, %d", major, minor, patch)
        if suffix == nil then
            file:print(")")
        else
            file:print(", '%s')", suffix:sub(2))
        end
        file:close()
        if not os.isdir("bin/EndstoneRuntime/Lib/endstone/_internal") then
            os.mkdir("bin/EndstoneRuntime/Lib/endstone/_internal")
        end
        os.cp("endstone/endstone", "bin/EndstoneRuntime/Lib")
        os.cp(target:targetfile(), "bin/EndstoneRuntime/Lib/endstone/_internal")
    end)
    

target("endstone_core")
    set_kind("static")
    set_languages("c++20")
    add_includedirs("endstone/src", "$(builddir)/generated", {public = true})
    add_files("endstone/src/bedrock/**.cpp")
    add_files("endstone/src/endstone/core/**.cpp")
    add_files("src/levistone/core/**.cpp")
    remove_files("endstone/src/endstone/core/command/defaults/version_command.cpp")
    remove_files("endstone/src/endstone/core/devtools/**.cpp")
    remove_files("endstone/src/endstone/core/spdlog/**.cpp")
    remove_files("endstone/src/endstone/core/crash_handler.cpp")
    remove_files("endstone/src/endstone/core/logger_factory.cpp")
    remove_files("endstone/src/endstone/core/signal_handler.cpp")
    add_deps("endstone")
    add_packages(common_packages)
    add_packages("python", "pybind11", {links = python_libname})
    on_load(function (target)
        target:add("defines", "ENDSTONE_VERSION=\""..get_version(os).."\"")
    end)

target("endstone_runtime")
    set_kind("shared")
    on_load(function (target)
        target:add("rules", "@levibuildscript/modpacker", {
            modName = "EndstoneRuntime", 
            modVersion = get_version(os)
        })
    end)
    add_files("src/levistone/memory_operators.cpp")
    add_files("endstone/src/endstone/runtime/windows.cpp")
    add_files("endstone/src/endstone/runtime/vtable_hook.cpp")
    add_files("endstone/src/endstone/runtime/bedrock_hooks/**.cpp")
    add_files("src/levistone/runtime/**.cpp")
    add_deps("endstone", "endstone_core")
    add_packages(common_packages)
    add_packages("python", "pybind11", {links = python_libname})
    add_packages("funchook")
    add_links("dbghelp", "ws2_32","Psapi")
    set_symbols("debug")
