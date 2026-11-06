#ifdef _WIN32
#include <Windows.h>
#include <winioctl.h>
#endif

namespace levistone {

namespace {

#ifdef _WIN32

struct JunctionReparseBuffer {
    ULONG ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    USHORT SubstituteNameOffset;
    USHORT SubstituteNameLength;
    USHORT PrintNameOffset;
    USHORT PrintNameLength;
    WCHAR PathBuffer[1];
};

std::error_code createJunction(std::filesystem::path const &link, std::filesystem::path const &target)
{
    std::error_code ec;
    auto targetPath = std::filesystem::absolute(target, ec).lexically_normal();
    if (ec) {
        return ec;
    }
    if (targetPath.filename().empty() && targetPath.has_parent_path()) {
        targetPath = targetPath.parent_path();
    }

    auto const targetNative = targetPath.native();
    auto substitute =
        targetNative.starts_with(L"\\\\") ? L"\\??\\UNC\\" + targetNative.substr(2) : L"\\??\\" + targetNative;

    auto const substituteBytes = static_cast<DWORD>(substitute.size() * sizeof(wchar_t));
    auto const printBytes = static_cast<DWORD>(targetNative.size() * sizeof(wchar_t));
    auto const pathBytes =
        substituteBytes + static_cast<DWORD>(sizeof(wchar_t)) + printBytes + static_cast<DWORD>(sizeof(wchar_t));

    std::vector<char> buffer(offsetof(JunctionReparseBuffer, PathBuffer) + pathBytes, 0);
    auto *const reparse = reinterpret_cast<JunctionReparseBuffer *>(buffer.data());

    reparse->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
    reparse->ReparseDataLength = static_cast<USHORT>(8 + pathBytes);
    reparse->Reserved = 0;
    reparse->SubstituteNameOffset = 0;
    reparse->SubstituteNameLength = static_cast<USHORT>(substituteBytes);
    reparse->PrintNameOffset = static_cast<USHORT>(substituteBytes + sizeof(wchar_t));
    reparse->PrintNameLength = static_cast<USHORT>(printBytes);

    auto *const pathBuffer = reparse->PathBuffer;
    std::memcpy(pathBuffer, substitute.data(), substituteBytes);
    pathBuffer[substitute.size()] = L'\0';
    std::memcpy(pathBuffer + substitute.size() + 1, targetNative.data(), printBytes);
    pathBuffer[substitute.size() + 1 + targetNative.size()] = L'\0';

    if (!::CreateDirectoryW(link.c_str(), nullptr)) {
        return {static_cast<int>(::GetLastError()), std::system_category()};
    }

    auto const handle = ::CreateFileW(link.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        ec = std::error_code(static_cast<int>(::GetLastError()), std::system_category());
        ::RemoveDirectoryW(link.c_str());
        return ec;
    }

    DWORD returned = 0;
    auto const succeeded = ::DeviceIoControl(handle, FSCTL_SET_REPARSE_POINT, buffer.data(),
                                             static_cast<DWORD>(buffer.size()), nullptr, 0, &returned, nullptr);
    ec = succeeded ? std::error_code{} : std::error_code(static_cast<int>(::GetLastError()), std::system_category());

    ::CloseHandle(handle);
    if (!succeeded) {
        ::RemoveDirectoryW(link.c_str());
    }
    return ec;
}

#endif

std::error_code createDirectoryLink(std::filesystem::path const &link, std::filesystem::path const &target)
{
#ifdef _WIN32
    if (auto ec = createJunction(link, target); !ec) {
        return {};
    }
#endif
    std::error_code ec;
    std::filesystem::create_directory_symlink(target, link, ec);
    return ec;
}

}  // namespace

void ensurePluginsLink(ll::io::Logger &logger)
{
    std::error_code ec;
    auto const serverRoot = std::filesystem::current_path(ec);
    if (ec) {
        return;
    }

    auto const &modsRoot = ll::mod::getModsRoot();
    auto const pluginsPath = serverRoot / "plugins";

    if (std::filesystem::exists(pluginsPath, ec)) {
        if (std::filesystem::equivalent(pluginsPath, modsRoot, ec)) {
            return;
        }

        logger.error("Failed to create a link at {0}: a file or directory with the same name already exists"_tr(
            ll::string_utils::u8str2str(pluginsPath.u8string())));
        return;
    }
    ec.clear();

    if (auto linkError = createDirectoryLink(pluginsPath, modsRoot); linkError) {
        logger.error("Failed to create a link at {0} pointing to {1}: {2}"_tr(
            ll::string_utils::u8str2str(pluginsPath.u8string()), ll::string_utils::u8str2str(modsRoot.u8string()),
            linkError.message()));
    }
}

}  // namespace levistone
