/* Generated code for Python module 'numpy.lib.mixins'
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

/* The "module_numpy$lib$mixins" is a Python object pointer of module type.
 *
 * Note: For full compatibility with CPython, every module variable access
 * needs to go through it except for cases where the module cannot possibly
 * have changed in the mean time.
 */

PyObject *module_numpy$lib$mixins;
PyDictObject *moduledict_numpy$lib$mixins;

/* The declarations of module constants used, if any. */
static PyObject *mod_consts[156];

static PyObject *module_filename_obj = NULL;

/* Indicator if this modules private constants were created yet. */
static bool constants_created = false;

/* Function to create module private constants. */
static void createModuleConstants(void) {
    if (constants_created == false) {
        loadConstantsBlob(&mod_consts[0], UNTRANSLATE("numpy.lib.mixins"));
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
void checkModuleConstants_numpy$lib$mixins(void) {
    // The module may not have been used at all, then ignore this.
    if (constants_created == false) return;

    checkConstantsBlob(&mod_consts[0], "numpy.lib.mixins");
}
#endif

// The module code objects.
static PyCodeObject *codeobj_049ab1cedaec4f0e1c6f449948e8a004;
static PyCodeObject *codeobj_96540219adf36c3ee819922f8ccafbde;
static PyCodeObject *codeobj_a8e2ab87403ef29db33ae7013a7762fe;
static PyCodeObject *codeobj_c5c28261fdcc67cff35ddbe017ec2a4d;
static PyCodeObject *codeobj_3201d19d44251e03b4a52feebab50039;
static PyCodeObject *codeobj_ac637e788d3151f4f7a7198cd226edc4;
static PyCodeObject *codeobj_c9e580a5d85811acf9c8bd21bc0f25f5;
static PyCodeObject *codeobj_1c6c5d8ae1a8582d47a320ef4f42364b;
static PyCodeObject *codeobj_0ec1be6219c8ef5f552813a036cd0538;
static PyCodeObject *codeobj_f1f0d9a2fec2b30ec382e4e325eb718c;
static PyCodeObject *codeobj_fca0f1d46b2b060868903807c74d7c11;
static PyCodeObject *codeobj_7c807946885ac77a2963804208a7dfdc;

static void createModuleCodeObjects(void) {
    module_filename_obj = mod_consts[25]; CHECK_OBJECT(module_filename_obj);
    codeobj_049ab1cedaec4f0e1c6f449948e8a004 = MAKE_CODEOBJECT(module_filename_obj, 1, CO_NOFREE, mod_consts[148], NULL, NULL, 0, 0, 0);
    codeobj_96540219adf36c3ee819922f8ccafbde = MAKE_CODEOBJECT(module_filename_obj, 59, CO_NOFREE, mod_consts[50], mod_consts[149], NULL, 0, 0, 0);
    codeobj_a8e2ab87403ef29db33ae7013a7762fe = MAKE_CODEOBJECT(module_filename_obj, 16, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[17], mod_consts[150], NULL, 2, 0, 0);
    codeobj_c5c28261fdcc67cff35ddbe017ec2a4d = MAKE_CODEOBJECT(module_filename_obj, 8, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[8], mod_consts[151], NULL, 1, 0, 0);
    codeobj_3201d19d44251e03b4a52feebab50039 = MAKE_CODEOBJECT(module_filename_obj, 36, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[19], mod_consts[150], NULL, 2, 0, 0);
    codeobj_ac637e788d3151f4f7a7198cd226edc4 = MAKE_CODEOBJECT(module_filename_obj, 44, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[38], mod_consts[152], NULL, 2, 0, 0);
    codeobj_c9e580a5d85811acf9c8bd21bc0f25f5 = MAKE_CODEOBJECT(module_filename_obj, 26, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[18], mod_consts[150], NULL, 2, 0, 0);
    codeobj_1c6c5d8ae1a8582d47a320ef4f42364b = MAKE_CODEOBJECT(module_filename_obj, 51, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[39], mod_consts[150], NULL, 2, 0, 0);
    codeobj_0ec1be6219c8ef5f552813a036cd0538 = MAKE_CODEOBJECT(module_filename_obj, 18, CO_OPTIMIZED | CO_NEWLOCALS, mod_consts[2], mod_consts[153], mod_consts[154], 2, 0, 0);
    codeobj_f1f0d9a2fec2b30ec382e4e325eb718c = MAKE_CODEOBJECT(module_filename_obj, 28, CO_OPTIMIZED | CO_NEWLOCALS, mod_consts[2], mod_consts[153], mod_consts[154], 2, 0, 0);
    codeobj_fca0f1d46b2b060868903807c74d7c11 = MAKE_CODEOBJECT(module_filename_obj, 38, CO_OPTIMIZED | CO_NEWLOCALS, mod_consts[2], mod_consts[153], mod_consts[154], 2, 0, 0);
    codeobj_7c807946885ac77a2963804208a7dfdc = MAKE_CODEOBJECT(module_filename_obj, 53, CO_OPTIMIZED | CO_NEWLOCALS, mod_consts[2], mod_consts[155], mod_consts[154], 1, 0, 0);
}

// The module function declarations.
static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__1__disables_array_ufunc();


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__2__binary_method();


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__2__binary_method$$$function__1_func(struct Nuitka_CellObject **closure);


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__3__reflected_binary_method();


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__3__reflected_binary_method$$$function__1_func(struct Nuitka_CellObject **closure);


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__4__inplace_binary_method();


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__4__inplace_binary_method$$$function__1_func(struct Nuitka_CellObject **closure);


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__5__numeric_methods();


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__6__unary_method();


static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__6__unary_method$$$function__1_func(struct Nuitka_CellObject **closure);


// The module function definitions.
static PyObject *impl_numpy$lib$mixins$$$function__1__disables_array_ufunc(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_obj = python_pars[0];
    struct Nuitka_FrameObject *frame_c5c28261fdcc67cff35ddbe017ec2a4d;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;
    PyObject *exception_preserved_type_1;
    PyObject *exception_preserved_value_1;
    PyTracebackObject *exception_preserved_tb_1;
    int tmp_res;
    bool tmp_result;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_2;
    static struct Nuitka_FrameObject *cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d = NULL;
    PyObject *exception_keeper_type_3;
    PyObject *exception_keeper_value_3;
    PyTracebackObject *exception_keeper_tb_3;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_3;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d)) {
        Py_XDECREF(cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d);

#if _DEBUG_REFCOUNTS
        if (cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d = MAKE_FUNCTION_FRAME(codeobj_c5c28261fdcc67cff35ddbe017ec2a4d, module_numpy$lib$mixins, sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d->m_type_description == NULL);
    frame_c5c28261fdcc67cff35ddbe017ec2a4d = cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_c5c28261fdcc67cff35ddbe017ec2a4d);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_c5c28261fdcc67cff35ddbe017ec2a4d) == 2); // Frame stack

    // Framed code:
    // Tried code:
    {
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        PyObject *tmp_expression_name_1;
        CHECK_OBJECT(par_obj);
        tmp_expression_name_1 = par_obj;
        tmp_compexpr_left_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[0]);
        if (tmp_compexpr_left_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 11;
            type_description_1 = "o";
            goto try_except_handler_2;
        }
        tmp_compexpr_right_1 = Py_None;
        tmp_return_value = (tmp_compexpr_left_1 == tmp_compexpr_right_1) ? Py_True : Py_False;
        Py_DECREF(tmp_compexpr_left_1);
        Py_INCREF(tmp_return_value);
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
        exception_keeper_tb_1 = MAKE_TRACEBACK(frame_c5c28261fdcc67cff35ddbe017ec2a4d, exception_keeper_lineno_1);
    } else if (exception_keeper_lineno_1 != 0) {
        exception_keeper_tb_1 = ADD_TRACEBACK(exception_keeper_tb_1, frame_c5c28261fdcc67cff35ddbe017ec2a4d, exception_keeper_lineno_1);
    }

    NORMALIZE_EXCEPTION(&exception_keeper_type_1, &exception_keeper_value_1, &exception_keeper_tb_1);
    ATTACH_TRACEBACK_TO_EXCEPTION_VALUE(exception_keeper_value_1, exception_keeper_tb_1);
    PUBLISH_EXCEPTION(&exception_keeper_type_1, &exception_keeper_value_1, &exception_keeper_tb_1);
    // Tried code:
    {
        bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_2;
        PyObject *tmp_compexpr_right_2;
        tmp_compexpr_left_2 = EXC_TYPE(PyThreadState_GET());
        tmp_compexpr_right_2 = PyExc_AttributeError;
        tmp_res = EXCEPTION_MATCH_BOOL(tmp_compexpr_left_2, tmp_compexpr_right_2);
        assert(!(tmp_res == -1));
        tmp_condition_result_1 = (tmp_res != 0) ? true : false;
        if (tmp_condition_result_1 != false) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    tmp_return_value = Py_False;
    Py_INCREF(tmp_return_value);
    goto try_return_handler_3;
    goto branch_end_1;
    branch_no_1:;
    tmp_result = RERAISE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
    if (unlikely(tmp_result == false)) {
        exception_lineno = 10;
    }

    if (exception_tb && exception_tb->tb_frame == &frame_c5c28261fdcc67cff35ddbe017ec2a4d->m_frame) frame_c5c28261fdcc67cff35ddbe017ec2a4d->m_frame.f_lineno = exception_tb->tb_lineno;
    type_description_1 = "o";
    goto try_except_handler_3;
    branch_end_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_3:;
    // Restore previous exception id 1.
    SET_CURRENT_EXCEPTION(exception_preserved_type_1, exception_preserved_value_1, exception_preserved_tb_1);

    goto frame_return_exit_1;
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
    // End of try:

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c5c28261fdcc67cff35ddbe017ec2a4d);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_c5c28261fdcc67cff35ddbe017ec2a4d);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c5c28261fdcc67cff35ddbe017ec2a4d);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_c5c28261fdcc67cff35ddbe017ec2a4d, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_c5c28261fdcc67cff35ddbe017ec2a4d->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_c5c28261fdcc67cff35ddbe017ec2a4d, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_c5c28261fdcc67cff35ddbe017ec2a4d,
        type_description_1,
        par_obj
    );


    // Release cached frame if used for exception.
    if (frame_c5c28261fdcc67cff35ddbe017ec2a4d == cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d);
        cache_frame_c5c28261fdcc67cff35ddbe017ec2a4d = NULL;
    }

    assertFrameObject(frame_c5c28261fdcc67cff35ddbe017ec2a4d);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    Py_XDECREF(par_obj);
    par_obj = NULL;
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

    Py_XDECREF(par_obj);
    par_obj = NULL;
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


static PyObject *impl_numpy$lib$mixins$$$function__2__binary_method(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    struct Nuitka_CellObject *par_ufunc = Nuitka_Cell_New1(python_pars[0]);
    PyObject *par_name = python_pars[1];
    PyObject *var_func = NULL;
    struct Nuitka_FrameObject *frame_a8e2ab87403ef29db33ae7013a7762fe;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_a8e2ab87403ef29db33ae7013a7762fe = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    {
        PyObject *tmp_assign_source_1;
        struct Nuitka_CellObject *tmp_closure_1[1];

        tmp_closure_1[0] = par_ufunc;
        Py_INCREF(tmp_closure_1[0]);

        tmp_assign_source_1 = MAKE_FUNCTION_numpy$lib$mixins$$$function__2__binary_method$$$function__1_func(tmp_closure_1);

        assert(var_func == NULL);
        var_func = tmp_assign_source_1;
    }
    // Tried code:
    if (isFrameUnusable(cache_frame_a8e2ab87403ef29db33ae7013a7762fe)) {
        Py_XDECREF(cache_frame_a8e2ab87403ef29db33ae7013a7762fe);

#if _DEBUG_REFCOUNTS
        if (cache_frame_a8e2ab87403ef29db33ae7013a7762fe == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_a8e2ab87403ef29db33ae7013a7762fe = MAKE_FUNCTION_FRAME(codeobj_a8e2ab87403ef29db33ae7013a7762fe, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_a8e2ab87403ef29db33ae7013a7762fe->m_type_description == NULL);
    frame_a8e2ab87403ef29db33ae7013a7762fe = cache_frame_a8e2ab87403ef29db33ae7013a7762fe;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_a8e2ab87403ef29db33ae7013a7762fe);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_a8e2ab87403ef29db33ae7013a7762fe) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_called_instance_1 = mod_consts[4];
        CHECK_OBJECT(par_name);
        tmp_args_element_name_1 = par_name;
        frame_a8e2ab87403ef29db33ae7013a7762fe->m_frame.f_lineno = 22;
        tmp_assattr_name_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[5], tmp_args_element_name_1);
        if (tmp_assattr_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 22;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(var_func);
        tmp_assattr_target_1 = var_func;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[6], tmp_assattr_name_1);
        Py_DECREF(tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 22;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_a8e2ab87403ef29db33ae7013a7762fe);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_a8e2ab87403ef29db33ae7013a7762fe);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_a8e2ab87403ef29db33ae7013a7762fe, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_a8e2ab87403ef29db33ae7013a7762fe->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_a8e2ab87403ef29db33ae7013a7762fe, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_a8e2ab87403ef29db33ae7013a7762fe,
        type_description_1,
        par_ufunc,
        par_name,
        var_func
    );


    // Release cached frame if used for exception.
    if (frame_a8e2ab87403ef29db33ae7013a7762fe == cache_frame_a8e2ab87403ef29db33ae7013a7762fe) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_a8e2ab87403ef29db33ae7013a7762fe);
        cache_frame_a8e2ab87403ef29db33ae7013a7762fe = NULL;
    }

    assertFrameObject(frame_a8e2ab87403ef29db33ae7013a7762fe);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    CHECK_OBJECT(var_func);
    tmp_return_value = var_func;
    Py_INCREF(tmp_return_value);
    goto try_return_handler_1;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__2__binary_method$$$function__1_func(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_other = python_pars[1];
    struct Nuitka_FrameObject *frame_0ec1be6219c8ef5f552813a036cd0538;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    PyObject *tmp_return_value = NULL;
    static struct Nuitka_FrameObject *cache_frame_0ec1be6219c8ef5f552813a036cd0538 = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_0ec1be6219c8ef5f552813a036cd0538)) {
        Py_XDECREF(cache_frame_0ec1be6219c8ef5f552813a036cd0538);

#if _DEBUG_REFCOUNTS
        if (cache_frame_0ec1be6219c8ef5f552813a036cd0538 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_0ec1be6219c8ef5f552813a036cd0538 = MAKE_FUNCTION_FRAME(codeobj_0ec1be6219c8ef5f552813a036cd0538, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_0ec1be6219c8ef5f552813a036cd0538->m_type_description == NULL);
    frame_0ec1be6219c8ef5f552813a036cd0538 = cache_frame_0ec1be6219c8ef5f552813a036cd0538;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_0ec1be6219c8ef5f552813a036cd0538);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_0ec1be6219c8ef5f552813a036cd0538) == 2); // Frame stack

    // Framed code:
    {
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_called_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        int tmp_truth_name_1;
        tmp_called_name_1 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[8]);

        if (unlikely(tmp_called_name_1 == NULL)) {
            tmp_called_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[8]);
        }

        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 19;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_other);
        tmp_args_element_name_1 = par_other;
        frame_0ec1be6219c8ef5f552813a036cd0538->m_frame.f_lineno = 19;
        tmp_call_result_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_1, tmp_args_element_name_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 19;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        tmp_truth_name_1 = CHECK_IF_TRUE(tmp_call_result_1);
        if (tmp_truth_name_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_call_result_1);

            exception_lineno = 19;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_1 = tmp_truth_name_1 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_call_result_1);
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    tmp_return_value = Py_NotImplemented;
    Py_INCREF(tmp_return_value);
    goto frame_return_exit_1;
    branch_no_1:;
    {
        PyObject *tmp_called_name_2;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        if (Nuitka_Cell_GET(self->m_closure[0]) == NULL) {

            FORMAT_UNBOUND_CLOSURE_ERROR(&exception_type, &exception_value, mod_consts[9]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 21;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }

        tmp_called_name_2 = Nuitka_Cell_GET(self->m_closure[0]);
        CHECK_OBJECT(par_self);
        tmp_args_element_name_2 = par_self;
        CHECK_OBJECT(par_other);
        tmp_args_element_name_3 = par_other;
        frame_0ec1be6219c8ef5f552813a036cd0538->m_frame.f_lineno = 21;
        {
            PyObject *call_args[] = {tmp_args_element_name_2, tmp_args_element_name_3};
            tmp_return_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_2, call_args);
        }

        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 21;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_0ec1be6219c8ef5f552813a036cd0538);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_0ec1be6219c8ef5f552813a036cd0538);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_0ec1be6219c8ef5f552813a036cd0538);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_0ec1be6219c8ef5f552813a036cd0538, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_0ec1be6219c8ef5f552813a036cd0538->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_0ec1be6219c8ef5f552813a036cd0538, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_0ec1be6219c8ef5f552813a036cd0538,
        type_description_1,
        par_self,
        par_other,
        self->m_closure[0]
    );


    // Release cached frame if used for exception.
    if (frame_0ec1be6219c8ef5f552813a036cd0538 == cache_frame_0ec1be6219c8ef5f552813a036cd0538) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_0ec1be6219c8ef5f552813a036cd0538);
        cache_frame_0ec1be6219c8ef5f552813a036cd0538 = NULL;
    }

    assertFrameObject(frame_0ec1be6219c8ef5f552813a036cd0538);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_other);
    Py_DECREF(par_other);
    par_other = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_other);
    Py_DECREF(par_other);
    par_other = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__3__reflected_binary_method(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    struct Nuitka_CellObject *par_ufunc = Nuitka_Cell_New1(python_pars[0]);
    PyObject *par_name = python_pars[1];
    PyObject *var_func = NULL;
    struct Nuitka_FrameObject *frame_c9e580a5d85811acf9c8bd21bc0f25f5;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    {
        PyObject *tmp_assign_source_1;
        struct Nuitka_CellObject *tmp_closure_1[1];

        tmp_closure_1[0] = par_ufunc;
        Py_INCREF(tmp_closure_1[0]);

        tmp_assign_source_1 = MAKE_FUNCTION_numpy$lib$mixins$$$function__3__reflected_binary_method$$$function__1_func(tmp_closure_1);

        assert(var_func == NULL);
        var_func = tmp_assign_source_1;
    }
    // Tried code:
    if (isFrameUnusable(cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5)) {
        Py_XDECREF(cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5);

#if _DEBUG_REFCOUNTS
        if (cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5 = MAKE_FUNCTION_FRAME(codeobj_c9e580a5d85811acf9c8bd21bc0f25f5, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5->m_type_description == NULL);
    frame_c9e580a5d85811acf9c8bd21bc0f25f5 = cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_c9e580a5d85811acf9c8bd21bc0f25f5);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_c9e580a5d85811acf9c8bd21bc0f25f5) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_called_instance_1 = mod_consts[11];
        CHECK_OBJECT(par_name);
        tmp_args_element_name_1 = par_name;
        frame_c9e580a5d85811acf9c8bd21bc0f25f5->m_frame.f_lineno = 32;
        tmp_assattr_name_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[5], tmp_args_element_name_1);
        if (tmp_assattr_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(var_func);
        tmp_assattr_target_1 = var_func;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[6], tmp_assattr_name_1);
        Py_DECREF(tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c9e580a5d85811acf9c8bd21bc0f25f5);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c9e580a5d85811acf9c8bd21bc0f25f5);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_c9e580a5d85811acf9c8bd21bc0f25f5, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_c9e580a5d85811acf9c8bd21bc0f25f5->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_c9e580a5d85811acf9c8bd21bc0f25f5, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_c9e580a5d85811acf9c8bd21bc0f25f5,
        type_description_1,
        par_ufunc,
        par_name,
        var_func
    );


    // Release cached frame if used for exception.
    if (frame_c9e580a5d85811acf9c8bd21bc0f25f5 == cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5);
        cache_frame_c9e580a5d85811acf9c8bd21bc0f25f5 = NULL;
    }

    assertFrameObject(frame_c9e580a5d85811acf9c8bd21bc0f25f5);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    CHECK_OBJECT(var_func);
    tmp_return_value = var_func;
    Py_INCREF(tmp_return_value);
    goto try_return_handler_1;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__3__reflected_binary_method$$$function__1_func(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_other = python_pars[1];
    struct Nuitka_FrameObject *frame_f1f0d9a2fec2b30ec382e4e325eb718c;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    PyObject *tmp_return_value = NULL;
    static struct Nuitka_FrameObject *cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c)) {
        Py_XDECREF(cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c);

#if _DEBUG_REFCOUNTS
        if (cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c = MAKE_FUNCTION_FRAME(codeobj_f1f0d9a2fec2b30ec382e4e325eb718c, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c->m_type_description == NULL);
    frame_f1f0d9a2fec2b30ec382e4e325eb718c = cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_f1f0d9a2fec2b30ec382e4e325eb718c);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_f1f0d9a2fec2b30ec382e4e325eb718c) == 2); // Frame stack

    // Framed code:
    {
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_called_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        int tmp_truth_name_1;
        tmp_called_name_1 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[8]);

        if (unlikely(tmp_called_name_1 == NULL)) {
            tmp_called_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[8]);
        }

        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_other);
        tmp_args_element_name_1 = par_other;
        frame_f1f0d9a2fec2b30ec382e4e325eb718c->m_frame.f_lineno = 29;
        tmp_call_result_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_1, tmp_args_element_name_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        tmp_truth_name_1 = CHECK_IF_TRUE(tmp_call_result_1);
        if (tmp_truth_name_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_call_result_1);

            exception_lineno = 29;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_1 = tmp_truth_name_1 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_call_result_1);
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    tmp_return_value = Py_NotImplemented;
    Py_INCREF(tmp_return_value);
    goto frame_return_exit_1;
    branch_no_1:;
    {
        PyObject *tmp_called_name_2;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        if (Nuitka_Cell_GET(self->m_closure[0]) == NULL) {

            FORMAT_UNBOUND_CLOSURE_ERROR(&exception_type, &exception_value, mod_consts[9]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 31;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }

        tmp_called_name_2 = Nuitka_Cell_GET(self->m_closure[0]);
        CHECK_OBJECT(par_other);
        tmp_args_element_name_2 = par_other;
        CHECK_OBJECT(par_self);
        tmp_args_element_name_3 = par_self;
        frame_f1f0d9a2fec2b30ec382e4e325eb718c->m_frame.f_lineno = 31;
        {
            PyObject *call_args[] = {tmp_args_element_name_2, tmp_args_element_name_3};
            tmp_return_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_2, call_args);
        }

        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 31;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_f1f0d9a2fec2b30ec382e4e325eb718c);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_f1f0d9a2fec2b30ec382e4e325eb718c);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_f1f0d9a2fec2b30ec382e4e325eb718c);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_f1f0d9a2fec2b30ec382e4e325eb718c, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_f1f0d9a2fec2b30ec382e4e325eb718c->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_f1f0d9a2fec2b30ec382e4e325eb718c, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_f1f0d9a2fec2b30ec382e4e325eb718c,
        type_description_1,
        par_self,
        par_other,
        self->m_closure[0]
    );


    // Release cached frame if used for exception.
    if (frame_f1f0d9a2fec2b30ec382e4e325eb718c == cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c);
        cache_frame_f1f0d9a2fec2b30ec382e4e325eb718c = NULL;
    }

    assertFrameObject(frame_f1f0d9a2fec2b30ec382e4e325eb718c);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_other);
    Py_DECREF(par_other);
    par_other = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_other);
    Py_DECREF(par_other);
    par_other = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__4__inplace_binary_method(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    struct Nuitka_CellObject *par_ufunc = Nuitka_Cell_New1(python_pars[0]);
    PyObject *par_name = python_pars[1];
    PyObject *var_func = NULL;
    struct Nuitka_FrameObject *frame_3201d19d44251e03b4a52feebab50039;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_3201d19d44251e03b4a52feebab50039 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    {
        PyObject *tmp_assign_source_1;
        struct Nuitka_CellObject *tmp_closure_1[1];

        tmp_closure_1[0] = par_ufunc;
        Py_INCREF(tmp_closure_1[0]);

        tmp_assign_source_1 = MAKE_FUNCTION_numpy$lib$mixins$$$function__4__inplace_binary_method$$$function__1_func(tmp_closure_1);

        assert(var_func == NULL);
        var_func = tmp_assign_source_1;
    }
    // Tried code:
    if (isFrameUnusable(cache_frame_3201d19d44251e03b4a52feebab50039)) {
        Py_XDECREF(cache_frame_3201d19d44251e03b4a52feebab50039);

#if _DEBUG_REFCOUNTS
        if (cache_frame_3201d19d44251e03b4a52feebab50039 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_3201d19d44251e03b4a52feebab50039 = MAKE_FUNCTION_FRAME(codeobj_3201d19d44251e03b4a52feebab50039, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_3201d19d44251e03b4a52feebab50039->m_type_description == NULL);
    frame_3201d19d44251e03b4a52feebab50039 = cache_frame_3201d19d44251e03b4a52feebab50039;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_3201d19d44251e03b4a52feebab50039);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_3201d19d44251e03b4a52feebab50039) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_called_instance_1 = mod_consts[14];
        CHECK_OBJECT(par_name);
        tmp_args_element_name_1 = par_name;
        frame_3201d19d44251e03b4a52feebab50039->m_frame.f_lineno = 40;
        tmp_assattr_name_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[5], tmp_args_element_name_1);
        if (tmp_assattr_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 40;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(var_func);
        tmp_assattr_target_1 = var_func;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[6], tmp_assattr_name_1);
        Py_DECREF(tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 40;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_3201d19d44251e03b4a52feebab50039);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_3201d19d44251e03b4a52feebab50039);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_3201d19d44251e03b4a52feebab50039, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_3201d19d44251e03b4a52feebab50039->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_3201d19d44251e03b4a52feebab50039, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_3201d19d44251e03b4a52feebab50039,
        type_description_1,
        par_ufunc,
        par_name,
        var_func
    );


    // Release cached frame if used for exception.
    if (frame_3201d19d44251e03b4a52feebab50039 == cache_frame_3201d19d44251e03b4a52feebab50039) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_3201d19d44251e03b4a52feebab50039);
        cache_frame_3201d19d44251e03b4a52feebab50039 = NULL;
    }

    assertFrameObject(frame_3201d19d44251e03b4a52feebab50039);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    CHECK_OBJECT(var_func);
    tmp_return_value = var_func;
    Py_INCREF(tmp_return_value);
    goto try_return_handler_1;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__4__inplace_binary_method$$$function__1_func(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_other = python_pars[1];
    struct Nuitka_FrameObject *frame_fca0f1d46b2b060868903807c74d7c11;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    int tmp_res;
    static struct Nuitka_FrameObject *cache_frame_fca0f1d46b2b060868903807c74d7c11 = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_fca0f1d46b2b060868903807c74d7c11)) {
        Py_XDECREF(cache_frame_fca0f1d46b2b060868903807c74d7c11);

#if _DEBUG_REFCOUNTS
        if (cache_frame_fca0f1d46b2b060868903807c74d7c11 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_fca0f1d46b2b060868903807c74d7c11 = MAKE_FUNCTION_FRAME(codeobj_fca0f1d46b2b060868903807c74d7c11, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_fca0f1d46b2b060868903807c74d7c11->m_type_description == NULL);
    frame_fca0f1d46b2b060868903807c74d7c11 = cache_frame_fca0f1d46b2b060868903807c74d7c11;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_fca0f1d46b2b060868903807c74d7c11);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_fca0f1d46b2b060868903807c74d7c11) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_args_name_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_kwargs_name_1;
        PyObject *tmp_dict_key_1;
        PyObject *tmp_dict_value_1;
        PyObject *tmp_tuple_element_2;
        if (Nuitka_Cell_GET(self->m_closure[0]) == NULL) {

            FORMAT_UNBOUND_CLOSURE_ERROR(&exception_type, &exception_value, mod_consts[9]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 39;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }

        tmp_called_name_1 = Nuitka_Cell_GET(self->m_closure[0]);
        CHECK_OBJECT(par_self);
        tmp_tuple_element_1 = par_self;
        tmp_args_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_1, 0, tmp_tuple_element_1);
        CHECK_OBJECT(par_other);
        tmp_tuple_element_1 = par_other;
        PyTuple_SET_ITEM0(tmp_args_name_1, 1, tmp_tuple_element_1);
        tmp_dict_key_1 = mod_consts[16];
        CHECK_OBJECT(par_self);
        tmp_tuple_element_2 = par_self;
        tmp_dict_value_1 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_dict_value_1, 0, tmp_tuple_element_2);
        tmp_kwargs_name_1 = _PyDict_NewPresized( 1 );
        tmp_res = PyDict_SetItem(tmp_kwargs_name_1, tmp_dict_key_1, tmp_dict_value_1);
        Py_DECREF(tmp_dict_value_1);
        assert(!(tmp_res != 0));
        frame_fca0f1d46b2b060868903807c74d7c11->m_frame.f_lineno = 39;
        tmp_return_value = CALL_FUNCTION(tmp_called_name_1, tmp_args_name_1, tmp_kwargs_name_1);
        Py_DECREF(tmp_args_name_1);
        Py_DECREF(tmp_kwargs_name_1);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 39;
            type_description_1 = "ooc";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_fca0f1d46b2b060868903807c74d7c11);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_fca0f1d46b2b060868903807c74d7c11);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_fca0f1d46b2b060868903807c74d7c11);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_fca0f1d46b2b060868903807c74d7c11, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_fca0f1d46b2b060868903807c74d7c11->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_fca0f1d46b2b060868903807c74d7c11, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_fca0f1d46b2b060868903807c74d7c11,
        type_description_1,
        par_self,
        par_other,
        self->m_closure[0]
    );


    // Release cached frame if used for exception.
    if (frame_fca0f1d46b2b060868903807c74d7c11 == cache_frame_fca0f1d46b2b060868903807c74d7c11) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_fca0f1d46b2b060868903807c74d7c11);
        cache_frame_fca0f1d46b2b060868903807c74d7c11 = NULL;
    }

    assertFrameObject(frame_fca0f1d46b2b060868903807c74d7c11);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_other);
    Py_DECREF(par_other);
    par_other = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_other);
    Py_DECREF(par_other);
    par_other = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__5__numeric_methods(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_ufunc = python_pars[0];
    PyObject *par_name = python_pars[1];
    struct Nuitka_FrameObject *frame_ac637e788d3151f4f7a7198cd226edc4;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_ac637e788d3151f4f7a7198cd226edc4 = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_ac637e788d3151f4f7a7198cd226edc4)) {
        Py_XDECREF(cache_frame_ac637e788d3151f4f7a7198cd226edc4);

#if _DEBUG_REFCOUNTS
        if (cache_frame_ac637e788d3151f4f7a7198cd226edc4 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_ac637e788d3151f4f7a7198cd226edc4 = MAKE_FUNCTION_FRAME(codeobj_ac637e788d3151f4f7a7198cd226edc4, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_ac637e788d3151f4f7a7198cd226edc4->m_type_description == NULL);
    frame_ac637e788d3151f4f7a7198cd226edc4 = cache_frame_ac637e788d3151f4f7a7198cd226edc4;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_ac637e788d3151f4f7a7198cd226edc4);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_ac637e788d3151f4f7a7198cd226edc4) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_called_name_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        tmp_called_name_1 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

        if (unlikely(tmp_called_name_1 == NULL)) {
            tmp_called_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
        }

        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 46;
            type_description_1 = "oo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_ufunc);
        tmp_args_element_name_1 = par_ufunc;
        CHECK_OBJECT(par_name);
        tmp_args_element_name_2 = par_name;
        frame_ac637e788d3151f4f7a7198cd226edc4->m_frame.f_lineno = 46;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_tuple_element_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_1, call_args);
        }

        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 46;
            type_description_1 = "oo";
            goto frame_exception_exit_1;
        }
        tmp_return_value = PyTuple_New(3);
        {
            PyObject *tmp_called_name_2;
            PyObject *tmp_args_element_name_3;
            PyObject *tmp_args_element_name_4;
            PyObject *tmp_called_name_3;
            PyObject *tmp_args_element_name_5;
            PyObject *tmp_args_element_name_6;
            PyTuple_SET_ITEM(tmp_return_value, 0, tmp_tuple_element_1);
            tmp_called_name_2 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[18]);

            if (unlikely(tmp_called_name_2 == NULL)) {
                tmp_called_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[18]);
            }

            if (tmp_called_name_2 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 47;
                type_description_1 = "oo";
                goto tuple_build_exception_1;
            }
            CHECK_OBJECT(par_ufunc);
            tmp_args_element_name_3 = par_ufunc;
            CHECK_OBJECT(par_name);
            tmp_args_element_name_4 = par_name;
            frame_ac637e788d3151f4f7a7198cd226edc4->m_frame.f_lineno = 47;
            {
                PyObject *call_args[] = {tmp_args_element_name_3, tmp_args_element_name_4};
                tmp_tuple_element_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_2, call_args);
            }

            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 47;
                type_description_1 = "oo";
                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_return_value, 1, tmp_tuple_element_1);
            tmp_called_name_3 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[19]);

            if (unlikely(tmp_called_name_3 == NULL)) {
                tmp_called_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[19]);
            }

            if (tmp_called_name_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 48;
                type_description_1 = "oo";
                goto tuple_build_exception_1;
            }
            CHECK_OBJECT(par_ufunc);
            tmp_args_element_name_5 = par_ufunc;
            CHECK_OBJECT(par_name);
            tmp_args_element_name_6 = par_name;
            frame_ac637e788d3151f4f7a7198cd226edc4->m_frame.f_lineno = 48;
            {
                PyObject *call_args[] = {tmp_args_element_name_5, tmp_args_element_name_6};
                tmp_tuple_element_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_3, call_args);
            }

            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 48;
                type_description_1 = "oo";
                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_return_value, 2, tmp_tuple_element_1);
        }
        goto tuple_build_noexception_1;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_return_value);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_1:;
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_ac637e788d3151f4f7a7198cd226edc4);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_ac637e788d3151f4f7a7198cd226edc4);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_ac637e788d3151f4f7a7198cd226edc4);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_ac637e788d3151f4f7a7198cd226edc4, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_ac637e788d3151f4f7a7198cd226edc4->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_ac637e788d3151f4f7a7198cd226edc4, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_ac637e788d3151f4f7a7198cd226edc4,
        type_description_1,
        par_ufunc,
        par_name
    );


    // Release cached frame if used for exception.
    if (frame_ac637e788d3151f4f7a7198cd226edc4 == cache_frame_ac637e788d3151f4f7a7198cd226edc4) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_ac637e788d3151f4f7a7198cd226edc4);
        cache_frame_ac637e788d3151f4f7a7198cd226edc4 = NULL;
    }

    assertFrameObject(frame_ac637e788d3151f4f7a7198cd226edc4);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    CHECK_OBJECT(par_name);
    Py_DECREF(par_name);
    par_name = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    CHECK_OBJECT(par_name);
    Py_DECREF(par_name);
    par_name = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__6__unary_method(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    struct Nuitka_CellObject *par_ufunc = Nuitka_Cell_New1(python_pars[0]);
    PyObject *par_name = python_pars[1];
    PyObject *var_func = NULL;
    struct Nuitka_FrameObject *frame_1c6c5d8ae1a8582d47a320ef4f42364b;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    {
        PyObject *tmp_assign_source_1;
        struct Nuitka_CellObject *tmp_closure_1[1];

        tmp_closure_1[0] = par_ufunc;
        Py_INCREF(tmp_closure_1[0]);

        tmp_assign_source_1 = MAKE_FUNCTION_numpy$lib$mixins$$$function__6__unary_method$$$function__1_func(tmp_closure_1);

        assert(var_func == NULL);
        var_func = tmp_assign_source_1;
    }
    // Tried code:
    if (isFrameUnusable(cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b)) {
        Py_XDECREF(cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b);

#if _DEBUG_REFCOUNTS
        if (cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b = MAKE_FUNCTION_FRAME(codeobj_1c6c5d8ae1a8582d47a320ef4f42364b, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b->m_type_description == NULL);
    frame_1c6c5d8ae1a8582d47a320ef4f42364b = cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_1c6c5d8ae1a8582d47a320ef4f42364b);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_1c6c5d8ae1a8582d47a320ef4f42364b) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_called_instance_1 = mod_consts[4];
        CHECK_OBJECT(par_name);
        tmp_args_element_name_1 = par_name;
        frame_1c6c5d8ae1a8582d47a320ef4f42364b->m_frame.f_lineno = 55;
        tmp_assattr_name_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[5], tmp_args_element_name_1);
        if (tmp_assattr_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 55;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(var_func);
        tmp_assattr_target_1 = var_func;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[6], tmp_assattr_name_1);
        Py_DECREF(tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 55;
            type_description_1 = "coo";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_1c6c5d8ae1a8582d47a320ef4f42364b);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_1c6c5d8ae1a8582d47a320ef4f42364b);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_1c6c5d8ae1a8582d47a320ef4f42364b, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_1c6c5d8ae1a8582d47a320ef4f42364b->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_1c6c5d8ae1a8582d47a320ef4f42364b, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_1c6c5d8ae1a8582d47a320ef4f42364b,
        type_description_1,
        par_ufunc,
        par_name,
        var_func
    );


    // Release cached frame if used for exception.
    if (frame_1c6c5d8ae1a8582d47a320ef4f42364b == cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b);
        cache_frame_1c6c5d8ae1a8582d47a320ef4f42364b = NULL;
    }

    assertFrameObject(frame_1c6c5d8ae1a8582d47a320ef4f42364b);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    CHECK_OBJECT(var_func);
    tmp_return_value = var_func;
    Py_INCREF(tmp_return_value);
    goto try_return_handler_1;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_ufunc);
    Py_DECREF(par_ufunc);
    par_ufunc = NULL;
    Py_XDECREF(par_name);
    par_name = NULL;
    CHECK_OBJECT(var_func);
    Py_DECREF(var_func);
    var_func = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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


static PyObject *impl_numpy$lib$mixins$$$function__6__unary_method$$$function__1_func(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    struct Nuitka_FrameObject *frame_7c807946885ac77a2963804208a7dfdc;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_7c807946885ac77a2963804208a7dfdc = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_7c807946885ac77a2963804208a7dfdc)) {
        Py_XDECREF(cache_frame_7c807946885ac77a2963804208a7dfdc);

#if _DEBUG_REFCOUNTS
        if (cache_frame_7c807946885ac77a2963804208a7dfdc == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_7c807946885ac77a2963804208a7dfdc = MAKE_FUNCTION_FRAME(codeobj_7c807946885ac77a2963804208a7dfdc, module_numpy$lib$mixins, sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_7c807946885ac77a2963804208a7dfdc->m_type_description == NULL);
    frame_7c807946885ac77a2963804208a7dfdc = cache_frame_7c807946885ac77a2963804208a7dfdc;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_7c807946885ac77a2963804208a7dfdc);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_7c807946885ac77a2963804208a7dfdc) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_args_element_name_1;
        if (Nuitka_Cell_GET(self->m_closure[0]) == NULL) {

            FORMAT_UNBOUND_CLOSURE_ERROR(&exception_type, &exception_value, mod_consts[9]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 54;
            type_description_1 = "oc";
            goto frame_exception_exit_1;
        }

        tmp_called_name_1 = Nuitka_Cell_GET(self->m_closure[0]);
        CHECK_OBJECT(par_self);
        tmp_args_element_name_1 = par_self;
        frame_7c807946885ac77a2963804208a7dfdc->m_frame.f_lineno = 54;
        tmp_return_value = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_1, tmp_args_element_name_1);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 54;
            type_description_1 = "oc";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_7c807946885ac77a2963804208a7dfdc);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_7c807946885ac77a2963804208a7dfdc);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_7c807946885ac77a2963804208a7dfdc);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_7c807946885ac77a2963804208a7dfdc, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_7c807946885ac77a2963804208a7dfdc->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_7c807946885ac77a2963804208a7dfdc, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_7c807946885ac77a2963804208a7dfdc,
        type_description_1,
        par_self,
        self->m_closure[0]
    );


    // Release cached frame if used for exception.
    if (frame_7c807946885ac77a2963804208a7dfdc == cache_frame_7c807946885ac77a2963804208a7dfdc) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_7c807946885ac77a2963804208a7dfdc);
        cache_frame_7c807946885ac77a2963804208a7dfdc = NULL;
    }

    assertFrameObject(frame_7c807946885ac77a2963804208a7dfdc);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    goto function_return_exit;
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

    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

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



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__1__disables_array_ufunc() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__1__disables_array_ufunc,
        mod_consts[8],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_c5c28261fdcc67cff35ddbe017ec2a4d,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        mod_consts[1],
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__2__binary_method() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__2__binary_method,
        mod_consts[17],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_a8e2ab87403ef29db33ae7013a7762fe,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        mod_consts[7],
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__2__binary_method$$$function__1_func(struct Nuitka_CellObject **closure) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__2__binary_method$$$function__1_func,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[3],
#endif
        codeobj_0ec1be6219c8ef5f552813a036cd0538,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        NULL,
        closure,
        1
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__3__reflected_binary_method() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__3__reflected_binary_method,
        mod_consts[18],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_c9e580a5d85811acf9c8bd21bc0f25f5,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        mod_consts[12],
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__3__reflected_binary_method$$$function__1_func(struct Nuitka_CellObject **closure) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__3__reflected_binary_method$$$function__1_func,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[10],
#endif
        codeobj_f1f0d9a2fec2b30ec382e4e325eb718c,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        NULL,
        closure,
        1
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__4__inplace_binary_method() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__4__inplace_binary_method,
        mod_consts[19],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_3201d19d44251e03b4a52feebab50039,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        mod_consts[15],
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__4__inplace_binary_method$$$function__1_func(struct Nuitka_CellObject **closure) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__4__inplace_binary_method$$$function__1_func,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[13],
#endif
        codeobj_fca0f1d46b2b060868903807c74d7c11,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        NULL,
        closure,
        1
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__5__numeric_methods() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__5__numeric_methods,
        mod_consts[38],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_ac637e788d3151f4f7a7198cd226edc4,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        mod_consts[20],
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__6__unary_method() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__6__unary_method,
        mod_consts[39],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_1c6c5d8ae1a8582d47a320ef4f42364b,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        mod_consts[22],
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$lib$mixins$$$function__6__unary_method$$$function__1_func(struct Nuitka_CellObject **closure) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$lib$mixins$$$function__6__unary_method$$$function__1_func,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[21],
#endif
        codeobj_7c807946885ac77a2963804208a7dfdc,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$lib$mixins,
        NULL,
        closure,
        1
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

function_impl_code functable_numpy$lib$mixins[] = {
    impl_numpy$lib$mixins$$$function__2__binary_method$$$function__1_func,
    impl_numpy$lib$mixins$$$function__3__reflected_binary_method$$$function__1_func,
    impl_numpy$lib$mixins$$$function__4__inplace_binary_method$$$function__1_func,
    impl_numpy$lib$mixins$$$function__6__unary_method$$$function__1_func,
    impl_numpy$lib$mixins$$$function__1__disables_array_ufunc,
    impl_numpy$lib$mixins$$$function__2__binary_method,
    impl_numpy$lib$mixins$$$function__3__reflected_binary_method,
    impl_numpy$lib$mixins$$$function__4__inplace_binary_method,
    impl_numpy$lib$mixins$$$function__5__numeric_methods,
    impl_numpy$lib$mixins$$$function__6__unary_method,
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

    function_impl_code *current = functable_numpy$lib$mixins;
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

    if (offset > sizeof(functable_numpy$lib$mixins) || offset < 0) {
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
        functable_numpy$lib$mixins[offset],
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
        module_numpy$lib$mixins,
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
PyObject *modulecode_numpy$lib$mixins(PyObject *module, struct Nuitka_MetaPathBasedLoaderEntry const *module_entry) {
    module_numpy$lib$mixins = module;

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
    PRINT_STRING("numpy.lib.mixins: Calling setupMetaPathBasedLoader().\n");
#endif
    setupMetaPathBasedLoader();

#if PYTHON_VERSION >= 0x300
    patchInspectModule();
#endif

#endif

    /* The constants only used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("numpy.lib.mixins: Calling createModuleConstants().\n");
#endif
    createModuleConstants();

    /* The code objects used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("numpy.lib.mixins: Calling createModuleCodeObjects().\n");
#endif
    createModuleCodeObjects();

    // PRINT_STRING("in initnumpy$lib$mixins\n");

    // Create the module object first. There are no methods initially, all are
    // added dynamically in actual code only.  Also no "__doc__" is initially
    // set at this time, as it could not contain NUL characters this way, they
    // are instead set in early module code.  No "self" for modules, we have no
    // use for it.

    moduledict_numpy$lib$mixins = MODULE_DICT(module_numpy$lib$mixins);

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
    registerDillPluginTables(module_entry->name, &_method_def_reduce_compiled_function, &_method_def_create_compiled_function);
#endif

    // Set "__compiled__" to what version information we have.
    UPDATE_STRING_DICT0(
        moduledict_numpy$lib$mixins,
        (Nuitka_StringObject *)const_str_plain___compiled__,
        Nuitka_dunder_compiled_value
    );

    // Update "__package__" value to what it ought to be.
    {
#if 0
        UPDATE_STRING_DICT0(
            moduledict_numpy$lib$mixins,
            (Nuitka_StringObject *)const_str_plain___package__,
            const_str_empty
        );
#elif 0
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___name__);

        UPDATE_STRING_DICT0(
            moduledict_numpy$lib$mixins,
            (Nuitka_StringObject *)const_str_plain___package__,
            module_name
        );
#else

#if PYTHON_VERSION < 0x300
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___name__);
        char const *module_name_cstr = PyString_AS_STRING(module_name);

        char const *last_dot = strrchr(module_name_cstr, '.');

        if (last_dot != NULL) {
            UPDATE_STRING_DICT1(
                moduledict_numpy$lib$mixins,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyString_FromStringAndSize(module_name_cstr, last_dot - module_name_cstr)
            );
        }
#else
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___name__);
        Py_ssize_t dot_index = PyUnicode_Find(module_name, const_str_dot, 0, PyUnicode_GetLength(module_name), -1);

        if (dot_index != -1) {
            UPDATE_STRING_DICT1(
                moduledict_numpy$lib$mixins,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyUnicode_Substring(module_name, 0, dot_index)
            );
        }
#endif
#endif
    }

    CHECK_OBJECT(module_numpy$lib$mixins);

    // For deep importing of a module we need to have "__builtins__", so we set
    // it ourselves in the same way than CPython does. Note: This must be done
    // before the frame object is allocated, or else it may fail.

    if (GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___builtins__) == NULL) {
        PyObject *value = (PyObject *)builtin_module;

        // Check if main module, not a dict then but the module itself.
#if !defined(_NUITKA_EXE) || !0
        value = PyModule_GetDict(value);
#endif

        UPDATE_STRING_DICT0(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___builtins__, value);
    }

#if PYTHON_VERSION >= 0x300
    UPDATE_STRING_DICT0(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___loader__, (PyObject *)&Nuitka_Loader_Type);
#endif

#if PYTHON_VERSION >= 0x340
// Set the "__spec__" value

#if 0
    // Main modules just get "None" as spec.
    UPDATE_STRING_DICT0(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___spec__, Py_None);
#else
    // Other modules get a "ModuleSpec" from the standard mechanism.
    {
        PyObject *bootstrap_module = getImportLibBootstrapModule();
        CHECK_OBJECT(bootstrap_module);

        PyObject *_spec_from_module = PyObject_GetAttrString(bootstrap_module, "_spec_from_module");
        CHECK_OBJECT(_spec_from_module);

        PyObject *spec_value = CALL_FUNCTION_WITH_SINGLE_ARG(_spec_from_module, module_numpy$lib$mixins);
        Py_DECREF(_spec_from_module);

        // We can assume this to never fail, or else we are in trouble anyway.
        // CHECK_OBJECT(spec_value);

        if (spec_value == NULL) {
            PyErr_PrintEx(0);
            abort();
        }

// Mark the execution in the "__spec__" value.
        SET_ATTRIBUTE(spec_value, const_str_plain__initializing, Py_True);

        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)const_str_plain___spec__, spec_value);
    }
#endif
#endif

    // Temp variables if any
    PyObject *outline_0_var___class__ = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3 = NULL;
    PyObject *tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter = NULL;
    PyObject *tmp_class_creation_1__class_decl_dict = NULL;
    PyObject *tmp_class_creation_1__metaclass = NULL;
    PyObject *tmp_class_creation_1__prepared = NULL;
    struct Nuitka_FrameObject *frame_049ab1cedaec4f0e1c6f449948e8a004;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    bool tmp_result;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    int tmp_res;
    PyObject *tmp_dictdel_dict;
    PyObject *tmp_dictdel_key;
    PyObject *locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59 = NULL;
    PyObject *tmp_dictset_value;
    struct Nuitka_FrameObject *frame_96540219adf36c3ee819922f8ccafbde_2;
    NUITKA_MAY_BE_UNUSED char const *type_description_2 = NULL;
    PyObject *tmp_iterator_attempt;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;
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
    PyObject *exception_keeper_type_12;
    PyObject *exception_keeper_value_12;
    PyTracebackObject *exception_keeper_tb_12;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_12;
    PyObject *exception_keeper_type_13;
    PyObject *exception_keeper_value_13;
    PyTracebackObject *exception_keeper_tb_13;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_13;
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
    PyObject *exception_keeper_type_20;
    PyObject *exception_keeper_value_20;
    PyTracebackObject *exception_keeper_tb_20;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_20;
    PyObject *exception_keeper_type_21;
    PyObject *exception_keeper_value_21;
    PyTracebackObject *exception_keeper_tb_21;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_21;
    PyObject *exception_keeper_type_22;
    PyObject *exception_keeper_value_22;
    PyTracebackObject *exception_keeper_tb_22;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_22;
    PyObject *exception_keeper_type_23;
    PyObject *exception_keeper_value_23;
    PyTracebackObject *exception_keeper_tb_23;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_23;
    PyObject *exception_keeper_type_24;
    PyObject *exception_keeper_value_24;
    PyTracebackObject *exception_keeper_tb_24;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_24;
    PyObject *exception_keeper_type_25;
    PyObject *exception_keeper_value_25;
    PyTracebackObject *exception_keeper_tb_25;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_25;
    PyObject *exception_keeper_type_26;
    PyObject *exception_keeper_value_26;
    PyTracebackObject *exception_keeper_tb_26;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_26;
    static struct Nuitka_FrameObject *cache_frame_96540219adf36c3ee819922f8ccafbde_2 = NULL;
    PyObject *exception_keeper_type_27;
    PyObject *exception_keeper_value_27;
    PyTracebackObject *exception_keeper_tb_27;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_27;
    PyObject *exception_keeper_type_28;
    PyObject *exception_keeper_value_28;
    PyTracebackObject *exception_keeper_tb_28;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_28;
    PyObject *exception_keeper_type_29;
    PyObject *exception_keeper_value_29;
    PyTracebackObject *exception_keeper_tb_29;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_29;

    // Module code.
    {
        PyObject *tmp_assign_source_1;
        tmp_assign_source_1 = mod_consts[23];
        UPDATE_STRING_DICT0(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[24], tmp_assign_source_1);
    }
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = mod_consts[25];
        UPDATE_STRING_DICT0(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[26], tmp_assign_source_2);
    }
    // Frame without reuse.
    frame_049ab1cedaec4f0e1c6f449948e8a004 = MAKE_MODULE_FRAME(codeobj_049ab1cedaec4f0e1c6f449948e8a004, module_numpy$lib$mixins);

    // Push the new frame as the currently active one, and we should be exclusively
    // owning it.
    pushFrameStack(frame_049ab1cedaec4f0e1c6f449948e8a004);
    assert(Py_REFCNT(frame_049ab1cedaec4f0e1c6f449948e8a004) == 2);

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_assattr_name_1 = mod_consts[25];
        tmp_assattr_target_1 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[27]);

        if (unlikely(tmp_assattr_target_1 == NULL)) {
            tmp_assattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[27]);
        }

        assert(!(tmp_assattr_target_1 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[28], tmp_assattr_name_1);
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
        tmp_assattr_target_2 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[27]);

        if (unlikely(tmp_assattr_target_2 == NULL)) {
            tmp_assattr_target_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[27]);
        }

        assert(!(tmp_assattr_target_2 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[29], tmp_assattr_name_2);
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
        UPDATE_STRING_DICT0(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[30], tmp_assign_source_3);
    }
    {
        PyObject *tmp_assign_source_4;
        PyObject *tmp_import_name_from_1;
        PyObject *tmp_name_name_1;
        PyObject *tmp_globals_arg_name_1;
        PyObject *tmp_locals_arg_name_1;
        PyObject *tmp_fromlist_name_1;
        PyObject *tmp_level_name_1;
        tmp_name_name_1 = mod_consts[31];
        tmp_globals_arg_name_1 = (PyObject *)moduledict_numpy$lib$mixins;
        tmp_locals_arg_name_1 = Py_None;
        tmp_fromlist_name_1 = mod_consts[32];
        tmp_level_name_1 = mod_consts[33];
        frame_049ab1cedaec4f0e1c6f449948e8a004->m_frame.f_lineno = 2;
        tmp_import_name_from_1 = IMPORT_MODULE5(tmp_name_name_1, tmp_globals_arg_name_1, tmp_locals_arg_name_1, tmp_fromlist_name_1, tmp_level_name_1);
        if (tmp_import_name_from_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_1)) {
            tmp_assign_source_4 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_1,
                (PyObject *)moduledict_numpy$lib$mixins,
                mod_consts[34],
                mod_consts[33]
            );
        } else {
            tmp_assign_source_4 = IMPORT_NAME(tmp_import_name_from_1, mod_consts[34]);
        }

        Py_DECREF(tmp_import_name_from_1);
        if (tmp_assign_source_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35], tmp_assign_source_4);
    }
    {
        PyObject *tmp_assign_source_5;
        tmp_assign_source_5 = LIST_COPY(mod_consts[36]);
        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[37], tmp_assign_source_5);
    }
    {
        PyObject *tmp_assign_source_6;


        tmp_assign_source_6 = MAKE_FUNCTION_numpy$lib$mixins$$$function__1__disables_array_ufunc();

        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[8], tmp_assign_source_6);
    }
    {
        PyObject *tmp_assign_source_7;


        tmp_assign_source_7 = MAKE_FUNCTION_numpy$lib$mixins$$$function__2__binary_method();

        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17], tmp_assign_source_7);
    }
    {
        PyObject *tmp_assign_source_8;


        tmp_assign_source_8 = MAKE_FUNCTION_numpy$lib$mixins$$$function__3__reflected_binary_method();

        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[18], tmp_assign_source_8);
    }
    {
        PyObject *tmp_assign_source_9;


        tmp_assign_source_9 = MAKE_FUNCTION_numpy$lib$mixins$$$function__4__inplace_binary_method();

        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[19], tmp_assign_source_9);
    }
    {
        PyObject *tmp_assign_source_10;


        tmp_assign_source_10 = MAKE_FUNCTION_numpy$lib$mixins$$$function__5__numeric_methods();

        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38], tmp_assign_source_10);
    }
    {
        PyObject *tmp_assign_source_11;


        tmp_assign_source_11 = MAKE_FUNCTION_numpy$lib$mixins$$$function__6__unary_method();

        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[39], tmp_assign_source_11);
    }
    {
        PyObject *tmp_assign_source_12;
        tmp_assign_source_12 = PyDict_New();
        assert(tmp_class_creation_1__class_decl_dict == NULL);
        tmp_class_creation_1__class_decl_dict = tmp_assign_source_12;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_13;
        PyObject *tmp_metaclass_name_1;
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_key_name_1;
        PyObject *tmp_dict_arg_name_1;
        PyObject *tmp_dict_arg_name_2;
        PyObject *tmp_key_name_2;
        PyObject *tmp_bases_name_1;
        tmp_key_name_1 = mod_consts[40];
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_1 = tmp_class_creation_1__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_1, tmp_key_name_1);
        assert(!(tmp_res == -1));
        tmp_condition_result_1 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_1;
        } else {
            goto condexpr_false_1;
        }
        condexpr_true_1:;
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_2 = tmp_class_creation_1__class_decl_dict;
        tmp_key_name_2 = mod_consts[40];
        tmp_metaclass_name_1 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_2, tmp_key_name_2);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_1;
        }
        goto condexpr_end_1;
        condexpr_false_1:;
        tmp_metaclass_name_1 = (PyObject *)&PyType_Type;
        Py_INCREF(tmp_metaclass_name_1);
        condexpr_end_1:;
        tmp_bases_name_1 = mod_consts[41];
        tmp_assign_source_13 = SELECT_METACLASS(tmp_metaclass_name_1, tmp_bases_name_1);
        Py_DECREF(tmp_metaclass_name_1);
        if (tmp_assign_source_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_1;
        }
        assert(tmp_class_creation_1__metaclass == NULL);
        tmp_class_creation_1__metaclass = tmp_assign_source_13;
    }
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_key_name_3;
        PyObject *tmp_dict_arg_name_3;
        tmp_key_name_3 = mod_consts[40];
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_dict_arg_name_3 = tmp_class_creation_1__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_3, tmp_key_name_3);
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
    tmp_dictdel_key = mod_consts[40];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 59;

        goto try_except_handler_1;
    }
    branch_no_1:;
    {
        nuitka_bool tmp_condition_result_3;
        PyObject *tmp_expression_name_1;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_1 = tmp_class_creation_1__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_1, mod_consts[42]);
        tmp_condition_result_3 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_3 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_assign_source_14;
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_args_name_1;
        PyObject *tmp_kwargs_name_1;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_2 = tmp_class_creation_1__metaclass;
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[42]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_1;
        }
        tmp_args_name_1 = mod_consts[43];
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_kwargs_name_1 = tmp_class_creation_1__class_decl_dict;
        frame_049ab1cedaec4f0e1c6f449948e8a004->m_frame.f_lineno = 59;
        tmp_assign_source_14 = CALL_FUNCTION(tmp_called_name_1, tmp_args_name_1, tmp_kwargs_name_1);
        Py_DECREF(tmp_called_name_1);
        if (tmp_assign_source_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_1;
        }
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_14;
    }
    {
        nuitka_bool tmp_condition_result_4;
        PyObject *tmp_operand_name_1;
        PyObject *tmp_expression_name_3;
        CHECK_OBJECT(tmp_class_creation_1__prepared);
        tmp_expression_name_3 = tmp_class_creation_1__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_3, mod_consts[44]);
        tmp_operand_name_1 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_1;
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
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_getattr_target_1;
        PyObject *tmp_getattr_attr_1;
        PyObject *tmp_getattr_default_1;
        tmp_raise_type_1 = PyExc_TypeError;
        tmp_left_name_1 = mod_consts[45];
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_getattr_target_1 = tmp_class_creation_1__metaclass;
        tmp_getattr_attr_1 = mod_consts[6];
        tmp_getattr_default_1 = mod_consts[46];
        tmp_tuple_element_1 = BUILTIN_GETATTR(tmp_getattr_target_1, tmp_getattr_attr_1, tmp_getattr_default_1);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_1;
        }
        tmp_right_name_1 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_4;
            PyObject *tmp_type_arg_1;
            PyTuple_SET_ITEM(tmp_right_name_1, 0, tmp_tuple_element_1);
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_type_arg_1 = tmp_class_creation_1__prepared;
            tmp_expression_name_4 = BUILTIN_TYPE1(tmp_type_arg_1);
            assert(!(tmp_expression_name_4 == NULL));
            tmp_tuple_element_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_4, mod_consts[6]);
            Py_DECREF(tmp_expression_name_4);
            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 59;

                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_right_name_1, 1, tmp_tuple_element_1);
        }
        goto tuple_build_noexception_1;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_right_name_1);
        goto try_except_handler_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_1:;
        tmp_raise_value_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_raise_value_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_1;
        }
        exception_type = tmp_raise_type_1;
        Py_INCREF(tmp_raise_type_1);
        exception_value = tmp_raise_value_1;
        exception_lineno = 59;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_1;
    }
    branch_no_3:;
    goto branch_end_2;
    branch_no_2:;
    {
        PyObject *tmp_assign_source_15;
        tmp_assign_source_15 = PyDict_New();
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_15;
    }
    branch_end_2:;
    {
        PyObject *tmp_assign_source_16;
        {
            PyObject *tmp_set_locals_1;
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_set_locals_1 = tmp_class_creation_1__prepared;
            locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59 = tmp_set_locals_1;
            Py_INCREF(tmp_set_locals_1);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[47];
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[48], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_3;
        }
        tmp_dictset_value = mod_consts[49];
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[24], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_3;
        }
        tmp_dictset_value = mod_consts[50];
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[51], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 59;

            goto try_except_handler_3;
        }
        if (isFrameUnusable(cache_frame_96540219adf36c3ee819922f8ccafbde_2)) {
            Py_XDECREF(cache_frame_96540219adf36c3ee819922f8ccafbde_2);

#if _DEBUG_REFCOUNTS
            if (cache_frame_96540219adf36c3ee819922f8ccafbde_2 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_96540219adf36c3ee819922f8ccafbde_2 = MAKE_FUNCTION_FRAME(codeobj_96540219adf36c3ee819922f8ccafbde, module_numpy$lib$mixins, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_96540219adf36c3ee819922f8ccafbde_2->m_type_description == NULL);
        frame_96540219adf36c3ee819922f8ccafbde_2 = cache_frame_96540219adf36c3ee819922f8ccafbde_2;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_96540219adf36c3ee819922f8ccafbde_2);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_96540219adf36c3ee819922f8ccafbde_2) == 2); // Frame stack

        // Framed code:
        {
            PyObject *tmp_called_name_2;
            PyObject *tmp_args_element_name_1;
            PyObject *tmp_expression_name_5;
            PyObject *tmp_args_element_name_2;
            tmp_called_name_2 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[17]);

            if (tmp_called_name_2 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_2 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

                    if (unlikely(tmp_called_name_2 == NULL)) {
                        tmp_called_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
                    }

                    if (tmp_called_name_2 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 140;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_2);
                }
            }

            tmp_expression_name_5 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_5 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_5 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_5 == NULL)) {
                        tmp_expression_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_5 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_2);

                        exception_lineno = 140;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_5);
                }
            }

            tmp_args_element_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_5, mod_consts[52]);
            Py_DECREF(tmp_expression_name_5);
            if (tmp_args_element_name_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_2);

                exception_lineno = 140;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_2 = mod_consts[53];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 140;
            {
                PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_2, call_args);
            }

            Py_DECREF(tmp_called_name_2);
            Py_DECREF(tmp_args_element_name_1);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[54], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_3;
            PyObject *tmp_args_element_name_3;
            PyObject *tmp_expression_name_6;
            PyObject *tmp_args_element_name_4;
            tmp_called_name_3 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[17]);

            if (tmp_called_name_3 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_3 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

                    if (unlikely(tmp_called_name_3 == NULL)) {
                        tmp_called_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
                    }

                    if (tmp_called_name_3 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 141;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_3);
                }
            }

            tmp_expression_name_6 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_6 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_6 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_6 == NULL)) {
                        tmp_expression_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_6 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_3);

                        exception_lineno = 141;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_6);
                }
            }

            tmp_args_element_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_6, mod_consts[55]);
            Py_DECREF(tmp_expression_name_6);
            if (tmp_args_element_name_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_3);

                exception_lineno = 141;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_4 = mod_consts[56];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 141;
            {
                PyObject *call_args[] = {tmp_args_element_name_3, tmp_args_element_name_4};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_3, call_args);
            }

            Py_DECREF(tmp_called_name_3);
            Py_DECREF(tmp_args_element_name_3);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 141;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[57], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 141;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_4;
            PyObject *tmp_args_element_name_5;
            PyObject *tmp_expression_name_7;
            PyObject *tmp_args_element_name_6;
            tmp_called_name_4 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[17]);

            if (tmp_called_name_4 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_4 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

                    if (unlikely(tmp_called_name_4 == NULL)) {
                        tmp_called_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
                    }

                    if (tmp_called_name_4 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 142;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_4);
                }
            }

            tmp_expression_name_7 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_7 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_7 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_7 == NULL)) {
                        tmp_expression_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_7 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_4);

                        exception_lineno = 142;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_7);
                }
            }

            tmp_args_element_name_5 = LOOKUP_ATTRIBUTE(tmp_expression_name_7, mod_consts[58]);
            Py_DECREF(tmp_expression_name_7);
            if (tmp_args_element_name_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_4);

                exception_lineno = 142;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_6 = mod_consts[59];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 142;
            {
                PyObject *call_args[] = {tmp_args_element_name_5, tmp_args_element_name_6};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_4, call_args);
            }

            Py_DECREF(tmp_called_name_4);
            Py_DECREF(tmp_args_element_name_5);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 142;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[60], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 142;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_5;
            PyObject *tmp_args_element_name_7;
            PyObject *tmp_expression_name_8;
            PyObject *tmp_args_element_name_8;
            tmp_called_name_5 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[17]);

            if (tmp_called_name_5 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_5 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

                    if (unlikely(tmp_called_name_5 == NULL)) {
                        tmp_called_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
                    }

                    if (tmp_called_name_5 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 143;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_5);
                }
            }

            tmp_expression_name_8 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_8 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_8 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_8 == NULL)) {
                        tmp_expression_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_8 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_5);

                        exception_lineno = 143;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_8);
                }
            }

            tmp_args_element_name_7 = LOOKUP_ATTRIBUTE(tmp_expression_name_8, mod_consts[61]);
            Py_DECREF(tmp_expression_name_8);
            if (tmp_args_element_name_7 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_5);

                exception_lineno = 143;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_8 = mod_consts[62];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 143;
            {
                PyObject *call_args[] = {tmp_args_element_name_7, tmp_args_element_name_8};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_5, call_args);
            }

            Py_DECREF(tmp_called_name_5);
            Py_DECREF(tmp_args_element_name_7);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 143;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[63], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 143;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_6;
            PyObject *tmp_args_element_name_9;
            PyObject *tmp_expression_name_9;
            PyObject *tmp_args_element_name_10;
            tmp_called_name_6 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[17]);

            if (tmp_called_name_6 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_6 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

                    if (unlikely(tmp_called_name_6 == NULL)) {
                        tmp_called_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
                    }

                    if (tmp_called_name_6 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 144;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_6);
                }
            }

            tmp_expression_name_9 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_9 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_9 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_9 == NULL)) {
                        tmp_expression_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_9 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_6);

                        exception_lineno = 144;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_9);
                }
            }

            tmp_args_element_name_9 = LOOKUP_ATTRIBUTE(tmp_expression_name_9, mod_consts[64]);
            Py_DECREF(tmp_expression_name_9);
            if (tmp_args_element_name_9 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_6);

                exception_lineno = 144;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_10 = mod_consts[65];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 144;
            {
                PyObject *call_args[] = {tmp_args_element_name_9, tmp_args_element_name_10};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_6, call_args);
            }

            Py_DECREF(tmp_called_name_6);
            Py_DECREF(tmp_args_element_name_9);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 144;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[66], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 144;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_7;
            PyObject *tmp_args_element_name_11;
            PyObject *tmp_expression_name_10;
            PyObject *tmp_args_element_name_12;
            tmp_called_name_7 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[17]);

            if (tmp_called_name_7 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_7 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

                    if (unlikely(tmp_called_name_7 == NULL)) {
                        tmp_called_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
                    }

                    if (tmp_called_name_7 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 145;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_7);
                }
            }

            tmp_expression_name_10 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_10 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_10 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_10 == NULL)) {
                        tmp_expression_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_10 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_7);

                        exception_lineno = 145;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_10);
                }
            }

            tmp_args_element_name_11 = LOOKUP_ATTRIBUTE(tmp_expression_name_10, mod_consts[67]);
            Py_DECREF(tmp_expression_name_10);
            if (tmp_args_element_name_11 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_7);

                exception_lineno = 145;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_12 = mod_consts[68];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 145;
            {
                PyObject *call_args[] = {tmp_args_element_name_11, tmp_args_element_name_12};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_7, call_args);
            }

            Py_DECREF(tmp_called_name_7);
            Py_DECREF(tmp_args_element_name_11);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 145;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[69], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 145;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_17;
            PyObject *tmp_iter_arg_1;
            PyObject *tmp_called_name_8;
            PyObject *tmp_args_element_name_13;
            PyObject *tmp_expression_name_11;
            PyObject *tmp_args_element_name_14;
            tmp_called_name_8 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_8 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_8 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_8 == NULL)) {
                        tmp_called_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_8 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 148;
                        type_description_2 = "o";
                        goto try_except_handler_4;
                    }
                    Py_INCREF(tmp_called_name_8);
                }
            }

            tmp_expression_name_11 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_11 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_11 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_11 == NULL)) {
                        tmp_expression_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_11 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_8);

                        exception_lineno = 148;
                        type_description_2 = "o";
                        goto try_except_handler_4;
                    }
                    Py_INCREF(tmp_expression_name_11);
                }
            }

            tmp_args_element_name_13 = LOOKUP_ATTRIBUTE(tmp_expression_name_11, mod_consts[70]);
            Py_DECREF(tmp_expression_name_11);
            if (tmp_args_element_name_13 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_8);

                exception_lineno = 148;
                type_description_2 = "o";
                goto try_except_handler_4;
            }
            tmp_args_element_name_14 = mod_consts[70];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 148;
            {
                PyObject *call_args[] = {tmp_args_element_name_13, tmp_args_element_name_14};
                tmp_iter_arg_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_8, call_args);
            }

            Py_DECREF(tmp_called_name_8);
            Py_DECREF(tmp_args_element_name_13);
            if (tmp_iter_arg_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 148;
                type_description_2 = "o";
                goto try_except_handler_4;
            }
            tmp_assign_source_17 = MAKE_ITERATOR(tmp_iter_arg_1);
            Py_DECREF(tmp_iter_arg_1);
            if (tmp_assign_source_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 148;
                type_description_2 = "o";
                goto try_except_handler_4;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter = tmp_assign_source_17;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_18;
            PyObject *tmp_unpack_1;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter);
            tmp_unpack_1 = tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter;
            tmp_assign_source_18 = UNPACK_NEXT(tmp_unpack_1, 0, 3);
            if (tmp_assign_source_18 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 148;
                goto try_except_handler_5;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1 = tmp_assign_source_18;
        }
        {
            PyObject *tmp_assign_source_19;
            PyObject *tmp_unpack_2;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter);
            tmp_unpack_2 = tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter;
            tmp_assign_source_19 = UNPACK_NEXT(tmp_unpack_2, 1, 3);
            if (tmp_assign_source_19 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 148;
                goto try_except_handler_5;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2 = tmp_assign_source_19;
        }
        {
            PyObject *tmp_assign_source_20;
            PyObject *tmp_unpack_3;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter);
            tmp_unpack_3 = tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter;
            tmp_assign_source_20 = UNPACK_NEXT(tmp_unpack_3, 2, 3);
            if (tmp_assign_source_20 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 148;
                goto try_except_handler_5;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3 = tmp_assign_source_20;
        }
        {
            PyObject *tmp_iterator_name_1;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter);
            tmp_iterator_name_1 = tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter;
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

                        type_description_2 = "o";
                        exception_lineno = 148;
                        goto try_except_handler_5;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 148;
                goto try_except_handler_5;
            }
        }
        goto try_end_1;
        // Exception handler code:
        try_except_handler_5:;
        exception_keeper_type_1 = exception_type;
        exception_keeper_value_1 = exception_value;
        exception_keeper_tb_1 = exception_tb;
        exception_keeper_lineno_1 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_1;
        exception_value = exception_keeper_value_1;
        exception_tb = exception_keeper_tb_1;
        exception_lineno = exception_keeper_lineno_1;

        goto try_except_handler_4;
        // End of try:
        try_end_1:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[72], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 148;
            type_description_2 = "o";
            goto try_except_handler_4;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[73], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 148;
            type_description_2 = "o";
            goto try_except_handler_4;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[74], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 148;
            type_description_2 = "o";
            goto try_except_handler_4;
        }
        goto try_end_2;
        // Exception handler code:
        try_except_handler_4:;
        exception_keeper_type_2 = exception_type;
        exception_keeper_value_2 = exception_value;
        exception_keeper_tb_2 = exception_tb;
        exception_keeper_lineno_2 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_2;
        exception_value = exception_keeper_value_2;
        exception_tb = exception_keeper_tb_2;
        exception_lineno = exception_keeper_lineno_2;

        goto frame_exception_exit_2;
        // End of try:
        try_end_2:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_1__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_21;
            PyObject *tmp_iter_arg_2;
            PyObject *tmp_called_name_9;
            PyObject *tmp_args_element_name_15;
            PyObject *tmp_expression_name_12;
            PyObject *tmp_args_element_name_16;
            tmp_called_name_9 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_9 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_9 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_9 == NULL)) {
                        tmp_called_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_9 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 149;
                        type_description_2 = "o";
                        goto try_except_handler_6;
                    }
                    Py_INCREF(tmp_called_name_9);
                }
            }

            tmp_expression_name_12 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_12 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_12 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_12 == NULL)) {
                        tmp_expression_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_12 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_9);

                        exception_lineno = 149;
                        type_description_2 = "o";
                        goto try_except_handler_6;
                    }
                    Py_INCREF(tmp_expression_name_12);
                }
            }

            tmp_args_element_name_15 = LOOKUP_ATTRIBUTE(tmp_expression_name_12, mod_consts[75]);
            Py_DECREF(tmp_expression_name_12);
            if (tmp_args_element_name_15 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_9);

                exception_lineno = 149;
                type_description_2 = "o";
                goto try_except_handler_6;
            }
            tmp_args_element_name_16 = mod_consts[76];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 149;
            {
                PyObject *call_args[] = {tmp_args_element_name_15, tmp_args_element_name_16};
                tmp_iter_arg_2 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_9, call_args);
            }

            Py_DECREF(tmp_called_name_9);
            Py_DECREF(tmp_args_element_name_15);
            if (tmp_iter_arg_2 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 149;
                type_description_2 = "o";
                goto try_except_handler_6;
            }
            tmp_assign_source_21 = MAKE_ITERATOR(tmp_iter_arg_2);
            Py_DECREF(tmp_iter_arg_2);
            if (tmp_assign_source_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 149;
                type_description_2 = "o";
                goto try_except_handler_6;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter = tmp_assign_source_21;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_22;
            PyObject *tmp_unpack_4;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter);
            tmp_unpack_4 = tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter;
            tmp_assign_source_22 = UNPACK_NEXT(tmp_unpack_4, 0, 3);
            if (tmp_assign_source_22 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 149;
                goto try_except_handler_7;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1 = tmp_assign_source_22;
        }
        {
            PyObject *tmp_assign_source_23;
            PyObject *tmp_unpack_5;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter);
            tmp_unpack_5 = tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter;
            tmp_assign_source_23 = UNPACK_NEXT(tmp_unpack_5, 1, 3);
            if (tmp_assign_source_23 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 149;
                goto try_except_handler_7;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2 = tmp_assign_source_23;
        }
        {
            PyObject *tmp_assign_source_24;
            PyObject *tmp_unpack_6;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter);
            tmp_unpack_6 = tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter;
            tmp_assign_source_24 = UNPACK_NEXT(tmp_unpack_6, 2, 3);
            if (tmp_assign_source_24 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 149;
                goto try_except_handler_7;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3 = tmp_assign_source_24;
        }
        {
            PyObject *tmp_iterator_name_2;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter);
            tmp_iterator_name_2 = tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter;
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

                        type_description_2 = "o";
                        exception_lineno = 149;
                        goto try_except_handler_7;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 149;
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

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_3;
        exception_value = exception_keeper_value_3;
        exception_tb = exception_keeper_tb_3;
        exception_lineno = exception_keeper_lineno_3;

        goto try_except_handler_6;
        // End of try:
        try_end_3:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[77], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 149;
            type_description_2 = "o";
            goto try_except_handler_6;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[78], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 149;
            type_description_2 = "o";
            goto try_except_handler_6;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[79], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 149;
            type_description_2 = "o";
            goto try_except_handler_6;
        }
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

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_4;
        exception_value = exception_keeper_value_4;
        exception_tb = exception_keeper_tb_4;
        exception_lineno = exception_keeper_lineno_4;

        goto frame_exception_exit_2;
        // End of try:
        try_end_4:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_2__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_25;
            PyObject *tmp_iter_arg_3;
            PyObject *tmp_called_name_10;
            PyObject *tmp_args_element_name_17;
            PyObject *tmp_expression_name_13;
            PyObject *tmp_args_element_name_18;
            tmp_called_name_10 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_10 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_10 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_10 == NULL)) {
                        tmp_called_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_10 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 150;
                        type_description_2 = "o";
                        goto try_except_handler_8;
                    }
                    Py_INCREF(tmp_called_name_10);
                }
            }

            tmp_expression_name_13 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_13 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_13 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_13 == NULL)) {
                        tmp_expression_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_13 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_10);

                        exception_lineno = 150;
                        type_description_2 = "o";
                        goto try_except_handler_8;
                    }
                    Py_INCREF(tmp_expression_name_13);
                }
            }

            tmp_args_element_name_17 = LOOKUP_ATTRIBUTE(tmp_expression_name_13, mod_consts[80]);
            Py_DECREF(tmp_expression_name_13);
            if (tmp_args_element_name_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_10);

                exception_lineno = 150;
                type_description_2 = "o";
                goto try_except_handler_8;
            }
            tmp_args_element_name_18 = mod_consts[81];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 150;
            {
                PyObject *call_args[] = {tmp_args_element_name_17, tmp_args_element_name_18};
                tmp_iter_arg_3 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_10, call_args);
            }

            Py_DECREF(tmp_called_name_10);
            Py_DECREF(tmp_args_element_name_17);
            if (tmp_iter_arg_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 150;
                type_description_2 = "o";
                goto try_except_handler_8;
            }
            tmp_assign_source_25 = MAKE_ITERATOR(tmp_iter_arg_3);
            Py_DECREF(tmp_iter_arg_3);
            if (tmp_assign_source_25 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 150;
                type_description_2 = "o";
                goto try_except_handler_8;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter = tmp_assign_source_25;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_26;
            PyObject *tmp_unpack_7;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter);
            tmp_unpack_7 = tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter;
            tmp_assign_source_26 = UNPACK_NEXT(tmp_unpack_7, 0, 3);
            if (tmp_assign_source_26 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 150;
                goto try_except_handler_9;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1 = tmp_assign_source_26;
        }
        {
            PyObject *tmp_assign_source_27;
            PyObject *tmp_unpack_8;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter);
            tmp_unpack_8 = tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter;
            tmp_assign_source_27 = UNPACK_NEXT(tmp_unpack_8, 1, 3);
            if (tmp_assign_source_27 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 150;
                goto try_except_handler_9;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2 = tmp_assign_source_27;
        }
        {
            PyObject *tmp_assign_source_28;
            PyObject *tmp_unpack_9;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter);
            tmp_unpack_9 = tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter;
            tmp_assign_source_28 = UNPACK_NEXT(tmp_unpack_9, 2, 3);
            if (tmp_assign_source_28 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 150;
                goto try_except_handler_9;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3 = tmp_assign_source_28;
        }
        {
            PyObject *tmp_iterator_name_3;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter);
            tmp_iterator_name_3 = tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter;
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

                        type_description_2 = "o";
                        exception_lineno = 150;
                        goto try_except_handler_9;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 150;
                goto try_except_handler_9;
            }
        }
        goto try_end_5;
        // Exception handler code:
        try_except_handler_9:;
        exception_keeper_type_5 = exception_type;
        exception_keeper_value_5 = exception_value;
        exception_keeper_tb_5 = exception_tb;
        exception_keeper_lineno_5 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_5;
        exception_value = exception_keeper_value_5;
        exception_tb = exception_keeper_tb_5;
        exception_lineno = exception_keeper_lineno_5;

        goto try_except_handler_8;
        // End of try:
        try_end_5:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[82], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 150;
            type_description_2 = "o";
            goto try_except_handler_8;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[83], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 150;
            type_description_2 = "o";
            goto try_except_handler_8;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[84], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 150;
            type_description_2 = "o";
            goto try_except_handler_8;
        }
        goto try_end_6;
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

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_6;
        exception_value = exception_keeper_value_6;
        exception_tb = exception_keeper_tb_6;
        exception_lineno = exception_keeper_lineno_6;

        goto frame_exception_exit_2;
        // End of try:
        try_end_6:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_3__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_29;
            PyObject *tmp_iter_arg_4;
            PyObject *tmp_called_name_11;
            PyObject *tmp_args_element_name_19;
            PyObject *tmp_expression_name_14;
            PyObject *tmp_args_element_name_20;
            tmp_called_name_11 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_11 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_11 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_11 == NULL)) {
                        tmp_called_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_11 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 151;
                        type_description_2 = "o";
                        goto try_except_handler_10;
                    }
                    Py_INCREF(tmp_called_name_11);
                }
            }

            tmp_expression_name_14 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_14 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_14 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_14 == NULL)) {
                        tmp_expression_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_14 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_11);

                        exception_lineno = 152;
                        type_description_2 = "o";
                        goto try_except_handler_10;
                    }
                    Py_INCREF(tmp_expression_name_14);
                }
            }

            tmp_args_element_name_19 = LOOKUP_ATTRIBUTE(tmp_expression_name_14, mod_consts[85]);
            Py_DECREF(tmp_expression_name_14);
            if (tmp_args_element_name_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_11);

                exception_lineno = 152;
                type_description_2 = "o";
                goto try_except_handler_10;
            }
            tmp_args_element_name_20 = mod_consts[85];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 151;
            {
                PyObject *call_args[] = {tmp_args_element_name_19, tmp_args_element_name_20};
                tmp_iter_arg_4 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_11, call_args);
            }

            Py_DECREF(tmp_called_name_11);
            Py_DECREF(tmp_args_element_name_19);
            if (tmp_iter_arg_4 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 151;
                type_description_2 = "o";
                goto try_except_handler_10;
            }
            tmp_assign_source_29 = MAKE_ITERATOR(tmp_iter_arg_4);
            Py_DECREF(tmp_iter_arg_4);
            if (tmp_assign_source_29 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 151;
                type_description_2 = "o";
                goto try_except_handler_10;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter = tmp_assign_source_29;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_30;
            PyObject *tmp_unpack_10;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter);
            tmp_unpack_10 = tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter;
            tmp_assign_source_30 = UNPACK_NEXT(tmp_unpack_10, 0, 3);
            if (tmp_assign_source_30 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 151;
                goto try_except_handler_11;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1 = tmp_assign_source_30;
        }
        {
            PyObject *tmp_assign_source_31;
            PyObject *tmp_unpack_11;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter);
            tmp_unpack_11 = tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter;
            tmp_assign_source_31 = UNPACK_NEXT(tmp_unpack_11, 1, 3);
            if (tmp_assign_source_31 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 151;
                goto try_except_handler_11;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2 = tmp_assign_source_31;
        }
        {
            PyObject *tmp_assign_source_32;
            PyObject *tmp_unpack_12;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter);
            tmp_unpack_12 = tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter;
            tmp_assign_source_32 = UNPACK_NEXT(tmp_unpack_12, 2, 3);
            if (tmp_assign_source_32 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 151;
                goto try_except_handler_11;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3 = tmp_assign_source_32;
        }
        {
            PyObject *tmp_iterator_name_4;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter);
            tmp_iterator_name_4 = tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_4); assert(HAS_ITERNEXT(tmp_iterator_name_4));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_4)->tp_iternext)(tmp_iterator_name_4);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 151;
                        goto try_except_handler_11;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 151;
                goto try_except_handler_11;
            }
        }
        goto try_end_7;
        // Exception handler code:
        try_except_handler_11:;
        exception_keeper_type_7 = exception_type;
        exception_keeper_value_7 = exception_value;
        exception_keeper_tb_7 = exception_tb;
        exception_keeper_lineno_7 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_7;
        exception_value = exception_keeper_value_7;
        exception_tb = exception_keeper_tb_7;
        exception_lineno = exception_keeper_lineno_7;

        goto try_except_handler_10;
        // End of try:
        try_end_7:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[86], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 151;
            type_description_2 = "o";
            goto try_except_handler_10;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[87], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 151;
            type_description_2 = "o";
            goto try_except_handler_10;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[88], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 151;
            type_description_2 = "o";
            goto try_except_handler_10;
        }
        goto try_end_8;
        // Exception handler code:
        try_except_handler_10:;
        exception_keeper_type_8 = exception_type;
        exception_keeper_value_8 = exception_value;
        exception_keeper_tb_8 = exception_tb;
        exception_keeper_lineno_8 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_8;
        exception_value = exception_keeper_value_8;
        exception_tb = exception_keeper_tb_8;
        exception_lineno = exception_keeper_lineno_8;

        goto frame_exception_exit_2;
        // End of try:
        try_end_8:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_4__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_33;
            PyObject *tmp_iter_arg_5;
            PyObject *tmp_called_name_12;
            PyObject *tmp_args_element_name_21;
            PyObject *tmp_expression_name_15;
            PyObject *tmp_args_element_name_22;
            tmp_called_name_12 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_12 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_12 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_12 == NULL)) {
                        tmp_called_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_12 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 154;
                        type_description_2 = "o";
                        goto try_except_handler_12;
                    }
                    Py_INCREF(tmp_called_name_12);
                }
            }

            tmp_expression_name_15 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_15 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_15 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_15 == NULL)) {
                        tmp_expression_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_15 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_12);

                        exception_lineno = 155;
                        type_description_2 = "o";
                        goto try_except_handler_12;
                    }
                    Py_INCREF(tmp_expression_name_15);
                }
            }

            tmp_args_element_name_21 = LOOKUP_ATTRIBUTE(tmp_expression_name_15, mod_consts[89]);
            Py_DECREF(tmp_expression_name_15);
            if (tmp_args_element_name_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_12);

                exception_lineno = 155;
                type_description_2 = "o";
                goto try_except_handler_12;
            }
            tmp_args_element_name_22 = mod_consts[90];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 154;
            {
                PyObject *call_args[] = {tmp_args_element_name_21, tmp_args_element_name_22};
                tmp_iter_arg_5 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_12, call_args);
            }

            Py_DECREF(tmp_called_name_12);
            Py_DECREF(tmp_args_element_name_21);
            if (tmp_iter_arg_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 154;
                type_description_2 = "o";
                goto try_except_handler_12;
            }
            tmp_assign_source_33 = MAKE_ITERATOR(tmp_iter_arg_5);
            Py_DECREF(tmp_iter_arg_5);
            if (tmp_assign_source_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 154;
                type_description_2 = "o";
                goto try_except_handler_12;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter = tmp_assign_source_33;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_34;
            PyObject *tmp_unpack_13;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter);
            tmp_unpack_13 = tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter;
            tmp_assign_source_34 = UNPACK_NEXT(tmp_unpack_13, 0, 3);
            if (tmp_assign_source_34 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 154;
                goto try_except_handler_13;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1 = tmp_assign_source_34;
        }
        {
            PyObject *tmp_assign_source_35;
            PyObject *tmp_unpack_14;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter);
            tmp_unpack_14 = tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter;
            tmp_assign_source_35 = UNPACK_NEXT(tmp_unpack_14, 1, 3);
            if (tmp_assign_source_35 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 154;
                goto try_except_handler_13;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2 = tmp_assign_source_35;
        }
        {
            PyObject *tmp_assign_source_36;
            PyObject *tmp_unpack_15;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter);
            tmp_unpack_15 = tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter;
            tmp_assign_source_36 = UNPACK_NEXT(tmp_unpack_15, 2, 3);
            if (tmp_assign_source_36 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 154;
                goto try_except_handler_13;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3 = tmp_assign_source_36;
        }
        {
            PyObject *tmp_iterator_name_5;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter);
            tmp_iterator_name_5 = tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_5); assert(HAS_ITERNEXT(tmp_iterator_name_5));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_5)->tp_iternext)(tmp_iterator_name_5);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 154;
                        goto try_except_handler_13;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 154;
                goto try_except_handler_13;
            }
        }
        goto try_end_9;
        // Exception handler code:
        try_except_handler_13:;
        exception_keeper_type_9 = exception_type;
        exception_keeper_value_9 = exception_value;
        exception_keeper_tb_9 = exception_tb;
        exception_keeper_lineno_9 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_9;
        exception_value = exception_keeper_value_9;
        exception_tb = exception_keeper_tb_9;
        exception_lineno = exception_keeper_lineno_9;

        goto try_except_handler_12;
        // End of try:
        try_end_9:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[91], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 154;
            type_description_2 = "o";
            goto try_except_handler_12;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[92], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 154;
            type_description_2 = "o";
            goto try_except_handler_12;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[93], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 154;
            type_description_2 = "o";
            goto try_except_handler_12;
        }
        goto try_end_10;
        // Exception handler code:
        try_except_handler_12:;
        exception_keeper_type_10 = exception_type;
        exception_keeper_value_10 = exception_value;
        exception_keeper_tb_10 = exception_tb;
        exception_keeper_lineno_10 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_10;
        exception_value = exception_keeper_value_10;
        exception_tb = exception_keeper_tb_10;
        exception_lineno = exception_keeper_lineno_10;

        goto frame_exception_exit_2;
        // End of try:
        try_end_10:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_5__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_37;
            PyObject *tmp_iter_arg_6;
            PyObject *tmp_called_name_13;
            PyObject *tmp_args_element_name_23;
            PyObject *tmp_expression_name_16;
            PyObject *tmp_args_element_name_24;
            tmp_called_name_13 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_13 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_13 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_13 == NULL)) {
                        tmp_called_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_13 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 156;
                        type_description_2 = "o";
                        goto try_except_handler_14;
                    }
                    Py_INCREF(tmp_called_name_13);
                }
            }

            tmp_expression_name_16 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_16 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_16 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_16 == NULL)) {
                        tmp_expression_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_16 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_13);

                        exception_lineno = 157;
                        type_description_2 = "o";
                        goto try_except_handler_14;
                    }
                    Py_INCREF(tmp_expression_name_16);
                }
            }

            tmp_args_element_name_23 = LOOKUP_ATTRIBUTE(tmp_expression_name_16, mod_consts[94]);
            Py_DECREF(tmp_expression_name_16);
            if (tmp_args_element_name_23 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_13);

                exception_lineno = 157;
                type_description_2 = "o";
                goto try_except_handler_14;
            }
            tmp_args_element_name_24 = mod_consts[95];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 156;
            {
                PyObject *call_args[] = {tmp_args_element_name_23, tmp_args_element_name_24};
                tmp_iter_arg_6 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_13, call_args);
            }

            Py_DECREF(tmp_called_name_13);
            Py_DECREF(tmp_args_element_name_23);
            if (tmp_iter_arg_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 156;
                type_description_2 = "o";
                goto try_except_handler_14;
            }
            tmp_assign_source_37 = MAKE_ITERATOR(tmp_iter_arg_6);
            Py_DECREF(tmp_iter_arg_6);
            if (tmp_assign_source_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 156;
                type_description_2 = "o";
                goto try_except_handler_14;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter = tmp_assign_source_37;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_38;
            PyObject *tmp_unpack_16;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter);
            tmp_unpack_16 = tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter;
            tmp_assign_source_38 = UNPACK_NEXT(tmp_unpack_16, 0, 3);
            if (tmp_assign_source_38 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 156;
                goto try_except_handler_15;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1 = tmp_assign_source_38;
        }
        {
            PyObject *tmp_assign_source_39;
            PyObject *tmp_unpack_17;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter);
            tmp_unpack_17 = tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter;
            tmp_assign_source_39 = UNPACK_NEXT(tmp_unpack_17, 1, 3);
            if (tmp_assign_source_39 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 156;
                goto try_except_handler_15;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2 = tmp_assign_source_39;
        }
        {
            PyObject *tmp_assign_source_40;
            PyObject *tmp_unpack_18;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter);
            tmp_unpack_18 = tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter;
            tmp_assign_source_40 = UNPACK_NEXT(tmp_unpack_18, 2, 3);
            if (tmp_assign_source_40 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 156;
                goto try_except_handler_15;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3 = tmp_assign_source_40;
        }
        {
            PyObject *tmp_iterator_name_6;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter);
            tmp_iterator_name_6 = tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_6); assert(HAS_ITERNEXT(tmp_iterator_name_6));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_6)->tp_iternext)(tmp_iterator_name_6);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 156;
                        goto try_except_handler_15;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 156;
                goto try_except_handler_15;
            }
        }
        goto try_end_11;
        // Exception handler code:
        try_except_handler_15:;
        exception_keeper_type_11 = exception_type;
        exception_keeper_value_11 = exception_value;
        exception_keeper_tb_11 = exception_tb;
        exception_keeper_lineno_11 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_11;
        exception_value = exception_keeper_value_11;
        exception_tb = exception_keeper_tb_11;
        exception_lineno = exception_keeper_lineno_11;

        goto try_except_handler_14;
        // End of try:
        try_end_11:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[96], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 156;
            type_description_2 = "o";
            goto try_except_handler_14;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[97], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 156;
            type_description_2 = "o";
            goto try_except_handler_14;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[98], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 156;
            type_description_2 = "o";
            goto try_except_handler_14;
        }
        goto try_end_12;
        // Exception handler code:
        try_except_handler_14:;
        exception_keeper_type_12 = exception_type;
        exception_keeper_value_12 = exception_value;
        exception_keeper_tb_12 = exception_tb;
        exception_keeper_lineno_12 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_12;
        exception_value = exception_keeper_value_12;
        exception_tb = exception_keeper_tb_12;
        exception_lineno = exception_keeper_lineno_12;

        goto frame_exception_exit_2;
        // End of try:
        try_end_12:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_6__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_41;
            PyObject *tmp_iter_arg_7;
            PyObject *tmp_called_name_14;
            PyObject *tmp_args_element_name_25;
            PyObject *tmp_expression_name_17;
            PyObject *tmp_args_element_name_26;
            tmp_called_name_14 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_14 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_14 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_14 == NULL)) {
                        tmp_called_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_14 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 158;
                        type_description_2 = "o";
                        goto try_except_handler_16;
                    }
                    Py_INCREF(tmp_called_name_14);
                }
            }

            tmp_expression_name_17 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_17 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_17 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_17 == NULL)) {
                        tmp_expression_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_17 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_14);

                        exception_lineno = 158;
                        type_description_2 = "o";
                        goto try_except_handler_16;
                    }
                    Py_INCREF(tmp_expression_name_17);
                }
            }

            tmp_args_element_name_25 = LOOKUP_ATTRIBUTE(tmp_expression_name_17, mod_consts[99]);
            Py_DECREF(tmp_expression_name_17);
            if (tmp_args_element_name_25 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_14);

                exception_lineno = 158;
                type_description_2 = "o";
                goto try_except_handler_16;
            }
            tmp_args_element_name_26 = mod_consts[100];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 158;
            {
                PyObject *call_args[] = {tmp_args_element_name_25, tmp_args_element_name_26};
                tmp_iter_arg_7 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_14, call_args);
            }

            Py_DECREF(tmp_called_name_14);
            Py_DECREF(tmp_args_element_name_25);
            if (tmp_iter_arg_7 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 158;
                type_description_2 = "o";
                goto try_except_handler_16;
            }
            tmp_assign_source_41 = MAKE_ITERATOR(tmp_iter_arg_7);
            Py_DECREF(tmp_iter_arg_7);
            if (tmp_assign_source_41 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 158;
                type_description_2 = "o";
                goto try_except_handler_16;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter = tmp_assign_source_41;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_42;
            PyObject *tmp_unpack_19;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter);
            tmp_unpack_19 = tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter;
            tmp_assign_source_42 = UNPACK_NEXT(tmp_unpack_19, 0, 3);
            if (tmp_assign_source_42 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 158;
                goto try_except_handler_17;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1 = tmp_assign_source_42;
        }
        {
            PyObject *tmp_assign_source_43;
            PyObject *tmp_unpack_20;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter);
            tmp_unpack_20 = tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter;
            tmp_assign_source_43 = UNPACK_NEXT(tmp_unpack_20, 1, 3);
            if (tmp_assign_source_43 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 158;
                goto try_except_handler_17;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2 = tmp_assign_source_43;
        }
        {
            PyObject *tmp_assign_source_44;
            PyObject *tmp_unpack_21;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter);
            tmp_unpack_21 = tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter;
            tmp_assign_source_44 = UNPACK_NEXT(tmp_unpack_21, 2, 3);
            if (tmp_assign_source_44 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 158;
                goto try_except_handler_17;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3 = tmp_assign_source_44;
        }
        {
            PyObject *tmp_iterator_name_7;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter);
            tmp_iterator_name_7 = tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_7); assert(HAS_ITERNEXT(tmp_iterator_name_7));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_7)->tp_iternext)(tmp_iterator_name_7);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 158;
                        goto try_except_handler_17;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 158;
                goto try_except_handler_17;
            }
        }
        goto try_end_13;
        // Exception handler code:
        try_except_handler_17:;
        exception_keeper_type_13 = exception_type;
        exception_keeper_value_13 = exception_value;
        exception_keeper_tb_13 = exception_tb;
        exception_keeper_lineno_13 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_13;
        exception_value = exception_keeper_value_13;
        exception_tb = exception_keeper_tb_13;
        exception_lineno = exception_keeper_lineno_13;

        goto try_except_handler_16;
        // End of try:
        try_end_13:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[101], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;
            type_description_2 = "o";
            goto try_except_handler_16;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[102], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;
            type_description_2 = "o";
            goto try_except_handler_16;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[103], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;
            type_description_2 = "o";
            goto try_except_handler_16;
        }
        goto try_end_14;
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

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_14;
        exception_value = exception_keeper_value_14;
        exception_tb = exception_keeper_tb_14;
        exception_lineno = exception_keeper_lineno_14;

        goto frame_exception_exit_2;
        // End of try:
        try_end_14:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_7__element_3 = NULL;
        {
            PyObject *tmp_called_name_15;
            PyObject *tmp_args_element_name_27;
            PyObject *tmp_expression_name_18;
            PyObject *tmp_args_element_name_28;
            tmp_called_name_15 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[17]);

            if (tmp_called_name_15 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_15 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[17]);

                    if (unlikely(tmp_called_name_15 == NULL)) {
                        tmp_called_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
                    }

                    if (tmp_called_name_15 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 159;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_15);
                }
            }

            tmp_expression_name_18 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_18 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_18 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_18 == NULL)) {
                        tmp_expression_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_18 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_15);

                        exception_lineno = 159;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_18);
                }
            }

            tmp_args_element_name_27 = LOOKUP_ATTRIBUTE(tmp_expression_name_18, mod_consts[104]);
            Py_DECREF(tmp_expression_name_18);
            if (tmp_args_element_name_27 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_15);

                exception_lineno = 159;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_28 = mod_consts[104];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 159;
            {
                PyObject *call_args[] = {tmp_args_element_name_27, tmp_args_element_name_28};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_15, call_args);
            }

            Py_DECREF(tmp_called_name_15);
            Py_DECREF(tmp_args_element_name_27);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 159;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[105], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 159;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_16;
            PyObject *tmp_args_element_name_29;
            PyObject *tmp_expression_name_19;
            PyObject *tmp_args_element_name_30;
            tmp_called_name_16 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[18]);

            if (tmp_called_name_16 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_16 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[18]);

                    if (unlikely(tmp_called_name_16 == NULL)) {
                        tmp_called_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[18]);
                    }

                    if (tmp_called_name_16 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 160;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_16);
                }
            }

            tmp_expression_name_19 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_19 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_19 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_19 == NULL)) {
                        tmp_expression_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_19 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_16);

                        exception_lineno = 160;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_19);
                }
            }

            tmp_args_element_name_29 = LOOKUP_ATTRIBUTE(tmp_expression_name_19, mod_consts[104]);
            Py_DECREF(tmp_expression_name_19);
            if (tmp_args_element_name_29 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_16);

                exception_lineno = 160;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_30 = mod_consts[104];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 160;
            {
                PyObject *call_args[] = {tmp_args_element_name_29, tmp_args_element_name_30};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_16, call_args);
            }

            Py_DECREF(tmp_called_name_16);
            Py_DECREF(tmp_args_element_name_29);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 160;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[106], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 160;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_45;
            PyObject *tmp_iter_arg_8;
            PyObject *tmp_called_name_17;
            PyObject *tmp_args_element_name_31;
            PyObject *tmp_expression_name_20;
            PyObject *tmp_args_element_name_32;
            tmp_called_name_17 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_17 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_17 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_17 == NULL)) {
                        tmp_called_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_17 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 163;
                        type_description_2 = "o";
                        goto try_except_handler_18;
                    }
                    Py_INCREF(tmp_called_name_17);
                }
            }

            tmp_expression_name_20 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_20 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_20 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_20 == NULL)) {
                        tmp_expression_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_20 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_17);

                        exception_lineno = 163;
                        type_description_2 = "o";
                        goto try_except_handler_18;
                    }
                    Py_INCREF(tmp_expression_name_20);
                }
            }

            tmp_args_element_name_31 = LOOKUP_ATTRIBUTE(tmp_expression_name_20, mod_consts[107]);
            Py_DECREF(tmp_expression_name_20);
            if (tmp_args_element_name_31 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_17);

                exception_lineno = 163;
                type_description_2 = "o";
                goto try_except_handler_18;
            }
            tmp_args_element_name_32 = mod_consts[108];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 163;
            {
                PyObject *call_args[] = {tmp_args_element_name_31, tmp_args_element_name_32};
                tmp_iter_arg_8 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_17, call_args);
            }

            Py_DECREF(tmp_called_name_17);
            Py_DECREF(tmp_args_element_name_31);
            if (tmp_iter_arg_8 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 163;
                type_description_2 = "o";
                goto try_except_handler_18;
            }
            tmp_assign_source_45 = MAKE_ITERATOR(tmp_iter_arg_8);
            Py_DECREF(tmp_iter_arg_8);
            if (tmp_assign_source_45 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 163;
                type_description_2 = "o";
                goto try_except_handler_18;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter = tmp_assign_source_45;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_46;
            PyObject *tmp_unpack_22;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter);
            tmp_unpack_22 = tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter;
            tmp_assign_source_46 = UNPACK_NEXT(tmp_unpack_22, 0, 3);
            if (tmp_assign_source_46 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 163;
                goto try_except_handler_19;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1 = tmp_assign_source_46;
        }
        {
            PyObject *tmp_assign_source_47;
            PyObject *tmp_unpack_23;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter);
            tmp_unpack_23 = tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter;
            tmp_assign_source_47 = UNPACK_NEXT(tmp_unpack_23, 1, 3);
            if (tmp_assign_source_47 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 163;
                goto try_except_handler_19;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2 = tmp_assign_source_47;
        }
        {
            PyObject *tmp_assign_source_48;
            PyObject *tmp_unpack_24;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter);
            tmp_unpack_24 = tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter;
            tmp_assign_source_48 = UNPACK_NEXT(tmp_unpack_24, 2, 3);
            if (tmp_assign_source_48 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 163;
                goto try_except_handler_19;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3 = tmp_assign_source_48;
        }
        {
            PyObject *tmp_iterator_name_8;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter);
            tmp_iterator_name_8 = tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_8); assert(HAS_ITERNEXT(tmp_iterator_name_8));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_8)->tp_iternext)(tmp_iterator_name_8);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 163;
                        goto try_except_handler_19;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 163;
                goto try_except_handler_19;
            }
        }
        goto try_end_15;
        // Exception handler code:
        try_except_handler_19:;
        exception_keeper_type_15 = exception_type;
        exception_keeper_value_15 = exception_value;
        exception_keeper_tb_15 = exception_tb;
        exception_keeper_lineno_15 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_15;
        exception_value = exception_keeper_value_15;
        exception_tb = exception_keeper_tb_15;
        exception_lineno = exception_keeper_lineno_15;

        goto try_except_handler_18;
        // End of try:
        try_end_15:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[109], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;
            type_description_2 = "o";
            goto try_except_handler_18;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[110], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;
            type_description_2 = "o";
            goto try_except_handler_18;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[111], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;
            type_description_2 = "o";
            goto try_except_handler_18;
        }
        goto try_end_16;
        // Exception handler code:
        try_except_handler_18:;
        exception_keeper_type_16 = exception_type;
        exception_keeper_value_16 = exception_value;
        exception_keeper_tb_16 = exception_tb;
        exception_keeper_lineno_16 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_16;
        exception_value = exception_keeper_value_16;
        exception_tb = exception_keeper_tb_16;
        exception_lineno = exception_keeper_lineno_16;

        goto frame_exception_exit_2;
        // End of try:
        try_end_16:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_8__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_49;
            PyObject *tmp_iter_arg_9;
            PyObject *tmp_called_name_18;
            PyObject *tmp_args_element_name_33;
            PyObject *tmp_expression_name_21;
            PyObject *tmp_args_element_name_34;
            tmp_called_name_18 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_18 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_18 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_18 == NULL)) {
                        tmp_called_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_18 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 164;
                        type_description_2 = "o";
                        goto try_except_handler_20;
                    }
                    Py_INCREF(tmp_called_name_18);
                }
            }

            tmp_expression_name_21 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_21 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_21 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_21 == NULL)) {
                        tmp_expression_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_21 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_18);

                        exception_lineno = 165;
                        type_description_2 = "o";
                        goto try_except_handler_20;
                    }
                    Py_INCREF(tmp_expression_name_21);
                }
            }

            tmp_args_element_name_33 = LOOKUP_ATTRIBUTE(tmp_expression_name_21, mod_consts[112]);
            Py_DECREF(tmp_expression_name_21);
            if (tmp_args_element_name_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_18);

                exception_lineno = 165;
                type_description_2 = "o";
                goto try_except_handler_20;
            }
            tmp_args_element_name_34 = mod_consts[113];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 164;
            {
                PyObject *call_args[] = {tmp_args_element_name_33, tmp_args_element_name_34};
                tmp_iter_arg_9 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_18, call_args);
            }

            Py_DECREF(tmp_called_name_18);
            Py_DECREF(tmp_args_element_name_33);
            if (tmp_iter_arg_9 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 164;
                type_description_2 = "o";
                goto try_except_handler_20;
            }
            tmp_assign_source_49 = MAKE_ITERATOR(tmp_iter_arg_9);
            Py_DECREF(tmp_iter_arg_9);
            if (tmp_assign_source_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 164;
                type_description_2 = "o";
                goto try_except_handler_20;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter = tmp_assign_source_49;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_50;
            PyObject *tmp_unpack_25;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter);
            tmp_unpack_25 = tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter;
            tmp_assign_source_50 = UNPACK_NEXT(tmp_unpack_25, 0, 3);
            if (tmp_assign_source_50 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 164;
                goto try_except_handler_21;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1 = tmp_assign_source_50;
        }
        {
            PyObject *tmp_assign_source_51;
            PyObject *tmp_unpack_26;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter);
            tmp_unpack_26 = tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter;
            tmp_assign_source_51 = UNPACK_NEXT(tmp_unpack_26, 1, 3);
            if (tmp_assign_source_51 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 164;
                goto try_except_handler_21;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2 = tmp_assign_source_51;
        }
        {
            PyObject *tmp_assign_source_52;
            PyObject *tmp_unpack_27;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter);
            tmp_unpack_27 = tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter;
            tmp_assign_source_52 = UNPACK_NEXT(tmp_unpack_27, 2, 3);
            if (tmp_assign_source_52 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 164;
                goto try_except_handler_21;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3 = tmp_assign_source_52;
        }
        {
            PyObject *tmp_iterator_name_9;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter);
            tmp_iterator_name_9 = tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_9); assert(HAS_ITERNEXT(tmp_iterator_name_9));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_9)->tp_iternext)(tmp_iterator_name_9);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 164;
                        goto try_except_handler_21;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 164;
                goto try_except_handler_21;
            }
        }
        goto try_end_17;
        // Exception handler code:
        try_except_handler_21:;
        exception_keeper_type_17 = exception_type;
        exception_keeper_value_17 = exception_value;
        exception_keeper_tb_17 = exception_tb;
        exception_keeper_lineno_17 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_17;
        exception_value = exception_keeper_value_17;
        exception_tb = exception_keeper_tb_17;
        exception_lineno = exception_keeper_lineno_17;

        goto try_except_handler_20;
        // End of try:
        try_end_17:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[114], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 164;
            type_description_2 = "o";
            goto try_except_handler_20;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[115], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 164;
            type_description_2 = "o";
            goto try_except_handler_20;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[116], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 164;
            type_description_2 = "o";
            goto try_except_handler_20;
        }
        goto try_end_18;
        // Exception handler code:
        try_except_handler_20:;
        exception_keeper_type_18 = exception_type;
        exception_keeper_value_18 = exception_value;
        exception_keeper_tb_18 = exception_tb;
        exception_keeper_lineno_18 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_18;
        exception_value = exception_keeper_value_18;
        exception_tb = exception_keeper_tb_18;
        exception_lineno = exception_keeper_lineno_18;

        goto frame_exception_exit_2;
        // End of try:
        try_end_18:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_9__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_53;
            PyObject *tmp_iter_arg_10;
            PyObject *tmp_called_name_19;
            PyObject *tmp_args_element_name_35;
            PyObject *tmp_expression_name_22;
            PyObject *tmp_args_element_name_36;
            tmp_called_name_19 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_19 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_19 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_19 == NULL)) {
                        tmp_called_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_19 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 166;
                        type_description_2 = "o";
                        goto try_except_handler_22;
                    }
                    Py_INCREF(tmp_called_name_19);
                }
            }

            tmp_expression_name_22 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_22 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_22 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_22 == NULL)) {
                        tmp_expression_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_22 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_19);

                        exception_lineno = 167;
                        type_description_2 = "o";
                        goto try_except_handler_22;
                    }
                    Py_INCREF(tmp_expression_name_22);
                }
            }

            tmp_args_element_name_35 = LOOKUP_ATTRIBUTE(tmp_expression_name_22, mod_consts[117]);
            Py_DECREF(tmp_expression_name_22);
            if (tmp_args_element_name_35 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_19);

                exception_lineno = 167;
                type_description_2 = "o";
                goto try_except_handler_22;
            }
            tmp_args_element_name_36 = mod_consts[118];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 166;
            {
                PyObject *call_args[] = {tmp_args_element_name_35, tmp_args_element_name_36};
                tmp_iter_arg_10 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_19, call_args);
            }

            Py_DECREF(tmp_called_name_19);
            Py_DECREF(tmp_args_element_name_35);
            if (tmp_iter_arg_10 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 166;
                type_description_2 = "o";
                goto try_except_handler_22;
            }
            tmp_assign_source_53 = MAKE_ITERATOR(tmp_iter_arg_10);
            Py_DECREF(tmp_iter_arg_10);
            if (tmp_assign_source_53 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 166;
                type_description_2 = "o";
                goto try_except_handler_22;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter = tmp_assign_source_53;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_54;
            PyObject *tmp_unpack_28;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter);
            tmp_unpack_28 = tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter;
            tmp_assign_source_54 = UNPACK_NEXT(tmp_unpack_28, 0, 3);
            if (tmp_assign_source_54 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 166;
                goto try_except_handler_23;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1 = tmp_assign_source_54;
        }
        {
            PyObject *tmp_assign_source_55;
            PyObject *tmp_unpack_29;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter);
            tmp_unpack_29 = tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter;
            tmp_assign_source_55 = UNPACK_NEXT(tmp_unpack_29, 1, 3);
            if (tmp_assign_source_55 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 166;
                goto try_except_handler_23;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2 = tmp_assign_source_55;
        }
        {
            PyObject *tmp_assign_source_56;
            PyObject *tmp_unpack_30;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter);
            tmp_unpack_30 = tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter;
            tmp_assign_source_56 = UNPACK_NEXT(tmp_unpack_30, 2, 3);
            if (tmp_assign_source_56 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 166;
                goto try_except_handler_23;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3 = tmp_assign_source_56;
        }
        {
            PyObject *tmp_iterator_name_10;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter);
            tmp_iterator_name_10 = tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_10); assert(HAS_ITERNEXT(tmp_iterator_name_10));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_10)->tp_iternext)(tmp_iterator_name_10);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 166;
                        goto try_except_handler_23;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 166;
                goto try_except_handler_23;
            }
        }
        goto try_end_19;
        // Exception handler code:
        try_except_handler_23:;
        exception_keeper_type_19 = exception_type;
        exception_keeper_value_19 = exception_value;
        exception_keeper_tb_19 = exception_tb;
        exception_keeper_lineno_19 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_19;
        exception_value = exception_keeper_value_19;
        exception_tb = exception_keeper_tb_19;
        exception_lineno = exception_keeper_lineno_19;

        goto try_except_handler_22;
        // End of try:
        try_end_19:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[119], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 166;
            type_description_2 = "o";
            goto try_except_handler_22;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[120], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 166;
            type_description_2 = "o";
            goto try_except_handler_22;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[121], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 166;
            type_description_2 = "o";
            goto try_except_handler_22;
        }
        goto try_end_20;
        // Exception handler code:
        try_except_handler_22:;
        exception_keeper_type_20 = exception_type;
        exception_keeper_value_20 = exception_value;
        exception_keeper_tb_20 = exception_tb;
        exception_keeper_lineno_20 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_20;
        exception_value = exception_keeper_value_20;
        exception_tb = exception_keeper_tb_20;
        exception_lineno = exception_keeper_lineno_20;

        goto frame_exception_exit_2;
        // End of try:
        try_end_20:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_10__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_57;
            PyObject *tmp_iter_arg_11;
            PyObject *tmp_called_name_20;
            PyObject *tmp_args_element_name_37;
            PyObject *tmp_expression_name_23;
            PyObject *tmp_args_element_name_38;
            tmp_called_name_20 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_20 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_20 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_20 == NULL)) {
                        tmp_called_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_20 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 168;
                        type_description_2 = "o";
                        goto try_except_handler_24;
                    }
                    Py_INCREF(tmp_called_name_20);
                }
            }

            tmp_expression_name_23 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_23 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_23 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_23 == NULL)) {
                        tmp_expression_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_23 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_20);

                        exception_lineno = 168;
                        type_description_2 = "o";
                        goto try_except_handler_24;
                    }
                    Py_INCREF(tmp_expression_name_23);
                }
            }

            tmp_args_element_name_37 = LOOKUP_ATTRIBUTE(tmp_expression_name_23, mod_consts[122]);
            Py_DECREF(tmp_expression_name_23);
            if (tmp_args_element_name_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_20);

                exception_lineno = 168;
                type_description_2 = "o";
                goto try_except_handler_24;
            }
            tmp_args_element_name_38 = mod_consts[123];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 168;
            {
                PyObject *call_args[] = {tmp_args_element_name_37, tmp_args_element_name_38};
                tmp_iter_arg_11 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_20, call_args);
            }

            Py_DECREF(tmp_called_name_20);
            Py_DECREF(tmp_args_element_name_37);
            if (tmp_iter_arg_11 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 168;
                type_description_2 = "o";
                goto try_except_handler_24;
            }
            tmp_assign_source_57 = MAKE_ITERATOR(tmp_iter_arg_11);
            Py_DECREF(tmp_iter_arg_11);
            if (tmp_assign_source_57 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 168;
                type_description_2 = "o";
                goto try_except_handler_24;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter = tmp_assign_source_57;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_58;
            PyObject *tmp_unpack_31;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter);
            tmp_unpack_31 = tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter;
            tmp_assign_source_58 = UNPACK_NEXT(tmp_unpack_31, 0, 3);
            if (tmp_assign_source_58 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 168;
                goto try_except_handler_25;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1 = tmp_assign_source_58;
        }
        {
            PyObject *tmp_assign_source_59;
            PyObject *tmp_unpack_32;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter);
            tmp_unpack_32 = tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter;
            tmp_assign_source_59 = UNPACK_NEXT(tmp_unpack_32, 1, 3);
            if (tmp_assign_source_59 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 168;
                goto try_except_handler_25;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2 = tmp_assign_source_59;
        }
        {
            PyObject *tmp_assign_source_60;
            PyObject *tmp_unpack_33;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter);
            tmp_unpack_33 = tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter;
            tmp_assign_source_60 = UNPACK_NEXT(tmp_unpack_33, 2, 3);
            if (tmp_assign_source_60 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 168;
                goto try_except_handler_25;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3 = tmp_assign_source_60;
        }
        {
            PyObject *tmp_iterator_name_11;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter);
            tmp_iterator_name_11 = tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_11); assert(HAS_ITERNEXT(tmp_iterator_name_11));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_11)->tp_iternext)(tmp_iterator_name_11);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 168;
                        goto try_except_handler_25;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 168;
                goto try_except_handler_25;
            }
        }
        goto try_end_21;
        // Exception handler code:
        try_except_handler_25:;
        exception_keeper_type_21 = exception_type;
        exception_keeper_value_21 = exception_value;
        exception_keeper_tb_21 = exception_tb;
        exception_keeper_lineno_21 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_21;
        exception_value = exception_keeper_value_21;
        exception_tb = exception_keeper_tb_21;
        exception_lineno = exception_keeper_lineno_21;

        goto try_except_handler_24;
        // End of try:
        try_end_21:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[124], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;
            type_description_2 = "o";
            goto try_except_handler_24;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[125], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;
            type_description_2 = "o";
            goto try_except_handler_24;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[126], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;
            type_description_2 = "o";
            goto try_except_handler_24;
        }
        goto try_end_22;
        // Exception handler code:
        try_except_handler_24:;
        exception_keeper_type_22 = exception_type;
        exception_keeper_value_22 = exception_value;
        exception_keeper_tb_22 = exception_tb;
        exception_keeper_lineno_22 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_22;
        exception_value = exception_keeper_value_22;
        exception_tb = exception_keeper_tb_22;
        exception_lineno = exception_keeper_lineno_22;

        goto frame_exception_exit_2;
        // End of try:
        try_end_22:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_11__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_61;
            PyObject *tmp_iter_arg_12;
            PyObject *tmp_called_name_21;
            PyObject *tmp_args_element_name_39;
            PyObject *tmp_expression_name_24;
            PyObject *tmp_args_element_name_40;
            tmp_called_name_21 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_21 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_21 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_21 == NULL)) {
                        tmp_called_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_21 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 169;
                        type_description_2 = "o";
                        goto try_except_handler_26;
                    }
                    Py_INCREF(tmp_called_name_21);
                }
            }

            tmp_expression_name_24 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_24 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_24 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_24 == NULL)) {
                        tmp_expression_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_24 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_21);

                        exception_lineno = 169;
                        type_description_2 = "o";
                        goto try_except_handler_26;
                    }
                    Py_INCREF(tmp_expression_name_24);
                }
            }

            tmp_args_element_name_39 = LOOKUP_ATTRIBUTE(tmp_expression_name_24, mod_consts[127]);
            Py_DECREF(tmp_expression_name_24);
            if (tmp_args_element_name_39 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_21);

                exception_lineno = 169;
                type_description_2 = "o";
                goto try_except_handler_26;
            }
            tmp_args_element_name_40 = mod_consts[128];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 169;
            {
                PyObject *call_args[] = {tmp_args_element_name_39, tmp_args_element_name_40};
                tmp_iter_arg_12 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_21, call_args);
            }

            Py_DECREF(tmp_called_name_21);
            Py_DECREF(tmp_args_element_name_39);
            if (tmp_iter_arg_12 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 169;
                type_description_2 = "o";
                goto try_except_handler_26;
            }
            tmp_assign_source_61 = MAKE_ITERATOR(tmp_iter_arg_12);
            Py_DECREF(tmp_iter_arg_12);
            if (tmp_assign_source_61 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 169;
                type_description_2 = "o";
                goto try_except_handler_26;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter = tmp_assign_source_61;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_62;
            PyObject *tmp_unpack_34;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter);
            tmp_unpack_34 = tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter;
            tmp_assign_source_62 = UNPACK_NEXT(tmp_unpack_34, 0, 3);
            if (tmp_assign_source_62 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 169;
                goto try_except_handler_27;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1 = tmp_assign_source_62;
        }
        {
            PyObject *tmp_assign_source_63;
            PyObject *tmp_unpack_35;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter);
            tmp_unpack_35 = tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter;
            tmp_assign_source_63 = UNPACK_NEXT(tmp_unpack_35, 1, 3);
            if (tmp_assign_source_63 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 169;
                goto try_except_handler_27;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2 = tmp_assign_source_63;
        }
        {
            PyObject *tmp_assign_source_64;
            PyObject *tmp_unpack_36;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter);
            tmp_unpack_36 = tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter;
            tmp_assign_source_64 = UNPACK_NEXT(tmp_unpack_36, 2, 3);
            if (tmp_assign_source_64 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 169;
                goto try_except_handler_27;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3 = tmp_assign_source_64;
        }
        {
            PyObject *tmp_iterator_name_12;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter);
            tmp_iterator_name_12 = tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_12); assert(HAS_ITERNEXT(tmp_iterator_name_12));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_12)->tp_iternext)(tmp_iterator_name_12);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 169;
                        goto try_except_handler_27;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 169;
                goto try_except_handler_27;
            }
        }
        goto try_end_23;
        // Exception handler code:
        try_except_handler_27:;
        exception_keeper_type_23 = exception_type;
        exception_keeper_value_23 = exception_value;
        exception_keeper_tb_23 = exception_tb;
        exception_keeper_lineno_23 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_23;
        exception_value = exception_keeper_value_23;
        exception_tb = exception_keeper_tb_23;
        exception_lineno = exception_keeper_lineno_23;

        goto try_except_handler_26;
        // End of try:
        try_end_23:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[129], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 169;
            type_description_2 = "o";
            goto try_except_handler_26;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[130], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 169;
            type_description_2 = "o";
            goto try_except_handler_26;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[131], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 169;
            type_description_2 = "o";
            goto try_except_handler_26;
        }
        goto try_end_24;
        // Exception handler code:
        try_except_handler_26:;
        exception_keeper_type_24 = exception_type;
        exception_keeper_value_24 = exception_value;
        exception_keeper_tb_24 = exception_tb;
        exception_keeper_lineno_24 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_24;
        exception_value = exception_keeper_value_24;
        exception_tb = exception_keeper_tb_24;
        exception_lineno = exception_keeper_lineno_24;

        goto frame_exception_exit_2;
        // End of try:
        try_end_24:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_12__element_3 = NULL;
        // Tried code:
        {
            PyObject *tmp_assign_source_65;
            PyObject *tmp_iter_arg_13;
            PyObject *tmp_called_name_22;
            PyObject *tmp_args_element_name_41;
            PyObject *tmp_expression_name_25;
            PyObject *tmp_args_element_name_42;
            tmp_called_name_22 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[38]);

            if (tmp_called_name_22 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_22 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[38]);

                    if (unlikely(tmp_called_name_22 == NULL)) {
                        tmp_called_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
                    }

                    if (tmp_called_name_22 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 170;
                        type_description_2 = "o";
                        goto try_except_handler_28;
                    }
                    Py_INCREF(tmp_called_name_22);
                }
            }

            tmp_expression_name_25 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_25 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_25 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_25 == NULL)) {
                        tmp_expression_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_25 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_22);

                        exception_lineno = 170;
                        type_description_2 = "o";
                        goto try_except_handler_28;
                    }
                    Py_INCREF(tmp_expression_name_25);
                }
            }

            tmp_args_element_name_41 = LOOKUP_ATTRIBUTE(tmp_expression_name_25, mod_consts[132]);
            Py_DECREF(tmp_expression_name_25);
            if (tmp_args_element_name_41 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_22);

                exception_lineno = 170;
                type_description_2 = "o";
                goto try_except_handler_28;
            }
            tmp_args_element_name_42 = mod_consts[133];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 170;
            {
                PyObject *call_args[] = {tmp_args_element_name_41, tmp_args_element_name_42};
                tmp_iter_arg_13 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_22, call_args);
            }

            Py_DECREF(tmp_called_name_22);
            Py_DECREF(tmp_args_element_name_41);
            if (tmp_iter_arg_13 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 170;
                type_description_2 = "o";
                goto try_except_handler_28;
            }
            tmp_assign_source_65 = MAKE_ITERATOR(tmp_iter_arg_13);
            Py_DECREF(tmp_iter_arg_13);
            if (tmp_assign_source_65 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 170;
                type_description_2 = "o";
                goto try_except_handler_28;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter = tmp_assign_source_65;
        }
        // Tried code:
        {
            PyObject *tmp_assign_source_66;
            PyObject *tmp_unpack_37;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter);
            tmp_unpack_37 = tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter;
            tmp_assign_source_66 = UNPACK_NEXT(tmp_unpack_37, 0, 3);
            if (tmp_assign_source_66 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 170;
                goto try_except_handler_29;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1 = tmp_assign_source_66;
        }
        {
            PyObject *tmp_assign_source_67;
            PyObject *tmp_unpack_38;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter);
            tmp_unpack_38 = tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter;
            tmp_assign_source_67 = UNPACK_NEXT(tmp_unpack_38, 1, 3);
            if (tmp_assign_source_67 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 170;
                goto try_except_handler_29;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2 = tmp_assign_source_67;
        }
        {
            PyObject *tmp_assign_source_68;
            PyObject *tmp_unpack_39;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter);
            tmp_unpack_39 = tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter;
            tmp_assign_source_68 = UNPACK_NEXT(tmp_unpack_39, 2, 3);
            if (tmp_assign_source_68 == NULL) {
                if (!ERROR_OCCURRED()) {
                    exception_type = PyExc_StopIteration;
                    Py_INCREF(exception_type);
                    exception_value = NULL;
                    exception_tb = NULL;
                } else {
                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                }


                type_description_2 = "o";
                exception_lineno = 170;
                goto try_except_handler_29;
            }
            assert(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3 == NULL);
            tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3 = tmp_assign_source_68;
        }
        {
            PyObject *tmp_iterator_name_13;
            CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter);
            tmp_iterator_name_13 = tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter;
            // Check if iterator has left-over elements.
            CHECK_OBJECT(tmp_iterator_name_13); assert(HAS_ITERNEXT(tmp_iterator_name_13));

            tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_13)->tp_iternext)(tmp_iterator_name_13);

            if (likely(tmp_iterator_attempt == NULL)) {
                PyObject *error = GET_ERROR_OCCURRED();

                if (error != NULL) {
                    if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                        CLEAR_ERROR_OCCURRED();
                    } else {
                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                        type_description_2 = "o";
                        exception_lineno = 170;
                        goto try_except_handler_29;
                    }
                }
            } else {
                Py_DECREF(tmp_iterator_attempt);

                exception_type = PyExc_ValueError;
                Py_INCREF(PyExc_ValueError);
                exception_value = mod_consts[71];
                Py_INCREF(exception_value);
                exception_tb = NULL;

                type_description_2 = "o";
                exception_lineno = 170;
                goto try_except_handler_29;
            }
        }
        goto try_end_25;
        // Exception handler code:
        try_except_handler_29:;
        exception_keeper_type_25 = exception_type;
        exception_keeper_value_25 = exception_value;
        exception_keeper_tb_25 = exception_tb;
        exception_keeper_lineno_25 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_25;
        exception_value = exception_keeper_value_25;
        exception_tb = exception_keeper_tb_25;
        exception_lineno = exception_keeper_lineno_25;

        goto try_except_handler_28;
        // End of try:
        try_end_25:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__source_iter = NULL;
        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[134], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 170;
            type_description_2 = "o";
            goto try_except_handler_28;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[135], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 170;
            type_description_2 = "o";
            goto try_except_handler_28;
        }
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2 = NULL;

        CHECK_OBJECT(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3);
        tmp_dictset_value = tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3;
        tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[136], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 170;
            type_description_2 = "o";
            goto try_except_handler_28;
        }
        goto try_end_26;
        // Exception handler code:
        try_except_handler_28:;
        exception_keeper_type_26 = exception_type;
        exception_keeper_value_26 = exception_value;
        exception_keeper_tb_26 = exception_tb;
        exception_keeper_lineno_26 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_26;
        exception_value = exception_keeper_value_26;
        exception_tb = exception_keeper_tb_26;
        exception_lineno = exception_keeper_lineno_26;

        goto frame_exception_exit_2;
        // End of try:
        try_end_26:;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3 = NULL;

        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_1 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_2 = NULL;
        Py_XDECREF(tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3);
        tmp_NDArrayOperatorsMixin$tuple_unpack_13__element_3 = NULL;
        {
            PyObject *tmp_called_name_23;
            PyObject *tmp_args_element_name_43;
            PyObject *tmp_expression_name_26;
            PyObject *tmp_args_element_name_44;
            tmp_called_name_23 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[39]);

            if (tmp_called_name_23 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_23 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[39]);

                    if (unlikely(tmp_called_name_23 == NULL)) {
                        tmp_called_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[39]);
                    }

                    if (tmp_called_name_23 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 173;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_23);
                }
            }

            tmp_expression_name_26 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_26 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_26 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_26 == NULL)) {
                        tmp_expression_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_26 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_23);

                        exception_lineno = 173;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_26);
                }
            }

            tmp_args_element_name_43 = LOOKUP_ATTRIBUTE(tmp_expression_name_26, mod_consts[137]);
            Py_DECREF(tmp_expression_name_26);
            if (tmp_args_element_name_43 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_23);

                exception_lineno = 173;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_44 = mod_consts[138];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 173;
            {
                PyObject *call_args[] = {tmp_args_element_name_43, tmp_args_element_name_44};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_23, call_args);
            }

            Py_DECREF(tmp_called_name_23);
            Py_DECREF(tmp_args_element_name_43);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 173;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[139], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 173;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_24;
            PyObject *tmp_args_element_name_45;
            PyObject *tmp_expression_name_27;
            PyObject *tmp_args_element_name_46;
            tmp_called_name_24 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[39]);

            if (tmp_called_name_24 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_24 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[39]);

                    if (unlikely(tmp_called_name_24 == NULL)) {
                        tmp_called_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[39]);
                    }

                    if (tmp_called_name_24 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 174;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_24);
                }
            }

            tmp_expression_name_27 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_27 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_27 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_27 == NULL)) {
                        tmp_expression_name_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_27 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_24);

                        exception_lineno = 174;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_27);
                }
            }

            tmp_args_element_name_45 = LOOKUP_ATTRIBUTE(tmp_expression_name_27, mod_consts[140]);
            Py_DECREF(tmp_expression_name_27);
            if (tmp_args_element_name_45 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_24);

                exception_lineno = 174;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_46 = mod_consts[141];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 174;
            {
                PyObject *call_args[] = {tmp_args_element_name_45, tmp_args_element_name_46};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_24, call_args);
            }

            Py_DECREF(tmp_called_name_24);
            Py_DECREF(tmp_args_element_name_45);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 174;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[142], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 174;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_25;
            PyObject *tmp_args_element_name_47;
            PyObject *tmp_expression_name_28;
            PyObject *tmp_args_element_name_48;
            tmp_called_name_25 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[39]);

            if (tmp_called_name_25 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_25 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[39]);

                    if (unlikely(tmp_called_name_25 == NULL)) {
                        tmp_called_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[39]);
                    }

                    if (tmp_called_name_25 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 175;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_25);
                }
            }

            tmp_expression_name_28 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_28 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_28 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_28 == NULL)) {
                        tmp_expression_name_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_28 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_25);

                        exception_lineno = 175;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_28);
                }
            }

            tmp_args_element_name_47 = LOOKUP_ATTRIBUTE(tmp_expression_name_28, mod_consts[143]);
            Py_DECREF(tmp_expression_name_28);
            if (tmp_args_element_name_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_25);

                exception_lineno = 175;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_48 = mod_consts[144];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 175;
            {
                PyObject *call_args[] = {tmp_args_element_name_47, tmp_args_element_name_48};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_25, call_args);
            }

            Py_DECREF(tmp_called_name_25);
            Py_DECREF(tmp_args_element_name_47);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 175;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[145], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 175;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }
        {
            PyObject *tmp_called_name_26;
            PyObject *tmp_args_element_name_49;
            PyObject *tmp_expression_name_29;
            PyObject *tmp_args_element_name_50;
            tmp_called_name_26 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[39]);

            if (tmp_called_name_26 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_called_name_26 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[39]);

                    if (unlikely(tmp_called_name_26 == NULL)) {
                        tmp_called_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[39]);
                    }

                    if (tmp_called_name_26 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 176;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_called_name_26);
                }
            }

            tmp_expression_name_29 = PyObject_GetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[35]);

            if (tmp_expression_name_29 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_expression_name_29 = GET_STRING_DICT_VALUE(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[35]);

                    if (unlikely(tmp_expression_name_29 == NULL)) {
                        tmp_expression_name_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[35]);
                    }

                    if (tmp_expression_name_29 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                        Py_DECREF(tmp_called_name_26);

                        exception_lineno = 176;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_expression_name_29);
                }
            }

            tmp_args_element_name_49 = LOOKUP_ATTRIBUTE(tmp_expression_name_29, mod_consts[146]);
            Py_DECREF(tmp_expression_name_29);
            if (tmp_args_element_name_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                Py_DECREF(tmp_called_name_26);

                exception_lineno = 176;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_args_element_name_50 = mod_consts[146];
            frame_96540219adf36c3ee819922f8ccafbde_2->m_frame.f_lineno = 176;
            {
                PyObject *call_args[] = {tmp_args_element_name_49, tmp_args_element_name_50};
                tmp_dictset_value = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_26, call_args);
            }

            Py_DECREF(tmp_called_name_26);
            Py_DECREF(tmp_args_element_name_49);
            if (tmp_dictset_value == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 176;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
            tmp_res = PyObject_SetItem(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59, mod_consts[147], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 176;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_96540219adf36c3ee819922f8ccafbde_2);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_1;

        frame_exception_exit_2:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_96540219adf36c3ee819922f8ccafbde_2);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_96540219adf36c3ee819922f8ccafbde_2, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_96540219adf36c3ee819922f8ccafbde_2->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_96540219adf36c3ee819922f8ccafbde_2, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_96540219adf36c3ee819922f8ccafbde_2,
            type_description_2,
            outline_0_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_96540219adf36c3ee819922f8ccafbde_2 == cache_frame_96540219adf36c3ee819922f8ccafbde_2) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_96540219adf36c3ee819922f8ccafbde_2);
            cache_frame_96540219adf36c3ee819922f8ccafbde_2 = NULL;
        }

        assertFrameObject(frame_96540219adf36c3ee819922f8ccafbde_2);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_1;

        frame_no_exception_1:;
        goto skip_nested_handling_1;
        nested_frame_exit_1:;

        goto try_except_handler_3;
        skip_nested_handling_1:;
        {
            PyObject *tmp_assign_source_69;
            PyObject *tmp_called_name_27;
            PyObject *tmp_args_name_2;
            PyObject *tmp_tuple_element_2;
            PyObject *tmp_kwargs_name_2;
            CHECK_OBJECT(tmp_class_creation_1__metaclass);
            tmp_called_name_27 = tmp_class_creation_1__metaclass;
            tmp_tuple_element_2 = mod_consts[50];
            tmp_args_name_2 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_2, 0, tmp_tuple_element_2);
            tmp_tuple_element_2 = mod_consts[41];
            PyTuple_SET_ITEM0(tmp_args_name_2, 1, tmp_tuple_element_2);
            tmp_tuple_element_2 = locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59;
            PyTuple_SET_ITEM0(tmp_args_name_2, 2, tmp_tuple_element_2);
            CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
            tmp_kwargs_name_2 = tmp_class_creation_1__class_decl_dict;
            frame_049ab1cedaec4f0e1c6f449948e8a004->m_frame.f_lineno = 59;
            tmp_assign_source_69 = CALL_FUNCTION(tmp_called_name_27, tmp_args_name_2, tmp_kwargs_name_2);
            Py_DECREF(tmp_args_name_2);
            if (tmp_assign_source_69 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 59;

                goto try_except_handler_3;
            }
            assert(outline_0_var___class__ == NULL);
            outline_0_var___class__ = tmp_assign_source_69;
        }
        CHECK_OBJECT(outline_0_var___class__);
        tmp_assign_source_16 = outline_0_var___class__;
        Py_INCREF(tmp_assign_source_16);
        goto try_return_handler_3;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_3:;
        Py_DECREF(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59);
        locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59 = NULL;
        goto try_return_handler_2;
        // Exception handler code:
        try_except_handler_3:;
        exception_keeper_type_27 = exception_type;
        exception_keeper_value_27 = exception_value;
        exception_keeper_tb_27 = exception_tb;
        exception_keeper_lineno_27 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59);
        locals_numpy$lib$mixins$$$class__1_NDArrayOperatorsMixin_59 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_27;
        exception_value = exception_keeper_value_27;
        exception_tb = exception_keeper_tb_27;
        exception_lineno = exception_keeper_lineno_27;

        goto try_except_handler_2;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_2:;
        CHECK_OBJECT(outline_0_var___class__);
        Py_DECREF(outline_0_var___class__);
        outline_0_var___class__ = NULL;
        goto outline_result_1;
        // Exception handler code:
        try_except_handler_2:;
        exception_keeper_type_28 = exception_type;
        exception_keeper_value_28 = exception_value;
        exception_keeper_tb_28 = exception_tb;
        exception_keeper_lineno_28 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_28;
        exception_value = exception_keeper_value_28;
        exception_tb = exception_keeper_tb_28;
        exception_lineno = exception_keeper_lineno_28;

        goto outline_exception_1;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_1:;
        exception_lineno = 59;
        goto try_except_handler_1;
        outline_result_1:;
        UPDATE_STRING_DICT1(moduledict_numpy$lib$mixins, (Nuitka_StringObject *)mod_consts[50], tmp_assign_source_16);
    }
    goto try_end_27;
    // Exception handler code:
    try_except_handler_1:;
    exception_keeper_type_29 = exception_type;
    exception_keeper_value_29 = exception_value;
    exception_keeper_tb_29 = exception_tb;
    exception_keeper_lineno_29 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_29;
    exception_value = exception_keeper_value_29;
    exception_tb = exception_keeper_tb_29;
    exception_lineno = exception_keeper_lineno_29;

    goto frame_exception_exit_1;
    // End of try:
    try_end_27:;

    // Restore frame exception if necessary.
#if 0
    RESTORE_FRAME_EXCEPTION(frame_049ab1cedaec4f0e1c6f449948e8a004);
#endif
    popFrameStack();

    assertFrameObject(frame_049ab1cedaec4f0e1c6f449948e8a004);

    goto frame_no_exception_2;

    frame_exception_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_049ab1cedaec4f0e1c6f449948e8a004);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_049ab1cedaec4f0e1c6f449948e8a004, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_049ab1cedaec4f0e1c6f449948e8a004->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_049ab1cedaec4f0e1c6f449948e8a004, exception_lineno);
    }

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto module_exception_exit;

    frame_no_exception_2:;
    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_1__prepared);
    Py_DECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;

    return module_numpy$lib$mixins;
    module_exception_exit:
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);
    return NULL;
}

