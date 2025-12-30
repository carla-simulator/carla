// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLEntityResolver.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/Platform.h"
#include "GenericPlatform/GenericApplication.h"
#include "Containers/ArrayBuilder.h"
#include "MDLPathUtility.h"
#include "MDLModule.h"

using namespace MDLPathUtility;
#if WITH_MDL_SDK


namespace
{

    static char const UDIM_MARI_MARKER[] = "<UDIM>";
    static char const UDIM_ZBRUSH_MARKER[] = "<UVTILE0>";
    static char const UDIM_MUDBOX_MARKER[] = "<UVTILE1>";

    /// Return the OS-dependent path separator. This is the character separating the single
    /// directories from each other. E.g.  "\\" or "/".
    inline char os_separator()
    {
        return '/';
    }

    /// Join two paths using the OS specific separator.
    ///
    /// \param path1  incoming path
    /// \param path2  incoming path
    ///
    /// \return the resulting string; if path1 or path2 is empty returns the other component, else
    ///         both component connected by OS-specific path separator
    std::string join_path(
        std::string const& path1,
        std::string const& path2)
    {
        if (path1.empty())
        {
            return path2;
        }
        if (path2.empty())
        {
            return path1;
        }
        size_t l = path1.length() - 1;
        if (path1[l] == os_separator())
        {
            return path1 + path2;
        }
        return path1 + os_separator() + path2;
    }

    /// Simplifies a file path by removing directory names "." and pairs of directory names like
    /// ("foo", ".."). Slashes are used as separators. Leading and trailing slashes in the input are
    /// preserved.
    ///
    /// \param file_path  the file path to simplify
    /// \param sep        the separator (as string)
    /// The input must be valid w.r.t. to the number of directory names "..".
    std::string simplify_path(
        std::string const& file_path,
        char              sep)
    {
        check(!file_path.empty());

        TArray<std::string> directory_names;

        size_t start = 0;
        size_t length = file_path.size();

        size_t slash;
        do
        {
            slash = file_path.find(sep, start);
            if (slash == std::string::npos)
            {
                slash = length;
            }
            std::string directory_name = file_path.substr(start, slash - start);
            if (directory_name == ".")
            {
                // ignore
            }
            else if (directory_name == "..")
            {
                // drop one
                if (directory_names.Num() == 0)
                {
                    // trying to go above root. Linux AND windows allow this.
                    // In Unix, '..' is always a link, and in the case of root a link to itself
                    // Windows handles it the same way, so allow it
                }
                else
                {
                    directory_names.Pop();
                }
            }
            else if (!directory_name.empty())
            {
                if (directory_names.Num() == 0)
                {
                    // check website
                    if (file_path.find(sep, slash + 1) == slash + 1)
                    {
                        directory_name += sep;
                    }
                }

                directory_names.Push(directory_name);
            }
            start = slash + 1;
        } while (slash != length);

        std::string result;
        if (file_path[0] == sep)
        {
            result += sep;
        }
        if (length > 1 && file_path[1] == sep)
        {
            result += sep;
        }
        if (directory_names.Num() != 0)
        {
            result += directory_names[0];
        }
        for (size_t i = 1, n = directory_names.Num(); i < n; ++i)
        {
            result += sep + directory_names[i];
        }
        if (file_path[length - 1] == sep && (result.length() != 1 || result[0] != sep))
        {
            result += sep;
        }

        return result;
    }

    /// Converts OS-specific directory separators into slashes.
    ///
    /// \param s  the string to convert
    std::string convert_os_separators_to_slashes(std::string const& s)
    {
        char sep = os_separator();
        if (sep == '/')
        {
            return s;
        }

        std::string r(s);

        for (size_t i = 0, n = r.length(); i < n; ++i)
        {
            if (r[i] == sep)
            {
                r[i] = '/';
            }
        }
        return r;
    }

    /// Converts slashes into OS-specific directory separators.
    ///
    /// \param s  the string to convert
    std::string convert_slashes_to_os_separators(std::string const& s)
    {
        char sep = os_separator();
        if (sep == '/')
        {
            return s;
        }

        std::string r(s);

        for (size_t i = 0, n = r.length(); i < n; ++i)
        {
            if (r[i] == '/')
            {
                r[i] = sep;
            }
        }
        return r;
    }

    /// Retrieve the current working directory.
    ///
    /// \return the current working directory or the empty string else
    std::string get_cwd()
    {
        FString CurrentWorkingDirectory = FPlatformProcess::GetCurrentWorkingDirectory();
        std::string result = TCHAR_TO_UTF8(*CurrentWorkingDirectory);
        return result;
    }

    // test if character is file path separator
    static inline bool is_path_separator(
        char  c)
    {
        return c == '/' || c == '\\';
    }

    /// Return true if a path is absolute, i.e. begins with / or X:/
    ///
    /// \param path   check this path
    bool is_path_absolute(
        char const* path)
    {
        if (path == nullptr)
        {
            return false;
        }

        if (is_path_separator(path[0]))
        {
            return true;
        }

        if (strstr(path, ":"))
        {
            return true;
        }

        return false;
    }

    /// Implementation of the IInput_stream interface using FILE I/O.
    class FSimpleFileReader : public mi::base::Interface_implement<IReader>
    {
        typedef mi::base::Interface_implement<IReader> Base;
    public:
        /// Constructor.
        ///
        /// \param f         the FILE handle, takes ownership
        /// \param filename  the filename of the handle
        explicit FSimpleFileReader(
            IFileHandle* InFileHandle,
            char const* filename)
            : FileHandle(InFileHandle)
            , FileName(filename)
            , ReadPosition(0)
        {}

        /// Destructor.
        ///
        /// \note Closes the file handle.
        ~FSimpleFileReader()
        {
            delete FileHandle;
        }

        /// Read a character from the input stream.
        /// \returns    The code of the character read, or -1 on the end of the stream.
        virtual mi::Sint64 read(char* buffer, mi::Sint64 size)
        {
            if (ReadPosition < FileHandle->Size())
            {
                if (FileHandle->Seek(ReadPosition))
                {
                    ReadPosition += size;

                    if (FileHandle->Read((uint8*)buffer, size))
                    {
                        return size;
                    }
                }
            }

            return -1;
        }

        virtual bool readline(char* buffer, mi::Sint32 size)
        {
            return false;
        }

        virtual bool supports_lookahead() const
        {
            return false;
        }

        virtual mi::Sint64 lookahead(mi::Sint64 size, const char** buffer) const
        {
            return -1;
        }

        virtual mi::Sint32 get_error_number() const
        {
            return 0;
        }

        /// Returns the error message of the last error that happened in this reader or writer.
        /// Returns \c NULL if #get_error_number() returns 0.
        virtual const char* get_error_message() const
        {
            return nullptr;
        }

        /// Returns \c true if the end of the file has been reached.
        /// The result is undefined before reading or writing for the first time.
        virtual bool eof() const
        {
            return false;
        }

        /// Returns the file descriptor of the stream, or -1 if it is not available.
        virtual mi::Sint32 get_file_descriptor() const
        {
            return -1;
        }

        virtual bool supports_recorded_access() const
        {
            return false;
        }

        /// Returns the current position in this stream.
        virtual const IStream_position* tell_position() const
        {
            return nullptr;
        }

        /// Repositions the stream to the position \p stream_position.
        /// \return \c true in case of success, or \c false in case of errors, e.g., if
        ///         \p stream_position is not valid or recorded access is not supported and the state of
        ///         the stream remains unchanged.
        virtual bool seek_position(const IStream_position* stream_position)
        {
            return false;
        }

        /// Resets the stream position to the beginning.
        /// \return \c true in case of success (and clears the error condition), and \c false in case
        /// of errors.
        virtual bool rewind()
        {
            return false;
        }

        //@}
        /// \name Random absolute access
        //@{

        /// Returns \c true if random absolute access is supported, and \c false otherwise.
        virtual bool supports_absolute_access() const
        {
            return false;
        }

        /// Returns the absolute position in bytes from the beginning of the stream beginning, or -1
        /// if absolute access is not supported.
        virtual mi::Sint64 tell_absolute() const
        {
            return -1;
        }

        /// Repositions the stream to the absolute position \p pos.
        /// \return \c true in case of success, or \c false in case of errors, e.g., if \p pos is not
        ///         valid or absolute access is not supported and the state of the stream remains
        ///         unchanged.
        virtual bool seek_absolute(mi::Sint64 pos)
        {
            return false;
        }

        /// Returns the size in bytes of the data in the stream.
        /// Based on random access, this is a fast operation.
        virtual mi::Sint64 get_file_size() const
        {
            return 0;
        }

        /// Sets the stream position to the end of the file.
        /// \return \c true in case of success, or \c false in case of errors.
        virtual bool seek_end()
        {
            return false;
        }

    private:
        /// The file handle.
        IFileHandle* FileHandle;
        /// The filename.
        std::string FileName;
        /// Current Read Position
        mi::Sint64 ReadPosition;
    };

} // anonymous

// ------------------ import result ------------------

/// Return the absolute MDL name of the found entity, or nullptr, if the entity could not
/// be resolved.
char const* FMDLImportResult::get_module_name() const
{
    return AbsoluteName.empty() ? nullptr : AbsoluteName.c_str();
}

/// Return the OS-dependent file name of the found entity, or nullptr, if the entity could not
/// be resolved.
char const* FMDLImportResult::get_filename() const
{
    return FileName.empty() ? nullptr : FileName.c_str();
}

/// Return an input stream to the given entity if found, nullptr otherwise.
IReader* FMDLImportResult::create_reader() const
{
    if (FileName.empty())
    {
        return nullptr;
    }

    FString FilenameUTF8 = UTF8_TO_TCHAR(FileName.c_str());
    IFileHandle* FileHandle = nullptr;
    // Always assume local file
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FileHandle = PlatformFile.OpenRead(*FilenameUTF8);
    if (FileHandle == nullptr)
    {
        return nullptr;
    }

    // Only simple files yet
    return new FSimpleFileReader(
        FileHandle, FileName.c_str());
}

/// Constructor.
///
/// \param abs_name      the absolute name of the resolved import
/// \param os_file_name  the OS dependent file name of the resolved module.
FMDLImportResult::FMDLImportResult(
    TSharedPtr<IMDLExternalReader> InFileReader,
    std::string const& abs_name,
    std::string const& os_file_name)
    : FileReader(InFileReader)
    , AbsoluteName(abs_name)
    , FileName(os_file_name)
{
}

FMDLImportResult::~FMDLImportResult()
{
    FileReader = nullptr;
}


// --------------------------------------------------------------------------
// IMdl_resolved_resource_element
FMDLResourceElement::FMDLResourceElement(
    char const* url,
    char const* filename)
    : UrlMask(url)
    , FileNameMask(filename)
{
}

FMDLResourceElement::~FMDLResourceElement()
{
}

mi::Size FMDLResourceElement::get_frame_number() const
{
    return 0;
}

mi::Size FMDLResourceElement::get_count() const
{
    return 1;
}

// Get the i'th file name of the ordered set.
char const* FMDLResourceElement::get_filename(mi::Size i) const
{
    if (i > 0)
    {
        return nullptr;
    }

    return FileNameMask.c_str();
}

// Opens a reader for the i'th entry.
IReader* FMDLResourceElement::create_reader(mi::Size i) const
{
    return nullptr;
}

mi::base::Uuid FMDLResourceElement::get_resource_hash(mi::Size i) const
{
    return mi::base::Uuid();
}

// If the ordered set represents an UDIM mapping, returns it, otherwise nullptr.
bool FMDLResourceElement::get_uvtile_uv(mi::Size i, mi::Sint32& u, mi::Sint32& v) const
{
    return false;
}

// Get the i'th MDL url of the ordered set.
char const* FMDLResourceElement::get_mdl_file_path(mi::Size i) const
{
    if (i > 0)
    {
        return nullptr;
    }

    return UrlMask.c_str();
}
// --------------------------------------------------------------------------

// Constructor from one file name/url pair (typical case).
FMDLResourceSet::FMDLResourceSet(
    char const* url,
    char const* filename)
    : UvtileMode(UVTILE_MODE_NONE)
    , UrlMask(url)
    , FileNameMask(filename)
{
    ResourceElement = mi::base::make_handle(new FMDLResourceElement(url, filename));
}

FMDLResourceSet::~FMDLResourceSet()
{
}

// Get the MDL URL mask of the ordered set.
char const* FMDLResourceSet::get_mdl_file_path_mask() const
{
    return UrlMask.c_str();
}

// Get the file name mask of the ordered set.
char const* FMDLResourceSet::get_filename_mask() const
{
    return FileNameMask.c_str();
}

// Get the number of resolved file names.
mi::Size FMDLResourceSet::get_count() const
{
    return 1;
}

// Get the UDIM mode for this set.
Uvtile_mode FMDLResourceSet::get_uvtile_mode() const
{
    return UvtileMode;
}

bool FMDLResourceSet::has_sequence_marker() const
{
    return false;
}

const IMdl_resolved_resource_element* FMDLResourceSet::get_element(mi::Size i) const
{
    if (i > 0)
    {
        return nullptr;
    }

    if (ResourceElement)
    {
        ResourceElement->retain();
    }
    return ResourceElement.get();
}

// ------------------ entity resolver ----------------

// Resolve a resource file name.
IMdl_resolved_resource* FMDLEntityResolver::resolve_resource(
    const char* file_path,
    const char* owner_file_path,
    const char* owner_name,
    mi::Sint32 pos_line,
    mi::Sint32 pos_column,
    IMdl_execution_context* context)
{
    if (owner_name != nullptr && owner_name[0] == '\0')
    {
        owner_name = nullptr;
    }
    if (owner_file_path != nullptr && owner_file_path[0] == '\0')
    {
        owner_file_path = nullptr;
    }

    FString UnmangledFilePath = UnmangleMdlPath(UTF8_TO_TCHAR(file_path), false);

    // Make owner file system path absolute
    std::string owner_filename_str;
    if (owner_file_path != nullptr)
    {
        check(is_path_absolute(owner_file_path));
        owner_filename_str = owner_file_path;
    }

    Uvtile_mode uvtile_mode = UVTILE_MODE_NONE;

    FString UnmangledOwnerName;
    if (owner_name)
    {
        if (auto MDLPluginModule = FModuleManager::GetModulePtr<IMDLModule>("MDL"))
        {
            const mi::IString* DecodedOwnerName = MDLPluginModule->GetFactory()->decode_name(owner_name);
            UnmangledOwnerName = UnmangleMdlPath(UTF8_TO_TCHAR(DecodedOwnerName->get_c_str()));
        }
    }

    std::string os_file_name;
    std::string abs_file_name = resolve_filename(
        os_file_name, TCHAR_TO_UTF8(*UnmangledFilePath), /*is_resource=*/true,
        owner_filename_str.c_str(),
        owner_name ? TCHAR_TO_UTF8(*UnmangledOwnerName) : nullptr,
        uvtile_mode, pos_line, pos_column);

    if (os_file_name.empty())
    {
        return nullptr;
    }
    return new FMDLResourceSet(
        abs_file_name.c_str(),
        os_file_name.c_str());
}


/// Check if a given MDL url is absolute.
static bool is_url_absolute(char const* url)
{
    return url != nullptr && url[0] == '/';
}

// Resolve a MDL file name
std::string FMDLEntityResolver::resolve_filename(
    std::string& abs_file_name,
    char const* mdl_url,
    bool                    is_resource,
    char const* owner_file_path,
    char const* owner_name,
    Uvtile_mode& uvtile_mode,
    mi::Sint32 pos_line,
    mi::Sint32 pos_column)
{
    abs_file_name.clear();

    std::string module_file_system_path_str(owner_file_path == nullptr ? "" : owner_file_path);
    std::string owner_name_str(owner_name == nullptr ? "" : owner_name); 
    std::string url(mdl_url);

    size_t nesting_level = owner_name != nullptr ? get_module_nesting_level(owner_name) : 0;

    // check if this is an mdle or a resource inside one
    bool is_mdle = false;
    if (is_resource && owner_name != nullptr)
    {
        size_t l = owner_name_str.size();
        if (l > 5 &&
            owner_name[l - 5] == '.' &&
            owner_name[l - 4] == 'm' &&
            owner_name[l - 3] == 'd' &&
            owner_name[l - 2] == 'l' &&
            owner_name[l - 1] == 'e')
        {

            is_mdle = true;
        }
    }
    else
    {
        size_t l = url.size();
        if (l > 5 &&
            url[l - 5] == '.' &&
            url[l - 4] == 'm' &&
            url[l - 3] == 'd' &&
            url[l - 2] == 'l' &&
            url[l - 1] == 'e')
        {

            is_mdle = true;
        }
    }

    // Step 0: compute terms defined in MDL spec
    std::string directory_path, file_name;
    split_file_path(mdl_url, directory_path, file_name);

    std::string current_working_directory;
    bool cwd_is_archive = false;
    std::string current_search_path;
    bool csp_is_archive = false;
    std::string current_module_path;

    split_module_file_system_path(
        module_file_system_path_str,
        owner_name_str,
        
        nesting_level,
        current_working_directory,
        cwd_is_archive,
        current_search_path,
        csp_is_archive,
        current_module_path);

    // handle UDIM
    std::string url_mask;
    uvtile_mode = UVTILE_MODE_NONE;
    url_mask = url;

    // Step 1: normalize file path
    std::string canonical_file_path = normalize_file_path(
        url,
        url_mask,
        directory_path,
        file_name,
        nesting_level,
        module_file_system_path_str,
        current_working_directory,
        cwd_is_archive,
        current_module_path);
    if (canonical_file_path.empty())
    {
        if (is_resource)
        {
            // error: UNABLE_TO_RESOLVE
        }
        return std::string();
    }

    std::string canonical_file_mask(canonical_file_path);
    std::string resolved_file_system_location;
    if (is_mdle)
    {
        std::string file;

        if (is_resource)
        {
            file = simplify_path(url_mask, os_separator());
            if (!is_url_absolute(file.c_str()))
            {
                file =
#if defined(MI_PLATFORM_WINDOWS)
                    "/" +
#endif
                    current_working_directory + ':' + file;
            }
            file = convert_slashes_to_os_separators(file);

#if defined(MI_PLATFORM_WINDOWS)
            file = file.substr(1);
#endif
        }
        else
        {
            file = std::string(mdl_url);
            file = convert_slashes_to_os_separators(file);
        }

        if (!file_exists(file.c_str()))
        {
            // error: UNABLE_TO_RESOLVE
            return std::string();
        }

        resolved_file_system_location = file.c_str();
    }
    else
    {
        // Step 2: consider search paths
        resolved_file_system_location = consider_search_paths(
            canonical_file_mask, false, mdl_url, UVTILE_MODE_NONE);

        // the referenced resource is part of an MDLE
        // Note, this is invalid for mdl modules in the search paths!
        if (resolved_file_system_location.empty() && strstr(mdl_url, ".mdle:") != nullptr)
        {
#ifdef MI_PLATFORM_WINDOWS
            size_t offset = 1;
#else
            size_t offset = 0;
#endif
            if (file_exists(mdl_url + offset))
            {
                resolved_file_system_location = mdl_url + offset;
            }
        }

        if (resolved_file_system_location.empty())
        {
            return std::string();
        }
    }

#if 0
    // Step 3: consistency checks
    if (!check_consistency(
        resolved_file_system_location,
        canonical_file_path,
        url,
        current_working_directory,
        current_search_path,
        is_resource,
        csp_is_archive))
    {
        return std::string();
    }
#endif

    abs_file_name = resolved_file_system_location;

    if (is_resource)
    {
        // do nothing
        return canonical_file_path;
    }
    else
    {
        // convert to module name
        if (is_mdle)
        {
            std::string absolute_name = to_module_name(canonical_file_path.c_str());
            check(absolute_name.substr(absolute_name.size() - 5) == ".mdle");
            return absolute_name;
        }

        std::string absolute_name = to_module_name(canonical_file_path.c_str());
        check(absolute_name.substr(absolute_name.size() - 4) == ".mdl");
        return absolute_name.substr(0, absolute_name.size() - 4);
    }
}

// Resolve a module name.
IMdl_resolved_module* FMDLEntityResolver::resolve_module(
    const char* module_name,
    const char* owner_file_path,
    const char* owner_name,
    mi::Sint32 pos_line,
    mi::Sint32 pos_column,
    IMdl_execution_context* context)
{
    static const TArray<FString> StdModules = TArrayBuilder<FString>()
        .Add(FString(TEXT("anno.mdl")))
        .Add(FString(TEXT("builtins.mdl")))
        .Add(FString(TEXT("debug.mdl")))
        .Add(FString(TEXT("df.mdl")))
        .Add(FString(TEXT("limits.mdl")))
        .Add(FString(TEXT("math.mdl")))
        .Add(FString(TEXT("noise.mdl")))
        .Add(FString(TEXT("nvidia_baking.mdl"))) // mdl 1.7
        .Add(FString(TEXT("nvidia_df.mdl")))
        .Add(FString(TEXT("scene.mdl"))) // mdl 1.7
        .Add(FString(TEXT("state.mdl")))
        .Add(FString(TEXT("std.mdl")))
        .Add(FString(TEXT("tex.mdl")))
        .Add(FString(TEXT("base.mdl")));

    if (auto MDLPluginModule = FModuleManager::GetModulePtr<IMDLModule>("MDL"))
    {
        // Need decode module at first, mdl will encode module before resolve_module
        const mi::IString* DecodedModuleName = MDLPluginModule->GetFactory()->decode_name(module_name);
        FString MdlUrl(UTF8_TO_TCHAR(DecodedModuleName->get_c_str()));
        MdlUrl.ReplaceInline(TEXT("::"), TEXT("/"));
        if (!MdlUrl.EndsWith(TEXT(".mdl")))
        {
            MdlUrl += TEXT(".mdl");
        }

        for (auto Module : StdModules)
        {
            if (MdlUrl.Equals(Module) || MdlUrl.Equals(TEXT("/") + Module))
            {
                return nullptr;
            }
        }

        Uvtile_mode uvtile_mode;
        std::string resolved_file_system_location;

        FString UnmangledMdlUrl = UnmangleMdlPath(MdlUrl, false);
        bool IsMangled = !MdlUrl.Equals(UnmangledMdlUrl);

        FString UnmangledOwnerName;
        if (owner_name)
        {
            const mi::IString* DecodedOwnerName = MDLPluginModule->GetFactory()->decode_name(owner_name);
            UnmangledOwnerName = UnmangleMdlPath(UTF8_TO_TCHAR(DecodedOwnerName->get_c_str()));
        }

        std::string result = resolve_filename(
            resolved_file_system_location,
            TCHAR_TO_UTF8(*UnmangledMdlUrl),
            /*is_resource=*/false,
            owner_file_path,
            owner_name ? TCHAR_TO_UTF8(*UnmangledOwnerName) : nullptr,//owner_name,
            uvtile_mode,
            pos_line,
            pos_column);
        check(uvtile_mode == UVTILE_MODE_NONE);

        if (result.empty())
        {
            return nullptr;
        }

        FString OutModuleName(UTF8_TO_TCHAR(result.c_str()));
        // The output module name should be mangled when the input module_name is mangled
        if (IsMangled)
        {
            OutModuleName = MangleMdlPath(OutModuleName);
        }

        // Must encode module name before returning
        const mi::IString* EncodeModuleName = MDLPluginModule->GetFactory()->encode_module_name(TCHAR_TO_UTF8(*OutModuleName));

        return new FMDLImportResult(
            MdlFileReader,
            EncodeModuleName->get_c_str(),
            resolved_file_system_location);
    }

    return nullptr;
}

// Checks whether the resolved file system location passes the consistency checks in the MDL spec.
bool FMDLEntityResolver::check_consistency(
    std::string const& resolved_file_system_location,
    std::string const& canonical_file_path,
    std::string const& file_path,
    std::string const& current_working_directory,
    std::string const& current_search_path,
    bool              is_resource,
    bool              csp_is_archive)
{
    // strict relative file paths
    if (file_path.substr(0, 2) == "./" || file_path.substr(0, 3) == "../")
    {
        // should have already been rejected in the normalization step for string-based modules

        // check whether resolved file system location is in current search path
        size_t len = current_search_path.size();
        std::string resolved_search_path = resolved_file_system_location.substr(0, len);
        if (resolved_search_path == current_search_path &&
            (resolved_file_system_location[len] == os_separator() ||
                (len > 4 &&
                    resolved_file_system_location[len] == ':' &&
                    resolved_file_system_location[len - 1] == 'r' &&
                    resolved_file_system_location[len - 2] == 'd' &&
                    resolved_file_system_location[len - 3] == 'm' &&
                    resolved_file_system_location[len - 4] == '.')
                )
            )
        {
            return true;
        }
        else
        {
            // error: FILE_PATH_CONSISTENCY
            return false;
        }
    }

    // absolute or weak relative file paths

    // check precondition whether canonical file path exists w.r.t. current search path
    std::string canonical_file_path_os = convert_slashes_to_os_separators(canonical_file_path);
    std::string file;
    if (csp_is_archive)
    {
        // construct an "archive path"
        check(canonical_file_path_os[0] == os_separator());
        file = current_search_path + ':' + canonical_file_path_os.substr(1);
        if (!file_exists(file.c_str()))
        {
            return true;
        }
    }
    else
    {
        file = current_search_path + canonical_file_path_os;
        if (!file_exists(file.c_str()))
        {
            return true;
        }
    }

    // check precondition whether local file is in current working directory (and not below)
    size_t len = current_working_directory.size();
    std::string directory = file.substr(0, len + 1);
    if (directory != current_working_directory + os_separator())
    {
        return true;
    }
    if (file.substr(len + 1).find(os_separator()) != std::string::npos)
    {
        return true;
    }

    // check whether resolved file system location is different from file in current working
    // directory
    if (resolved_file_system_location != file)
    {
        // error: FILE_PATH_CONSISTENCY
        return false;
    }
    return true;
}

// Splits a file path into a directory path and file name.
void FMDLEntityResolver::split_file_path(
    std::string const& input_url,
    std::string& directory_path,
    std::string& file_path)
{
    std::string::size_type pos = input_url.rfind('/');
    if (pos != std::string::npos)
    {
        directory_path.assign(input_url.substr(0, pos + 1));
        file_path.assign(input_url.substr(pos + 1));
    }
    else
    {
        directory_path.clear();
        file_path.assign(input_url);
    }
}

// Splits a module file system path into current working directory, current search path, and
// current module path.
void FMDLEntityResolver::split_module_file_system_path(
    std::string const& module_file_system_path,
    std::string const& module_name,
    size_t            module_nesting_level,
    std::string& current_working_directory,
    bool& cwd_is_container,
    std::string& current_search_path,
    bool& csp_is_container,
    std::string& current_module_path)
{
    cwd_is_container = false;
    csp_is_container = false;
    // special case for string-based modules (not in spec)
    if (module_file_system_path.empty())
    {
        current_working_directory.clear();
        current_search_path.clear();

        // remove last "::" and remainder, replace "::" by "/"
        size_t pos = module_name.rfind("::");
        current_module_path = to_url(module_name.substr(0, pos).c_str());
        check(current_module_path.empty() || current_module_path[0] == '/');
        return;
    }

    // regular case for file-based modules (as in spec)
    char sep = os_separator();

    size_t container_pos = module_file_system_path.find(".mdr:");
    bool is_mdr_archive = true;

    if (container_pos == std::string::npos)
    {
        container_pos = module_file_system_path.find(".mdle:");
        is_mdr_archive = false;
    }

    if (container_pos != std::string::npos)
    {
        // inside an archive
        if (is_mdr_archive)
        {
            container_pos += 4; // add ".mdr"
                                // inside an archive
        }
        else
        {
            container_pos += 5; // add ".mdle"
        }

        std::string simple_path = module_file_system_path.substr(container_pos + 1);

        size_t last_sep = simple_path.find_last_of(sep);
        if (last_sep != std::string::npos)
        {
            current_working_directory =
                module_file_system_path.substr(0, container_pos + 1) +
                simple_path.substr(0, last_sep);
        }
        else
        {
            // only the archive
            current_working_directory = module_file_system_path.substr(0, container_pos);
            cwd_is_container = true;
        }

        // points now to ':'
        ++container_pos;
    }
    else
    {
        size_t last_sep = module_file_system_path.find_last_of(sep);
        check(last_sep != std::string::npos);
        current_working_directory = module_file_system_path.substr(0, last_sep);

        container_pos = 0;
    }

    current_search_path = current_working_directory;
    size_t strip_dotdot = 0;
    while (module_nesting_level-- > 0)
    {
        size_t last_sep = current_search_path.find_last_of(sep);
        check(last_sep != std::string::npos);
        if (last_sep < container_pos)
        {
            // do NOT remove the archive name, thread its ':' like '/'
            last_sep = container_pos - 1;
            // should never try to go out!
            check(module_nesting_level == 0);
            container_pos = 0;
            strip_dotdot = 1;
        }
        else
        {
            strip_dotdot = 0;
        }
        current_search_path = current_search_path.substr(0, last_sep);
    }

    csp_is_container = strip_dotdot != 0;
    current_module_path = convert_os_separators_to_slashes(
        current_working_directory.substr(current_search_path.size() + strip_dotdot));
    if (strip_dotdot)
    {
        current_module_path = '/' + current_module_path;
    }
    check(current_module_path.empty() || current_module_path[0] == '/');
}

// Converts OS-specific directory separators into slashes.
std::string FMDLEntityResolver::convert_os_separators_to_slashes(std::string const& s)
{
    char sep = os_separator();
    if (sep == '/')
    {
        return s;
    }

    std::string r(s);

    for (size_t i = 0, n = r.length(); i < n; ++i)
    {
        if (r[i] == sep)
        {
            r[i] = '/';
        }
    }
    return r;
}

// Returns the nesting level of a module, i.e., the number of "::" substrings in the
// fully-qualified module name minus 1.
size_t FMDLEntityResolver::get_module_nesting_level(char const* module_name)
{
    check(module_name[0] == ':' && module_name[1] == ':');

    char const* p = module_name;
    size_t     level = 0;
    do
    {
        ++level;
        p = strstr(p + 2, "::");
    } while (p != nullptr);

    check(level > 0);
    return level - 1;
}

// Checks that \p file_path contains no "." or ".." directory names.
bool FMDLEntityResolver::check_no_dots(
    char const* s)
{
    bool absolute = s[0] == '/';
    if (absolute)
    {
        ++s;
    }

    bool start = true;
    for (; s[0] != '\0'; ++s)
    {
        if (start)
        {
            if (s[0] == '.')
            {
                ++s;
                char const* p = ".";
                if (s[0] == '.')
                {
                    ++s;
                    p = "..";
                }
                if (s[0] == '/' || s[0] == '\0')
                {
                    // error: INVALID_DIRECTORY_NAME
                    return false;
                }
            }
            start = false;
        }
        if (s[0] == '/')
        {
            start = true;
        }
    }
    return true;
}

// Checks that \p file_path contains at most one leading "." directory name, at most
// nesting_level leading ".." directory names, and no such non-leading directory names.
bool FMDLEntityResolver::check_no_dots_strict_relative(
    char const* s,
    size_t     nesting_level)
{
    check(s[0] == '.');
    char const* b = s;

    bool leading = true;
    bool start = true;
    bool too_many = false;

    char const* err = nullptr;
    for (; s[0] != '\0'; ++s)
    {
        if (start)
        {
            if (s[0] == '.')
            {
                if (s[1] == '/' || s[1] == '\0')
                {
                    if (s != b)
                    {
                        err = ".";
                        break;
                    }
                    else
                    {
                        leading = false;
                    }
                }
                else if (s[1] == '.' && (s[2] == '/' || s[2] == '\0'))
                {
                    if (!leading)
                    {
                        err = "..";
                        break;
                    }
                    else if (nesting_level == 0)
                    {
                        too_many = true;
                        break;
                    }
                    else
                    {
                        --nesting_level;
                    }
                }
                else
                {
                    leading = false;
                }
            }
            else
            {
                leading = false;
            }
            start = false;
        }
        if (s[0] == '/')
        {
            start = true;
        }
    }

    if (err != nullptr)
    {
        // error: INVALID_DIRECTORY_NAME,
        return false;
    }
    if (too_many)
    {
        // error: INVALID_DIRECTORY_NAME_NESTING
        return false;
    }

    return true;
}

// Convert a name which might be either an url (separator '/') or an module name (separator '::')
// into an url.
std::string FMDLEntityResolver::to_url(
    char const* input_name) const
{
    std::string input_url;
    for (;;)
    {
        char const* p = strstr(input_name, "::");

        if (p == nullptr)
        {
            input_url.append(input_name);
            return input_url;
        }
        input_url.append(input_name, p - input_name);
        input_url.append("/");
        input_name = p + 2;
    }
}

// Convert an url (separator '/') into  a module name (separator '::')
std::string FMDLEntityResolver::to_module_name(
    char const* input_url) const
{
    // handle MDLe
    std::string input_url_str(input_url);
    size_t l = input_url_str.size();
    if (l > 5 &&
        input_url_str[l - 5] == '.' &&
        input_url_str[l - 4] == 'm' &&
        input_url_str[l - 3] == 'd' &&
        input_url_str[l - 2] == 'l' &&
        input_url_str[l - 1] == 'e')
    {

        std::string input_name("::");
        input_name.append(convert_os_separators_to_slashes(input_url_str));
        return input_name;
    }

    // handle MDL
    std::string input_name;
    for (;;)
    {
        char const* p = strchr(input_url, '/');

#ifdef MI_PLATFORM_WINDOWS
        char const* q = strchr(input_url, '\\');

        if (q != nullptr)
        {
            if (p == nullptr || q < p)
            {
                p = q;
            }
        }
#endif

        if (p == nullptr)
        {
            input_name.append(input_url);
            return input_name;
        }
        input_name.append(input_url, p - input_url);
        input_name.append("::");
        input_url = p + 1;
    }
}

// Normalizes a file path given by its directory path and file name.
std::string FMDLEntityResolver::normalize_file_path(
    std::string const& file_path,
    std::string const& file_mask,
    std::string const& directory_path,
    std::string const& file_name,
    size_t            nesting_level,
    std::string const& module_file_system_path,
    std::string const& current_working_directory,
    bool              cwd_is_archive,
    std::string const& current_module_path)
{
    // strict relative file paths
    if (file_path[0] == '.' &&
        (file_path[1] == '/' || (file_path[1] == '.' && file_path[2] == '/')))
    {
        // special case (not in spec)
        if (module_file_system_path.empty())
        {
            // error: strict relative path in string module is dangerous and that's why forbidden
            return std::string();
        }

        // reject invalid strict relative file paths
        if (!check_no_dots_strict_relative(file_path.c_str(), nesting_level))
        {
            return std::string();
        }

        // reject if file does not exist w.r.t. current working directory
        std::string file_mask_os = convert_slashes_to_os_separators(file_mask);

        std::string file = cwd_is_archive ?
            current_working_directory + ':' + simplify_path(file_mask_os, os_separator()) :
            simplify_path(join_path(current_working_directory, file_mask_os), os_separator());
        if (!file_exists(file.c_str()))
        {
            // does not exist
            return std::string();
        }

        // canonical path is the file path resolved w.r.t. the current module path
        return simplify_path(current_module_path + "/" + file_path, '/');
    }

    // absolute file paths
    if (file_path[0] == '/')
    {
        // reject invalid absolute paths
        if (!check_no_dots(file_path.c_str()))
        {
            return std::string();
        }

        // canonical path is the same as the file path
        return file_path;
    }


    // weak relative file paths

    // reject invalid weak relative paths
    if (!check_no_dots(file_path.c_str()))
    {
        return std::string();
    }

    // special case (not in spec)
    if (module_file_system_path.empty())
    {
        return "/" + file_path;
    }

    // if file does not exist w.r.t. current working directory: canonical path is file path
    // prepended with a slash
    std::string file_mask_os = convert_slashes_to_os_separators(file_mask);

    // if the searched file does not exists locally, assume the weak relative path is absolute
    if (!file_exists(current_working_directory.c_str(), file_mask_os.c_str()))
    {
        return "/" + file_path;
    }

    // otherwise canonical path is the file path resolved w.r.t. the current module path
    return current_module_path + "/" + file_path;
}

// Check if the given file name (UTF8 encoded) names a file on the file system or inside
// an archive.
bool FMDLEntityResolver::file_exists(
    char const* fname, char const* fmask) const
{
    std::string joined_file_name = fname;
    if (fmask)
    {
        joined_file_name = join_path(fname, fmask);
    }

    char const* p_archive = strstr(joined_file_name.c_str(), ".mdr:");
    char const* p_mdle = (p_archive == nullptr) ? strstr(joined_file_name.c_str(), ".mdle:") : nullptr;

    if (is_path_absolute(joined_file_name.c_str()) && p_archive == nullptr && p_mdle == nullptr)
    {
        // Always assume local file
        FString FilenameUTF8 = UTF8_TO_TCHAR(joined_file_name.c_str());
        return FPaths::FileExists(FilenameUTF8);
    }

    return false;
}

// Loops over the search paths to resolve \p canonical_file_path.
std::string FMDLEntityResolver::consider_search_paths(
    std::string const& canonical_file_mask,
    bool               is_resource,
    char const* file_path,
    Uvtile_mode uvtile_mode)
{
    check(canonical_file_mask[0] == '/');

    std::string canonical_file_mask_os = convert_slashes_to_os_separators(canonical_file_mask);

    std::string resolved_filename = search_mdl_path(
        canonical_file_mask_os.c_str() + 1,
        uvtile_mode);

    // Make resolved filename absolute.
    if (!resolved_filename.empty() && !is_path_absolute(resolved_filename.c_str()))
    {
        if (resolved_filename.size() >= 2)
        {
            if (resolved_filename[0] == '.' && resolved_filename[1] == os_separator())
            {
                resolved_filename = resolved_filename.substr(2);
            }
        }
        resolved_filename = join_path(get_cwd(), resolved_filename);
        resolved_filename = simplify_path(resolved_filename, os_separator());
    }

    return resolved_filename;
}

// Search the given path in all MDL search paths and return the complete path if found
std::string FMDLEntityResolver::search_mdl_path(
    char const* file_mask,
    Uvtile_mode uvtile_mode)
{
    // the archive name ('.' separators)
    std::string archive_path = to_archive(file_mask);

    // names inside archive using only '/' as a separator
    std::string file_name_fs = convert_os_separators_to_slashes(file_mask);

    for (auto search_path : SearchPaths)
    {
        char const* path = search_path.c_str();
        // no archives
        if (file_exists(path, file_mask))
        {
            std::string joined_file_name = join_path(path, file_mask);
            return convert_slashes_to_os_separators(joined_file_name);
        }
    }

    return std::string();
}

// Convert an url (separator '/') into an archive name (separator '.')
std::string FMDLEntityResolver::to_archive(
    char const* input_url) const
{
    std::string input_name;
    for (;;)
    {
        char const* p = strchr(input_url, '/');

#ifdef MI_PLATFORM_WINDOWS
        char const* q = strchr(input_url, '\\');

        if (q != nullptr)
        {
            if (p == nullptr || q < p)
            {
                p = q;
            }
        }
#endif

        if (p == nullptr) {
            input_name.append(input_url);
            return input_name;
        }
        input_name.append(input_url, p - input_url);
        input_name += '.';
        input_url = p + 1;
    }
}

// Check if an archive name is a prefix of a archive path.
bool FMDLEntityResolver::is_archive_prefix(
    std::string const& archive_name,
    std::string const& archive_path) const
{
    char const* prefix = archive_name.c_str();
    size_t l = archive_name.length() - 4; // strip ".mdr"

    if (l >= archive_path.size())
    {
        return false;
    }

    if (archive_path[l] != '.')
    {
        // fast check: prefix must be followed by '.'
        return false;
    }

    return strncmp(prefix, archive_path.c_str(), l) == 0;
}

void FMDLEntityResolver::AddModulePath(const FString& Path, bool IgnoreCheck)
{
    if (FPaths::DirectoryExists(Path) || IgnoreCheck)
    {
        SearchPaths.Add(TCHAR_TO_UTF8(*Path));
    }
}


void FMDLEntityResolver::RemoveModulePath(const FString& Path)
{
    SearchPaths.Remove(TCHAR_TO_UTF8(*Path));
}


#endif
