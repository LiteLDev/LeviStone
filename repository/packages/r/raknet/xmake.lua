package("raknet")
    set_homepage("https://github.com/facebookarchive/RakNet")
    set_description("Cross-platform C++ networking engine for game programmers (Oculus VR fork, archived).")
    set_license("BSD-2-Clause")

    set_urls("https://github.com/facebookarchive/RakNet/archive/1a169895a900c9fc4841c556e16514182b75faf8.tar.gz")
    add_versions("4.081", "79d42a4daef9201ba93a4204257d740ffa6d225e37805b8a3b915c350b721144")

    add_configs("mojang", {description = "Apply Mojang BDS compatibility patches.", default = true, type = "boolean"})
    add_configs("minecraft_version", {description = "BDS version for the RakPeerInterface vtable patch.", default = "r26u3", values = {"r26u2", "r26u3"}})

    add_deps("cmake")

    add_patches("4.081", "patches/4.081-fix-gettimeofday-include.patch", "d6389090efd6816ac8c4511f69ae744a9213e5fd595cde239213ae643124e3c7")
    add_patches("4.081", "patches/4.081-linux-portability.patch", "50e914511c9a5d6ce261c646584e4d6dbdd964ebcb53f8edf369632c0e4fc9d2")
    add_patches("4.081", "patches/4.081-enable-ipv6.patch", "e946a50bbea979672a83a5bc0370f2484efe9b61f4de9181a99fd86d17b8beca")
    add_patches("4.081", "patches/4.081-disable-plugins.patch", "68130ebd50dbfc4de4100bc9cfb821472a92645c745a9a5027fd3539c34d98a8")

    if is_plat("windows") then
        add_syslinks("ws2_32")
    else
        add_syslinks("pthread")
    end

    on_install("windows", "linux", function (package)
        import("devel.git")

        -- add_patches() cannot key on configs, so the Mojang patches are
        -- applied here, mirroring the endstone conan recipe.
        if package:config("mojang") then
            local patchesdir = path.join(package:scriptdir(), "patches")
            local function apply_patch(name, sha256)
                local patchfile = path.join(patchesdir, name)
                local tmpfile = os.tmpfile(name)
                local content = io.readfile(patchfile, {encoding = "binary"}):gsub("\r\n", "\n")
                io.writefile(tmpfile, content, {encoding = "binary"})
                if hash.sha256(tmpfile) ~= sha256 then
                    raise("patch(%s): unmatched checksum!", name)
                end
                git.apply(tmpfile)
                os.rm(tmpfile)
            end
            apply_patch("4.081-mojang-define-overrides.patch", "d9c8faac1ddb34928e2b37b6606ca18a70f56103af3dbdf0d6e3e4a0b578d8a8")
            apply_patch("4.081-mojang-rakpeer-vtable-" .. package:config("minecraft_version") .. ".patch", package:config("minecraft_version") == "r26u2" and "e7761343e82edc6e09ed8fecc8807216931ebbc5fbc3bfa2551b7bdc71271f5d" or "70b400e8e928d3b6f2c04c700c1ebdac69b82437df86d1d1bbe74f56e4d30f6a")
        end

        os.cp(path.join(package:scriptdir(), "cmake", "CMakeLists.txt"), "CMakeLists.txt")
        os.cp(path.join(package:scriptdir(), "cmake", "raknetConfig.cmake.in"), "raknetConfig.cmake.in")

        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DRAKNET_BUILD_SHARED=OFF")
        import("package.tools.cmake").install(package, configs)
    end)

    -- The Mojang variant intentionally keeps RakPeer overrides undefined;
    -- consumers must provide the link stubs (see rak_peer_helper.cpp).
    on_test(function (package)
        assert(package:has_cxxincludes("RakPeerInterface.h"))
    end)

package_end()
