/* Generated code for Python module 'pip._vendor.html5lib.constants'
 * created by Nuitka version 0.6.16.2
 *
 * This code is in part copyright 2021 Kay Hayen.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nuitka/prelude.h"

#include "nuitka/unfreezing.h"

#include "__helpers.h"

/* The "module_pip$_vendor$html5lib$constants" is a Python object pointer of module type.
 *
 * Note: For full compatibility with CPython, every module variable access
 * needs to go through it except for cases where the module cannot possibly
 * have changed in the mean time.
 */

PyObject *module_pip$_vendor$html5lib$constants;
PyDictObject *moduledict_pip$_vendor$html5lib$constants;

/* The declarations of module constants used, if any. */
static PyObject *mod_consts[216];

static PyObject *module_filename_obj = NULL;

/* Indicator if this modules private constants were created yet. */
static bool constants_created = false;

/* Function to create module private constants. */
static void createModuleConstants(void) {
    if (constants_created == false) {
        loadConstantsBlob(&mod_consts[0], UNTRANSLATE("pip._vendor.html5lib.constants"));
        constants_created = true;
    }
}

/* For multiprocessing, we want to be able to initialize the __main__ constants. */
#if (_NUITKA_PLUGIN_MULTIPROCESSING_ENABLED || _NUITKA_PLUGIN_TRACEBACK_ENCRYPTION_ENABLED) && 0
void createMainModuleConstants(void) {
    createModuleConstants();
}
#endif

/* Function to verify module private constants for non-corruption. */
#ifndef __NUITKA_NO_ASSERT__
void checkModuleConstants_pip$_vendor$html5lib$constants(void) {
    // The module may not have been used at all, then ignore this.
    if (constants_created == false) return;

    checkConstantsBlob(&mod_consts[0], "pip._vendor.html5lib.constants");
}
#endif

// The module code objects.
static PyCodeObject *codeobj_26c5d084b8dc3930bde29c9decd1859d;
static PyCodeObject *codeobj_d3bae8f8a8228db9f43b8f6e0fa66d2f;
static PyCodeObject *codeobj_ca4c1f39d7300194360d6f48084cfe9e;
static PyCodeObject *codeobj_ab5cd1d9cd36c1f152d39a0bfa133d17;

static void createModuleCodeObjects(void) {
    module_filename_obj = mod_consts[1]; CHECK_OBJECT(module_filename_obj);
    codeobj_26c5d084b8dc3930bde29c9decd1859d = MAKE_CODEOBJECT(module_filename_obj, 547, CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[211], mod_consts[212], NULL, 1, 0, 0);
    codeobj_d3bae8f8a8228db9f43b8f6e0fa66d2f = MAKE_CODEOBJECT(module_filename_obj, 2936, CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[211], mod_consts[213], NULL, 1, 0, 0);
    codeobj_ca4c1f39d7300194360d6f48084cfe9e = MAKE_CODEOBJECT(module_filename_obj, 522, CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[211], mod_consts[214], NULL, 1, 0, 0);
    codeobj_ab5cd1d9cd36c1f152d39a0bfa133d17 = MAKE_CODEOBJECT(module_filename_obj, 1, CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[215], NULL, NULL, 0, 0, 0);
}

// The module function declarations.


// The module function definitions.


extern void _initCompiledCellType();
extern void _initCompiledGeneratorType();
extern void _initCompiledFunctionType();
extern void _initCompiledMethodType();
extern void _initCompiledFrameType();

extern PyTypeObject Nuitka_Loader_Type;

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
// Provide a way to create find a function via its C code and create it back
// in another process, useful for multiprocessing extensions like dill
extern void registerDillPluginTables(char const *module_name, PyMethodDef *reduce_compiled_function, PyMethodDef *create_compiled_function);

function_impl_code functable_pip$_vendor$html5lib$constants[] = {

    NULL
};

static char const *_reduce_compiled_function_argnames[] = {
    "func",
    NULL
};

static PyObject *_reduce_compiled_function(PyObject *self, PyObject *args, PyObject *kwds) {
    PyObject *func;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O:reduce_compiled_function", (char **)_reduce_compiled_function_argnames, &func, NULL)) {
        return NULL;
    }

    if (Nuitka_Function_Check(func) == false) {
        SET_CURRENT_EXCEPTION_TYPE0_STR(PyExc_TypeError, "not a compiled function");
        return NULL;
    }

    struct Nuitka_FunctionObject *function = (struct Nuitka_FunctionObject *)func;

    function_impl_code *current = functable_pip$_vendor$html5lib$constants;
    int offset = 0;

    while (*current != NULL) {
        if (*current == function->m_c_code) {
            break;
        }

        current += 1;
        offset += 1;
    }

    if (*current == NULL) {
        SET_CURRENT_EXCEPTION_TYPE0_STR(PyExc_TypeError, "Cannot find compiled function in module.");
        return NULL;
    }

    PyObject *code_object_desc = PyTuple_New(6);
    PyTuple_SET_ITEM0(code_object_desc, 0, function->m_code_object->co_filename);
    PyTuple_SET_ITEM0(code_object_desc, 1, function->m_code_object->co_name);
    PyTuple_SET_ITEM(code_object_desc, 2, PyLong_FromLong(function->m_code_object->co_firstlineno));
    PyTuple_SET_ITEM0(code_object_desc, 3, function->m_code_object->co_varnames);
    PyTuple_SET_ITEM(code_object_desc, 4, PyLong_FromLong(function->m_code_object->co_argcount));
    PyTuple_SET_ITEM(code_object_desc, 5, PyLong_FromLong(function->m_code_object->co_flags));

    CHECK_OBJECT_DEEP(code_object_desc);

    PyObject *result = PyTuple_New(4);
    PyTuple_SET_ITEM(result, 0, PyLong_FromLong(offset));
    PyTuple_SET_ITEM(result, 1, code_object_desc);
    PyTuple_SET_ITEM0(result, 2, function->m_defaults);
    PyTuple_SET_ITEM0(result, 3, function->m_doc != NULL ? function->m_doc : Py_None);

    CHECK_OBJECT_DEEP(result);

    return result;
}

static PyMethodDef _method_def_reduce_compiled_function = {"reduce_compiled_function", (PyCFunction)_reduce_compiled_function,
                                                           METH_VARARGS | METH_KEYWORDS, NULL};

static char const *_create_compiled_function_argnames[] = {
    "func",
    "code_object_desc",
    "defaults",
    "doc",
    NULL
};


static PyObject *_create_compiled_function(PyObject *self, PyObject *args, PyObject *kwds) {
    CHECK_OBJECT_DEEP(args);

    PyObject *func;
    PyObject *code_object_desc;
    PyObject *defaults;
    PyObject *doc;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO:create_compiled_function", (char **)_create_compiled_function_argnames, &func, &code_object_desc, &defaults, &doc, NULL)) {
        return NULL;
    }

    int offset = PyLong_AsLong(func);

    if (offset == -1 && ERROR_OCCURRED()) {
        return NULL;
    }

    if (offset > sizeof(functable_pip$_vendor$html5lib$constants) || offset < 0) {
        SET_CURRENT_EXCEPTION_TYPE0_STR(PyExc_TypeError, "Wrong offset for compiled function.");
        return NULL;
    }

    PyObject *filename = PyTuple_GET_ITEM(code_object_desc, 0);
    PyObject *function_name = PyTuple_GET_ITEM(code_object_desc, 1);
    PyObject *line = PyTuple_GET_ITEM(code_object_desc, 2);
    int line_int = PyLong_AsLong(line);
    assert(!ERROR_OCCURRED());

    PyObject *argnames = PyTuple_GET_ITEM(code_object_desc, 3);
    PyObject *arg_count = PyTuple_GET_ITEM(code_object_desc, 4);
    int arg_count_int = PyLong_AsLong(arg_count);
    assert(!ERROR_OCCURRED());
    PyObject *flags = PyTuple_GET_ITEM(code_object_desc, 5);
    int flags_int = PyLong_AsLong(flags);
    assert(!ERROR_OCCURRED());

    PyCodeObject *code_object = MAKE_CODEOBJECT(
        filename,
        line_int,
        flags_int,
        function_name,
        argnames,
        NULL, // freevars
        arg_count_int,
        0, // TODO: Missing kw_only_count
        0 // TODO: Missing pos_only_count
    );

    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        functable_pip$_vendor$html5lib$constants[offset],
        code_object->co_name,
#if PYTHON_VERSION >= 0x300
        NULL, // TODO: Not transferring qualname yet
#endif
        code_object,
        defaults,
#if PYTHON_VERSION >= 0x300
        NULL, // kwdefaults are done on the outside currently
        NULL, // TODO: Not transferring annotations
#endif
        module_pip$_vendor$html5lib$constants,
        doc,
        NULL,
        0
    );

    return (PyObject *)result;
}

static PyMethodDef _method_def_create_compiled_function = {
    "create_compiled_function",
    (PyCFunction)_create_compiled_function,
    METH_VARARGS | METH_KEYWORDS, NULL
};


#endif

// Internal entry point for module code.
PyObject *modulecode_pip$_vendor$html5lib$constants(PyObject *module, struct Nuitka_MetaPathBasedLoaderEntry const *module_entry) {
    module_pip$_vendor$html5lib$constants = module;

#ifdef _NUITKA_MODULE
    // In case of a stand alone extension module, need to call initialization
    // the init here because that's the first and only time we are going to get
    // called here.

    // Initialize the constant values used.
    _initBuiltinModule();
    createGlobalConstants();

    /* Initialize the compiled types of Nuitka. */
    _initCompiledCellType();
    _initCompiledGeneratorType();
    _initCompiledFunctionType();
    _initCompiledMethodType();
    _initCompiledFrameType();

#if PYTHON_VERSION < 0x300
    _initSlotCompare();
#endif
#if PYTHON_VERSION >= 0x270
    _initSlotIternext();
#endif

    patchBuiltinModule();
    patchTypeComparison();

    // Enable meta path based loader if not already done.
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.html5lib.constants: Calling setupMetaPathBasedLoader().\n");
#endif
    setupMetaPathBasedLoader();

#if PYTHON_VERSION >= 0x300
    patchInspectModule();
#endif

#endif

    /* The constants only used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.html5lib.constants: Calling createModuleConstants().\n");
#endif
    createModuleConstants();

    /* The code objects used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.html5lib.constants: Calling createModuleCodeObjects().\n");
#endif
    createModuleCodeObjects();

    // PRINT_STRING("in initpip$_vendor$html5lib$constants\n");

    // Create the module object first. There are no methods initially, all are
    // added dynamically in actual code only.  Also no "__doc__" is initially
    // set at this time, as it could not contain NUL characters this way, they
    // are instead set in early module code.  No "self" for modules, we have no
    // use for it.

    moduledict_pip$_vendor$html5lib$constants = MODULE_DICT(module_pip$_vendor$html5lib$constants);

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
    registerDillPluginTables(module_entry->name, &_method_def_reduce_compiled_function, &_method_def_create_compiled_function);
#endif

    // Set "__compiled__" to what version information we have.
    UPDATE_STRING_DICT0(
        moduledict_pip$_vendor$html5lib$constants,
        (Nuitka_StringObject *)const_str_plain___compiled__,
        Nuitka_dunder_compiled_value
    );

    // Update "__package__" value to what it ought to be.
    {
#if 0
        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$html5lib$constants,
            (Nuitka_StringObject *)const_str_plain___package__,
            const_str_empty
        );
#elif 0
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___name__);

        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$html5lib$constants,
            (Nuitka_StringObject *)const_str_plain___package__,
            module_name
        );
#else

#if PYTHON_VERSION < 0x300
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___name__);
        char const *module_name_cstr = PyString_AS_STRING(module_name);

        char const *last_dot = strrchr(module_name_cstr, '.');

        if (last_dot != NULL) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$html5lib$constants,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyString_FromStringAndSize(module_name_cstr, last_dot - module_name_cstr)
            );
        }
#else
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___name__);
        Py_ssize_t dot_index = PyUnicode_Find(module_name, const_str_dot, 0, PyUnicode_GetLength(module_name), -1);

        if (dot_index != -1) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$html5lib$constants,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyUnicode_Substring(module_name, 0, dot_index)
            );
        }
#endif
#endif
    }

    CHECK_OBJECT(module_pip$_vendor$html5lib$constants);

    // For deep importing of a module we need to have "__builtins__", so we set
    // it ourselves in the same way than CPython does. Note: This must be done
    // before the frame object is allocated, or else it may fail.

    if (GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___builtins__) == NULL) {
        PyObject *value = (PyObject *)builtin_module;

        // Check if main module, not a dict then but the module itself.
#if !defined(_NUITKA_EXE) || !0
        value = PyModule_GetDict(value);
#endif

        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___builtins__, value);
    }

#if PYTHON_VERSION >= 0x300
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___loader__, (PyObject *)&Nuitka_Loader_Type);
#endif

#if PYTHON_VERSION >= 0x340
// Set the "__spec__" value

#if 0
    // Main modules just get "None" as spec.
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___spec__, Py_None);
#else
    // Other modules get a "ModuleSpec" from the standard mechanism.
    {
        PyObject *bootstrap_module = getImportLibBootstrapModule();
        CHECK_OBJECT(bootstrap_module);

        PyObject *_spec_from_module = PyObject_GetAttrString(bootstrap_module, "_spec_from_module");
        CHECK_OBJECT(_spec_from_module);

        PyObject *spec_value = CALL_FUNCTION_WITH_SINGLE_ARG(_spec_from_module, module_pip$_vendor$html5lib$constants);
        Py_DECREF(_spec_from_module);

        // We can assume this to never fail, or else we are in trouble anyway.
        // CHECK_OBJECT(spec_value);

        if (spec_value == NULL) {
            PyErr_PrintEx(0);
            abort();
        }

// Mark the execution in the "__spec__" value.
        SET_ATTRIBUTE(spec_value, const_str_plain__initializing, Py_True);

        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)const_str_plain___spec__, spec_value);
    }
#endif
#endif

    // Temp variables if any
    PyObject *outline_0_var_qname = NULL;
    PyObject *outline_0_var_prefix = NULL;
    PyObject *outline_0_var_local = NULL;
    PyObject *outline_0_var_ns = NULL;
    PyObject *outline_1_var_c = NULL;
    PyObject *outline_2_var_k = NULL;
    PyObject *outline_2_var_v = NULL;
    PyObject *outline_3_var___class__ = NULL;
    PyObject *outline_4_var___class__ = NULL;
    PyObject *tmp_class_creation_1__bases = NULL;
    PyObject *tmp_class_creation_1__class_decl_dict = NULL;
    PyObject *tmp_class_creation_1__metaclass = NULL;
    PyObject *tmp_class_creation_1__prepared = NULL;
    PyObject *tmp_class_creation_2__bases = NULL;
    PyObject *tmp_class_creation_2__class_decl_dict = NULL;
    PyObject *tmp_class_creation_2__metaclass = NULL;
    PyObject *tmp_class_creation_2__prepared = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_1__element_1 = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_1__element_2 = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_1__source_iter = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_2__element_1 = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_2__element_2 = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_2__element_3 = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_2__source_iter = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_3__element_1 = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_3__element_2 = NULL;
    PyObject *tmp_dictcontraction$tuple_unpack_3__source_iter = NULL;
    PyObject *tmp_dictcontraction_1__$0 = NULL;
    PyObject *tmp_dictcontraction_1__contraction = NULL;
    PyObject *tmp_dictcontraction_1__iter_value_0 = NULL;
    PyObject *tmp_dictcontraction_2__$0 = NULL;
    PyObject *tmp_dictcontraction_2__contraction = NULL;
    PyObject *tmp_dictcontraction_2__iter_value_0 = NULL;
    PyObject *tmp_dictcontraction_3__$0 = NULL;
    PyObject *tmp_dictcontraction_3__contraction = NULL;
    PyObject *tmp_dictcontraction_3__iter_value_0 = NULL;
    PyObject *tmp_import_from_1__module = NULL;
    struct Nuitka_FrameObject *frame_ab5cd1d9cd36c1f152d39a0bfa133d17;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    bool tmp_result;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;
    int tmp_res;
    struct Nuitka_FrameObject *frame_ca4c1f39d7300194360d6f48084cfe9e_2;
    NUITKA_MAY_BE_UNUSED char const *type_description_2 = NULL;
    PyObject *tmp_iterator_attempt;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_2;
    PyObject *exception_keeper_type_3;
    PyObject *exception_keeper_value_3;
    PyTracebackObject *exception_keeper_tb_3;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_3;
    PyObject *exception_keeper_type_4;
    PyObject *exception_keeper_value_4;
    PyTracebackObject *exception_keeper_tb_4;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_4;
    PyObject *exception_keeper_type_5;
    PyObject *exception_keeper_value_5;
    PyTracebackObject *exception_keeper_tb_5;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_5;
    PyObject *tmp_dictset_value;
    PyObject *tmp_dictset_dict;
    PyObject *tmp_dictset_key;
    PyObject *exception_keeper_type_6;
    PyObject *exception_keeper_value_6;
    PyTracebackObject *exception_keeper_tb_6;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_6;
    static struct Nuitka_FrameObject *cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2 = NULL;
    PyObject *exception_keeper_type_7;
    PyObject *exception_keeper_value_7;
    PyTracebackObject *exception_keeper_tb_7;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_7;
    struct Nuitka_FrameObject *frame_26c5d084b8dc3930bde29c9decd1859d_3;
    NUITKA_MAY_BE_UNUSED char const *type_description_3 = NULL;
    PyObject *exception_keeper_type_8;
    PyObject *exception_keeper_value_8;
    PyTracebackObject *exception_keeper_tb_8;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_8;
    static struct Nuitka_FrameObject *cache_frame_26c5d084b8dc3930bde29c9decd1859d_3 = NULL;
    PyObject *exception_keeper_type_9;
    PyObject *exception_keeper_value_9;
    PyTracebackObject *exception_keeper_tb_9;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_9;
    struct Nuitka_FrameObject *frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4;
    NUITKA_MAY_BE_UNUSED char const *type_description_4 = NULL;
    PyObject *exception_keeper_type_10;
    PyObject *exception_keeper_value_10;
    PyTracebackObject *exception_keeper_tb_10;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_10;
    PyObject *exception_keeper_type_11;
    PyObject *exception_keeper_value_11;
    PyTracebackObject *exception_keeper_tb_11;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_11;
    PyObject *exception_keeper_type_12;
    PyObject *exception_keeper_value_12;
    PyTracebackObject *exception_keeper_tb_12;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_12;
    static struct Nuitka_FrameObject *cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4 = NULL;
    PyObject *exception_keeper_type_13;
    PyObject *exception_keeper_value_13;
    PyTracebackObject *exception_keeper_tb_13;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_13;
    PyObject *tmp_dictdel_dict;
    PyObject *tmp_dictdel_key;
    PyObject *locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940 = NULL;
    PyObject *exception_keeper_type_14;
    PyObject *exception_keeper_value_14;
    PyTracebackObject *exception_keeper_tb_14;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_14;
    PyObject *exception_keeper_type_15;
    PyObject *exception_keeper_value_15;
    PyTracebackObject *exception_keeper_tb_15;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_15;
    PyObject *exception_keeper_type_16;
    PyObject *exception_keeper_value_16;
    PyTracebackObject *exception_keeper_tb_16;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_16;
    PyObject *locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945 = NULL;
    PyObject *exception_keeper_type_17;
    PyObject *exception_keeper_value_17;
    PyTracebackObject *exception_keeper_tb_17;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_17;
    PyObject *exception_keeper_type_18;
    PyObject *exception_keeper_value_18;
    PyTracebackObject *exception_keeper_tb_18;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_18;
    PyObject *exception_keeper_type_19;
    PyObject *exception_keeper_value_19;
    PyTracebackObject *exception_keeper_tb_19;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_19;

    // Module code.
    {
        PyObject *tmp_assign_source_1;
        tmp_assign_source_1 = Py_None;
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[0], tmp_assign_source_1);
    }
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = mod_consts[1];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[2], tmp_assign_source_2);
    }
    // Frame without reuse.
    frame_ab5cd1d9cd36c1f152d39a0bfa133d17 = MAKE_MODULE_FRAME(codeobj_ab5cd1d9cd36c1f152d39a0bfa133d17, module_pip$_vendor$html5lib$constants);

    // Push the new frame as the currently active one, and we should be exclusively
    // owning it.
    pushFrameStack(frame_ab5cd1d9cd36c1f152d39a0bfa133d17);
    assert(Py_REFCNT(frame_ab5cd1d9cd36c1f152d39a0bfa133d17) == 2);

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_assattr_name_1 = mod_consts[1];
        tmp_assattr_target_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[3]);

        if (unlikely(tmp_assattr_target_1 == NULL)) {
            tmp_assattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[3]);
        }

        assert(!(tmp_assattr_target_1 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[4], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_2;
        PyObject *tmp_assattr_target_2;
        tmp_assattr_name_2 = Py_True;
        tmp_assattr_target_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[3]);

        if (unlikely(tmp_assattr_target_2 == NULL)) {
            tmp_assattr_target_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[3]);
        }

        assert(!(tmp_assattr_target_2 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[5], tmp_assattr_name_2);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assign_source_3;
        tmp_assign_source_3 = Py_None;
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[6], tmp_assign_source_3);
    }
    {
        PyObject *tmp_assign_source_4;
        frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 1;
        tmp_assign_source_4 = IMPORT_HARD___FUTURE__();
        assert(!(tmp_assign_source_4 == NULL));
        assert(tmp_import_from_1__module == NULL);
        Py_INCREF(tmp_assign_source_4);
        tmp_import_from_1__module = tmp_assign_source_4;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_import_name_from_1;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_1 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_1)) {
            tmp_assign_source_5 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_1,
                (PyObject *)moduledict_pip$_vendor$html5lib$constants,
                mod_consts[7],
                mod_consts[8]
            );
        } else {
            tmp_assign_source_5 = IMPORT_NAME(tmp_import_name_from_1, mod_consts[7]);
        }

        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[7], tmp_assign_source_5);
    }
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_import_name_from_2;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_2 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_2)) {
            tmp_assign_source_6 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_2,
                (PyObject *)moduledict_pip$_vendor$html5lib$constants,
                mod_consts[9],
                mod_consts[8]
            );
        } else {
            tmp_assign_source_6 = IMPORT_NAME(tmp_import_name_from_2, mod_consts[9]);
        }

        if (tmp_assign_source_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[9], tmp_assign_source_6);
    }
    {
        PyObject *tmp_assign_source_7;
        PyObject *tmp_import_name_from_3;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_3 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_3)) {
            tmp_assign_source_7 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_3,
                (PyObject *)moduledict_pip$_vendor$html5lib$constants,
                mod_consts[10],
                mod_consts[8]
            );
        } else {
            tmp_assign_source_7 = IMPORT_NAME(tmp_import_name_from_3, mod_consts[10]);
        }

        if (tmp_assign_source_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[10], tmp_assign_source_7);
    }
    goto try_end_1;
    // Exception handler code:
    try_except_handler_1:;
    exception_keeper_type_1 = exception_type;
    exception_keeper_value_1 = exception_value;
    exception_keeper_tb_1 = exception_tb;
    exception_keeper_lineno_1 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    CHECK_OBJECT(tmp_import_from_1__module);
    Py_DECREF(tmp_import_from_1__module);
    tmp_import_from_1__module = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

    goto frame_exception_exit_1;
    // End of try:
    try_end_1:;
    CHECK_OBJECT(tmp_import_from_1__module);
    Py_DECREF(tmp_import_from_1__module);
    tmp_import_from_1__module = NULL;
    {
        PyObject *tmp_assign_source_8;
        PyObject *tmp_name_name_1;
        PyObject *tmp_globals_arg_name_1;
        PyObject *tmp_locals_arg_name_1;
        PyObject *tmp_fromlist_name_1;
        PyObject *tmp_level_name_1;
        tmp_name_name_1 = mod_consts[11];
        tmp_globals_arg_name_1 = (PyObject *)moduledict_pip$_vendor$html5lib$constants;
        tmp_locals_arg_name_1 = Py_None;
        tmp_fromlist_name_1 = Py_None;
        tmp_level_name_1 = mod_consts[8];
        frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 3;
        tmp_assign_source_8 = IMPORT_MODULE5(tmp_name_name_1, tmp_globals_arg_name_1, tmp_locals_arg_name_1, tmp_fromlist_name_1, tmp_level_name_1);
        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[11], tmp_assign_source_8);
    }
    {
        PyObject *tmp_assign_source_9;
        tmp_assign_source_9 = Py_None;
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[12], tmp_assign_source_9);
    }
    {
        PyObject *tmp_assign_source_10;
        tmp_assign_source_10 = PyDict_Copy(mod_consts[13]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[14], tmp_assign_source_10);
    }
    {
        PyObject *tmp_assign_source_11;
        tmp_assign_source_11 = PyDict_Copy(mod_consts[15]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16], tmp_assign_source_11);
    }
    {
        PyObject *tmp_assign_source_12;
        PyObject *tmp_frozenset_arg_1;
        PyObject *tmp_list_element_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_dict_arg_name_1;
        PyObject *tmp_key_name_1;
        tmp_dict_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

        if (unlikely(tmp_dict_arg_name_1 == NULL)) {
            tmp_dict_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
        }

        assert(!(tmp_dict_arg_name_1 == NULL));
        tmp_key_name_1 = mod_consts[17];
        tmp_tuple_element_1 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_1, tmp_key_name_1);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 306;

            goto frame_exception_exit_1;
        }
        tmp_list_element_1 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_1);
        tmp_tuple_element_1 = mod_consts[18];
        PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_1);
        tmp_frozenset_arg_1 = PyList_New(17);
        {
            PyObject *tmp_tuple_element_2;
            PyObject *tmp_dict_arg_name_2;
            PyObject *tmp_key_name_2;
            PyObject *tmp_tuple_element_3;
            PyObject *tmp_dict_arg_name_3;
            PyObject *tmp_key_name_3;
            PyObject *tmp_tuple_element_4;
            PyObject *tmp_dict_arg_name_4;
            PyObject *tmp_key_name_4;
            PyObject *tmp_tuple_element_5;
            PyObject *tmp_dict_arg_name_5;
            PyObject *tmp_key_name_5;
            PyObject *tmp_tuple_element_6;
            PyObject *tmp_dict_arg_name_6;
            PyObject *tmp_key_name_6;
            PyObject *tmp_tuple_element_7;
            PyObject *tmp_dict_arg_name_7;
            PyObject *tmp_key_name_7;
            PyObject *tmp_tuple_element_8;
            PyObject *tmp_dict_arg_name_8;
            PyObject *tmp_key_name_8;
            PyObject *tmp_tuple_element_9;
            PyObject *tmp_dict_arg_name_9;
            PyObject *tmp_key_name_9;
            PyObject *tmp_tuple_element_10;
            PyObject *tmp_dict_arg_name_10;
            PyObject *tmp_key_name_10;
            PyObject *tmp_tuple_element_11;
            PyObject *tmp_dict_arg_name_11;
            PyObject *tmp_key_name_11;
            PyObject *tmp_tuple_element_12;
            PyObject *tmp_dict_arg_name_12;
            PyObject *tmp_key_name_12;
            PyObject *tmp_tuple_element_13;
            PyObject *tmp_dict_arg_name_13;
            PyObject *tmp_key_name_13;
            PyObject *tmp_tuple_element_14;
            PyObject *tmp_dict_arg_name_14;
            PyObject *tmp_key_name_14;
            PyObject *tmp_tuple_element_15;
            PyObject *tmp_dict_arg_name_15;
            PyObject *tmp_key_name_15;
            PyObject *tmp_tuple_element_16;
            PyObject *tmp_dict_arg_name_16;
            PyObject *tmp_key_name_16;
            PyObject *tmp_tuple_element_17;
            PyObject *tmp_dict_arg_name_17;
            PyObject *tmp_key_name_17;
            PyList_SET_ITEM(tmp_frozenset_arg_1, 0, tmp_list_element_1);
            tmp_dict_arg_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_2 == NULL)) {
                tmp_dict_arg_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_2 == NULL));
            tmp_key_name_2 = mod_consts[17];
            tmp_tuple_element_2 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_2, tmp_key_name_2);
            if (tmp_tuple_element_2 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 307;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_2);
            tmp_tuple_element_2 = mod_consts[19];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_2);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 1, tmp_list_element_1);
            tmp_dict_arg_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_3 == NULL)) {
                tmp_dict_arg_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_3 == NULL));
            tmp_key_name_3 = mod_consts[17];
            tmp_tuple_element_3 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_3, tmp_key_name_3);
            if (tmp_tuple_element_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 308;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_3);
            tmp_tuple_element_3 = mod_consts[17];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_3);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 2, tmp_list_element_1);
            tmp_dict_arg_name_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_4 == NULL)) {
                tmp_dict_arg_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_4 == NULL));
            tmp_key_name_4 = mod_consts[17];
            tmp_tuple_element_4 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_4, tmp_key_name_4);
            if (tmp_tuple_element_4 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 309;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_4);
            tmp_tuple_element_4 = mod_consts[20];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_4);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 3, tmp_list_element_1);
            tmp_dict_arg_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_5 == NULL)) {
                tmp_dict_arg_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_5 == NULL));
            tmp_key_name_5 = mod_consts[17];
            tmp_tuple_element_5 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_5, tmp_key_name_5);
            if (tmp_tuple_element_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 310;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_5);
            tmp_tuple_element_5 = mod_consts[21];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_5);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 4, tmp_list_element_1);
            tmp_dict_arg_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_6 == NULL)) {
                tmp_dict_arg_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_6 == NULL));
            tmp_key_name_6 = mod_consts[17];
            tmp_tuple_element_6 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_6, tmp_key_name_6);
            if (tmp_tuple_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 311;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_6);
            tmp_tuple_element_6 = mod_consts[22];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_6);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 5, tmp_list_element_1);
            tmp_dict_arg_name_7 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_7 == NULL)) {
                tmp_dict_arg_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_7 == NULL));
            tmp_key_name_7 = mod_consts[17];
            tmp_tuple_element_7 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_7, tmp_key_name_7);
            if (tmp_tuple_element_7 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 312;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_7);
            tmp_tuple_element_7 = mod_consts[23];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_7);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 6, tmp_list_element_1);
            tmp_dict_arg_name_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_8 == NULL)) {
                tmp_dict_arg_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_8 == NULL));
            tmp_key_name_8 = mod_consts[17];
            tmp_tuple_element_8 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_8, tmp_key_name_8);
            if (tmp_tuple_element_8 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 313;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_8);
            tmp_tuple_element_8 = mod_consts[24];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_8);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 7, tmp_list_element_1);
            tmp_dict_arg_name_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_9 == NULL)) {
                tmp_dict_arg_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_9 == NULL));
            tmp_key_name_9 = mod_consts[25];
            tmp_tuple_element_9 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_9, tmp_key_name_9);
            if (tmp_tuple_element_9 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 314;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_9);
            tmp_tuple_element_9 = mod_consts[26];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_9);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 8, tmp_list_element_1);
            tmp_dict_arg_name_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_10 == NULL)) {
                tmp_dict_arg_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_10 == NULL));
            tmp_key_name_10 = mod_consts[25];
            tmp_tuple_element_10 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_10, tmp_key_name_10);
            if (tmp_tuple_element_10 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 315;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_10);
            tmp_tuple_element_10 = mod_consts[27];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_10);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 9, tmp_list_element_1);
            tmp_dict_arg_name_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_11 == NULL)) {
                tmp_dict_arg_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_11 == NULL));
            tmp_key_name_11 = mod_consts[25];
            tmp_tuple_element_11 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_11, tmp_key_name_11);
            if (tmp_tuple_element_11 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 316;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_11);
            tmp_tuple_element_11 = mod_consts[28];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_11);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 10, tmp_list_element_1);
            tmp_dict_arg_name_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_12 == NULL)) {
                tmp_dict_arg_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_12 == NULL));
            tmp_key_name_12 = mod_consts[25];
            tmp_tuple_element_12 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_12, tmp_key_name_12);
            if (tmp_tuple_element_12 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 317;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_12);
            tmp_tuple_element_12 = mod_consts[29];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_12);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 11, tmp_list_element_1);
            tmp_dict_arg_name_13 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_13 == NULL)) {
                tmp_dict_arg_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_13 == NULL));
            tmp_key_name_13 = mod_consts[25];
            tmp_tuple_element_13 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_13, tmp_key_name_13);
            if (tmp_tuple_element_13 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 318;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_13);
            tmp_tuple_element_13 = mod_consts[30];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_13);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 12, tmp_list_element_1);
            tmp_dict_arg_name_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_14 == NULL)) {
                tmp_dict_arg_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_14 == NULL));
            tmp_key_name_14 = mod_consts[25];
            tmp_tuple_element_14 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_14, tmp_key_name_14);
            if (tmp_tuple_element_14 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 319;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_14);
            tmp_tuple_element_14 = mod_consts[31];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_14);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 13, tmp_list_element_1);
            tmp_dict_arg_name_15 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_15 == NULL)) {
                tmp_dict_arg_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_15 == NULL));
            tmp_key_name_15 = mod_consts[32];
            tmp_tuple_element_15 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_15, tmp_key_name_15);
            if (tmp_tuple_element_15 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 320;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_15);
            tmp_tuple_element_15 = mod_consts[33];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_15);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 14, tmp_list_element_1);
            tmp_dict_arg_name_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_16 == NULL)) {
                tmp_dict_arg_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_16 == NULL));
            tmp_key_name_16 = mod_consts[32];
            tmp_tuple_element_16 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_16, tmp_key_name_16);
            if (tmp_tuple_element_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 321;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_16);
            tmp_tuple_element_16 = mod_consts[34];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_16);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 15, tmp_list_element_1);
            tmp_dict_arg_name_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_17 == NULL)) {
                tmp_dict_arg_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_17 == NULL));
            tmp_key_name_17 = mod_consts[32];
            tmp_tuple_element_17 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_17, tmp_key_name_17);
            if (tmp_tuple_element_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 322;

                goto list_build_exception_1;
            }
            tmp_list_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_1, 0, tmp_tuple_element_17);
            tmp_tuple_element_17 = mod_consts[35];
            PyTuple_SET_ITEM0(tmp_list_element_1, 1, tmp_tuple_element_17);
            PyList_SET_ITEM(tmp_frozenset_arg_1, 16, tmp_list_element_1);
        }
        goto list_build_noexception_1;
        // Exception handling pass through code for list_build:
        list_build_exception_1:;
        Py_DECREF(tmp_frozenset_arg_1);
        goto frame_exception_exit_1;
        // Finished with no exception for list_build:
        list_build_noexception_1:;
        tmp_assign_source_12 = PyFrozenSet_New(tmp_frozenset_arg_1);
        Py_DECREF(tmp_frozenset_arg_1);
        if (tmp_assign_source_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 305;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[36], tmp_assign_source_12);
    }
    {
        PyObject *tmp_assign_source_13;
        PyObject *tmp_frozenset_arg_2;
        PyObject *tmp_list_element_2;
        PyObject *tmp_tuple_element_18;
        PyObject *tmp_dict_arg_name_18;
        PyObject *tmp_key_name_18;
        tmp_dict_arg_name_18 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

        if (unlikely(tmp_dict_arg_name_18 == NULL)) {
            tmp_dict_arg_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
        }

        assert(!(tmp_dict_arg_name_18 == NULL));
        tmp_key_name_18 = mod_consts[17];
        tmp_tuple_element_18 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_18, tmp_key_name_18);
        if (tmp_tuple_element_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 326;

            goto frame_exception_exit_1;
        }
        tmp_list_element_2 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_18);
        tmp_tuple_element_18 = mod_consts[37];
        PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_18);
        tmp_frozenset_arg_2 = PyList_New(14);
        {
            PyObject *tmp_tuple_element_19;
            PyObject *tmp_dict_arg_name_19;
            PyObject *tmp_key_name_19;
            PyObject *tmp_tuple_element_20;
            PyObject *tmp_dict_arg_name_20;
            PyObject *tmp_key_name_20;
            PyObject *tmp_tuple_element_21;
            PyObject *tmp_dict_arg_name_21;
            PyObject *tmp_key_name_21;
            PyObject *tmp_tuple_element_22;
            PyObject *tmp_dict_arg_name_22;
            PyObject *tmp_key_name_22;
            PyObject *tmp_tuple_element_23;
            PyObject *tmp_dict_arg_name_23;
            PyObject *tmp_key_name_23;
            PyObject *tmp_tuple_element_24;
            PyObject *tmp_dict_arg_name_24;
            PyObject *tmp_key_name_24;
            PyObject *tmp_tuple_element_25;
            PyObject *tmp_dict_arg_name_25;
            PyObject *tmp_key_name_25;
            PyObject *tmp_tuple_element_26;
            PyObject *tmp_dict_arg_name_26;
            PyObject *tmp_key_name_26;
            PyObject *tmp_tuple_element_27;
            PyObject *tmp_dict_arg_name_27;
            PyObject *tmp_key_name_27;
            PyObject *tmp_tuple_element_28;
            PyObject *tmp_dict_arg_name_28;
            PyObject *tmp_key_name_28;
            PyObject *tmp_tuple_element_29;
            PyObject *tmp_dict_arg_name_29;
            PyObject *tmp_key_name_29;
            PyObject *tmp_tuple_element_30;
            PyObject *tmp_dict_arg_name_30;
            PyObject *tmp_key_name_30;
            PyObject *tmp_tuple_element_31;
            PyObject *tmp_dict_arg_name_31;
            PyObject *tmp_key_name_31;
            PyList_SET_ITEM(tmp_frozenset_arg_2, 0, tmp_list_element_2);
            tmp_dict_arg_name_19 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_19 == NULL)) {
                tmp_dict_arg_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_19 == NULL));
            tmp_key_name_19 = mod_consts[17];
            tmp_tuple_element_19 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_19, tmp_key_name_19);
            if (tmp_tuple_element_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 327;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_19);
            tmp_tuple_element_19 = mod_consts[38];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_19);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 1, tmp_list_element_2);
            tmp_dict_arg_name_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_20 == NULL)) {
                tmp_dict_arg_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_20 == NULL));
            tmp_key_name_20 = mod_consts[17];
            tmp_tuple_element_20 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_20, tmp_key_name_20);
            if (tmp_tuple_element_20 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 328;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_20);
            tmp_tuple_element_20 = mod_consts[39];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_20);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 2, tmp_list_element_2);
            tmp_dict_arg_name_21 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_21 == NULL)) {
                tmp_dict_arg_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_21 == NULL));
            tmp_key_name_21 = mod_consts[17];
            tmp_tuple_element_21 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_21, tmp_key_name_21);
            if (tmp_tuple_element_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 329;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_21);
            tmp_tuple_element_21 = mod_consts[40];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_21);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 3, tmp_list_element_2);
            tmp_dict_arg_name_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_22 == NULL)) {
                tmp_dict_arg_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_22 == NULL));
            tmp_key_name_22 = mod_consts[17];
            tmp_tuple_element_22 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_22, tmp_key_name_22);
            if (tmp_tuple_element_22 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 330;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_22);
            tmp_tuple_element_22 = mod_consts[41];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_22);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 4, tmp_list_element_2);
            tmp_dict_arg_name_23 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_23 == NULL)) {
                tmp_dict_arg_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_23 == NULL));
            tmp_key_name_23 = mod_consts[17];
            tmp_tuple_element_23 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_23, tmp_key_name_23);
            if (tmp_tuple_element_23 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 331;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_23);
            tmp_tuple_element_23 = mod_consts[42];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_23);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 5, tmp_list_element_2);
            tmp_dict_arg_name_24 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_24 == NULL)) {
                tmp_dict_arg_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_24 == NULL));
            tmp_key_name_24 = mod_consts[17];
            tmp_tuple_element_24 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_24, tmp_key_name_24);
            if (tmp_tuple_element_24 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 332;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_24);
            tmp_tuple_element_24 = mod_consts[43];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_24);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 6, tmp_list_element_2);
            tmp_dict_arg_name_25 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_25 == NULL)) {
                tmp_dict_arg_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_25 == NULL));
            tmp_key_name_25 = mod_consts[17];
            tmp_tuple_element_25 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_25, tmp_key_name_25);
            if (tmp_tuple_element_25 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 333;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_25);
            tmp_tuple_element_25 = mod_consts[44];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_25);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 7, tmp_list_element_2);
            tmp_dict_arg_name_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_26 == NULL)) {
                tmp_dict_arg_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_26 == NULL));
            tmp_key_name_26 = mod_consts[17];
            tmp_tuple_element_26 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_26, tmp_key_name_26);
            if (tmp_tuple_element_26 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 334;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_26);
            tmp_tuple_element_26 = mod_consts[45];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_26);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 8, tmp_list_element_2);
            tmp_dict_arg_name_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_27 == NULL)) {
                tmp_dict_arg_name_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_27 == NULL));
            tmp_key_name_27 = mod_consts[17];
            tmp_tuple_element_27 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_27, tmp_key_name_27);
            if (tmp_tuple_element_27 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 335;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_27);
            tmp_tuple_element_27 = mod_consts[46];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_27);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 9, tmp_list_element_2);
            tmp_dict_arg_name_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_28 == NULL)) {
                tmp_dict_arg_name_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_28 == NULL));
            tmp_key_name_28 = mod_consts[17];
            tmp_tuple_element_28 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_28, tmp_key_name_28);
            if (tmp_tuple_element_28 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 336;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_28);
            tmp_tuple_element_28 = mod_consts[47];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_28);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 10, tmp_list_element_2);
            tmp_dict_arg_name_29 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_29 == NULL)) {
                tmp_dict_arg_name_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_29 == NULL));
            tmp_key_name_29 = mod_consts[17];
            tmp_tuple_element_29 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_29, tmp_key_name_29);
            if (tmp_tuple_element_29 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 337;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_29);
            tmp_tuple_element_29 = mod_consts[48];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_29);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 11, tmp_list_element_2);
            tmp_dict_arg_name_30 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_30 == NULL)) {
                tmp_dict_arg_name_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_30 == NULL));
            tmp_key_name_30 = mod_consts[17];
            tmp_tuple_element_30 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_30, tmp_key_name_30);
            if (tmp_tuple_element_30 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 338;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_30);
            tmp_tuple_element_30 = mod_consts[49];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_30);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 12, tmp_list_element_2);
            tmp_dict_arg_name_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_31 == NULL)) {
                tmp_dict_arg_name_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_31 == NULL));
            tmp_key_name_31 = mod_consts[17];
            tmp_tuple_element_31 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_31, tmp_key_name_31);
            if (tmp_tuple_element_31 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 339;

                goto list_build_exception_2;
            }
            tmp_list_element_2 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_2, 0, tmp_tuple_element_31);
            tmp_tuple_element_31 = mod_consts[50];
            PyTuple_SET_ITEM0(tmp_list_element_2, 1, tmp_tuple_element_31);
            PyList_SET_ITEM(tmp_frozenset_arg_2, 13, tmp_list_element_2);
        }
        goto list_build_noexception_2;
        // Exception handling pass through code for list_build:
        list_build_exception_2:;
        Py_DECREF(tmp_frozenset_arg_2);
        goto frame_exception_exit_1;
        // Finished with no exception for list_build:
        list_build_noexception_2:;
        tmp_assign_source_13 = PyFrozenSet_New(tmp_frozenset_arg_2);
        Py_DECREF(tmp_frozenset_arg_2);
        if (tmp_assign_source_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 325;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[51], tmp_assign_source_13);
    }
    {
        PyObject *tmp_assign_source_14;
        PyObject *tmp_frozenset_arg_3;
        PyObject *tmp_list_element_3;
        PyObject *tmp_tuple_element_32;
        PyObject *tmp_dict_arg_name_32;
        PyObject *tmp_key_name_32;
        tmp_dict_arg_name_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

        if (unlikely(tmp_dict_arg_name_32 == NULL)) {
            tmp_dict_arg_name_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
        }

        assert(!(tmp_dict_arg_name_32 == NULL));
        tmp_key_name_32 = mod_consts[17];
        tmp_tuple_element_32 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_32, tmp_key_name_32);
        if (tmp_tuple_element_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 343;

            goto frame_exception_exit_1;
        }
        tmp_list_element_3 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_32);
        tmp_tuple_element_32 = mod_consts[52];
        PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_32);
        tmp_frozenset_arg_3 = PyList_New(78);
        {
            PyObject *tmp_tuple_element_33;
            PyObject *tmp_dict_arg_name_33;
            PyObject *tmp_key_name_33;
            PyObject *tmp_tuple_element_34;
            PyObject *tmp_dict_arg_name_34;
            PyObject *tmp_key_name_34;
            PyObject *tmp_tuple_element_35;
            PyObject *tmp_dict_arg_name_35;
            PyObject *tmp_key_name_35;
            PyObject *tmp_tuple_element_36;
            PyObject *tmp_dict_arg_name_36;
            PyObject *tmp_key_name_36;
            PyObject *tmp_tuple_element_37;
            PyObject *tmp_dict_arg_name_37;
            PyObject *tmp_key_name_37;
            PyObject *tmp_tuple_element_38;
            PyObject *tmp_dict_arg_name_38;
            PyObject *tmp_key_name_38;
            PyObject *tmp_tuple_element_39;
            PyObject *tmp_dict_arg_name_39;
            PyObject *tmp_key_name_39;
            PyObject *tmp_tuple_element_40;
            PyObject *tmp_dict_arg_name_40;
            PyObject *tmp_key_name_40;
            PyObject *tmp_tuple_element_41;
            PyObject *tmp_dict_arg_name_41;
            PyObject *tmp_key_name_41;
            PyObject *tmp_tuple_element_42;
            PyObject *tmp_dict_arg_name_42;
            PyObject *tmp_key_name_42;
            PyObject *tmp_tuple_element_43;
            PyObject *tmp_dict_arg_name_43;
            PyObject *tmp_key_name_43;
            PyObject *tmp_tuple_element_44;
            PyObject *tmp_dict_arg_name_44;
            PyObject *tmp_key_name_44;
            PyObject *tmp_tuple_element_45;
            PyObject *tmp_dict_arg_name_45;
            PyObject *tmp_key_name_45;
            PyObject *tmp_tuple_element_46;
            PyObject *tmp_dict_arg_name_46;
            PyObject *tmp_key_name_46;
            PyObject *tmp_tuple_element_47;
            PyObject *tmp_dict_arg_name_47;
            PyObject *tmp_key_name_47;
            PyObject *tmp_tuple_element_48;
            PyObject *tmp_dict_arg_name_48;
            PyObject *tmp_key_name_48;
            PyObject *tmp_tuple_element_49;
            PyObject *tmp_dict_arg_name_49;
            PyObject *tmp_key_name_49;
            PyObject *tmp_tuple_element_50;
            PyObject *tmp_dict_arg_name_50;
            PyObject *tmp_key_name_50;
            PyObject *tmp_tuple_element_51;
            PyObject *tmp_dict_arg_name_51;
            PyObject *tmp_key_name_51;
            PyObject *tmp_tuple_element_52;
            PyObject *tmp_dict_arg_name_52;
            PyObject *tmp_key_name_52;
            PyObject *tmp_tuple_element_53;
            PyObject *tmp_dict_arg_name_53;
            PyObject *tmp_key_name_53;
            PyObject *tmp_tuple_element_54;
            PyObject *tmp_dict_arg_name_54;
            PyObject *tmp_key_name_54;
            PyObject *tmp_tuple_element_55;
            PyObject *tmp_dict_arg_name_55;
            PyObject *tmp_key_name_55;
            PyObject *tmp_tuple_element_56;
            PyObject *tmp_dict_arg_name_56;
            PyObject *tmp_key_name_56;
            PyObject *tmp_tuple_element_57;
            PyObject *tmp_dict_arg_name_57;
            PyObject *tmp_key_name_57;
            PyObject *tmp_tuple_element_58;
            PyObject *tmp_dict_arg_name_58;
            PyObject *tmp_key_name_58;
            PyObject *tmp_tuple_element_59;
            PyObject *tmp_dict_arg_name_59;
            PyObject *tmp_key_name_59;
            PyObject *tmp_tuple_element_60;
            PyObject *tmp_dict_arg_name_60;
            PyObject *tmp_key_name_60;
            PyObject *tmp_tuple_element_61;
            PyObject *tmp_dict_arg_name_61;
            PyObject *tmp_key_name_61;
            PyObject *tmp_tuple_element_62;
            PyObject *tmp_dict_arg_name_62;
            PyObject *tmp_key_name_62;
            PyObject *tmp_tuple_element_63;
            PyObject *tmp_dict_arg_name_63;
            PyObject *tmp_key_name_63;
            PyObject *tmp_tuple_element_64;
            PyObject *tmp_dict_arg_name_64;
            PyObject *tmp_key_name_64;
            PyObject *tmp_tuple_element_65;
            PyObject *tmp_dict_arg_name_65;
            PyObject *tmp_key_name_65;
            PyObject *tmp_tuple_element_66;
            PyObject *tmp_dict_arg_name_66;
            PyObject *tmp_key_name_66;
            PyObject *tmp_tuple_element_67;
            PyObject *tmp_dict_arg_name_67;
            PyObject *tmp_key_name_67;
            PyObject *tmp_tuple_element_68;
            PyObject *tmp_dict_arg_name_68;
            PyObject *tmp_key_name_68;
            PyObject *tmp_tuple_element_69;
            PyObject *tmp_dict_arg_name_69;
            PyObject *tmp_key_name_69;
            PyObject *tmp_tuple_element_70;
            PyObject *tmp_dict_arg_name_70;
            PyObject *tmp_key_name_70;
            PyObject *tmp_tuple_element_71;
            PyObject *tmp_dict_arg_name_71;
            PyObject *tmp_key_name_71;
            PyObject *tmp_tuple_element_72;
            PyObject *tmp_dict_arg_name_72;
            PyObject *tmp_key_name_72;
            PyObject *tmp_tuple_element_73;
            PyObject *tmp_dict_arg_name_73;
            PyObject *tmp_key_name_73;
            PyObject *tmp_tuple_element_74;
            PyObject *tmp_dict_arg_name_74;
            PyObject *tmp_key_name_74;
            PyObject *tmp_tuple_element_75;
            PyObject *tmp_dict_arg_name_75;
            PyObject *tmp_key_name_75;
            PyObject *tmp_tuple_element_76;
            PyObject *tmp_dict_arg_name_76;
            PyObject *tmp_key_name_76;
            PyObject *tmp_tuple_element_77;
            PyObject *tmp_dict_arg_name_77;
            PyObject *tmp_key_name_77;
            PyObject *tmp_tuple_element_78;
            PyObject *tmp_dict_arg_name_78;
            PyObject *tmp_key_name_78;
            PyObject *tmp_tuple_element_79;
            PyObject *tmp_dict_arg_name_79;
            PyObject *tmp_key_name_79;
            PyObject *tmp_tuple_element_80;
            PyObject *tmp_dict_arg_name_80;
            PyObject *tmp_key_name_80;
            PyObject *tmp_tuple_element_81;
            PyObject *tmp_dict_arg_name_81;
            PyObject *tmp_key_name_81;
            PyObject *tmp_tuple_element_82;
            PyObject *tmp_dict_arg_name_82;
            PyObject *tmp_key_name_82;
            PyObject *tmp_tuple_element_83;
            PyObject *tmp_dict_arg_name_83;
            PyObject *tmp_key_name_83;
            PyObject *tmp_tuple_element_84;
            PyObject *tmp_dict_arg_name_84;
            PyObject *tmp_key_name_84;
            PyObject *tmp_tuple_element_85;
            PyObject *tmp_dict_arg_name_85;
            PyObject *tmp_key_name_85;
            PyObject *tmp_tuple_element_86;
            PyObject *tmp_dict_arg_name_86;
            PyObject *tmp_key_name_86;
            PyObject *tmp_tuple_element_87;
            PyObject *tmp_dict_arg_name_87;
            PyObject *tmp_key_name_87;
            PyObject *tmp_tuple_element_88;
            PyObject *tmp_dict_arg_name_88;
            PyObject *tmp_key_name_88;
            PyObject *tmp_tuple_element_89;
            PyObject *tmp_dict_arg_name_89;
            PyObject *tmp_key_name_89;
            PyObject *tmp_tuple_element_90;
            PyObject *tmp_dict_arg_name_90;
            PyObject *tmp_key_name_90;
            PyObject *tmp_tuple_element_91;
            PyObject *tmp_dict_arg_name_91;
            PyObject *tmp_key_name_91;
            PyObject *tmp_tuple_element_92;
            PyObject *tmp_dict_arg_name_92;
            PyObject *tmp_key_name_92;
            PyObject *tmp_tuple_element_93;
            PyObject *tmp_dict_arg_name_93;
            PyObject *tmp_key_name_93;
            PyObject *tmp_tuple_element_94;
            PyObject *tmp_dict_arg_name_94;
            PyObject *tmp_key_name_94;
            PyObject *tmp_tuple_element_95;
            PyObject *tmp_dict_arg_name_95;
            PyObject *tmp_key_name_95;
            PyObject *tmp_tuple_element_96;
            PyObject *tmp_dict_arg_name_96;
            PyObject *tmp_key_name_96;
            PyObject *tmp_tuple_element_97;
            PyObject *tmp_dict_arg_name_97;
            PyObject *tmp_key_name_97;
            PyObject *tmp_tuple_element_98;
            PyObject *tmp_dict_arg_name_98;
            PyObject *tmp_key_name_98;
            PyObject *tmp_tuple_element_99;
            PyObject *tmp_dict_arg_name_99;
            PyObject *tmp_key_name_99;
            PyObject *tmp_tuple_element_100;
            PyObject *tmp_dict_arg_name_100;
            PyObject *tmp_key_name_100;
            PyObject *tmp_tuple_element_101;
            PyObject *tmp_dict_arg_name_101;
            PyObject *tmp_key_name_101;
            PyObject *tmp_tuple_element_102;
            PyObject *tmp_dict_arg_name_102;
            PyObject *tmp_key_name_102;
            PyObject *tmp_tuple_element_103;
            PyObject *tmp_dict_arg_name_103;
            PyObject *tmp_key_name_103;
            PyObject *tmp_tuple_element_104;
            PyObject *tmp_dict_arg_name_104;
            PyObject *tmp_key_name_104;
            PyObject *tmp_tuple_element_105;
            PyObject *tmp_dict_arg_name_105;
            PyObject *tmp_key_name_105;
            PyObject *tmp_tuple_element_106;
            PyObject *tmp_dict_arg_name_106;
            PyObject *tmp_key_name_106;
            PyObject *tmp_tuple_element_107;
            PyObject *tmp_dict_arg_name_107;
            PyObject *tmp_key_name_107;
            PyObject *tmp_tuple_element_108;
            PyObject *tmp_dict_arg_name_108;
            PyObject *tmp_key_name_108;
            PyObject *tmp_tuple_element_109;
            PyObject *tmp_dict_arg_name_109;
            PyObject *tmp_key_name_109;
            PyList_SET_ITEM(tmp_frozenset_arg_3, 0, tmp_list_element_3);
            tmp_dict_arg_name_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_33 == NULL)) {
                tmp_dict_arg_name_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_33 == NULL));
            tmp_key_name_33 = mod_consts[17];
            tmp_tuple_element_33 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_33, tmp_key_name_33);
            if (tmp_tuple_element_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 344;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_33);
            tmp_tuple_element_33 = mod_consts[18];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_33);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 1, tmp_list_element_3);
            tmp_dict_arg_name_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_34 == NULL)) {
                tmp_dict_arg_name_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_34 == NULL));
            tmp_key_name_34 = mod_consts[17];
            tmp_tuple_element_34 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_34, tmp_key_name_34);
            if (tmp_tuple_element_34 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 345;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_34);
            tmp_tuple_element_34 = mod_consts[53];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_34);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 2, tmp_list_element_3);
            tmp_dict_arg_name_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_35 == NULL)) {
                tmp_dict_arg_name_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_35 == NULL));
            tmp_key_name_35 = mod_consts[17];
            tmp_tuple_element_35 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_35, tmp_key_name_35);
            if (tmp_tuple_element_35 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 346;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_35);
            tmp_tuple_element_35 = mod_consts[54];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_35);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 3, tmp_list_element_3);
            tmp_dict_arg_name_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_36 == NULL)) {
                tmp_dict_arg_name_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_36 == NULL));
            tmp_key_name_36 = mod_consts[17];
            tmp_tuple_element_36 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_36, tmp_key_name_36);
            if (tmp_tuple_element_36 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 347;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_36);
            tmp_tuple_element_36 = mod_consts[55];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_36);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 4, tmp_list_element_3);
            tmp_dict_arg_name_37 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_37 == NULL)) {
                tmp_dict_arg_name_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_37 == NULL));
            tmp_key_name_37 = mod_consts[17];
            tmp_tuple_element_37 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_37, tmp_key_name_37);
            if (tmp_tuple_element_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 348;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_37);
            tmp_tuple_element_37 = mod_consts[56];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_37);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 5, tmp_list_element_3);
            tmp_dict_arg_name_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_38 == NULL)) {
                tmp_dict_arg_name_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_38 == NULL));
            tmp_key_name_38 = mod_consts[17];
            tmp_tuple_element_38 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_38, tmp_key_name_38);
            if (tmp_tuple_element_38 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 349;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_38);
            tmp_tuple_element_38 = mod_consts[57];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_38);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 6, tmp_list_element_3);
            tmp_dict_arg_name_39 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_39 == NULL)) {
                tmp_dict_arg_name_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_39 == NULL));
            tmp_key_name_39 = mod_consts[17];
            tmp_tuple_element_39 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_39, tmp_key_name_39);
            if (tmp_tuple_element_39 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 350;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_39);
            tmp_tuple_element_39 = mod_consts[58];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_39);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 7, tmp_list_element_3);
            tmp_dict_arg_name_40 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_40 == NULL)) {
                tmp_dict_arg_name_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_40 == NULL));
            tmp_key_name_40 = mod_consts[17];
            tmp_tuple_element_40 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_40, tmp_key_name_40);
            if (tmp_tuple_element_40 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 351;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_40);
            tmp_tuple_element_40 = mod_consts[59];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_40);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 8, tmp_list_element_3);
            tmp_dict_arg_name_41 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_41 == NULL)) {
                tmp_dict_arg_name_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_41 == NULL));
            tmp_key_name_41 = mod_consts[17];
            tmp_tuple_element_41 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_41, tmp_key_name_41);
            if (tmp_tuple_element_41 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 352;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_41);
            tmp_tuple_element_41 = mod_consts[60];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_41);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 9, tmp_list_element_3);
            tmp_dict_arg_name_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_42 == NULL)) {
                tmp_dict_arg_name_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_42 == NULL));
            tmp_key_name_42 = mod_consts[17];
            tmp_tuple_element_42 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_42, tmp_key_name_42);
            if (tmp_tuple_element_42 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 353;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_42);
            tmp_tuple_element_42 = mod_consts[61];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_42);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 10, tmp_list_element_3);
            tmp_dict_arg_name_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_43 == NULL)) {
                tmp_dict_arg_name_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_43 == NULL));
            tmp_key_name_43 = mod_consts[17];
            tmp_tuple_element_43 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_43, tmp_key_name_43);
            if (tmp_tuple_element_43 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 354;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_43);
            tmp_tuple_element_43 = mod_consts[62];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_43);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 11, tmp_list_element_3);
            tmp_dict_arg_name_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_44 == NULL)) {
                tmp_dict_arg_name_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_44 == NULL));
            tmp_key_name_44 = mod_consts[17];
            tmp_tuple_element_44 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_44, tmp_key_name_44);
            if (tmp_tuple_element_44 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 355;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_44);
            tmp_tuple_element_44 = mod_consts[19];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_44);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 12, tmp_list_element_3);
            tmp_dict_arg_name_45 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_45 == NULL)) {
                tmp_dict_arg_name_45 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_45 == NULL));
            tmp_key_name_45 = mod_consts[17];
            tmp_tuple_element_45 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_45, tmp_key_name_45);
            if (tmp_tuple_element_45 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 356;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_45);
            tmp_tuple_element_45 = mod_consts[63];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_45);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 13, tmp_list_element_3);
            tmp_dict_arg_name_46 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_46 == NULL)) {
                tmp_dict_arg_name_46 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_46 == NULL));
            tmp_key_name_46 = mod_consts[17];
            tmp_tuple_element_46 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_46, tmp_key_name_46);
            if (tmp_tuple_element_46 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 357;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_46);
            tmp_tuple_element_46 = mod_consts[64];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_46);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 14, tmp_list_element_3);
            tmp_dict_arg_name_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_47 == NULL)) {
                tmp_dict_arg_name_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_47 == NULL));
            tmp_key_name_47 = mod_consts[17];
            tmp_tuple_element_47 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_47, tmp_key_name_47);
            if (tmp_tuple_element_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 358;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_47);
            tmp_tuple_element_47 = mod_consts[65];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_47);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 15, tmp_list_element_3);
            tmp_dict_arg_name_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_48 == NULL)) {
                tmp_dict_arg_name_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_48 == NULL));
            tmp_key_name_48 = mod_consts[17];
            tmp_tuple_element_48 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_48, tmp_key_name_48);
            if (tmp_tuple_element_48 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 359;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_48);
            tmp_tuple_element_48 = mod_consts[66];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_48);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 16, tmp_list_element_3);
            tmp_dict_arg_name_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_49 == NULL)) {
                tmp_dict_arg_name_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_49 == NULL));
            tmp_key_name_49 = mod_consts[17];
            tmp_tuple_element_49 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_49, tmp_key_name_49);
            if (tmp_tuple_element_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 360;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_49);
            tmp_tuple_element_49 = mod_consts[67];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_49);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 17, tmp_list_element_3);
            tmp_dict_arg_name_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_50 == NULL)) {
                tmp_dict_arg_name_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_50 == NULL));
            tmp_key_name_50 = mod_consts[17];
            tmp_tuple_element_50 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_50, tmp_key_name_50);
            if (tmp_tuple_element_50 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 361;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_50);
            tmp_tuple_element_50 = mod_consts[68];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_50);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 18, tmp_list_element_3);
            tmp_dict_arg_name_51 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_51 == NULL)) {
                tmp_dict_arg_name_51 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_51 == NULL));
            tmp_key_name_51 = mod_consts[17];
            tmp_tuple_element_51 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_51, tmp_key_name_51);
            if (tmp_tuple_element_51 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 362;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_51);
            tmp_tuple_element_51 = mod_consts[69];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_51);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 19, tmp_list_element_3);
            tmp_dict_arg_name_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_52 == NULL)) {
                tmp_dict_arg_name_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_52 == NULL));
            tmp_key_name_52 = mod_consts[17];
            tmp_tuple_element_52 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_52, tmp_key_name_52);
            if (tmp_tuple_element_52 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 363;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_52);
            tmp_tuple_element_52 = mod_consts[70];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_52);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 20, tmp_list_element_3);
            tmp_dict_arg_name_53 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_53 == NULL)) {
                tmp_dict_arg_name_53 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_53 == NULL));
            tmp_key_name_53 = mod_consts[17];
            tmp_tuple_element_53 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_53, tmp_key_name_53);
            if (tmp_tuple_element_53 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 364;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_53);
            tmp_tuple_element_53 = mod_consts[71];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_53);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 21, tmp_list_element_3);
            tmp_dict_arg_name_54 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_54 == NULL)) {
                tmp_dict_arg_name_54 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_54 == NULL));
            tmp_key_name_54 = mod_consts[17];
            tmp_tuple_element_54 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_54, tmp_key_name_54);
            if (tmp_tuple_element_54 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 365;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_54);
            tmp_tuple_element_54 = mod_consts[72];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_54);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 22, tmp_list_element_3);
            tmp_dict_arg_name_55 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_55 == NULL)) {
                tmp_dict_arg_name_55 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_55 == NULL));
            tmp_key_name_55 = mod_consts[17];
            tmp_tuple_element_55 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_55, tmp_key_name_55);
            if (tmp_tuple_element_55 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 366;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_55);
            tmp_tuple_element_55 = mod_consts[73];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_55);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 23, tmp_list_element_3);
            tmp_dict_arg_name_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_56 == NULL)) {
                tmp_dict_arg_name_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_56 == NULL));
            tmp_key_name_56 = mod_consts[17];
            tmp_tuple_element_56 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_56, tmp_key_name_56);
            if (tmp_tuple_element_56 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 367;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_56);
            tmp_tuple_element_56 = mod_consts[74];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_56);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 24, tmp_list_element_3);
            tmp_dict_arg_name_57 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_57 == NULL)) {
                tmp_dict_arg_name_57 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_57 == NULL));
            tmp_key_name_57 = mod_consts[17];
            tmp_tuple_element_57 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_57, tmp_key_name_57);
            if (tmp_tuple_element_57 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 368;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_57);
            tmp_tuple_element_57 = mod_consts[75];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_57);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 25, tmp_list_element_3);
            tmp_dict_arg_name_58 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_58 == NULL)) {
                tmp_dict_arg_name_58 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_58 == NULL));
            tmp_key_name_58 = mod_consts[17];
            tmp_tuple_element_58 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_58, tmp_key_name_58);
            if (tmp_tuple_element_58 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 369;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_58);
            tmp_tuple_element_58 = mod_consts[76];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_58);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 26, tmp_list_element_3);
            tmp_dict_arg_name_59 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_59 == NULL)) {
                tmp_dict_arg_name_59 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_59 == NULL));
            tmp_key_name_59 = mod_consts[17];
            tmp_tuple_element_59 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_59, tmp_key_name_59);
            if (tmp_tuple_element_59 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 370;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_59);
            tmp_tuple_element_59 = mod_consts[77];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_59);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 27, tmp_list_element_3);
            tmp_dict_arg_name_60 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_60 == NULL)) {
                tmp_dict_arg_name_60 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_60 == NULL));
            tmp_key_name_60 = mod_consts[17];
            tmp_tuple_element_60 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_60, tmp_key_name_60);
            if (tmp_tuple_element_60 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 371;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_60);
            tmp_tuple_element_60 = mod_consts[78];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_60);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 28, tmp_list_element_3);
            tmp_dict_arg_name_61 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_61 == NULL)) {
                tmp_dict_arg_name_61 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_61 == NULL));
            tmp_key_name_61 = mod_consts[17];
            tmp_tuple_element_61 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_61, tmp_key_name_61);
            if (tmp_tuple_element_61 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 372;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_61);
            tmp_tuple_element_61 = mod_consts[79];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_61);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 29, tmp_list_element_3);
            tmp_dict_arg_name_62 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_62 == NULL)) {
                tmp_dict_arg_name_62 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_62 == NULL));
            tmp_key_name_62 = mod_consts[17];
            tmp_tuple_element_62 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_62, tmp_key_name_62);
            if (tmp_tuple_element_62 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 373;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_62);
            tmp_tuple_element_62 = mod_consts[80];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_62);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 30, tmp_list_element_3);
            tmp_dict_arg_name_63 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_63 == NULL)) {
                tmp_dict_arg_name_63 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_63 == NULL));
            tmp_key_name_63 = mod_consts[17];
            tmp_tuple_element_63 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_63, tmp_key_name_63);
            if (tmp_tuple_element_63 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 374;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_63);
            tmp_tuple_element_63 = mod_consts[81];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_63);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 31, tmp_list_element_3);
            tmp_dict_arg_name_64 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_64 == NULL)) {
                tmp_dict_arg_name_64 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_64 == NULL));
            tmp_key_name_64 = mod_consts[17];
            tmp_tuple_element_64 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_64, tmp_key_name_64);
            if (tmp_tuple_element_64 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 375;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_64);
            tmp_tuple_element_64 = mod_consts[82];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_64);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 32, tmp_list_element_3);
            tmp_dict_arg_name_65 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_65 == NULL)) {
                tmp_dict_arg_name_65 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_65 == NULL));
            tmp_key_name_65 = mod_consts[17];
            tmp_tuple_element_65 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_65, tmp_key_name_65);
            if (tmp_tuple_element_65 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 376;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_65);
            tmp_tuple_element_65 = mod_consts[83];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_65);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 33, tmp_list_element_3);
            tmp_dict_arg_name_66 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_66 == NULL)) {
                tmp_dict_arg_name_66 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_66 == NULL));
            tmp_key_name_66 = mod_consts[17];
            tmp_tuple_element_66 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_66, tmp_key_name_66);
            if (tmp_tuple_element_66 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 377;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_66);
            tmp_tuple_element_66 = mod_consts[84];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_66);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 34, tmp_list_element_3);
            tmp_dict_arg_name_67 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_67 == NULL)) {
                tmp_dict_arg_name_67 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_67 == NULL));
            tmp_key_name_67 = mod_consts[17];
            tmp_tuple_element_67 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_67, tmp_key_name_67);
            if (tmp_tuple_element_67 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 378;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_67);
            tmp_tuple_element_67 = mod_consts[85];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_67);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 35, tmp_list_element_3);
            tmp_dict_arg_name_68 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_68 == NULL)) {
                tmp_dict_arg_name_68 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_68 == NULL));
            tmp_key_name_68 = mod_consts[17];
            tmp_tuple_element_68 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_68, tmp_key_name_68);
            if (tmp_tuple_element_68 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 379;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_68);
            tmp_tuple_element_68 = mod_consts[86];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_68);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 36, tmp_list_element_3);
            tmp_dict_arg_name_69 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_69 == NULL)) {
                tmp_dict_arg_name_69 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_69 == NULL));
            tmp_key_name_69 = mod_consts[17];
            tmp_tuple_element_69 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_69, tmp_key_name_69);
            if (tmp_tuple_element_69 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 380;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_69);
            tmp_tuple_element_69 = mod_consts[87];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_69);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 37, tmp_list_element_3);
            tmp_dict_arg_name_70 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_70 == NULL)) {
                tmp_dict_arg_name_70 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_70 == NULL));
            tmp_key_name_70 = mod_consts[17];
            tmp_tuple_element_70 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_70, tmp_key_name_70);
            if (tmp_tuple_element_70 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 381;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_70);
            tmp_tuple_element_70 = mod_consts[88];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_70);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 38, tmp_list_element_3);
            tmp_dict_arg_name_71 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_71 == NULL)) {
                tmp_dict_arg_name_71 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_71 == NULL));
            tmp_key_name_71 = mod_consts[17];
            tmp_tuple_element_71 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_71, tmp_key_name_71);
            if (tmp_tuple_element_71 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 382;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_71);
            tmp_tuple_element_71 = mod_consts[17];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_71);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 39, tmp_list_element_3);
            tmp_dict_arg_name_72 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_72 == NULL)) {
                tmp_dict_arg_name_72 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_72 == NULL));
            tmp_key_name_72 = mod_consts[17];
            tmp_tuple_element_72 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_72, tmp_key_name_72);
            if (tmp_tuple_element_72 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 383;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_72);
            tmp_tuple_element_72 = mod_consts[89];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_72);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 40, tmp_list_element_3);
            tmp_dict_arg_name_73 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_73 == NULL)) {
                tmp_dict_arg_name_73 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_73 == NULL));
            tmp_key_name_73 = mod_consts[17];
            tmp_tuple_element_73 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_73, tmp_key_name_73);
            if (tmp_tuple_element_73 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 386;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_73);
            tmp_tuple_element_73 = mod_consts[90];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_73);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 41, tmp_list_element_3);
            tmp_dict_arg_name_74 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_74 == NULL)) {
                tmp_dict_arg_name_74 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_74 == NULL));
            tmp_key_name_74 = mod_consts[17];
            tmp_tuple_element_74 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_74, tmp_key_name_74);
            if (tmp_tuple_element_74 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 387;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_74);
            tmp_tuple_element_74 = mod_consts[91];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_74);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 42, tmp_list_element_3);
            tmp_dict_arg_name_75 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_75 == NULL)) {
                tmp_dict_arg_name_75 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_75 == NULL));
            tmp_key_name_75 = mod_consts[17];
            tmp_tuple_element_75 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_75, tmp_key_name_75);
            if (tmp_tuple_element_75 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 388;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_75);
            tmp_tuple_element_75 = mod_consts[92];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_75);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 43, tmp_list_element_3);
            tmp_dict_arg_name_76 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_76 == NULL)) {
                tmp_dict_arg_name_76 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_76 == NULL));
            tmp_key_name_76 = mod_consts[17];
            tmp_tuple_element_76 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_76, tmp_key_name_76);
            if (tmp_tuple_element_76 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 389;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_76);
            tmp_tuple_element_76 = mod_consts[93];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_76);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 44, tmp_list_element_3);
            tmp_dict_arg_name_77 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_77 == NULL)) {
                tmp_dict_arg_name_77 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_77 == NULL));
            tmp_key_name_77 = mod_consts[17];
            tmp_tuple_element_77 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_77, tmp_key_name_77);
            if (tmp_tuple_element_77 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 390;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_77);
            tmp_tuple_element_77 = mod_consts[94];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_77);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 45, tmp_list_element_3);
            tmp_dict_arg_name_78 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_78 == NULL)) {
                tmp_dict_arg_name_78 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_78 == NULL));
            tmp_key_name_78 = mod_consts[17];
            tmp_tuple_element_78 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_78, tmp_key_name_78);
            if (tmp_tuple_element_78 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 391;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_78);
            tmp_tuple_element_78 = mod_consts[95];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_78);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 46, tmp_list_element_3);
            tmp_dict_arg_name_79 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_79 == NULL)) {
                tmp_dict_arg_name_79 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_79 == NULL));
            tmp_key_name_79 = mod_consts[17];
            tmp_tuple_element_79 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_79, tmp_key_name_79);
            if (tmp_tuple_element_79 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 392;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_79);
            tmp_tuple_element_79 = mod_consts[96];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_79);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 47, tmp_list_element_3);
            tmp_dict_arg_name_80 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_80 == NULL)) {
                tmp_dict_arg_name_80 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_80 == NULL));
            tmp_key_name_80 = mod_consts[17];
            tmp_tuple_element_80 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_80, tmp_key_name_80);
            if (tmp_tuple_element_80 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 393;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_80);
            tmp_tuple_element_80 = mod_consts[20];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_80);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 48, tmp_list_element_3);
            tmp_dict_arg_name_81 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_81 == NULL)) {
                tmp_dict_arg_name_81 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_81 == NULL));
            tmp_key_name_81 = mod_consts[17];
            tmp_tuple_element_81 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_81, tmp_key_name_81);
            if (tmp_tuple_element_81 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 394;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_81);
            tmp_tuple_element_81 = mod_consts[97];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_81);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 49, tmp_list_element_3);
            tmp_dict_arg_name_82 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_82 == NULL)) {
                tmp_dict_arg_name_82 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_82 == NULL));
            tmp_key_name_82 = mod_consts[17];
            tmp_tuple_element_82 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_82, tmp_key_name_82);
            if (tmp_tuple_element_82 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 395;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_82);
            tmp_tuple_element_82 = mod_consts[98];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_82);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 50, tmp_list_element_3);
            tmp_dict_arg_name_83 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_83 == NULL)) {
                tmp_dict_arg_name_83 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_83 == NULL));
            tmp_key_name_83 = mod_consts[17];
            tmp_tuple_element_83 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_83, tmp_key_name_83);
            if (tmp_tuple_element_83 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 396;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_83);
            tmp_tuple_element_83 = mod_consts[99];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_83);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 51, tmp_list_element_3);
            tmp_dict_arg_name_84 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_84 == NULL)) {
                tmp_dict_arg_name_84 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_84 == NULL));
            tmp_key_name_84 = mod_consts[17];
            tmp_tuple_element_84 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_84, tmp_key_name_84);
            if (tmp_tuple_element_84 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 397;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_84);
            tmp_tuple_element_84 = mod_consts[100];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_84);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 52, tmp_list_element_3);
            tmp_dict_arg_name_85 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_85 == NULL)) {
                tmp_dict_arg_name_85 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_85 == NULL));
            tmp_key_name_85 = mod_consts[17];
            tmp_tuple_element_85 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_85, tmp_key_name_85);
            if (tmp_tuple_element_85 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 398;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_85);
            tmp_tuple_element_85 = mod_consts[101];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_85);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 53, tmp_list_element_3);
            tmp_dict_arg_name_86 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_86 == NULL)) {
                tmp_dict_arg_name_86 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_86 == NULL));
            tmp_key_name_86 = mod_consts[17];
            tmp_tuple_element_86 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_86, tmp_key_name_86);
            if (tmp_tuple_element_86 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 399;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_86);
            tmp_tuple_element_86 = mod_consts[102];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_86);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 54, tmp_list_element_3);
            tmp_dict_arg_name_87 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_87 == NULL)) {
                tmp_dict_arg_name_87 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_87 == NULL));
            tmp_key_name_87 = mod_consts[17];
            tmp_tuple_element_87 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_87, tmp_key_name_87);
            if (tmp_tuple_element_87 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 400;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_87);
            tmp_tuple_element_87 = mod_consts[21];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_87);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 55, tmp_list_element_3);
            tmp_dict_arg_name_88 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_88 == NULL)) {
                tmp_dict_arg_name_88 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_88 == NULL));
            tmp_key_name_88 = mod_consts[17];
            tmp_tuple_element_88 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_88, tmp_key_name_88);
            if (tmp_tuple_element_88 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 401;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_88);
            tmp_tuple_element_88 = mod_consts[103];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_88);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 56, tmp_list_element_3);
            tmp_dict_arg_name_89 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_89 == NULL)) {
                tmp_dict_arg_name_89 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_89 == NULL));
            tmp_key_name_89 = mod_consts[17];
            tmp_tuple_element_89 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_89, tmp_key_name_89);
            if (tmp_tuple_element_89 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 402;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_89);
            tmp_tuple_element_89 = mod_consts[104];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_89);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 57, tmp_list_element_3);
            tmp_dict_arg_name_90 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_90 == NULL)) {
                tmp_dict_arg_name_90 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_90 == NULL));
            tmp_key_name_90 = mod_consts[17];
            tmp_tuple_element_90 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_90, tmp_key_name_90);
            if (tmp_tuple_element_90 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 403;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_90);
            tmp_tuple_element_90 = mod_consts[105];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_90);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 58, tmp_list_element_3);
            tmp_dict_arg_name_91 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_91 == NULL)) {
                tmp_dict_arg_name_91 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_91 == NULL));
            tmp_key_name_91 = mod_consts[17];
            tmp_tuple_element_91 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_91, tmp_key_name_91);
            if (tmp_tuple_element_91 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 404;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_91);
            tmp_tuple_element_91 = mod_consts[106];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_91);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 59, tmp_list_element_3);
            tmp_dict_arg_name_92 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_92 == NULL)) {
                tmp_dict_arg_name_92 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_92 == NULL));
            tmp_key_name_92 = mod_consts[17];
            tmp_tuple_element_92 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_92, tmp_key_name_92);
            if (tmp_tuple_element_92 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 405;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_92);
            tmp_tuple_element_92 = mod_consts[107];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_92);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 60, tmp_list_element_3);
            tmp_dict_arg_name_93 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_93 == NULL)) {
                tmp_dict_arg_name_93 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_93 == NULL));
            tmp_key_name_93 = mod_consts[17];
            tmp_tuple_element_93 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_93, tmp_key_name_93);
            if (tmp_tuple_element_93 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 406;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_93);
            tmp_tuple_element_93 = mod_consts[108];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_93);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 61, tmp_list_element_3);
            tmp_dict_arg_name_94 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_94 == NULL)) {
                tmp_dict_arg_name_94 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_94 == NULL));
            tmp_key_name_94 = mod_consts[17];
            tmp_tuple_element_94 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_94, tmp_key_name_94);
            if (tmp_tuple_element_94 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 407;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_94);
            tmp_tuple_element_94 = mod_consts[109];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_94);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 62, tmp_list_element_3);
            tmp_dict_arg_name_95 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_95 == NULL)) {
                tmp_dict_arg_name_95 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_95 == NULL));
            tmp_key_name_95 = mod_consts[17];
            tmp_tuple_element_95 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_95, tmp_key_name_95);
            if (tmp_tuple_element_95 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 408;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_95);
            tmp_tuple_element_95 = mod_consts[110];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_95);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 63, tmp_list_element_3);
            tmp_dict_arg_name_96 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_96 == NULL)) {
                tmp_dict_arg_name_96 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_96 == NULL));
            tmp_key_name_96 = mod_consts[17];
            tmp_tuple_element_96 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_96, tmp_key_name_96);
            if (tmp_tuple_element_96 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 409;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_96);
            tmp_tuple_element_96 = mod_consts[111];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_96);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 64, tmp_list_element_3);
            tmp_dict_arg_name_97 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_97 == NULL)) {
                tmp_dict_arg_name_97 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_97 == NULL));
            tmp_key_name_97 = mod_consts[17];
            tmp_tuple_element_97 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_97, tmp_key_name_97);
            if (tmp_tuple_element_97 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 410;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_97);
            tmp_tuple_element_97 = mod_consts[22];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_97);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 65, tmp_list_element_3);
            tmp_dict_arg_name_98 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_98 == NULL)) {
                tmp_dict_arg_name_98 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_98 == NULL));
            tmp_key_name_98 = mod_consts[17];
            tmp_tuple_element_98 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_98, tmp_key_name_98);
            if (tmp_tuple_element_98 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 411;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_98);
            tmp_tuple_element_98 = mod_consts[112];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_98);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 66, tmp_list_element_3);
            tmp_dict_arg_name_99 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_99 == NULL)) {
                tmp_dict_arg_name_99 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_99 == NULL));
            tmp_key_name_99 = mod_consts[17];
            tmp_tuple_element_99 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_99, tmp_key_name_99);
            if (tmp_tuple_element_99 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 412;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_99);
            tmp_tuple_element_99 = mod_consts[23];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_99);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 67, tmp_list_element_3);
            tmp_dict_arg_name_100 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_100 == NULL)) {
                tmp_dict_arg_name_100 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_100 == NULL));
            tmp_key_name_100 = mod_consts[17];
            tmp_tuple_element_100 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_100, tmp_key_name_100);
            if (tmp_tuple_element_100 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 413;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_100);
            tmp_tuple_element_100 = mod_consts[113];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_100);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 68, tmp_list_element_3);
            tmp_dict_arg_name_101 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_101 == NULL)) {
                tmp_dict_arg_name_101 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_101 == NULL));
            tmp_key_name_101 = mod_consts[17];
            tmp_tuple_element_101 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_101, tmp_key_name_101);
            if (tmp_tuple_element_101 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 414;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_101);
            tmp_tuple_element_101 = mod_consts[114];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_101);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 69, tmp_list_element_3);
            tmp_dict_arg_name_102 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_102 == NULL)) {
                tmp_dict_arg_name_102 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_102 == NULL));
            tmp_key_name_102 = mod_consts[17];
            tmp_tuple_element_102 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_102, tmp_key_name_102);
            if (tmp_tuple_element_102 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 415;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_102);
            tmp_tuple_element_102 = mod_consts[24];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_102);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 70, tmp_list_element_3);
            tmp_dict_arg_name_103 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_103 == NULL)) {
                tmp_dict_arg_name_103 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_103 == NULL));
            tmp_key_name_103 = mod_consts[17];
            tmp_tuple_element_103 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_103, tmp_key_name_103);
            if (tmp_tuple_element_103 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 416;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_103);
            tmp_tuple_element_103 = mod_consts[115];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_103);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 71, tmp_list_element_3);
            tmp_dict_arg_name_104 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_104 == NULL)) {
                tmp_dict_arg_name_104 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_104 == NULL));
            tmp_key_name_104 = mod_consts[17];
            tmp_tuple_element_104 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_104, tmp_key_name_104);
            if (tmp_tuple_element_104 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 417;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_104);
            tmp_tuple_element_104 = mod_consts[35];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_104);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 72, tmp_list_element_3);
            tmp_dict_arg_name_105 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_105 == NULL)) {
                tmp_dict_arg_name_105 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_105 == NULL));
            tmp_key_name_105 = mod_consts[17];
            tmp_tuple_element_105 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_105, tmp_key_name_105);
            if (tmp_tuple_element_105 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 418;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_105);
            tmp_tuple_element_105 = mod_consts[116];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_105);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 73, tmp_list_element_3);
            tmp_dict_arg_name_106 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_106 == NULL)) {
                tmp_dict_arg_name_106 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_106 == NULL));
            tmp_key_name_106 = mod_consts[17];
            tmp_tuple_element_106 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_106, tmp_key_name_106);
            if (tmp_tuple_element_106 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 419;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_106);
            tmp_tuple_element_106 = mod_consts[117];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_106);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 74, tmp_list_element_3);
            tmp_dict_arg_name_107 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_107 == NULL)) {
                tmp_dict_arg_name_107 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_107 == NULL));
            tmp_key_name_107 = mod_consts[17];
            tmp_tuple_element_107 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_107, tmp_key_name_107);
            if (tmp_tuple_element_107 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 420;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_107);
            tmp_tuple_element_107 = mod_consts[118];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_107);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 75, tmp_list_element_3);
            tmp_dict_arg_name_108 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_108 == NULL)) {
                tmp_dict_arg_name_108 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_108 == NULL));
            tmp_key_name_108 = mod_consts[17];
            tmp_tuple_element_108 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_108, tmp_key_name_108);
            if (tmp_tuple_element_108 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 421;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_108);
            tmp_tuple_element_108 = mod_consts[119];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_108);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 76, tmp_list_element_3);
            tmp_dict_arg_name_109 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_109 == NULL)) {
                tmp_dict_arg_name_109 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_109 == NULL));
            tmp_key_name_109 = mod_consts[32];
            tmp_tuple_element_109 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_109, tmp_key_name_109);
            if (tmp_tuple_element_109 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 422;

                goto list_build_exception_3;
            }
            tmp_list_element_3 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_3, 0, tmp_tuple_element_109);
            tmp_tuple_element_109 = mod_consts[33];
            PyTuple_SET_ITEM0(tmp_list_element_3, 1, tmp_tuple_element_109);
            PyList_SET_ITEM(tmp_frozenset_arg_3, 77, tmp_list_element_3);
        }
        goto list_build_noexception_3;
        // Exception handling pass through code for list_build:
        list_build_exception_3:;
        Py_DECREF(tmp_frozenset_arg_3);
        goto frame_exception_exit_1;
        // Finished with no exception for list_build:
        list_build_noexception_3:;
        tmp_assign_source_14 = PyFrozenSet_New(tmp_frozenset_arg_3);
        Py_DECREF(tmp_frozenset_arg_3);
        if (tmp_assign_source_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 342;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[120], tmp_assign_source_14);
    }
    {
        PyObject *tmp_assign_source_15;
        PyObject *tmp_frozenset_arg_4;
        PyObject *tmp_list_element_4;
        PyObject *tmp_tuple_element_110;
        PyObject *tmp_dict_arg_name_110;
        PyObject *tmp_key_name_110;
        tmp_dict_arg_name_110 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

        if (unlikely(tmp_dict_arg_name_110 == NULL)) {
            tmp_dict_arg_name_110 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
        }

        assert(!(tmp_dict_arg_name_110 == NULL));
        tmp_key_name_110 = mod_consts[25];
        tmp_tuple_element_110 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_110, tmp_key_name_110);
        if (tmp_tuple_element_110 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 426;

            goto frame_exception_exit_1;
        }
        tmp_list_element_4 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_list_element_4, 0, tmp_tuple_element_110);
        tmp_tuple_element_110 = mod_consts[31];
        PyTuple_SET_ITEM0(tmp_list_element_4, 1, tmp_tuple_element_110);
        tmp_frozenset_arg_4 = PyList_New(4);
        {
            PyObject *tmp_tuple_element_111;
            PyObject *tmp_dict_arg_name_111;
            PyObject *tmp_key_name_111;
            PyObject *tmp_tuple_element_112;
            PyObject *tmp_dict_arg_name_112;
            PyObject *tmp_key_name_112;
            PyObject *tmp_tuple_element_113;
            PyObject *tmp_dict_arg_name_113;
            PyObject *tmp_key_name_113;
            PyList_SET_ITEM(tmp_frozenset_arg_4, 0, tmp_list_element_4);
            tmp_dict_arg_name_111 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_111 == NULL)) {
                tmp_dict_arg_name_111 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_111 == NULL));
            tmp_key_name_111 = mod_consts[32];
            tmp_tuple_element_111 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_111, tmp_key_name_111);
            if (tmp_tuple_element_111 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 427;

                goto list_build_exception_4;
            }
            tmp_list_element_4 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_4, 0, tmp_tuple_element_111);
            tmp_tuple_element_111 = mod_consts[33];
            PyTuple_SET_ITEM0(tmp_list_element_4, 1, tmp_tuple_element_111);
            PyList_SET_ITEM(tmp_frozenset_arg_4, 1, tmp_list_element_4);
            tmp_dict_arg_name_112 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_112 == NULL)) {
                tmp_dict_arg_name_112 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_112 == NULL));
            tmp_key_name_112 = mod_consts[32];
            tmp_tuple_element_112 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_112, tmp_key_name_112);
            if (tmp_tuple_element_112 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 428;

                goto list_build_exception_4;
            }
            tmp_list_element_4 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_4, 0, tmp_tuple_element_112);
            tmp_tuple_element_112 = mod_consts[34];
            PyTuple_SET_ITEM0(tmp_list_element_4, 1, tmp_tuple_element_112);
            PyList_SET_ITEM(tmp_frozenset_arg_4, 2, tmp_list_element_4);
            tmp_dict_arg_name_113 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_113 == NULL)) {
                tmp_dict_arg_name_113 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_113 == NULL));
            tmp_key_name_113 = mod_consts[32];
            tmp_tuple_element_113 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_113, tmp_key_name_113);
            if (tmp_tuple_element_113 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 429;

                goto list_build_exception_4;
            }
            tmp_list_element_4 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_4, 0, tmp_tuple_element_113);
            tmp_tuple_element_113 = mod_consts[35];
            PyTuple_SET_ITEM0(tmp_list_element_4, 1, tmp_tuple_element_113);
            PyList_SET_ITEM(tmp_frozenset_arg_4, 3, tmp_list_element_4);
        }
        goto list_build_noexception_4;
        // Exception handling pass through code for list_build:
        list_build_exception_4:;
        Py_DECREF(tmp_frozenset_arg_4);
        goto frame_exception_exit_1;
        // Finished with no exception for list_build:
        list_build_noexception_4:;
        tmp_assign_source_15 = PyFrozenSet_New(tmp_frozenset_arg_4);
        Py_DECREF(tmp_frozenset_arg_4);
        if (tmp_assign_source_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 425;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[121], tmp_assign_source_15);
    }
    {
        PyObject *tmp_assign_source_16;
        PyObject *tmp_frozenset_arg_5;
        PyObject *tmp_list_element_5;
        PyObject *tmp_tuple_element_114;
        PyObject *tmp_dict_arg_name_114;
        PyObject *tmp_key_name_114;
        tmp_dict_arg_name_114 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

        if (unlikely(tmp_dict_arg_name_114 == NULL)) {
            tmp_dict_arg_name_114 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
        }

        assert(!(tmp_dict_arg_name_114 == NULL));
        tmp_key_name_114 = mod_consts[25];
        tmp_tuple_element_114 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_114, tmp_key_name_114);
        if (tmp_tuple_element_114 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 433;

            goto frame_exception_exit_1;
        }
        tmp_list_element_5 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_list_element_5, 0, tmp_tuple_element_114);
        tmp_tuple_element_114 = mod_consts[26];
        PyTuple_SET_ITEM0(tmp_list_element_5, 1, tmp_tuple_element_114);
        tmp_frozenset_arg_5 = PyList_New(5);
        {
            PyObject *tmp_tuple_element_115;
            PyObject *tmp_dict_arg_name_115;
            PyObject *tmp_key_name_115;
            PyObject *tmp_tuple_element_116;
            PyObject *tmp_dict_arg_name_116;
            PyObject *tmp_key_name_116;
            PyObject *tmp_tuple_element_117;
            PyObject *tmp_dict_arg_name_117;
            PyObject *tmp_key_name_117;
            PyObject *tmp_tuple_element_118;
            PyObject *tmp_dict_arg_name_118;
            PyObject *tmp_key_name_118;
            PyList_SET_ITEM(tmp_frozenset_arg_5, 0, tmp_list_element_5);
            tmp_dict_arg_name_115 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_115 == NULL)) {
                tmp_dict_arg_name_115 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_115 == NULL));
            tmp_key_name_115 = mod_consts[25];
            tmp_tuple_element_115 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_115, tmp_key_name_115);
            if (tmp_tuple_element_115 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 434;

                goto list_build_exception_5;
            }
            tmp_list_element_5 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_5, 0, tmp_tuple_element_115);
            tmp_tuple_element_115 = mod_consts[27];
            PyTuple_SET_ITEM0(tmp_list_element_5, 1, tmp_tuple_element_115);
            PyList_SET_ITEM(tmp_frozenset_arg_5, 1, tmp_list_element_5);
            tmp_dict_arg_name_116 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_116 == NULL)) {
                tmp_dict_arg_name_116 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_116 == NULL));
            tmp_key_name_116 = mod_consts[25];
            tmp_tuple_element_116 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_116, tmp_key_name_116);
            if (tmp_tuple_element_116 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 435;

                goto list_build_exception_5;
            }
            tmp_list_element_5 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_5, 0, tmp_tuple_element_116);
            tmp_tuple_element_116 = mod_consts[28];
            PyTuple_SET_ITEM0(tmp_list_element_5, 1, tmp_tuple_element_116);
            PyList_SET_ITEM(tmp_frozenset_arg_5, 2, tmp_list_element_5);
            tmp_dict_arg_name_117 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_117 == NULL)) {
                tmp_dict_arg_name_117 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_117 == NULL));
            tmp_key_name_117 = mod_consts[25];
            tmp_tuple_element_117 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_117, tmp_key_name_117);
            if (tmp_tuple_element_117 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 436;

                goto list_build_exception_5;
            }
            tmp_list_element_5 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_5, 0, tmp_tuple_element_117);
            tmp_tuple_element_117 = mod_consts[29];
            PyTuple_SET_ITEM0(tmp_list_element_5, 1, tmp_tuple_element_117);
            PyList_SET_ITEM(tmp_frozenset_arg_5, 3, tmp_list_element_5);
            tmp_dict_arg_name_118 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_118 == NULL)) {
                tmp_dict_arg_name_118 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_118 == NULL));
            tmp_key_name_118 = mod_consts[25];
            tmp_tuple_element_118 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_118, tmp_key_name_118);
            if (tmp_tuple_element_118 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 437;

                goto list_build_exception_5;
            }
            tmp_list_element_5 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_list_element_5, 0, tmp_tuple_element_118);
            tmp_tuple_element_118 = mod_consts[30];
            PyTuple_SET_ITEM0(tmp_list_element_5, 1, tmp_tuple_element_118);
            PyList_SET_ITEM(tmp_frozenset_arg_5, 4, tmp_list_element_5);
        }
        goto list_build_noexception_5;
        // Exception handling pass through code for list_build:
        list_build_exception_5:;
        Py_DECREF(tmp_frozenset_arg_5);
        goto frame_exception_exit_1;
        // Finished with no exception for list_build:
        list_build_noexception_5:;
        tmp_assign_source_16 = PyFrozenSet_New(tmp_frozenset_arg_5);
        Py_DECREF(tmp_frozenset_arg_5);
        if (tmp_assign_source_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[122], tmp_assign_source_16);
    }
    {
        PyObject *tmp_assign_source_17;
        tmp_assign_source_17 = PyDict_Copy(mod_consts[123]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[124], tmp_assign_source_17);
    }
    {
        PyObject *tmp_assign_source_18;
        tmp_assign_source_18 = PyDict_Copy(mod_consts[125]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[126], tmp_assign_source_18);
    }
    {
        PyObject *tmp_assign_source_19;
        PyObject *tmp_dict_key_1;
        PyObject *tmp_dict_value_1;
        PyObject *tmp_tuple_element_119;
        tmp_dict_key_1 = mod_consts[127];
        tmp_tuple_element_119 = mod_consts[128];
        tmp_dict_value_1 = PyTuple_New(3);
        {
            PyObject *tmp_dict_arg_name_119;
            PyObject *tmp_key_name_119;
            PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_119);
            tmp_tuple_element_119 = mod_consts[129];
            PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_119);
            tmp_dict_arg_name_119 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_dict_arg_name_119 == NULL)) {
                tmp_dict_arg_name_119 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            assert(!(tmp_dict_arg_name_119 == NULL));
            tmp_key_name_119 = mod_consts[128];
            tmp_tuple_element_119 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_119, tmp_key_name_119);
            if (tmp_tuple_element_119 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 508;

                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_119);
        }
        goto tuple_build_noexception_1;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_dict_value_1);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_1:;
        tmp_assign_source_19 = _PyDict_NewPresized( 12 );
        {
            PyObject *tmp_tuple_element_120;
            PyObject *tmp_tuple_element_121;
            PyObject *tmp_tuple_element_122;
            PyObject *tmp_tuple_element_123;
            PyObject *tmp_tuple_element_124;
            PyObject *tmp_tuple_element_125;
            PyObject *tmp_tuple_element_126;
            PyObject *tmp_tuple_element_127;
            PyObject *tmp_tuple_element_128;
            PyObject *tmp_tuple_element_129;
            PyObject *tmp_tuple_element_130;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[130];
            tmp_tuple_element_120 = mod_consts[128];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_120;
                PyObject *tmp_key_name_120;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_120);
                tmp_tuple_element_120 = mod_consts[131];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_120);
                tmp_dict_arg_name_120 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_120 == NULL)) {
                    tmp_dict_arg_name_120 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_120 == NULL));
                tmp_key_name_120 = mod_consts[128];
                tmp_tuple_element_120 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_120, tmp_key_name_120);
                if (tmp_tuple_element_120 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 509;

                    goto tuple_build_exception_2;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_120);
            }
            goto tuple_build_noexception_2;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_2:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_2:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[132];
            tmp_tuple_element_121 = mod_consts[128];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_121;
                PyObject *tmp_key_name_121;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_121);
                tmp_tuple_element_121 = mod_consts[133];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_121);
                tmp_dict_arg_name_121 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_121 == NULL)) {
                    tmp_dict_arg_name_121 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_121 == NULL));
                tmp_key_name_121 = mod_consts[128];
                tmp_tuple_element_121 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_121, tmp_key_name_121);
                if (tmp_tuple_element_121 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 510;

                    goto tuple_build_exception_3;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_121);
            }
            goto tuple_build_noexception_3;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_3:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_3:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[134];
            tmp_tuple_element_122 = mod_consts[128];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_122;
                PyObject *tmp_key_name_122;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_122);
                tmp_tuple_element_122 = mod_consts[135];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_122);
                tmp_dict_arg_name_122 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_122 == NULL)) {
                    tmp_dict_arg_name_122 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_122 == NULL));
                tmp_key_name_122 = mod_consts[128];
                tmp_tuple_element_122 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_122, tmp_key_name_122);
                if (tmp_tuple_element_122 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 511;

                    goto tuple_build_exception_4;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_122);
            }
            goto tuple_build_noexception_4;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_4:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_4:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[136];
            tmp_tuple_element_123 = mod_consts[128];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_123;
                PyObject *tmp_key_name_123;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_123);
                tmp_tuple_element_123 = mod_consts[137];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_123);
                tmp_dict_arg_name_123 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_123 == NULL)) {
                    tmp_dict_arg_name_123 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_123 == NULL));
                tmp_key_name_123 = mod_consts[128];
                tmp_tuple_element_123 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_123, tmp_key_name_123);
                if (tmp_tuple_element_123 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 512;

                    goto tuple_build_exception_5;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_123);
            }
            goto tuple_build_noexception_5;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_5:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_5:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[138];
            tmp_tuple_element_124 = mod_consts[128];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_124;
                PyObject *tmp_key_name_124;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_124);
                tmp_tuple_element_124 = mod_consts[35];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_124);
                tmp_dict_arg_name_124 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_124 == NULL)) {
                    tmp_dict_arg_name_124 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_124 == NULL));
                tmp_key_name_124 = mod_consts[128];
                tmp_tuple_element_124 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_124, tmp_key_name_124);
                if (tmp_tuple_element_124 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 513;

                    goto tuple_build_exception_6;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_124);
            }
            goto tuple_build_noexception_6;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_6:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_6:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[139];
            tmp_tuple_element_125 = mod_consts[128];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_125;
                PyObject *tmp_key_name_125;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_125);
                tmp_tuple_element_125 = mod_consts[140];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_125);
                tmp_dict_arg_name_125 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_125 == NULL)) {
                    tmp_dict_arg_name_125 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_125 == NULL));
                tmp_key_name_125 = mod_consts[128];
                tmp_tuple_element_125 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_125, tmp_key_name_125);
                if (tmp_tuple_element_125 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 514;

                    goto tuple_build_exception_7;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_125);
            }
            goto tuple_build_noexception_7;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_7:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_7:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[141];
            tmp_tuple_element_126 = mod_consts[142];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_126;
                PyObject *tmp_key_name_126;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_126);
                tmp_tuple_element_126 = mod_consts[56];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_126);
                tmp_dict_arg_name_126 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_126 == NULL)) {
                    tmp_dict_arg_name_126 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_126 == NULL));
                tmp_key_name_126 = mod_consts[142];
                tmp_tuple_element_126 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_126, tmp_key_name_126);
                if (tmp_tuple_element_126 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 515;

                    goto tuple_build_exception_8;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_126);
            }
            goto tuple_build_noexception_8;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_8:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_8:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[143];
            tmp_tuple_element_127 = mod_consts[142];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_127;
                PyObject *tmp_key_name_127;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_127);
                tmp_tuple_element_127 = mod_consts[144];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_127);
                tmp_dict_arg_name_127 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_127 == NULL)) {
                    tmp_dict_arg_name_127 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_127 == NULL));
                tmp_key_name_127 = mod_consts[142];
                tmp_tuple_element_127 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_127, tmp_key_name_127);
                if (tmp_tuple_element_127 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 516;

                    goto tuple_build_exception_9;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_127);
            }
            goto tuple_build_noexception_9;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_9:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_9:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[145];
            tmp_tuple_element_128 = mod_consts[142];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_128;
                PyObject *tmp_key_name_128;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_128);
                tmp_tuple_element_128 = mod_consts[146];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_128);
                tmp_dict_arg_name_128 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_128 == NULL)) {
                    tmp_dict_arg_name_128 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_128 == NULL));
                tmp_key_name_128 = mod_consts[142];
                tmp_tuple_element_128 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_128, tmp_key_name_128);
                if (tmp_tuple_element_128 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 517;

                    goto tuple_build_exception_10;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_128);
            }
            goto tuple_build_noexception_10;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_10:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_10:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[147];
            tmp_tuple_element_129 = Py_None;
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_129;
                PyObject *tmp_key_name_129;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_129);
                tmp_tuple_element_129 = mod_consts[147];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_129);
                tmp_dict_arg_name_129 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_129 == NULL)) {
                    tmp_dict_arg_name_129 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_129 == NULL));
                tmp_key_name_129 = mod_consts[147];
                tmp_tuple_element_129 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_129, tmp_key_name_129);
                if (tmp_tuple_element_129 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 518;

                    goto tuple_build_exception_11;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_129);
            }
            goto tuple_build_noexception_11;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_11:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_11:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
            tmp_dict_key_1 = mod_consts[148];
            tmp_tuple_element_130 = mod_consts[147];
            tmp_dict_value_1 = PyTuple_New(3);
            {
                PyObject *tmp_dict_arg_name_130;
                PyObject *tmp_key_name_130;
                PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_130);
                tmp_tuple_element_130 = mod_consts[128];
                PyTuple_SET_ITEM0(tmp_dict_value_1, 1, tmp_tuple_element_130);
                tmp_dict_arg_name_130 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

                if (unlikely(tmp_dict_arg_name_130 == NULL)) {
                    tmp_dict_arg_name_130 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
                }

                assert(!(tmp_dict_arg_name_130 == NULL));
                tmp_key_name_130 = mod_consts[147];
                tmp_tuple_element_130 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_130, tmp_key_name_130);
                if (tmp_tuple_element_130 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 519;

                    goto tuple_build_exception_12;
                }
                PyTuple_SET_ITEM(tmp_dict_value_1, 2, tmp_tuple_element_130);
            }
            goto tuple_build_noexception_12;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_12:;
            Py_DECREF(tmp_dict_value_1);
            goto dict_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_12:;
            tmp_res = PyDict_SetItem(tmp_assign_source_19, tmp_dict_key_1, tmp_dict_value_1);
            Py_DECREF(tmp_dict_value_1);
            assert(!(tmp_res != 0));
        }
        goto dict_build_noexception_1;
        // Exception handling pass through code for dict_build:
        dict_build_exception_1:;
        Py_DECREF(tmp_assign_source_19);
        goto frame_exception_exit_1;
        // Finished with no exception for dict_build:
        dict_build_noexception_1:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[149], tmp_assign_source_19);
    }
    {
        PyObject *tmp_assign_source_20;
        // Tried code:
        {
            PyObject *tmp_assign_source_21;
            PyObject *tmp_iter_arg_1;
            PyObject *tmp_called_instance_1;
            tmp_called_instance_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[149]);

            if (unlikely(tmp_called_instance_1 == NULL)) {
                tmp_called_instance_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[149]);
            }

            assert(!(tmp_called_instance_1 == NULL));
            frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 523;
            tmp_iter_arg_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_1, mod_consts[150]);
            if (tmp_iter_arg_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 523;

                goto try_except_handler_2;
            }
            tmp_assign_source_21 = MAKE_ITERATOR(tmp_iter_arg_1);
            Py_DECREF(tmp_iter_arg_1);
            if (tmp_assign_source_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 522;

                goto try_except_handler_2;
            }
            assert(tmp_dictcontraction_1__$0 == NULL);
            tmp_dictcontraction_1__$0 = tmp_assign_source_21;
        }
        {
            PyObject *tmp_assign_source_22;
            tmp_assign_source_22 = PyDict_New();
            assert(tmp_dictcontraction_1__contraction == NULL);
            tmp_dictcontraction_1__contraction = tmp_assign_source_22;
        }
        if (isFrameUnusable(cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2)) {
            Py_XDECREF(cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2);

#if _DEBUG_REFCOUNTS
            if (cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2 = MAKE_FUNCTION_FRAME(codeobj_ca4c1f39d7300194360d6f48084cfe9e, module_pip$_vendor$html5lib$constants, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2->m_type_description == NULL);
        frame_ca4c1f39d7300194360d6f48084cfe9e_2 = cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_ca4c1f39d7300194360d6f48084cfe9e_2);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_ca4c1f39d7300194360d6f48084cfe9e_2) == 2); // Frame stack

        // Framed code:
        // Tried code:
        loop_start_1:;
        {
            PyObject *tmp_next_source_1;
            PyObject *tmp_assign_source_23;
            CHECK_OBJECT(tmp_dictcontraction_1__$0);
            tmp_next_source_1 = tmp_dictcontraction_1__$0;
            tmp_assign_source_23 = ITERATOR_NEXT(tmp_next_source_1);
            if (tmp_assign_source_23 == NULL) {
                if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                    goto loop_end_1;
                } else {

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                    type_description_2 = "oooo";
                    exception_lineno = 522;
                    goto try_except_handler_3;
                }
            }

            {
                PyObject *old = tmp_dictcontraction_1__iter_value_0;
                tmp_dictcontraction_1__iter_value_0 = tmp_assign_source_23;
                Py_XDECREF(old);
            }

        }
        // Tried code:
        {
            PyObject *tmp_assign_source_24;
            PyObject *tmp_iter_arg_2;
            CHECK_OBJECT(tmp_dictcontraction_1__iter_value_0);
            tmp_iter_arg_2 = tmp_dictcontraction_1__iter_value_0;
            tmp_assign_source_24 = MAKE_ITERATOR(tmp_iter_arg_2);
            if (tmp_assign_source_24 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 522;
                type_description_2 = "oooo";
                goto try_except_handler_4;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_1__source_iter;
                tmp_dictcontraction$tuple_unpack_1__source_iter = tmp_assign_source_24;
                Py_XDECREF(old);
            }

        }
        // Tried code:
        {
            PyObject *tmp_assign_source_25;
            PyObject *tmp_unpack_1;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_1__source_iter);
            tmp_unpack_1 = tmp_dictcontraction$tuple_unpack_1__source_iter;
            tmp_assign_source_25 = UNPACK_NEXT(tmp_unpack_1, 0, 2);
            if (tmp_assign_source_25 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "oooo";
                exception_lineno = 522;
                goto try_except_handler_5;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_1__element_1;
                tmp_dictcontraction$tuple_unpack_1__element_1 = tmp_assign_source_25;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_assign_source_26;
            PyObject *tmp_unpack_2;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_1__source_iter);
            tmp_unpack_2 = tmp_dictcontraction$tuple_unpack_1__source_iter;
            tmp_assign_source_26 = UNPACK_NEXT(tmp_unpack_2, 1, 2);
            if (tmp_assign_source_26 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "oooo";
                exception_lineno = 522;
                goto try_except_handler_5;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_1__element_2;
                tmp_dictcontraction$tuple_unpack_1__element_2 = tmp_assign_source_26;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_iterator_name_1;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_1__source_iter);
            tmp_iterator_name_1 = tmp_dictcontraction$tuple_unpack_1__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_1); assert(HAS_ITERNEXT(tmp_iterator_name_1));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_1)->tp_iternext)(tmp_iterator_name_1);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "oooo";
                        exception_lineno = 522;
                        goto try_except_handler_5;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[151];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "oooo";
                exception_lineno = 522;
                goto try_except_handler_5;
            }
        }
        goto try_end_2;
        // Exception handler code:
        try_except_handler_5:;
        exception_keeper_type_2 = exception_type;
        exception_keeper_value_2 = exception_value;
        exception_keeper_tb_2 = exception_tb;
        exception_keeper_lineno_2 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__source_iter);
        tmp_dictcontraction$tuple_unpack_1__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_2;
        exception_value = exception_keeper_value_2;
        exception_tb = exception_keeper_tb_2;
        exception_lineno = exception_keeper_lineno_2;

        goto try_except_handler_4;
        // End of try:
        try_end_2:;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__source_iter);
        tmp_dictcontraction$tuple_unpack_1__source_iter = NULL;
        {
            PyObject *tmp_assign_source_27;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_1__element_1);
            tmp_assign_source_27 = tmp_dictcontraction$tuple_unpack_1__element_1;
            {
                PyObject *old = outline_0_var_qname;
                outline_0_var_qname = tmp_assign_source_27;
                Py_INCREF(outline_0_var_qname);
                Py_XDECREF(old);
            }

        }
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__element_1);
        tmp_dictcontraction$tuple_unpack_1__element_1 = NULL;

        // Tried code:
        {
            PyObject *tmp_assign_source_28;
            PyObject *tmp_iter_arg_3;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_1__element_2);
            tmp_iter_arg_3 = tmp_dictcontraction$tuple_unpack_1__element_2;
            tmp_assign_source_28 = MAKE_ITERATOR(tmp_iter_arg_3);
            if (tmp_assign_source_28 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 522;
                type_description_2 = "oooo";
                goto try_except_handler_6;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_2__source_iter;
                tmp_dictcontraction$tuple_unpack_2__source_iter = tmp_assign_source_28;
                Py_XDECREF(old);
            }

        }
        // Tried code:
        {
            PyObject *tmp_assign_source_29;
            PyObject *tmp_unpack_3;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_2__source_iter);
            tmp_unpack_3 = tmp_dictcontraction$tuple_unpack_2__source_iter;
            tmp_assign_source_29 = UNPACK_NEXT(tmp_unpack_3, 0, 3);
            if (tmp_assign_source_29 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "oooo";
                exception_lineno = 522;
                goto try_except_handler_7;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_2__element_1;
                tmp_dictcontraction$tuple_unpack_2__element_1 = tmp_assign_source_29;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_assign_source_30;
            PyObject *tmp_unpack_4;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_2__source_iter);
            tmp_unpack_4 = tmp_dictcontraction$tuple_unpack_2__source_iter;
            tmp_assign_source_30 = UNPACK_NEXT(tmp_unpack_4, 1, 3);
            if (tmp_assign_source_30 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "oooo";
                exception_lineno = 522;
                goto try_except_handler_7;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_2__element_2;
                tmp_dictcontraction$tuple_unpack_2__element_2 = tmp_assign_source_30;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_assign_source_31;
            PyObject *tmp_unpack_5;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_2__source_iter);
            tmp_unpack_5 = tmp_dictcontraction$tuple_unpack_2__source_iter;
            tmp_assign_source_31 = UNPACK_NEXT(tmp_unpack_5, 2, 3);
            if (tmp_assign_source_31 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "oooo";
                exception_lineno = 522;
                goto try_except_handler_7;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_2__element_3;
                tmp_dictcontraction$tuple_unpack_2__element_3 = tmp_assign_source_31;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_iterator_name_2;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_2__source_iter);
            tmp_iterator_name_2 = tmp_dictcontraction$tuple_unpack_2__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_2); assert(HAS_ITERNEXT(tmp_iterator_name_2));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_2)->tp_iternext)(tmp_iterator_name_2);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "oooo";
                        exception_lineno = 522;
                        goto try_except_handler_7;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[152];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "oooo";
                exception_lineno = 522;
                goto try_except_handler_7;
            }
        }
        goto try_end_3;
        // Exception handler code:
        try_except_handler_7:;
        exception_keeper_type_3 = exception_type;
        exception_keeper_value_3 = exception_value;
        exception_keeper_tb_3 = exception_tb;
        exception_keeper_lineno_3 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__source_iter);
        tmp_dictcontraction$tuple_unpack_2__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_3;
        exception_value = exception_keeper_value_3;
        exception_tb = exception_keeper_tb_3;
        exception_lineno = exception_keeper_lineno_3;

        goto try_except_handler_6;
        // End of try:
        try_end_3:;
        goto try_end_4;
        // Exception handler code:
        try_except_handler_6:;
        exception_keeper_type_4 = exception_type;
        exception_keeper_value_4 = exception_value;
        exception_keeper_tb_4 = exception_tb;
        exception_keeper_lineno_4 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_1);
        tmp_dictcontraction$tuple_unpack_2__element_1 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_2);
        tmp_dictcontraction$tuple_unpack_2__element_2 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_3);
        tmp_dictcontraction$tuple_unpack_2__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_4;
        exception_value = exception_keeper_value_4;
        exception_tb = exception_keeper_tb_4;
        exception_lineno = exception_keeper_lineno_4;

        goto try_except_handler_4;
        // End of try:
        try_end_4:;
        goto try_end_5;
        // Exception handler code:
        try_except_handler_4:;
        exception_keeper_type_5 = exception_type;
        exception_keeper_value_5 = exception_value;
        exception_keeper_tb_5 = exception_tb;
        exception_keeper_lineno_5 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__element_1);
        tmp_dictcontraction$tuple_unpack_1__element_1 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__element_2);
        tmp_dictcontraction$tuple_unpack_1__element_2 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_5;
        exception_value = exception_keeper_value_5;
        exception_tb = exception_keeper_tb_5;
        exception_lineno = exception_keeper_lineno_5;

        goto try_except_handler_3;
        // End of try:
        try_end_5:;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__source_iter);
        tmp_dictcontraction$tuple_unpack_2__source_iter = NULL;
        {
            PyObject *tmp_assign_source_32;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_2__element_1);
            tmp_assign_source_32 = tmp_dictcontraction$tuple_unpack_2__element_1;
            {
                PyObject *old = outline_0_var_prefix;
                outline_0_var_prefix = tmp_assign_source_32;
                Py_INCREF(outline_0_var_prefix);
                Py_XDECREF(old);
            }

        }
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_1);
        tmp_dictcontraction$tuple_unpack_2__element_1 = NULL;

        {
            PyObject *tmp_assign_source_33;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_2__element_2);
            tmp_assign_source_33 = tmp_dictcontraction$tuple_unpack_2__element_2;
            {
                PyObject *old = outline_0_var_local;
                outline_0_var_local = tmp_assign_source_33;
                Py_INCREF(outline_0_var_local);
                Py_XDECREF(old);
            }

        }
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_2);
        tmp_dictcontraction$tuple_unpack_2__element_2 = NULL;

        {
            PyObject *tmp_assign_source_34;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_2__element_3);
            tmp_assign_source_34 = tmp_dictcontraction$tuple_unpack_2__element_3;
            {
                PyObject *old = outline_0_var_ns;
                outline_0_var_ns = tmp_assign_source_34;
                Py_INCREF(outline_0_var_ns);
                Py_XDECREF(old);
            }

        }
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_3);
        tmp_dictcontraction$tuple_unpack_2__element_3 = NULL;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_1);
        tmp_dictcontraction$tuple_unpack_2__element_1 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_2);
        tmp_dictcontraction$tuple_unpack_2__element_2 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_2__element_3);
        tmp_dictcontraction$tuple_unpack_2__element_3 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__element_2);
        tmp_dictcontraction$tuple_unpack_1__element_2 = NULL;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__element_1);
        tmp_dictcontraction$tuple_unpack_1__element_1 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_1__element_2);
        tmp_dictcontraction$tuple_unpack_1__element_2 = NULL;
        {
            PyObject *tmp_tuple_element_131;
            CHECK_OBJECT(outline_0_var_qname);
            tmp_dictset_value = outline_0_var_qname;
            CHECK_OBJECT(tmp_dictcontraction_1__contraction);
            tmp_dictset_dict = tmp_dictcontraction_1__contraction;
            CHECK_OBJECT(outline_0_var_ns);
            tmp_tuple_element_131 = outline_0_var_ns;
            tmp_dictset_key = PyTuple_New(2);
            PyTuple_SET_ITEM0(tmp_dictset_key, 0, tmp_tuple_element_131);
            CHECK_OBJECT(outline_0_var_local);
            tmp_tuple_element_131 = outline_0_var_local;
            PyTuple_SET_ITEM0(tmp_dictset_key, 1, tmp_tuple_element_131);
            tmp_res = PyDict_SetItem(tmp_dictset_dict, tmp_dictset_key, tmp_dictset_value);
            Py_DECREF(tmp_dictset_key);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 522;
                type_description_2 = "oooo";
                goto try_except_handler_3;
            }
        }
        if (CONSIDER_THREADING() == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 522;
            type_description_2 = "oooo";
            goto try_except_handler_3;
        }
        goto loop_start_1;
        loop_end_1:;
        CHECK_OBJECT(tmp_dictcontraction_1__contraction);
        tmp_assign_source_20 = tmp_dictcontraction_1__contraction;
        Py_INCREF(tmp_assign_source_20);
        goto try_return_handler_3;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_3:;
        Py_XDECREF(tmp_dictcontraction_1__$0);
        tmp_dictcontraction_1__$0 = NULL;
        CHECK_OBJECT(tmp_dictcontraction_1__contraction);
        Py_DECREF(tmp_dictcontraction_1__contraction);
        tmp_dictcontraction_1__contraction = NULL;
        Py_XDECREF(tmp_dictcontraction_1__iter_value_0);
        tmp_dictcontraction_1__iter_value_0 = NULL;
        goto frame_return_exit_1;
        // Exception handler code:
        try_except_handler_3:;
        exception_keeper_type_6 = exception_type;
        exception_keeper_value_6 = exception_value;
        exception_keeper_tb_6 = exception_tb;
        exception_keeper_lineno_6 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction_1__$0);
        tmp_dictcontraction_1__$0 = NULL;
        CHECK_OBJECT(tmp_dictcontraction_1__contraction);
        Py_DECREF(tmp_dictcontraction_1__contraction);
        tmp_dictcontraction_1__contraction = NULL;
        Py_XDECREF(tmp_dictcontraction_1__iter_value_0);
        tmp_dictcontraction_1__iter_value_0 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_6;
        exception_value = exception_keeper_value_6;
        exception_tb = exception_keeper_tb_6;
        exception_lineno = exception_keeper_lineno_6;

        goto frame_exception_exit_2;
        // End of try:

#if 0
        RESTORE_FRAME_EXCEPTION(frame_ca4c1f39d7300194360d6f48084cfe9e_2);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_1;

        frame_return_exit_1:;
#if 0
        RESTORE_FRAME_EXCEPTION(frame_ca4c1f39d7300194360d6f48084cfe9e_2);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto try_return_handler_2;

        frame_exception_exit_2:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_ca4c1f39d7300194360d6f48084cfe9e_2);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_ca4c1f39d7300194360d6f48084cfe9e_2, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_ca4c1f39d7300194360d6f48084cfe9e_2->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_ca4c1f39d7300194360d6f48084cfe9e_2, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_ca4c1f39d7300194360d6f48084cfe9e_2,
            type_description_2,
            outline_0_var_qname,
            outline_0_var_prefix,
            outline_0_var_local,
            outline_0_var_ns
        );


        // Release cached frame if used for exception.
        if (frame_ca4c1f39d7300194360d6f48084cfe9e_2 == cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2);
            cache_frame_ca4c1f39d7300194360d6f48084cfe9e_2 = NULL;
        }

        assertFrameObject(frame_ca4c1f39d7300194360d6f48084cfe9e_2);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_1;

        frame_no_exception_1:;
        goto skip_nested_handling_1;
        nested_frame_exit_1:;

        goto try_except_handler_2;
        skip_nested_handling_1:;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_2:;
        Py_XDECREF(outline_0_var_qname);
        outline_0_var_qname = NULL;
        Py_XDECREF(outline_0_var_prefix);
        outline_0_var_prefix = NULL;
        Py_XDECREF(outline_0_var_local);
        outline_0_var_local = NULL;
        Py_XDECREF(outline_0_var_ns);
        outline_0_var_ns = NULL;
        goto outline_result_1;
        // Exception handler code:
        try_except_handler_2:;
        exception_keeper_type_7 = exception_type;
        exception_keeper_value_7 = exception_value;
        exception_keeper_tb_7 = exception_tb;
        exception_keeper_lineno_7 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(outline_0_var_qname);
        outline_0_var_qname = NULL;
        Py_XDECREF(outline_0_var_prefix);
        outline_0_var_prefix = NULL;
        Py_XDECREF(outline_0_var_local);
        outline_0_var_local = NULL;
        Py_XDECREF(outline_0_var_ns);
        outline_0_var_ns = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_7;
        exception_value = exception_keeper_value_7;
        exception_tb = exception_keeper_tb_7;
        exception_lineno = exception_keeper_lineno_7;

        goto outline_exception_1;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_1:;
        exception_lineno = 522;
        goto frame_exception_exit_1;
        outline_result_1:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[153], tmp_assign_source_20);
    }
    {
        PyObject *tmp_assign_source_35;
        tmp_assign_source_35 = mod_consts[154];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[155], tmp_assign_source_35);
    }
    {
        PyObject *tmp_assign_source_36;
        tmp_assign_source_36 = mod_consts[156];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[157], tmp_assign_source_36);
    }
    {
        PyObject *tmp_assign_source_37;
        PyObject *tmp_frozenset_arg_6;
        PyObject *tmp_expression_name_1;
        tmp_expression_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[11]);

        if (unlikely(tmp_expression_name_1 == NULL)) {
            tmp_expression_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[11]);
        }

        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 541;

            goto frame_exception_exit_1;
        }
        tmp_frozenset_arg_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[158]);
        if (tmp_frozenset_arg_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 541;

            goto frame_exception_exit_1;
        }
        tmp_assign_source_37 = PyFrozenSet_New(tmp_frozenset_arg_6);
        Py_DECREF(tmp_frozenset_arg_6);
        if (tmp_assign_source_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 541;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[159], tmp_assign_source_37);
    }
    {
        PyObject *tmp_assign_source_38;
        PyObject *tmp_frozenset_arg_7;
        PyObject *tmp_expression_name_2;
        tmp_expression_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[11]);

        if (unlikely(tmp_expression_name_2 == NULL)) {
            tmp_expression_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[11]);
        }

        if (tmp_expression_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 542;

            goto frame_exception_exit_1;
        }
        tmp_frozenset_arg_7 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[160]);
        if (tmp_frozenset_arg_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 542;

            goto frame_exception_exit_1;
        }
        tmp_assign_source_38 = PyFrozenSet_New(tmp_frozenset_arg_7);
        Py_DECREF(tmp_frozenset_arg_7);
        if (tmp_assign_source_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 542;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[161], tmp_assign_source_38);
    }
    {
        PyObject *tmp_assign_source_39;
        PyObject *tmp_frozenset_arg_8;
        PyObject *tmp_expression_name_3;
        tmp_expression_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[11]);

        if (unlikely(tmp_expression_name_3 == NULL)) {
            tmp_expression_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[11]);
        }

        if (tmp_expression_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 543;

            goto frame_exception_exit_1;
        }
        tmp_frozenset_arg_8 = LOOKUP_ATTRIBUTE(tmp_expression_name_3, mod_consts[162]);
        if (tmp_frozenset_arg_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 543;

            goto frame_exception_exit_1;
        }
        tmp_assign_source_39 = PyFrozenSet_New(tmp_frozenset_arg_8);
        Py_DECREF(tmp_frozenset_arg_8);
        if (tmp_assign_source_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 543;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[163], tmp_assign_source_39);
    }
    {
        PyObject *tmp_assign_source_40;
        PyObject *tmp_frozenset_arg_9;
        PyObject *tmp_expression_name_4;
        tmp_expression_name_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[11]);

        if (unlikely(tmp_expression_name_4 == NULL)) {
            tmp_expression_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[11]);
        }

        if (tmp_expression_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 544;

            goto frame_exception_exit_1;
        }
        tmp_frozenset_arg_9 = LOOKUP_ATTRIBUTE(tmp_expression_name_4, mod_consts[164]);
        if (tmp_frozenset_arg_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 544;

            goto frame_exception_exit_1;
        }
        tmp_assign_source_40 = PyFrozenSet_New(tmp_frozenset_arg_9);
        Py_DECREF(tmp_frozenset_arg_9);
        if (tmp_assign_source_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 544;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[164], tmp_assign_source_40);
    }
    {
        PyObject *tmp_assign_source_41;
        PyObject *tmp_frozenset_arg_10;
        PyObject *tmp_expression_name_5;
        tmp_expression_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[11]);

        if (unlikely(tmp_expression_name_5 == NULL)) {
            tmp_expression_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[11]);
        }

        if (tmp_expression_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 545;

            goto frame_exception_exit_1;
        }
        tmp_frozenset_arg_10 = LOOKUP_ATTRIBUTE(tmp_expression_name_5, mod_consts[165]);
        if (tmp_frozenset_arg_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 545;

            goto frame_exception_exit_1;
        }
        tmp_assign_source_41 = PyFrozenSet_New(tmp_frozenset_arg_10);
        Py_DECREF(tmp_frozenset_arg_10);
        if (tmp_assign_source_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 545;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[166], tmp_assign_source_41);
    }
    {
        PyObject *tmp_assign_source_42;
        // Tried code:
        {
            PyObject *tmp_assign_source_43;
            PyObject *tmp_iter_arg_4;
            PyObject *tmp_expression_name_6;
            tmp_expression_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[11]);

            if (unlikely(tmp_expression_name_6 == NULL)) {
                tmp_expression_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[11]);
            }

            if (tmp_expression_name_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 547;

                goto try_except_handler_8;
            }
            tmp_iter_arg_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_6, mod_consts[160]);
            if (tmp_iter_arg_4 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 547;

                goto try_except_handler_8;
            }
            tmp_assign_source_43 = MAKE_ITERATOR(tmp_iter_arg_4);
            Py_DECREF(tmp_iter_arg_4);
            if (tmp_assign_source_43 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 547;

                goto try_except_handler_8;
            }
            assert(tmp_dictcontraction_2__$0 == NULL);
            tmp_dictcontraction_2__$0 = tmp_assign_source_43;
        }
        {
            PyObject *tmp_assign_source_44;
            tmp_assign_source_44 = PyDict_New();
            assert(tmp_dictcontraction_2__contraction == NULL);
            tmp_dictcontraction_2__contraction = tmp_assign_source_44;
        }
        if (isFrameUnusable(cache_frame_26c5d084b8dc3930bde29c9decd1859d_3)) {
            Py_XDECREF(cache_frame_26c5d084b8dc3930bde29c9decd1859d_3);

#if _DEBUG_REFCOUNTS
            if (cache_frame_26c5d084b8dc3930bde29c9decd1859d_3 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_26c5d084b8dc3930bde29c9decd1859d_3 = MAKE_FUNCTION_FRAME(codeobj_26c5d084b8dc3930bde29c9decd1859d, module_pip$_vendor$html5lib$constants, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_26c5d084b8dc3930bde29c9decd1859d_3->m_type_description == NULL);
        frame_26c5d084b8dc3930bde29c9decd1859d_3 = cache_frame_26c5d084b8dc3930bde29c9decd1859d_3;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_26c5d084b8dc3930bde29c9decd1859d_3);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_26c5d084b8dc3930bde29c9decd1859d_3) == 2); // Frame stack

        // Framed code:
        // Tried code:
        loop_start_2:;
        {
            PyObject *tmp_next_source_2;
            PyObject *tmp_assign_source_45;
            CHECK_OBJECT(tmp_dictcontraction_2__$0);
            tmp_next_source_2 = tmp_dictcontraction_2__$0;
            tmp_assign_source_45 = ITERATOR_NEXT(tmp_next_source_2);
            if (tmp_assign_source_45 == NULL) {
                if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                    goto loop_end_2;
                } else {

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                    type_description_2 = "o";
                    exception_lineno = 547;
                    goto try_except_handler_9;
                }
            }

            {
                PyObject *old = tmp_dictcontraction_2__iter_value_0;
                tmp_dictcontraction_2__iter_value_0 = tmp_assign_source_45;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_assign_source_46;
            CHECK_OBJECT(tmp_dictcontraction_2__iter_value_0);
            tmp_assign_source_46 = tmp_dictcontraction_2__iter_value_0;
            {
                PyObject *old = outline_1_var_c;
                outline_1_var_c = tmp_assign_source_46;
                Py_INCREF(outline_1_var_c);
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_ord_arg_1;
            PyObject *tmp_called_instance_2;
            PyObject *tmp_ord_arg_2;
            CHECK_OBJECT(outline_1_var_c);
            tmp_called_instance_2 = outline_1_var_c;
            frame_26c5d084b8dc3930bde29c9decd1859d_3->m_frame.f_lineno = 547;
            tmp_ord_arg_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_2, mod_consts[167]);
            if (tmp_ord_arg_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 547;
                type_description_2 = "o";
                goto try_except_handler_9;
            }
            tmp_dictset_value = BUILTIN_ORD(tmp_ord_arg_1);
            Py_DECREF(tmp_ord_arg_1);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 547;
                type_description_2 = "o";
                goto try_except_handler_9;
            }
            CHECK_OBJECT(tmp_dictcontraction_2__contraction);
            tmp_dictset_dict = tmp_dictcontraction_2__contraction;
            if (outline_1_var_c == NULL) {
                Py_DECREF(tmp_dictset_value);
                FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[168]);
                exception_tb = NULL;
                NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
                CHAIN_EXCEPTION(exception_value);

                exception_lineno = 547;
                type_description_2 = "o";
                goto try_except_handler_9;
            }

            tmp_ord_arg_2 = outline_1_var_c;
            tmp_dictset_key = BUILTIN_ORD(tmp_ord_arg_2);
            if (tmp_dictset_key == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_dictset_value);

                exception_lineno = 547;
                type_description_2 = "o";
                goto try_except_handler_9;
            }
            tmp_res = PyDict_SetItem(tmp_dictset_dict, tmp_dictset_key, tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            Py_DECREF(tmp_dictset_key);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 547;
                type_description_2 = "o";
                goto try_except_handler_9;
            }
        }
        if (CONSIDER_THREADING() == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 547;
            type_description_2 = "o";
            goto try_except_handler_9;
        }
        goto loop_start_2;
        loop_end_2:;
        CHECK_OBJECT(tmp_dictcontraction_2__contraction);
        tmp_assign_source_42 = tmp_dictcontraction_2__contraction;
        Py_INCREF(tmp_assign_source_42);
        goto try_return_handler_9;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_9:;
        Py_XDECREF(tmp_dictcontraction_2__$0);
        tmp_dictcontraction_2__$0 = NULL;
        CHECK_OBJECT(tmp_dictcontraction_2__contraction);
        Py_DECREF(tmp_dictcontraction_2__contraction);
        tmp_dictcontraction_2__contraction = NULL;
        Py_XDECREF(tmp_dictcontraction_2__iter_value_0);
        tmp_dictcontraction_2__iter_value_0 = NULL;
        goto frame_return_exit_2;
        // Exception handler code:
        try_except_handler_9:;
        exception_keeper_type_8 = exception_type;
        exception_keeper_value_8 = exception_value;
        exception_keeper_tb_8 = exception_tb;
        exception_keeper_lineno_8 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction_2__$0);
        tmp_dictcontraction_2__$0 = NULL;
        CHECK_OBJECT(tmp_dictcontraction_2__contraction);
        Py_DECREF(tmp_dictcontraction_2__contraction);
        tmp_dictcontraction_2__contraction = NULL;
        Py_XDECREF(tmp_dictcontraction_2__iter_value_0);
        tmp_dictcontraction_2__iter_value_0 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_8;
        exception_value = exception_keeper_value_8;
        exception_tb = exception_keeper_tb_8;
        exception_lineno = exception_keeper_lineno_8;

        goto frame_exception_exit_3;
        // End of try:

#if 0
        RESTORE_FRAME_EXCEPTION(frame_26c5d084b8dc3930bde29c9decd1859d_3);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_2;

        frame_return_exit_2:;
#if 0
        RESTORE_FRAME_EXCEPTION(frame_26c5d084b8dc3930bde29c9decd1859d_3);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto try_return_handler_8;

        frame_exception_exit_3:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_26c5d084b8dc3930bde29c9decd1859d_3);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_26c5d084b8dc3930bde29c9decd1859d_3, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_26c5d084b8dc3930bde29c9decd1859d_3->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_26c5d084b8dc3930bde29c9decd1859d_3, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_26c5d084b8dc3930bde29c9decd1859d_3,
            type_description_2,
            outline_1_var_c
        );


        // Release cached frame if used for exception.
        if (frame_26c5d084b8dc3930bde29c9decd1859d_3 == cache_frame_26c5d084b8dc3930bde29c9decd1859d_3) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_26c5d084b8dc3930bde29c9decd1859d_3);
            cache_frame_26c5d084b8dc3930bde29c9decd1859d_3 = NULL;
        }

        assertFrameObject(frame_26c5d084b8dc3930bde29c9decd1859d_3);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_2;

        frame_no_exception_2:;
        goto skip_nested_handling_2;
        nested_frame_exit_2:;

        goto try_except_handler_8;
        skip_nested_handling_2:;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_8:;
        Py_XDECREF(outline_1_var_c);
        outline_1_var_c = NULL;
        goto outline_result_2;
        // Exception handler code:
        try_except_handler_8:;
        exception_keeper_type_9 = exception_type;
        exception_keeper_value_9 = exception_value;
        exception_keeper_tb_9 = exception_tb;
        exception_keeper_lineno_9 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(outline_1_var_c);
        outline_1_var_c = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_9;
        exception_value = exception_keeper_value_9;
        exception_tb = exception_keeper_tb_9;
        exception_lineno = exception_keeper_lineno_9;

        goto outline_exception_2;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_2:;
        exception_lineno = 547;
        goto frame_exception_exit_1;
        outline_result_2:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[169], tmp_assign_source_42);
    }
    {
        PyObject *tmp_assign_source_47;
        tmp_assign_source_47 = mod_consts[170];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[171], tmp_assign_source_47);
    }
    {
        PyObject *tmp_assign_source_48;
        tmp_assign_source_48 = mod_consts[172];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[173], tmp_assign_source_48);
    }
    {
        PyObject *tmp_assign_source_49;
        tmp_assign_source_49 = mod_consts[174];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[175], tmp_assign_source_49);
    }
    {
        PyObject *tmp_assign_source_50;
        tmp_assign_source_50 = mod_consts[176];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[177], tmp_assign_source_50);
    }
    {
        PyObject *tmp_assign_source_51;
        tmp_assign_source_51 = PyDict_Copy(mod_consts[178]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[179], tmp_assign_source_51);
    }
    {
        PyObject *tmp_assign_source_52;
        tmp_assign_source_52 = mod_consts[180];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[181], tmp_assign_source_52);
    }
    {
        PyObject *tmp_assign_source_53;
        tmp_assign_source_53 = mod_consts[182];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[183], tmp_assign_source_53);
    }
    {
        PyObject *tmp_assign_source_54;
        tmp_assign_source_54 = PyDict_Copy(mod_consts[184]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[185], tmp_assign_source_54);
    }
    {
        PyObject *tmp_assign_source_55;
        tmp_assign_source_55 = PyDict_Copy(mod_consts[186]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[187], tmp_assign_source_55);
    }
    {
        PyObject *tmp_assign_source_56;
        tmp_assign_source_56 = PyDict_Copy(mod_consts[188]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[189], tmp_assign_source_56);
    }
    {
        PyObject *tmp_assign_source_57;
        PyObject *tmp_frozenset_arg_11;
        PyObject *tmp_list_element_6;
        PyObject *tmp_dict_arg_name_131;
        PyObject *tmp_key_name_131;
        tmp_dict_arg_name_131 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[189]);

        if (unlikely(tmp_dict_arg_name_131 == NULL)) {
            tmp_dict_arg_name_131 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[189]);
        }

        assert(!(tmp_dict_arg_name_131 == NULL));
        tmp_key_name_131 = mod_consts[190];
        tmp_list_element_6 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_131, tmp_key_name_131);
        if (tmp_list_element_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2932;

            goto frame_exception_exit_1;
        }
        tmp_frozenset_arg_11 = PyList_New(3);
        {
            PyObject *tmp_dict_arg_name_132;
            PyObject *tmp_key_name_132;
            PyObject *tmp_dict_arg_name_133;
            PyObject *tmp_key_name_133;
            PyList_SET_ITEM(tmp_frozenset_arg_11, 0, tmp_list_element_6);
            tmp_dict_arg_name_132 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[189]);

            if (unlikely(tmp_dict_arg_name_132 == NULL)) {
                tmp_dict_arg_name_132 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[189]);
            }

            assert(!(tmp_dict_arg_name_132 == NULL));
            tmp_key_name_132 = mod_consts[191];
            tmp_list_element_6 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_132, tmp_key_name_132);
            if (tmp_list_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2932;

                goto list_build_exception_6;
            }
            PyList_SET_ITEM(tmp_frozenset_arg_11, 1, tmp_list_element_6);
            tmp_dict_arg_name_133 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[189]);

            if (unlikely(tmp_dict_arg_name_133 == NULL)) {
                tmp_dict_arg_name_133 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[189]);
            }

            assert(!(tmp_dict_arg_name_133 == NULL));
            tmp_key_name_133 = mod_consts[192];
            tmp_list_element_6 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_133, tmp_key_name_133);
            if (tmp_list_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2933;

                goto list_build_exception_6;
            }
            PyList_SET_ITEM(tmp_frozenset_arg_11, 2, tmp_list_element_6);
        }
        goto list_build_noexception_6;
        // Exception handling pass through code for list_build:
        list_build_exception_6:;
        Py_DECREF(tmp_frozenset_arg_11);
        goto frame_exception_exit_1;
        // Finished with no exception for list_build:
        list_build_noexception_6:;
        tmp_assign_source_57 = PyFrozenSet_New(tmp_frozenset_arg_11);
        Py_DECREF(tmp_frozenset_arg_11);
        if (tmp_assign_source_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2932;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[193], tmp_assign_source_57);
    }
    {
        PyObject *tmp_assign_source_58;
        // Tried code:
        {
            PyObject *tmp_assign_source_59;
            PyObject *tmp_iter_arg_5;
            PyObject *tmp_called_instance_3;
            tmp_called_instance_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[16]);

            if (unlikely(tmp_called_instance_3 == NULL)) {
                tmp_called_instance_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[16]);
            }

            if (tmp_called_instance_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2936;

                goto try_except_handler_10;
            }
            frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 2936;
            tmp_iter_arg_5 = CALL_METHOD_NO_ARGS(tmp_called_instance_3, mod_consts[150]);
            if (tmp_iter_arg_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2936;

                goto try_except_handler_10;
            }
            tmp_assign_source_59 = MAKE_ITERATOR(tmp_iter_arg_5);
            Py_DECREF(tmp_iter_arg_5);
            if (tmp_assign_source_59 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2936;

                goto try_except_handler_10;
            }
            assert(tmp_dictcontraction_3__$0 == NULL);
            tmp_dictcontraction_3__$0 = tmp_assign_source_59;
        }
        {
            PyObject *tmp_assign_source_60;
            tmp_assign_source_60 = PyDict_New();
            assert(tmp_dictcontraction_3__contraction == NULL);
            tmp_dictcontraction_3__contraction = tmp_assign_source_60;
        }
        if (isFrameUnusable(cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4)) {
            Py_XDECREF(cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4);

#if _DEBUG_REFCOUNTS
            if (cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4 = MAKE_FUNCTION_FRAME(codeobj_d3bae8f8a8228db9f43b8f6e0fa66d2f, module_pip$_vendor$html5lib$constants, sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4->m_type_description == NULL);
        frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4 = cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4) == 2); // Frame stack

        // Framed code:
        // Tried code:
        loop_start_3:;
        {
            PyObject *tmp_next_source_3;
            PyObject *tmp_assign_source_61;
            CHECK_OBJECT(tmp_dictcontraction_3__$0);
            tmp_next_source_3 = tmp_dictcontraction_3__$0;
            tmp_assign_source_61 = ITERATOR_NEXT(tmp_next_source_3);
            if (tmp_assign_source_61 == NULL) {
                if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                    goto loop_end_3;
                } else {

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                    type_description_2 = "oo";
                    exception_lineno = 2936;
                    goto try_except_handler_11;
                }
            }

            {
                PyObject *old = tmp_dictcontraction_3__iter_value_0;
                tmp_dictcontraction_3__iter_value_0 = tmp_assign_source_61;
                Py_XDECREF(old);
            }

        }
        // Tried code:
        {
            PyObject *tmp_assign_source_62;
            PyObject *tmp_iter_arg_6;
            CHECK_OBJECT(tmp_dictcontraction_3__iter_value_0);
            tmp_iter_arg_6 = tmp_dictcontraction_3__iter_value_0;
            tmp_assign_source_62 = MAKE_ITERATOR(tmp_iter_arg_6);
            if (tmp_assign_source_62 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2936;
                type_description_2 = "oo";
                goto try_except_handler_12;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_3__source_iter;
                tmp_dictcontraction$tuple_unpack_3__source_iter = tmp_assign_source_62;
                Py_XDECREF(old);
            }

        }
        // Tried code:
        {
            PyObject *tmp_assign_source_63;
            PyObject *tmp_unpack_6;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_3__source_iter);
            tmp_unpack_6 = tmp_dictcontraction$tuple_unpack_3__source_iter;
            tmp_assign_source_63 = UNPACK_NEXT(tmp_unpack_6, 0, 2);
            if (tmp_assign_source_63 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "oo";
                exception_lineno = 2936;
                goto try_except_handler_13;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_3__element_1;
                tmp_dictcontraction$tuple_unpack_3__element_1 = tmp_assign_source_63;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_assign_source_64;
            PyObject *tmp_unpack_7;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_3__source_iter);
            tmp_unpack_7 = tmp_dictcontraction$tuple_unpack_3__source_iter;
            tmp_assign_source_64 = UNPACK_NEXT(tmp_unpack_7, 1, 2);
            if (tmp_assign_source_64 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "oo";
                exception_lineno = 2936;
                goto try_except_handler_13;
            }
            {
                PyObject *old = tmp_dictcontraction$tuple_unpack_3__element_2;
                tmp_dictcontraction$tuple_unpack_3__element_2 = tmp_assign_source_64;
                Py_XDECREF(old);
            }

        }
        {
            PyObject *tmp_iterator_name_3;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_3__source_iter);
            tmp_iterator_name_3 = tmp_dictcontraction$tuple_unpack_3__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_3); assert(HAS_ITERNEXT(tmp_iterator_name_3));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_3)->tp_iternext)(tmp_iterator_name_3);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "oo";
                        exception_lineno = 2936;
                        goto try_except_handler_13;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[151];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "oo";
                exception_lineno = 2936;
                goto try_except_handler_13;
            }
        }
        goto try_end_6;
        // Exception handler code:
        try_except_handler_13:;
        exception_keeper_type_10 = exception_type;
        exception_keeper_value_10 = exception_value;
        exception_keeper_tb_10 = exception_tb;
        exception_keeper_lineno_10 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__source_iter);
        tmp_dictcontraction$tuple_unpack_3__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_10;
        exception_value = exception_keeper_value_10;
        exception_tb = exception_keeper_tb_10;
        exception_lineno = exception_keeper_lineno_10;

        goto try_except_handler_12;
        // End of try:
        try_end_6:;
        goto try_end_7;
        // Exception handler code:
        try_except_handler_12:;
        exception_keeper_type_11 = exception_type;
        exception_keeper_value_11 = exception_value;
        exception_keeper_tb_11 = exception_tb;
        exception_keeper_lineno_11 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__element_1);
        tmp_dictcontraction$tuple_unpack_3__element_1 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__element_2);
        tmp_dictcontraction$tuple_unpack_3__element_2 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_11;
        exception_value = exception_keeper_value_11;
        exception_tb = exception_keeper_tb_11;
        exception_lineno = exception_keeper_lineno_11;

        goto try_except_handler_11;
        // End of try:
        try_end_7:;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__source_iter);
        tmp_dictcontraction$tuple_unpack_3__source_iter = NULL;
        {
            PyObject *tmp_assign_source_65;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_3__element_1);
            tmp_assign_source_65 = tmp_dictcontraction$tuple_unpack_3__element_1;
            {
                PyObject *old = outline_2_var_k;
                outline_2_var_k = tmp_assign_source_65;
                Py_INCREF(outline_2_var_k);
                Py_XDECREF(old);
            }

        }
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__element_1);
        tmp_dictcontraction$tuple_unpack_3__element_1 = NULL;

        {
            PyObject *tmp_assign_source_66;
            CHECK_OBJECT(tmp_dictcontraction$tuple_unpack_3__element_2);
            tmp_assign_source_66 = tmp_dictcontraction$tuple_unpack_3__element_2;
            {
                PyObject *old = outline_2_var_v;
                outline_2_var_v = tmp_assign_source_66;
                Py_INCREF(outline_2_var_v);
                Py_XDECREF(old);
            }

        }
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__element_2);
        tmp_dictcontraction$tuple_unpack_3__element_2 = NULL;

        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__element_1);
        tmp_dictcontraction$tuple_unpack_3__element_1 = NULL;
        Py_XDECREF(tmp_dictcontraction$tuple_unpack_3__element_2);
        tmp_dictcontraction$tuple_unpack_3__element_2 = NULL;
        CHECK_OBJECT(outline_2_var_k);
        tmp_dictset_value = outline_2_var_k;
        CHECK_OBJECT(tmp_dictcontraction_3__contraction);
        tmp_dictset_dict = tmp_dictcontraction_3__contraction;
        CHECK_OBJECT(outline_2_var_v);
        tmp_dictset_key = outline_2_var_v;
        tmp_res = PyDict_SetItem(tmp_dictset_dict, tmp_dictset_key, tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2936;
            type_description_2 = "oo";
            goto try_except_handler_11;
        }
        if (CONSIDER_THREADING() == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2936;
            type_description_2 = "oo";
            goto try_except_handler_11;
        }
        goto loop_start_3;
        loop_end_3:;
        CHECK_OBJECT(tmp_dictcontraction_3__contraction);
        tmp_assign_source_58 = tmp_dictcontraction_3__contraction;
        Py_INCREF(tmp_assign_source_58);
        goto try_return_handler_11;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_11:;
        Py_XDECREF(tmp_dictcontraction_3__$0);
        tmp_dictcontraction_3__$0 = NULL;
        CHECK_OBJECT(tmp_dictcontraction_3__contraction);
        Py_DECREF(tmp_dictcontraction_3__contraction);
        tmp_dictcontraction_3__contraction = NULL;
        Py_XDECREF(tmp_dictcontraction_3__iter_value_0);
        tmp_dictcontraction_3__iter_value_0 = NULL;
        goto frame_return_exit_3;
        // Exception handler code:
        try_except_handler_11:;
        exception_keeper_type_12 = exception_type;
        exception_keeper_value_12 = exception_value;
        exception_keeper_tb_12 = exception_tb;
        exception_keeper_lineno_12 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_dictcontraction_3__$0);
        tmp_dictcontraction_3__$0 = NULL;
        CHECK_OBJECT(tmp_dictcontraction_3__contraction);
        Py_DECREF(tmp_dictcontraction_3__contraction);
        tmp_dictcontraction_3__contraction = NULL;
        Py_XDECREF(tmp_dictcontraction_3__iter_value_0);
        tmp_dictcontraction_3__iter_value_0 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_12;
        exception_value = exception_keeper_value_12;
        exception_tb = exception_keeper_tb_12;
        exception_lineno = exception_keeper_lineno_12;

        goto frame_exception_exit_4;
        // End of try:

#if 0
        RESTORE_FRAME_EXCEPTION(frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_3;

        frame_return_exit_3:;
#if 0
        RESTORE_FRAME_EXCEPTION(frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto try_return_handler_10;

        frame_exception_exit_4:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4,
            type_description_2,
            outline_2_var_k,
            outline_2_var_v
        );


        // Release cached frame if used for exception.
        if (frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4 == cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4);
            cache_frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4 = NULL;
        }

        assertFrameObject(frame_d3bae8f8a8228db9f43b8f6e0fa66d2f_4);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_3;

        frame_no_exception_3:;
        goto skip_nested_handling_3;
        nested_frame_exit_3:;

        goto try_except_handler_10;
        skip_nested_handling_3:;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_10:;
        Py_XDECREF(outline_2_var_k);
        outline_2_var_k = NULL;
        Py_XDECREF(outline_2_var_v);
        outline_2_var_v = NULL;
        goto outline_result_3;
        // Exception handler code:
        try_except_handler_10:;
        exception_keeper_type_13 = exception_type;
        exception_keeper_value_13 = exception_value;
        exception_keeper_tb_13 = exception_tb;
        exception_keeper_lineno_13 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(outline_2_var_k);
        outline_2_var_k = NULL;
        Py_XDECREF(outline_2_var_v);
        outline_2_var_v = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_13;
        exception_value = exception_keeper_value_13;
        exception_tb = exception_keeper_tb_13;
        exception_lineno = exception_keeper_lineno_13;

        goto outline_exception_3;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_3:;
        exception_lineno = 2936;
        goto frame_exception_exit_1;
        outline_result_3:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[194], tmp_assign_source_58);
    }
    {
        PyObject *tmp_ass_subvalue_1;
        PyObject *tmp_ass_subscribed_1;
        PyObject *tmp_ass_subscript_1;
        tmp_ass_subvalue_1 = mod_consts[195];
        tmp_ass_subscribed_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[194]);

        if (unlikely(tmp_ass_subscribed_1 == NULL)) {
            tmp_ass_subscribed_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[194]);
        }

        assert(!(tmp_ass_subscribed_1 == NULL));
        tmp_ass_subscript_1 = mod_consts[196];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_1, tmp_ass_subscript_1, tmp_ass_subvalue_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2937;

            goto frame_exception_exit_1;
        }
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_67;
        PyObject *tmp_tuple_element_132;
        tmp_tuple_element_132 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[197]);

        if (unlikely(tmp_tuple_element_132 == NULL)) {
            tmp_tuple_element_132 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[197]);
        }

        if (tmp_tuple_element_132 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        tmp_assign_source_67 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_67, 0, tmp_tuple_element_132);
        assert(tmp_class_creation_1__bases == NULL);
        tmp_class_creation_1__bases = tmp_assign_source_67;
    }
    {
        PyObject *tmp_assign_source_68;
        tmp_assign_source_68 = PyDict_New();
        assert(tmp_class_creation_1__class_decl_dict == NULL);
        tmp_class_creation_1__class_decl_dict = tmp_assign_source_68;
    }
    {
        PyObject *tmp_assign_source_69;
        PyObject *tmp_metaclass_name_1;
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_key_name_134;
        PyObject *tmp_dict_arg_name_134;
        PyObject *tmp_dict_arg_name_135;
        PyObject *tmp_key_name_135;
        PyObject *tmp_type_arg_1;
        PyObject *tmp_expression_name_7;
        PyObject *tmp_subscript_name_1;
        PyObject *tmp_bases_name_1;
        tmp_key_name_134 = mod_consts[198];
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_134 = tmp_class_creation_1__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_134, tmp_key_name_134);
        assert(!(tmp_res == -1));
        tmp_condition_result_1 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_1;
        } else {
            goto condexpr_false_1;
        }
        condexpr_true_1:;
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_135 = tmp_class_creation_1__class_decl_dict;
        tmp_key_name_135 = mod_consts[198];
        tmp_metaclass_name_1 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_135, tmp_key_name_135);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        goto condexpr_end_1;
        condexpr_false_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_expression_name_7 = tmp_class_creation_1__bases;
        tmp_subscript_name_1 = mod_consts[8];
        tmp_type_arg_1 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_7, tmp_subscript_name_1, 0);
        if (tmp_type_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        tmp_metaclass_name_1 = BUILTIN_TYPE1(tmp_type_arg_1);
        Py_DECREF(tmp_type_arg_1);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        condexpr_end_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_bases_name_1 = tmp_class_creation_1__bases;
        tmp_assign_source_69 = SELECT_METACLASS(tmp_metaclass_name_1, tmp_bases_name_1);
        Py_DECREF(tmp_metaclass_name_1);
        if (tmp_assign_source_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        assert(tmp_class_creation_1__metaclass == NULL);
        tmp_class_creation_1__metaclass = tmp_assign_source_69;
    }
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_key_name_136;
        PyObject *tmp_dict_arg_name_136;
        tmp_key_name_136 = mod_consts[198];
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_136 = tmp_class_creation_1__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_136, tmp_key_name_136);
        assert(!(tmp_res == -1));
        tmp_condition_result_2 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_2 == NUITKA_BOOL_TRUE) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_1__class_decl_dict;
    tmp_dictdel_key = mod_consts[198];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 2940;

        goto try_except_handler_14;
    }
    branch_no_1:;
    {
        nuitka_bool tmp_condition_result_3;
        PyObject *tmp_expression_name_8;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_8 = tmp_class_creation_1__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_8, mod_consts[199]);
        tmp_condition_result_3 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_3 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_assign_source_70;
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_9;
        PyObject *tmp_args_name_1;
        PyObject *tmp_tuple_element_133;
        PyObject *tmp_kwargs_name_1;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_9 = tmp_class_creation_1__metaclass;
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_9, mod_consts[199]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        tmp_tuple_element_133 = mod_consts[200];
        tmp_args_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_1, 0, tmp_tuple_element_133);
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_tuple_element_133 = tmp_class_creation_1__bases;
        PyTuple_SET_ITEM0(tmp_args_name_1, 1, tmp_tuple_element_133);
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_kwargs_name_1 = tmp_class_creation_1__class_decl_dict;
        frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 2940;
        tmp_assign_source_70 = CALL_FUNCTION(tmp_called_name_1, tmp_args_name_1, tmp_kwargs_name_1);
        Py_DECREF(tmp_called_name_1);
        Py_DECREF(tmp_args_name_1);
        if (tmp_assign_source_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_70;
    }
    {
        nuitka_bool tmp_condition_result_4;
        PyObject *tmp_operand_name_1;
        PyObject *tmp_expression_name_10;
        CHECK_OBJECT(tmp_class_creation_1__prepared);
        tmp_expression_name_10 = tmp_class_creation_1__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_10, mod_consts[201]);
        tmp_operand_name_1 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        tmp_condition_result_4 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_4 == NUITKA_BOOL_TRUE) {
            goto branch_yes_3;
        } else {
            goto branch_no_3;
        }
    }
    branch_yes_3:;
    {
        PyObject *tmp_raise_type_1;
        PyObject *tmp_raise_value_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_tuple_element_134;
        PyObject *tmp_getattr_target_1;
        PyObject *tmp_getattr_attr_1;
        PyObject *tmp_getattr_default_1;
        tmp_raise_type_1 = PyExc_TypeError;
        tmp_left_name_1 = mod_consts[202];
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_getattr_target_1 = tmp_class_creation_1__metaclass;
        tmp_getattr_attr_1 = mod_consts[203];
        tmp_getattr_default_1 = mod_consts[204];
        tmp_tuple_element_134 = BUILTIN_GETATTR(tmp_getattr_target_1, tmp_getattr_attr_1, tmp_getattr_default_1);
        if (tmp_tuple_element_134 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        tmp_right_name_1 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_11;
            PyObject *tmp_type_arg_2;
            PyTuple_SET_ITEM(tmp_right_name_1, 0, tmp_tuple_element_134);
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_type_arg_2 = tmp_class_creation_1__prepared;
            tmp_expression_name_11 = BUILTIN_TYPE1(tmp_type_arg_2);
            assert(!(tmp_expression_name_11 == NULL));
            tmp_tuple_element_134 = LOOKUP_ATTRIBUTE(tmp_expression_name_11, mod_consts[203]);
            Py_DECREF(tmp_expression_name_11);
            if (tmp_tuple_element_134 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2940;

                goto tuple_build_exception_13;
            }
            PyTuple_SET_ITEM(tmp_right_name_1, 1, tmp_tuple_element_134);
        }
        goto tuple_build_noexception_13;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_13:;
        Py_DECREF(tmp_right_name_1);
        goto try_except_handler_14;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_13:;
        tmp_raise_value_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_raise_value_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_14;
        }
        exception_type = tmp_raise_type_1;
        Py_INCREF(tmp_raise_type_1);
        exception_value = tmp_raise_value_1;
        exception_lineno = 2940;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_14;
    }
    branch_no_3:;
    goto branch_end_2;
    branch_no_2:;
    {
        PyObject *tmp_assign_source_71;
        tmp_assign_source_71 = PyDict_New();
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_71;
    }
    branch_end_2:;
    {
        PyObject *tmp_assign_source_72;
        {
            PyObject *tmp_set_locals_1;
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_set_locals_1 = tmp_class_creation_1__prepared;
            locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940 = tmp_set_locals_1;
            Py_INCREF(tmp_set_locals_1);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[205];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940, mod_consts[206], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_16;
        }
        tmp_dictset_value = mod_consts[207];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940, mod_consts[0], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_16;
        }
        tmp_dictset_value = mod_consts[200];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940, mod_consts[208], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2940;

            goto try_except_handler_16;
        }
        {
            PyObject *tmp_assign_source_73;
            PyObject *tmp_called_name_2;
            PyObject *tmp_args_name_2;
            PyObject *tmp_tuple_element_135;
            PyObject *tmp_kwargs_name_2;
            CHECK_OBJECT(tmp_class_creation_1__metaclass);
            tmp_called_name_2 = tmp_class_creation_1__metaclass;
            tmp_tuple_element_135 = mod_consts[200];
            tmp_args_name_2 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_2, 0, tmp_tuple_element_135);
            CHECK_OBJECT(tmp_class_creation_1__bases);
            tmp_tuple_element_135 = tmp_class_creation_1__bases;
            PyTuple_SET_ITEM0(tmp_args_name_2, 1, tmp_tuple_element_135);
            tmp_tuple_element_135 = locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940;
            PyTuple_SET_ITEM0(tmp_args_name_2, 2, tmp_tuple_element_135);
            CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
            tmp_kwargs_name_2 = tmp_class_creation_1__class_decl_dict;
            frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 2940;
            tmp_assign_source_73 = CALL_FUNCTION(tmp_called_name_2, tmp_args_name_2, tmp_kwargs_name_2);
            Py_DECREF(tmp_args_name_2);
            if (tmp_assign_source_73 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2940;

                goto try_except_handler_16;
            }
            assert(outline_3_var___class__ == NULL);
            outline_3_var___class__ = tmp_assign_source_73;
        }
        CHECK_OBJECT(outline_3_var___class__);
        tmp_assign_source_72 = outline_3_var___class__;
        Py_INCREF(tmp_assign_source_72);
        goto try_return_handler_16;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_16:;
        Py_DECREF(locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940);
        locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940 = NULL;
        goto try_return_handler_15;
        // Exception handler code:
        try_except_handler_16:;
        exception_keeper_type_14 = exception_type;
        exception_keeper_value_14 = exception_value;
        exception_keeper_tb_14 = exception_tb;
        exception_keeper_lineno_14 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940);
        locals_pip$_vendor$html5lib$constants$$$class__1_DataLossWarning_2940 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_14;
        exception_value = exception_keeper_value_14;
        exception_tb = exception_keeper_tb_14;
        exception_lineno = exception_keeper_lineno_14;

        goto try_except_handler_15;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_15:;
        CHECK_OBJECT(outline_3_var___class__);
        Py_DECREF(outline_3_var___class__);
        outline_3_var___class__ = NULL;
        goto outline_result_4;
        // Exception handler code:
        try_except_handler_15:;
        exception_keeper_type_15 = exception_type;
        exception_keeper_value_15 = exception_value;
        exception_keeper_tb_15 = exception_tb;
        exception_keeper_lineno_15 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_15;
        exception_value = exception_keeper_value_15;
        exception_tb = exception_keeper_tb_15;
        exception_lineno = exception_keeper_lineno_15;

        goto outline_exception_4;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_4:;
        exception_lineno = 2940;
        goto try_except_handler_14;
        outline_result_4:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[200], tmp_assign_source_72);
    }
    goto try_end_8;
    // Exception handler code:
    try_except_handler_14:;
    exception_keeper_type_16 = exception_type;
    exception_keeper_value_16 = exception_value;
    exception_keeper_tb_16 = exception_tb;
    exception_keeper_lineno_16 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_1__bases);
    tmp_class_creation_1__bases = NULL;
    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_16;
    exception_value = exception_keeper_value_16;
    exception_tb = exception_keeper_tb_16;
    exception_lineno = exception_keeper_lineno_16;

    goto frame_exception_exit_1;
    // End of try:
    try_end_8:;
    Py_XDECREF(tmp_class_creation_1__bases);
    tmp_class_creation_1__bases = NULL;
    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_1__prepared);
    Py_DECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;
    {
        PyObject *tmp_assign_source_74;
        tmp_assign_source_74 = mod_consts[209];
        assert(tmp_class_creation_2__bases == NULL);
        Py_INCREF(tmp_assign_source_74);
        tmp_class_creation_2__bases = tmp_assign_source_74;
    }
    {
        PyObject *tmp_assign_source_75;
        tmp_assign_source_75 = PyDict_New();
        assert(tmp_class_creation_2__class_decl_dict == NULL);
        tmp_class_creation_2__class_decl_dict = tmp_assign_source_75;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_76;
        PyObject *tmp_metaclass_name_2;
        nuitka_bool tmp_condition_result_5;
        PyObject *tmp_key_name_137;
        PyObject *tmp_dict_arg_name_137;
        PyObject *tmp_dict_arg_name_138;
        PyObject *tmp_key_name_138;
        PyObject *tmp_type_arg_3;
        PyObject *tmp_expression_name_12;
        PyObject *tmp_subscript_name_2;
        PyObject *tmp_bases_name_2;
        tmp_key_name_137 = mod_consts[198];
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_137 = tmp_class_creation_2__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_137, tmp_key_name_137);
        assert(!(tmp_res == -1));
        tmp_condition_result_5 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_5 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_2;
        } else {
            goto condexpr_false_2;
        }
        condexpr_true_2:;
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_138 = tmp_class_creation_2__class_decl_dict;
        tmp_key_name_138 = mod_consts[198];
        tmp_metaclass_name_2 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_138, tmp_key_name_138);
        if (tmp_metaclass_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        goto condexpr_end_2;
        condexpr_false_2:;
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_expression_name_12 = tmp_class_creation_2__bases;
        tmp_subscript_name_2 = mod_consts[8];
        tmp_type_arg_3 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_12, tmp_subscript_name_2, 0);
        if (tmp_type_arg_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        tmp_metaclass_name_2 = BUILTIN_TYPE1(tmp_type_arg_3);
        Py_DECREF(tmp_type_arg_3);
        if (tmp_metaclass_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        condexpr_end_2:;
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_bases_name_2 = tmp_class_creation_2__bases;
        tmp_assign_source_76 = SELECT_METACLASS(tmp_metaclass_name_2, tmp_bases_name_2);
        Py_DECREF(tmp_metaclass_name_2);
        if (tmp_assign_source_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        assert(tmp_class_creation_2__metaclass == NULL);
        tmp_class_creation_2__metaclass = tmp_assign_source_76;
    }
    {
        nuitka_bool tmp_condition_result_6;
        PyObject *tmp_key_name_139;
        PyObject *tmp_dict_arg_name_139;
        tmp_key_name_139 = mod_consts[198];
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_139 = tmp_class_creation_2__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_139, tmp_key_name_139);
        assert(!(tmp_res == -1));
        tmp_condition_result_6 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_6 == NUITKA_BOOL_TRUE) {
            goto branch_yes_4;
        } else {
            goto branch_no_4;
        }
    }
    branch_yes_4:;
    CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_2__class_decl_dict;
    tmp_dictdel_key = mod_consts[198];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 2945;

        goto try_except_handler_17;
    }
    branch_no_4:;
    {
        nuitka_bool tmp_condition_result_7;
        PyObject *tmp_expression_name_13;
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_expression_name_13 = tmp_class_creation_2__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_13, mod_consts[199]);
        tmp_condition_result_7 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_7 == NUITKA_BOOL_TRUE) {
            goto branch_yes_5;
        } else {
            goto branch_no_5;
        }
    }
    branch_yes_5:;
    {
        PyObject *tmp_assign_source_77;
        PyObject *tmp_called_name_3;
        PyObject *tmp_expression_name_14;
        PyObject *tmp_args_name_3;
        PyObject *tmp_tuple_element_136;
        PyObject *tmp_kwargs_name_3;
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_expression_name_14 = tmp_class_creation_2__metaclass;
        tmp_called_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_14, mod_consts[199]);
        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        tmp_tuple_element_136 = mod_consts[210];
        tmp_args_name_3 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_3, 0, tmp_tuple_element_136);
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_tuple_element_136 = tmp_class_creation_2__bases;
        PyTuple_SET_ITEM0(tmp_args_name_3, 1, tmp_tuple_element_136);
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_kwargs_name_3 = tmp_class_creation_2__class_decl_dict;
        frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 2945;
        tmp_assign_source_77 = CALL_FUNCTION(tmp_called_name_3, tmp_args_name_3, tmp_kwargs_name_3);
        Py_DECREF(tmp_called_name_3);
        Py_DECREF(tmp_args_name_3);
        if (tmp_assign_source_77 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        assert(tmp_class_creation_2__prepared == NULL);
        tmp_class_creation_2__prepared = tmp_assign_source_77;
    }
    {
        nuitka_bool tmp_condition_result_8;
        PyObject *tmp_operand_name_2;
        PyObject *tmp_expression_name_15;
        CHECK_OBJECT(tmp_class_creation_2__prepared);
        tmp_expression_name_15 = tmp_class_creation_2__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_15, mod_consts[201]);
        tmp_operand_name_2 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        tmp_condition_result_8 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_8 == NUITKA_BOOL_TRUE) {
            goto branch_yes_6;
        } else {
            goto branch_no_6;
        }
    }
    branch_yes_6:;
    {
        PyObject *tmp_raise_type_2;
        PyObject *tmp_raise_value_2;
        PyObject *tmp_left_name_2;
        PyObject *tmp_right_name_2;
        PyObject *tmp_tuple_element_137;
        PyObject *tmp_getattr_target_2;
        PyObject *tmp_getattr_attr_2;
        PyObject *tmp_getattr_default_2;
        tmp_raise_type_2 = PyExc_TypeError;
        tmp_left_name_2 = mod_consts[202];
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_getattr_target_2 = tmp_class_creation_2__metaclass;
        tmp_getattr_attr_2 = mod_consts[203];
        tmp_getattr_default_2 = mod_consts[204];
        tmp_tuple_element_137 = BUILTIN_GETATTR(tmp_getattr_target_2, tmp_getattr_attr_2, tmp_getattr_default_2);
        if (tmp_tuple_element_137 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        tmp_right_name_2 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_16;
            PyObject *tmp_type_arg_4;
            PyTuple_SET_ITEM(tmp_right_name_2, 0, tmp_tuple_element_137);
            CHECK_OBJECT(tmp_class_creation_2__prepared);
            tmp_type_arg_4 = tmp_class_creation_2__prepared;
            tmp_expression_name_16 = BUILTIN_TYPE1(tmp_type_arg_4);
            assert(!(tmp_expression_name_16 == NULL));
            tmp_tuple_element_137 = LOOKUP_ATTRIBUTE(tmp_expression_name_16, mod_consts[203]);
            Py_DECREF(tmp_expression_name_16);
            if (tmp_tuple_element_137 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2945;

                goto tuple_build_exception_14;
            }
            PyTuple_SET_ITEM(tmp_right_name_2, 1, tmp_tuple_element_137);
        }
        goto tuple_build_noexception_14;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_14:;
        Py_DECREF(tmp_right_name_2);
        goto try_except_handler_17;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_14:;
        tmp_raise_value_2 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_2, tmp_right_name_2);
        Py_DECREF(tmp_right_name_2);
        if (tmp_raise_value_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_17;
        }
        exception_type = tmp_raise_type_2;
        Py_INCREF(tmp_raise_type_2);
        exception_value = tmp_raise_value_2;
        exception_lineno = 2945;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_17;
    }
    branch_no_6:;
    goto branch_end_5;
    branch_no_5:;
    {
        PyObject *tmp_assign_source_78;
        tmp_assign_source_78 = PyDict_New();
        assert(tmp_class_creation_2__prepared == NULL);
        tmp_class_creation_2__prepared = tmp_assign_source_78;
    }
    branch_end_5:;
    {
        PyObject *tmp_assign_source_79;
        {
            PyObject *tmp_set_locals_2;
            CHECK_OBJECT(tmp_class_creation_2__prepared);
            tmp_set_locals_2 = tmp_class_creation_2__prepared;
            locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945 = tmp_set_locals_2;
            Py_INCREF(tmp_set_locals_2);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[205];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945, mod_consts[206], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_19;
        }
        tmp_dictset_value = mod_consts[210];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945, mod_consts[208], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2945;

            goto try_except_handler_19;
        }
        {
            PyObject *tmp_assign_source_80;
            PyObject *tmp_called_name_4;
            PyObject *tmp_args_name_4;
            PyObject *tmp_tuple_element_138;
            PyObject *tmp_kwargs_name_4;
            CHECK_OBJECT(tmp_class_creation_2__metaclass);
            tmp_called_name_4 = tmp_class_creation_2__metaclass;
            tmp_tuple_element_138 = mod_consts[210];
            tmp_args_name_4 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_4, 0, tmp_tuple_element_138);
            CHECK_OBJECT(tmp_class_creation_2__bases);
            tmp_tuple_element_138 = tmp_class_creation_2__bases;
            PyTuple_SET_ITEM0(tmp_args_name_4, 1, tmp_tuple_element_138);
            tmp_tuple_element_138 = locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945;
            PyTuple_SET_ITEM0(tmp_args_name_4, 2, tmp_tuple_element_138);
            CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
            tmp_kwargs_name_4 = tmp_class_creation_2__class_decl_dict;
            frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame.f_lineno = 2945;
            tmp_assign_source_80 = CALL_FUNCTION(tmp_called_name_4, tmp_args_name_4, tmp_kwargs_name_4);
            Py_DECREF(tmp_args_name_4);
            if (tmp_assign_source_80 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 2945;

                goto try_except_handler_19;
            }
            assert(outline_4_var___class__ == NULL);
            outline_4_var___class__ = tmp_assign_source_80;
        }
        CHECK_OBJECT(outline_4_var___class__);
        tmp_assign_source_79 = outline_4_var___class__;
        Py_INCREF(tmp_assign_source_79);
        goto try_return_handler_19;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_19:;
        Py_DECREF(locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945);
        locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945 = NULL;
        goto try_return_handler_18;
        // Exception handler code:
        try_except_handler_19:;
        exception_keeper_type_17 = exception_type;
        exception_keeper_value_17 = exception_value;
        exception_keeper_tb_17 = exception_tb;
        exception_keeper_lineno_17 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945);
        locals_pip$_vendor$html5lib$constants$$$class__2__ReparseException_2945 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_17;
        exception_value = exception_keeper_value_17;
        exception_tb = exception_keeper_tb_17;
        exception_lineno = exception_keeper_lineno_17;

        goto try_except_handler_18;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_18:;
        CHECK_OBJECT(outline_4_var___class__);
        Py_DECREF(outline_4_var___class__);
        outline_4_var___class__ = NULL;
        goto outline_result_5;
        // Exception handler code:
        try_except_handler_18:;
        exception_keeper_type_18 = exception_type;
        exception_keeper_value_18 = exception_value;
        exception_keeper_tb_18 = exception_tb;
        exception_keeper_lineno_18 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_18;
        exception_value = exception_keeper_value_18;
        exception_tb = exception_keeper_tb_18;
        exception_lineno = exception_keeper_lineno_18;

        goto outline_exception_5;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_5:;
        exception_lineno = 2945;
        goto try_except_handler_17;
        outline_result_5:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$constants, (Nuitka_StringObject *)mod_consts[210], tmp_assign_source_79);
    }
    goto try_end_9;
    // Exception handler code:
    try_except_handler_17:;
    exception_keeper_type_19 = exception_type;
    exception_keeper_value_19 = exception_value;
    exception_keeper_tb_19 = exception_tb;
    exception_keeper_lineno_19 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_2__bases);
    tmp_class_creation_2__bases = NULL;
    Py_XDECREF(tmp_class_creation_2__class_decl_dict);
    tmp_class_creation_2__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_2__metaclass);
    tmp_class_creation_2__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_2__prepared);
    tmp_class_creation_2__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_19;
    exception_value = exception_keeper_value_19;
    exception_tb = exception_keeper_tb_19;
    exception_lineno = exception_keeper_lineno_19;

    goto frame_exception_exit_1;
    // End of try:
    try_end_9:;

    // Restore frame exception if necessary.
#if 0
    RESTORE_FRAME_EXCEPTION(frame_ab5cd1d9cd36c1f152d39a0bfa133d17);
#endif
    popFrameStack();

    assertFrameObject(frame_ab5cd1d9cd36c1f152d39a0bfa133d17);

    goto frame_no_exception_4;

    frame_exception_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_ab5cd1d9cd36c1f152d39a0bfa133d17);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_ab5cd1d9cd36c1f152d39a0bfa133d17, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_ab5cd1d9cd36c1f152d39a0bfa133d17->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_ab5cd1d9cd36c1f152d39a0bfa133d17, exception_lineno);
    }

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto module_exception_exit;

    frame_no_exception_4:;
    Py_XDECREF(tmp_class_creation_2__bases);
    tmp_class_creation_2__bases = NULL;
    Py_XDECREF(tmp_class_creation_2__class_decl_dict);
    tmp_class_creation_2__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_2__metaclass);
    tmp_class_creation_2__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_2__prepared);
    Py_DECREF(tmp_class_creation_2__prepared);
    tmp_class_creation_2__prepared = NULL;

    return module_pip$_vendor$html5lib$constants;
    module_exception_exit:
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);
    return NULL;
}

