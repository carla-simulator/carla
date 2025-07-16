// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once


#if WITH_MDL_SDK

#include "MDLDependencies.h"
#include "MDLExternalReader.h"

#include <string>
using namespace mi::neuraylib;

/// An entry inside the resource file set;
struct FResourceEntry {
    /// Constructor.
    FResourceEntry(
        std::string const& url,
        std::string const& filename,
        int               u,
        int               v)
        : Url(url), FileName(filename), U(u), V(v)
    {
    }

    std::string Url;
    std::string FileName;
    int         U;
    int         V;
};

class FMDLResourceElement : public mi::base::Interface_implement<IMdl_resolved_resource_element>
{
    typedef mi::base::Interface_implement<IMdl_resolved_resource_element> Base;

public:
    /// Returns the frame number of this element.
    ///
    /// \return   Always 0 for textures without frame index, and for light profiles and BSDF
    ///           measurements.
    virtual mi::Size get_frame_number() const;

    /// Returns the number of resources for this element.
    virtual mi::Size get_count() const;

    /// Returns the MDL file path of the \p i -th resource, or \c NULL if the index is out of range.
    ///
    /// \see #mi::neuraylib::IMdl_resolved_resource::get_mdl_file_path_mask()
    virtual const char* get_mdl_file_path(mi::Size i) const;

    /// Returns the absolute resolved filename of the \p i -th resource, or \c NULL if the index
    /// is out of range.
    ///
    /// \note If this resource is located inside a container (an MDL archive or MDLE), the returned
    ///       string is a concatenation of the container filename, a colon, and the container member
    ///       name.
    ///
    /// \see #mi::neuraylib::IMdl_resolved_resource::get_filename_mask()
    virtual const char* get_filename(mi::Size i) const;

    /// Returns a reader for the \p i -th resource (or \c NULL if the index is out of range).
    ///
    /// The reader needs to support absolute access.
    virtual IReader* create_reader(mi::Size i) const;

    /// Returns the resource hash value for the \p i -th resource.
    ///
    /// \return The hash value of the \p i -th resource, or a zero-initialized value if the hash
    ///         value is unknown or the index is out of range.
    virtual mi::base::Uuid get_resource_hash(mi::Size i) const;

    /// Returns the u and v tile indices for the \p i -th resource.
    ///
    /// \param[in]  i  The index of the resource.
    /// \param[out] u  The u coordinate of the resource.
    /// \param[out] v  The v coordinate of the resource.
    /// \return        \c true if the uvtile mode is not #mi::neuraylib::UVTILE_MODE_NONE and \p i is
    ///                in range, \c false otherwise (and the output values are undefined).
    virtual bool get_uvtile_uv(mi::Size i, mi::Sint32& u, mi::Sint32& v) const;

public:

    FMDLResourceElement(char const* url, char const* filename);

    ~FMDLResourceElement();

private:
    /// The url mask.
    std::string UrlMask;

    /// The filename mask.
    std::string FileNameMask;
};

class FMDLResourceSet : public mi::base::Interface_implement<IMdl_resolved_resource>
{
    typedef mi::base::Interface_implement<IMdl_resolved_resource> Base;

public:
    /// Indicates whether this resource has a sequence marker.
    ///
    /// The return value \c false implies that there is a single frame with frame number 0.
    virtual bool has_sequence_marker() const;

    /// Returns the uvtile mode for this resource.
    ///
    /// The return value \c false implies that there is a single uv-tile (per frame) with u- and v-
    /// coordinates of 0.
    ///
    /// \return   Always #mi::neuraylib::UVTILE_MODE_NONE for light profiles and BSDF measurements.
    virtual Uvtile_mode get_uvtile_mode() const;

    /// Returns the MDL file path mask for this resource.
    ///
    /// The MDL file path mask is identical to the MDL file path, except that it contains the uvtile
    /// marker if the uvtile mode is not #mi::neuraylib::UVTILE_MODE_NONE.
    ///
    /// \see #mi::neuraylib::IMdl_resolved_resource_element::get_mdl_file_path(),
    ///      #get_uvtile_mode()
    virtual const char* get_mdl_file_path_mask() const;

    /// Returns the absolute resolved filename mask for this resource.
    ///
    /// The filename mask is identical to the filename, except that it contains the uvtile marker if
    /// the uvtile mode is not #mi::neuraylib::UVTILE_MODE_NONE.
    ///
    /// \note If this resource is located inside a container (an MDL archive or MDLE), the returned
    ///       string is a concatenation of the container filename, a colon, and the container member
    ///       name.
    ///
    /// \see #mi::neuraylib::IMdl_resolved_resource_element::get_filename(),
    ///      #get_uvtile_mode()
    virtual const char* get_filename_mask() const;

    /// Returns the number of elements of the resolved resource.
    virtual mi::Size get_count() const;

    /// Returns the \p i -th element of the resolved resource, or \c NULL if \p i is out of bounds.
    ///
    /// Resource elements are sorted by increasing frame numbers.
    virtual const IMdl_resolved_resource_element* get_element(mi::Size i) const;

public:
    /// Constructor from one file name/url pair (typical case).
    ///
    /// \param url       the absolute MDL url
    /// \param filename  the file name
    FMDLResourceSet(
        char const* url,
        char const* filename);

    ~FMDLResourceSet();

private:

    /// The Uvtile mode.
    Uvtile_mode UvtileMode;

    /// The url mask.
    std::string UrlMask;

    /// The filename mask.
    std::string FileNameMask;

    mi::base::Handle<FMDLResourceElement> ResourceElement;
};

/// An entity resolver.
class FMDLImportResult : public mi::base::Interface_implement<IMdl_resolved_module>
{
    typedef mi::base::Interface_implement<IMdl_resolved_module> Base;
public:
    /// Returns the fully qualified MDL name of the module.
    virtual char const* get_module_name() const;

    /// Returns the absolute resolved filename of the module.
    virtual char const* get_filename() const;

    /// Return an input stream to the given entity if found, NULL otherwise.
    virtual IReader* create_reader() const;

    /// Constructor.
    ///
    /// \param abs_name      the absolute name of the resolved import
    /// \param os_file_name  the OS dependent file name of the resolved module.
    FMDLImportResult(
        TSharedPtr<IMDLExternalReader> InFileReader,
        std::string const& abs_name,
        std::string const& os_file_name);

    ~FMDLImportResult();

private:
    /// External Reader
    TSharedPtr<IMDLExternalReader> FileReader;
    /// The absolute MDL name of the import.
    std::string const AbsoluteName;
    /// The OS-dependent file name of the resolved module.
    std::string const FileName;
};

/// An entity resolver.
class FMDLEntityResolver : public mi::base::Interface_implement<IMdl_entity_resolver>
{
    typedef mi::base::Interface_implement<IMdl_entity_resolver> Base;

public:
    /// Resolves a resource file path.
    ///
    /// If \p owner_name and \p owner_file_path are not provided, no relative paths can be resolved.
    /// The method can also be used to resolve files in an MDLE, e.g, to get a resource set for 
    /// embedded UDIM textures. 
    ///
    /// \param file_path         The MDL file path of the resource to resolve. In addition, for
    ///                          resources from MDLE files, it is also possible to provide the
    ///                          absolute OS file system path to the MDLE file (with slashes instead
    ///                          of backslashes on Windows), followed by a colon, followed by the
    ///                          relative path inside the MDLE container.
    /// \param owner_file_path   The optional file path of the owner (or \c NULL if not available).
    /// \param owner_name        The absolute name of the owner (or \c NULL if not available).
    /// \param pos_line          The line of the corresponding source code location (or 0 if not
    ///                          available).
    /// \param pos_column        The column of the corresponding source code location (or 0 if not
    ///                          available).
    /// \param context           The execution context which can be used to retrieve messages.
    /// \return                  A description of the resolved resource, or \c NULL in case of
    ///                          errors.
    virtual IMdl_resolved_resource* resolve_resource(
        const char* file_path,
        const char* owner_file_path,
        const char* owner_name,
        mi::Sint32 pos_line,
        mi::Sint32 pos_column,
        IMdl_execution_context* context = 0);


    /// Resolves a module name.
    ///
    /// If \p owner_name and \p owner_file_path are not provided, no relative module names can be
    /// resolved.
    ///
    /// \param module_name       The relative or absolute MDL module name to resolve.
    /// \param owner_file_path   The optional file path of the owner (or \c NULL if not available).
    /// \param owner_name        The absolute name of the owner (or \c NULL if not available).
    /// \param pos_line          The line of the corresponding source code location (or 0 if not
    ///                          available).
    /// \param pos_column        The column of the corresponding source code location (or 0 if not
    ///                          available).
    /// \param context           The execution context which can be used to retrieve messages.
    /// \return                  A description of the resolved module, or \c NULL in case of
    ///  
    virtual IMdl_resolved_module* resolve_module(
        const char* module_name,
        const char* owner_file_path,
        const char* owner_name,
        mi::Sint32 pos_line,
        mi::Sint32 pos_column,
        IMdl_execution_context* context = 0);

private:

    /// Resolve a MDL file name
    ///
    /// \param[out] abs_file_name    the resolved absolute file name (on file system)
    /// \param[in]  file_path        the MDL file path to resolve
    /// \param[in]  is_resource      true if file_path names a resource
    /// \param[in]  owner_name       the absolute name of the owner
    /// \param[in]  owner_file_path  the file path of the owner
    /// \param[in]  pos              the position of the import statement
    /// \param[out] udim_mode        if != NO_UDIM the returned absolute file name is a file mask
    std::string resolve_filename(
        std::string& abs_file_name,
        char const* file_path,
        bool                    is_resource,
        char const* module_file_system_path,
        char const* module_name,
        Uvtile_mode& uvtile_mode,
        mi::Sint32 pos_line,
        mi::Sint32 pos_column);


    /// Splits a file path into a directory path and file name.
    void split_file_path(
        std::string const& input_url,
        std::string& directory_path,
        std::string& file_path);

    /// Splits a module file system path into current working directory, current search path, and
    /// current module path.
    ///
    /// See MDL spec, section 2.2 for details.
    ///
    /// \param module_file_system_path          The module file system path to split (OS).
    /// \param module_name                      The fully-qualified MDL module name (only used for
    ///                                         string-based modules).
    /// \param module_nesting_level             The nesting level of the importing MDL module.
    /// \param[out] current_working_directory   The computed current working directory (OS).
    /// \param[out] cwd_is_container            True, if the \c current_working_directory names a
    ///                                         container.
    /// \param[out] current_search_path         The computed search path (OS).
    /// \param[out] csp_is_container            True, if the \c current_search_path names a
    ///                                         container.
    /// \param[out] current_module_path         The computed module path (SLASH).
    ///                                         Either empty or begins with a slash.
    void split_module_file_system_path(
        std::string const& module_file_system_path,
        std::string const& module_name,
        size_t            module_nesting_level,
        std::string& current_working_directory,
        bool& cwd_is_container,
        std::string& current_search_path,
        bool& csp_is_container,
        std::string& current_module_path);

    /// Converts OS-specific directory separators into slashes.
    ///
    /// \param s  the string to convert
    static std::string convert_os_separators_to_slashes(std::string const& s);

    /// Returns the nesting level of a module, i.e., the number of "::" substrings in the
    /// fully-qualified module name minus 1.
    static size_t get_module_nesting_level(char const* module_name);

    /// Checks that \p file_path contains no "." or ".." directory names.
    ///
    /// \param file_path  The file path to be checked (SLASH).
    ///
    /// \return           \c true in case of success, \c false otherwise.
    bool check_no_dots(
        char const* file_path);

    /// Checks that \p file_path contains at most one leading "." directory name, at most
    /// \p nesting_level leading ".." directory names, and no such non-leading directory names.
    ///
    /// \param file_path      The file path to be checked (SLASH).
    /// \param nesting_level  The nesting level of the importing MDL module (\see check_no_dots()).
    ///
    /// \return               \c true in case of success, \c false otherwise.
    bool check_no_dots_strict_relative(
        char const* file_path,
        size_t     nesting_level);

    /// Convert a name which might be either an url (separator '/') or an module name
    /// (separator '::') into an url.
    std::string to_url(
        char const* input_name) const;

    /// Convert an url (separator '/') into  a module name (separator '::')
    std::string to_module_name(
        char const* input_url) const;

    /// Normalizes a file path given by its directory path and file name.
    ///
    /// See MDL spec, section 2.2 for details.
    ///
    /// \param file_path                   The file path to normalize (SLASH).
    /// \param file_mask                   The file mask to normalize (SLASH).
    /// \param directory_path              The directory path split from the file path (SLASH).
    /// \param file_name                   The file name split from the file path (SLASH).
    /// \param nesting_level               The nesting level of the importing MDL module.
    /// \param module_file_system_path     The module file system path (OS).
    /// \param current_working_directory   The current working directory split from the module file
    ///                                    system path (OS).
    /// \param cwd_is_container            True, if the \c current_working_directory is a container.
    /// \param current_module_path         The current module path split from the module file system
    ///                                    path (SLASH).
    /// \return                            The normalized (canonical) file path, or \c "" in case of
    ///                                    failure.
    std::string normalize_file_path(
        std::string const& file_path,
        std::string const& file_mask,
        std::string const& directory_path,
        std::string const& file_name,
        size_t            nesting_level,
        std::string const& module_file_system_path,
        std::string const& current_working_directory,
        bool              cwd_is_container,
        std::string const& current_module_path);

    /// Check if the given file name (UTF8 encoded) names a file on the file system or inside
    /// an archive.
    ///
    /// \param fname  a file name (might contain a regex)
    bool file_exists(
        char const* fname, char const* fmask = nullptr) const;

    /// Loops over the search paths to resolve \p canonical_file_path.
    ///
    /// \param canonical_file_mask  The canonical file path (maybe regex) to resolve (SLASH).
    /// \param is_resource          True if the file path describes a resource.
    /// \param file_path            Used to report the filepath of error messages
    /// \param udim_mode            If != NO_UDIM the returned file path is a mask.
    ///
    /// \return                     The resolved file path, or \c "" in case of failures.
    std::string consider_search_paths(
        std::string const& canonical_file_mask,
        bool               is_resource,
        char const* file_path,
        Uvtile_mode uvtile_mode);

    /// Search the given path in all MDL search paths and return the absolute path if found.
    ///
    /// \param file_mask    the path to search (maybe a regex)
    /// \param udim_mode    if != NO_UDIM the returned value is a file mask
    ///
    /// \return the absolute path or mask if found
    std::string search_mdl_path(
        char const* file_mask,
        Uvtile_mode uvtile_mode);

    /// Checks whether the resolved file system location passes the consistency checks in the
    /// MDL spec.
    ///
    /// See MDL spec, section 2.2 for details.
    ///
    /// \param resolved_file_system_location  The resolved file system location to be checked (OS).
    /// \param canonical_file_path            The canonical file path (SLASH).
    /// \param file_path                      The (original) file path (to select the type of check
    ///                                       and error messages, SLASH).
    /// \param current_working_directory      The current working directory (OS).
    /// \param current_search_path            The current search path (OS).
    /// \param is_resource                    True if the location belongs to a resource.
    /// \param csp_is_archive                 True if \c current_search_path names an archive.
    /// \return                               \c true in case of success, \c false otherwise.
    bool check_consistency(
        std::string const& resolved_file_system_location,
        std::string const& canonical_file_path,
        std::string const& file_path,
        std::string const& current_working_directory,
        std::string const& current_search_path,
        bool              is_resource,
        bool              csp_is_archive);

    /// Convert an url (separator '/') into an archive name (separator '.').
    std::string to_archive(
        char const* file_name) const;

    /// Check if an archive name is a prefix of a archive path.
    bool is_archive_prefix(
        std::string const& archive_name,
        std::string const& archive_path) const;


public:
    /// Constructor.
    FMDLEntityResolver()
        : MdlFileReader(nullptr)
    {
    }

    ~FMDLEntityResolver()
    {
        MdlFileReader = nullptr;
    }

    void AddModulePath(const FString& Path, bool IgnoreCheck = false);
    void RemoveModulePath(const FString& Path);
    void SetExternalFileReader(TSharedPtr<IMDLExternalReader> FileReader)
    {
        MdlFileReader = FileReader;
    }

private:
    TArray<std::string> SearchPaths;
    TSharedPtr<IMDLExternalReader> MdlFileReader;
};

#endif
