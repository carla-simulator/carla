/* Generated code for Python module 'click'
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

/* The "module_click" is a Python object pointer of module type.
 *
 * Note: For full compatibility with CPython, every module variable access
 * needs to go through it except for cases where the module cannot possibly
 * have changed in the mean time.
 */

PyObject *module_click;
PyDictObject *moduledict_click;

/* The declarations of module constants used, if any. */
static PyObject *mod_consts[159];

static PyObject *module_filename_obj = NULL;

/* Indicator if this modules private constants were created yet. */
static bool constants_created = false;

/* Function to create module private constants. */
static void createModuleConstants(void) {
    if (constants_created == false) {
        loadConstantsBlob(&mod_consts[0], UNTRANSLATE("click"));
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
void checkModuleConstants_click(void) {
    // The module may not have been used at all, then ignore this.
    if (constants_created == false) return;

    checkConstantsBlob(&mod_consts[0], "click");
}
#endif

// The module code objects.
static PyCodeObject *codeobj_0bf55338aa053bcca70d25585371a4c2;

static void createModuleCodeObjects(void) {
    module_filename_obj = mod_consts[2]; CHECK_OBJECT(module_filename_obj);
    codeobj_0bf55338aa053bcca70d25585371a4c2 = MAKE_CODEOBJECT(module_filename_obj, 1, CO_NOFREE, mod_consts[158], NULL, NULL, 0, 0, 0);
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

function_impl_code functable_click[] = {

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

    function_impl_code *current = functable_click;
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

    if (offset > sizeof(functable_click) || offset < 0) {
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
        functable_click[offset],
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
        module_click,
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
PyObject *modulecode_click(PyObject *module, struct Nuitka_MetaPathBasedLoaderEntry const *module_entry) {
    module_click = module;

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
    PRINT_STRING("click: Calling setupMetaPathBasedLoader().\n");
#endif
    setupMetaPathBasedLoader();

#if PYTHON_VERSION >= 0x300
    patchInspectModule();
#endif

#endif

    /* The constants only used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("click: Calling createModuleConstants().\n");
#endif
    createModuleConstants();

    /* The code objects used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("click: Calling createModuleCodeObjects().\n");
#endif
    createModuleCodeObjects();

    // PRINT_STRING("in initclick\n");

    // Create the module object first. There are no methods initially, all are
    // added dynamically in actual code only.  Also no "__doc__" is initially
    // set at this time, as it could not contain NUL characters this way, they
    // are instead set in early module code.  No "self" for modules, we have no
    // use for it.

    moduledict_click = MODULE_DICT(module_click);

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
    registerDillPluginTables(module_entry->name, &_method_def_reduce_compiled_function, &_method_def_create_compiled_function);
#endif

    // Set "__compiled__" to what version information we have.
    UPDATE_STRING_DICT0(
        moduledict_click,
        (Nuitka_StringObject *)const_str_plain___compiled__,
        Nuitka_dunder_compiled_value
    );

    // Update "__package__" value to what it ought to be.
    {
#if 0
        UPDATE_STRING_DICT0(
            moduledict_click,
            (Nuitka_StringObject *)const_str_plain___package__,
            const_str_empty
        );
#elif 1
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)const_str_plain___name__);

        UPDATE_STRING_DICT0(
            moduledict_click,
            (Nuitka_StringObject *)const_str_plain___package__,
            module_name
        );
#else

#if PYTHON_VERSION < 0x300
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)const_str_plain___name__);
        char const *module_name_cstr = PyString_AS_STRING(module_name);

        char const *last_dot = strrchr(module_name_cstr, '.');

        if (last_dot != NULL) {
            UPDATE_STRING_DICT1(
                moduledict_click,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyString_FromStringAndSize(module_name_cstr, last_dot - module_name_cstr)
            );
        }
#else
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)const_str_plain___name__);
        Py_ssize_t dot_index = PyUnicode_Find(module_name, const_str_dot, 0, PyUnicode_GetLength(module_name), -1);

        if (dot_index != -1) {
            UPDATE_STRING_DICT1(
                moduledict_click,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyUnicode_Substring(module_name, 0, dot_index)
            );
        }
#endif
#endif
    }

    CHECK_OBJECT(module_click);

    // For deep importing of a module we need to have "__builtins__", so we set
    // it ourselves in the same way than CPython does. Note: This must be done
    // before the frame object is allocated, or else it may fail.

    if (GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)const_str_plain___builtins__) == NULL) {
        PyObject *value = (PyObject *)builtin_module;

        // Check if main module, not a dict then but the module itself.
#if !defined(_NUITKA_EXE) || !0
        value = PyModule_GetDict(value);
#endif

        UPDATE_STRING_DICT0(moduledict_click, (Nuitka_StringObject *)const_str_plain___builtins__, value);
    }

#if PYTHON_VERSION >= 0x300
    UPDATE_STRING_DICT0(moduledict_click, (Nuitka_StringObject *)const_str_plain___loader__, (PyObject *)&Nuitka_Loader_Type);
#endif

#if PYTHON_VERSION >= 0x340
// Set the "__spec__" value

#if 0
    // Main modules just get "None" as spec.
    UPDATE_STRING_DICT0(moduledict_click, (Nuitka_StringObject *)const_str_plain___spec__, Py_None);
#else
    // Other modules get a "ModuleSpec" from the standard mechanism.
    {
        PyObject *bootstrap_module = getImportLibBootstrapModule();
        CHECK_OBJECT(bootstrap_module);

        PyObject *_spec_from_module = PyObject_GetAttrString(bootstrap_module, "_spec_from_module");
        CHECK_OBJECT(_spec_from_module);

        PyObject *spec_value = CALL_FUNCTION_WITH_SINGLE_ARG(_spec_from_module, module_click);
        Py_DECREF(_spec_from_module);

        // We can assume this to never fail, or else we are in trouble anyway.
        // CHECK_OBJECT(spec_value);

        if (spec_value == NULL) {
            PyErr_PrintEx(0);
            abort();
        }

// Mark the execution in the "__spec__" value.
        SET_ATTRIBUTE(spec_value, const_str_plain__initializing, Py_True);

        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)const_str_plain___spec__, spec_value);
    }
#endif
#endif

    // Temp variables if any
    struct Nuitka_FrameObject *frame_0bf55338aa053bcca70d25585371a4c2;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_dictset_value;
    PyObject *tmp_dictset_dict;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    PyObject *tmp_dictset_key;
    int tmp_res;
    bool tmp_result;

    // Module code.
    {
        PyObject *tmp_assign_source_1;
        tmp_assign_source_1 = mod_consts[0];
        UPDATE_STRING_DICT0(moduledict_click, (Nuitka_StringObject *)mod_consts[1], tmp_assign_source_1);
    }
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = mod_consts[2];
        UPDATE_STRING_DICT0(moduledict_click, (Nuitka_StringObject *)mod_consts[3], tmp_assign_source_2);
    }
    {
        PyObject *tmp_assign_source_3;
        tmp_assign_source_3 = LIST_COPY(mod_consts[4]);
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[5], tmp_assign_source_3);
    }
    // Frame without reuse.
    frame_0bf55338aa053bcca70d25585371a4c2 = MAKE_MODULE_FRAME(codeobj_0bf55338aa053bcca70d25585371a4c2, module_click);

    // Push the new frame as the currently active one, and we should be exclusively
    // owning it.
    pushFrameStack(frame_0bf55338aa053bcca70d25585371a4c2);
    assert(Py_REFCNT(frame_0bf55338aa053bcca70d25585371a4c2) == 2);

    // Framed code:
    {
        PyObject *tmp_expression_name_1;
        PyObject *tmp_subscript_name_1;
        tmp_dictset_value = Nuitka_Loader_New(module_entry);
        tmp_dictset_dict = PySys_GetObject((char *)"path_importer_cache");
        if (tmp_dictset_dict == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
        tmp_expression_name_1 = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)mod_consts[5]);

        if (unlikely(tmp_expression_name_1 == NULL)) {
            tmp_expression_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[5]);
        }

        assert(!(tmp_expression_name_1 == NULL));
        tmp_subscript_name_1 = mod_consts[6];
        tmp_dictset_key = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_1, tmp_subscript_name_1, 0);
        if (tmp_dictset_key == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
        tmp_res = PyDict_SetItem(tmp_dictset_dict, tmp_dictset_key, tmp_dictset_value);
        Py_DECREF(tmp_dictset_key);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_assattr_name_1 = mod_consts[2];
        tmp_assattr_target_1 = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)mod_consts[7]);

        if (unlikely(tmp_assattr_target_1 == NULL)) {
            tmp_assattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[7]);
        }

        assert(!(tmp_assattr_target_1 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[8], tmp_assattr_name_1);
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
        tmp_assattr_target_2 = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)mod_consts[7]);

        if (unlikely(tmp_assattr_target_2 == NULL)) {
            tmp_assattr_target_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[7]);
        }

        assert(!(tmp_assattr_target_2 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[9], tmp_assattr_name_2);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_3;
        PyObject *tmp_assattr_target_3;
        tmp_assattr_name_3 = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)mod_consts[5]);

        if (unlikely(tmp_assattr_name_3 == NULL)) {
            tmp_assattr_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[5]);
        }

        if (tmp_assattr_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
        tmp_assattr_target_3 = GET_STRING_DICT_VALUE(moduledict_click, (Nuitka_StringObject *)mod_consts[7]);

        if (unlikely(tmp_assattr_target_3 == NULL)) {
            tmp_assattr_target_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[7]);
        }

        assert(!(tmp_assattr_target_3 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_3, mod_consts[10], tmp_assattr_name_3);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1;

            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assign_source_4;
        tmp_assign_source_4 = Py_None;
        UPDATE_STRING_DICT0(moduledict_click, (Nuitka_StringObject *)mod_consts[11], tmp_assign_source_4);
    }
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_import_name_from_1;
        PyObject *tmp_name_name_1;
        PyObject *tmp_globals_arg_name_1;
        PyObject *tmp_locals_arg_name_1;
        PyObject *tmp_fromlist_name_1;
        PyObject *tmp_level_name_1;
        tmp_name_name_1 = mod_consts[12];
        tmp_globals_arg_name_1 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_1 = Py_None;
        tmp_fromlist_name_1 = mod_consts[13];
        tmp_level_name_1 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 7;
        tmp_import_name_from_1 = IMPORT_MODULE5(tmp_name_name_1, tmp_globals_arg_name_1, tmp_locals_arg_name_1, tmp_fromlist_name_1, tmp_level_name_1);
        if (tmp_import_name_from_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 7;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_1)) {
            tmp_assign_source_5 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_1,
                (PyObject *)moduledict_click,
                mod_consts[15],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_5 = IMPORT_NAME(tmp_import_name_from_1, mod_consts[15]);
        }

        Py_DECREF(tmp_import_name_from_1);
        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 7;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[15], tmp_assign_source_5);
    }
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_import_name_from_2;
        PyObject *tmp_name_name_2;
        PyObject *tmp_globals_arg_name_2;
        PyObject *tmp_locals_arg_name_2;
        PyObject *tmp_fromlist_name_2;
        PyObject *tmp_level_name_2;
        tmp_name_name_2 = mod_consts[12];
        tmp_globals_arg_name_2 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_2 = Py_None;
        tmp_fromlist_name_2 = mod_consts[16];
        tmp_level_name_2 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 8;
        tmp_import_name_from_2 = IMPORT_MODULE5(tmp_name_name_2, tmp_globals_arg_name_2, tmp_locals_arg_name_2, tmp_fromlist_name_2, tmp_level_name_2);
        if (tmp_import_name_from_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_2)) {
            tmp_assign_source_6 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_2,
                (PyObject *)moduledict_click,
                mod_consts[17],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_6 = IMPORT_NAME(tmp_import_name_from_2, mod_consts[17]);
        }

        Py_DECREF(tmp_import_name_from_2);
        if (tmp_assign_source_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[17], tmp_assign_source_6);
    }
    {
        PyObject *tmp_assign_source_7;
        PyObject *tmp_import_name_from_3;
        PyObject *tmp_name_name_3;
        PyObject *tmp_globals_arg_name_3;
        PyObject *tmp_locals_arg_name_3;
        PyObject *tmp_fromlist_name_3;
        PyObject *tmp_level_name_3;
        tmp_name_name_3 = mod_consts[12];
        tmp_globals_arg_name_3 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_3 = Py_None;
        tmp_fromlist_name_3 = mod_consts[18];
        tmp_level_name_3 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 9;
        tmp_import_name_from_3 = IMPORT_MODULE5(tmp_name_name_3, tmp_globals_arg_name_3, tmp_locals_arg_name_3, tmp_fromlist_name_3, tmp_level_name_3);
        if (tmp_import_name_from_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 9;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_3)) {
            tmp_assign_source_7 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_3,
                (PyObject *)moduledict_click,
                mod_consts[19],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_7 = IMPORT_NAME(tmp_import_name_from_3, mod_consts[19]);
        }

        Py_DECREF(tmp_import_name_from_3);
        if (tmp_assign_source_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 9;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[19], tmp_assign_source_7);
    }
    {
        PyObject *tmp_assign_source_8;
        PyObject *tmp_import_name_from_4;
        PyObject *tmp_name_name_4;
        PyObject *tmp_globals_arg_name_4;
        PyObject *tmp_locals_arg_name_4;
        PyObject *tmp_fromlist_name_4;
        PyObject *tmp_level_name_4;
        tmp_name_name_4 = mod_consts[12];
        tmp_globals_arg_name_4 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_4 = Py_None;
        tmp_fromlist_name_4 = mod_consts[20];
        tmp_level_name_4 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 10;
        tmp_import_name_from_4 = IMPORT_MODULE5(tmp_name_name_4, tmp_globals_arg_name_4, tmp_locals_arg_name_4, tmp_fromlist_name_4, tmp_level_name_4);
        if (tmp_import_name_from_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 10;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_4)) {
            tmp_assign_source_8 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_4,
                (PyObject *)moduledict_click,
                mod_consts[21],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_8 = IMPORT_NAME(tmp_import_name_from_4, mod_consts[21]);
        }

        Py_DECREF(tmp_import_name_from_4);
        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 10;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[21], tmp_assign_source_8);
    }
    {
        PyObject *tmp_assign_source_9;
        PyObject *tmp_import_name_from_5;
        PyObject *tmp_name_name_5;
        PyObject *tmp_globals_arg_name_5;
        PyObject *tmp_locals_arg_name_5;
        PyObject *tmp_fromlist_name_5;
        PyObject *tmp_level_name_5;
        tmp_name_name_5 = mod_consts[12];
        tmp_globals_arg_name_5 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_5 = Py_None;
        tmp_fromlist_name_5 = mod_consts[22];
        tmp_level_name_5 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 11;
        tmp_import_name_from_5 = IMPORT_MODULE5(tmp_name_name_5, tmp_globals_arg_name_5, tmp_locals_arg_name_5, tmp_fromlist_name_5, tmp_level_name_5);
        if (tmp_import_name_from_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 11;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_5)) {
            tmp_assign_source_9 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_5,
                (PyObject *)moduledict_click,
                mod_consts[23],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_9 = IMPORT_NAME(tmp_import_name_from_5, mod_consts[23]);
        }

        Py_DECREF(tmp_import_name_from_5);
        if (tmp_assign_source_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 11;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[23], tmp_assign_source_9);
    }
    {
        PyObject *tmp_assign_source_10;
        PyObject *tmp_import_name_from_6;
        PyObject *tmp_name_name_6;
        PyObject *tmp_globals_arg_name_6;
        PyObject *tmp_locals_arg_name_6;
        PyObject *tmp_fromlist_name_6;
        PyObject *tmp_level_name_6;
        tmp_name_name_6 = mod_consts[12];
        tmp_globals_arg_name_6 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_6 = Py_None;
        tmp_fromlist_name_6 = mod_consts[24];
        tmp_level_name_6 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 12;
        tmp_import_name_from_6 = IMPORT_MODULE5(tmp_name_name_6, tmp_globals_arg_name_6, tmp_locals_arg_name_6, tmp_fromlist_name_6, tmp_level_name_6);
        if (tmp_import_name_from_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 12;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_6)) {
            tmp_assign_source_10 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_6,
                (PyObject *)moduledict_click,
                mod_consts[25],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_10 = IMPORT_NAME(tmp_import_name_from_6, mod_consts[25]);
        }

        Py_DECREF(tmp_import_name_from_6);
        if (tmp_assign_source_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 12;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[25], tmp_assign_source_10);
    }
    {
        PyObject *tmp_assign_source_11;
        PyObject *tmp_import_name_from_7;
        PyObject *tmp_name_name_7;
        PyObject *tmp_globals_arg_name_7;
        PyObject *tmp_locals_arg_name_7;
        PyObject *tmp_fromlist_name_7;
        PyObject *tmp_level_name_7;
        tmp_name_name_7 = mod_consts[12];
        tmp_globals_arg_name_7 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_7 = Py_None;
        tmp_fromlist_name_7 = mod_consts[26];
        tmp_level_name_7 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 13;
        tmp_import_name_from_7 = IMPORT_MODULE5(tmp_name_name_7, tmp_globals_arg_name_7, tmp_locals_arg_name_7, tmp_fromlist_name_7, tmp_level_name_7);
        if (tmp_import_name_from_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 13;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_7)) {
            tmp_assign_source_11 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_7,
                (PyObject *)moduledict_click,
                mod_consts[27],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_11 = IMPORT_NAME(tmp_import_name_from_7, mod_consts[27]);
        }

        Py_DECREF(tmp_import_name_from_7);
        if (tmp_assign_source_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 13;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[27], tmp_assign_source_11);
    }
    {
        PyObject *tmp_assign_source_12;
        PyObject *tmp_import_name_from_8;
        PyObject *tmp_name_name_8;
        PyObject *tmp_globals_arg_name_8;
        PyObject *tmp_locals_arg_name_8;
        PyObject *tmp_fromlist_name_8;
        PyObject *tmp_level_name_8;
        tmp_name_name_8 = mod_consts[12];
        tmp_globals_arg_name_8 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_8 = Py_None;
        tmp_fromlist_name_8 = mod_consts[28];
        tmp_level_name_8 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 14;
        tmp_import_name_from_8 = IMPORT_MODULE5(tmp_name_name_8, tmp_globals_arg_name_8, tmp_locals_arg_name_8, tmp_fromlist_name_8, tmp_level_name_8);
        if (tmp_import_name_from_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_8)) {
            tmp_assign_source_12 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_8,
                (PyObject *)moduledict_click,
                mod_consts[29],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_12 = IMPORT_NAME(tmp_import_name_from_8, mod_consts[29]);
        }

        Py_DECREF(tmp_import_name_from_8);
        if (tmp_assign_source_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[29], tmp_assign_source_12);
    }
    {
        PyObject *tmp_assign_source_13;
        PyObject *tmp_import_name_from_9;
        PyObject *tmp_name_name_9;
        PyObject *tmp_globals_arg_name_9;
        PyObject *tmp_locals_arg_name_9;
        PyObject *tmp_fromlist_name_9;
        PyObject *tmp_level_name_9;
        tmp_name_name_9 = mod_consts[12];
        tmp_globals_arg_name_9 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_9 = Py_None;
        tmp_fromlist_name_9 = mod_consts[30];
        tmp_level_name_9 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 15;
        tmp_import_name_from_9 = IMPORT_MODULE5(tmp_name_name_9, tmp_globals_arg_name_9, tmp_locals_arg_name_9, tmp_fromlist_name_9, tmp_level_name_9);
        if (tmp_import_name_from_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 15;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_9)) {
            tmp_assign_source_13 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_9,
                (PyObject *)moduledict_click,
                mod_consts[31],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_13 = IMPORT_NAME(tmp_import_name_from_9, mod_consts[31]);
        }

        Py_DECREF(tmp_import_name_from_9);
        if (tmp_assign_source_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 15;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[31], tmp_assign_source_13);
    }
    {
        PyObject *tmp_assign_source_14;
        PyObject *tmp_import_name_from_10;
        PyObject *tmp_name_name_10;
        PyObject *tmp_globals_arg_name_10;
        PyObject *tmp_locals_arg_name_10;
        PyObject *tmp_fromlist_name_10;
        PyObject *tmp_level_name_10;
        tmp_name_name_10 = mod_consts[32];
        tmp_globals_arg_name_10 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_10 = Py_None;
        tmp_fromlist_name_10 = mod_consts[33];
        tmp_level_name_10 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 16;
        tmp_import_name_from_10 = IMPORT_MODULE5(tmp_name_name_10, tmp_globals_arg_name_10, tmp_locals_arg_name_10, tmp_fromlist_name_10, tmp_level_name_10);
        if (tmp_import_name_from_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 16;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_10)) {
            tmp_assign_source_14 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_10,
                (PyObject *)moduledict_click,
                mod_consts[34],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_14 = IMPORT_NAME(tmp_import_name_from_10, mod_consts[34]);
        }

        Py_DECREF(tmp_import_name_from_10);
        if (tmp_assign_source_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 16;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[34], tmp_assign_source_14);
    }
    {
        PyObject *tmp_assign_source_15;
        PyObject *tmp_import_name_from_11;
        PyObject *tmp_name_name_11;
        PyObject *tmp_globals_arg_name_11;
        PyObject *tmp_locals_arg_name_11;
        PyObject *tmp_fromlist_name_11;
        PyObject *tmp_level_name_11;
        tmp_name_name_11 = mod_consts[32];
        tmp_globals_arg_name_11 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_11 = Py_None;
        tmp_fromlist_name_11 = mod_consts[35];
        tmp_level_name_11 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 17;
        tmp_import_name_from_11 = IMPORT_MODULE5(tmp_name_name_11, tmp_globals_arg_name_11, tmp_locals_arg_name_11, tmp_fromlist_name_11, tmp_level_name_11);
        if (tmp_import_name_from_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 17;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_11)) {
            tmp_assign_source_15 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_11,
                (PyObject *)moduledict_click,
                mod_consts[36],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_15 = IMPORT_NAME(tmp_import_name_from_11, mod_consts[36]);
        }

        Py_DECREF(tmp_import_name_from_11);
        if (tmp_assign_source_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 17;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[36], tmp_assign_source_15);
    }
    {
        PyObject *tmp_assign_source_16;
        PyObject *tmp_import_name_from_12;
        PyObject *tmp_name_name_12;
        PyObject *tmp_globals_arg_name_12;
        PyObject *tmp_locals_arg_name_12;
        PyObject *tmp_fromlist_name_12;
        PyObject *tmp_level_name_12;
        tmp_name_name_12 = mod_consts[32];
        tmp_globals_arg_name_12 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_12 = Py_None;
        tmp_fromlist_name_12 = mod_consts[37];
        tmp_level_name_12 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 18;
        tmp_import_name_from_12 = IMPORT_MODULE5(tmp_name_name_12, tmp_globals_arg_name_12, tmp_locals_arg_name_12, tmp_fromlist_name_12, tmp_level_name_12);
        if (tmp_import_name_from_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 18;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_12)) {
            tmp_assign_source_16 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_12,
                (PyObject *)moduledict_click,
                mod_consts[38],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_16 = IMPORT_NAME(tmp_import_name_from_12, mod_consts[38]);
        }

        Py_DECREF(tmp_import_name_from_12);
        if (tmp_assign_source_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 18;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[38], tmp_assign_source_16);
    }
    {
        PyObject *tmp_assign_source_17;
        PyObject *tmp_import_name_from_13;
        PyObject *tmp_name_name_13;
        PyObject *tmp_globals_arg_name_13;
        PyObject *tmp_locals_arg_name_13;
        PyObject *tmp_fromlist_name_13;
        PyObject *tmp_level_name_13;
        tmp_name_name_13 = mod_consts[32];
        tmp_globals_arg_name_13 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_13 = Py_None;
        tmp_fromlist_name_13 = mod_consts[39];
        tmp_level_name_13 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 19;
        tmp_import_name_from_13 = IMPORT_MODULE5(tmp_name_name_13, tmp_globals_arg_name_13, tmp_locals_arg_name_13, tmp_fromlist_name_13, tmp_level_name_13);
        if (tmp_import_name_from_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 19;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_13)) {
            tmp_assign_source_17 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_13,
                (PyObject *)moduledict_click,
                mod_consts[40],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_17 = IMPORT_NAME(tmp_import_name_from_13, mod_consts[40]);
        }

        Py_DECREF(tmp_import_name_from_13);
        if (tmp_assign_source_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 19;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[40], tmp_assign_source_17);
    }
    {
        PyObject *tmp_assign_source_18;
        PyObject *tmp_import_name_from_14;
        PyObject *tmp_name_name_14;
        PyObject *tmp_globals_arg_name_14;
        PyObject *tmp_locals_arg_name_14;
        PyObject *tmp_fromlist_name_14;
        PyObject *tmp_level_name_14;
        tmp_name_name_14 = mod_consts[32];
        tmp_globals_arg_name_14 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_14 = Py_None;
        tmp_fromlist_name_14 = mod_consts[41];
        tmp_level_name_14 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 20;
        tmp_import_name_from_14 = IMPORT_MODULE5(tmp_name_name_14, tmp_globals_arg_name_14, tmp_locals_arg_name_14, tmp_fromlist_name_14, tmp_level_name_14);
        if (tmp_import_name_from_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_14)) {
            tmp_assign_source_18 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_14,
                (PyObject *)moduledict_click,
                mod_consts[42],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_18 = IMPORT_NAME(tmp_import_name_from_14, mod_consts[42]);
        }

        Py_DECREF(tmp_import_name_from_14);
        if (tmp_assign_source_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[42], tmp_assign_source_18);
    }
    {
        PyObject *tmp_assign_source_19;
        PyObject *tmp_import_name_from_15;
        PyObject *tmp_name_name_15;
        PyObject *tmp_globals_arg_name_15;
        PyObject *tmp_locals_arg_name_15;
        PyObject *tmp_fromlist_name_15;
        PyObject *tmp_level_name_15;
        tmp_name_name_15 = mod_consts[32];
        tmp_globals_arg_name_15 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_15 = Py_None;
        tmp_fromlist_name_15 = mod_consts[43];
        tmp_level_name_15 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 21;
        tmp_import_name_from_15 = IMPORT_MODULE5(tmp_name_name_15, tmp_globals_arg_name_15, tmp_locals_arg_name_15, tmp_fromlist_name_15, tmp_level_name_15);
        if (tmp_import_name_from_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 21;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_15)) {
            tmp_assign_source_19 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_15,
                (PyObject *)moduledict_click,
                mod_consts[44],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_19 = IMPORT_NAME(tmp_import_name_from_15, mod_consts[44]);
        }

        Py_DECREF(tmp_import_name_from_15);
        if (tmp_assign_source_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 21;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[44], tmp_assign_source_19);
    }
    {
        PyObject *tmp_assign_source_20;
        PyObject *tmp_import_name_from_16;
        PyObject *tmp_name_name_16;
        PyObject *tmp_globals_arg_name_16;
        PyObject *tmp_locals_arg_name_16;
        PyObject *tmp_fromlist_name_16;
        PyObject *tmp_level_name_16;
        tmp_name_name_16 = mod_consts[32];
        tmp_globals_arg_name_16 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_16 = Py_None;
        tmp_fromlist_name_16 = mod_consts[45];
        tmp_level_name_16 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 22;
        tmp_import_name_from_16 = IMPORT_MODULE5(tmp_name_name_16, tmp_globals_arg_name_16, tmp_locals_arg_name_16, tmp_fromlist_name_16, tmp_level_name_16);
        if (tmp_import_name_from_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 22;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_16)) {
            tmp_assign_source_20 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_16,
                (PyObject *)moduledict_click,
                mod_consts[46],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_20 = IMPORT_NAME(tmp_import_name_from_16, mod_consts[46]);
        }

        Py_DECREF(tmp_import_name_from_16);
        if (tmp_assign_source_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 22;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[46], tmp_assign_source_20);
    }
    {
        PyObject *tmp_assign_source_21;
        PyObject *tmp_import_name_from_17;
        PyObject *tmp_name_name_17;
        PyObject *tmp_globals_arg_name_17;
        PyObject *tmp_locals_arg_name_17;
        PyObject *tmp_fromlist_name_17;
        PyObject *tmp_level_name_17;
        tmp_name_name_17 = mod_consts[32];
        tmp_globals_arg_name_17 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_17 = Py_None;
        tmp_fromlist_name_17 = mod_consts[47];
        tmp_level_name_17 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 23;
        tmp_import_name_from_17 = IMPORT_MODULE5(tmp_name_name_17, tmp_globals_arg_name_17, tmp_locals_arg_name_17, tmp_fromlist_name_17, tmp_level_name_17);
        if (tmp_import_name_from_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 23;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_17)) {
            tmp_assign_source_21 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_17,
                (PyObject *)moduledict_click,
                mod_consts[48],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_21 = IMPORT_NAME(tmp_import_name_from_17, mod_consts[48]);
        }

        Py_DECREF(tmp_import_name_from_17);
        if (tmp_assign_source_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 23;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[48], tmp_assign_source_21);
    }
    {
        PyObject *tmp_assign_source_22;
        PyObject *tmp_import_name_from_18;
        PyObject *tmp_name_name_18;
        PyObject *tmp_globals_arg_name_18;
        PyObject *tmp_locals_arg_name_18;
        PyObject *tmp_fromlist_name_18;
        PyObject *tmp_level_name_18;
        tmp_name_name_18 = mod_consts[32];
        tmp_globals_arg_name_18 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_18 = Py_None;
        tmp_fromlist_name_18 = mod_consts[49];
        tmp_level_name_18 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 24;
        tmp_import_name_from_18 = IMPORT_MODULE5(tmp_name_name_18, tmp_globals_arg_name_18, tmp_locals_arg_name_18, tmp_fromlist_name_18, tmp_level_name_18);
        if (tmp_import_name_from_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 24;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_18)) {
            tmp_assign_source_22 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_18,
                (PyObject *)moduledict_click,
                mod_consts[50],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_22 = IMPORT_NAME(tmp_import_name_from_18, mod_consts[50]);
        }

        Py_DECREF(tmp_import_name_from_18);
        if (tmp_assign_source_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 24;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[50], tmp_assign_source_22);
    }
    {
        PyObject *tmp_assign_source_23;
        PyObject *tmp_import_name_from_19;
        PyObject *tmp_name_name_19;
        PyObject *tmp_globals_arg_name_19;
        PyObject *tmp_locals_arg_name_19;
        PyObject *tmp_fromlist_name_19;
        PyObject *tmp_level_name_19;
        tmp_name_name_19 = mod_consts[32];
        tmp_globals_arg_name_19 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_19 = Py_None;
        tmp_fromlist_name_19 = mod_consts[51];
        tmp_level_name_19 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 25;
        tmp_import_name_from_19 = IMPORT_MODULE5(tmp_name_name_19, tmp_globals_arg_name_19, tmp_locals_arg_name_19, tmp_fromlist_name_19, tmp_level_name_19);
        if (tmp_import_name_from_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 25;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_19)) {
            tmp_assign_source_23 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_19,
                (PyObject *)moduledict_click,
                mod_consts[52],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_23 = IMPORT_NAME(tmp_import_name_from_19, mod_consts[52]);
        }

        Py_DECREF(tmp_import_name_from_19);
        if (tmp_assign_source_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 25;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[52], tmp_assign_source_23);
    }
    {
        PyObject *tmp_assign_source_24;
        PyObject *tmp_import_name_from_20;
        PyObject *tmp_name_name_20;
        PyObject *tmp_globals_arg_name_20;
        PyObject *tmp_locals_arg_name_20;
        PyObject *tmp_fromlist_name_20;
        PyObject *tmp_level_name_20;
        tmp_name_name_20 = mod_consts[32];
        tmp_globals_arg_name_20 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_20 = Py_None;
        tmp_fromlist_name_20 = mod_consts[53];
        tmp_level_name_20 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 26;
        tmp_import_name_from_20 = IMPORT_MODULE5(tmp_name_name_20, tmp_globals_arg_name_20, tmp_locals_arg_name_20, tmp_fromlist_name_20, tmp_level_name_20);
        if (tmp_import_name_from_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 26;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_20)) {
            tmp_assign_source_24 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_20,
                (PyObject *)moduledict_click,
                mod_consts[54],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_24 = IMPORT_NAME(tmp_import_name_from_20, mod_consts[54]);
        }

        Py_DECREF(tmp_import_name_from_20);
        if (tmp_assign_source_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 26;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[54], tmp_assign_source_24);
    }
    {
        PyObject *tmp_assign_source_25;
        PyObject *tmp_import_name_from_21;
        PyObject *tmp_name_name_21;
        PyObject *tmp_globals_arg_name_21;
        PyObject *tmp_locals_arg_name_21;
        PyObject *tmp_fromlist_name_21;
        PyObject *tmp_level_name_21;
        tmp_name_name_21 = mod_consts[55];
        tmp_globals_arg_name_21 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_21 = Py_None;
        tmp_fromlist_name_21 = mod_consts[56];
        tmp_level_name_21 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 27;
        tmp_import_name_from_21 = IMPORT_MODULE5(tmp_name_name_21, tmp_globals_arg_name_21, tmp_locals_arg_name_21, tmp_fromlist_name_21, tmp_level_name_21);
        if (tmp_import_name_from_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 27;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_21)) {
            tmp_assign_source_25 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_21,
                (PyObject *)moduledict_click,
                mod_consts[57],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_25 = IMPORT_NAME(tmp_import_name_from_21, mod_consts[57]);
        }

        Py_DECREF(tmp_import_name_from_21);
        if (tmp_assign_source_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 27;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[57], tmp_assign_source_25);
    }
    {
        PyObject *tmp_assign_source_26;
        PyObject *tmp_import_name_from_22;
        PyObject *tmp_name_name_22;
        PyObject *tmp_globals_arg_name_22;
        PyObject *tmp_locals_arg_name_22;
        PyObject *tmp_fromlist_name_22;
        PyObject *tmp_level_name_22;
        tmp_name_name_22 = mod_consts[55];
        tmp_globals_arg_name_22 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_22 = Py_None;
        tmp_fromlist_name_22 = mod_consts[58];
        tmp_level_name_22 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 28;
        tmp_import_name_from_22 = IMPORT_MODULE5(tmp_name_name_22, tmp_globals_arg_name_22, tmp_locals_arg_name_22, tmp_fromlist_name_22, tmp_level_name_22);
        if (tmp_import_name_from_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 28;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_22)) {
            tmp_assign_source_26 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_22,
                (PyObject *)moduledict_click,
                mod_consts[59],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_26 = IMPORT_NAME(tmp_import_name_from_22, mod_consts[59]);
        }

        Py_DECREF(tmp_import_name_from_22);
        if (tmp_assign_source_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 28;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[59], tmp_assign_source_26);
    }
    {
        PyObject *tmp_assign_source_27;
        PyObject *tmp_import_name_from_23;
        PyObject *tmp_name_name_23;
        PyObject *tmp_globals_arg_name_23;
        PyObject *tmp_locals_arg_name_23;
        PyObject *tmp_fromlist_name_23;
        PyObject *tmp_level_name_23;
        tmp_name_name_23 = mod_consts[55];
        tmp_globals_arg_name_23 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_23 = Py_None;
        tmp_fromlist_name_23 = mod_consts[60];
        tmp_level_name_23 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 29;
        tmp_import_name_from_23 = IMPORT_MODULE5(tmp_name_name_23, tmp_globals_arg_name_23, tmp_locals_arg_name_23, tmp_fromlist_name_23, tmp_level_name_23);
        if (tmp_import_name_from_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_23)) {
            tmp_assign_source_27 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_23,
                (PyObject *)moduledict_click,
                mod_consts[61],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_27 = IMPORT_NAME(tmp_import_name_from_23, mod_consts[61]);
        }

        Py_DECREF(tmp_import_name_from_23);
        if (tmp_assign_source_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[61], tmp_assign_source_27);
    }
    {
        PyObject *tmp_assign_source_28;
        PyObject *tmp_import_name_from_24;
        PyObject *tmp_name_name_24;
        PyObject *tmp_globals_arg_name_24;
        PyObject *tmp_locals_arg_name_24;
        PyObject *tmp_fromlist_name_24;
        PyObject *tmp_level_name_24;
        tmp_name_name_24 = mod_consts[55];
        tmp_globals_arg_name_24 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_24 = Py_None;
        tmp_fromlist_name_24 = mod_consts[62];
        tmp_level_name_24 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 30;
        tmp_import_name_from_24 = IMPORT_MODULE5(tmp_name_name_24, tmp_globals_arg_name_24, tmp_locals_arg_name_24, tmp_fromlist_name_24, tmp_level_name_24);
        if (tmp_import_name_from_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 30;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_24)) {
            tmp_assign_source_28 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_24,
                (PyObject *)moduledict_click,
                mod_consts[63],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_28 = IMPORT_NAME(tmp_import_name_from_24, mod_consts[63]);
        }

        Py_DECREF(tmp_import_name_from_24);
        if (tmp_assign_source_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 30;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[63], tmp_assign_source_28);
    }
    {
        PyObject *tmp_assign_source_29;
        PyObject *tmp_import_name_from_25;
        PyObject *tmp_name_name_25;
        PyObject *tmp_globals_arg_name_25;
        PyObject *tmp_locals_arg_name_25;
        PyObject *tmp_fromlist_name_25;
        PyObject *tmp_level_name_25;
        tmp_name_name_25 = mod_consts[55];
        tmp_globals_arg_name_25 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_25 = Py_None;
        tmp_fromlist_name_25 = mod_consts[64];
        tmp_level_name_25 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 31;
        tmp_import_name_from_25 = IMPORT_MODULE5(tmp_name_name_25, tmp_globals_arg_name_25, tmp_locals_arg_name_25, tmp_fromlist_name_25, tmp_level_name_25);
        if (tmp_import_name_from_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 31;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_25)) {
            tmp_assign_source_29 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_25,
                (PyObject *)moduledict_click,
                mod_consts[65],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_29 = IMPORT_NAME(tmp_import_name_from_25, mod_consts[65]);
        }

        Py_DECREF(tmp_import_name_from_25);
        if (tmp_assign_source_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 31;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[65], tmp_assign_source_29);
    }
    {
        PyObject *tmp_assign_source_30;
        PyObject *tmp_import_name_from_26;
        PyObject *tmp_name_name_26;
        PyObject *tmp_globals_arg_name_26;
        PyObject *tmp_locals_arg_name_26;
        PyObject *tmp_fromlist_name_26;
        PyObject *tmp_level_name_26;
        tmp_name_name_26 = mod_consts[55];
        tmp_globals_arg_name_26 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_26 = Py_None;
        tmp_fromlist_name_26 = mod_consts[66];
        tmp_level_name_26 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 32;
        tmp_import_name_from_26 = IMPORT_MODULE5(tmp_name_name_26, tmp_globals_arg_name_26, tmp_locals_arg_name_26, tmp_fromlist_name_26, tmp_level_name_26);
        if (tmp_import_name_from_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_26)) {
            tmp_assign_source_30 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_26,
                (PyObject *)moduledict_click,
                mod_consts[67],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_30 = IMPORT_NAME(tmp_import_name_from_26, mod_consts[67]);
        }

        Py_DECREF(tmp_import_name_from_26);
        if (tmp_assign_source_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[67], tmp_assign_source_30);
    }
    {
        PyObject *tmp_assign_source_31;
        PyObject *tmp_import_name_from_27;
        PyObject *tmp_name_name_27;
        PyObject *tmp_globals_arg_name_27;
        PyObject *tmp_locals_arg_name_27;
        PyObject *tmp_fromlist_name_27;
        PyObject *tmp_level_name_27;
        tmp_name_name_27 = mod_consts[55];
        tmp_globals_arg_name_27 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_27 = Py_None;
        tmp_fromlist_name_27 = mod_consts[68];
        tmp_level_name_27 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 33;
        tmp_import_name_from_27 = IMPORT_MODULE5(tmp_name_name_27, tmp_globals_arg_name_27, tmp_locals_arg_name_27, tmp_fromlist_name_27, tmp_level_name_27);
        if (tmp_import_name_from_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 33;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_27)) {
            tmp_assign_source_31 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_27,
                (PyObject *)moduledict_click,
                mod_consts[69],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_31 = IMPORT_NAME(tmp_import_name_from_27, mod_consts[69]);
        }

        Py_DECREF(tmp_import_name_from_27);
        if (tmp_assign_source_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 33;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[69], tmp_assign_source_31);
    }
    {
        PyObject *tmp_assign_source_32;
        PyObject *tmp_import_name_from_28;
        PyObject *tmp_name_name_28;
        PyObject *tmp_globals_arg_name_28;
        PyObject *tmp_locals_arg_name_28;
        PyObject *tmp_fromlist_name_28;
        PyObject *tmp_level_name_28;
        tmp_name_name_28 = mod_consts[55];
        tmp_globals_arg_name_28 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_28 = Py_None;
        tmp_fromlist_name_28 = mod_consts[70];
        tmp_level_name_28 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 34;
        tmp_import_name_from_28 = IMPORT_MODULE5(tmp_name_name_28, tmp_globals_arg_name_28, tmp_locals_arg_name_28, tmp_fromlist_name_28, tmp_level_name_28);
        if (tmp_import_name_from_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 34;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_28)) {
            tmp_assign_source_32 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_28,
                (PyObject *)moduledict_click,
                mod_consts[71],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_32 = IMPORT_NAME(tmp_import_name_from_28, mod_consts[71]);
        }

        Py_DECREF(tmp_import_name_from_28);
        if (tmp_assign_source_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 34;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[71], tmp_assign_source_32);
    }
    {
        PyObject *tmp_assign_source_33;
        PyObject *tmp_import_name_from_29;
        PyObject *tmp_name_name_29;
        PyObject *tmp_globals_arg_name_29;
        PyObject *tmp_locals_arg_name_29;
        PyObject *tmp_fromlist_name_29;
        PyObject *tmp_level_name_29;
        tmp_name_name_29 = mod_consts[55];
        tmp_globals_arg_name_29 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_29 = Py_None;
        tmp_fromlist_name_29 = mod_consts[72];
        tmp_level_name_29 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 35;
        tmp_import_name_from_29 = IMPORT_MODULE5(tmp_name_name_29, tmp_globals_arg_name_29, tmp_locals_arg_name_29, tmp_fromlist_name_29, tmp_level_name_29);
        if (tmp_import_name_from_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_29)) {
            tmp_assign_source_33 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_29,
                (PyObject *)moduledict_click,
                mod_consts[73],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_33 = IMPORT_NAME(tmp_import_name_from_29, mod_consts[73]);
        }

        Py_DECREF(tmp_import_name_from_29);
        if (tmp_assign_source_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[73], tmp_assign_source_33);
    }
    {
        PyObject *tmp_assign_source_34;
        PyObject *tmp_import_name_from_30;
        PyObject *tmp_name_name_30;
        PyObject *tmp_globals_arg_name_30;
        PyObject *tmp_locals_arg_name_30;
        PyObject *tmp_fromlist_name_30;
        PyObject *tmp_level_name_30;
        tmp_name_name_30 = mod_consts[74];
        tmp_globals_arg_name_30 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_30 = Py_None;
        tmp_fromlist_name_30 = mod_consts[75];
        tmp_level_name_30 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 36;
        tmp_import_name_from_30 = IMPORT_MODULE5(tmp_name_name_30, tmp_globals_arg_name_30, tmp_locals_arg_name_30, tmp_fromlist_name_30, tmp_level_name_30);
        if (tmp_import_name_from_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 36;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_30)) {
            tmp_assign_source_34 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_30,
                (PyObject *)moduledict_click,
                mod_consts[76],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_34 = IMPORT_NAME(tmp_import_name_from_30, mod_consts[76]);
        }

        Py_DECREF(tmp_import_name_from_30);
        if (tmp_assign_source_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 36;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[76], tmp_assign_source_34);
    }
    {
        PyObject *tmp_assign_source_35;
        PyObject *tmp_import_name_from_31;
        PyObject *tmp_name_name_31;
        PyObject *tmp_globals_arg_name_31;
        PyObject *tmp_locals_arg_name_31;
        PyObject *tmp_fromlist_name_31;
        PyObject *tmp_level_name_31;
        tmp_name_name_31 = mod_consts[74];
        tmp_globals_arg_name_31 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_31 = Py_None;
        tmp_fromlist_name_31 = mod_consts[77];
        tmp_level_name_31 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 37;
        tmp_import_name_from_31 = IMPORT_MODULE5(tmp_name_name_31, tmp_globals_arg_name_31, tmp_locals_arg_name_31, tmp_fromlist_name_31, tmp_level_name_31);
        if (tmp_import_name_from_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 37;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_31)) {
            tmp_assign_source_35 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_31,
                (PyObject *)moduledict_click,
                mod_consts[78],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_35 = IMPORT_NAME(tmp_import_name_from_31, mod_consts[78]);
        }

        Py_DECREF(tmp_import_name_from_31);
        if (tmp_assign_source_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 37;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[78], tmp_assign_source_35);
    }
    {
        PyObject *tmp_assign_source_36;
        PyObject *tmp_import_name_from_32;
        PyObject *tmp_name_name_32;
        PyObject *tmp_globals_arg_name_32;
        PyObject *tmp_locals_arg_name_32;
        PyObject *tmp_fromlist_name_32;
        PyObject *tmp_level_name_32;
        tmp_name_name_32 = mod_consts[79];
        tmp_globals_arg_name_32 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_32 = Py_None;
        tmp_fromlist_name_32 = mod_consts[80];
        tmp_level_name_32 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 38;
        tmp_import_name_from_32 = IMPORT_MODULE5(tmp_name_name_32, tmp_globals_arg_name_32, tmp_locals_arg_name_32, tmp_fromlist_name_32, tmp_level_name_32);
        if (tmp_import_name_from_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 38;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_32)) {
            tmp_assign_source_36 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_32,
                (PyObject *)moduledict_click,
                mod_consts[81],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_36 = IMPORT_NAME(tmp_import_name_from_32, mod_consts[81]);
        }

        Py_DECREF(tmp_import_name_from_32);
        if (tmp_assign_source_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 38;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[81], tmp_assign_source_36);
    }
    {
        PyObject *tmp_assign_source_37;
        PyObject *tmp_import_name_from_33;
        PyObject *tmp_name_name_33;
        PyObject *tmp_globals_arg_name_33;
        PyObject *tmp_locals_arg_name_33;
        PyObject *tmp_fromlist_name_33;
        PyObject *tmp_level_name_33;
        tmp_name_name_33 = mod_consts[82];
        tmp_globals_arg_name_33 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_33 = Py_None;
        tmp_fromlist_name_33 = mod_consts[83];
        tmp_level_name_33 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 39;
        tmp_import_name_from_33 = IMPORT_MODULE5(tmp_name_name_33, tmp_globals_arg_name_33, tmp_locals_arg_name_33, tmp_fromlist_name_33, tmp_level_name_33);
        if (tmp_import_name_from_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 39;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_33)) {
            tmp_assign_source_37 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_33,
                (PyObject *)moduledict_click,
                mod_consts[84],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_37 = IMPORT_NAME(tmp_import_name_from_33, mod_consts[84]);
        }

        Py_DECREF(tmp_import_name_from_33);
        if (tmp_assign_source_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 39;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[84], tmp_assign_source_37);
    }
    {
        PyObject *tmp_assign_source_38;
        PyObject *tmp_import_name_from_34;
        PyObject *tmp_name_name_34;
        PyObject *tmp_globals_arg_name_34;
        PyObject *tmp_locals_arg_name_34;
        PyObject *tmp_fromlist_name_34;
        PyObject *tmp_level_name_34;
        tmp_name_name_34 = mod_consts[85];
        tmp_globals_arg_name_34 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_34 = Py_None;
        tmp_fromlist_name_34 = mod_consts[86];
        tmp_level_name_34 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 40;
        tmp_import_name_from_34 = IMPORT_MODULE5(tmp_name_name_34, tmp_globals_arg_name_34, tmp_locals_arg_name_34, tmp_fromlist_name_34, tmp_level_name_34);
        if (tmp_import_name_from_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 40;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_34)) {
            tmp_assign_source_38 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_34,
                (PyObject *)moduledict_click,
                mod_consts[87],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_38 = IMPORT_NAME(tmp_import_name_from_34, mod_consts[87]);
        }

        Py_DECREF(tmp_import_name_from_34);
        if (tmp_assign_source_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 40;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[87], tmp_assign_source_38);
    }
    {
        PyObject *tmp_assign_source_39;
        PyObject *tmp_import_name_from_35;
        PyObject *tmp_name_name_35;
        PyObject *tmp_globals_arg_name_35;
        PyObject *tmp_locals_arg_name_35;
        PyObject *tmp_fromlist_name_35;
        PyObject *tmp_level_name_35;
        tmp_name_name_35 = mod_consts[85];
        tmp_globals_arg_name_35 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_35 = Py_None;
        tmp_fromlist_name_35 = mod_consts[88];
        tmp_level_name_35 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 41;
        tmp_import_name_from_35 = IMPORT_MODULE5(tmp_name_name_35, tmp_globals_arg_name_35, tmp_locals_arg_name_35, tmp_fromlist_name_35, tmp_level_name_35);
        if (tmp_import_name_from_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 41;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_35)) {
            tmp_assign_source_39 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_35,
                (PyObject *)moduledict_click,
                mod_consts[89],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_39 = IMPORT_NAME(tmp_import_name_from_35, mod_consts[89]);
        }

        Py_DECREF(tmp_import_name_from_35);
        if (tmp_assign_source_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 41;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[89], tmp_assign_source_39);
    }
    {
        PyObject *tmp_assign_source_40;
        PyObject *tmp_import_name_from_36;
        PyObject *tmp_name_name_36;
        PyObject *tmp_globals_arg_name_36;
        PyObject *tmp_locals_arg_name_36;
        PyObject *tmp_fromlist_name_36;
        PyObject *tmp_level_name_36;
        tmp_name_name_36 = mod_consts[85];
        tmp_globals_arg_name_36 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_36 = Py_None;
        tmp_fromlist_name_36 = mod_consts[90];
        tmp_level_name_36 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 42;
        tmp_import_name_from_36 = IMPORT_MODULE5(tmp_name_name_36, tmp_globals_arg_name_36, tmp_locals_arg_name_36, tmp_fromlist_name_36, tmp_level_name_36);
        if (tmp_import_name_from_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 42;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_36)) {
            tmp_assign_source_40 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_36,
                (PyObject *)moduledict_click,
                mod_consts[91],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_40 = IMPORT_NAME(tmp_import_name_from_36, mod_consts[91]);
        }

        Py_DECREF(tmp_import_name_from_36);
        if (tmp_assign_source_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 42;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[91], tmp_assign_source_40);
    }
    {
        PyObject *tmp_assign_source_41;
        PyObject *tmp_import_name_from_37;
        PyObject *tmp_name_name_37;
        PyObject *tmp_globals_arg_name_37;
        PyObject *tmp_locals_arg_name_37;
        PyObject *tmp_fromlist_name_37;
        PyObject *tmp_level_name_37;
        tmp_name_name_37 = mod_consts[85];
        tmp_globals_arg_name_37 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_37 = Py_None;
        tmp_fromlist_name_37 = mod_consts[92];
        tmp_level_name_37 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 43;
        tmp_import_name_from_37 = IMPORT_MODULE5(tmp_name_name_37, tmp_globals_arg_name_37, tmp_locals_arg_name_37, tmp_fromlist_name_37, tmp_level_name_37);
        if (tmp_import_name_from_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 43;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_37)) {
            tmp_assign_source_41 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_37,
                (PyObject *)moduledict_click,
                mod_consts[93],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_41 = IMPORT_NAME(tmp_import_name_from_37, mod_consts[93]);
        }

        Py_DECREF(tmp_import_name_from_37);
        if (tmp_assign_source_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 43;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[93], tmp_assign_source_41);
    }
    {
        PyObject *tmp_assign_source_42;
        PyObject *tmp_import_name_from_38;
        PyObject *tmp_name_name_38;
        PyObject *tmp_globals_arg_name_38;
        PyObject *tmp_locals_arg_name_38;
        PyObject *tmp_fromlist_name_38;
        PyObject *tmp_level_name_38;
        tmp_name_name_38 = mod_consts[85];
        tmp_globals_arg_name_38 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_38 = Py_None;
        tmp_fromlist_name_38 = mod_consts[94];
        tmp_level_name_38 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 44;
        tmp_import_name_from_38 = IMPORT_MODULE5(tmp_name_name_38, tmp_globals_arg_name_38, tmp_locals_arg_name_38, tmp_fromlist_name_38, tmp_level_name_38);
        if (tmp_import_name_from_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_38)) {
            tmp_assign_source_42 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_38,
                (PyObject *)moduledict_click,
                mod_consts[95],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_42 = IMPORT_NAME(tmp_import_name_from_38, mod_consts[95]);
        }

        Py_DECREF(tmp_import_name_from_38);
        if (tmp_assign_source_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[95], tmp_assign_source_42);
    }
    {
        PyObject *tmp_assign_source_43;
        PyObject *tmp_import_name_from_39;
        PyObject *tmp_name_name_39;
        PyObject *tmp_globals_arg_name_39;
        PyObject *tmp_locals_arg_name_39;
        PyObject *tmp_fromlist_name_39;
        PyObject *tmp_level_name_39;
        tmp_name_name_39 = mod_consts[85];
        tmp_globals_arg_name_39 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_39 = Py_None;
        tmp_fromlist_name_39 = mod_consts[96];
        tmp_level_name_39 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 45;
        tmp_import_name_from_39 = IMPORT_MODULE5(tmp_name_name_39, tmp_globals_arg_name_39, tmp_locals_arg_name_39, tmp_fromlist_name_39, tmp_level_name_39);
        if (tmp_import_name_from_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 45;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_39)) {
            tmp_assign_source_43 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_39,
                (PyObject *)moduledict_click,
                mod_consts[97],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_43 = IMPORT_NAME(tmp_import_name_from_39, mod_consts[97]);
        }

        Py_DECREF(tmp_import_name_from_39);
        if (tmp_assign_source_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 45;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[97], tmp_assign_source_43);
    }
    {
        PyObject *tmp_assign_source_44;
        PyObject *tmp_import_name_from_40;
        PyObject *tmp_name_name_40;
        PyObject *tmp_globals_arg_name_40;
        PyObject *tmp_locals_arg_name_40;
        PyObject *tmp_fromlist_name_40;
        PyObject *tmp_level_name_40;
        tmp_name_name_40 = mod_consts[85];
        tmp_globals_arg_name_40 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_40 = Py_None;
        tmp_fromlist_name_40 = mod_consts[98];
        tmp_level_name_40 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 46;
        tmp_import_name_from_40 = IMPORT_MODULE5(tmp_name_name_40, tmp_globals_arg_name_40, tmp_locals_arg_name_40, tmp_fromlist_name_40, tmp_level_name_40);
        if (tmp_import_name_from_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 46;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_40)) {
            tmp_assign_source_44 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_40,
                (PyObject *)moduledict_click,
                mod_consts[99],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_44 = IMPORT_NAME(tmp_import_name_from_40, mod_consts[99]);
        }

        Py_DECREF(tmp_import_name_from_40);
        if (tmp_assign_source_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 46;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[99], tmp_assign_source_44);
    }
    {
        PyObject *tmp_assign_source_45;
        PyObject *tmp_import_name_from_41;
        PyObject *tmp_name_name_41;
        PyObject *tmp_globals_arg_name_41;
        PyObject *tmp_locals_arg_name_41;
        PyObject *tmp_fromlist_name_41;
        PyObject *tmp_level_name_41;
        tmp_name_name_41 = mod_consts[85];
        tmp_globals_arg_name_41 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_41 = Py_None;
        tmp_fromlist_name_41 = mod_consts[100];
        tmp_level_name_41 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 47;
        tmp_import_name_from_41 = IMPORT_MODULE5(tmp_name_name_41, tmp_globals_arg_name_41, tmp_locals_arg_name_41, tmp_fromlist_name_41, tmp_level_name_41);
        if (tmp_import_name_from_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 47;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_41)) {
            tmp_assign_source_45 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_41,
                (PyObject *)moduledict_click,
                mod_consts[101],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_45 = IMPORT_NAME(tmp_import_name_from_41, mod_consts[101]);
        }

        Py_DECREF(tmp_import_name_from_41);
        if (tmp_assign_source_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 47;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[101], tmp_assign_source_45);
    }
    {
        PyObject *tmp_assign_source_46;
        PyObject *tmp_import_name_from_42;
        PyObject *tmp_name_name_42;
        PyObject *tmp_globals_arg_name_42;
        PyObject *tmp_locals_arg_name_42;
        PyObject *tmp_fromlist_name_42;
        PyObject *tmp_level_name_42;
        tmp_name_name_42 = mod_consts[85];
        tmp_globals_arg_name_42 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_42 = Py_None;
        tmp_fromlist_name_42 = mod_consts[102];
        tmp_level_name_42 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 48;
        tmp_import_name_from_42 = IMPORT_MODULE5(tmp_name_name_42, tmp_globals_arg_name_42, tmp_locals_arg_name_42, tmp_fromlist_name_42, tmp_level_name_42);
        if (tmp_import_name_from_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 48;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_42)) {
            tmp_assign_source_46 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_42,
                (PyObject *)moduledict_click,
                mod_consts[103],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_46 = IMPORT_NAME(tmp_import_name_from_42, mod_consts[103]);
        }

        Py_DECREF(tmp_import_name_from_42);
        if (tmp_assign_source_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 48;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[103], tmp_assign_source_46);
    }
    {
        PyObject *tmp_assign_source_47;
        PyObject *tmp_import_name_from_43;
        PyObject *tmp_name_name_43;
        PyObject *tmp_globals_arg_name_43;
        PyObject *tmp_locals_arg_name_43;
        PyObject *tmp_fromlist_name_43;
        PyObject *tmp_level_name_43;
        tmp_name_name_43 = mod_consts[85];
        tmp_globals_arg_name_43 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_43 = Py_None;
        tmp_fromlist_name_43 = mod_consts[104];
        tmp_level_name_43 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 49;
        tmp_import_name_from_43 = IMPORT_MODULE5(tmp_name_name_43, tmp_globals_arg_name_43, tmp_locals_arg_name_43, tmp_fromlist_name_43, tmp_level_name_43);
        if (tmp_import_name_from_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 49;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_43)) {
            tmp_assign_source_47 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_43,
                (PyObject *)moduledict_click,
                mod_consts[105],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_47 = IMPORT_NAME(tmp_import_name_from_43, mod_consts[105]);
        }

        Py_DECREF(tmp_import_name_from_43);
        if (tmp_assign_source_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 49;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[105], tmp_assign_source_47);
    }
    {
        PyObject *tmp_assign_source_48;
        PyObject *tmp_import_name_from_44;
        PyObject *tmp_name_name_44;
        PyObject *tmp_globals_arg_name_44;
        PyObject *tmp_locals_arg_name_44;
        PyObject *tmp_fromlist_name_44;
        PyObject *tmp_level_name_44;
        tmp_name_name_44 = mod_consts[85];
        tmp_globals_arg_name_44 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_44 = Py_None;
        tmp_fromlist_name_44 = mod_consts[106];
        tmp_level_name_44 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 50;
        tmp_import_name_from_44 = IMPORT_MODULE5(tmp_name_name_44, tmp_globals_arg_name_44, tmp_locals_arg_name_44, tmp_fromlist_name_44, tmp_level_name_44);
        if (tmp_import_name_from_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_44)) {
            tmp_assign_source_48 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_44,
                (PyObject *)moduledict_click,
                mod_consts[107],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_48 = IMPORT_NAME(tmp_import_name_from_44, mod_consts[107]);
        }

        Py_DECREF(tmp_import_name_from_44);
        if (tmp_assign_source_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[107], tmp_assign_source_48);
    }
    {
        PyObject *tmp_assign_source_49;
        PyObject *tmp_import_name_from_45;
        PyObject *tmp_name_name_45;
        PyObject *tmp_globals_arg_name_45;
        PyObject *tmp_locals_arg_name_45;
        PyObject *tmp_fromlist_name_45;
        PyObject *tmp_level_name_45;
        tmp_name_name_45 = mod_consts[85];
        tmp_globals_arg_name_45 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_45 = Py_None;
        tmp_fromlist_name_45 = mod_consts[108];
        tmp_level_name_45 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 51;
        tmp_import_name_from_45 = IMPORT_MODULE5(tmp_name_name_45, tmp_globals_arg_name_45, tmp_locals_arg_name_45, tmp_fromlist_name_45, tmp_level_name_45);
        if (tmp_import_name_from_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 51;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_45)) {
            tmp_assign_source_49 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_45,
                (PyObject *)moduledict_click,
                mod_consts[109],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_49 = IMPORT_NAME(tmp_import_name_from_45, mod_consts[109]);
        }

        Py_DECREF(tmp_import_name_from_45);
        if (tmp_assign_source_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 51;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[109], tmp_assign_source_49);
    }
    {
        PyObject *tmp_assign_source_50;
        PyObject *tmp_import_name_from_46;
        PyObject *tmp_name_name_46;
        PyObject *tmp_globals_arg_name_46;
        PyObject *tmp_locals_arg_name_46;
        PyObject *tmp_fromlist_name_46;
        PyObject *tmp_level_name_46;
        tmp_name_name_46 = mod_consts[85];
        tmp_globals_arg_name_46 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_46 = Py_None;
        tmp_fromlist_name_46 = mod_consts[110];
        tmp_level_name_46 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 52;
        tmp_import_name_from_46 = IMPORT_MODULE5(tmp_name_name_46, tmp_globals_arg_name_46, tmp_locals_arg_name_46, tmp_fromlist_name_46, tmp_level_name_46);
        if (tmp_import_name_from_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 52;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_46)) {
            tmp_assign_source_50 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_46,
                (PyObject *)moduledict_click,
                mod_consts[111],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_50 = IMPORT_NAME(tmp_import_name_from_46, mod_consts[111]);
        }

        Py_DECREF(tmp_import_name_from_46);
        if (tmp_assign_source_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 52;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[111], tmp_assign_source_50);
    }
    {
        PyObject *tmp_assign_source_51;
        PyObject *tmp_import_name_from_47;
        PyObject *tmp_name_name_47;
        PyObject *tmp_globals_arg_name_47;
        PyObject *tmp_locals_arg_name_47;
        PyObject *tmp_fromlist_name_47;
        PyObject *tmp_level_name_47;
        tmp_name_name_47 = mod_consts[112];
        tmp_globals_arg_name_47 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_47 = Py_None;
        tmp_fromlist_name_47 = mod_consts[113];
        tmp_level_name_47 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 53;
        tmp_import_name_from_47 = IMPORT_MODULE5(tmp_name_name_47, tmp_globals_arg_name_47, tmp_locals_arg_name_47, tmp_fromlist_name_47, tmp_level_name_47);
        if (tmp_import_name_from_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 53;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_47)) {
            tmp_assign_source_51 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_47,
                (PyObject *)moduledict_click,
                mod_consts[114],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_51 = IMPORT_NAME(tmp_import_name_from_47, mod_consts[114]);
        }

        Py_DECREF(tmp_import_name_from_47);
        if (tmp_assign_source_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 53;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[114], tmp_assign_source_51);
    }
    {
        PyObject *tmp_assign_source_52;
        PyObject *tmp_import_name_from_48;
        PyObject *tmp_name_name_48;
        PyObject *tmp_globals_arg_name_48;
        PyObject *tmp_locals_arg_name_48;
        PyObject *tmp_fromlist_name_48;
        PyObject *tmp_level_name_48;
        tmp_name_name_48 = mod_consts[112];
        tmp_globals_arg_name_48 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_48 = Py_None;
        tmp_fromlist_name_48 = mod_consts[115];
        tmp_level_name_48 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 54;
        tmp_import_name_from_48 = IMPORT_MODULE5(tmp_name_name_48, tmp_globals_arg_name_48, tmp_locals_arg_name_48, tmp_fromlist_name_48, tmp_level_name_48);
        if (tmp_import_name_from_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 54;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_48)) {
            tmp_assign_source_52 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_48,
                (PyObject *)moduledict_click,
                mod_consts[116],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_52 = IMPORT_NAME(tmp_import_name_from_48, mod_consts[116]);
        }

        Py_DECREF(tmp_import_name_from_48);
        if (tmp_assign_source_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 54;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[116], tmp_assign_source_52);
    }
    {
        PyObject *tmp_assign_source_53;
        PyObject *tmp_import_name_from_49;
        PyObject *tmp_name_name_49;
        PyObject *tmp_globals_arg_name_49;
        PyObject *tmp_locals_arg_name_49;
        PyObject *tmp_fromlist_name_49;
        PyObject *tmp_level_name_49;
        tmp_name_name_49 = mod_consts[112];
        tmp_globals_arg_name_49 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_49 = Py_None;
        tmp_fromlist_name_49 = mod_consts[117];
        tmp_level_name_49 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 55;
        tmp_import_name_from_49 = IMPORT_MODULE5(tmp_name_name_49, tmp_globals_arg_name_49, tmp_locals_arg_name_49, tmp_fromlist_name_49, tmp_level_name_49);
        if (tmp_import_name_from_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 55;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_49)) {
            tmp_assign_source_53 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_49,
                (PyObject *)moduledict_click,
                mod_consts[118],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_53 = IMPORT_NAME(tmp_import_name_from_49, mod_consts[118]);
        }

        Py_DECREF(tmp_import_name_from_49);
        if (tmp_assign_source_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 55;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[118], tmp_assign_source_53);
    }
    {
        PyObject *tmp_assign_source_54;
        PyObject *tmp_import_name_from_50;
        PyObject *tmp_name_name_50;
        PyObject *tmp_globals_arg_name_50;
        PyObject *tmp_locals_arg_name_50;
        PyObject *tmp_fromlist_name_50;
        PyObject *tmp_level_name_50;
        tmp_name_name_50 = mod_consts[112];
        tmp_globals_arg_name_50 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_50 = Py_None;
        tmp_fromlist_name_50 = mod_consts[119];
        tmp_level_name_50 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 56;
        tmp_import_name_from_50 = IMPORT_MODULE5(tmp_name_name_50, tmp_globals_arg_name_50, tmp_locals_arg_name_50, tmp_fromlist_name_50, tmp_level_name_50);
        if (tmp_import_name_from_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 56;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_50)) {
            tmp_assign_source_54 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_50,
                (PyObject *)moduledict_click,
                mod_consts[120],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_54 = IMPORT_NAME(tmp_import_name_from_50, mod_consts[120]);
        }

        Py_DECREF(tmp_import_name_from_50);
        if (tmp_assign_source_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 56;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[120], tmp_assign_source_54);
    }
    {
        PyObject *tmp_assign_source_55;
        PyObject *tmp_import_name_from_51;
        PyObject *tmp_name_name_51;
        PyObject *tmp_globals_arg_name_51;
        PyObject *tmp_locals_arg_name_51;
        PyObject *tmp_fromlist_name_51;
        PyObject *tmp_level_name_51;
        tmp_name_name_51 = mod_consts[112];
        tmp_globals_arg_name_51 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_51 = Py_None;
        tmp_fromlist_name_51 = mod_consts[121];
        tmp_level_name_51 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 57;
        tmp_import_name_from_51 = IMPORT_MODULE5(tmp_name_name_51, tmp_globals_arg_name_51, tmp_locals_arg_name_51, tmp_fromlist_name_51, tmp_level_name_51);
        if (tmp_import_name_from_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 57;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_51)) {
            tmp_assign_source_55 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_51,
                (PyObject *)moduledict_click,
                mod_consts[122],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_55 = IMPORT_NAME(tmp_import_name_from_51, mod_consts[122]);
        }

        Py_DECREF(tmp_import_name_from_51);
        if (tmp_assign_source_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 57;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[122], tmp_assign_source_55);
    }
    {
        PyObject *tmp_assign_source_56;
        PyObject *tmp_import_name_from_52;
        PyObject *tmp_name_name_52;
        PyObject *tmp_globals_arg_name_52;
        PyObject *tmp_locals_arg_name_52;
        PyObject *tmp_fromlist_name_52;
        PyObject *tmp_level_name_52;
        tmp_name_name_52 = mod_consts[112];
        tmp_globals_arg_name_52 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_52 = Py_None;
        tmp_fromlist_name_52 = mod_consts[123];
        tmp_level_name_52 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 58;
        tmp_import_name_from_52 = IMPORT_MODULE5(tmp_name_name_52, tmp_globals_arg_name_52, tmp_locals_arg_name_52, tmp_fromlist_name_52, tmp_level_name_52);
        if (tmp_import_name_from_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_52)) {
            tmp_assign_source_56 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_52,
                (PyObject *)moduledict_click,
                mod_consts[124],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_56 = IMPORT_NAME(tmp_import_name_from_52, mod_consts[124]);
        }

        Py_DECREF(tmp_import_name_from_52);
        if (tmp_assign_source_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[124], tmp_assign_source_56);
    }
    {
        PyObject *tmp_assign_source_57;
        PyObject *tmp_import_name_from_53;
        PyObject *tmp_name_name_53;
        PyObject *tmp_globals_arg_name_53;
        PyObject *tmp_locals_arg_name_53;
        PyObject *tmp_fromlist_name_53;
        PyObject *tmp_level_name_53;
        tmp_name_name_53 = mod_consts[112];
        tmp_globals_arg_name_53 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_53 = Py_None;
        tmp_fromlist_name_53 = mod_consts[125];
        tmp_level_name_53 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 59;
        tmp_import_name_from_53 = IMPORT_MODULE5(tmp_name_name_53, tmp_globals_arg_name_53, tmp_locals_arg_name_53, tmp_fromlist_name_53, tmp_level_name_53);
        if (tmp_import_name_from_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_53)) {
            tmp_assign_source_57 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_53,
                (PyObject *)moduledict_click,
                mod_consts[126],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_57 = IMPORT_NAME(tmp_import_name_from_53, mod_consts[126]);
        }

        Py_DECREF(tmp_import_name_from_53);
        if (tmp_assign_source_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[126], tmp_assign_source_57);
    }
    {
        PyObject *tmp_assign_source_58;
        PyObject *tmp_import_name_from_54;
        PyObject *tmp_name_name_54;
        PyObject *tmp_globals_arg_name_54;
        PyObject *tmp_locals_arg_name_54;
        PyObject *tmp_fromlist_name_54;
        PyObject *tmp_level_name_54;
        tmp_name_name_54 = mod_consts[112];
        tmp_globals_arg_name_54 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_54 = Py_None;
        tmp_fromlist_name_54 = mod_consts[127];
        tmp_level_name_54 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 60;
        tmp_import_name_from_54 = IMPORT_MODULE5(tmp_name_name_54, tmp_globals_arg_name_54, tmp_locals_arg_name_54, tmp_fromlist_name_54, tmp_level_name_54);
        if (tmp_import_name_from_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 60;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_54)) {
            tmp_assign_source_58 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_54,
                (PyObject *)moduledict_click,
                mod_consts[128],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_58 = IMPORT_NAME(tmp_import_name_from_54, mod_consts[128]);
        }

        Py_DECREF(tmp_import_name_from_54);
        if (tmp_assign_source_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 60;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[128], tmp_assign_source_58);
    }
    {
        PyObject *tmp_assign_source_59;
        PyObject *tmp_import_name_from_55;
        PyObject *tmp_name_name_55;
        PyObject *tmp_globals_arg_name_55;
        PyObject *tmp_locals_arg_name_55;
        PyObject *tmp_fromlist_name_55;
        PyObject *tmp_level_name_55;
        tmp_name_name_55 = mod_consts[112];
        tmp_globals_arg_name_55 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_55 = Py_None;
        tmp_fromlist_name_55 = mod_consts[129];
        tmp_level_name_55 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 61;
        tmp_import_name_from_55 = IMPORT_MODULE5(tmp_name_name_55, tmp_globals_arg_name_55, tmp_locals_arg_name_55, tmp_fromlist_name_55, tmp_level_name_55);
        if (tmp_import_name_from_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 61;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_55)) {
            tmp_assign_source_59 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_55,
                (PyObject *)moduledict_click,
                mod_consts[130],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_59 = IMPORT_NAME(tmp_import_name_from_55, mod_consts[130]);
        }

        Py_DECREF(tmp_import_name_from_55);
        if (tmp_assign_source_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 61;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[130], tmp_assign_source_59);
    }
    {
        PyObject *tmp_assign_source_60;
        PyObject *tmp_import_name_from_56;
        PyObject *tmp_name_name_56;
        PyObject *tmp_globals_arg_name_56;
        PyObject *tmp_locals_arg_name_56;
        PyObject *tmp_fromlist_name_56;
        PyObject *tmp_level_name_56;
        tmp_name_name_56 = mod_consts[112];
        tmp_globals_arg_name_56 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_56 = Py_None;
        tmp_fromlist_name_56 = mod_consts[131];
        tmp_level_name_56 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 62;
        tmp_import_name_from_56 = IMPORT_MODULE5(tmp_name_name_56, tmp_globals_arg_name_56, tmp_locals_arg_name_56, tmp_fromlist_name_56, tmp_level_name_56);
        if (tmp_import_name_from_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 62;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_56)) {
            tmp_assign_source_60 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_56,
                (PyObject *)moduledict_click,
                mod_consts[132],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_60 = IMPORT_NAME(tmp_import_name_from_56, mod_consts[132]);
        }

        Py_DECREF(tmp_import_name_from_56);
        if (tmp_assign_source_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 62;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[132], tmp_assign_source_60);
    }
    {
        PyObject *tmp_assign_source_61;
        PyObject *tmp_import_name_from_57;
        PyObject *tmp_name_name_57;
        PyObject *tmp_globals_arg_name_57;
        PyObject *tmp_locals_arg_name_57;
        PyObject *tmp_fromlist_name_57;
        PyObject *tmp_level_name_57;
        tmp_name_name_57 = mod_consts[112];
        tmp_globals_arg_name_57 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_57 = Py_None;
        tmp_fromlist_name_57 = mod_consts[133];
        tmp_level_name_57 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 63;
        tmp_import_name_from_57 = IMPORT_MODULE5(tmp_name_name_57, tmp_globals_arg_name_57, tmp_locals_arg_name_57, tmp_fromlist_name_57, tmp_level_name_57);
        if (tmp_import_name_from_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 63;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_57)) {
            tmp_assign_source_61 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_57,
                (PyObject *)moduledict_click,
                mod_consts[134],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_61 = IMPORT_NAME(tmp_import_name_from_57, mod_consts[134]);
        }

        Py_DECREF(tmp_import_name_from_57);
        if (tmp_assign_source_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 63;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[134], tmp_assign_source_61);
    }
    {
        PyObject *tmp_assign_source_62;
        PyObject *tmp_import_name_from_58;
        PyObject *tmp_name_name_58;
        PyObject *tmp_globals_arg_name_58;
        PyObject *tmp_locals_arg_name_58;
        PyObject *tmp_fromlist_name_58;
        PyObject *tmp_level_name_58;
        tmp_name_name_58 = mod_consts[112];
        tmp_globals_arg_name_58 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_58 = Py_None;
        tmp_fromlist_name_58 = mod_consts[135];
        tmp_level_name_58 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 64;
        tmp_import_name_from_58 = IMPORT_MODULE5(tmp_name_name_58, tmp_globals_arg_name_58, tmp_locals_arg_name_58, tmp_fromlist_name_58, tmp_level_name_58);
        if (tmp_import_name_from_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_58)) {
            tmp_assign_source_62 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_58,
                (PyObject *)moduledict_click,
                mod_consts[136],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_62 = IMPORT_NAME(tmp_import_name_from_58, mod_consts[136]);
        }

        Py_DECREF(tmp_import_name_from_58);
        if (tmp_assign_source_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[136], tmp_assign_source_62);
    }
    {
        PyObject *tmp_assign_source_63;
        PyObject *tmp_import_name_from_59;
        PyObject *tmp_name_name_59;
        PyObject *tmp_globals_arg_name_59;
        PyObject *tmp_locals_arg_name_59;
        PyObject *tmp_fromlist_name_59;
        PyObject *tmp_level_name_59;
        tmp_name_name_59 = mod_consts[112];
        tmp_globals_arg_name_59 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_59 = Py_None;
        tmp_fromlist_name_59 = mod_consts[137];
        tmp_level_name_59 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 65;
        tmp_import_name_from_59 = IMPORT_MODULE5(tmp_name_name_59, tmp_globals_arg_name_59, tmp_locals_arg_name_59, tmp_fromlist_name_59, tmp_level_name_59);
        if (tmp_import_name_from_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 65;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_59)) {
            tmp_assign_source_63 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_59,
                (PyObject *)moduledict_click,
                mod_consts[138],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_63 = IMPORT_NAME(tmp_import_name_from_59, mod_consts[138]);
        }

        Py_DECREF(tmp_import_name_from_59);
        if (tmp_assign_source_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 65;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[138], tmp_assign_source_63);
    }
    {
        PyObject *tmp_assign_source_64;
        PyObject *tmp_import_name_from_60;
        PyObject *tmp_name_name_60;
        PyObject *tmp_globals_arg_name_60;
        PyObject *tmp_locals_arg_name_60;
        PyObject *tmp_fromlist_name_60;
        PyObject *tmp_level_name_60;
        tmp_name_name_60 = mod_consts[112];
        tmp_globals_arg_name_60 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_60 = Py_None;
        tmp_fromlist_name_60 = mod_consts[139];
        tmp_level_name_60 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 66;
        tmp_import_name_from_60 = IMPORT_MODULE5(tmp_name_name_60, tmp_globals_arg_name_60, tmp_locals_arg_name_60, tmp_fromlist_name_60, tmp_level_name_60);
        if (tmp_import_name_from_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 66;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_60)) {
            tmp_assign_source_64 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_60,
                (PyObject *)moduledict_click,
                mod_consts[140],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_64 = IMPORT_NAME(tmp_import_name_from_60, mod_consts[140]);
        }

        Py_DECREF(tmp_import_name_from_60);
        if (tmp_assign_source_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 66;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[140], tmp_assign_source_64);
    }
    {
        PyObject *tmp_assign_source_65;
        PyObject *tmp_import_name_from_61;
        PyObject *tmp_name_name_61;
        PyObject *tmp_globals_arg_name_61;
        PyObject *tmp_locals_arg_name_61;
        PyObject *tmp_fromlist_name_61;
        PyObject *tmp_level_name_61;
        tmp_name_name_61 = mod_consts[141];
        tmp_globals_arg_name_61 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_61 = Py_None;
        tmp_fromlist_name_61 = mod_consts[142];
        tmp_level_name_61 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 67;
        tmp_import_name_from_61 = IMPORT_MODULE5(tmp_name_name_61, tmp_globals_arg_name_61, tmp_locals_arg_name_61, tmp_fromlist_name_61, tmp_level_name_61);
        if (tmp_import_name_from_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 67;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_61)) {
            tmp_assign_source_65 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_61,
                (PyObject *)moduledict_click,
                mod_consts[143],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_65 = IMPORT_NAME(tmp_import_name_from_61, mod_consts[143]);
        }

        Py_DECREF(tmp_import_name_from_61);
        if (tmp_assign_source_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 67;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[143], tmp_assign_source_65);
    }
    {
        PyObject *tmp_assign_source_66;
        PyObject *tmp_import_name_from_62;
        PyObject *tmp_name_name_62;
        PyObject *tmp_globals_arg_name_62;
        PyObject *tmp_locals_arg_name_62;
        PyObject *tmp_fromlist_name_62;
        PyObject *tmp_level_name_62;
        tmp_name_name_62 = mod_consts[141];
        tmp_globals_arg_name_62 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_62 = Py_None;
        tmp_fromlist_name_62 = mod_consts[144];
        tmp_level_name_62 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 68;
        tmp_import_name_from_62 = IMPORT_MODULE5(tmp_name_name_62, tmp_globals_arg_name_62, tmp_locals_arg_name_62, tmp_fromlist_name_62, tmp_level_name_62);
        if (tmp_import_name_from_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 68;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_62)) {
            tmp_assign_source_66 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_62,
                (PyObject *)moduledict_click,
                mod_consts[145],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_66 = IMPORT_NAME(tmp_import_name_from_62, mod_consts[145]);
        }

        Py_DECREF(tmp_import_name_from_62);
        if (tmp_assign_source_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 68;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[145], tmp_assign_source_66);
    }
    {
        PyObject *tmp_assign_source_67;
        PyObject *tmp_import_name_from_63;
        PyObject *tmp_name_name_63;
        PyObject *tmp_globals_arg_name_63;
        PyObject *tmp_locals_arg_name_63;
        PyObject *tmp_fromlist_name_63;
        PyObject *tmp_level_name_63;
        tmp_name_name_63 = mod_consts[141];
        tmp_globals_arg_name_63 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_63 = Py_None;
        tmp_fromlist_name_63 = mod_consts[146];
        tmp_level_name_63 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 69;
        tmp_import_name_from_63 = IMPORT_MODULE5(tmp_name_name_63, tmp_globals_arg_name_63, tmp_locals_arg_name_63, tmp_fromlist_name_63, tmp_level_name_63);
        if (tmp_import_name_from_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 69;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_63)) {
            tmp_assign_source_67 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_63,
                (PyObject *)moduledict_click,
                mod_consts[147],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_67 = IMPORT_NAME(tmp_import_name_from_63, mod_consts[147]);
        }

        Py_DECREF(tmp_import_name_from_63);
        if (tmp_assign_source_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 69;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[147], tmp_assign_source_67);
    }
    {
        PyObject *tmp_assign_source_68;
        PyObject *tmp_import_name_from_64;
        PyObject *tmp_name_name_64;
        PyObject *tmp_globals_arg_name_64;
        PyObject *tmp_locals_arg_name_64;
        PyObject *tmp_fromlist_name_64;
        PyObject *tmp_level_name_64;
        tmp_name_name_64 = mod_consts[141];
        tmp_globals_arg_name_64 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_64 = Py_None;
        tmp_fromlist_name_64 = mod_consts[148];
        tmp_level_name_64 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 70;
        tmp_import_name_from_64 = IMPORT_MODULE5(tmp_name_name_64, tmp_globals_arg_name_64, tmp_locals_arg_name_64, tmp_fromlist_name_64, tmp_level_name_64);
        if (tmp_import_name_from_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 70;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_64)) {
            tmp_assign_source_68 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_64,
                (PyObject *)moduledict_click,
                mod_consts[149],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_68 = IMPORT_NAME(tmp_import_name_from_64, mod_consts[149]);
        }

        Py_DECREF(tmp_import_name_from_64);
        if (tmp_assign_source_68 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 70;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[149], tmp_assign_source_68);
    }
    {
        PyObject *tmp_assign_source_69;
        PyObject *tmp_import_name_from_65;
        PyObject *tmp_name_name_65;
        PyObject *tmp_globals_arg_name_65;
        PyObject *tmp_locals_arg_name_65;
        PyObject *tmp_fromlist_name_65;
        PyObject *tmp_level_name_65;
        tmp_name_name_65 = mod_consts[141];
        tmp_globals_arg_name_65 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_65 = Py_None;
        tmp_fromlist_name_65 = mod_consts[150];
        tmp_level_name_65 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 71;
        tmp_import_name_from_65 = IMPORT_MODULE5(tmp_name_name_65, tmp_globals_arg_name_65, tmp_locals_arg_name_65, tmp_fromlist_name_65, tmp_level_name_65);
        if (tmp_import_name_from_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 71;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_65)) {
            tmp_assign_source_69 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_65,
                (PyObject *)moduledict_click,
                mod_consts[151],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_69 = IMPORT_NAME(tmp_import_name_from_65, mod_consts[151]);
        }

        Py_DECREF(tmp_import_name_from_65);
        if (tmp_assign_source_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 71;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[151], tmp_assign_source_69);
    }
    {
        PyObject *tmp_assign_source_70;
        PyObject *tmp_import_name_from_66;
        PyObject *tmp_name_name_66;
        PyObject *tmp_globals_arg_name_66;
        PyObject *tmp_locals_arg_name_66;
        PyObject *tmp_fromlist_name_66;
        PyObject *tmp_level_name_66;
        tmp_name_name_66 = mod_consts[141];
        tmp_globals_arg_name_66 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_66 = Py_None;
        tmp_fromlist_name_66 = mod_consts[152];
        tmp_level_name_66 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 72;
        tmp_import_name_from_66 = IMPORT_MODULE5(tmp_name_name_66, tmp_globals_arg_name_66, tmp_locals_arg_name_66, tmp_fromlist_name_66, tmp_level_name_66);
        if (tmp_import_name_from_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 72;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_66)) {
            tmp_assign_source_70 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_66,
                (PyObject *)moduledict_click,
                mod_consts[153],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_70 = IMPORT_NAME(tmp_import_name_from_66, mod_consts[153]);
        }

        Py_DECREF(tmp_import_name_from_66);
        if (tmp_assign_source_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 72;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[153], tmp_assign_source_70);
    }
    {
        PyObject *tmp_assign_source_71;
        PyObject *tmp_import_name_from_67;
        PyObject *tmp_name_name_67;
        PyObject *tmp_globals_arg_name_67;
        PyObject *tmp_locals_arg_name_67;
        PyObject *tmp_fromlist_name_67;
        PyObject *tmp_level_name_67;
        tmp_name_name_67 = mod_consts[141];
        tmp_globals_arg_name_67 = (PyObject *)moduledict_click;
        tmp_locals_arg_name_67 = Py_None;
        tmp_fromlist_name_67 = mod_consts[154];
        tmp_level_name_67 = mod_consts[14];
        frame_0bf55338aa053bcca70d25585371a4c2->m_frame.f_lineno = 73;
        tmp_import_name_from_67 = IMPORT_MODULE5(tmp_name_name_67, tmp_globals_arg_name_67, tmp_locals_arg_name_67, tmp_fromlist_name_67, tmp_level_name_67);
        if (tmp_import_name_from_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 73;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_67)) {
            tmp_assign_source_71 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_67,
                (PyObject *)moduledict_click,
                mod_consts[155],
                mod_consts[6]
            );
        } else {
            tmp_assign_source_71 = IMPORT_NAME(tmp_import_name_from_67, mod_consts[155]);
        }

        Py_DECREF(tmp_import_name_from_67);
        if (tmp_assign_source_71 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 73;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_click, (Nuitka_StringObject *)mod_consts[155], tmp_assign_source_71);
    }

    // Restore frame exception if necessary.
#if 0
    RESTORE_FRAME_EXCEPTION(frame_0bf55338aa053bcca70d25585371a4c2);
#endif
    popFrameStack();

    assertFrameObject(frame_0bf55338aa053bcca70d25585371a4c2);

    goto frame_no_exception_1;

    frame_exception_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_0bf55338aa053bcca70d25585371a4c2);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_0bf55338aa053bcca70d25585371a4c2, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_0bf55338aa053bcca70d25585371a4c2->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_0bf55338aa053bcca70d25585371a4c2, exception_lineno);
    }

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto module_exception_exit;

    frame_no_exception_1:;
    {
        PyObject *tmp_assign_source_72;
        tmp_assign_source_72 = mod_consts[156];
        UPDATE_STRING_DICT0(moduledict_click, (Nuitka_StringObject *)mod_consts[157], tmp_assign_source_72);
    }

    return module_click;
    module_exception_exit:
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);
    return NULL;
}

