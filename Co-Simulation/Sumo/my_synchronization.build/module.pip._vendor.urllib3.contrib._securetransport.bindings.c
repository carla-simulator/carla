/* Generated code for Python module 'pip._vendor.urllib3.contrib._securetransport.bindings'
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

/* The "module_pip$_vendor$urllib3$contrib$_securetransport$bindings" is a Python object pointer of module type.
 *
 * Note: For full compatibility with CPython, every module variable access
 * needs to go through it except for cases where the module cannot possibly
 * have changed in the mean time.
 */

PyObject *module_pip$_vendor$urllib3$contrib$_securetransport$bindings;
PyDictObject *moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings;

/* The declarations of module constants used, if any. */
static PyObject *mod_consts[309];

static PyObject *module_filename_obj = NULL;

/* Indicator if this modules private constants were created yet. */
static bool constants_created = false;

/* Function to create module private constants. */
static void createModuleConstants(void) {
    if (constants_created == false) {
        loadConstantsBlob(&mod_consts[0], UNTRANSLATE("pip._vendor.urllib3.contrib._securetransport.bindings"));
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
void checkModuleConstants_pip$_vendor$urllib3$contrib$_securetransport$bindings(void) {
    // The module may not have been used at all, then ignore this.
    if (constants_created == false) return;

    checkConstantsBlob(&mod_consts[0], "pip._vendor.urllib3.contrib._securetransport.bindings");
}
#endif

// The module code objects.
static PyCodeObject *codeobj_1d818adf931272325ef94f3fb8c15fe1;
static PyCodeObject *codeobj_192bd6ff2c8f6e701d6413b996cc4ba3;
static PyCodeObject *codeobj_7bb482a1d6119c41bca3daa9420852e8;
static PyCodeObject *codeobj_94d3635f10bde552e88826fb750ea1ec;

static void createModuleCodeObjects(void) {
    module_filename_obj = mod_consts[10]; CHECK_OBJECT(module_filename_obj);
    codeobj_1d818adf931272325ef94f3fb8c15fe1 = MAKE_CODEOBJECT(module_filename_obj, 1, CO_NOFREE, mod_consts[306], NULL, NULL, 0, 0, 0);
    codeobj_192bd6ff2c8f6e701d6413b996cc4ba3 = MAKE_CODEOBJECT(module_filename_obj, 423, CO_NOFREE, mod_consts[162], mod_consts[307], NULL, 0, 0, 0);
    codeobj_7bb482a1d6119c41bca3daa9420852e8 = MAKE_CODEOBJECT(module_filename_obj, 432, CO_NOFREE, mod_consts[173], mod_consts[307], NULL, 0, 0, 0);
    codeobj_94d3635f10bde552e88826fb750ea1ec = MAKE_CODEOBJECT(module_filename_obj, 65, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[46], mod_consts[308], NULL, 2, 0, 0);
}

// The module function declarations.
static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$function__1_load_cdll();


// The module function definitions.
static PyObject *impl_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$function__1_load_cdll(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_name = python_pars[0];
    PyObject *par_macos10_16_path = python_pars[1];
    PyObject *var_path = NULL;
    struct Nuitka_FrameObject *frame_94d3635f10bde552e88826fb750ea1ec;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    int tmp_res;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;
    PyObject *exception_preserved_type_1;
    PyObject *exception_preserved_value_1;
    PyTracebackObject *exception_preserved_tb_1;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    bool tmp_result;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_2;
    static struct Nuitka_FrameObject *cache_frame_94d3635f10bde552e88826fb750ea1ec = NULL;
    PyObject *exception_keeper_type_3;
    PyObject *exception_keeper_value_3;
    PyTracebackObject *exception_keeper_tb_3;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_3;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_94d3635f10bde552e88826fb750ea1ec)) {
        Py_XDECREF(cache_frame_94d3635f10bde552e88826fb750ea1ec);

#if _DEBUG_REFCOUNTS
        if (cache_frame_94d3635f10bde552e88826fb750ea1ec == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_94d3635f10bde552e88826fb750ea1ec = MAKE_FUNCTION_FRAME(codeobj_94d3635f10bde552e88826fb750ea1ec, module_pip$_vendor$urllib3$contrib$_securetransport$bindings, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_94d3635f10bde552e88826fb750ea1ec->m_type_description == NULL);
    frame_94d3635f10bde552e88826fb750ea1ec = cache_frame_94d3635f10bde552e88826fb750ea1ec;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_94d3635f10bde552e88826fb750ea1ec);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_94d3635f10bde552e88826fb750ea1ec) == 2); // Frame stack

    // Framed code:
    // Tried code:
    {
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        tmp_compexpr_left_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[0]);

        if (unlikely(tmp_compexpr_left_1 == NULL)) {
            tmp_compexpr_left_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[0]);
        }

        if (tmp_compexpr_left_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 70;
            type_description_1 = "ooo";
            goto try_except_handler_2;
        }
        tmp_compexpr_right_1 = mod_consts[1];
        tmp_condition_result_1 = RICH_COMPARE_GE_NBOOL_OBJECT_TUPLE(tmp_compexpr_left_1, tmp_compexpr_right_1);
        if (tmp_condition_result_1 == NUITKA_BOOL_EXCEPTION) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 70;
            type_description_1 = "ooo";
            goto try_except_handler_2;
        }
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
        assert(tmp_condition_result_1 != NUITKA_BOOL_UNASSIGNED);
    }
    branch_yes_1:;
    {
        PyObject *tmp_assign_source_1;
        CHECK_OBJECT(par_macos10_16_path);
        tmp_assign_source_1 = par_macos10_16_path;
        assert(var_path == NULL);
        Py_INCREF(tmp_assign_source_1);
        var_path = tmp_assign_source_1;
    }
    goto branch_end_1;
    branch_no_1:;
    {
        PyObject *tmp_assign_source_2;
        PyObject *tmp_called_name_1;
        PyObject *tmp_args_element_name_1;
        tmp_called_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[2]);

        if (unlikely(tmp_called_name_1 == NULL)) {
            tmp_called_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[2]);
        }

        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 73;
            type_description_1 = "ooo";
            goto try_except_handler_2;
        }
        CHECK_OBJECT(par_name);
        tmp_args_element_name_1 = par_name;
        frame_94d3635f10bde552e88826fb750ea1ec->m_frame.f_lineno = 73;
        tmp_assign_source_2 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_1, tmp_args_element_name_1);
        if (tmp_assign_source_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 73;
            type_description_1 = "ooo";
            goto try_except_handler_2;
        }
        assert(var_path == NULL);
        var_path = tmp_assign_source_2;
    }
    branch_end_1:;
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_operand_name_1;
        CHECK_OBJECT(var_path);
        tmp_operand_name_1 = var_path;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 74;
            type_description_1 = "ooo";
            goto try_except_handler_2;
        }
        tmp_condition_result_2 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_2 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_raise_type_1;
        tmp_raise_type_1 = PyExc_OSError;
        exception_type = tmp_raise_type_1;
        Py_INCREF(tmp_raise_type_1);
        exception_lineno = 75;
        RAISE_EXCEPTION_WITH_TYPE(&exception_type, &exception_value, &exception_tb);
        type_description_1 = "ooo";
        goto try_except_handler_2;
    }
    branch_no_2:;
    {
        PyObject *tmp_called_name_2;
        PyObject *tmp_args_name_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_kwargs_name_1;
        tmp_called_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[3]);

        if (unlikely(tmp_called_name_2 == NULL)) {
            tmp_called_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[3]);
        }

        if (tmp_called_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 76;
            type_description_1 = "ooo";
            goto try_except_handler_2;
        }
        CHECK_OBJECT(var_path);
        tmp_tuple_element_1 = var_path;
        tmp_args_name_1 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_args_name_1, 0, tmp_tuple_element_1);
        tmp_kwargs_name_1 = PyDict_Copy(mod_consts[4]);
        frame_94d3635f10bde552e88826fb750ea1ec->m_frame.f_lineno = 76;
        tmp_return_value = CALL_FUNCTION(tmp_called_name_2, tmp_args_name_1, tmp_kwargs_name_1);
        Py_DECREF(tmp_args_name_1);
        Py_DECREF(tmp_kwargs_name_1);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 76;
            type_description_1 = "ooo";
            goto try_except_handler_2;
        }
        goto frame_return_exit_1;
    }
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Exception handler code:
    try_except_handler_2:;
    exception_keeper_type_1 = exception_type;
    exception_keeper_value_1 = exception_value;
    exception_keeper_tb_1 = exception_tb;
    exception_keeper_lineno_1 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    // Preserve existing published exception id 1.
    GET_CURRENT_EXCEPTION(&exception_preserved_type_1, &exception_preserved_value_1, &exception_preserved_tb_1);

    if (exception_keeper_tb_1 == NULL) {
        exception_keeper_tb_1 = MAKE_TRACEBACK(frame_94d3635f10bde552e88826fb750ea1ec, exception_keeper_lineno_1);
    } else if (exception_keeper_lineno_1 != 0) {
        exception_keeper_tb_1 = ADD_TRACEBACK(exception_keeper_tb_1, frame_94d3635f10bde552e88826fb750ea1ec, exception_keeper_lineno_1);
    }

    NORMALIZE_EXCEPTION(&exception_keeper_type_1, &exception_keeper_value_1, &exception_keeper_tb_1);
    ATTACH_TRACEBACK_TO_EXCEPTION_VALUE(exception_keeper_value_1, exception_keeper_tb_1);
    PUBLISH_EXCEPTION(&exception_keeper_type_1, &exception_keeper_value_1, &exception_keeper_tb_1);
    // Tried code:
    {
        bool tmp_condition_result_3;
        PyObject *tmp_compexpr_left_2;
        PyObject *tmp_compexpr_right_2;
        tmp_compexpr_left_2 = EXC_TYPE(PyThreadState_GET());
        tmp_compexpr_right_2 = PyExc_OSError;
        tmp_res = EXCEPTION_MATCH_BOOL(tmp_compexpr_left_2, tmp_compexpr_right_2);
        assert(!(tmp_res == -1));
        tmp_condition_result_3 = (tmp_res != 0) ? true : false;
        if (tmp_condition_result_3 != false) {
            goto branch_yes_3;
        } else {
            goto branch_no_3;
        }
    }
    branch_yes_3:;
    {
        PyObject *tmp_called_name_3;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_make_exception_arg_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_args_element_name_3;
        tmp_called_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[5]);

        if (unlikely(tmp_called_name_3 == NULL)) {
            tmp_called_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[5]);
        }

        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 78;
            type_description_1 = "ooo";
            goto try_except_handler_3;
        }
        tmp_left_name_1 = mod_consts[6];
        CHECK_OBJECT(par_name);
        tmp_right_name_1 = par_name;
        tmp_make_exception_arg_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_OBJECT(tmp_left_name_1, tmp_right_name_1);
        if (tmp_make_exception_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 78;
            type_description_1 = "ooo";
            goto try_except_handler_3;
        }
        frame_94d3635f10bde552e88826fb750ea1ec->m_frame.f_lineno = 78;
        tmp_args_element_name_2 = CALL_FUNCTION_WITH_SINGLE_ARG(PyExc_ImportError, tmp_make_exception_arg_1);
        Py_DECREF(tmp_make_exception_arg_1);
        assert(!(tmp_args_element_name_2 == NULL));
        tmp_args_element_name_3 = Py_None;
        frame_94d3635f10bde552e88826fb750ea1ec->m_frame.f_lineno = 78;
        {
            PyObject *call_args[] = {tmp_args_element_name_2, tmp_args_element_name_3};
            tmp_call_result_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_3, call_args);
        }

        Py_DECREF(tmp_args_element_name_2);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 78;
            type_description_1 = "ooo";
            goto try_except_handler_3;
        }
        Py_DECREF(tmp_call_result_1);
    }
    goto branch_end_3;
    branch_no_3:;
    tmp_result = RERAISE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
    if (unlikely(tmp_result == false)) {
        exception_lineno = 67;
    }

    if (exception_tb && exception_tb->tb_frame == &frame_94d3635f10bde552e88826fb750ea1ec->m_frame) frame_94d3635f10bde552e88826fb750ea1ec->m_frame.f_lineno = exception_tb->tb_lineno;
    type_description_1 = "ooo";
    goto try_except_handler_3;
    branch_end_3:;
    goto try_end_1;
    // Exception handler code:
    try_except_handler_3:;
    exception_keeper_type_2 = exception_type;
    exception_keeper_value_2 = exception_value;
    exception_keeper_tb_2 = exception_tb;
    exception_keeper_lineno_2 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    // Restore previous exception id 1.
    SET_CURRENT_EXCEPTION(exception_preserved_type_1, exception_preserved_value_1, exception_preserved_tb_1);

    // Re-raise.
    exception_type = exception_keeper_type_2;
    exception_value = exception_keeper_value_2;
    exception_tb = exception_keeper_tb_2;
    exception_lineno = exception_keeper_lineno_2;

    goto frame_exception_exit_1;
    // End of try:
    try_end_1:;
    // Restore previous exception id 1.
    SET_CURRENT_EXCEPTION(exception_preserved_type_1, exception_preserved_value_1, exception_preserved_tb_1);

    goto try_end_2;
    NUITKA_CANNOT_GET_HERE("exception handler codes exits in all cases");
    return NULL;
    // End of try:
    try_end_2:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_94d3635f10bde552e88826fb750ea1ec);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_94d3635f10bde552e88826fb750ea1ec);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_94d3635f10bde552e88826fb750ea1ec);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_94d3635f10bde552e88826fb750ea1ec, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_94d3635f10bde552e88826fb750ea1ec->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_94d3635f10bde552e88826fb750ea1ec, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_94d3635f10bde552e88826fb750ea1ec,
        type_description_1,
        par_name,
        par_macos10_16_path,
        var_path
    );


    // Release cached frame if used for exception.
    if (frame_94d3635f10bde552e88826fb750ea1ec == cache_frame_94d3635f10bde552e88826fb750ea1ec) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_94d3635f10bde552e88826fb750ea1ec);
        cache_frame_94d3635f10bde552e88826fb750ea1ec = NULL;
    }

    assertFrameObject(frame_94d3635f10bde552e88826fb750ea1ec);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    tmp_return_value = Py_None;
    Py_INCREF(tmp_return_value);
    goto try_return_handler_1;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(par_macos10_16_path);
    Py_DECREF(par_macos10_16_path);
    par_macos10_16_path = NULL;
    Py_XDECREF(var_path);
    var_path = NULL;
    goto function_return_exit;
    // Exception handler code:
    try_except_handler_1:;
    exception_keeper_type_3 = exception_type;
    exception_keeper_value_3 = exception_value;
    exception_keeper_tb_3 = exception_tb;
    exception_keeper_lineno_3 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(par_macos10_16_path);
    Py_DECREF(par_macos10_16_path);
    par_macos10_16_path = NULL;
    Py_XDECREF(var_path);
    var_path = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_3;
    exception_value = exception_keeper_value_3;
    exception_tb = exception_keeper_tb_3;
    exception_lineno = exception_keeper_lineno_3;

    goto function_exception_exit;
    // End of try:

    NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
    return NULL;

function_exception_exit:
    assert(exception_type);
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);

    return NULL;

function_return_exit:
   // Function cleanup code if any.


   // Actual function exit with return value, making sure we did not make
   // the error status worse despite non-NULL return.
   CHECK_OBJECT(tmp_return_value);
   assert(had_error || !ERROR_OCCURRED());
   return tmp_return_value;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$function__1_load_cdll() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$function__1_load_cdll,
        mod_consts[46],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_94d3635f10bde552e88826fb750ea1ec,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$contrib$_securetransport$bindings,
        mod_consts[7],
        NULL,
        0
    );


    return (PyObject *)result;
}


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

function_impl_code functable_pip$_vendor$urllib3$contrib$_securetransport$bindings[] = {
    impl_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$function__1_load_cdll,
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

    function_impl_code *current = functable_pip$_vendor$urllib3$contrib$_securetransport$bindings;
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

    if (offset > sizeof(functable_pip$_vendor$urllib3$contrib$_securetransport$bindings) || offset < 0) {
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
        functable_pip$_vendor$urllib3$contrib$_securetransport$bindings[offset],
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
        module_pip$_vendor$urllib3$contrib$_securetransport$bindings,
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
PyObject *modulecode_pip$_vendor$urllib3$contrib$_securetransport$bindings(PyObject *module, struct Nuitka_MetaPathBasedLoaderEntry const *module_entry) {
    module_pip$_vendor$urllib3$contrib$_securetransport$bindings = module;

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
    PRINT_STRING("pip._vendor.urllib3.contrib._securetransport.bindings: Calling setupMetaPathBasedLoader().\n");
#endif
    setupMetaPathBasedLoader();

#if PYTHON_VERSION >= 0x300
    patchInspectModule();
#endif

#endif

    /* The constants only used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.urllib3.contrib._securetransport.bindings: Calling createModuleConstants().\n");
#endif
    createModuleConstants();

    /* The code objects used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.urllib3.contrib._securetransport.bindings: Calling createModuleCodeObjects().\n");
#endif
    createModuleCodeObjects();

    // PRINT_STRING("in initpip$_vendor$urllib3$contrib$_securetransport$bindings\n");

    // Create the module object first. There are no methods initially, all are
    // added dynamically in actual code only.  Also no "__doc__" is initially
    // set at this time, as it could not contain NUL characters this way, they
    // are instead set in early module code.  No "self" for modules, we have no
    // use for it.

    moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings = MODULE_DICT(module_pip$_vendor$urllib3$contrib$_securetransport$bindings);

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
    registerDillPluginTables(module_entry->name, &_method_def_reduce_compiled_function, &_method_def_create_compiled_function);
#endif

    // Set "__compiled__" to what version information we have.
    UPDATE_STRING_DICT0(
        moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
        (Nuitka_StringObject *)const_str_plain___compiled__,
        Nuitka_dunder_compiled_value
    );

    // Update "__package__" value to what it ought to be.
    {
#if 0
        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
            (Nuitka_StringObject *)const_str_plain___package__,
            const_str_empty
        );
#elif 0
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___name__);

        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
            (Nuitka_StringObject *)const_str_plain___package__,
            module_name
        );
#else

#if PYTHON_VERSION < 0x300
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___name__);
        char const *module_name_cstr = PyString_AS_STRING(module_name);

        char const *last_dot = strrchr(module_name_cstr, '.');

        if (last_dot != NULL) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyString_FromStringAndSize(module_name_cstr, last_dot - module_name_cstr)
            );
        }
#else
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___name__);
        Py_ssize_t dot_index = PyUnicode_Find(module_name, const_str_dot, 0, PyUnicode_GetLength(module_name), -1);

        if (dot_index != -1) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyUnicode_Substring(module_name, 0, dot_index)
            );
        }
#endif
#endif
    }

    CHECK_OBJECT(module_pip$_vendor$urllib3$contrib$_securetransport$bindings);

    // For deep importing of a module we need to have "__builtins__", so we set
    // it ourselves in the same way than CPython does. Note: This must be done
    // before the frame object is allocated, or else it may fail.

    if (GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___builtins__) == NULL) {
        PyObject *value = (PyObject *)builtin_module;

        // Check if main module, not a dict then but the module itself.
#if !defined(_NUITKA_EXE) || !0
        value = PyModule_GetDict(value);
#endif

        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___builtins__, value);
    }

#if PYTHON_VERSION >= 0x300
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___loader__, (PyObject *)&Nuitka_Loader_Type);
#endif

#if PYTHON_VERSION >= 0x340
// Set the "__spec__" value

#if 0
    // Main modules just get "None" as spec.
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___spec__, Py_None);
#else
    // Other modules get a "ModuleSpec" from the standard mechanism.
    {
        PyObject *bootstrap_module = getImportLibBootstrapModule();
        CHECK_OBJECT(bootstrap_module);

        PyObject *_spec_from_module = PyObject_GetAttrString(bootstrap_module, "_spec_from_module");
        CHECK_OBJECT(_spec_from_module);

        PyObject *spec_value = CALL_FUNCTION_WITH_SINGLE_ARG(_spec_from_module, module_pip$_vendor$urllib3$contrib$_securetransport$bindings);
        Py_DECREF(_spec_from_module);

        // We can assume this to never fail, or else we are in trouble anyway.
        // CHECK_OBJECT(spec_value);

        if (spec_value == NULL) {
            PyErr_PrintEx(0);
            abort();
        }

// Mark the execution in the "__spec__" value.
        SET_ATTRIBUTE(spec_value, const_str_plain__initializing, Py_True);

        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)const_str_plain___spec__, spec_value);
    }
#endif
#endif

    // Temp variables if any
    PyObject *outline_0_var___class__ = NULL;
    PyObject *outline_1_var___class__ = NULL;
    PyObject *tmp_class_creation_1__bases = NULL;
    PyObject *tmp_class_creation_1__class_decl_dict = NULL;
    PyObject *tmp_class_creation_1__metaclass = NULL;
    PyObject *tmp_class_creation_1__prepared = NULL;
    PyObject *tmp_class_creation_2__bases = NULL;
    PyObject *tmp_class_creation_2__class_decl_dict = NULL;
    PyObject *tmp_class_creation_2__metaclass = NULL;
    PyObject *tmp_class_creation_2__prepared = NULL;
    PyObject *tmp_import_from_1__module = NULL;
    struct Nuitka_FrameObject *frame_1d818adf931272325ef94f3fb8c15fe1;
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
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_2;
    PyObject *exception_preserved_type_1;
    PyObject *exception_preserved_value_1;
    PyTracebackObject *exception_preserved_tb_1;
    int tmp_res;
    PyObject *exception_keeper_type_3;
    PyObject *exception_keeper_value_3;
    PyTracebackObject *exception_keeper_tb_3;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_3;
    PyObject *exception_keeper_type_4;
    PyObject *exception_keeper_value_4;
    PyTracebackObject *exception_keeper_tb_4;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_4;
    PyObject *exception_preserved_type_2;
    PyObject *exception_preserved_value_2;
    PyTracebackObject *exception_preserved_tb_2;
    PyObject *exception_keeper_type_5;
    PyObject *exception_keeper_value_5;
    PyTracebackObject *exception_keeper_tb_5;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_5;
    PyObject *tmp_dictdel_dict;
    PyObject *tmp_dictdel_key;
    PyObject *locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423 = NULL;
    PyObject *tmp_dictset_value;
    struct Nuitka_FrameObject *frame_192bd6ff2c8f6e701d6413b996cc4ba3_2;
    NUITKA_MAY_BE_UNUSED char const *type_description_2 = NULL;
    static struct Nuitka_FrameObject *cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2 = NULL;
    PyObject *exception_keeper_type_6;
    PyObject *exception_keeper_value_6;
    PyTracebackObject *exception_keeper_tb_6;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_6;
    PyObject *exception_keeper_type_7;
    PyObject *exception_keeper_value_7;
    PyTracebackObject *exception_keeper_tb_7;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_7;
    PyObject *exception_keeper_type_8;
    PyObject *exception_keeper_value_8;
    PyTracebackObject *exception_keeper_tb_8;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_8;
    PyObject *locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432 = NULL;
    struct Nuitka_FrameObject *frame_7bb482a1d6119c41bca3daa9420852e8_3;
    NUITKA_MAY_BE_UNUSED char const *type_description_3 = NULL;
    static struct Nuitka_FrameObject *cache_frame_7bb482a1d6119c41bca3daa9420852e8_3 = NULL;
    PyObject *exception_keeper_type_9;
    PyObject *exception_keeper_value_9;
    PyTracebackObject *exception_keeper_tb_9;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_9;
    PyObject *exception_keeper_type_10;
    PyObject *exception_keeper_value_10;
    PyTracebackObject *exception_keeper_tb_10;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_10;
    PyObject *exception_keeper_type_11;
    PyObject *exception_keeper_value_11;
    PyTracebackObject *exception_keeper_tb_11;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_11;

    // Module code.
    {
        PyObject *tmp_assign_source_1;
        tmp_assign_source_1 = mod_consts[8];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[9], tmp_assign_source_1);
    }
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = mod_consts[10];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[11], tmp_assign_source_2);
    }
    // Frame without reuse.
    frame_1d818adf931272325ef94f3fb8c15fe1 = MAKE_MODULE_FRAME(codeobj_1d818adf931272325ef94f3fb8c15fe1, module_pip$_vendor$urllib3$contrib$_securetransport$bindings);

    // Push the new frame as the currently active one, and we should be exclusively
    // owning it.
    pushFrameStack(frame_1d818adf931272325ef94f3fb8c15fe1);
    assert(Py_REFCNT(frame_1d818adf931272325ef94f3fb8c15fe1) == 2);

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_assattr_name_1 = mod_consts[10];
        tmp_assattr_target_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[12]);

        if (unlikely(tmp_assattr_target_1 == NULL)) {
            tmp_assattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
        }

        assert(!(tmp_assattr_target_1 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[13], tmp_assattr_name_1);
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
        tmp_assattr_target_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[12]);

        if (unlikely(tmp_assattr_target_2 == NULL)) {
            tmp_assattr_target_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
        }

        assert(!(tmp_assattr_target_2 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[14], tmp_assattr_name_2);
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
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[15], tmp_assign_source_3);
    }
    {
        PyObject *tmp_assign_source_4;
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 32;
        {
            PyObject *hard_module = IMPORT_HARD___FUTURE__();
            tmp_assign_source_4 = LOOKUP_ATTRIBUTE(hard_module, mod_consts[16]);
        }

        assert(!(tmp_assign_source_4 == NULL));
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[16], tmp_assign_source_4);
    }
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_name_name_1;
        PyObject *tmp_globals_arg_name_1;
        PyObject *tmp_locals_arg_name_1;
        PyObject *tmp_fromlist_name_1;
        PyObject *tmp_level_name_1;
        tmp_name_name_1 = mod_consts[17];
        tmp_globals_arg_name_1 = (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings;
        tmp_locals_arg_name_1 = Py_None;
        tmp_fromlist_name_1 = Py_None;
        tmp_level_name_1 = mod_consts[18];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 34;
        tmp_assign_source_5 = IMPORT_MODULE5(tmp_name_name_1, tmp_globals_arg_name_1, tmp_locals_arg_name_1, tmp_fromlist_name_1, tmp_level_name_1);
        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 34;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[17], tmp_assign_source_5);
    }
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_name_name_2;
        PyObject *tmp_globals_arg_name_2;
        PyObject *tmp_locals_arg_name_2;
        PyObject *tmp_fromlist_name_2;
        PyObject *tmp_level_name_2;
        tmp_name_name_2 = mod_consts[19];
        tmp_globals_arg_name_2 = (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings;
        tmp_locals_arg_name_2 = Py_None;
        tmp_fromlist_name_2 = mod_consts[20];
        tmp_level_name_2 = mod_consts[18];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 35;
        tmp_assign_source_6 = IMPORT_MODULE5(tmp_name_name_2, tmp_globals_arg_name_2, tmp_locals_arg_name_2, tmp_fromlist_name_2, tmp_level_name_2);
        if (tmp_assign_source_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto frame_exception_exit_1;
        }
        assert(tmp_import_from_1__module == NULL);
        tmp_import_from_1__module = tmp_assign_source_6;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_7;
        PyObject *tmp_import_name_from_1;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_1 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_1)) {
            tmp_assign_source_7 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_1,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[3],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_7 = IMPORT_NAME(tmp_import_name_from_1, mod_consts[3]);
        }

        if (tmp_assign_source_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[3], tmp_assign_source_7);
    }
    {
        PyObject *tmp_assign_source_8;
        PyObject *tmp_import_name_from_2;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_2 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_2)) {
            tmp_assign_source_8 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_2,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[21],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_8 = IMPORT_NAME(tmp_import_name_from_2, mod_consts[21]);
        }

        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[21], tmp_assign_source_8);
    }
    {
        PyObject *tmp_assign_source_9;
        PyObject *tmp_import_name_from_3;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_3 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_3)) {
            tmp_assign_source_9 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_3,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[22],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_9 = IMPORT_NAME(tmp_import_name_from_3, mod_consts[22]);
        }

        if (tmp_assign_source_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22], tmp_assign_source_9);
    }
    {
        PyObject *tmp_assign_source_10;
        PyObject *tmp_import_name_from_4;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_4 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_4)) {
            tmp_assign_source_10 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_4,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[23],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_10 = IMPORT_NAME(tmp_import_name_from_4, mod_consts[23]);
        }

        if (tmp_assign_source_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[23], tmp_assign_source_10);
    }
    {
        PyObject *tmp_assign_source_11;
        PyObject *tmp_import_name_from_5;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_5 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_5)) {
            tmp_assign_source_11 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_5,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[24],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_11 = IMPORT_NAME(tmp_import_name_from_5, mod_consts[24]);
        }

        if (tmp_assign_source_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[24], tmp_assign_source_11);
    }
    {
        PyObject *tmp_assign_source_12;
        PyObject *tmp_import_name_from_6;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_6 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_6)) {
            tmp_assign_source_12 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_6,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[25],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_12 = IMPORT_NAME(tmp_import_name_from_6, mod_consts[25]);
        }

        if (tmp_assign_source_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25], tmp_assign_source_12);
    }
    {
        PyObject *tmp_assign_source_13;
        PyObject *tmp_import_name_from_7;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_7 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_7)) {
            tmp_assign_source_13 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_7,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[26],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_13 = IMPORT_NAME(tmp_import_name_from_7, mod_consts[26]);
        }

        if (tmp_assign_source_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[26], tmp_assign_source_13);
    }
    {
        PyObject *tmp_assign_source_14;
        PyObject *tmp_import_name_from_8;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_8 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_8)) {
            tmp_assign_source_14 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_8,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[27],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_14 = IMPORT_NAME(tmp_import_name_from_8, mod_consts[27]);
        }

        if (tmp_assign_source_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[27], tmp_assign_source_14);
    }
    {
        PyObject *tmp_assign_source_15;
        PyObject *tmp_import_name_from_9;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_9 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_9)) {
            tmp_assign_source_15 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_9,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[28],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_15 = IMPORT_NAME(tmp_import_name_from_9, mod_consts[28]);
        }

        if (tmp_assign_source_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28], tmp_assign_source_15);
    }
    {
        PyObject *tmp_assign_source_16;
        PyObject *tmp_import_name_from_10;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_10 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_10)) {
            tmp_assign_source_16 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_10,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[29],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_16 = IMPORT_NAME(tmp_import_name_from_10, mod_consts[29]);
        }

        if (tmp_assign_source_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29], tmp_assign_source_16);
    }
    {
        PyObject *tmp_assign_source_17;
        PyObject *tmp_import_name_from_11;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_11 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_11)) {
            tmp_assign_source_17 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_11,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[30],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_17 = IMPORT_NAME(tmp_import_name_from_11, mod_consts[30]);
        }

        if (tmp_assign_source_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[30], tmp_assign_source_17);
    }
    {
        PyObject *tmp_assign_source_18;
        PyObject *tmp_import_name_from_12;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_12 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_12)) {
            tmp_assign_source_18 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_12,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[31],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_18 = IMPORT_NAME(tmp_import_name_from_12, mod_consts[31]);
        }

        if (tmp_assign_source_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31], tmp_assign_source_18);
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
        PyObject *tmp_assign_source_19;
        PyObject *tmp_import_name_from_13;
        PyObject *tmp_name_name_3;
        PyObject *tmp_globals_arg_name_3;
        PyObject *tmp_locals_arg_name_3;
        PyObject *tmp_fromlist_name_3;
        PyObject *tmp_level_name_3;
        tmp_name_name_3 = mod_consts[32];
        tmp_globals_arg_name_3 = (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings;
        tmp_locals_arg_name_3 = Py_None;
        tmp_fromlist_name_3 = mod_consts[33];
        tmp_level_name_3 = mod_consts[18];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 49;
        tmp_import_name_from_13 = IMPORT_MODULE5(tmp_name_name_3, tmp_globals_arg_name_3, tmp_locals_arg_name_3, tmp_fromlist_name_3, tmp_level_name_3);
        if (tmp_import_name_from_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 49;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_13)) {
            tmp_assign_source_19 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_13,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[2],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_19 = IMPORT_NAME(tmp_import_name_from_13, mod_consts[2]);
        }

        Py_DECREF(tmp_import_name_from_13);
        if (tmp_assign_source_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 49;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[2], tmp_assign_source_19);
    }
    {
        PyObject *tmp_assign_source_20;
        PyObject *tmp_import_name_from_14;
        PyObject *tmp_name_name_4;
        PyObject *tmp_globals_arg_name_4;
        PyObject *tmp_locals_arg_name_4;
        PyObject *tmp_fromlist_name_4;
        PyObject *tmp_level_name_4;
        tmp_name_name_4 = mod_consts[34];
        tmp_globals_arg_name_4 = (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings;
        tmp_locals_arg_name_4 = Py_None;
        tmp_fromlist_name_4 = mod_consts[35];
        tmp_level_name_4 = mod_consts[18];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 51;
        tmp_import_name_from_14 = IMPORT_MODULE5(tmp_name_name_4, tmp_globals_arg_name_4, tmp_locals_arg_name_4, tmp_fromlist_name_4, tmp_level_name_4);
        if (tmp_import_name_from_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 51;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_14)) {
            tmp_assign_source_20 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_14,
                (PyObject *)moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings,
                mod_consts[5],
                mod_consts[18]
            );
        } else {
            tmp_assign_source_20 = IMPORT_NAME(tmp_import_name_from_14, mod_consts[5]);
        }

        Py_DECREF(tmp_import_name_from_14);
        if (tmp_assign_source_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 51;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[5], tmp_assign_source_20);
    }
    {
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_tmp_condition_result_1_object_1;
        int tmp_truth_name_1;
        tmp_called_instance_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[17]);

        if (unlikely(tmp_called_instance_1 == NULL)) {
            tmp_called_instance_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
        }

        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 53;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 53;
        tmp_compexpr_left_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_1, mod_consts[36]);
        if (tmp_compexpr_left_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 53;

            goto frame_exception_exit_1;
        }
        tmp_compexpr_right_1 = mod_consts[37];
        tmp_tmp_condition_result_1_object_1 = RICH_COMPARE_NE_OBJECT_OBJECT_OBJECT(tmp_compexpr_left_1, tmp_compexpr_right_1);
        Py_DECREF(tmp_compexpr_left_1);
        if (tmp_tmp_condition_result_1_object_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 53;

            goto frame_exception_exit_1;
        }
        tmp_truth_name_1 = CHECK_IF_TRUE(tmp_tmp_condition_result_1_object_1);
        if (tmp_truth_name_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_tmp_condition_result_1_object_1);

            exception_lineno = 53;

            goto frame_exception_exit_1;
        }
        tmp_condition_result_1 = tmp_truth_name_1 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_tmp_condition_result_1_object_1);
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    {
        PyObject *tmp_raise_type_1;
        PyObject *tmp_make_exception_arg_1;
        tmp_make_exception_arg_1 = mod_consts[38];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 54;
        tmp_raise_type_1 = CALL_FUNCTION_WITH_SINGLE_ARG(PyExc_ImportError, tmp_make_exception_arg_1);
        assert(!(tmp_raise_type_1 == NULL));
        exception_type = tmp_raise_type_1;
        exception_lineno = 54;
        RAISE_EXCEPTION_WITH_TYPE(&exception_type, &exception_value, &exception_tb);

        goto frame_exception_exit_1;
    }
    branch_no_1:;
    {
        PyObject *tmp_assign_source_21;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_called_instance_2;
        PyObject *tmp_subscript_name_1;
        tmp_called_instance_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[17]);

        if (unlikely(tmp_called_instance_2 == NULL)) {
            tmp_called_instance_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
        }

        if (tmp_called_instance_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 56;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 56;
        tmp_expression_name_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_2, mod_consts[39]);
        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 56;

            goto frame_exception_exit_1;
        }
        tmp_subscript_name_1 = mod_consts[18];
        tmp_assign_source_21 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_1, tmp_subscript_name_1, 0);
        Py_DECREF(tmp_expression_name_1);
        if (tmp_assign_source_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 56;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[40], tmp_assign_source_21);
    }
    {
        PyObject *tmp_assign_source_22;
        PyObject *tmp_tuple_arg_1;
        PyObject *tmp_called_name_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_called_instance_3;
        tmp_called_name_1 = (PyObject *)&PyMap_Type;
        tmp_args_element_name_1 = (PyObject *)&PyLong_Type;
        tmp_called_instance_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[40]);

        if (unlikely(tmp_called_instance_3 == NULL)) {
            tmp_called_instance_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[40]);
        }

        assert(!(tmp_called_instance_3 == NULL));
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 57;
        tmp_args_element_name_2 = CALL_METHOD_WITH_ARGS1(
            tmp_called_instance_3,
            mod_consts[41],
            &PyTuple_GET_ITEM(mod_consts[42], 0)
        );

        if (tmp_args_element_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 57;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 57;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_tuple_arg_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_args_element_name_2);
        if (tmp_tuple_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 57;

            goto frame_exception_exit_1;
        }
        tmp_assign_source_22 = PySequence_Tuple(tmp_tuple_arg_1);
        Py_DECREF(tmp_tuple_arg_1);
        if (tmp_assign_source_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 57;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[0], tmp_assign_source_22);
    }
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_compexpr_left_2;
        PyObject *tmp_compexpr_right_2;
        tmp_compexpr_left_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[0]);

        if (unlikely(tmp_compexpr_left_2 == NULL)) {
            tmp_compexpr_left_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[0]);
        }

        assert(!(tmp_compexpr_left_2 == NULL));
        tmp_compexpr_right_2 = mod_consts[43];
        tmp_condition_result_2 = RICH_COMPARE_LT_NBOOL_TUPLE_TUPLE(tmp_compexpr_left_2, tmp_compexpr_right_2);
        if (tmp_condition_result_2 == NUITKA_BOOL_EXCEPTION) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto frame_exception_exit_1;
        }
        if (tmp_condition_result_2 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
        assert(tmp_condition_result_2 != NUITKA_BOOL_UNASSIGNED);
    }
    branch_yes_2:;
    {
        PyObject *tmp_raise_type_2;
        PyObject *tmp_make_exception_arg_2;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_subscript_name_2;
        tmp_left_name_1 = mod_consts[44];
        tmp_expression_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[0]);

        if (unlikely(tmp_expression_name_2 == NULL)) {
            tmp_expression_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[0]);
        }

        if (tmp_expression_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 61;

            goto frame_exception_exit_1;
        }
        tmp_subscript_name_2 = mod_consts[18];
        tmp_tuple_element_1 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_2, tmp_subscript_name_2, 0);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 61;

            goto frame_exception_exit_1;
        }
        tmp_right_name_1 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_3;
            PyObject *tmp_subscript_name_3;
            PyTuple_SET_ITEM(tmp_right_name_1, 0, tmp_tuple_element_1);
            tmp_expression_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[0]);

            if (unlikely(tmp_expression_name_3 == NULL)) {
                tmp_expression_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[0]);
            }

            if (tmp_expression_name_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 61;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_3 = mod_consts[45];
            tmp_tuple_element_1 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_3, tmp_subscript_name_3, 1);
            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 61;

                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_right_name_1, 1, tmp_tuple_element_1);
        }
        goto tuple_build_noexception_1;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_right_name_1);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_1:;
        tmp_make_exception_arg_2 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_make_exception_arg_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 60;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 59;
        tmp_raise_type_2 = CALL_FUNCTION_WITH_SINGLE_ARG(PyExc_OSError, tmp_make_exception_arg_2);
        Py_DECREF(tmp_make_exception_arg_2);
        assert(!(tmp_raise_type_2 == NULL));
        exception_type = tmp_raise_type_2;
        exception_lineno = 59;
        RAISE_EXCEPTION_WITH_TYPE(&exception_type, &exception_value, &exception_tb);

        goto frame_exception_exit_1;
    }
    branch_no_2:;
    {
        PyObject *tmp_assign_source_23;


        tmp_assign_source_23 = MAKE_FUNCTION_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$function__1_load_cdll();

        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[46], tmp_assign_source_23);
    }
    {
        PyObject *tmp_assign_source_24;
        PyObject *tmp_called_name_2;
        tmp_called_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[46]);

        if (unlikely(tmp_called_name_2 == NULL)) {
            tmp_called_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[46]);
        }

        assert(!(tmp_called_name_2 == NULL));
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 81;
        tmp_assign_source_24 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_2, &PyTuple_GET_ITEM(mod_consts[47], 0));

        if (tmp_assign_source_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 81;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48], tmp_assign_source_24);
    }
    {
        PyObject *tmp_assign_source_25;
        PyObject *tmp_called_name_3;
        tmp_called_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[46]);

        if (unlikely(tmp_called_name_3 == NULL)) {
            tmp_called_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[46]);
        }

        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 84;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 84;
        tmp_assign_source_25 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_3, &PyTuple_GET_ITEM(mod_consts[49], 0));

        if (tmp_assign_source_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 84;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50], tmp_assign_source_25);
    }
    {
        PyObject *tmp_assign_source_26;
        tmp_assign_source_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[23]);

        if (unlikely(tmp_assign_source_26 == NULL)) {
            tmp_assign_source_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[23]);
        }

        if (tmp_assign_source_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 90;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[51], tmp_assign_source_26);
    }
    {
        PyObject *tmp_assign_source_27;
        tmp_assign_source_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[27]);

        if (unlikely(tmp_assign_source_27 == NULL)) {
            tmp_assign_source_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[27]);
        }

        if (tmp_assign_source_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 91;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52], tmp_assign_source_27);
    }
    {
        PyObject *tmp_assign_source_28;
        tmp_assign_source_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_28 == NULL)) {
            tmp_assign_source_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 92;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[53], tmp_assign_source_28);
    }
    {
        PyObject *tmp_assign_source_29;
        tmp_assign_source_29 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_29 == NULL)) {
            tmp_assign_source_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 93;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[54], tmp_assign_source_29);
    }
    {
        PyObject *tmp_assign_source_30;
        tmp_assign_source_30 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_30 == NULL)) {
            tmp_assign_source_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 94;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[55], tmp_assign_source_30);
    }
    {
        PyObject *tmp_assign_source_31;
        tmp_assign_source_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_31 == NULL)) {
            tmp_assign_source_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[56], tmp_assign_source_31);
    }
    {
        PyObject *tmp_assign_source_32;
        tmp_assign_source_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_32 == NULL)) {
            tmp_assign_source_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 96;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[57], tmp_assign_source_32);
    }
    {
        PyObject *tmp_assign_source_33;
        tmp_assign_source_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_33 == NULL)) {
            tmp_assign_source_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 97;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[58], tmp_assign_source_33);
    }
    {
        PyObject *tmp_assign_source_34;
        tmp_assign_source_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_34 == NULL)) {
            tmp_assign_source_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 98;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[59], tmp_assign_source_34);
    }
    {
        PyObject *tmp_assign_source_35;
        tmp_assign_source_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_35 == NULL)) {
            tmp_assign_source_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 99;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[60], tmp_assign_source_35);
    }
    {
        PyObject *tmp_assign_source_36;
        tmp_assign_source_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[30]);

        if (unlikely(tmp_assign_source_36 == NULL)) {
            tmp_assign_source_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[30]);
        }

        if (tmp_assign_source_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 100;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[61], tmp_assign_source_36);
    }
    {
        PyObject *tmp_assign_source_37;
        PyObject *tmp_called_name_4;
        PyObject *tmp_args_element_name_3;
        tmp_called_name_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_4 == NULL)) {
            tmp_called_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 102;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[60]);

        if (unlikely(tmp_args_element_name_3 == NULL)) {
            tmp_args_element_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[60]);
        }

        assert(!(tmp_args_element_name_3 == NULL));
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 102;
        tmp_assign_source_37 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_4, tmp_args_element_name_3);
        if (tmp_assign_source_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 102;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62], tmp_assign_source_37);
    }
    {
        PyObject *tmp_assign_source_38;
        tmp_assign_source_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_38 == NULL)) {
            tmp_assign_source_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 103;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63], tmp_assign_source_38);
    }
    {
        PyObject *tmp_assign_source_39;
        tmp_assign_source_39 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[26]);

        if (unlikely(tmp_assign_source_39 == NULL)) {
            tmp_assign_source_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
        }

        if (tmp_assign_source_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 105;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64], tmp_assign_source_39);
    }
    {
        PyObject *tmp_assign_source_40;
        PyObject *tmp_called_name_5;
        PyObject *tmp_args_element_name_4;
        tmp_called_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_5 == NULL)) {
            tmp_called_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 107;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[54]);

        if (unlikely(tmp_args_element_name_4 == NULL)) {
            tmp_args_element_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[54]);
        }

        if (tmp_args_element_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 107;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 107;
        tmp_assign_source_40 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_5, tmp_args_element_name_4);
        if (tmp_assign_source_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 107;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65], tmp_assign_source_40);
    }
    {
        PyObject *tmp_assign_source_41;
        PyObject *tmp_called_name_6;
        PyObject *tmp_args_element_name_5;
        tmp_called_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_6 == NULL)) {
            tmp_called_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 108;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[55]);

        if (unlikely(tmp_args_element_name_5 == NULL)) {
            tmp_args_element_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[55]);
        }

        if (tmp_args_element_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 108;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 108;
        tmp_assign_source_41 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_6, tmp_args_element_name_5);
        if (tmp_assign_source_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 108;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66], tmp_assign_source_41);
    }
    {
        PyObject *tmp_assign_source_42;
        PyObject *tmp_called_name_7;
        PyObject *tmp_args_element_name_6;
        tmp_called_name_7 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_7 == NULL)) {
            tmp_called_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 109;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[56]);

        if (unlikely(tmp_args_element_name_6 == NULL)) {
            tmp_args_element_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[56]);
        }

        if (tmp_args_element_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 109;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 109;
        tmp_assign_source_42 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_7, tmp_args_element_name_6);
        if (tmp_assign_source_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 109;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67], tmp_assign_source_42);
    }
    {
        PyObject *tmp_assign_source_43;
        PyObject *tmp_called_name_8;
        PyObject *tmp_args_element_name_7;
        tmp_called_name_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_8 == NULL)) {
            tmp_called_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_7 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[57]);

        if (unlikely(tmp_args_element_name_7 == NULL)) {
            tmp_args_element_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[57]);
        }

        if (tmp_args_element_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 110;
        tmp_assign_source_43 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_8, tmp_args_element_name_7);
        if (tmp_assign_source_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[68], tmp_assign_source_43);
    }
    {
        PyObject *tmp_assign_source_44;
        PyObject *tmp_called_name_9;
        PyObject *tmp_args_element_name_8;
        tmp_called_name_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_9 == NULL)) {
            tmp_called_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 111;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[58]);

        if (unlikely(tmp_args_element_name_8 == NULL)) {
            tmp_args_element_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[58]);
        }

        if (tmp_args_element_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 111;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 111;
        tmp_assign_source_44 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_9, tmp_args_element_name_8);
        if (tmp_assign_source_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 111;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[69], tmp_assign_source_44);
    }
    {
        PyObject *tmp_assign_source_45;
        tmp_assign_source_45 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_45 == NULL)) {
            tmp_assign_source_45 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 112;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[70], tmp_assign_source_45);
    }
    {
        PyObject *tmp_assign_source_46;
        tmp_assign_source_46 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_46 == NULL)) {
            tmp_assign_source_46 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 113;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[71], tmp_assign_source_46);
    }
    {
        PyObject *tmp_assign_source_47;
        tmp_assign_source_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_47 == NULL)) {
            tmp_assign_source_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 114;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[72], tmp_assign_source_47);
    }
    {
        PyObject *tmp_assign_source_48;
        PyObject *tmp_called_name_10;
        PyObject *tmp_args_element_name_9;
        tmp_called_name_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_10 == NULL)) {
            tmp_called_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 116;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_args_element_name_9 == NULL)) {
            tmp_args_element_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_args_element_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 116;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 116;
        tmp_assign_source_48 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_10, tmp_args_element_name_9);
        if (tmp_assign_source_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 116;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[73], tmp_assign_source_48);
    }
    {
        PyObject *tmp_assign_source_49;
        tmp_assign_source_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_49 == NULL)) {
            tmp_assign_source_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 117;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[74], tmp_assign_source_49);
    }
    {
        PyObject *tmp_assign_source_50;
        tmp_assign_source_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_50 == NULL)) {
            tmp_assign_source_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 118;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[75], tmp_assign_source_50);
    }
    {
        PyObject *tmp_assign_source_51;
        PyObject *tmp_called_name_11;
        PyObject *tmp_args_element_name_10;
        tmp_called_name_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_11 == NULL)) {
            tmp_called_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 119;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_args_element_name_10 == NULL)) {
            tmp_args_element_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_args_element_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 119;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 119;
        tmp_assign_source_51 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_11, tmp_args_element_name_10);
        if (tmp_assign_source_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 119;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[76], tmp_assign_source_51);
    }
    {
        PyObject *tmp_assign_source_52;
        tmp_assign_source_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_52 == NULL)) {
            tmp_assign_source_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[77], tmp_assign_source_52);
    }
    {
        PyObject *tmp_assign_source_53;
        tmp_assign_source_53 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assign_source_53 == NULL)) {
            tmp_assign_source_53 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assign_source_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 121;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[78], tmp_assign_source_53);
    }
    {
        PyObject *tmp_assign_source_54;
        PyObject *tmp_called_name_12;
        PyObject *tmp_args_element_name_11;
        tmp_called_name_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_12 == NULL)) {
            tmp_called_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 122;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_args_element_name_11 == NULL)) {
            tmp_args_element_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_args_element_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 122;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 122;
        tmp_assign_source_54 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_12, tmp_args_element_name_11);
        if (tmp_assign_source_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 122;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[79], tmp_assign_source_54);
    }
    {
        PyObject *tmp_assign_source_55;
        tmp_assign_source_55 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_55 == NULL)) {
            tmp_assign_source_55 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 123;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[80], tmp_assign_source_55);
    }
    {
        PyObject *tmp_assign_source_56;
        tmp_assign_source_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_56 == NULL)) {
            tmp_assign_source_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 124;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[81], tmp_assign_source_56);
    }
    {
        PyObject *tmp_assign_source_57;
        PyObject *tmp_called_name_13;
        PyObject *tmp_args_element_name_12;
        tmp_called_name_13 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_13 == NULL)) {
            tmp_called_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 125;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_args_element_name_12 == NULL)) {
            tmp_args_element_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_args_element_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 125;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 125;
        tmp_assign_source_57 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_13, tmp_args_element_name_12);
        if (tmp_assign_source_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 125;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82], tmp_assign_source_57);
    }
    {
        PyObject *tmp_assign_source_58;
        PyObject *tmp_called_name_14;
        PyObject *tmp_args_element_name_13;
        tmp_called_name_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_14 == NULL)) {
            tmp_called_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 126;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_13 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_args_element_name_13 == NULL)) {
            tmp_args_element_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_args_element_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 126;

            goto frame_exception_exit_1;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 126;
        tmp_assign_source_58 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_14, tmp_args_element_name_13);
        if (tmp_assign_source_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 126;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83], tmp_assign_source_58);
    }
    {
        PyObject *tmp_assign_source_59;
        tmp_assign_source_59 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_59 == NULL)) {
            tmp_assign_source_59 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 127;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[84], tmp_assign_source_59);
    }
    {
        PyObject *tmp_assign_source_60;
        tmp_assign_source_60 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_60 == NULL)) {
            tmp_assign_source_60 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[85], tmp_assign_source_60);
    }
    {
        PyObject *tmp_assign_source_61;
        tmp_assign_source_61 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_61 == NULL)) {
            tmp_assign_source_61 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 129;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[86], tmp_assign_source_61);
    }
    {
        PyObject *tmp_assign_source_62;
        tmp_assign_source_62 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_62 == NULL)) {
            tmp_assign_source_62 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 130;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[87], tmp_assign_source_62);
    }
    {
        PyObject *tmp_assign_source_63;
        tmp_assign_source_63 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_63 == NULL)) {
            tmp_assign_source_63 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 131;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[88], tmp_assign_source_63);
    }
    {
        PyObject *tmp_assign_source_64;
        tmp_assign_source_64 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

        if (unlikely(tmp_assign_source_64 == NULL)) {
            tmp_assign_source_64 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
        }

        if (tmp_assign_source_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 132;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[89], tmp_assign_source_64);
    }
    // Tried code:
    {
        PyObject *tmp_assattr_name_3;
        PyObject *tmp_list_element_1;
        PyObject *tmp_assattr_target_3;
        PyObject *tmp_expression_name_4;
        tmp_list_element_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65]);

        if (unlikely(tmp_list_element_1 == NULL)) {
            tmp_list_element_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[65]);
        }

        if (tmp_list_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 137;

            goto try_except_handler_2;
        }
        tmp_assattr_name_3 = PyList_New(8);
        {
            PyObject *tmp_called_name_15;
            PyObject *tmp_args_element_name_14;
            PyObject *tmp_called_name_16;
            PyObject *tmp_args_element_name_15;
            PyObject *tmp_called_name_17;
            PyObject *tmp_args_element_name_16;
            PyObject *tmp_called_name_18;
            PyObject *tmp_args_element_name_17;
            PyList_SET_ITEM0(tmp_assattr_name_3, 0, tmp_list_element_1);
            tmp_list_element_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

            if (unlikely(tmp_list_element_1 == NULL)) {
                tmp_list_element_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
            }

            if (tmp_list_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 138;

                goto list_build_exception_1;
            }
            PyList_SET_ITEM0(tmp_assattr_name_3, 1, tmp_list_element_1);
            tmp_called_name_15 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_15 == NULL)) {
                tmp_called_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_15 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 139;

                goto list_build_exception_1;
            }
            tmp_args_element_name_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[74]);

            if (unlikely(tmp_args_element_name_14 == NULL)) {
                tmp_args_element_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[74]);
            }

            if (tmp_args_element_name_14 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 139;

                goto list_build_exception_1;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 139;
            tmp_list_element_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_15, tmp_args_element_name_14);
            if (tmp_list_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 139;

                goto list_build_exception_1;
            }
            PyList_SET_ITEM(tmp_assattr_name_3, 2, tmp_list_element_1);
            tmp_called_name_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_16 == NULL)) {
                tmp_called_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;

                goto list_build_exception_1;
            }
            tmp_args_element_name_15 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[75]);

            if (unlikely(tmp_args_element_name_15 == NULL)) {
                tmp_args_element_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[75]);
            }

            if (tmp_args_element_name_15 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;

                goto list_build_exception_1;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 140;
            tmp_list_element_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_16, tmp_args_element_name_15);
            if (tmp_list_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;

                goto list_build_exception_1;
            }
            PyList_SET_ITEM(tmp_assattr_name_3, 3, tmp_list_element_1);
            tmp_list_element_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[77]);

            if (unlikely(tmp_list_element_1 == NULL)) {
                tmp_list_element_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[77]);
            }

            if (tmp_list_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 141;

                goto list_build_exception_1;
            }
            PyList_SET_ITEM0(tmp_assattr_name_3, 4, tmp_list_element_1);
            tmp_called_name_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_17 == NULL)) {
                tmp_called_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 142;

                goto list_build_exception_1;
            }
            tmp_args_element_name_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[78]);

            if (unlikely(tmp_args_element_name_16 == NULL)) {
                tmp_args_element_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[78]);
            }

            if (tmp_args_element_name_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 142;

                goto list_build_exception_1;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 142;
            tmp_list_element_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_17, tmp_args_element_name_16);
            if (tmp_list_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 142;

                goto list_build_exception_1;
            }
            PyList_SET_ITEM(tmp_assattr_name_3, 5, tmp_list_element_1);
            tmp_list_element_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[79]);

            if (unlikely(tmp_list_element_1 == NULL)) {
                tmp_list_element_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[79]);
            }

            if (tmp_list_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 143;

                goto list_build_exception_1;
            }
            PyList_SET_ITEM0(tmp_assattr_name_3, 6, tmp_list_element_1);
            tmp_called_name_18 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_18 == NULL)) {
                tmp_called_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_18 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 144;

                goto list_build_exception_1;
            }
            tmp_args_element_name_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

            if (unlikely(tmp_args_element_name_17 == NULL)) {
                tmp_args_element_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
            }

            if (tmp_args_element_name_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 144;

                goto list_build_exception_1;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 144;
            tmp_list_element_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_18, tmp_args_element_name_17);
            if (tmp_list_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 144;

                goto list_build_exception_1;
            }
            PyList_SET_ITEM(tmp_assattr_name_3, 7, tmp_list_element_1);
        }
        goto list_build_noexception_1;
        // Exception handling pass through code for list_build:
        list_build_exception_1:;
        Py_DECREF(tmp_assattr_name_3);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_1:;
        tmp_expression_name_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_4 == NULL)) {
            tmp_expression_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_3);

            exception_lineno = 136;

            goto try_except_handler_2;
        }
        tmp_assattr_target_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_4, mod_consts[90]);
        if (tmp_assattr_target_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_3);

            exception_lineno = 136;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_3, mod_consts[91], tmp_assattr_name_3);
        Py_DECREF(tmp_assattr_name_3);
        Py_DECREF(tmp_assattr_target_3);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 136;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_4;
        PyObject *tmp_assattr_target_4;
        PyObject *tmp_expression_name_5;
        tmp_assattr_name_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_4 == NULL)) {
            tmp_assattr_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_2;
        }
        tmp_expression_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_5 == NULL)) {
            tmp_expression_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_2;
        }
        tmp_assattr_target_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_5, mod_consts[90]);
        if (tmp_assattr_target_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_4, mod_consts[92], tmp_assattr_name_4);
        Py_DECREF(tmp_assattr_target_4);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_5;
        PyObject *tmp_assattr_target_5;
        PyObject *tmp_expression_name_6;
        tmp_assattr_name_5 = PyList_New(0);
        tmp_expression_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_6 == NULL)) {
            tmp_expression_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_5);

            exception_lineno = 148;

            goto try_except_handler_2;
        }
        tmp_assattr_target_5 = LOOKUP_ATTRIBUTE(tmp_expression_name_6, mod_consts[93]);
        if (tmp_assattr_target_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_5);

            exception_lineno = 148;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_5, mod_consts[91], tmp_assattr_name_5);
        Py_DECREF(tmp_assattr_name_5);
        Py_DECREF(tmp_assattr_target_5);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 148;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_6;
        PyObject *tmp_assattr_target_6;
        PyObject *tmp_expression_name_7;
        tmp_assattr_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[61]);

        if (unlikely(tmp_assattr_name_6 == NULL)) {
            tmp_assattr_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[61]);
        }

        if (tmp_assattr_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 149;

            goto try_except_handler_2;
        }
        tmp_expression_name_7 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_7 == NULL)) {
            tmp_expression_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 149;

            goto try_except_handler_2;
        }
        tmp_assattr_target_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_7, mod_consts[93]);
        if (tmp_assattr_target_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 149;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_6, mod_consts[92], tmp_assattr_name_6);
        Py_DECREF(tmp_assattr_target_6);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 149;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_7;
        PyObject *tmp_assattr_target_7;
        PyObject *tmp_expression_name_8;
        tmp_assattr_name_7 = PyList_New(0);
        tmp_expression_name_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_8 == NULL)) {
            tmp_expression_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_7);

            exception_lineno = 151;

            goto try_except_handler_2;
        }
        tmp_assattr_target_7 = LOOKUP_ATTRIBUTE(tmp_expression_name_8, mod_consts[94]);
        if (tmp_assattr_target_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_7);

            exception_lineno = 151;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_7, mod_consts[91], tmp_assattr_name_7);
        Py_DECREF(tmp_assattr_name_7);
        Py_DECREF(tmp_assattr_target_7);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 151;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_8;
        PyObject *tmp_assattr_target_8;
        PyObject *tmp_expression_name_9;
        tmp_assattr_name_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[61]);

        if (unlikely(tmp_assattr_name_8 == NULL)) {
            tmp_assattr_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[61]);
        }

        if (tmp_assattr_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_2;
        }
        tmp_expression_name_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_9 == NULL)) {
            tmp_expression_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_2;
        }
        tmp_assattr_target_8 = LOOKUP_ATTRIBUTE(tmp_expression_name_9, mod_consts[94]);
        if (tmp_assattr_target_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_8, mod_consts[92], tmp_assattr_name_8);
        Py_DECREF(tmp_assattr_target_8);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_9;
        PyObject *tmp_assattr_target_9;
        PyObject *tmp_expression_name_10;
        tmp_assattr_name_9 = PyList_New(0);
        tmp_expression_name_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_10 == NULL)) {
            tmp_expression_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_9);

            exception_lineno = 154;

            goto try_except_handler_2;
        }
        tmp_assattr_target_9 = LOOKUP_ATTRIBUTE(tmp_expression_name_10, mod_consts[95]);
        if (tmp_assattr_target_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_9);

            exception_lineno = 154;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_9, mod_consts[91], tmp_assattr_name_9);
        Py_DECREF(tmp_assattr_name_9);
        Py_DECREF(tmp_assattr_target_9);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 154;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_10;
        PyObject *tmp_assattr_target_10;
        PyObject *tmp_expression_name_11;
        tmp_assattr_name_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[61]);

        if (unlikely(tmp_assattr_name_10 == NULL)) {
            tmp_assattr_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[61]);
        }

        if (tmp_assattr_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 155;

            goto try_except_handler_2;
        }
        tmp_expression_name_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_11 == NULL)) {
            tmp_expression_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 155;

            goto try_except_handler_2;
        }
        tmp_assattr_target_10 = LOOKUP_ATTRIBUTE(tmp_expression_name_11, mod_consts[95]);
        if (tmp_assattr_target_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 155;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_10, mod_consts[92], tmp_assattr_name_10);
        Py_DECREF(tmp_assattr_target_10);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 155;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_11;
        PyObject *tmp_list_element_2;
        PyObject *tmp_assattr_target_11;
        PyObject *tmp_expression_name_12;
        tmp_list_element_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_list_element_2 == NULL)) {
            tmp_list_element_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_list_element_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 157;

            goto try_except_handler_2;
        }
        tmp_assattr_name_11 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_11, 0, tmp_list_element_2);
        tmp_list_element_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65]);

        if (unlikely(tmp_list_element_2 == NULL)) {
            tmp_list_element_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[65]);
        }

        if (tmp_list_element_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 157;

            goto list_build_exception_2;
        }
        PyList_SET_ITEM0(tmp_assattr_name_11, 1, tmp_list_element_2);
        goto list_build_noexception_2;
        // Exception handling pass through code for list_build:
        list_build_exception_2:;
        Py_DECREF(tmp_assattr_name_11);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_2:;
        tmp_expression_name_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_12 == NULL)) {
            tmp_expression_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_11);

            exception_lineno = 157;

            goto try_except_handler_2;
        }
        tmp_assattr_target_11 = LOOKUP_ATTRIBUTE(tmp_expression_name_12, mod_consts[96]);
        if (tmp_assattr_target_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_11);

            exception_lineno = 157;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_11, mod_consts[91], tmp_assattr_name_11);
        Py_DECREF(tmp_assattr_name_11);
        Py_DECREF(tmp_assattr_target_11);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 157;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_12;
        PyObject *tmp_assattr_target_12;
        PyObject *tmp_expression_name_13;
        tmp_assattr_name_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[73]);

        if (unlikely(tmp_assattr_name_12 == NULL)) {
            tmp_assattr_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[73]);
        }

        if (tmp_assattr_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_2;
        }
        tmp_expression_name_13 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_13 == NULL)) {
            tmp_expression_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_2;
        }
        tmp_assattr_target_12 = LOOKUP_ATTRIBUTE(tmp_expression_name_13, mod_consts[96]);
        if (tmp_assattr_target_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_12, mod_consts[92], tmp_assattr_name_12);
        Py_DECREF(tmp_assattr_target_12);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_13;
        PyObject *tmp_list_element_3;
        PyObject *tmp_assattr_target_13;
        PyObject *tmp_expression_name_14;
        tmp_list_element_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[73]);

        if (unlikely(tmp_list_element_3 == NULL)) {
            tmp_list_element_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[73]);
        }

        if (tmp_list_element_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 160;

            goto try_except_handler_2;
        }
        tmp_assattr_name_13 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_13, 0, tmp_list_element_3);
        tmp_expression_name_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_14 == NULL)) {
            tmp_expression_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_13);

            exception_lineno = 160;

            goto try_except_handler_2;
        }
        tmp_assattr_target_13 = LOOKUP_ATTRIBUTE(tmp_expression_name_14, mod_consts[97]);
        if (tmp_assattr_target_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_13);

            exception_lineno = 160;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_13, mod_consts[91], tmp_assattr_name_13);
        Py_DECREF(tmp_assattr_name_13);
        Py_DECREF(tmp_assattr_target_13);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 160;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_14;
        PyObject *tmp_assattr_target_14;
        PyObject *tmp_expression_name_15;
        tmp_assattr_name_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65]);

        if (unlikely(tmp_assattr_name_14 == NULL)) {
            tmp_assattr_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[65]);
        }

        if (tmp_assattr_name_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 161;

            goto try_except_handler_2;
        }
        tmp_expression_name_15 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_15 == NULL)) {
            tmp_expression_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 161;

            goto try_except_handler_2;
        }
        tmp_assattr_target_14 = LOOKUP_ATTRIBUTE(tmp_expression_name_15, mod_consts[97]);
        if (tmp_assattr_target_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 161;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_14, mod_consts[92], tmp_assattr_name_14);
        Py_DECREF(tmp_assattr_target_14);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 161;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_15;
        PyObject *tmp_list_element_4;
        PyObject *tmp_assattr_target_15;
        PyObject *tmp_expression_name_16;
        tmp_list_element_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_list_element_4 == NULL)) {
            tmp_list_element_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_list_element_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;

            goto try_except_handler_2;
        }
        tmp_assattr_name_15 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_15, 0, tmp_list_element_4);
        tmp_list_element_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_list_element_4 == NULL)) {
            tmp_list_element_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_list_element_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;

            goto list_build_exception_3;
        }
        PyList_SET_ITEM0(tmp_assattr_name_15, 1, tmp_list_element_4);
        goto list_build_noexception_3;
        // Exception handling pass through code for list_build:
        list_build_exception_3:;
        Py_DECREF(tmp_assattr_name_15);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_3:;
        tmp_expression_name_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_16 == NULL)) {
            tmp_expression_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_15);

            exception_lineno = 163;

            goto try_except_handler_2;
        }
        tmp_assattr_target_15 = LOOKUP_ATTRIBUTE(tmp_expression_name_16, mod_consts[98]);
        if (tmp_assattr_target_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_15);

            exception_lineno = 163;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_15, mod_consts[91], tmp_assattr_name_15);
        Py_DECREF(tmp_assattr_name_15);
        Py_DECREF(tmp_assattr_target_15);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_16;
        PyObject *tmp_assattr_target_16;
        PyObject *tmp_expression_name_17;
        tmp_assattr_name_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_assattr_name_16 == NULL)) {
            tmp_assattr_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_assattr_name_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 164;

            goto try_except_handler_2;
        }
        tmp_expression_name_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_17 == NULL)) {
            tmp_expression_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 164;

            goto try_except_handler_2;
        }
        tmp_assattr_target_16 = LOOKUP_ATTRIBUTE(tmp_expression_name_17, mod_consts[98]);
        if (tmp_assattr_target_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 164;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_16, mod_consts[92], tmp_assattr_name_16);
        Py_DECREF(tmp_assattr_target_16);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 164;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_17;
        PyObject *tmp_list_element_5;
        PyObject *tmp_assattr_target_17;
        PyObject *tmp_expression_name_18;
        tmp_list_element_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_list_element_5 == NULL)) {
            tmp_list_element_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_list_element_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 167;

            goto try_except_handler_2;
        }
        tmp_assattr_name_17 = PyList_New(3);
        {
            PyObject *tmp_called_name_19;
            PyObject *tmp_args_element_name_18;
            PyList_SET_ITEM0(tmp_assattr_name_17, 0, tmp_list_element_5);
            tmp_list_element_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[73]);

            if (unlikely(tmp_list_element_5 == NULL)) {
                tmp_list_element_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[73]);
            }

            if (tmp_list_element_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 168;

                goto list_build_exception_4;
            }
            PyList_SET_ITEM0(tmp_assattr_name_17, 1, tmp_list_element_5);
            tmp_called_name_19 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_19 == NULL)) {
                tmp_called_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 169;

                goto list_build_exception_4;
            }
            tmp_args_element_name_18 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[76]);

            if (unlikely(tmp_args_element_name_18 == NULL)) {
                tmp_args_element_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[76]);
            }

            if (tmp_args_element_name_18 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 169;

                goto list_build_exception_4;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 169;
            tmp_list_element_5 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_19, tmp_args_element_name_18);
            if (tmp_list_element_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 169;

                goto list_build_exception_4;
            }
            PyList_SET_ITEM(tmp_assattr_name_17, 2, tmp_list_element_5);
        }
        goto list_build_noexception_4;
        // Exception handling pass through code for list_build:
        list_build_exception_4:;
        Py_DECREF(tmp_assattr_name_17);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_4:;
        tmp_expression_name_18 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_18 == NULL)) {
            tmp_expression_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_17);

            exception_lineno = 166;

            goto try_except_handler_2;
        }
        tmp_assattr_target_17 = LOOKUP_ATTRIBUTE(tmp_expression_name_18, mod_consts[99]);
        if (tmp_assattr_target_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_17);

            exception_lineno = 166;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_17, mod_consts[91], tmp_assattr_name_17);
        Py_DECREF(tmp_assattr_name_17);
        Py_DECREF(tmp_assattr_target_17);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 166;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_18;
        PyObject *tmp_assattr_target_18;
        PyObject *tmp_expression_name_19;
        tmp_assattr_name_18 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_18 == NULL)) {
            tmp_assattr_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 171;

            goto try_except_handler_2;
        }
        tmp_expression_name_19 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_19 == NULL)) {
            tmp_expression_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 171;

            goto try_except_handler_2;
        }
        tmp_assattr_target_18 = LOOKUP_ATTRIBUTE(tmp_expression_name_19, mod_consts[99]);
        if (tmp_assattr_target_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 171;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_18, mod_consts[92], tmp_assattr_name_18);
        Py_DECREF(tmp_assattr_target_18);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 171;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_19;
        PyObject *tmp_list_element_6;
        PyObject *tmp_assattr_target_19;
        PyObject *tmp_expression_name_20;
        tmp_list_element_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

        if (unlikely(tmp_list_element_6 == NULL)) {
            tmp_list_element_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
        }

        if (tmp_list_element_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 174;

            goto try_except_handler_2;
        }
        tmp_assattr_name_19 = PyList_New(6);
        {
            PyObject *tmp_called_name_20;
            PyObject *tmp_args_element_name_19;
            PyList_SET_ITEM0(tmp_assattr_name_19, 0, tmp_list_element_6);
            tmp_list_element_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[29]);

            if (unlikely(tmp_list_element_6 == NULL)) {
                tmp_list_element_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[29]);
            }

            if (tmp_list_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 175;

                goto list_build_exception_5;
            }
            PyList_SET_ITEM0(tmp_assattr_name_19, 1, tmp_list_element_6);
            tmp_list_element_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

            if (unlikely(tmp_list_element_6 == NULL)) {
                tmp_list_element_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
            }

            if (tmp_list_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 176;

                goto list_build_exception_5;
            }
            PyList_SET_ITEM0(tmp_assattr_name_19, 2, tmp_list_element_6);
            tmp_list_element_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[51]);

            if (unlikely(tmp_list_element_6 == NULL)) {
                tmp_list_element_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[51]);
            }

            if (tmp_list_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 177;

                goto list_build_exception_5;
            }
            PyList_SET_ITEM0(tmp_assattr_name_19, 3, tmp_list_element_6);
            tmp_list_element_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

            if (unlikely(tmp_list_element_6 == NULL)) {
                tmp_list_element_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
            }

            if (tmp_list_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 178;

                goto list_build_exception_5;
            }
            PyList_SET_ITEM0(tmp_assattr_name_19, 4, tmp_list_element_6);
            tmp_called_name_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_20 == NULL)) {
                tmp_called_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_20 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 179;

                goto list_build_exception_5;
            }
            tmp_args_element_name_19 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[79]);

            if (unlikely(tmp_args_element_name_19 == NULL)) {
                tmp_args_element_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[79]);
            }

            if (tmp_args_element_name_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 179;

                goto list_build_exception_5;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 179;
            tmp_list_element_6 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_20, tmp_args_element_name_19);
            if (tmp_list_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 179;

                goto list_build_exception_5;
            }
            PyList_SET_ITEM(tmp_assattr_name_19, 5, tmp_list_element_6);
        }
        goto list_build_noexception_5;
        // Exception handling pass through code for list_build:
        list_build_exception_5:;
        Py_DECREF(tmp_assattr_name_19);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_5:;
        tmp_expression_name_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_20 == NULL)) {
            tmp_expression_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_19);

            exception_lineno = 173;

            goto try_except_handler_2;
        }
        tmp_assattr_target_19 = LOOKUP_ATTRIBUTE(tmp_expression_name_20, mod_consts[100]);
        if (tmp_assattr_target_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_19);

            exception_lineno = 173;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_19, mod_consts[91], tmp_assattr_name_19);
        Py_DECREF(tmp_assattr_name_19);
        Py_DECREF(tmp_assattr_target_19);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 173;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_20;
        PyObject *tmp_assattr_target_20;
        PyObject *tmp_expression_name_21;
        tmp_assattr_name_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_20 == NULL)) {
            tmp_assattr_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 181;

            goto try_except_handler_2;
        }
        tmp_expression_name_21 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_21 == NULL)) {
            tmp_expression_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 181;

            goto try_except_handler_2;
        }
        tmp_assattr_target_20 = LOOKUP_ATTRIBUTE(tmp_expression_name_21, mod_consts[100]);
        if (tmp_assattr_target_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 181;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_20, mod_consts[92], tmp_assattr_name_20);
        Py_DECREF(tmp_assattr_target_20);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 181;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_21;
        PyObject *tmp_list_element_7;
        PyObject *tmp_assattr_target_21;
        PyObject *tmp_expression_name_22;
        tmp_list_element_7 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[79]);

        if (unlikely(tmp_list_element_7 == NULL)) {
            tmp_list_element_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[79]);
        }

        if (tmp_list_element_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 183;

            goto try_except_handler_2;
        }
        tmp_assattr_name_21 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_21, 0, tmp_list_element_7);
        tmp_expression_name_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_22 == NULL)) {
            tmp_expression_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_21);

            exception_lineno = 183;

            goto try_except_handler_2;
        }
        tmp_assattr_target_21 = LOOKUP_ATTRIBUTE(tmp_expression_name_22, mod_consts[101]);
        if (tmp_assattr_target_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_21);

            exception_lineno = 183;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_21, mod_consts[91], tmp_assattr_name_21);
        Py_DECREF(tmp_assattr_name_21);
        Py_DECREF(tmp_assattr_target_21);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 183;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_22;
        PyObject *tmp_assattr_target_22;
        PyObject *tmp_expression_name_23;
        tmp_assattr_name_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_22 == NULL)) {
            tmp_assattr_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 184;

            goto try_except_handler_2;
        }
        tmp_expression_name_23 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_23 == NULL)) {
            tmp_expression_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 184;

            goto try_except_handler_2;
        }
        tmp_assattr_target_22 = LOOKUP_ATTRIBUTE(tmp_expression_name_23, mod_consts[101]);
        if (tmp_assattr_target_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 184;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_22, mod_consts[92], tmp_assattr_name_22);
        Py_DECREF(tmp_assattr_target_22);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 184;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_23;
        PyObject *tmp_list_element_8;
        PyObject *tmp_assattr_target_23;
        PyObject *tmp_expression_name_24;
        tmp_list_element_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65]);

        if (unlikely(tmp_list_element_8 == NULL)) {
            tmp_list_element_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[65]);
        }

        if (tmp_list_element_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 187;

            goto try_except_handler_2;
        }
        tmp_assattr_name_23 = PyList_New(3);
        {
            PyObject *tmp_called_name_21;
            PyObject *tmp_args_element_name_20;
            PyList_SET_ITEM0(tmp_assattr_name_23, 0, tmp_list_element_8);
            tmp_list_element_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[69]);

            if (unlikely(tmp_list_element_8 == NULL)) {
                tmp_list_element_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[69]);
            }

            if (tmp_list_element_8 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 188;

                goto list_build_exception_6;
            }
            PyList_SET_ITEM0(tmp_assattr_name_23, 1, tmp_list_element_8);
            tmp_called_name_21 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_21 == NULL)) {
                tmp_called_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 189;

                goto list_build_exception_6;
            }
            tmp_args_element_name_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

            if (unlikely(tmp_args_element_name_20 == NULL)) {
                tmp_args_element_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
            }

            if (tmp_args_element_name_20 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 189;

                goto list_build_exception_6;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 189;
            tmp_list_element_8 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_21, tmp_args_element_name_20);
            if (tmp_list_element_8 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 189;

                goto list_build_exception_6;
            }
            PyList_SET_ITEM(tmp_assattr_name_23, 2, tmp_list_element_8);
        }
        goto list_build_noexception_6;
        // Exception handling pass through code for list_build:
        list_build_exception_6:;
        Py_DECREF(tmp_assattr_name_23);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_6:;
        tmp_expression_name_24 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_24 == NULL)) {
            tmp_expression_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_23);

            exception_lineno = 186;

            goto try_except_handler_2;
        }
        tmp_assattr_target_23 = LOOKUP_ATTRIBUTE(tmp_expression_name_24, mod_consts[102]);
        if (tmp_assattr_target_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_23);

            exception_lineno = 186;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_23, mod_consts[91], tmp_assattr_name_23);
        Py_DECREF(tmp_assattr_name_23);
        Py_DECREF(tmp_assattr_target_23);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 186;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_24;
        PyObject *tmp_assattr_target_24;
        PyObject *tmp_expression_name_25;
        tmp_assattr_name_24 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_24 == NULL)) {
            tmp_assattr_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_2;
        }
        tmp_expression_name_25 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_25 == NULL)) {
            tmp_expression_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_2;
        }
        tmp_assattr_target_24 = LOOKUP_ATTRIBUTE(tmp_expression_name_25, mod_consts[102]);
        if (tmp_assattr_target_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_24, mod_consts[92], tmp_assattr_name_24);
        Py_DECREF(tmp_assattr_target_24);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assign_source_65;
        PyObject *tmp_called_name_22;
        PyObject *tmp_args_element_name_21;
        PyObject *tmp_args_element_name_22;
        PyObject *tmp_args_element_name_23;
        PyObject *tmp_args_element_name_24;
        PyObject *tmp_called_name_23;
        PyObject *tmp_args_element_name_25;
        tmp_called_name_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_22 == NULL)) {
            tmp_called_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        tmp_args_element_name_21 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_args_element_name_21 == NULL)) {
            tmp_args_element_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_args_element_name_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        tmp_args_element_name_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[84]);

        if (unlikely(tmp_args_element_name_22 == NULL)) {
            tmp_args_element_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[84]);
        }

        if (tmp_args_element_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        tmp_args_element_name_23 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_args_element_name_23 == NULL)) {
            tmp_args_element_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_args_element_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        tmp_called_name_23 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_23 == NULL)) {
            tmp_called_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        tmp_args_element_name_25 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

        if (unlikely(tmp_args_element_name_25 == NULL)) {
            tmp_args_element_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
        }

        if (tmp_args_element_name_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 193;
        tmp_args_element_name_24 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_23, tmp_args_element_name_25);
        if (tmp_args_element_name_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 193;
        {
            PyObject *call_args[] = {tmp_args_element_name_21, tmp_args_element_name_22, tmp_args_element_name_23, tmp_args_element_name_24};
            tmp_assign_source_65 = CALL_FUNCTION_WITH_ARGS4(tmp_called_name_22, call_args);
        }

        Py_DECREF(tmp_args_element_name_24);
        if (tmp_assign_source_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 193;

            goto try_except_handler_2;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[103], tmp_assign_source_65);
    }
    {
        PyObject *tmp_assign_source_66;
        PyObject *tmp_called_name_24;
        PyObject *tmp_args_element_name_26;
        PyObject *tmp_args_element_name_27;
        PyObject *tmp_args_element_name_28;
        PyObject *tmp_called_name_25;
        PyObject *tmp_args_element_name_29;
        PyObject *tmp_args_element_name_30;
        PyObject *tmp_called_name_26;
        PyObject *tmp_args_element_name_31;
        tmp_called_name_24 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_24 == NULL)) {
            tmp_called_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 194;

            goto try_except_handler_2;
        }
        tmp_args_element_name_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_args_element_name_26 == NULL)) {
            tmp_args_element_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_args_element_name_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 195;

            goto try_except_handler_2;
        }
        tmp_args_element_name_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[84]);

        if (unlikely(tmp_args_element_name_27 == NULL)) {
            tmp_args_element_name_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[84]);
        }

        if (tmp_args_element_name_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 195;

            goto try_except_handler_2;
        }
        tmp_called_name_25 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_25 == NULL)) {
            tmp_called_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 195;

            goto try_except_handler_2;
        }
        tmp_args_element_name_29 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[24]);

        if (unlikely(tmp_args_element_name_29 == NULL)) {
            tmp_args_element_name_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[24]);
        }

        if (tmp_args_element_name_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 195;

            goto try_except_handler_2;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 195;
        tmp_args_element_name_28 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_25, tmp_args_element_name_29);
        if (tmp_args_element_name_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 195;

            goto try_except_handler_2;
        }
        tmp_called_name_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

        if (unlikely(tmp_called_name_26 == NULL)) {
            tmp_called_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
        }

        if (tmp_called_name_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_args_element_name_28);

            exception_lineno = 195;

            goto try_except_handler_2;
        }
        tmp_args_element_name_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

        if (unlikely(tmp_args_element_name_31 == NULL)) {
            tmp_args_element_name_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
        }

        if (tmp_args_element_name_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_args_element_name_28);

            exception_lineno = 195;

            goto try_except_handler_2;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 195;
        tmp_args_element_name_30 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_26, tmp_args_element_name_31);
        if (tmp_args_element_name_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_args_element_name_28);

            exception_lineno = 195;

            goto try_except_handler_2;
        }
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 194;
        {
            PyObject *call_args[] = {tmp_args_element_name_26, tmp_args_element_name_27, tmp_args_element_name_28, tmp_args_element_name_30};
            tmp_assign_source_66 = CALL_FUNCTION_WITH_ARGS4(tmp_called_name_24, call_args);
        }

        Py_DECREF(tmp_args_element_name_28);
        Py_DECREF(tmp_args_element_name_30);
        if (tmp_assign_source_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 194;

            goto try_except_handler_2;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[104], tmp_assign_source_66);
    }
    {
        PyObject *tmp_assattr_name_25;
        PyObject *tmp_list_element_9;
        PyObject *tmp_assattr_target_25;
        PyObject *tmp_expression_name_26;
        tmp_list_element_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_9 == NULL)) {
            tmp_list_element_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 198;

            goto try_except_handler_2;
        }
        tmp_assattr_name_25 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_25, 0, tmp_list_element_9);
        tmp_list_element_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[103]);

        if (unlikely(tmp_list_element_9 == NULL)) {
            tmp_list_element_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[103]);
        }

        if (tmp_list_element_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 198;

            goto list_build_exception_7;
        }
        PyList_SET_ITEM0(tmp_assattr_name_25, 1, tmp_list_element_9);
        tmp_list_element_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[104]);

        if (unlikely(tmp_list_element_9 == NULL)) {
            tmp_list_element_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[104]);
        }

        assert(!(tmp_list_element_9 == NULL));
        PyList_SET_ITEM0(tmp_assattr_name_25, 2, tmp_list_element_9);
        goto list_build_noexception_7;
        // Exception handling pass through code for list_build:
        list_build_exception_7:;
        Py_DECREF(tmp_assattr_name_25);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_7:;
        tmp_expression_name_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_26 == NULL)) {
            tmp_expression_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_25);

            exception_lineno = 198;

            goto try_except_handler_2;
        }
        tmp_assattr_target_25 = LOOKUP_ATTRIBUTE(tmp_expression_name_26, mod_consts[105]);
        if (tmp_assattr_target_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_25);

            exception_lineno = 198;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_25, mod_consts[91], tmp_assattr_name_25);
        Py_DECREF(tmp_assattr_name_25);
        Py_DECREF(tmp_assattr_target_25);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 198;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_26;
        PyObject *tmp_assattr_target_26;
        PyObject *tmp_expression_name_27;
        tmp_assattr_name_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_26 == NULL)) {
            tmp_assattr_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 199;

            goto try_except_handler_2;
        }
        tmp_expression_name_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_27 == NULL)) {
            tmp_expression_name_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 199;

            goto try_except_handler_2;
        }
        tmp_assattr_target_26 = LOOKUP_ATTRIBUTE(tmp_expression_name_27, mod_consts[105]);
        if (tmp_assattr_target_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 199;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_26, mod_consts[92], tmp_assattr_name_26);
        Py_DECREF(tmp_assattr_target_26);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 199;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_27;
        PyObject *tmp_list_element_10;
        PyObject *tmp_assattr_target_27;
        PyObject *tmp_expression_name_28;
        tmp_list_element_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_10 == NULL)) {
            tmp_list_element_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 201;

            goto try_except_handler_2;
        }
        tmp_assattr_name_27 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_27, 0, tmp_list_element_10);
        tmp_list_element_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

        if (unlikely(tmp_list_element_10 == NULL)) {
            tmp_list_element_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
        }

        if (tmp_list_element_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 201;

            goto list_build_exception_8;
        }
        PyList_SET_ITEM0(tmp_assattr_name_27, 1, tmp_list_element_10);
        tmp_list_element_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

        if (unlikely(tmp_list_element_10 == NULL)) {
            tmp_list_element_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
        }

        if (tmp_list_element_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 201;

            goto list_build_exception_8;
        }
        PyList_SET_ITEM0(tmp_assattr_name_27, 2, tmp_list_element_10);
        goto list_build_noexception_8;
        // Exception handling pass through code for list_build:
        list_build_exception_8:;
        Py_DECREF(tmp_assattr_name_27);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_8:;
        tmp_expression_name_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_28 == NULL)) {
            tmp_expression_name_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_27);

            exception_lineno = 201;

            goto try_except_handler_2;
        }
        tmp_assattr_target_27 = LOOKUP_ATTRIBUTE(tmp_expression_name_28, mod_consts[106]);
        if (tmp_assattr_target_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_27);

            exception_lineno = 201;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_27, mod_consts[91], tmp_assattr_name_27);
        Py_DECREF(tmp_assattr_name_27);
        Py_DECREF(tmp_assattr_target_27);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 201;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_28;
        PyObject *tmp_assattr_target_28;
        PyObject *tmp_expression_name_29;
        tmp_assattr_name_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_28 == NULL)) {
            tmp_assattr_name_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 202;

            goto try_except_handler_2;
        }
        tmp_expression_name_29 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_29 == NULL)) {
            tmp_expression_name_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 202;

            goto try_except_handler_2;
        }
        tmp_assattr_target_28 = LOOKUP_ATTRIBUTE(tmp_expression_name_29, mod_consts[106]);
        if (tmp_assattr_target_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 202;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_28, mod_consts[92], tmp_assattr_name_28);
        Py_DECREF(tmp_assattr_target_28);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 202;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_29;
        PyObject *tmp_list_element_11;
        PyObject *tmp_assattr_target_29;
        PyObject *tmp_expression_name_30;
        tmp_list_element_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_11 == NULL)) {
            tmp_list_element_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 204;

            goto try_except_handler_2;
        }
        tmp_assattr_name_29 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_29, 0, tmp_list_element_11);
        tmp_list_element_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

        if (unlikely(tmp_list_element_11 == NULL)) {
            tmp_list_element_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
        }

        if (tmp_list_element_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 204;

            goto list_build_exception_9;
        }
        PyList_SET_ITEM0(tmp_assattr_name_29, 1, tmp_list_element_11);
        goto list_build_noexception_9;
        // Exception handling pass through code for list_build:
        list_build_exception_9:;
        Py_DECREF(tmp_assattr_name_29);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_9:;
        tmp_expression_name_30 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_30 == NULL)) {
            tmp_expression_name_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_29);

            exception_lineno = 204;

            goto try_except_handler_2;
        }
        tmp_assattr_target_29 = LOOKUP_ATTRIBUTE(tmp_expression_name_30, mod_consts[107]);
        if (tmp_assattr_target_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_29);

            exception_lineno = 204;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_29, mod_consts[91], tmp_assattr_name_29);
        Py_DECREF(tmp_assattr_name_29);
        Py_DECREF(tmp_assattr_target_29);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 204;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_30;
        PyObject *tmp_assattr_target_30;
        PyObject *tmp_expression_name_31;
        tmp_assattr_name_30 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_30 == NULL)) {
            tmp_assattr_name_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 205;

            goto try_except_handler_2;
        }
        tmp_expression_name_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_31 == NULL)) {
            tmp_expression_name_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 205;

            goto try_except_handler_2;
        }
        tmp_assattr_target_30 = LOOKUP_ATTRIBUTE(tmp_expression_name_31, mod_consts[107]);
        if (tmp_assattr_target_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 205;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_30, mod_consts[92], tmp_assattr_name_30);
        Py_DECREF(tmp_assattr_target_30);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 205;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_31;
        PyObject *tmp_list_element_12;
        PyObject *tmp_assattr_target_31;
        PyObject *tmp_expression_name_32;
        tmp_list_element_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_12 == NULL)) {
            tmp_list_element_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 207;

            goto try_except_handler_2;
        }
        tmp_assattr_name_31 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_31, 0, tmp_list_element_12);
        tmp_list_element_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_list_element_12 == NULL)) {
            tmp_list_element_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_list_element_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 207;

            goto list_build_exception_10;
        }
        PyList_SET_ITEM0(tmp_assattr_name_31, 1, tmp_list_element_12);
        tmp_list_element_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[51]);

        if (unlikely(tmp_list_element_12 == NULL)) {
            tmp_list_element_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[51]);
        }

        if (tmp_list_element_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 207;

            goto list_build_exception_10;
        }
        PyList_SET_ITEM0(tmp_assattr_name_31, 2, tmp_list_element_12);
        goto list_build_noexception_10;
        // Exception handling pass through code for list_build:
        list_build_exception_10:;
        Py_DECREF(tmp_assattr_name_31);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_10:;
        tmp_expression_name_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_32 == NULL)) {
            tmp_expression_name_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_31);

            exception_lineno = 207;

            goto try_except_handler_2;
        }
        tmp_assattr_target_31 = LOOKUP_ATTRIBUTE(tmp_expression_name_32, mod_consts[108]);
        if (tmp_assattr_target_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_31);

            exception_lineno = 207;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_31, mod_consts[91], tmp_assattr_name_31);
        Py_DECREF(tmp_assattr_name_31);
        Py_DECREF(tmp_assattr_target_31);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 207;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_32;
        PyObject *tmp_assattr_target_32;
        PyObject *tmp_expression_name_33;
        tmp_assattr_name_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_32 == NULL)) {
            tmp_assattr_name_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 208;

            goto try_except_handler_2;
        }
        tmp_expression_name_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_33 == NULL)) {
            tmp_expression_name_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 208;

            goto try_except_handler_2;
        }
        tmp_assattr_target_32 = LOOKUP_ATTRIBUTE(tmp_expression_name_33, mod_consts[108]);
        if (tmp_assattr_target_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 208;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_32, mod_consts[92], tmp_assattr_name_32);
        Py_DECREF(tmp_assattr_target_32);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 208;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_33;
        PyObject *tmp_list_element_13;
        PyObject *tmp_assattr_target_33;
        PyObject *tmp_expression_name_34;
        tmp_list_element_13 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_13 == NULL)) {
            tmp_list_element_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 210;

            goto try_except_handler_2;
        }
        tmp_assattr_name_33 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_33, 0, tmp_list_element_13);
        tmp_list_element_13 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[84]);

        if (unlikely(tmp_list_element_13 == NULL)) {
            tmp_list_element_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[84]);
        }

        if (tmp_list_element_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 210;

            goto list_build_exception_11;
        }
        PyList_SET_ITEM0(tmp_assattr_name_33, 1, tmp_list_element_13);
        goto list_build_noexception_11;
        // Exception handling pass through code for list_build:
        list_build_exception_11:;
        Py_DECREF(tmp_assattr_name_33);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_11:;
        tmp_expression_name_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_34 == NULL)) {
            tmp_expression_name_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_33);

            exception_lineno = 210;

            goto try_except_handler_2;
        }
        tmp_assattr_target_33 = LOOKUP_ATTRIBUTE(tmp_expression_name_34, mod_consts[109]);
        if (tmp_assattr_target_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_33);

            exception_lineno = 210;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_33, mod_consts[91], tmp_assattr_name_33);
        Py_DECREF(tmp_assattr_name_33);
        Py_DECREF(tmp_assattr_target_33);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 210;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_34;
        PyObject *tmp_assattr_target_34;
        PyObject *tmp_expression_name_35;
        tmp_assattr_name_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_34 == NULL)) {
            tmp_assattr_name_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 211;

            goto try_except_handler_2;
        }
        tmp_expression_name_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_35 == NULL)) {
            tmp_expression_name_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 211;

            goto try_except_handler_2;
        }
        tmp_assattr_target_34 = LOOKUP_ATTRIBUTE(tmp_expression_name_35, mod_consts[109]);
        if (tmp_assattr_target_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 211;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_34, mod_consts[92], tmp_assattr_name_34);
        Py_DECREF(tmp_assattr_target_34);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 211;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_35;
        PyObject *tmp_list_element_14;
        PyObject *tmp_assattr_target_35;
        PyObject *tmp_expression_name_36;
        tmp_list_element_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_14 == NULL)) {
            tmp_list_element_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 213;

            goto try_except_handler_2;
        }
        tmp_assattr_name_35 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_35, 0, tmp_list_element_14);
        tmp_list_element_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

        if (unlikely(tmp_list_element_14 == NULL)) {
            tmp_list_element_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
        }

        if (tmp_list_element_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 213;

            goto list_build_exception_12;
        }
        PyList_SET_ITEM0(tmp_assattr_name_35, 1, tmp_list_element_14);
        tmp_list_element_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

        if (unlikely(tmp_list_element_14 == NULL)) {
            tmp_list_element_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
        }

        if (tmp_list_element_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 213;

            goto list_build_exception_12;
        }
        PyList_SET_ITEM0(tmp_assattr_name_35, 2, tmp_list_element_14);
        goto list_build_noexception_12;
        // Exception handling pass through code for list_build:
        list_build_exception_12:;
        Py_DECREF(tmp_assattr_name_35);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_12:;
        tmp_expression_name_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_36 == NULL)) {
            tmp_expression_name_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_35);

            exception_lineno = 213;

            goto try_except_handler_2;
        }
        tmp_assattr_target_35 = LOOKUP_ATTRIBUTE(tmp_expression_name_36, mod_consts[110]);
        if (tmp_assattr_target_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_35);

            exception_lineno = 213;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_35, mod_consts[91], tmp_assattr_name_35);
        Py_DECREF(tmp_assattr_name_35);
        Py_DECREF(tmp_assattr_target_35);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 213;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_36;
        PyObject *tmp_assattr_target_36;
        PyObject *tmp_expression_name_37;
        tmp_assattr_name_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_36 == NULL)) {
            tmp_assattr_name_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 214;

            goto try_except_handler_2;
        }
        tmp_expression_name_37 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_37 == NULL)) {
            tmp_expression_name_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 214;

            goto try_except_handler_2;
        }
        tmp_assattr_target_36 = LOOKUP_ATTRIBUTE(tmp_expression_name_37, mod_consts[110]);
        if (tmp_assattr_target_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 214;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_36, mod_consts[92], tmp_assattr_name_36);
        Py_DECREF(tmp_assattr_target_36);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 214;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_37;
        PyObject *tmp_list_element_15;
        PyObject *tmp_assattr_target_37;
        PyObject *tmp_expression_name_38;
        tmp_list_element_15 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_15 == NULL)) {
            tmp_list_element_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 216;

            goto try_except_handler_2;
        }
        tmp_assattr_name_37 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_37, 0, tmp_list_element_15);
        tmp_expression_name_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_38 == NULL)) {
            tmp_expression_name_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_37);

            exception_lineno = 216;

            goto try_except_handler_2;
        }
        tmp_assattr_target_37 = LOOKUP_ATTRIBUTE(tmp_expression_name_38, mod_consts[111]);
        if (tmp_assattr_target_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_37);

            exception_lineno = 216;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_37, mod_consts[91], tmp_assattr_name_37);
        Py_DECREF(tmp_assattr_name_37);
        Py_DECREF(tmp_assattr_target_37);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 216;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_38;
        PyObject *tmp_assattr_target_38;
        PyObject *tmp_expression_name_39;
        tmp_assattr_name_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_38 == NULL)) {
            tmp_assattr_name_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 217;

            goto try_except_handler_2;
        }
        tmp_expression_name_39 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_39 == NULL)) {
            tmp_expression_name_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 217;

            goto try_except_handler_2;
        }
        tmp_assattr_target_38 = LOOKUP_ATTRIBUTE(tmp_expression_name_39, mod_consts[111]);
        if (tmp_assattr_target_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 217;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_38, mod_consts[92], tmp_assattr_name_38);
        Py_DECREF(tmp_assattr_target_38);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 217;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_39;
        PyObject *tmp_list_element_16;
        PyObject *tmp_assattr_target_39;
        PyObject *tmp_expression_name_40;
        tmp_list_element_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_16 == NULL)) {
            tmp_list_element_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 219;

            goto try_except_handler_2;
        }
        tmp_assattr_name_39 = PyList_New(4);
        {
            PyObject *tmp_called_name_27;
            PyObject *tmp_args_element_name_32;
            PyList_SET_ITEM0(tmp_assattr_name_39, 0, tmp_list_element_16);
            tmp_list_element_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

            if (unlikely(tmp_list_element_16 == NULL)) {
                tmp_list_element_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
            }

            if (tmp_list_element_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 219;

                goto list_build_exception_13;
            }
            PyList_SET_ITEM0(tmp_assattr_name_39, 1, tmp_list_element_16);
            tmp_list_element_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_list_element_16 == NULL)) {
                tmp_list_element_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_list_element_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 219;

                goto list_build_exception_13;
            }
            PyList_SET_ITEM0(tmp_assattr_name_39, 2, tmp_list_element_16);
            tmp_called_name_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_27 == NULL)) {
                tmp_called_name_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_27 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 219;

                goto list_build_exception_13;
            }
            tmp_args_element_name_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_args_element_name_32 == NULL)) {
                tmp_args_element_name_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_args_element_name_32 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 219;

                goto list_build_exception_13;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 219;
            tmp_list_element_16 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_27, tmp_args_element_name_32);
            if (tmp_list_element_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 219;

                goto list_build_exception_13;
            }
            PyList_SET_ITEM(tmp_assattr_name_39, 3, tmp_list_element_16);
        }
        goto list_build_noexception_13;
        // Exception handling pass through code for list_build:
        list_build_exception_13:;
        Py_DECREF(tmp_assattr_name_39);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_13:;
        tmp_expression_name_40 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_40 == NULL)) {
            tmp_expression_name_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_39);

            exception_lineno = 219;

            goto try_except_handler_2;
        }
        tmp_assattr_target_39 = LOOKUP_ATTRIBUTE(tmp_expression_name_40, mod_consts[112]);
        if (tmp_assattr_target_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_39);

            exception_lineno = 219;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_39, mod_consts[91], tmp_assattr_name_39);
        Py_DECREF(tmp_assattr_name_39);
        Py_DECREF(tmp_assattr_target_39);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 219;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_40;
        PyObject *tmp_assattr_target_40;
        PyObject *tmp_expression_name_41;
        tmp_assattr_name_40 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_40 == NULL)) {
            tmp_assattr_name_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 220;

            goto try_except_handler_2;
        }
        tmp_expression_name_41 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_41 == NULL)) {
            tmp_expression_name_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 220;

            goto try_except_handler_2;
        }
        tmp_assattr_target_40 = LOOKUP_ATTRIBUTE(tmp_expression_name_41, mod_consts[112]);
        if (tmp_assattr_target_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 220;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_40, mod_consts[92], tmp_assattr_name_40);
        Py_DECREF(tmp_assattr_target_40);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 220;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_41;
        PyObject *tmp_list_element_17;
        PyObject *tmp_assattr_target_41;
        PyObject *tmp_expression_name_42;
        tmp_list_element_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_17 == NULL)) {
            tmp_list_element_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 222;

            goto try_except_handler_2;
        }
        tmp_assattr_name_41 = PyList_New(4);
        {
            PyObject *tmp_called_name_28;
            PyObject *tmp_args_element_name_33;
            PyList_SET_ITEM0(tmp_assattr_name_41, 0, tmp_list_element_17);
            tmp_list_element_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

            if (unlikely(tmp_list_element_17 == NULL)) {
                tmp_list_element_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
            }

            if (tmp_list_element_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 222;

                goto list_build_exception_14;
            }
            PyList_SET_ITEM0(tmp_assattr_name_41, 1, tmp_list_element_17);
            tmp_list_element_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_list_element_17 == NULL)) {
                tmp_list_element_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_list_element_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 222;

                goto list_build_exception_14;
            }
            PyList_SET_ITEM0(tmp_assattr_name_41, 2, tmp_list_element_17);
            tmp_called_name_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_28 == NULL)) {
                tmp_called_name_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_28 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 222;

                goto list_build_exception_14;
            }
            tmp_args_element_name_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_args_element_name_33 == NULL)) {
                tmp_args_element_name_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_args_element_name_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 222;

                goto list_build_exception_14;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 222;
            tmp_list_element_17 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_28, tmp_args_element_name_33);
            if (tmp_list_element_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 222;

                goto list_build_exception_14;
            }
            PyList_SET_ITEM(tmp_assattr_name_41, 3, tmp_list_element_17);
        }
        goto list_build_noexception_14;
        // Exception handling pass through code for list_build:
        list_build_exception_14:;
        Py_DECREF(tmp_assattr_name_41);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_14:;
        tmp_expression_name_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_42 == NULL)) {
            tmp_expression_name_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_41);

            exception_lineno = 222;

            goto try_except_handler_2;
        }
        tmp_assattr_target_41 = LOOKUP_ATTRIBUTE(tmp_expression_name_42, mod_consts[113]);
        if (tmp_assattr_target_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_41);

            exception_lineno = 222;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_41, mod_consts[91], tmp_assattr_name_41);
        Py_DECREF(tmp_assattr_name_41);
        Py_DECREF(tmp_assattr_target_41);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 222;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_42;
        PyObject *tmp_assattr_target_42;
        PyObject *tmp_expression_name_43;
        tmp_assattr_name_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_42 == NULL)) {
            tmp_assattr_name_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 223;

            goto try_except_handler_2;
        }
        tmp_expression_name_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_43 == NULL)) {
            tmp_expression_name_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 223;

            goto try_except_handler_2;
        }
        tmp_assattr_target_42 = LOOKUP_ATTRIBUTE(tmp_expression_name_43, mod_consts[113]);
        if (tmp_assattr_target_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 223;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_42, mod_consts[92], tmp_assattr_name_42);
        Py_DECREF(tmp_assattr_target_42);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 223;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_43;
        PyObject *tmp_list_element_18;
        PyObject *tmp_assattr_target_43;
        PyObject *tmp_expression_name_44;
        tmp_list_element_18 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_18 == NULL)) {
            tmp_list_element_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 225;

            goto try_except_handler_2;
        }
        tmp_assattr_name_43 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_43, 0, tmp_list_element_18);
        tmp_expression_name_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_44 == NULL)) {
            tmp_expression_name_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_43);

            exception_lineno = 225;

            goto try_except_handler_2;
        }
        tmp_assattr_target_43 = LOOKUP_ATTRIBUTE(tmp_expression_name_44, mod_consts[114]);
        if (tmp_assattr_target_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_43);

            exception_lineno = 225;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_43, mod_consts[91], tmp_assattr_name_43);
        Py_DECREF(tmp_assattr_name_43);
        Py_DECREF(tmp_assattr_target_43);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 225;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_44;
        PyObject *tmp_assattr_target_44;
        PyObject *tmp_expression_name_45;
        tmp_assattr_name_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_44 == NULL)) {
            tmp_assattr_name_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 226;

            goto try_except_handler_2;
        }
        tmp_expression_name_45 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_45 == NULL)) {
            tmp_expression_name_45 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 226;

            goto try_except_handler_2;
        }
        tmp_assattr_target_44 = LOOKUP_ATTRIBUTE(tmp_expression_name_45, mod_consts[114]);
        if (tmp_assattr_target_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 226;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_44, mod_consts[92], tmp_assattr_name_44);
        Py_DECREF(tmp_assattr_target_44);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 226;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_45;
        PyObject *tmp_list_element_19;
        PyObject *tmp_assattr_target_45;
        PyObject *tmp_expression_name_46;
        tmp_list_element_19 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_19 == NULL)) {
            tmp_list_element_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 228;

            goto try_except_handler_2;
        }
        tmp_assattr_name_45 = PyList_New(2);
        {
            PyObject *tmp_called_name_29;
            PyObject *tmp_args_element_name_34;
            PyList_SET_ITEM0(tmp_assattr_name_45, 0, tmp_list_element_19);
            tmp_called_name_29 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_29 == NULL)) {
                tmp_called_name_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_29 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 228;

                goto list_build_exception_15;
            }
            tmp_args_element_name_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_args_element_name_34 == NULL)) {
                tmp_args_element_name_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_args_element_name_34 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 228;

                goto list_build_exception_15;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 228;
            tmp_list_element_19 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_29, tmp_args_element_name_34);
            if (tmp_list_element_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 228;

                goto list_build_exception_15;
            }
            PyList_SET_ITEM(tmp_assattr_name_45, 1, tmp_list_element_19);
        }
        goto list_build_noexception_15;
        // Exception handling pass through code for list_build:
        list_build_exception_15:;
        Py_DECREF(tmp_assattr_name_45);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_15:;
        tmp_expression_name_46 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_46 == NULL)) {
            tmp_expression_name_46 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_45);

            exception_lineno = 228;

            goto try_except_handler_2;
        }
        tmp_assattr_target_45 = LOOKUP_ATTRIBUTE(tmp_expression_name_46, mod_consts[115]);
        if (tmp_assattr_target_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_45);

            exception_lineno = 228;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_45, mod_consts[91], tmp_assattr_name_45);
        Py_DECREF(tmp_assattr_name_45);
        Py_DECREF(tmp_assattr_target_45);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 228;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_46;
        PyObject *tmp_assattr_target_46;
        PyObject *tmp_expression_name_47;
        tmp_assattr_name_46 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_46 == NULL)) {
            tmp_assattr_name_46 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 229;

            goto try_except_handler_2;
        }
        tmp_expression_name_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_47 == NULL)) {
            tmp_expression_name_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 229;

            goto try_except_handler_2;
        }
        tmp_assattr_target_46 = LOOKUP_ATTRIBUTE(tmp_expression_name_47, mod_consts[115]);
        if (tmp_assattr_target_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 229;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_46, mod_consts[92], tmp_assattr_name_46);
        Py_DECREF(tmp_assattr_target_46);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 229;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_47;
        PyObject *tmp_list_element_20;
        PyObject *tmp_assattr_target_47;
        PyObject *tmp_expression_name_48;
        tmp_list_element_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_20 == NULL)) {
            tmp_list_element_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 232;

            goto try_except_handler_2;
        }
        tmp_assattr_name_47 = PyList_New(3);
        {
            PyObject *tmp_called_name_30;
            PyObject *tmp_args_element_name_35;
            PyObject *tmp_called_name_31;
            PyObject *tmp_args_element_name_36;
            PyList_SET_ITEM0(tmp_assattr_name_47, 0, tmp_list_element_20);
            tmp_called_name_30 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_30 == NULL)) {
                tmp_called_name_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_30 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 233;

                goto list_build_exception_16;
            }
            tmp_args_element_name_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[81]);

            if (unlikely(tmp_args_element_name_35 == NULL)) {
                tmp_args_element_name_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[81]);
            }

            if (tmp_args_element_name_35 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 233;

                goto list_build_exception_16;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 233;
            tmp_list_element_20 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_30, tmp_args_element_name_35);
            if (tmp_list_element_20 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 233;

                goto list_build_exception_16;
            }
            PyList_SET_ITEM(tmp_assattr_name_47, 1, tmp_list_element_20);
            tmp_called_name_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_31 == NULL)) {
                tmp_called_name_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_31 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 234;

                goto list_build_exception_16;
            }
            tmp_args_element_name_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_args_element_name_36 == NULL)) {
                tmp_args_element_name_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_args_element_name_36 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 234;

                goto list_build_exception_16;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 234;
            tmp_list_element_20 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_31, tmp_args_element_name_36);
            if (tmp_list_element_20 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 234;

                goto list_build_exception_16;
            }
            PyList_SET_ITEM(tmp_assattr_name_47, 2, tmp_list_element_20);
        }
        goto list_build_noexception_16;
        // Exception handling pass through code for list_build:
        list_build_exception_16:;
        Py_DECREF(tmp_assattr_name_47);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_16:;
        tmp_expression_name_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_48 == NULL)) {
            tmp_expression_name_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_47);

            exception_lineno = 231;

            goto try_except_handler_2;
        }
        tmp_assattr_target_47 = LOOKUP_ATTRIBUTE(tmp_expression_name_48, mod_consts[116]);
        if (tmp_assattr_target_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_47);

            exception_lineno = 231;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_47, mod_consts[91], tmp_assattr_name_47);
        Py_DECREF(tmp_assattr_name_47);
        Py_DECREF(tmp_assattr_target_47);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 231;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_48;
        PyObject *tmp_assattr_target_48;
        PyObject *tmp_expression_name_49;
        tmp_assattr_name_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_48 == NULL)) {
            tmp_assattr_name_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_2;
        }
        tmp_expression_name_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_49 == NULL)) {
            tmp_expression_name_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_2;
        }
        tmp_assattr_target_48 = LOOKUP_ATTRIBUTE(tmp_expression_name_49, mod_consts[116]);
        if (tmp_assattr_target_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_48, mod_consts[92], tmp_assattr_name_48);
        Py_DECREF(tmp_assattr_target_48);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_49;
        PyObject *tmp_list_element_21;
        PyObject *tmp_assattr_target_49;
        PyObject *tmp_expression_name_50;
        tmp_list_element_21 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_21 == NULL)) {
            tmp_list_element_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 239;

            goto try_except_handler_2;
        }
        tmp_assattr_name_49 = PyList_New(3);
        {
            PyObject *tmp_called_name_32;
            PyObject *tmp_args_element_name_37;
            PyList_SET_ITEM0(tmp_assattr_name_49, 0, tmp_list_element_21);
            tmp_called_name_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_32 == NULL)) {
                tmp_called_name_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_32 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 240;

                goto list_build_exception_17;
            }
            tmp_args_element_name_37 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[81]);

            if (unlikely(tmp_args_element_name_37 == NULL)) {
                tmp_args_element_name_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[81]);
            }

            if (tmp_args_element_name_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 240;

                goto list_build_exception_17;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 240;
            tmp_list_element_21 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_32, tmp_args_element_name_37);
            if (tmp_list_element_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 240;

                goto list_build_exception_17;
            }
            PyList_SET_ITEM(tmp_assattr_name_49, 1, tmp_list_element_21);
            tmp_list_element_21 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_list_element_21 == NULL)) {
                tmp_list_element_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_list_element_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 241;

                goto list_build_exception_17;
            }
            PyList_SET_ITEM0(tmp_assattr_name_49, 2, tmp_list_element_21);
        }
        goto list_build_noexception_17;
        // Exception handling pass through code for list_build:
        list_build_exception_17:;
        Py_DECREF(tmp_assattr_name_49);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_17:;
        tmp_expression_name_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_50 == NULL)) {
            tmp_expression_name_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_49);

            exception_lineno = 238;

            goto try_except_handler_2;
        }
        tmp_assattr_target_49 = LOOKUP_ATTRIBUTE(tmp_expression_name_50, mod_consts[117]);
        if (tmp_assattr_target_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_49);

            exception_lineno = 238;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_49, mod_consts[91], tmp_assattr_name_49);
        Py_DECREF(tmp_assattr_name_49);
        Py_DECREF(tmp_assattr_target_49);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 238;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_50;
        PyObject *tmp_assattr_target_50;
        PyObject *tmp_expression_name_51;
        tmp_assattr_name_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_50 == NULL)) {
            tmp_assattr_name_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 243;

            goto try_except_handler_2;
        }
        tmp_expression_name_51 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_51 == NULL)) {
            tmp_expression_name_51 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 243;

            goto try_except_handler_2;
        }
        tmp_assattr_target_50 = LOOKUP_ATTRIBUTE(tmp_expression_name_51, mod_consts[117]);
        if (tmp_assattr_target_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 243;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_50, mod_consts[92], tmp_assattr_name_50);
        Py_DECREF(tmp_assattr_target_50);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 243;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_51;
        PyObject *tmp_list_element_22;
        PyObject *tmp_assattr_target_51;
        PyObject *tmp_expression_name_52;
        tmp_list_element_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_22 == NULL)) {
            tmp_list_element_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_2;
        }
        tmp_assattr_name_51 = PyList_New(2);
        {
            PyObject *tmp_called_name_33;
            PyObject *tmp_args_element_name_38;
            PyList_SET_ITEM0(tmp_assattr_name_51, 0, tmp_list_element_22);
            tmp_called_name_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_33 == NULL)) {
                tmp_called_name_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 245;

                goto list_build_exception_18;
            }
            tmp_args_element_name_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_args_element_name_38 == NULL)) {
                tmp_args_element_name_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_args_element_name_38 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 245;

                goto list_build_exception_18;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 245;
            tmp_list_element_22 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_33, tmp_args_element_name_38);
            if (tmp_list_element_22 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 245;

                goto list_build_exception_18;
            }
            PyList_SET_ITEM(tmp_assattr_name_51, 1, tmp_list_element_22);
        }
        goto list_build_noexception_18;
        // Exception handling pass through code for list_build:
        list_build_exception_18:;
        Py_DECREF(tmp_assattr_name_51);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_18:;
        tmp_expression_name_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_52 == NULL)) {
            tmp_expression_name_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_51);

            exception_lineno = 245;

            goto try_except_handler_2;
        }
        tmp_assattr_target_51 = LOOKUP_ATTRIBUTE(tmp_expression_name_52, mod_consts[118]);
        if (tmp_assattr_target_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_51);

            exception_lineno = 245;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_51, mod_consts[119], tmp_assattr_name_51);
        Py_DECREF(tmp_assattr_name_51);
        Py_DECREF(tmp_assattr_target_51);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_52;
        PyObject *tmp_assattr_target_52;
        PyObject *tmp_expression_name_53;
        tmp_assattr_name_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_52 == NULL)) {
            tmp_assattr_name_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 246;

            goto try_except_handler_2;
        }
        tmp_expression_name_53 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_53 == NULL)) {
            tmp_expression_name_53 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 246;

            goto try_except_handler_2;
        }
        tmp_assattr_target_52 = LOOKUP_ATTRIBUTE(tmp_expression_name_53, mod_consts[118]);
        if (tmp_assattr_target_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 246;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_52, mod_consts[92], tmp_assattr_name_52);
        Py_DECREF(tmp_assattr_target_52);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 246;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_53;
        PyObject *tmp_list_element_23;
        PyObject *tmp_assattr_target_53;
        PyObject *tmp_expression_name_54;
        tmp_list_element_23 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_23 == NULL)) {
            tmp_list_element_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 249;

            goto try_except_handler_2;
        }
        tmp_assattr_name_53 = PyList_New(3);
        {
            PyObject *tmp_called_name_34;
            PyObject *tmp_args_element_name_39;
            PyObject *tmp_called_name_35;
            PyObject *tmp_args_element_name_40;
            PyList_SET_ITEM0(tmp_assattr_name_53, 0, tmp_list_element_23);
            tmp_called_name_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_34 == NULL)) {
                tmp_called_name_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_34 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 250;

                goto list_build_exception_19;
            }
            tmp_args_element_name_39 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[81]);

            if (unlikely(tmp_args_element_name_39 == NULL)) {
                tmp_args_element_name_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[81]);
            }

            if (tmp_args_element_name_39 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 250;

                goto list_build_exception_19;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 250;
            tmp_list_element_23 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_34, tmp_args_element_name_39);
            if (tmp_list_element_23 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 250;

                goto list_build_exception_19;
            }
            PyList_SET_ITEM(tmp_assattr_name_53, 1, tmp_list_element_23);
            tmp_called_name_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_35 == NULL)) {
                tmp_called_name_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_35 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 251;

                goto list_build_exception_19;
            }
            tmp_args_element_name_40 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[28]);

            if (unlikely(tmp_args_element_name_40 == NULL)) {
                tmp_args_element_name_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[28]);
            }

            if (tmp_args_element_name_40 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 251;

                goto list_build_exception_19;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 251;
            tmp_list_element_23 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_35, tmp_args_element_name_40);
            if (tmp_list_element_23 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 251;

                goto list_build_exception_19;
            }
            PyList_SET_ITEM(tmp_assattr_name_53, 2, tmp_list_element_23);
        }
        goto list_build_noexception_19;
        // Exception handling pass through code for list_build:
        list_build_exception_19:;
        Py_DECREF(tmp_assattr_name_53);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_19:;
        tmp_expression_name_54 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_54 == NULL)) {
            tmp_expression_name_54 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_53);

            exception_lineno = 248;

            goto try_except_handler_2;
        }
        tmp_assattr_target_53 = LOOKUP_ATTRIBUTE(tmp_expression_name_54, mod_consts[120]);
        if (tmp_assattr_target_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_53);

            exception_lineno = 248;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_53, mod_consts[91], tmp_assattr_name_53);
        Py_DECREF(tmp_assattr_name_53);
        Py_DECREF(tmp_assattr_target_53);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 248;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_54;
        PyObject *tmp_assattr_target_54;
        PyObject *tmp_expression_name_55;
        tmp_assattr_name_54 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_54 == NULL)) {
            tmp_assattr_name_54 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 253;

            goto try_except_handler_2;
        }
        tmp_expression_name_55 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_55 == NULL)) {
            tmp_expression_name_55 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 253;

            goto try_except_handler_2;
        }
        tmp_assattr_target_54 = LOOKUP_ATTRIBUTE(tmp_expression_name_55, mod_consts[120]);
        if (tmp_assattr_target_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 253;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_54, mod_consts[92], tmp_assattr_name_54);
        Py_DECREF(tmp_assattr_target_54);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 253;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_55;
        PyObject *tmp_list_element_24;
        PyObject *tmp_assattr_target_55;
        PyObject *tmp_expression_name_56;
        tmp_list_element_24 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_24 == NULL)) {
            tmp_list_element_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 255;

            goto try_except_handler_2;
        }
        tmp_assattr_name_55 = PyList_New(2);
        {
            PyObject *tmp_called_name_36;
            PyObject *tmp_args_element_name_41;
            PyList_SET_ITEM0(tmp_assattr_name_55, 0, tmp_list_element_24);
            tmp_called_name_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_36 == NULL)) {
                tmp_called_name_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_36 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 255;

                goto list_build_exception_20;
            }
            tmp_args_element_name_41 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[81]);

            if (unlikely(tmp_args_element_name_41 == NULL)) {
                tmp_args_element_name_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[81]);
            }

            if (tmp_args_element_name_41 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 255;

                goto list_build_exception_20;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 255;
            tmp_list_element_24 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_36, tmp_args_element_name_41);
            if (tmp_list_element_24 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 255;

                goto list_build_exception_20;
            }
            PyList_SET_ITEM(tmp_assattr_name_55, 1, tmp_list_element_24);
        }
        goto list_build_noexception_20;
        // Exception handling pass through code for list_build:
        list_build_exception_20:;
        Py_DECREF(tmp_assattr_name_55);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_20:;
        tmp_expression_name_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_56 == NULL)) {
            tmp_expression_name_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_55);

            exception_lineno = 255;

            goto try_except_handler_2;
        }
        tmp_assattr_target_55 = LOOKUP_ATTRIBUTE(tmp_expression_name_56, mod_consts[121]);
        if (tmp_assattr_target_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_55);

            exception_lineno = 255;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_55, mod_consts[91], tmp_assattr_name_55);
        Py_DECREF(tmp_assattr_name_55);
        Py_DECREF(tmp_assattr_target_55);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 255;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_56;
        PyObject *tmp_assattr_target_56;
        PyObject *tmp_expression_name_57;
        tmp_assattr_name_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_56 == NULL)) {
            tmp_assattr_name_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 256;

            goto try_except_handler_2;
        }
        tmp_expression_name_57 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_57 == NULL)) {
            tmp_expression_name_57 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 256;

            goto try_except_handler_2;
        }
        tmp_assattr_target_56 = LOOKUP_ATTRIBUTE(tmp_expression_name_57, mod_consts[121]);
        if (tmp_assattr_target_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 256;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_56, mod_consts[92], tmp_assattr_name_56);
        Py_DECREF(tmp_assattr_target_56);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 256;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_57;
        PyObject *tmp_list_element_25;
        PyObject *tmp_assattr_target_57;
        PyObject *tmp_expression_name_58;
        tmp_list_element_25 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_25 == NULL)) {
            tmp_list_element_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 259;

            goto try_except_handler_2;
        }
        tmp_assattr_name_57 = PyList_New(2);
        {
            PyObject *tmp_called_name_37;
            PyObject *tmp_args_element_name_42;
            PyList_SET_ITEM0(tmp_assattr_name_57, 0, tmp_list_element_25);
            tmp_called_name_37 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_37 == NULL)) {
                tmp_called_name_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 260;

                goto list_build_exception_21;
            }
            tmp_args_element_name_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[80]);

            if (unlikely(tmp_args_element_name_42 == NULL)) {
                tmp_args_element_name_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[80]);
            }

            if (tmp_args_element_name_42 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 260;

                goto list_build_exception_21;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 260;
            tmp_list_element_25 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_37, tmp_args_element_name_42);
            if (tmp_list_element_25 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 260;

                goto list_build_exception_21;
            }
            PyList_SET_ITEM(tmp_assattr_name_57, 1, tmp_list_element_25);
        }
        goto list_build_noexception_21;
        // Exception handling pass through code for list_build:
        list_build_exception_21:;
        Py_DECREF(tmp_assattr_name_57);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_21:;
        tmp_expression_name_58 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_58 == NULL)) {
            tmp_expression_name_58 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_57);

            exception_lineno = 258;

            goto try_except_handler_2;
        }
        tmp_assattr_target_57 = LOOKUP_ATTRIBUTE(tmp_expression_name_58, mod_consts[122]);
        if (tmp_assattr_target_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_57);

            exception_lineno = 258;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_57, mod_consts[91], tmp_assattr_name_57);
        Py_DECREF(tmp_assattr_name_57);
        Py_DECREF(tmp_assattr_target_57);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 258;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_58;
        PyObject *tmp_assattr_target_58;
        PyObject *tmp_expression_name_59;
        tmp_assattr_name_58 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_58 == NULL)) {
            tmp_assattr_name_58 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 262;

            goto try_except_handler_2;
        }
        tmp_expression_name_59 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_59 == NULL)) {
            tmp_expression_name_59 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 262;

            goto try_except_handler_2;
        }
        tmp_assattr_target_58 = LOOKUP_ATTRIBUTE(tmp_expression_name_59, mod_consts[122]);
        if (tmp_assattr_target_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 262;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_58, mod_consts[92], tmp_assattr_name_58);
        Py_DECREF(tmp_assattr_target_58);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 262;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_59;
        PyObject *tmp_list_element_26;
        PyObject *tmp_assattr_target_59;
        PyObject *tmp_expression_name_60;
        tmp_list_element_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_26 == NULL)) {
            tmp_list_element_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 264;

            goto try_except_handler_2;
        }
        tmp_assattr_name_59 = PyList_New(2);
        {
            PyObject *tmp_called_name_38;
            PyObject *tmp_args_element_name_43;
            PyList_SET_ITEM0(tmp_assattr_name_59, 0, tmp_list_element_26);
            tmp_called_name_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_38 == NULL)) {
                tmp_called_name_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_38 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 264;

                goto list_build_exception_22;
            }
            tmp_args_element_name_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83]);

            if (unlikely(tmp_args_element_name_43 == NULL)) {
                tmp_args_element_name_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[83]);
            }

            if (tmp_args_element_name_43 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 264;

                goto list_build_exception_22;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 264;
            tmp_list_element_26 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_38, tmp_args_element_name_43);
            if (tmp_list_element_26 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 264;

                goto list_build_exception_22;
            }
            PyList_SET_ITEM(tmp_assattr_name_59, 1, tmp_list_element_26);
        }
        goto list_build_noexception_22;
        // Exception handling pass through code for list_build:
        list_build_exception_22:;
        Py_DECREF(tmp_assattr_name_59);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_22:;
        tmp_expression_name_60 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_60 == NULL)) {
            tmp_expression_name_60 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_59);

            exception_lineno = 264;

            goto try_except_handler_2;
        }
        tmp_assattr_target_59 = LOOKUP_ATTRIBUTE(tmp_expression_name_60, mod_consts[123]);
        if (tmp_assattr_target_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_59);

            exception_lineno = 264;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_59, mod_consts[91], tmp_assattr_name_59);
        Py_DECREF(tmp_assattr_name_59);
        Py_DECREF(tmp_assattr_target_59);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 264;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_60;
        PyObject *tmp_assattr_target_60;
        PyObject *tmp_expression_name_61;
        tmp_assattr_name_60 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_60 == NULL)) {
            tmp_assattr_name_60 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 265;

            goto try_except_handler_2;
        }
        tmp_expression_name_61 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_61 == NULL)) {
            tmp_expression_name_61 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 265;

            goto try_except_handler_2;
        }
        tmp_assattr_target_60 = LOOKUP_ATTRIBUTE(tmp_expression_name_61, mod_consts[123]);
        if (tmp_assattr_target_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 265;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_60, mod_consts[92], tmp_assattr_name_60);
        Py_DECREF(tmp_assattr_target_60);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 265;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_61;
        PyObject *tmp_list_element_27;
        PyObject *tmp_assattr_target_61;
        PyObject *tmp_expression_name_62;
        tmp_list_element_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83]);

        if (unlikely(tmp_list_element_27 == NULL)) {
            tmp_list_element_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[83]);
        }

        if (tmp_list_element_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 267;

            goto try_except_handler_2;
        }
        tmp_assattr_name_61 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_61, 0, tmp_list_element_27);
        tmp_list_element_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

        if (unlikely(tmp_list_element_27 == NULL)) {
            tmp_list_element_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
        }

        if (tmp_list_element_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 267;

            goto list_build_exception_23;
        }
        PyList_SET_ITEM0(tmp_assattr_name_61, 1, tmp_list_element_27);
        goto list_build_noexception_23;
        // Exception handling pass through code for list_build:
        list_build_exception_23:;
        Py_DECREF(tmp_assattr_name_61);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_23:;
        tmp_expression_name_62 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_62 == NULL)) {
            tmp_expression_name_62 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_61);

            exception_lineno = 267;

            goto try_except_handler_2;
        }
        tmp_assattr_target_61 = LOOKUP_ATTRIBUTE(tmp_expression_name_62, mod_consts[124]);
        if (tmp_assattr_target_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_61);

            exception_lineno = 267;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_61, mod_consts[91], tmp_assattr_name_61);
        Py_DECREF(tmp_assattr_name_61);
        Py_DECREF(tmp_assattr_target_61);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 267;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_62;
        PyObject *tmp_assattr_target_62;
        PyObject *tmp_expression_name_63;
        tmp_assattr_name_62 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_62 == NULL)) {
            tmp_assattr_name_62 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 268;

            goto try_except_handler_2;
        }
        tmp_expression_name_63 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_63 == NULL)) {
            tmp_expression_name_63 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 268;

            goto try_except_handler_2;
        }
        tmp_assattr_target_62 = LOOKUP_ATTRIBUTE(tmp_expression_name_63, mod_consts[124]);
        if (tmp_assattr_target_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 268;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_62, mod_consts[92], tmp_assattr_name_62);
        Py_DECREF(tmp_assattr_target_62);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 268;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_63;
        PyObject *tmp_list_element_28;
        PyObject *tmp_assattr_target_63;
        PyObject *tmp_expression_name_64;
        tmp_list_element_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83]);

        if (unlikely(tmp_list_element_28 == NULL)) {
            tmp_list_element_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[83]);
        }

        if (tmp_list_element_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 270;

            goto try_except_handler_2;
        }
        tmp_assattr_name_63 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_63, 0, tmp_list_element_28);
        tmp_list_element_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[51]);

        if (unlikely(tmp_list_element_28 == NULL)) {
            tmp_list_element_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[51]);
        }

        if (tmp_list_element_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 270;

            goto list_build_exception_24;
        }
        PyList_SET_ITEM0(tmp_assattr_name_63, 1, tmp_list_element_28);
        goto list_build_noexception_24;
        // Exception handling pass through code for list_build:
        list_build_exception_24:;
        Py_DECREF(tmp_assattr_name_63);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_24:;
        tmp_expression_name_64 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_64 == NULL)) {
            tmp_expression_name_64 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_63);

            exception_lineno = 270;

            goto try_except_handler_2;
        }
        tmp_assattr_target_63 = LOOKUP_ATTRIBUTE(tmp_expression_name_64, mod_consts[125]);
        if (tmp_assattr_target_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_63);

            exception_lineno = 270;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_63, mod_consts[126], tmp_assattr_name_63);
        Py_DECREF(tmp_assattr_name_63);
        Py_DECREF(tmp_assattr_target_63);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 270;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_64;
        PyObject *tmp_assattr_target_64;
        PyObject *tmp_expression_name_65;
        tmp_assattr_name_64 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_64 == NULL)) {
            tmp_assattr_name_64 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 271;

            goto try_except_handler_2;
        }
        tmp_expression_name_65 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_65 == NULL)) {
            tmp_expression_name_65 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 271;

            goto try_except_handler_2;
        }
        tmp_assattr_target_64 = LOOKUP_ATTRIBUTE(tmp_expression_name_65, mod_consts[125]);
        if (tmp_assattr_target_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 271;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_64, mod_consts[92], tmp_assattr_name_64);
        Py_DECREF(tmp_assattr_target_64);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 271;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_65;
        PyObject *tmp_list_element_29;
        PyObject *tmp_assattr_target_65;
        PyObject *tmp_expression_name_66;
        tmp_list_element_29 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83]);

        if (unlikely(tmp_list_element_29 == NULL)) {
            tmp_list_element_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[83]);
        }

        if (tmp_list_element_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 273;

            goto try_except_handler_2;
        }
        tmp_assattr_name_65 = PyList_New(2);
        {
            PyObject *tmp_called_name_39;
            PyObject *tmp_args_element_name_44;
            PyList_SET_ITEM0(tmp_assattr_name_65, 0, tmp_list_element_29);
            tmp_called_name_39 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_39 == NULL)) {
                tmp_called_name_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_39 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 273;

                goto list_build_exception_25;
            }
            tmp_args_element_name_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[85]);

            if (unlikely(tmp_args_element_name_44 == NULL)) {
                tmp_args_element_name_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[85]);
            }

            if (tmp_args_element_name_44 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 273;

                goto list_build_exception_25;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 273;
            tmp_list_element_29 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_39, tmp_args_element_name_44);
            if (tmp_list_element_29 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 273;

                goto list_build_exception_25;
            }
            PyList_SET_ITEM(tmp_assattr_name_65, 1, tmp_list_element_29);
        }
        goto list_build_noexception_25;
        // Exception handling pass through code for list_build:
        list_build_exception_25:;
        Py_DECREF(tmp_assattr_name_65);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_25:;
        tmp_expression_name_66 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_66 == NULL)) {
            tmp_expression_name_66 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_65);

            exception_lineno = 273;

            goto try_except_handler_2;
        }
        tmp_assattr_target_65 = LOOKUP_ATTRIBUTE(tmp_expression_name_66, mod_consts[127]);
        if (tmp_assattr_target_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_65);

            exception_lineno = 273;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_65, mod_consts[91], tmp_assattr_name_65);
        Py_DECREF(tmp_assattr_name_65);
        Py_DECREF(tmp_assattr_target_65);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 273;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_66;
        PyObject *tmp_assattr_target_66;
        PyObject *tmp_expression_name_67;
        tmp_assattr_name_66 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_66 == NULL)) {
            tmp_assattr_name_66 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 274;

            goto try_except_handler_2;
        }
        tmp_expression_name_67 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_67 == NULL)) {
            tmp_expression_name_67 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 274;

            goto try_except_handler_2;
        }
        tmp_assattr_target_66 = LOOKUP_ATTRIBUTE(tmp_expression_name_67, mod_consts[127]);
        if (tmp_assattr_target_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 274;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_66, mod_consts[92], tmp_assattr_name_66);
        Py_DECREF(tmp_assattr_target_66);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 274;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_67;
        PyObject *tmp_list_element_30;
        PyObject *tmp_assattr_target_67;
        PyObject *tmp_expression_name_68;
        tmp_list_element_30 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83]);

        if (unlikely(tmp_list_element_30 == NULL)) {
            tmp_list_element_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[83]);
        }

        if (tmp_list_element_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 276;

            goto try_except_handler_2;
        }
        tmp_assattr_name_67 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_67, 0, tmp_list_element_30);
        tmp_expression_name_68 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_68 == NULL)) {
            tmp_expression_name_68 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_68 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_67);

            exception_lineno = 276;

            goto try_except_handler_2;
        }
        tmp_assattr_target_67 = LOOKUP_ATTRIBUTE(tmp_expression_name_68, mod_consts[128]);
        if (tmp_assattr_target_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_67);

            exception_lineno = 276;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_67, mod_consts[91], tmp_assattr_name_67);
        Py_DECREF(tmp_assattr_name_67);
        Py_DECREF(tmp_assattr_target_67);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 276;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_68;
        PyObject *tmp_assattr_target_68;
        PyObject *tmp_expression_name_69;
        tmp_assattr_name_68 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_assattr_name_68 == NULL)) {
            tmp_assattr_name_68 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_assattr_name_68 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 277;

            goto try_except_handler_2;
        }
        tmp_expression_name_69 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_69 == NULL)) {
            tmp_expression_name_69 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 277;

            goto try_except_handler_2;
        }
        tmp_assattr_target_68 = LOOKUP_ATTRIBUTE(tmp_expression_name_69, mod_consts[128]);
        if (tmp_assattr_target_68 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 277;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_68, mod_consts[92], tmp_assattr_name_68);
        Py_DECREF(tmp_assattr_target_68);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 277;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_69;
        PyObject *tmp_list_element_31;
        PyObject *tmp_assattr_target_69;
        PyObject *tmp_expression_name_70;
        tmp_list_element_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83]);

        if (unlikely(tmp_list_element_31 == NULL)) {
            tmp_list_element_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[83]);
        }

        if (tmp_list_element_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 279;

            goto try_except_handler_2;
        }
        tmp_assattr_name_69 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_69, 0, tmp_list_element_31);
        tmp_list_element_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_list_element_31 == NULL)) {
            tmp_list_element_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_list_element_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 279;

            goto list_build_exception_26;
        }
        PyList_SET_ITEM0(tmp_assattr_name_69, 1, tmp_list_element_31);
        goto list_build_noexception_26;
        // Exception handling pass through code for list_build:
        list_build_exception_26:;
        Py_DECREF(tmp_assattr_name_69);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_26:;
        tmp_expression_name_70 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_70 == NULL)) {
            tmp_expression_name_70 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_69);

            exception_lineno = 279;

            goto try_except_handler_2;
        }
        tmp_assattr_target_69 = LOOKUP_ATTRIBUTE(tmp_expression_name_70, mod_consts[129]);
        if (tmp_assattr_target_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_69);

            exception_lineno = 279;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_69, mod_consts[91], tmp_assattr_name_69);
        Py_DECREF(tmp_assattr_name_69);
        Py_DECREF(tmp_assattr_target_69);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 279;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_70;
        PyObject *tmp_assattr_target_70;
        PyObject *tmp_expression_name_71;
        tmp_assattr_name_70 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[73]);

        if (unlikely(tmp_assattr_name_70 == NULL)) {
            tmp_assattr_name_70 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[73]);
        }

        if (tmp_assattr_name_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_2;
        }
        tmp_expression_name_71 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_71 == NULL)) {
            tmp_expression_name_71 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_71 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_2;
        }
        tmp_assattr_target_70 = LOOKUP_ATTRIBUTE(tmp_expression_name_71, mod_consts[129]);
        if (tmp_assattr_target_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_70, mod_consts[92], tmp_assattr_name_70);
        Py_DECREF(tmp_assattr_target_70);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_71;
        PyObject *tmp_list_element_32;
        PyObject *tmp_assattr_target_71;
        PyObject *tmp_expression_name_72;
        tmp_list_element_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_list_element_32 == NULL)) {
            tmp_list_element_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_list_element_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 283;

            goto try_except_handler_2;
        }
        tmp_assattr_name_71 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_71, 0, tmp_list_element_32);
        tmp_list_element_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[87]);

        if (unlikely(tmp_list_element_32 == NULL)) {
            tmp_list_element_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[87]);
        }

        if (tmp_list_element_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 284;

            goto list_build_exception_27;
        }
        PyList_SET_ITEM0(tmp_assattr_name_71, 1, tmp_list_element_32);
        tmp_list_element_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[88]);

        if (unlikely(tmp_list_element_32 == NULL)) {
            tmp_list_element_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[88]);
        }

        if (tmp_list_element_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 285;

            goto list_build_exception_27;
        }
        PyList_SET_ITEM0(tmp_assattr_name_71, 2, tmp_list_element_32);
        goto list_build_noexception_27;
        // Exception handling pass through code for list_build:
        list_build_exception_27:;
        Py_DECREF(tmp_assattr_name_71);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_27:;
        tmp_expression_name_72 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_72 == NULL)) {
            tmp_expression_name_72 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_72 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_71);

            exception_lineno = 282;

            goto try_except_handler_2;
        }
        tmp_assattr_target_71 = LOOKUP_ATTRIBUTE(tmp_expression_name_72, mod_consts[130]);
        if (tmp_assattr_target_71 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_71);

            exception_lineno = 282;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_71, mod_consts[91], tmp_assattr_name_71);
        Py_DECREF(tmp_assattr_name_71);
        Py_DECREF(tmp_assattr_target_71);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 282;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_72;
        PyObject *tmp_assattr_target_72;
        PyObject *tmp_expression_name_73;
        tmp_assattr_name_72 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_assattr_name_72 == NULL)) {
            tmp_assattr_name_72 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_assattr_name_72 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 287;

            goto try_except_handler_2;
        }
        tmp_expression_name_73 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_73 == NULL)) {
            tmp_expression_name_73 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_73 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 287;

            goto try_except_handler_2;
        }
        tmp_assattr_target_72 = LOOKUP_ATTRIBUTE(tmp_expression_name_73, mod_consts[130]);
        if (tmp_assattr_target_72 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 287;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_72, mod_consts[92], tmp_assattr_name_72);
        Py_DECREF(tmp_assattr_target_72);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 287;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_73;
        PyObject *tmp_list_element_33;
        PyObject *tmp_assattr_target_73;
        PyObject *tmp_expression_name_74;
        tmp_list_element_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_33 == NULL)) {
            tmp_list_element_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 289;

            goto try_except_handler_2;
        }
        tmp_assattr_name_73 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_73, 0, tmp_list_element_33);
        tmp_list_element_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[89]);

        if (unlikely(tmp_list_element_33 == NULL)) {
            tmp_list_element_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[89]);
        }

        if (tmp_list_element_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 289;

            goto list_build_exception_28;
        }
        PyList_SET_ITEM0(tmp_assattr_name_73, 1, tmp_list_element_33);
        tmp_list_element_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[51]);

        if (unlikely(tmp_list_element_33 == NULL)) {
            tmp_list_element_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[51]);
        }

        if (tmp_list_element_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 289;

            goto list_build_exception_28;
        }
        PyList_SET_ITEM0(tmp_assattr_name_73, 2, tmp_list_element_33);
        goto list_build_noexception_28;
        // Exception handling pass through code for list_build:
        list_build_exception_28:;
        Py_DECREF(tmp_assattr_name_73);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_28:;
        tmp_expression_name_74 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_74 == NULL)) {
            tmp_expression_name_74 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_74 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_73);

            exception_lineno = 289;

            goto try_except_handler_2;
        }
        tmp_assattr_target_73 = LOOKUP_ATTRIBUTE(tmp_expression_name_74, mod_consts[131]);
        if (tmp_assattr_target_73 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_73);

            exception_lineno = 289;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_73, mod_consts[91], tmp_assattr_name_73);
        Py_DECREF(tmp_assattr_name_73);
        Py_DECREF(tmp_assattr_target_73);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 289;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_74;
        PyObject *tmp_assattr_target_74;
        PyObject *tmp_expression_name_75;
        tmp_assattr_name_74 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_74 == NULL)) {
            tmp_assattr_name_74 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_74 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 290;

            goto try_except_handler_2;
        }
        tmp_expression_name_75 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_75 == NULL)) {
            tmp_expression_name_75 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_75 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 290;

            goto try_except_handler_2;
        }
        tmp_assattr_target_74 = LOOKUP_ATTRIBUTE(tmp_expression_name_75, mod_consts[131]);
        if (tmp_assattr_target_74 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 290;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_74, mod_consts[92], tmp_assattr_name_74);
        Py_DECREF(tmp_assattr_target_74);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 290;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_75;
        PyObject *tmp_list_element_34;
        PyObject *tmp_assattr_target_75;
        PyObject *tmp_expression_name_76;
        tmp_list_element_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_34 == NULL)) {
            tmp_list_element_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 292;

            goto try_except_handler_2;
        }
        tmp_assattr_name_75 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_75, 0, tmp_list_element_34);
        tmp_list_element_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[80]);

        if (unlikely(tmp_list_element_34 == NULL)) {
            tmp_list_element_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[80]);
        }

        if (tmp_list_element_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 292;

            goto list_build_exception_29;
        }
        PyList_SET_ITEM0(tmp_assattr_name_75, 1, tmp_list_element_34);
        goto list_build_noexception_29;
        // Exception handling pass through code for list_build:
        list_build_exception_29:;
        Py_DECREF(tmp_assattr_name_75);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_29:;
        tmp_expression_name_76 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_76 == NULL)) {
            tmp_expression_name_76 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_75);

            exception_lineno = 292;

            goto try_except_handler_2;
        }
        tmp_assattr_target_75 = LOOKUP_ATTRIBUTE(tmp_expression_name_76, mod_consts[132]);
        if (tmp_assattr_target_75 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_75);

            exception_lineno = 292;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_75, mod_consts[91], tmp_assattr_name_75);
        Py_DECREF(tmp_assattr_name_75);
        Py_DECREF(tmp_assattr_target_75);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 292;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_76;
        PyObject *tmp_assattr_target_76;
        PyObject *tmp_expression_name_77;
        tmp_assattr_name_76 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_76 == NULL)) {
            tmp_assattr_name_76 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 293;

            goto try_except_handler_2;
        }
        tmp_expression_name_77 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_77 == NULL)) {
            tmp_expression_name_77 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_77 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 293;

            goto try_except_handler_2;
        }
        tmp_assattr_target_76 = LOOKUP_ATTRIBUTE(tmp_expression_name_77, mod_consts[132]);
        if (tmp_assattr_target_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 293;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_76, mod_consts[92], tmp_assattr_name_76);
        Py_DECREF(tmp_assattr_target_76);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 293;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_77;
        PyObject *tmp_list_element_35;
        PyObject *tmp_assattr_target_77;
        PyObject *tmp_expression_name_78;
        tmp_list_element_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_35 == NULL)) {
            tmp_list_element_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 295;

            goto try_except_handler_2;
        }
        tmp_assattr_name_77 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_77, 0, tmp_list_element_35);
        tmp_list_element_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[80]);

        if (unlikely(tmp_list_element_35 == NULL)) {
            tmp_list_element_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[80]);
        }

        if (tmp_list_element_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 295;

            goto list_build_exception_30;
        }
        PyList_SET_ITEM0(tmp_assattr_name_77, 1, tmp_list_element_35);
        goto list_build_noexception_30;
        // Exception handling pass through code for list_build:
        list_build_exception_30:;
        Py_DECREF(tmp_assattr_name_77);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_30:;
        tmp_expression_name_78 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_78 == NULL)) {
            tmp_expression_name_78 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_78 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_77);

            exception_lineno = 295;

            goto try_except_handler_2;
        }
        tmp_assattr_target_77 = LOOKUP_ATTRIBUTE(tmp_expression_name_78, mod_consts[133]);
        if (tmp_assattr_target_77 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_77);

            exception_lineno = 295;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_77, mod_consts[91], tmp_assattr_name_77);
        Py_DECREF(tmp_assattr_name_77);
        Py_DECREF(tmp_assattr_target_77);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 295;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_78;
        PyObject *tmp_assattr_target_78;
        PyObject *tmp_expression_name_79;
        tmp_assattr_name_78 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_78 == NULL)) {
            tmp_assattr_name_78 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_78 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_2;
        }
        tmp_expression_name_79 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_79 == NULL)) {
            tmp_expression_name_79 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_79 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_2;
        }
        tmp_assattr_target_78 = LOOKUP_ATTRIBUTE(tmp_expression_name_79, mod_consts[133]);
        if (tmp_assattr_target_78 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_78, mod_consts[92], tmp_assattr_name_78);
        Py_DECREF(tmp_assattr_target_78);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_2;
        }
    }
    // Tried code:
    {
        PyObject *tmp_assattr_name_79;
        PyObject *tmp_list_element_36;
        PyObject *tmp_assattr_target_79;
        PyObject *tmp_expression_name_80;
        tmp_list_element_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_list_element_36 == NULL)) {
            tmp_list_element_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_list_element_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 299;

            goto try_except_handler_3;
        }
        tmp_assattr_name_79 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_79, 0, tmp_list_element_36);
        tmp_list_element_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

        if (unlikely(tmp_list_element_36 == NULL)) {
            tmp_list_element_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
        }

        if (tmp_list_element_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 299;

            goto list_build_exception_31;
        }
        PyList_SET_ITEM0(tmp_assattr_name_79, 1, tmp_list_element_36);
        goto list_build_noexception_31;
        // Exception handling pass through code for list_build:
        list_build_exception_31:;
        Py_DECREF(tmp_assattr_name_79);
        goto try_except_handler_3;
        // Finished with no exception for list_build:
        list_build_noexception_31:;
        tmp_expression_name_80 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_80 == NULL)) {
            tmp_expression_name_80 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_80 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_79);

            exception_lineno = 299;

            goto try_except_handler_3;
        }
        tmp_assattr_target_79 = LOOKUP_ATTRIBUTE(tmp_expression_name_80, mod_consts[134]);
        if (tmp_assattr_target_79 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_79);

            exception_lineno = 299;

            goto try_except_handler_3;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_79, mod_consts[91], tmp_assattr_name_79);
        Py_DECREF(tmp_assattr_name_79);
        Py_DECREF(tmp_assattr_target_79);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 299;

            goto try_except_handler_3;
        }
    }
    {
        PyObject *tmp_assattr_name_80;
        PyObject *tmp_assattr_target_80;
        PyObject *tmp_expression_name_81;
        tmp_assattr_name_80 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_80 == NULL)) {
            tmp_assattr_name_80 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_80 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 300;

            goto try_except_handler_3;
        }
        tmp_expression_name_81 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_81 == NULL)) {
            tmp_expression_name_81 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_81 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 300;

            goto try_except_handler_3;
        }
        tmp_assattr_target_80 = LOOKUP_ATTRIBUTE(tmp_expression_name_81, mod_consts[134]);
        if (tmp_assattr_target_80 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 300;

            goto try_except_handler_3;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_80, mod_consts[92], tmp_assattr_name_80);
        Py_DECREF(tmp_assattr_target_80);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 300;

            goto try_except_handler_3;
        }
    }
    goto try_end_2;
    // Exception handler code:
    try_except_handler_3:;
    exception_keeper_type_2 = exception_type;
    exception_keeper_value_2 = exception_value;
    exception_keeper_tb_2 = exception_tb;
    exception_keeper_lineno_2 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    // Preserve existing published exception id 1.
    GET_CURRENT_EXCEPTION(&exception_preserved_type_1, &exception_preserved_value_1, &exception_preserved_tb_1);

    if (exception_keeper_tb_2 == NULL) {
        exception_keeper_tb_2 = MAKE_TRACEBACK(frame_1d818adf931272325ef94f3fb8c15fe1, exception_keeper_lineno_2);
    } else if (exception_keeper_lineno_2 != 0) {
        exception_keeper_tb_2 = ADD_TRACEBACK(exception_keeper_tb_2, frame_1d818adf931272325ef94f3fb8c15fe1, exception_keeper_lineno_2);
    }

    NORMALIZE_EXCEPTION(&exception_keeper_type_2, &exception_keeper_value_2, &exception_keeper_tb_2);
    ATTACH_TRACEBACK_TO_EXCEPTION_VALUE(exception_keeper_value_2, exception_keeper_tb_2);
    PUBLISH_EXCEPTION(&exception_keeper_type_2, &exception_keeper_value_2, &exception_keeper_tb_2);
    // Tried code:
    {
        bool tmp_condition_result_3;
        PyObject *tmp_compexpr_left_3;
        PyObject *tmp_compexpr_right_3;
        tmp_compexpr_left_3 = EXC_TYPE(PyThreadState_GET());
        tmp_compexpr_right_3 = PyExc_AttributeError;
        tmp_res = EXCEPTION_MATCH_BOOL(tmp_compexpr_left_3, tmp_compexpr_right_3);
        assert(!(tmp_res == -1));
        tmp_condition_result_3 = (tmp_res == 0) ? true : false;
        if (tmp_condition_result_3 != false) {
            goto branch_yes_3;
        } else {
            goto branch_no_3;
        }
    }
    branch_yes_3:;
    tmp_result = RERAISE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
    if (unlikely(tmp_result == false)) {
        exception_lineno = 298;
    }

    if (exception_tb && exception_tb->tb_frame == &frame_1d818adf931272325ef94f3fb8c15fe1->m_frame) frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = exception_tb->tb_lineno;

    goto try_except_handler_4;
    branch_no_3:;
    goto try_end_3;
    // Exception handler code:
    try_except_handler_4:;
    exception_keeper_type_3 = exception_type;
    exception_keeper_value_3 = exception_value;
    exception_keeper_tb_3 = exception_tb;
    exception_keeper_lineno_3 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    // Restore previous exception id 1.
    SET_CURRENT_EXCEPTION(exception_preserved_type_1, exception_preserved_value_1, exception_preserved_tb_1);

    // Re-raise.
    exception_type = exception_keeper_type_3;
    exception_value = exception_keeper_value_3;
    exception_tb = exception_keeper_tb_3;
    exception_lineno = exception_keeper_lineno_3;

    goto try_except_handler_2;
    // End of try:
    try_end_3:;
    // Restore previous exception id 1.
    SET_CURRENT_EXCEPTION(exception_preserved_type_1, exception_preserved_value_1, exception_preserved_tb_1);

    goto try_end_2;
    NUITKA_CANNOT_GET_HERE("exception handler codes exits in all cases");
    return NULL;
    // End of try:
    try_end_2:;
    {
        PyObject *tmp_assattr_name_81;
        PyObject *tmp_list_element_37;
        PyObject *tmp_assattr_target_81;
        PyObject *tmp_expression_name_82;
        tmp_list_element_37 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_list_element_37 == NULL)) {
            tmp_list_element_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_list_element_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 305;

            goto try_except_handler_2;
        }
        tmp_assattr_name_81 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_81, 0, tmp_list_element_37);
        tmp_list_element_37 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_list_element_37 == NULL)) {
            tmp_list_element_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_list_element_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 305;

            goto list_build_exception_32;
        }
        PyList_SET_ITEM0(tmp_assattr_name_81, 1, tmp_list_element_37);
        goto list_build_noexception_32;
        // Exception handling pass through code for list_build:
        list_build_exception_32:;
        Py_DECREF(tmp_assattr_name_81);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_32:;
        tmp_expression_name_82 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_82 == NULL)) {
            tmp_expression_name_82 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_82 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_81);

            exception_lineno = 305;

            goto try_except_handler_2;
        }
        tmp_assattr_target_81 = LOOKUP_ATTRIBUTE(tmp_expression_name_82, mod_consts[98]);
        if (tmp_assattr_target_81 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_81);

            exception_lineno = 305;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_81, mod_consts[91], tmp_assattr_name_81);
        Py_DECREF(tmp_assattr_name_81);
        Py_DECREF(tmp_assattr_target_81);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 305;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_82;
        PyObject *tmp_assattr_target_82;
        PyObject *tmp_expression_name_83;
        tmp_assattr_name_82 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_assattr_name_82 == NULL)) {
            tmp_assattr_name_82 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_assattr_name_82 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 306;

            goto try_except_handler_2;
        }
        tmp_expression_name_83 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_83 == NULL)) {
            tmp_expression_name_83 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_83 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 306;

            goto try_except_handler_2;
        }
        tmp_assattr_target_82 = LOOKUP_ATTRIBUTE(tmp_expression_name_83, mod_consts[98]);
        if (tmp_assattr_target_82 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 306;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_82, mod_consts[92], tmp_assattr_name_82);
        Py_DECREF(tmp_assattr_target_82);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 306;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_83;
        PyObject *tmp_assattr_target_83;
        tmp_assattr_name_83 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[103]);

        if (unlikely(tmp_assattr_name_83 == NULL)) {
            tmp_assattr_name_83 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[103]);
        }

        if (tmp_assattr_name_83 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 308;

            goto try_except_handler_2;
        }
        tmp_assattr_target_83 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_83 == NULL)) {
            tmp_assattr_target_83 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_83 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 308;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_83, mod_consts[103], tmp_assattr_name_83);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 308;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_84;
        PyObject *tmp_assattr_target_84;
        tmp_assattr_name_84 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[104]);

        if (unlikely(tmp_assattr_name_84 == NULL)) {
            tmp_assattr_name_84 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[104]);
        }

        if (tmp_assattr_name_84 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 309;

            goto try_except_handler_2;
        }
        tmp_assattr_target_84 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_84 == NULL)) {
            tmp_assattr_target_84 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_84 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 309;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_84, mod_consts[104], tmp_assattr_name_84);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 309;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_85;
        PyObject *tmp_assattr_target_85;
        tmp_assattr_name_85 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[82]);

        if (unlikely(tmp_assattr_name_85 == NULL)) {
            tmp_assattr_name_85 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[82]);
        }

        if (tmp_assattr_name_85 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_2;
        }
        tmp_assattr_target_85 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_85 == NULL)) {
            tmp_assattr_target_85 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_85 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_85, mod_consts[82], tmp_assattr_name_85);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_86;
        PyObject *tmp_assattr_target_86;
        tmp_assattr_name_86 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[80]);

        if (unlikely(tmp_assattr_name_86 == NULL)) {
            tmp_assattr_name_86 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[80]);
        }

        if (tmp_assattr_name_86 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 311;

            goto try_except_handler_2;
        }
        tmp_assattr_target_86 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_86 == NULL)) {
            tmp_assattr_target_86 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_86 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 311;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_86, mod_consts[80], tmp_assattr_name_86);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 311;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_87;
        PyObject *tmp_assattr_target_87;
        tmp_assattr_name_87 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[81]);

        if (unlikely(tmp_assattr_name_87 == NULL)) {
            tmp_assattr_name_87 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[81]);
        }

        if (tmp_assattr_name_87 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 312;

            goto try_except_handler_2;
        }
        tmp_assattr_target_87 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_87 == NULL)) {
            tmp_assattr_target_87 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_87 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 312;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_87, mod_consts[81], tmp_assattr_name_87);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 312;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_88;
        PyObject *tmp_assattr_target_88;
        tmp_assattr_name_88 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[76]);

        if (unlikely(tmp_assattr_name_88 == NULL)) {
            tmp_assattr_name_88 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[76]);
        }

        if (tmp_assattr_name_88 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 313;

            goto try_except_handler_2;
        }
        tmp_assattr_target_88 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_88 == NULL)) {
            tmp_assattr_target_88 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_88 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 313;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_88, mod_consts[76], tmp_assattr_name_88);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 313;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_89;
        PyObject *tmp_assattr_target_89;
        tmp_assattr_name_89 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[79]);

        if (unlikely(tmp_assattr_name_89 == NULL)) {
            tmp_assattr_name_89 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[79]);
        }

        if (tmp_assattr_name_89 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 314;

            goto try_except_handler_2;
        }
        tmp_assattr_target_89 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_89 == NULL)) {
            tmp_assattr_target_89 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_89 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 314;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_89, mod_consts[79], tmp_assattr_name_89);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 314;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_90;
        PyObject *tmp_assattr_target_90;
        tmp_assattr_name_90 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[83]);

        if (unlikely(tmp_assattr_name_90 == NULL)) {
            tmp_assattr_name_90 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[83]);
        }

        if (tmp_assattr_name_90 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 315;

            goto try_except_handler_2;
        }
        tmp_assattr_target_90 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_90 == NULL)) {
            tmp_assattr_target_90 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_90 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 315;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_90, mod_consts[83], tmp_assattr_name_90);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 315;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_91;
        PyObject *tmp_assattr_target_91;
        tmp_assattr_name_91 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[85]);

        if (unlikely(tmp_assattr_name_91 == NULL)) {
            tmp_assattr_name_91 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[85]);
        }

        if (tmp_assattr_name_91 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 316;

            goto try_except_handler_2;
        }
        tmp_assattr_target_91 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_91 == NULL)) {
            tmp_assattr_target_91 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_91 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 316;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_91, mod_consts[85], tmp_assattr_name_91);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 316;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_92;
        PyObject *tmp_assattr_target_92;
        tmp_assattr_name_92 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[74]);

        if (unlikely(tmp_assattr_name_92 == NULL)) {
            tmp_assattr_name_92 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[74]);
        }

        if (tmp_assattr_name_92 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 317;

            goto try_except_handler_2;
        }
        tmp_assattr_target_92 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_92 == NULL)) {
            tmp_assattr_target_92 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_92 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 317;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_92, mod_consts[74], tmp_assattr_name_92);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 317;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_93;
        PyObject *tmp_assattr_target_93;
        tmp_assattr_name_93 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[64]);

        if (unlikely(tmp_assattr_name_93 == NULL)) {
            tmp_assattr_name_93 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[64]);
        }

        if (tmp_assattr_name_93 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 318;

            goto try_except_handler_2;
        }
        tmp_assattr_target_93 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_93 == NULL)) {
            tmp_assattr_target_93 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_93 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 318;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_93, mod_consts[64], tmp_assattr_name_93);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 318;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_94;
        PyObject *tmp_called_name_40;
        PyObject *tmp_expression_name_84;
        PyObject *tmp_args_element_name_45;
        PyObject *tmp_args_element_name_46;
        PyObject *tmp_assattr_target_94;
        tmp_expression_name_84 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_expression_name_84 == NULL)) {
            tmp_expression_name_84 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_expression_name_84 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 320;

            goto try_except_handler_2;
        }
        tmp_called_name_40 = LOOKUP_ATTRIBUTE(tmp_expression_name_84, mod_consts[135]);
        if (tmp_called_name_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 320;

            goto try_except_handler_2;
        }
        tmp_args_element_name_45 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_args_element_name_45 == NULL)) {
            tmp_args_element_name_45 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_args_element_name_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_40);

            exception_lineno = 321;

            goto try_except_handler_2;
        }
        tmp_args_element_name_46 = mod_consts[136];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 320;
        {
            PyObject *call_args[] = {tmp_args_element_name_45, tmp_args_element_name_46};
            tmp_assattr_name_94 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_40, call_args);
        }

        Py_DECREF(tmp_called_name_40);
        if (tmp_assattr_name_94 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 320;

            goto try_except_handler_2;
        }
        tmp_assattr_target_94 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_94 == NULL)) {
            tmp_assattr_target_94 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_94 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_94);

            exception_lineno = 320;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_94, mod_consts[136], tmp_assattr_name_94);
        Py_DECREF(tmp_assattr_name_94);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 320;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_95;
        PyObject *tmp_called_name_41;
        PyObject *tmp_expression_name_85;
        PyObject *tmp_args_element_name_47;
        PyObject *tmp_args_element_name_48;
        PyObject *tmp_assattr_target_95;
        tmp_expression_name_85 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_expression_name_85 == NULL)) {
            tmp_expression_name_85 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_expression_name_85 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 323;

            goto try_except_handler_2;
        }
        tmp_called_name_41 = LOOKUP_ATTRIBUTE(tmp_expression_name_85, mod_consts[135]);
        if (tmp_called_name_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 323;

            goto try_except_handler_2;
        }
        tmp_args_element_name_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_args_element_name_47 == NULL)) {
            tmp_args_element_name_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_args_element_name_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_41);

            exception_lineno = 324;

            goto try_except_handler_2;
        }
        tmp_args_element_name_48 = mod_consts[137];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 323;
        {
            PyObject *call_args[] = {tmp_args_element_name_47, tmp_args_element_name_48};
            tmp_assattr_name_95 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_41, call_args);
        }

        Py_DECREF(tmp_called_name_41);
        if (tmp_assattr_name_95 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 323;

            goto try_except_handler_2;
        }
        tmp_assattr_target_95 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_assattr_target_95 == NULL)) {
            tmp_assattr_target_95 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_assattr_target_95 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_95);

            exception_lineno = 323;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_95, mod_consts[137], tmp_assattr_name_95);
        Py_DECREF(tmp_assattr_name_95);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 323;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_96;
        PyObject *tmp_list_element_38;
        PyObject *tmp_assattr_target_96;
        PyObject *tmp_expression_name_86;
        tmp_list_element_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_list_element_38 == NULL)) {
            tmp_list_element_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_list_element_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 328;

            goto try_except_handler_2;
        }
        tmp_assattr_name_96 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_96, 0, tmp_list_element_38);
        tmp_expression_name_86 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_86 == NULL)) {
            tmp_expression_name_86 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_86 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_96);

            exception_lineno = 328;

            goto try_except_handler_2;
        }
        tmp_assattr_target_96 = LOOKUP_ATTRIBUTE(tmp_expression_name_86, mod_consts[138]);
        if (tmp_assattr_target_96 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_96);

            exception_lineno = 328;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_96, mod_consts[91], tmp_assattr_name_96);
        Py_DECREF(tmp_assattr_name_96);
        Py_DECREF(tmp_assattr_target_96);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 328;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_97;
        PyObject *tmp_assattr_target_97;
        PyObject *tmp_expression_name_87;
        tmp_assattr_name_97 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_assattr_name_97 == NULL)) {
            tmp_assattr_name_97 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_assattr_name_97 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 329;

            goto try_except_handler_2;
        }
        tmp_expression_name_87 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_87 == NULL)) {
            tmp_expression_name_87 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_87 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 329;

            goto try_except_handler_2;
        }
        tmp_assattr_target_97 = LOOKUP_ATTRIBUTE(tmp_expression_name_87, mod_consts[138]);
        if (tmp_assattr_target_97 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 329;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_97, mod_consts[92], tmp_assattr_name_97);
        Py_DECREF(tmp_assattr_target_97);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 329;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_98;
        PyObject *tmp_list_element_39;
        PyObject *tmp_assattr_target_98;
        PyObject *tmp_expression_name_88;
        tmp_list_element_39 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_list_element_39 == NULL)) {
            tmp_list_element_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_list_element_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 331;

            goto try_except_handler_2;
        }
        tmp_assattr_name_98 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_98, 0, tmp_list_element_39);
        tmp_expression_name_88 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_88 == NULL)) {
            tmp_expression_name_88 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_88 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_98);

            exception_lineno = 331;

            goto try_except_handler_2;
        }
        tmp_assattr_target_98 = LOOKUP_ATTRIBUTE(tmp_expression_name_88, mod_consts[139]);
        if (tmp_assattr_target_98 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_98);

            exception_lineno = 331;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_98, mod_consts[91], tmp_assattr_name_98);
        Py_DECREF(tmp_assattr_name_98);
        Py_DECREF(tmp_assattr_target_98);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 331;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_99;
        PyObject *tmp_assattr_target_99;
        PyObject *tmp_expression_name_89;
        tmp_assattr_name_99 = Py_None;
        tmp_expression_name_89 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_89 == NULL)) {
            tmp_expression_name_89 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_89 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 332;

            goto try_except_handler_2;
        }
        tmp_assattr_target_99 = LOOKUP_ATTRIBUTE(tmp_expression_name_89, mod_consts[139]);
        if (tmp_assattr_target_99 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 332;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_99, mod_consts[92], tmp_assattr_name_99);
        Py_DECREF(tmp_assattr_target_99);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 332;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_100;
        PyObject *tmp_list_element_40;
        PyObject *tmp_assattr_target_100;
        PyObject *tmp_expression_name_90;
        tmp_list_element_40 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_list_element_40 == NULL)) {
            tmp_list_element_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_list_element_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 334;

            goto try_except_handler_2;
        }
        tmp_assattr_name_100 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_100, 0, tmp_list_element_40);
        tmp_expression_name_90 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_90 == NULL)) {
            tmp_expression_name_90 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_90 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_100);

            exception_lineno = 334;

            goto try_except_handler_2;
        }
        tmp_assattr_target_100 = LOOKUP_ATTRIBUTE(tmp_expression_name_90, mod_consts[140]);
        if (tmp_assattr_target_100 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_100);

            exception_lineno = 334;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_100, mod_consts[91], tmp_assattr_name_100);
        Py_DECREF(tmp_assattr_name_100);
        Py_DECREF(tmp_assattr_target_100);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 334;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_101;
        PyObject *tmp_assattr_target_101;
        PyObject *tmp_expression_name_91;
        tmp_assattr_name_101 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[61]);

        if (unlikely(tmp_assattr_name_101 == NULL)) {
            tmp_assattr_name_101 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[61]);
        }

        if (tmp_assattr_name_101 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 335;

            goto try_except_handler_2;
        }
        tmp_expression_name_91 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_91 == NULL)) {
            tmp_expression_name_91 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_91 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 335;

            goto try_except_handler_2;
        }
        tmp_assattr_target_101 = LOOKUP_ATTRIBUTE(tmp_expression_name_91, mod_consts[140]);
        if (tmp_assattr_target_101 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 335;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_101, mod_consts[92], tmp_assattr_name_101);
        Py_DECREF(tmp_assattr_target_101);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 335;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_102;
        PyObject *tmp_list_element_41;
        PyObject *tmp_assattr_target_102;
        PyObject *tmp_expression_name_92;
        tmp_list_element_41 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_list_element_41 == NULL)) {
            tmp_list_element_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_list_element_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 338;

            goto try_except_handler_2;
        }
        tmp_assattr_name_102 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_102, 0, tmp_list_element_41);
        tmp_list_element_41 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

        if (unlikely(tmp_list_element_41 == NULL)) {
            tmp_list_element_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
        }

        if (tmp_list_element_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 339;

            goto list_build_exception_33;
        }
        PyList_SET_ITEM0(tmp_assattr_name_102, 1, tmp_list_element_41);
        tmp_list_element_41 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[53]);

        if (unlikely(tmp_list_element_41 == NULL)) {
            tmp_list_element_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[53]);
        }

        if (tmp_list_element_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 340;

            goto list_build_exception_33;
        }
        PyList_SET_ITEM0(tmp_assattr_name_102, 2, tmp_list_element_41);
        goto list_build_noexception_33;
        // Exception handling pass through code for list_build:
        list_build_exception_33:;
        Py_DECREF(tmp_assattr_name_102);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_33:;
        tmp_expression_name_92 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_92 == NULL)) {
            tmp_expression_name_92 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_92 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_102);

            exception_lineno = 337;

            goto try_except_handler_2;
        }
        tmp_assattr_target_102 = LOOKUP_ATTRIBUTE(tmp_expression_name_92, mod_consts[141]);
        if (tmp_assattr_target_102 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_102);

            exception_lineno = 337;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_102, mod_consts[91], tmp_assattr_name_102);
        Py_DECREF(tmp_assattr_name_102);
        Py_DECREF(tmp_assattr_target_102);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 337;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_103;
        PyObject *tmp_assattr_target_103;
        PyObject *tmp_expression_name_93;
        tmp_assattr_name_103 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_assattr_name_103 == NULL)) {
            tmp_assattr_name_103 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_assattr_name_103 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 342;

            goto try_except_handler_2;
        }
        tmp_expression_name_93 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_93 == NULL)) {
            tmp_expression_name_93 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_93 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 342;

            goto try_except_handler_2;
        }
        tmp_assattr_target_103 = LOOKUP_ATTRIBUTE(tmp_expression_name_93, mod_consts[141]);
        if (tmp_assattr_target_103 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 342;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_103, mod_consts[92], tmp_assattr_name_103);
        Py_DECREF(tmp_assattr_target_103);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 342;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_104;
        PyObject *tmp_list_element_42;
        PyObject *tmp_assattr_target_104;
        PyObject *tmp_expression_name_94;
        tmp_list_element_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_list_element_42 == NULL)) {
            tmp_list_element_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_list_element_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 344;

            goto try_except_handler_2;
        }
        tmp_assattr_name_104 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_104, 0, tmp_list_element_42);
        tmp_list_element_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[53]);

        if (unlikely(tmp_list_element_42 == NULL)) {
            tmp_list_element_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[53]);
        }

        if (tmp_list_element_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 344;

            goto list_build_exception_34;
        }
        PyList_SET_ITEM0(tmp_assattr_name_104, 1, tmp_list_element_42);
        goto list_build_noexception_34;
        // Exception handling pass through code for list_build:
        list_build_exception_34:;
        Py_DECREF(tmp_assattr_name_104);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_34:;
        tmp_expression_name_94 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_94 == NULL)) {
            tmp_expression_name_94 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_94 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_104);

            exception_lineno = 344;

            goto try_except_handler_2;
        }
        tmp_assattr_target_104 = LOOKUP_ATTRIBUTE(tmp_expression_name_94, mod_consts[142]);
        if (tmp_assattr_target_104 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_104);

            exception_lineno = 344;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_104, mod_consts[91], tmp_assattr_name_104);
        Py_DECREF(tmp_assattr_name_104);
        Py_DECREF(tmp_assattr_target_104);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 344;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_105;
        PyObject *tmp_assattr_target_105;
        PyObject *tmp_expression_name_95;
        tmp_assattr_name_105 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

        if (unlikely(tmp_assattr_name_105 == NULL)) {
            tmp_assattr_name_105 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
        }

        if (tmp_assattr_name_105 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 345;

            goto try_except_handler_2;
        }
        tmp_expression_name_95 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_95 == NULL)) {
            tmp_expression_name_95 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_95 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 345;

            goto try_except_handler_2;
        }
        tmp_assattr_target_105 = LOOKUP_ATTRIBUTE(tmp_expression_name_95, mod_consts[142]);
        if (tmp_assattr_target_105 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 345;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_105, mod_consts[92], tmp_assattr_name_105);
        Py_DECREF(tmp_assattr_target_105);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 345;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_106;
        PyObject *tmp_list_element_43;
        PyObject *tmp_assattr_target_106;
        PyObject *tmp_expression_name_96;
        tmp_list_element_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_list_element_43 == NULL)) {
            tmp_list_element_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_list_element_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 348;

            goto try_except_handler_2;
        }
        tmp_assattr_name_106 = PyList_New(4);
        PyList_SET_ITEM0(tmp_assattr_name_106, 0, tmp_list_element_43);
        tmp_list_element_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

        if (unlikely(tmp_list_element_43 == NULL)) {
            tmp_list_element_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
        }

        if (tmp_list_element_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 349;

            goto list_build_exception_35;
        }
        PyList_SET_ITEM0(tmp_assattr_name_106, 1, tmp_list_element_43);
        tmp_list_element_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_list_element_43 == NULL)) {
            tmp_list_element_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_list_element_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 350;

            goto list_build_exception_35;
        }
        PyList_SET_ITEM0(tmp_assattr_name_106, 2, tmp_list_element_43);
        tmp_list_element_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[53]);

        if (unlikely(tmp_list_element_43 == NULL)) {
            tmp_list_element_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[53]);
        }

        if (tmp_list_element_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 351;

            goto list_build_exception_35;
        }
        PyList_SET_ITEM0(tmp_assattr_name_106, 3, tmp_list_element_43);
        goto list_build_noexception_35;
        // Exception handling pass through code for list_build:
        list_build_exception_35:;
        Py_DECREF(tmp_assattr_name_106);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_35:;
        tmp_expression_name_96 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_96 == NULL)) {
            tmp_expression_name_96 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_96 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_106);

            exception_lineno = 347;

            goto try_except_handler_2;
        }
        tmp_assattr_target_106 = LOOKUP_ATTRIBUTE(tmp_expression_name_96, mod_consts[143]);
        if (tmp_assattr_target_106 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_106);

            exception_lineno = 347;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_106, mod_consts[91], tmp_assattr_name_106);
        Py_DECREF(tmp_assattr_name_106);
        Py_DECREF(tmp_assattr_target_106);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 347;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_107;
        PyObject *tmp_assattr_target_107;
        PyObject *tmp_expression_name_97;
        tmp_assattr_name_107 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[23]);

        if (unlikely(tmp_assattr_name_107 == NULL)) {
            tmp_assattr_name_107 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[23]);
        }

        if (tmp_assattr_name_107 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 353;

            goto try_except_handler_2;
        }
        tmp_expression_name_97 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_97 == NULL)) {
            tmp_expression_name_97 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_97 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 353;

            goto try_except_handler_2;
        }
        tmp_assattr_target_107 = LOOKUP_ATTRIBUTE(tmp_expression_name_97, mod_consts[143]);
        if (tmp_assattr_target_107 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 353;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_107, mod_consts[92], tmp_assattr_name_107);
        Py_DECREF(tmp_assattr_target_107);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 353;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_108;
        PyObject *tmp_list_element_44;
        PyObject *tmp_assattr_target_108;
        PyObject *tmp_expression_name_98;
        tmp_list_element_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_list_element_44 == NULL)) {
            tmp_list_element_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_list_element_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 355;

            goto try_except_handler_2;
        }
        tmp_assattr_name_108 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_108, 0, tmp_list_element_44);
        tmp_list_element_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[25]);

        if (unlikely(tmp_list_element_44 == NULL)) {
            tmp_list_element_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[25]);
        }

        if (tmp_list_element_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 355;

            goto list_build_exception_36;
        }
        PyList_SET_ITEM0(tmp_assattr_name_108, 1, tmp_list_element_44);
        tmp_list_element_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_list_element_44 == NULL)) {
            tmp_list_element_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_list_element_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 355;

            goto list_build_exception_36;
        }
        PyList_SET_ITEM0(tmp_assattr_name_108, 2, tmp_list_element_44);
        goto list_build_noexception_36;
        // Exception handling pass through code for list_build:
        list_build_exception_36:;
        Py_DECREF(tmp_assattr_name_108);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_36:;
        tmp_expression_name_98 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_98 == NULL)) {
            tmp_expression_name_98 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_98 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_108);

            exception_lineno = 355;

            goto try_except_handler_2;
        }
        tmp_assattr_target_108 = LOOKUP_ATTRIBUTE(tmp_expression_name_98, mod_consts[144]);
        if (tmp_assattr_target_108 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_108);

            exception_lineno = 355;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_108, mod_consts[91], tmp_assattr_name_108);
        Py_DECREF(tmp_assattr_name_108);
        Py_DECREF(tmp_assattr_target_108);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 355;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_109;
        PyObject *tmp_assattr_target_109;
        PyObject *tmp_expression_name_99;
        tmp_assattr_name_109 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65]);

        if (unlikely(tmp_assattr_name_109 == NULL)) {
            tmp_assattr_name_109 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[65]);
        }

        if (tmp_assattr_name_109 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 356;

            goto try_except_handler_2;
        }
        tmp_expression_name_99 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_99 == NULL)) {
            tmp_expression_name_99 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_99 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 356;

            goto try_except_handler_2;
        }
        tmp_assattr_target_109 = LOOKUP_ATTRIBUTE(tmp_expression_name_99, mod_consts[144]);
        if (tmp_assattr_target_109 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 356;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_109, mod_consts[92], tmp_assattr_name_109);
        Py_DECREF(tmp_assattr_target_109);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 356;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_110;
        PyObject *tmp_list_element_45;
        PyObject *tmp_assattr_target_110;
        PyObject *tmp_expression_name_100;
        tmp_list_element_45 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65]);

        if (unlikely(tmp_list_element_45 == NULL)) {
            tmp_list_element_45 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[65]);
        }

        if (tmp_list_element_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 358;

            goto try_except_handler_2;
        }
        tmp_assattr_name_110 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_110, 0, tmp_list_element_45);
        tmp_expression_name_100 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_100 == NULL)) {
            tmp_expression_name_100 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_100 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_110);

            exception_lineno = 358;

            goto try_except_handler_2;
        }
        tmp_assattr_target_110 = LOOKUP_ATTRIBUTE(tmp_expression_name_100, mod_consts[145]);
        if (tmp_assattr_target_110 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_110);

            exception_lineno = 358;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_110, mod_consts[91], tmp_assattr_name_110);
        Py_DECREF(tmp_assattr_name_110);
        Py_DECREF(tmp_assattr_target_110);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 358;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_111;
        PyObject *tmp_assattr_target_111;
        PyObject *tmp_expression_name_101;
        tmp_assattr_name_111 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_assattr_name_111 == NULL)) {
            tmp_assattr_name_111 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_assattr_name_111 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 359;

            goto try_except_handler_2;
        }
        tmp_expression_name_101 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_101 == NULL)) {
            tmp_expression_name_101 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_101 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 359;

            goto try_except_handler_2;
        }
        tmp_assattr_target_111 = LOOKUP_ATTRIBUTE(tmp_expression_name_101, mod_consts[145]);
        if (tmp_assattr_target_111 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 359;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_111, mod_consts[92], tmp_assattr_name_111);
        Py_DECREF(tmp_assattr_target_111);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 359;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_112;
        PyObject *tmp_list_element_46;
        PyObject *tmp_assattr_target_112;
        PyObject *tmp_expression_name_102;
        tmp_list_element_46 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[65]);

        if (unlikely(tmp_list_element_46 == NULL)) {
            tmp_list_element_46 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[65]);
        }

        if (tmp_list_element_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 361;

            goto try_except_handler_2;
        }
        tmp_assattr_name_112 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_112, 0, tmp_list_element_46);
        tmp_expression_name_102 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_102 == NULL)) {
            tmp_expression_name_102 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_102 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_112);

            exception_lineno = 361;

            goto try_except_handler_2;
        }
        tmp_assattr_target_112 = LOOKUP_ATTRIBUTE(tmp_expression_name_102, mod_consts[146]);
        if (tmp_assattr_target_112 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_112);

            exception_lineno = 361;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_112, mod_consts[91], tmp_assattr_name_112);
        Py_DECREF(tmp_assattr_name_112);
        Py_DECREF(tmp_assattr_target_112);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 361;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_113;
        PyObject *tmp_assattr_target_113;
        PyObject *tmp_expression_name_103;
        tmp_assattr_name_113 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assattr_name_113 == NULL)) {
            tmp_assattr_name_113 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assattr_name_113 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 362;

            goto try_except_handler_2;
        }
        tmp_expression_name_103 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_103 == NULL)) {
            tmp_expression_name_103 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_103 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 362;

            goto try_except_handler_2;
        }
        tmp_assattr_target_113 = LOOKUP_ATTRIBUTE(tmp_expression_name_103, mod_consts[146]);
        if (tmp_assattr_target_113 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 362;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_113, mod_consts[92], tmp_assattr_name_113);
        Py_DECREF(tmp_assattr_target_113);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 362;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_114;
        PyObject *tmp_list_element_47;
        PyObject *tmp_assattr_target_114;
        PyObject *tmp_expression_name_104;
        tmp_list_element_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_list_element_47 == NULL)) {
            tmp_list_element_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_list_element_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 365;

            goto try_except_handler_2;
        }
        tmp_assattr_name_114 = PyList_New(6);
        {
            PyObject *tmp_called_name_42;
            PyObject *tmp_args_element_name_49;
            PyObject *tmp_called_name_43;
            PyObject *tmp_args_element_name_50;
            PyList_SET_ITEM0(tmp_assattr_name_114, 0, tmp_list_element_47);
            tmp_called_name_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_42 == NULL)) {
                tmp_called_name_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_42 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 366;

                goto list_build_exception_37;
            }
            tmp_args_element_name_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

            if (unlikely(tmp_args_element_name_49 == NULL)) {
                tmp_args_element_name_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
            }

            if (tmp_args_element_name_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 366;

                goto list_build_exception_37;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 366;
            tmp_list_element_47 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_42, tmp_args_element_name_49);
            if (tmp_list_element_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 366;

                goto list_build_exception_37;
            }
            PyList_SET_ITEM(tmp_assattr_name_114, 1, tmp_list_element_47);
            tmp_called_name_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_43 == NULL)) {
                tmp_called_name_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_43 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 367;

                goto list_build_exception_37;
            }
            tmp_args_element_name_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

            if (unlikely(tmp_args_element_name_50 == NULL)) {
                tmp_args_element_name_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
            }

            if (tmp_args_element_name_50 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 367;

                goto list_build_exception_37;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 367;
            tmp_list_element_47 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_43, tmp_args_element_name_50);
            if (tmp_list_element_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 367;

                goto list_build_exception_37;
            }
            PyList_SET_ITEM(tmp_assattr_name_114, 2, tmp_list_element_47);
            tmp_list_element_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

            if (unlikely(tmp_list_element_47 == NULL)) {
                tmp_list_element_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
            }

            if (tmp_list_element_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 368;

                goto list_build_exception_37;
            }
            PyList_SET_ITEM0(tmp_assattr_name_114, 3, tmp_list_element_47);
            tmp_list_element_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[71]);

            if (unlikely(tmp_list_element_47 == NULL)) {
                tmp_list_element_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[71]);
            }

            if (tmp_list_element_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 369;

                goto list_build_exception_37;
            }
            PyList_SET_ITEM0(tmp_assattr_name_114, 4, tmp_list_element_47);
            tmp_list_element_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[72]);

            if (unlikely(tmp_list_element_47 == NULL)) {
                tmp_list_element_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[72]);
            }

            if (tmp_list_element_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 370;

                goto list_build_exception_37;
            }
            PyList_SET_ITEM0(tmp_assattr_name_114, 5, tmp_list_element_47);
        }
        goto list_build_noexception_37;
        // Exception handling pass through code for list_build:
        list_build_exception_37:;
        Py_DECREF(tmp_assattr_name_114);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_37:;
        tmp_expression_name_104 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_104 == NULL)) {
            tmp_expression_name_104 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_104 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_114);

            exception_lineno = 364;

            goto try_except_handler_2;
        }
        tmp_assattr_target_114 = LOOKUP_ATTRIBUTE(tmp_expression_name_104, mod_consts[147]);
        if (tmp_assattr_target_114 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_114);

            exception_lineno = 364;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_114, mod_consts[91], tmp_assattr_name_114);
        Py_DECREF(tmp_assattr_name_114);
        Py_DECREF(tmp_assattr_target_114);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 364;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_115;
        PyObject *tmp_assattr_target_115;
        PyObject *tmp_expression_name_105;
        tmp_assattr_name_115 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[69]);

        if (unlikely(tmp_assattr_name_115 == NULL)) {
            tmp_assattr_name_115 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[69]);
        }

        if (tmp_assattr_name_115 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 372;

            goto try_except_handler_2;
        }
        tmp_expression_name_105 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_105 == NULL)) {
            tmp_expression_name_105 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_105 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 372;

            goto try_except_handler_2;
        }
        tmp_assattr_target_115 = LOOKUP_ATTRIBUTE(tmp_expression_name_105, mod_consts[147]);
        if (tmp_assattr_target_115 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 372;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_115, mod_consts[92], tmp_assattr_name_115);
        Py_DECREF(tmp_assattr_target_115);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 372;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_116;
        PyObject *tmp_list_element_48;
        PyObject *tmp_assattr_target_116;
        PyObject *tmp_expression_name_106;
        tmp_list_element_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[69]);

        if (unlikely(tmp_list_element_48 == NULL)) {
            tmp_list_element_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[69]);
        }

        if (tmp_list_element_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 374;

            goto try_except_handler_2;
        }
        tmp_assattr_name_116 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_116, 0, tmp_list_element_48);
        tmp_list_element_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_list_element_48 == NULL)) {
            tmp_list_element_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_list_element_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 374;

            goto list_build_exception_38;
        }
        PyList_SET_ITEM0(tmp_assattr_name_116, 1, tmp_list_element_48);
        goto list_build_noexception_38;
        // Exception handling pass through code for list_build:
        list_build_exception_38:;
        Py_DECREF(tmp_assattr_name_116);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_38:;
        tmp_expression_name_106 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_106 == NULL)) {
            tmp_expression_name_106 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_106 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_116);

            exception_lineno = 374;

            goto try_except_handler_2;
        }
        tmp_assattr_target_116 = LOOKUP_ATTRIBUTE(tmp_expression_name_106, mod_consts[148]);
        if (tmp_assattr_target_116 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_116);

            exception_lineno = 374;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_116, mod_consts[91], tmp_assattr_name_116);
        Py_DECREF(tmp_assattr_name_116);
        Py_DECREF(tmp_assattr_target_116);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 374;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_117;
        PyObject *tmp_assattr_target_117;
        PyObject *tmp_expression_name_107;
        tmp_assattr_name_117 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_assattr_name_117 == NULL)) {
            tmp_assattr_name_117 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_assattr_name_117 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 375;

            goto try_except_handler_2;
        }
        tmp_expression_name_107 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_107 == NULL)) {
            tmp_expression_name_107 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_107 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 375;

            goto try_except_handler_2;
        }
        tmp_assattr_target_117 = LOOKUP_ATTRIBUTE(tmp_expression_name_107, mod_consts[148]);
        if (tmp_assattr_target_117 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 375;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_117, mod_consts[92], tmp_assattr_name_117);
        Py_DECREF(tmp_assattr_target_117);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 375;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_118;
        PyObject *tmp_list_element_49;
        PyObject *tmp_assattr_target_118;
        PyObject *tmp_expression_name_108;
        tmp_list_element_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_list_element_49 == NULL)) {
            tmp_list_element_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_list_element_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 378;

            goto try_except_handler_2;
        }
        tmp_assattr_name_118 = PyList_New(4);
        {
            PyObject *tmp_called_name_44;
            PyObject *tmp_args_element_name_51;
            PyList_SET_ITEM0(tmp_assattr_name_118, 0, tmp_list_element_49);
            tmp_called_name_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[22]);

            if (unlikely(tmp_called_name_44 == NULL)) {
                tmp_called_name_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[22]);
            }

            if (tmp_called_name_44 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 379;

                goto list_build_exception_39;
            }
            tmp_args_element_name_51 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

            if (unlikely(tmp_args_element_name_51 == NULL)) {
                tmp_args_element_name_51 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
            }

            if (tmp_args_element_name_51 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 379;

                goto list_build_exception_39;
            }
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 379;
            tmp_list_element_49 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_44, tmp_args_element_name_51);
            if (tmp_list_element_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 379;

                goto list_build_exception_39;
            }
            PyList_SET_ITEM(tmp_assattr_name_118, 1, tmp_list_element_49);
            tmp_list_element_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

            if (unlikely(tmp_list_element_49 == NULL)) {
                tmp_list_element_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
            }

            if (tmp_list_element_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 380;

                goto list_build_exception_39;
            }
            PyList_SET_ITEM0(tmp_assattr_name_118, 2, tmp_list_element_49);
            tmp_list_element_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[70]);

            if (unlikely(tmp_list_element_49 == NULL)) {
                tmp_list_element_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[70]);
            }

            if (tmp_list_element_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 381;

                goto list_build_exception_39;
            }
            PyList_SET_ITEM0(tmp_assattr_name_118, 3, tmp_list_element_49);
        }
        goto list_build_noexception_39;
        // Exception handling pass through code for list_build:
        list_build_exception_39:;
        Py_DECREF(tmp_assattr_name_118);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_39:;
        tmp_expression_name_108 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_108 == NULL)) {
            tmp_expression_name_108 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_108 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_118);

            exception_lineno = 377;

            goto try_except_handler_2;
        }
        tmp_assattr_target_118 = LOOKUP_ATTRIBUTE(tmp_expression_name_108, mod_consts[149]);
        if (tmp_assattr_target_118 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_118);

            exception_lineno = 377;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_118, mod_consts[91], tmp_assattr_name_118);
        Py_DECREF(tmp_assattr_name_118);
        Py_DECREF(tmp_assattr_target_118);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 377;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_119;
        PyObject *tmp_assattr_target_119;
        PyObject *tmp_expression_name_109;
        tmp_assattr_name_119 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

        if (unlikely(tmp_assattr_name_119 == NULL)) {
            tmp_assattr_name_119 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
        }

        if (tmp_assattr_name_119 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 383;

            goto try_except_handler_2;
        }
        tmp_expression_name_109 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_109 == NULL)) {
            tmp_expression_name_109 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_109 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 383;

            goto try_except_handler_2;
        }
        tmp_assattr_target_119 = LOOKUP_ATTRIBUTE(tmp_expression_name_109, mod_consts[149]);
        if (tmp_assattr_target_119 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 383;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_119, mod_consts[92], tmp_assattr_name_119);
        Py_DECREF(tmp_assattr_target_119);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 383;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_120;
        PyObject *tmp_list_element_50;
        PyObject *tmp_assattr_target_120;
        PyObject *tmp_expression_name_110;
        tmp_list_element_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_list_element_50 == NULL)) {
            tmp_list_element_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_list_element_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 386;

            goto try_except_handler_2;
        }
        tmp_assattr_name_120 = PyList_New(3);
        PyList_SET_ITEM0(tmp_assattr_name_120, 0, tmp_list_element_50);
        tmp_list_element_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_list_element_50 == NULL)) {
            tmp_list_element_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_list_element_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 387;

            goto list_build_exception_40;
        }
        PyList_SET_ITEM0(tmp_assattr_name_120, 1, tmp_list_element_50);
        tmp_list_element_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[70]);

        if (unlikely(tmp_list_element_50 == NULL)) {
            tmp_list_element_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[70]);
        }

        if (tmp_list_element_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 388;

            goto list_build_exception_40;
        }
        PyList_SET_ITEM0(tmp_assattr_name_120, 2, tmp_list_element_50);
        goto list_build_noexception_40;
        // Exception handling pass through code for list_build:
        list_build_exception_40:;
        Py_DECREF(tmp_assattr_name_120);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_40:;
        tmp_expression_name_110 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_110 == NULL)) {
            tmp_expression_name_110 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_110 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_120);

            exception_lineno = 385;

            goto try_except_handler_2;
        }
        tmp_assattr_target_120 = LOOKUP_ATTRIBUTE(tmp_expression_name_110, mod_consts[150]);
        if (tmp_assattr_target_120 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_120);

            exception_lineno = 385;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_120, mod_consts[91], tmp_assattr_name_120);
        Py_DECREF(tmp_assattr_name_120);
        Py_DECREF(tmp_assattr_target_120);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 385;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_121;
        PyObject *tmp_assattr_target_121;
        PyObject *tmp_expression_name_111;
        tmp_assattr_name_121 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[68]);

        if (unlikely(tmp_assattr_name_121 == NULL)) {
            tmp_assattr_name_121 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[68]);
        }

        if (tmp_assattr_name_121 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 390;

            goto try_except_handler_2;
        }
        tmp_expression_name_111 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_111 == NULL)) {
            tmp_expression_name_111 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_111 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 390;

            goto try_except_handler_2;
        }
        tmp_assattr_target_121 = LOOKUP_ATTRIBUTE(tmp_expression_name_111, mod_consts[150]);
        if (tmp_assattr_target_121 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 390;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_121, mod_consts[92], tmp_assattr_name_121);
        Py_DECREF(tmp_assattr_target_121);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 390;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_122;
        PyObject *tmp_list_element_51;
        PyObject *tmp_assattr_target_122;
        PyObject *tmp_expression_name_112;
        tmp_list_element_51 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[68]);

        if (unlikely(tmp_list_element_51 == NULL)) {
            tmp_list_element_51 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[68]);
        }

        if (tmp_list_element_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 392;

            goto try_except_handler_2;
        }
        tmp_assattr_name_122 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_122, 0, tmp_list_element_51);
        tmp_list_element_51 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_list_element_51 == NULL)) {
            tmp_list_element_51 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_list_element_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 392;

            goto list_build_exception_41;
        }
        PyList_SET_ITEM0(tmp_assattr_name_122, 1, tmp_list_element_51);
        goto list_build_noexception_41;
        // Exception handling pass through code for list_build:
        list_build_exception_41:;
        Py_DECREF(tmp_assattr_name_122);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_41:;
        tmp_expression_name_112 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_112 == NULL)) {
            tmp_expression_name_112 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_112 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_122);

            exception_lineno = 392;

            goto try_except_handler_2;
        }
        tmp_assattr_target_122 = LOOKUP_ATTRIBUTE(tmp_expression_name_112, mod_consts[151]);
        if (tmp_assattr_target_122 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_122);

            exception_lineno = 392;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_122, mod_consts[91], tmp_assattr_name_122);
        Py_DECREF(tmp_assattr_name_122);
        Py_DECREF(tmp_assattr_target_122);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 392;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_123;
        PyObject *tmp_assattr_target_123;
        PyObject *tmp_expression_name_113;
        tmp_assattr_name_123 = Py_None;
        tmp_expression_name_113 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_113 == NULL)) {
            tmp_expression_name_113 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_113 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 393;

            goto try_except_handler_2;
        }
        tmp_assattr_target_123 = LOOKUP_ATTRIBUTE(tmp_expression_name_113, mod_consts[151]);
        if (tmp_assattr_target_123 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 393;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_123, mod_consts[92], tmp_assattr_name_123);
        Py_DECREF(tmp_assattr_target_123);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 393;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_124;
        PyObject *tmp_list_element_52;
        PyObject *tmp_assattr_target_124;
        PyObject *tmp_expression_name_114;
        tmp_list_element_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

        if (unlikely(tmp_list_element_52 == NULL)) {
            tmp_list_element_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
        }

        if (tmp_list_element_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 395;

            goto try_except_handler_2;
        }
        tmp_assattr_name_124 = PyList_New(1);
        PyList_SET_ITEM0(tmp_assattr_name_124, 0, tmp_list_element_52);
        tmp_expression_name_114 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_114 == NULL)) {
            tmp_expression_name_114 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_114 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_124);

            exception_lineno = 395;

            goto try_except_handler_2;
        }
        tmp_assattr_target_124 = LOOKUP_ATTRIBUTE(tmp_expression_name_114, mod_consts[152]);
        if (tmp_assattr_target_124 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_124);

            exception_lineno = 395;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_124, mod_consts[91], tmp_assattr_name_124);
        Py_DECREF(tmp_assattr_name_124);
        Py_DECREF(tmp_assattr_target_124);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 395;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_125;
        PyObject *tmp_assattr_target_125;
        PyObject *tmp_expression_name_115;
        tmp_assattr_name_125 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_assattr_name_125 == NULL)) {
            tmp_assattr_name_125 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_assattr_name_125 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 396;

            goto try_except_handler_2;
        }
        tmp_expression_name_115 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_115 == NULL)) {
            tmp_expression_name_115 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_115 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 396;

            goto try_except_handler_2;
        }
        tmp_assattr_target_125 = LOOKUP_ATTRIBUTE(tmp_expression_name_115, mod_consts[152]);
        if (tmp_assattr_target_125 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 396;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_125, mod_consts[92], tmp_assattr_name_125);
        Py_DECREF(tmp_assattr_target_125);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 396;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_126;
        PyObject *tmp_list_element_53;
        PyObject *tmp_assattr_target_126;
        PyObject *tmp_expression_name_116;
        tmp_list_element_53 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

        if (unlikely(tmp_list_element_53 == NULL)) {
            tmp_list_element_53 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
        }

        if (tmp_list_element_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 398;

            goto try_except_handler_2;
        }
        tmp_assattr_name_126 = PyList_New(2);
        PyList_SET_ITEM0(tmp_assattr_name_126, 0, tmp_list_element_53);
        tmp_list_element_53 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[52]);

        if (unlikely(tmp_list_element_53 == NULL)) {
            tmp_list_element_53 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[52]);
        }

        if (tmp_list_element_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 398;

            goto list_build_exception_42;
        }
        PyList_SET_ITEM0(tmp_assattr_name_126, 1, tmp_list_element_53);
        goto list_build_noexception_42;
        // Exception handling pass through code for list_build:
        list_build_exception_42:;
        Py_DECREF(tmp_assattr_name_126);
        goto try_except_handler_2;
        // Finished with no exception for list_build:
        list_build_noexception_42:;
        tmp_expression_name_116 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_116 == NULL)) {
            tmp_expression_name_116 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_116 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_126);

            exception_lineno = 398;

            goto try_except_handler_2;
        }
        tmp_assattr_target_126 = LOOKUP_ATTRIBUTE(tmp_expression_name_116, mod_consts[153]);
        if (tmp_assattr_target_126 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_126);

            exception_lineno = 398;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_126, mod_consts[91], tmp_assattr_name_126);
        Py_DECREF(tmp_assattr_name_126);
        Py_DECREF(tmp_assattr_target_126);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 398;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_127;
        PyObject *tmp_assattr_target_127;
        PyObject *tmp_expression_name_117;
        tmp_assattr_name_127 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_assattr_name_127 == NULL)) {
            tmp_assattr_name_127 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_assattr_name_127 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 399;

            goto try_except_handler_2;
        }
        tmp_expression_name_117 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_expression_name_117 == NULL)) {
            tmp_expression_name_117 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_expression_name_117 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 399;

            goto try_except_handler_2;
        }
        tmp_assattr_target_127 = LOOKUP_ATTRIBUTE(tmp_expression_name_117, mod_consts[153]);
        if (tmp_assattr_target_127 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 399;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_127, mod_consts[92], tmp_assattr_name_127);
        Py_DECREF(tmp_assattr_target_127);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 399;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_128;
        PyObject *tmp_called_name_45;
        PyObject *tmp_expression_name_118;
        PyObject *tmp_args_element_name_52;
        PyObject *tmp_args_element_name_53;
        PyObject *tmp_assattr_target_128;
        tmp_expression_name_118 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[63]);

        if (unlikely(tmp_expression_name_118 == NULL)) {
            tmp_expression_name_118 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[63]);
        }

        if (tmp_expression_name_118 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 401;

            goto try_except_handler_2;
        }
        tmp_called_name_45 = LOOKUP_ATTRIBUTE(tmp_expression_name_118, mod_consts[135]);
        if (tmp_called_name_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 401;

            goto try_except_handler_2;
        }
        tmp_args_element_name_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_args_element_name_52 == NULL)) {
            tmp_args_element_name_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_args_element_name_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_45);

            exception_lineno = 402;

            goto try_except_handler_2;
        }
        tmp_args_element_name_53 = mod_consts[154];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 401;
        {
            PyObject *call_args[] = {tmp_args_element_name_52, tmp_args_element_name_53};
            tmp_assattr_name_128 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_45, call_args);
        }

        Py_DECREF(tmp_called_name_45);
        if (tmp_assattr_name_128 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 401;

            goto try_except_handler_2;
        }
        tmp_assattr_target_128 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_128 == NULL)) {
            tmp_assattr_target_128 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_128 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_128);

            exception_lineno = 401;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_128, mod_consts[154], tmp_assattr_name_128);
        Py_DECREF(tmp_assattr_name_128);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 401;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_129;
        PyObject *tmp_called_name_46;
        PyObject *tmp_expression_name_119;
        PyObject *tmp_args_element_name_54;
        PyObject *tmp_args_element_name_55;
        PyObject *tmp_assattr_target_129;
        tmp_expression_name_119 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_expression_name_119 == NULL)) {
            tmp_expression_name_119 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_expression_name_119 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 404;

            goto try_except_handler_2;
        }
        tmp_called_name_46 = LOOKUP_ATTRIBUTE(tmp_expression_name_119, mod_consts[135]);
        if (tmp_called_name_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 404;

            goto try_except_handler_2;
        }
        tmp_args_element_name_54 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_args_element_name_54 == NULL)) {
            tmp_args_element_name_54 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_args_element_name_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_46);

            exception_lineno = 405;

            goto try_except_handler_2;
        }
        tmp_args_element_name_55 = mod_consts[155];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 404;
        {
            PyObject *call_args[] = {tmp_args_element_name_54, tmp_args_element_name_55};
            tmp_assattr_name_129 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_46, call_args);
        }

        Py_DECREF(tmp_called_name_46);
        if (tmp_assattr_name_129 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 404;

            goto try_except_handler_2;
        }
        tmp_assattr_target_129 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_129 == NULL)) {
            tmp_assattr_target_129 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_129 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_129);

            exception_lineno = 404;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_129, mod_consts[155], tmp_assattr_name_129);
        Py_DECREF(tmp_assattr_name_129);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 404;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_130;
        PyObject *tmp_called_name_47;
        PyObject *tmp_expression_name_120;
        PyObject *tmp_args_element_name_56;
        PyObject *tmp_args_element_name_57;
        PyObject *tmp_assattr_target_130;
        tmp_expression_name_120 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_expression_name_120 == NULL)) {
            tmp_expression_name_120 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_expression_name_120 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 407;

            goto try_except_handler_2;
        }
        tmp_called_name_47 = LOOKUP_ATTRIBUTE(tmp_expression_name_120, mod_consts[135]);
        if (tmp_called_name_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 407;

            goto try_except_handler_2;
        }
        tmp_args_element_name_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_args_element_name_56 == NULL)) {
            tmp_args_element_name_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_args_element_name_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_47);

            exception_lineno = 408;

            goto try_except_handler_2;
        }
        tmp_args_element_name_57 = mod_consts[156];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 407;
        {
            PyObject *call_args[] = {tmp_args_element_name_56, tmp_args_element_name_57};
            tmp_assattr_name_130 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_47, call_args);
        }

        Py_DECREF(tmp_called_name_47);
        if (tmp_assattr_name_130 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 407;

            goto try_except_handler_2;
        }
        tmp_assattr_target_130 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_130 == NULL)) {
            tmp_assattr_target_130 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_130 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_130);

            exception_lineno = 407;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_130, mod_consts[156], tmp_assattr_name_130);
        Py_DECREF(tmp_assattr_name_130);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 407;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_131;
        PyObject *tmp_called_name_48;
        PyObject *tmp_expression_name_121;
        PyObject *tmp_args_element_name_58;
        PyObject *tmp_args_element_name_59;
        PyObject *tmp_assattr_target_131;
        tmp_expression_name_121 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_expression_name_121 == NULL)) {
            tmp_expression_name_121 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_expression_name_121 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 410;

            goto try_except_handler_2;
        }
        tmp_called_name_48 = LOOKUP_ATTRIBUTE(tmp_expression_name_121, mod_consts[135]);
        if (tmp_called_name_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 410;

            goto try_except_handler_2;
        }
        tmp_args_element_name_58 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_args_element_name_58 == NULL)) {
            tmp_args_element_name_58 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_args_element_name_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_48);

            exception_lineno = 411;

            goto try_except_handler_2;
        }
        tmp_args_element_name_59 = mod_consts[157];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 410;
        {
            PyObject *call_args[] = {tmp_args_element_name_58, tmp_args_element_name_59};
            tmp_assattr_name_131 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_48, call_args);
        }

        Py_DECREF(tmp_called_name_48);
        if (tmp_assattr_name_131 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 410;

            goto try_except_handler_2;
        }
        tmp_assattr_target_131 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_131 == NULL)) {
            tmp_assattr_target_131 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_131 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_assattr_name_131);

            exception_lineno = 410;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_131, mod_consts[157], tmp_assattr_name_131);
        Py_DECREF(tmp_assattr_name_131);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 410;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_132;
        PyObject *tmp_assattr_target_132;
        tmp_assattr_name_132 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[62]);

        if (unlikely(tmp_assattr_name_132 == NULL)) {
            tmp_assattr_name_132 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[62]);
        }

        if (tmp_assattr_name_132 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 414;

            goto try_except_handler_2;
        }
        tmp_assattr_target_132 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_132 == NULL)) {
            tmp_assattr_target_132 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_132 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 414;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_132, mod_consts[62], tmp_assattr_name_132);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 414;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_133;
        PyObject *tmp_assattr_target_133;
        tmp_assattr_name_133 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[67]);

        if (unlikely(tmp_assattr_name_133 == NULL)) {
            tmp_assattr_name_133 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[67]);
        }

        if (tmp_assattr_name_133 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 415;

            goto try_except_handler_2;
        }
        tmp_assattr_target_133 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_133 == NULL)) {
            tmp_assattr_target_133 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_133 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 415;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_133, mod_consts[67], tmp_assattr_name_133);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 415;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_134;
        PyObject *tmp_assattr_target_134;
        tmp_assattr_name_134 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[66]);

        if (unlikely(tmp_assattr_name_134 == NULL)) {
            tmp_assattr_name_134 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[66]);
        }

        if (tmp_assattr_name_134 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 416;

            goto try_except_handler_2;
        }
        tmp_assattr_target_134 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_134 == NULL)) {
            tmp_assattr_target_134 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_134 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 416;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_134, mod_consts[66], tmp_assattr_name_134);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 416;

            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_assattr_name_135;
        PyObject *tmp_assattr_target_135;
        tmp_assattr_name_135 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[69]);

        if (unlikely(tmp_assattr_name_135 == NULL)) {
            tmp_assattr_name_135 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[69]);
        }

        if (tmp_assattr_name_135 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 417;

            goto try_except_handler_2;
        }
        tmp_assattr_target_135 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[50]);

        if (unlikely(tmp_assattr_target_135 == NULL)) {
            tmp_assattr_target_135 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[50]);
        }

        if (tmp_assattr_target_135 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 417;

            goto try_except_handler_2;
        }
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_135, mod_consts[69], tmp_assattr_name_135);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 417;

            goto try_except_handler_2;
        }
    }
    goto try_end_4;
    // Exception handler code:
    try_except_handler_2:;
    exception_keeper_type_4 = exception_type;
    exception_keeper_value_4 = exception_value;
    exception_keeper_tb_4 = exception_tb;
    exception_keeper_lineno_4 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    // Preserve existing published exception id 2.
    GET_CURRENT_EXCEPTION(&exception_preserved_type_2, &exception_preserved_value_2, &exception_preserved_tb_2);

    if (exception_keeper_tb_4 == NULL) {
        exception_keeper_tb_4 = MAKE_TRACEBACK(frame_1d818adf931272325ef94f3fb8c15fe1, exception_keeper_lineno_4);
    } else if (exception_keeper_lineno_4 != 0) {
        exception_keeper_tb_4 = ADD_TRACEBACK(exception_keeper_tb_4, frame_1d818adf931272325ef94f3fb8c15fe1, exception_keeper_lineno_4);
    }

    NORMALIZE_EXCEPTION(&exception_keeper_type_4, &exception_keeper_value_4, &exception_keeper_tb_4);
    ATTACH_TRACEBACK_TO_EXCEPTION_VALUE(exception_keeper_value_4, exception_keeper_tb_4);
    PUBLISH_EXCEPTION(&exception_keeper_type_4, &exception_keeper_value_4, &exception_keeper_tb_4);
    // Tried code:
    {
        bool tmp_condition_result_4;
        PyObject *tmp_compexpr_left_4;
        PyObject *tmp_compexpr_right_4;
        tmp_compexpr_left_4 = EXC_TYPE(PyThreadState_GET());
        tmp_compexpr_right_4 = PyExc_AttributeError;
        tmp_res = EXCEPTION_MATCH_BOOL(tmp_compexpr_left_4, tmp_compexpr_right_4);
        assert(!(tmp_res == -1));
        tmp_condition_result_4 = (tmp_res != 0) ? true : false;
        if (tmp_condition_result_4 != false) {
            goto branch_yes_4;
        } else {
            goto branch_no_4;
        }
    }
    branch_yes_4:;
    {
        PyObject *tmp_raise_type_3;
        PyObject *tmp_make_exception_arg_3;
        tmp_make_exception_arg_3 = mod_consts[158];
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 420;
        tmp_raise_type_3 = CALL_FUNCTION_WITH_SINGLE_ARG(PyExc_ImportError, tmp_make_exception_arg_3);
        assert(!(tmp_raise_type_3 == NULL));
        exception_type = tmp_raise_type_3;
        exception_lineno = 420;
        RAISE_EXCEPTION_WITH_TYPE(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_5;
    }
    goto branch_end_4;
    branch_no_4:;
    tmp_result = RERAISE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
    if (unlikely(tmp_result == false)) {
        exception_lineno = 135;
    }

    if (exception_tb && exception_tb->tb_frame == &frame_1d818adf931272325ef94f3fb8c15fe1->m_frame) frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = exception_tb->tb_lineno;

    goto try_except_handler_5;
    branch_end_4:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Exception handler code:
    try_except_handler_5:;
    exception_keeper_type_5 = exception_type;
    exception_keeper_value_5 = exception_value;
    exception_keeper_tb_5 = exception_tb;
    exception_keeper_lineno_5 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    // Restore previous exception id 2.
    SET_CURRENT_EXCEPTION(exception_preserved_type_2, exception_preserved_value_2, exception_preserved_tb_2);

    // Re-raise.
    exception_type = exception_keeper_type_5;
    exception_value = exception_keeper_value_5;
    exception_tb = exception_keeper_tb_5;
    exception_lineno = exception_keeper_lineno_5;

    goto frame_exception_exit_1;
    // End of try:
    // End of try:
    try_end_4:;
    {
        PyObject *tmp_assign_source_67;
        tmp_assign_source_67 = mod_consts[159];
        assert(tmp_class_creation_1__bases == NULL);
        Py_INCREF(tmp_assign_source_67);
        tmp_class_creation_1__bases = tmp_assign_source_67;
    }
    {
        PyObject *tmp_assign_source_68;
        tmp_assign_source_68 = PyDict_New();
        assert(tmp_class_creation_1__class_decl_dict == NULL);
        tmp_class_creation_1__class_decl_dict = tmp_assign_source_68;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_69;
        PyObject *tmp_metaclass_name_1;
        nuitka_bool tmp_condition_result_5;
        PyObject *tmp_key_name_1;
        PyObject *tmp_dict_arg_name_1;
        PyObject *tmp_dict_arg_name_2;
        PyObject *tmp_key_name_2;
        PyObject *tmp_type_arg_1;
        PyObject *tmp_expression_name_122;
        PyObject *tmp_subscript_name_4;
        PyObject *tmp_bases_name_1;
        tmp_key_name_1 = mod_consts[160];
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_1 = tmp_class_creation_1__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_1, tmp_key_name_1);
        assert(!(tmp_res == -1));
        tmp_condition_result_5 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_5 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_1;
        } else {
            goto condexpr_false_1;
        }
        condexpr_true_1:;
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_2 = tmp_class_creation_1__class_decl_dict;
        tmp_key_name_2 = mod_consts[160];
        tmp_metaclass_name_1 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_2, tmp_key_name_2);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        goto condexpr_end_1;
        condexpr_false_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_expression_name_122 = tmp_class_creation_1__bases;
        tmp_subscript_name_4 = mod_consts[18];
        tmp_type_arg_1 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_122, tmp_subscript_name_4, 0);
        if (tmp_type_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        tmp_metaclass_name_1 = BUILTIN_TYPE1(tmp_type_arg_1);
        Py_DECREF(tmp_type_arg_1);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        condexpr_end_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_bases_name_1 = tmp_class_creation_1__bases;
        tmp_assign_source_69 = SELECT_METACLASS(tmp_metaclass_name_1, tmp_bases_name_1);
        Py_DECREF(tmp_metaclass_name_1);
        if (tmp_assign_source_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        assert(tmp_class_creation_1__metaclass == NULL);
        tmp_class_creation_1__metaclass = tmp_assign_source_69;
    }
    {
        nuitka_bool tmp_condition_result_6;
        PyObject *tmp_key_name_3;
        PyObject *tmp_dict_arg_name_3;
        tmp_key_name_3 = mod_consts[160];
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_3 = tmp_class_creation_1__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_3, tmp_key_name_3);
        assert(!(tmp_res == -1));
        tmp_condition_result_6 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_6 == NUITKA_BOOL_TRUE) {
            goto branch_yes_5;
        } else {
            goto branch_no_5;
        }
    }
    branch_yes_5:;
    CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_1__class_decl_dict;
    tmp_dictdel_key = mod_consts[160];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 423;

        goto try_except_handler_6;
    }
    branch_no_5:;
    {
        nuitka_bool tmp_condition_result_7;
        PyObject *tmp_expression_name_123;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_123 = tmp_class_creation_1__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_123, mod_consts[161]);
        tmp_condition_result_7 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_7 == NUITKA_BOOL_TRUE) {
            goto branch_yes_6;
        } else {
            goto branch_no_6;
        }
    }
    branch_yes_6:;
    {
        PyObject *tmp_assign_source_70;
        PyObject *tmp_called_name_49;
        PyObject *tmp_expression_name_124;
        PyObject *tmp_args_name_1;
        PyObject *tmp_tuple_element_2;
        PyObject *tmp_kwargs_name_1;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_124 = tmp_class_creation_1__metaclass;
        tmp_called_name_49 = LOOKUP_ATTRIBUTE(tmp_expression_name_124, mod_consts[161]);
        if (tmp_called_name_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        tmp_tuple_element_2 = mod_consts[162];
        tmp_args_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_1, 0, tmp_tuple_element_2);
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_tuple_element_2 = tmp_class_creation_1__bases;
        PyTuple_SET_ITEM0(tmp_args_name_1, 1, tmp_tuple_element_2);
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_kwargs_name_1 = tmp_class_creation_1__class_decl_dict;
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 423;
        tmp_assign_source_70 = CALL_FUNCTION(tmp_called_name_49, tmp_args_name_1, tmp_kwargs_name_1);
        Py_DECREF(tmp_called_name_49);
        Py_DECREF(tmp_args_name_1);
        if (tmp_assign_source_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_70;
    }
    {
        nuitka_bool tmp_condition_result_8;
        PyObject *tmp_operand_name_1;
        PyObject *tmp_expression_name_125;
        CHECK_OBJECT(tmp_class_creation_1__prepared);
        tmp_expression_name_125 = tmp_class_creation_1__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_125, mod_consts[163]);
        tmp_operand_name_1 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        tmp_condition_result_8 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_8 == NUITKA_BOOL_TRUE) {
            goto branch_yes_7;
        } else {
            goto branch_no_7;
        }
    }
    branch_yes_7:;
    {
        PyObject *tmp_raise_type_4;
        PyObject *tmp_raise_value_1;
        PyObject *tmp_left_name_2;
        PyObject *tmp_right_name_2;
        PyObject *tmp_tuple_element_3;
        PyObject *tmp_getattr_target_1;
        PyObject *tmp_getattr_attr_1;
        PyObject *tmp_getattr_default_1;
        tmp_raise_type_4 = PyExc_TypeError;
        tmp_left_name_2 = mod_consts[164];
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_getattr_target_1 = tmp_class_creation_1__metaclass;
        tmp_getattr_attr_1 = mod_consts[165];
        tmp_getattr_default_1 = mod_consts[166];
        tmp_tuple_element_3 = BUILTIN_GETATTR(tmp_getattr_target_1, tmp_getattr_attr_1, tmp_getattr_default_1);
        if (tmp_tuple_element_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        tmp_right_name_2 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_126;
            PyObject *tmp_type_arg_2;
            PyTuple_SET_ITEM(tmp_right_name_2, 0, tmp_tuple_element_3);
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_type_arg_2 = tmp_class_creation_1__prepared;
            tmp_expression_name_126 = BUILTIN_TYPE1(tmp_type_arg_2);
            assert(!(tmp_expression_name_126 == NULL));
            tmp_tuple_element_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_126, mod_consts[165]);
            Py_DECREF(tmp_expression_name_126);
            if (tmp_tuple_element_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 423;

                goto tuple_build_exception_2;
            }
            PyTuple_SET_ITEM(tmp_right_name_2, 1, tmp_tuple_element_3);
        }
        goto tuple_build_noexception_2;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_2:;
        Py_DECREF(tmp_right_name_2);
        goto try_except_handler_6;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_2:;
        tmp_raise_value_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_2, tmp_right_name_2);
        Py_DECREF(tmp_right_name_2);
        if (tmp_raise_value_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_6;
        }
        exception_type = tmp_raise_type_4;
        Py_INCREF(tmp_raise_type_4);
        exception_value = tmp_raise_value_1;
        exception_lineno = 423;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_6;
    }
    branch_no_7:;
    goto branch_end_6;
    branch_no_6:;
    {
        PyObject *tmp_assign_source_71;
        tmp_assign_source_71 = PyDict_New();
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_71;
    }
    branch_end_6:;
    {
        PyObject *tmp_assign_source_72;
        {
            PyObject *tmp_set_locals_1;
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_set_locals_1 = tmp_class_creation_1__prepared;
            locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423 = tmp_set_locals_1;
            Py_INCREF(tmp_set_locals_1);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[167];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423, mod_consts[168], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_8;
        }
        tmp_dictset_value = mod_consts[169];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423, mod_consts[9], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_8;
        }
        tmp_dictset_value = mod_consts[162];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423, mod_consts[170], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 423;

            goto try_except_handler_8;
        }
        if (isFrameUnusable(cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2)) {
            Py_XDECREF(cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2);

#if _DEBUG_REFCOUNTS
            if (cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2 = MAKE_FUNCTION_FRAME(codeobj_192bd6ff2c8f6e701d6413b996cc4ba3, module_pip$_vendor$urllib3$contrib$_securetransport$bindings, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2->m_type_description == NULL);
        frame_192bd6ff2c8f6e701d6413b996cc4ba3_2 = cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_192bd6ff2c8f6e701d6413b996cc4ba3_2);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_192bd6ff2c8f6e701d6413b996cc4ba3_2) == 2); // Frame stack

        // Framed code:
        {
            PyObject *tmp_called_name_50;
            tmp_called_name_50 = PyObject_GetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423, mod_consts[53]);

            if (tmp_called_name_50 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[53]);

                    if (unlikely(tmp_called_name_50 == NULL)) {
                        tmp_called_name_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[53]);
                    }

                    if (tmp_called_name_50 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 429;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_50);
                }
            }

            frame_192bd6ff2c8f6e701d6413b996cc4ba3_2->m_frame.f_lineno = 429;
            tmp_dictset_value = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_50, mod_consts[171]);
            Py_DECREF(tmp_called_name_50);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 429;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423, mod_consts[172], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 429;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_192bd6ff2c8f6e701d6413b996cc4ba3_2);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_1;

        frame_exception_exit_2:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_192bd6ff2c8f6e701d6413b996cc4ba3_2);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_192bd6ff2c8f6e701d6413b996cc4ba3_2, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_192bd6ff2c8f6e701d6413b996cc4ba3_2->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_192bd6ff2c8f6e701d6413b996cc4ba3_2, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_192bd6ff2c8f6e701d6413b996cc4ba3_2,
            type_description_2,
            outline_0_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_192bd6ff2c8f6e701d6413b996cc4ba3_2 == cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2);
            cache_frame_192bd6ff2c8f6e701d6413b996cc4ba3_2 = NULL;
        }

        assertFrameObject(frame_192bd6ff2c8f6e701d6413b996cc4ba3_2);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_1;

        frame_no_exception_1:;
        goto skip_nested_handling_1;
        nested_frame_exit_1:;

        goto try_except_handler_8;
        skip_nested_handling_1:;
        {
            PyObject *tmp_assign_source_73;
            PyObject *tmp_called_name_51;
            PyObject *tmp_args_name_2;
            PyObject *tmp_tuple_element_4;
            PyObject *tmp_kwargs_name_2;
            CHECK_OBJECT(tmp_class_creation_1__metaclass);
            tmp_called_name_51 = tmp_class_creation_1__metaclass;
            tmp_tuple_element_4 = mod_consts[162];
            tmp_args_name_2 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_2, 0, tmp_tuple_element_4);
            CHECK_OBJECT(tmp_class_creation_1__bases);
            tmp_tuple_element_4 = tmp_class_creation_1__bases;
            PyTuple_SET_ITEM0(tmp_args_name_2, 1, tmp_tuple_element_4);
            tmp_tuple_element_4 = locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423;
            PyTuple_SET_ITEM0(tmp_args_name_2, 2, tmp_tuple_element_4);
            CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
            tmp_kwargs_name_2 = tmp_class_creation_1__class_decl_dict;
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 423;
            tmp_assign_source_73 = CALL_FUNCTION(tmp_called_name_51, tmp_args_name_2, tmp_kwargs_name_2);
            Py_DECREF(tmp_args_name_2);
            if (tmp_assign_source_73 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 423;

                goto try_except_handler_8;
            }
            assert(outline_0_var___class__ == NULL);
            outline_0_var___class__ = tmp_assign_source_73;
        }
        CHECK_OBJECT(outline_0_var___class__);
        tmp_assign_source_72 = outline_0_var___class__;
        Py_INCREF(tmp_assign_source_72);
        goto try_return_handler_8;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_8:;
        Py_DECREF(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423);
        locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423 = NULL;
        goto try_return_handler_7;
        // Exception handler code:
        try_except_handler_8:;
        exception_keeper_type_6 = exception_type;
        exception_keeper_value_6 = exception_value;
        exception_keeper_tb_6 = exception_tb;
        exception_keeper_lineno_6 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423);
        locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__1_CFConst_423 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_6;
        exception_value = exception_keeper_value_6;
        exception_tb = exception_keeper_tb_6;
        exception_lineno = exception_keeper_lineno_6;

        goto try_except_handler_7;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_7:;
        CHECK_OBJECT(outline_0_var___class__);
        Py_DECREF(outline_0_var___class__);
        outline_0_var___class__ = NULL;
        goto outline_result_1;
        // Exception handler code:
        try_except_handler_7:;
        exception_keeper_type_7 = exception_type;
        exception_keeper_value_7 = exception_value;
        exception_keeper_tb_7 = exception_tb;
        exception_keeper_lineno_7 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

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
        exception_lineno = 423;
        goto try_except_handler_6;
        outline_result_1:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[162], tmp_assign_source_72);
    }
    goto try_end_5;
    // Exception handler code:
    try_except_handler_6:;
    exception_keeper_type_8 = exception_type;
    exception_keeper_value_8 = exception_value;
    exception_keeper_tb_8 = exception_tb;
    exception_keeper_lineno_8 = exception_lineno;
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
    exception_type = exception_keeper_type_8;
    exception_value = exception_keeper_value_8;
    exception_tb = exception_keeper_tb_8;
    exception_lineno = exception_keeper_lineno_8;

    goto frame_exception_exit_1;
    // End of try:
    try_end_5:;
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
        tmp_assign_source_74 = mod_consts[159];
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
        nuitka_bool tmp_condition_result_9;
        PyObject *tmp_key_name_4;
        PyObject *tmp_dict_arg_name_4;
        PyObject *tmp_dict_arg_name_5;
        PyObject *tmp_key_name_5;
        PyObject *tmp_type_arg_3;
        PyObject *tmp_expression_name_127;
        PyObject *tmp_subscript_name_5;
        PyObject *tmp_bases_name_2;
        tmp_key_name_4 = mod_consts[160];
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_4 = tmp_class_creation_2__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_4, tmp_key_name_4);
        assert(!(tmp_res == -1));
        tmp_condition_result_9 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_9 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_2;
        } else {
            goto condexpr_false_2;
        }
        condexpr_true_2:;
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_5 = tmp_class_creation_2__class_decl_dict;
        tmp_key_name_5 = mod_consts[160];
        tmp_metaclass_name_2 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_5, tmp_key_name_5);
        if (tmp_metaclass_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        goto condexpr_end_2;
        condexpr_false_2:;
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_expression_name_127 = tmp_class_creation_2__bases;
        tmp_subscript_name_5 = mod_consts[18];
        tmp_type_arg_3 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_127, tmp_subscript_name_5, 0);
        if (tmp_type_arg_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        tmp_metaclass_name_2 = BUILTIN_TYPE1(tmp_type_arg_3);
        Py_DECREF(tmp_type_arg_3);
        if (tmp_metaclass_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        condexpr_end_2:;
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_bases_name_2 = tmp_class_creation_2__bases;
        tmp_assign_source_76 = SELECT_METACLASS(tmp_metaclass_name_2, tmp_bases_name_2);
        Py_DECREF(tmp_metaclass_name_2);
        if (tmp_assign_source_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        assert(tmp_class_creation_2__metaclass == NULL);
        tmp_class_creation_2__metaclass = tmp_assign_source_76;
    }
    {
        nuitka_bool tmp_condition_result_10;
        PyObject *tmp_key_name_6;
        PyObject *tmp_dict_arg_name_6;
        tmp_key_name_6 = mod_consts[160];
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_6 = tmp_class_creation_2__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_6, tmp_key_name_6);
        assert(!(tmp_res == -1));
        tmp_condition_result_10 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_10 == NUITKA_BOOL_TRUE) {
            goto branch_yes_8;
        } else {
            goto branch_no_8;
        }
    }
    branch_yes_8:;
    CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_2__class_decl_dict;
    tmp_dictdel_key = mod_consts[160];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 432;

        goto try_except_handler_9;
    }
    branch_no_8:;
    {
        nuitka_bool tmp_condition_result_11;
        PyObject *tmp_expression_name_128;
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_expression_name_128 = tmp_class_creation_2__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_128, mod_consts[161]);
        tmp_condition_result_11 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_11 == NUITKA_BOOL_TRUE) {
            goto branch_yes_9;
        } else {
            goto branch_no_9;
        }
    }
    branch_yes_9:;
    {
        PyObject *tmp_assign_source_77;
        PyObject *tmp_called_name_52;
        PyObject *tmp_expression_name_129;
        PyObject *tmp_args_name_3;
        PyObject *tmp_tuple_element_5;
        PyObject *tmp_kwargs_name_3;
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_expression_name_129 = tmp_class_creation_2__metaclass;
        tmp_called_name_52 = LOOKUP_ATTRIBUTE(tmp_expression_name_129, mod_consts[161]);
        if (tmp_called_name_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        tmp_tuple_element_5 = mod_consts[173];
        tmp_args_name_3 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_3, 0, tmp_tuple_element_5);
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_tuple_element_5 = tmp_class_creation_2__bases;
        PyTuple_SET_ITEM0(tmp_args_name_3, 1, tmp_tuple_element_5);
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_kwargs_name_3 = tmp_class_creation_2__class_decl_dict;
        frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 432;
        tmp_assign_source_77 = CALL_FUNCTION(tmp_called_name_52, tmp_args_name_3, tmp_kwargs_name_3);
        Py_DECREF(tmp_called_name_52);
        Py_DECREF(tmp_args_name_3);
        if (tmp_assign_source_77 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        assert(tmp_class_creation_2__prepared == NULL);
        tmp_class_creation_2__prepared = tmp_assign_source_77;
    }
    {
        nuitka_bool tmp_condition_result_12;
        PyObject *tmp_operand_name_2;
        PyObject *tmp_expression_name_130;
        CHECK_OBJECT(tmp_class_creation_2__prepared);
        tmp_expression_name_130 = tmp_class_creation_2__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_130, mod_consts[163]);
        tmp_operand_name_2 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        tmp_condition_result_12 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_12 == NUITKA_BOOL_TRUE) {
            goto branch_yes_10;
        } else {
            goto branch_no_10;
        }
    }
    branch_yes_10:;
    {
        PyObject *tmp_raise_type_5;
        PyObject *tmp_raise_value_2;
        PyObject *tmp_left_name_3;
        PyObject *tmp_right_name_3;
        PyObject *tmp_tuple_element_6;
        PyObject *tmp_getattr_target_2;
        PyObject *tmp_getattr_attr_2;
        PyObject *tmp_getattr_default_2;
        tmp_raise_type_5 = PyExc_TypeError;
        tmp_left_name_3 = mod_consts[164];
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_getattr_target_2 = tmp_class_creation_2__metaclass;
        tmp_getattr_attr_2 = mod_consts[165];
        tmp_getattr_default_2 = mod_consts[166];
        tmp_tuple_element_6 = BUILTIN_GETATTR(tmp_getattr_target_2, tmp_getattr_attr_2, tmp_getattr_default_2);
        if (tmp_tuple_element_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        tmp_right_name_3 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_131;
            PyObject *tmp_type_arg_4;
            PyTuple_SET_ITEM(tmp_right_name_3, 0, tmp_tuple_element_6);
            CHECK_OBJECT(tmp_class_creation_2__prepared);
            tmp_type_arg_4 = tmp_class_creation_2__prepared;
            tmp_expression_name_131 = BUILTIN_TYPE1(tmp_type_arg_4);
            assert(!(tmp_expression_name_131 == NULL));
            tmp_tuple_element_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_131, mod_consts[165]);
            Py_DECREF(tmp_expression_name_131);
            if (tmp_tuple_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 432;

                goto tuple_build_exception_3;
            }
            PyTuple_SET_ITEM(tmp_right_name_3, 1, tmp_tuple_element_6);
        }
        goto tuple_build_noexception_3;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_3:;
        Py_DECREF(tmp_right_name_3);
        goto try_except_handler_9;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_3:;
        tmp_raise_value_2 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_3, tmp_right_name_3);
        Py_DECREF(tmp_right_name_3);
        if (tmp_raise_value_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_9;
        }
        exception_type = tmp_raise_type_5;
        Py_INCREF(tmp_raise_type_5);
        exception_value = tmp_raise_value_2;
        exception_lineno = 432;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_9;
    }
    branch_no_10:;
    goto branch_end_9;
    branch_no_9:;
    {
        PyObject *tmp_assign_source_78;
        tmp_assign_source_78 = PyDict_New();
        assert(tmp_class_creation_2__prepared == NULL);
        tmp_class_creation_2__prepared = tmp_assign_source_78;
    }
    branch_end_9:;
    {
        PyObject *tmp_assign_source_79;
        {
            PyObject *tmp_set_locals_2;
            CHECK_OBJECT(tmp_class_creation_2__prepared);
            tmp_set_locals_2 = tmp_class_creation_2__prepared;
            locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432 = tmp_set_locals_2;
            Py_INCREF(tmp_set_locals_2);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[167];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[168], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_11;
        }
        tmp_dictset_value = mod_consts[174];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[9], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_11;
        }
        tmp_dictset_value = mod_consts[173];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[170], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 432;

            goto try_except_handler_11;
        }
        if (isFrameUnusable(cache_frame_7bb482a1d6119c41bca3daa9420852e8_3)) {
            Py_XDECREF(cache_frame_7bb482a1d6119c41bca3daa9420852e8_3);

#if _DEBUG_REFCOUNTS
            if (cache_frame_7bb482a1d6119c41bca3daa9420852e8_3 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_7bb482a1d6119c41bca3daa9420852e8_3 = MAKE_FUNCTION_FRAME(codeobj_7bb482a1d6119c41bca3daa9420852e8, module_pip$_vendor$urllib3$contrib$_securetransport$bindings, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_7bb482a1d6119c41bca3daa9420852e8_3->m_type_description == NULL);
        frame_7bb482a1d6119c41bca3daa9420852e8_3 = cache_frame_7bb482a1d6119c41bca3daa9420852e8_3;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_7bb482a1d6119c41bca3daa9420852e8_3);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_7bb482a1d6119c41bca3daa9420852e8_3) == 2); // Frame stack

        // Framed code:
        tmp_dictset_value = mod_consts[18];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[175], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 437;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[45];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[176], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 439;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[177];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[178], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 440;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[179];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[180], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 441;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[181];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[182], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 442;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[183];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[184], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 443;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[185];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[186], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 445;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[187];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[188], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 446;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[45];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[189], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 448;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[18];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[190], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 449;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[185];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[191], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 451;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[18];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[192], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 453;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[45];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[193], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 454;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[194];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[195], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 457;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[179];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[196], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 458;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[197];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[198], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 459;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[199];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[200], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 460;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[181];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[201], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 461;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[202];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[203], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 463;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[204];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[205], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 464;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[206];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[207], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 465;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[208];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[209], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 466;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[210];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[211], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 467;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[212];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[213], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 469;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[214];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[215], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 470;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[216];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[217], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 471;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[218];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[219], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 472;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[220];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[221], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 473;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[222];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[223], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 474;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[224];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[225], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 475;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[226];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[227], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 476;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[228];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[229], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 477;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[230];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[231], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 478;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[232];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[233], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 479;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[234];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[235], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 480;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[236];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[237], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 481;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[238];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[239], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 483;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[240];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[241], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 484;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[242];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[243], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 485;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[244];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[245], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 486;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[246];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[247], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 490;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[248];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[249], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 491;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[250];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[251], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 492;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[252];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[253], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 493;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[254];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[255], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 494;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[256];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[257], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 495;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[258];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[259], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 496;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[260];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[261], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 497;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[262];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[263], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 498;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[264];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[265], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 499;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[266];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[267], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 500;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[268];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[269], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 501;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[270];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[271], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 502;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[272];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[273], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 503;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[274];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[275], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 504;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[276];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[277], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 505;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[278];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[279], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 506;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[280];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[281], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 507;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[282];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[283], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 508;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[284];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[285], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 509;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[286];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[287], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 510;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[288];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[289], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 511;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[290];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[291], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 512;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[292];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[293], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 513;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[294];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[295], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 514;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[296];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[297], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 515;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[298];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[299], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 516;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[300];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[301], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 517;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[302];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[303], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 518;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }
        tmp_dictset_value = mod_consts[304];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432, mod_consts[305], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 519;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_7bb482a1d6119c41bca3daa9420852e8_3);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_2;

        frame_exception_exit_3:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_7bb482a1d6119c41bca3daa9420852e8_3);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_7bb482a1d6119c41bca3daa9420852e8_3, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_7bb482a1d6119c41bca3daa9420852e8_3->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_7bb482a1d6119c41bca3daa9420852e8_3, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_7bb482a1d6119c41bca3daa9420852e8_3,
            type_description_2,
            outline_1_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_7bb482a1d6119c41bca3daa9420852e8_3 == cache_frame_7bb482a1d6119c41bca3daa9420852e8_3) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_7bb482a1d6119c41bca3daa9420852e8_3);
            cache_frame_7bb482a1d6119c41bca3daa9420852e8_3 = NULL;
        }

        assertFrameObject(frame_7bb482a1d6119c41bca3daa9420852e8_3);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_2;

        frame_no_exception_2:;
        goto skip_nested_handling_2;
        nested_frame_exit_2:;

        goto try_except_handler_11;
        skip_nested_handling_2:;
        {
            PyObject *tmp_assign_source_80;
            PyObject *tmp_called_name_53;
            PyObject *tmp_args_name_4;
            PyObject *tmp_tuple_element_7;
            PyObject *tmp_kwargs_name_4;
            CHECK_OBJECT(tmp_class_creation_2__metaclass);
            tmp_called_name_53 = tmp_class_creation_2__metaclass;
            tmp_tuple_element_7 = mod_consts[173];
            tmp_args_name_4 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_4, 0, tmp_tuple_element_7);
            CHECK_OBJECT(tmp_class_creation_2__bases);
            tmp_tuple_element_7 = tmp_class_creation_2__bases;
            PyTuple_SET_ITEM0(tmp_args_name_4, 1, tmp_tuple_element_7);
            tmp_tuple_element_7 = locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432;
            PyTuple_SET_ITEM0(tmp_args_name_4, 2, tmp_tuple_element_7);
            CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
            tmp_kwargs_name_4 = tmp_class_creation_2__class_decl_dict;
            frame_1d818adf931272325ef94f3fb8c15fe1->m_frame.f_lineno = 432;
            tmp_assign_source_80 = CALL_FUNCTION(tmp_called_name_53, tmp_args_name_4, tmp_kwargs_name_4);
            Py_DECREF(tmp_args_name_4);
            if (tmp_assign_source_80 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 432;

                goto try_except_handler_11;
            }
            assert(outline_1_var___class__ == NULL);
            outline_1_var___class__ = tmp_assign_source_80;
        }
        CHECK_OBJECT(outline_1_var___class__);
        tmp_assign_source_79 = outline_1_var___class__;
        Py_INCREF(tmp_assign_source_79);
        goto try_return_handler_11;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_11:;
        Py_DECREF(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432);
        locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432 = NULL;
        goto try_return_handler_10;
        // Exception handler code:
        try_except_handler_11:;
        exception_keeper_type_9 = exception_type;
        exception_keeper_value_9 = exception_value;
        exception_keeper_tb_9 = exception_tb;
        exception_keeper_lineno_9 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432);
        locals_pip$_vendor$urllib3$contrib$_securetransport$bindings$$$class__2_SecurityConst_432 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_9;
        exception_value = exception_keeper_value_9;
        exception_tb = exception_keeper_tb_9;
        exception_lineno = exception_keeper_lineno_9;

        goto try_except_handler_10;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_10:;
        CHECK_OBJECT(outline_1_var___class__);
        Py_DECREF(outline_1_var___class__);
        outline_1_var___class__ = NULL;
        goto outline_result_2;
        // Exception handler code:
        try_except_handler_10:;
        exception_keeper_type_10 = exception_type;
        exception_keeper_value_10 = exception_value;
        exception_keeper_tb_10 = exception_tb;
        exception_keeper_lineno_10 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_10;
        exception_value = exception_keeper_value_10;
        exception_tb = exception_keeper_tb_10;
        exception_lineno = exception_keeper_lineno_10;

        goto outline_exception_2;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_2:;
        exception_lineno = 432;
        goto try_except_handler_9;
        outline_result_2:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$contrib$_securetransport$bindings, (Nuitka_StringObject *)mod_consts[173], tmp_assign_source_79);
    }
    goto try_end_6;
    // Exception handler code:
    try_except_handler_9:;
    exception_keeper_type_11 = exception_type;
    exception_keeper_value_11 = exception_value;
    exception_keeper_tb_11 = exception_tb;
    exception_keeper_lineno_11 = exception_lineno;
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
    exception_type = exception_keeper_type_11;
    exception_value = exception_keeper_value_11;
    exception_tb = exception_keeper_tb_11;
    exception_lineno = exception_keeper_lineno_11;

    goto frame_exception_exit_1;
    // End of try:
    try_end_6:;

    // Restore frame exception if necessary.
#if 0
    RESTORE_FRAME_EXCEPTION(frame_1d818adf931272325ef94f3fb8c15fe1);
#endif
    popFrameStack();

    assertFrameObject(frame_1d818adf931272325ef94f3fb8c15fe1);

    goto frame_no_exception_3;

    frame_exception_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_1d818adf931272325ef94f3fb8c15fe1);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_1d818adf931272325ef94f3fb8c15fe1, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_1d818adf931272325ef94f3fb8c15fe1->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_1d818adf931272325ef94f3fb8c15fe1, exception_lineno);
    }

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto module_exception_exit;

    frame_no_exception_3:;
    Py_XDECREF(tmp_class_creation_2__bases);
    tmp_class_creation_2__bases = NULL;
    Py_XDECREF(tmp_class_creation_2__class_decl_dict);
    tmp_class_creation_2__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_2__metaclass);
    tmp_class_creation_2__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_2__prepared);
    Py_DECREF(tmp_class_creation_2__prepared);
    tmp_class_creation_2__prepared = NULL;

    return module_pip$_vendor$urllib3$contrib$_securetransport$bindings;
    module_exception_exit:
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);
    return NULL;
}

