#include "platform_dynamic_lib.hpp"
#include <cstring>
#include <cerrno>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>

#ifdef _WIN32
#include <shlwapi.h>
#include < VersionHelpers.h>
#else
#include <dlfcn.h>
#endif

namespace mem_band {

PlatformDynamicLib::LibraryHandle::~LibraryHandle() {
    close();
}

void PlatformDynamicLib::LibraryHandle::close() {
    if (handle != nullptr) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
#else
        dlclose(handle);
#endif
        handle = nullptr;
    }
}

PlatformDynamicLib::~PlatformDynamicLib() {
    for (auto& lib : loaded_libraries_) {
        lib.close();
    }
    loaded_libraries_.clear();
}

PlatformDynamicLib::LibraryHandle PlatformDynamicLib::open_library(
    const std::string& library_name, const std::string& search_path) {
    
    void* temp_handle = nullptr;
    std::string full_path;

    if (!search_path.empty()) {
        full_path = search_path + "/" + library_name;
#ifdef _WIN32
        temp_handle = LoadLibraryW(std::wstring(full_path.begin(), full_path.end()).c_str());
#else
        temp_handle = dlopen(full_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
        if (temp_handle == nullptr) {
            full_path.clear();
        }
    }

    if (temp_handle == nullptr) {
#ifdef _WIN32
        temp_handle = LoadLibraryA(library_name.c_str());
#else
        temp_handle = dlopen(library_name.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
    }

    if (temp_handle != nullptr) {
        loaded_libraries_.emplace_back(reinterpret_cast<void*>(temp_handle), full_path);
        return LibraryHandle(reinterpret_cast<void*>(temp_handle), full_path);
    }

    return LibraryHandle();
}

std::vector<std::string> PlatformDynamicLib::find_libraries(
    const std::string& library_name, const std::vector<std::string>& search_paths) {
    
    std::vector<std::string> found;
    
    for (const auto& path : search_paths) {
        std::string full_path = path + "/" + library_name;
#ifdef _WIN32
        void* test = LoadLibraryA(full_path.c_str());
        if (test != nullptr) {
            FreeLibrary(static_cast<HMODULE>(test));
            found.push_back(full_path);
        }
#else
        void* test = dlopen(full_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (test != nullptr) {
            dlclose(test);
            found.push_back(full_path);
        }
#endif
    }

    if (found.empty()) {
#ifdef _WIN32
        void* test = LoadLibraryA(library_name.c_str());
        if (test != nullptr) {
            FreeLibrary(static_cast<HMODULE>(test));
            found.push_back(library_name);
        }
#else
        void* test = dlopen(library_name.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (test != nullptr) {
            dlclose(test);
            found.push_back(library_name);
        }
#endif
    }

    return found;
}

std::string PlatformDynamicLib::get_error() {
#ifdef _WIN32
    DWORD err = GetLastError();
    if (err == 0) return std::string("No error");
    
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                  nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
    
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
#else
    const char* err = dlerror();
    return err ? std::string(err) : std::string("No error");
#endif
}

std::vector<std::string> PlatformDynamicLib::get_default_search_paths() {
    std::vector<std::string> paths;
    
#ifdef _WIN32
    paths = {
        "C:\\Windows\\System32",
        "C:\\Windows\\SysWOW64",
        "C:\\Program Files\\NVIDIA Corporation",
        "C:\\Program Files (x86)\\NVIDIA Corporation",
        "C:\\Program Files\\ROCm",
        "C:\\Program Files\\Adobe",
        "C:\\Program Files (x86)\\Adobe"
    };
    
    char windowsDir[MAX_PATH];
    GetWindowsDirectoryA(windowsDir, MAX_PATH);
    paths.push_back(std::string(windowsDir) + "\\System32\\DriverStore\\FileRepository");
    
    char* programFiles = nullptr;
    size_t len = 0;
    _dupenv_s(&programFiles, &len, "PROGRAMFILES");
    if (programFiles != nullptr) {
        paths.push_back(std::string(programFiles));
        free(programFiles);
    }
    
    char* programFilesX86 = nullptr;
    _dupenv_s(&programFilesX86, &len, "PROGRAMFILES(X86)");
    if (programFilesX86 != nullptr) {
        paths.push_back(std::string(programFilesX86));
        free(programFilesX86);
    }
    
    char* pathEnv = nullptr;
    _dupenv_s(&pathEnv, &len, "PATH");
    if (pathEnv != nullptr) {
        std::string pathStr(pathEnv);
        size_t pos = 0;
        while ((pos = pathStr.find(';')) != std::string::npos) {
            std::string segment = pathStr.substr(0, pos);
            if (!segment.empty()) {
                paths.push_back(segment);
            }
            pathStr.erase(0, pos + 1);
        }
        if (!pathStr.empty()) {
            paths.push_back(pathStr);
        }
        free(pathEnv);
    }
#else
    paths = {
        "/usr/lib",
        "/usr/lib64",
        "/usr/local/lib",
        "/usr/local/lib64",
        "/lib",
        "/lib64",
        "/opt/lib",
        "/opt/lib64"
    };

    char* ld_config = getenv("LD_LIBRARY_PATH");
    if (ld_config != nullptr) {
        std::string ld_path(ld_config);
        size_t pos = 0;
        while ((pos = ld_path.find(':')) != std::string::npos) {
            std::string segment = ld_path.substr(0, pos);
            if (!segment.empty()) {
                paths.push_back(segment);
            }
            ld_path.erase(0, pos + 1);
        }
        if (!ld_path.empty()) {
            paths.push_back(ld_path);
        }
    }
#endif

    return paths;
}

}
