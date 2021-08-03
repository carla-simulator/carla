/* Generated code for Python module 'pip._vendor.urllib3.exceptions'
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

/* The "module_pip$_vendor$urllib3$exceptions" is a Python object pointer of module type.
 *
 * Note: For full compatibility with CPython, every module variable access
 * needs to go through it except for cases where the module cannot possibly
 * have changed in the mean time.
 */

PyObject *module_pip$_vendor$urllib3$exceptions;
PyDictObject *moduledict_pip$_vendor$urllib3$exceptions;

/* The declarations of module constants used, if any. */
static PyObject *mod_consts[162];

static PyObject *module_filename_obj = NULL;

/* Indicator if this modules private constants were created yet. */
static bool constants_created = false;

/* Function to create module private constants. */
static void createModuleConstants(void) {
    if (constants_created == false) {
        loadConstantsBlob(&mod_consts[0], UNTRANSLATE("pip._vendor.urllib3.exceptions"));
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
void checkModuleConstants_pip$_vendor$urllib3$exceptions(void) {
    // The module may not have been used at all, then ignore this.
    if (constants_created == false) return;

    checkConstantsBlob(&mod_consts[0], "pip._vendor.urllib3.exceptions");
}
#endif

// The module code objects.
static PyCodeObject *codeobj_12850fd7f94e4dfb545b718c84e504f7;
static PyCodeObject *codeobj_741c7aa688cb6408fa2c7e94c2e034af;
static PyCodeObject *codeobj_c61a422c602595da887bdeb6ad5c1789;
static PyCodeObject *codeobj_90ad096779fac202a1b1540ce0977ce0;
static PyCodeObject *codeobj_546b5f9419c19af80309bdfac001c04f;
static PyCodeObject *codeobj_066d2c927324a67686dc7baef2a7a942;
static PyCodeObject *codeobj_d4612ad00762592c3f3b527f730e5d41;
static PyCodeObject *codeobj_174b7ceb75edad879b3d9d167edc3442;
static PyCodeObject *codeobj_ab51bb3d6332d4b560ddcc31e7b3fc22;
static PyCodeObject *codeobj_b067facd3b4500d3e185f1adaf50e20e;
static PyCodeObject *codeobj_390e0d2c1f0a89462d2b6e4f63d3fdc4;
static PyCodeObject *codeobj_91103067dcd586dfb73a9300382a2bf7;
static PyCodeObject *codeobj_db8cc6926c2108e877484bc63b18157c;
static PyCodeObject *codeobj_59493ff7ab18b1206e1b870f33336d38;
static PyCodeObject *codeobj_201187fceac633fe0ddb8cf64eed80a7;
static PyCodeObject *codeobj_13ccd233d3c49c0deeb5578d5d988a97;
static PyCodeObject *codeobj_789fae4f29ca82fe44ad83242d8d23ec;
static PyCodeObject *codeobj_99eacdb6046efa487fa94a7a939ebbc3;
static PyCodeObject *codeobj_df4920181a454a81b8219ced28b38ee7;
static PyCodeObject *codeobj_6b4910f8e728dcfd0803ea56207884a7;
static PyCodeObject *codeobj_c4818851a6696683b8f85d2a009b0859;
static PyCodeObject *codeobj_98ee57df943e033264d984d9d9bfbb6d;
static PyCodeObject *codeobj_71824b5007f999f47c76f01b04b4a7ec;
static PyCodeObject *codeobj_2a30d9dcbcedcf3e6be926983f3c62ce;
static PyCodeObject *codeobj_9efec6d4ef9615db68fe3db532fafbec;
static PyCodeObject *codeobj_195b6cb8f13f233cfa7abbadc4ef4ae7;
static PyCodeObject *codeobj_7163e59336e1ab5575db0a821461ddb9;
static PyCodeObject *codeobj_208f80491e1b413f20da0e11d5be6e91;

static void createModuleCodeObjects(void) {
    module_filename_obj = mod_consts[36]; CHECK_OBJECT(module_filename_obj);
    codeobj_12850fd7f94e4dfb545b718c84e504f7 = MAKE_CODEOBJECT(module_filename_obj, 1, CO_NOFREE, mod_consts[149], NULL, NULL, 0, 0, 0);
    codeobj_741c7aa688cb6408fa2c7e94c2e034af = MAKE_CODEOBJECT(module_filename_obj, 302, CO_NOFREE, mod_consts[34], mod_consts[150], NULL, 0, 0, 0);
    codeobj_c61a422c602595da887bdeb6ad5c1789 = MAKE_CODEOBJECT(module_filename_obj, 95, CO_NOFREE, mod_consts[82], mod_consts[150], NULL, 0, 0, 0);
    codeobj_90ad096779fac202a1b1540ce0977ce0 = MAKE_CODEOBJECT(module_filename_obj, 245, CO_NOFREE, mod_consts[20], mod_consts[150], NULL, 0, 0, 0);
    codeobj_546b5f9419c19af80309bdfac001c04f = MAKE_CODEOBJECT(module_filename_obj, 263, CO_NOFREE, mod_consts[24], mod_consts[150], NULL, 0, 0, 0);
    codeobj_066d2c927324a67686dc7baef2a7a942 = MAKE_CODEOBJECT(module_filename_obj, 158, CO_NOFREE, mod_consts[102], mod_consts[150], NULL, 0, 0, 0);
    codeobj_d4612ad00762592c3f3b527f730e5d41 = MAKE_CODEOBJECT(module_filename_obj, 77, CO_NOFREE, mod_consts[78], mod_consts[150], NULL, 0, 0, 0);
    codeobj_174b7ceb75edad879b3d9d167edc3442 = MAKE_CODEOBJECT(module_filename_obj, 20, CO_NOFREE, mod_consts[6], mod_consts[150], NULL, 0, 0, 0);
    codeobj_ab51bb3d6332d4b560ddcc31e7b3fc22 = MAKE_CODEOBJECT(module_filename_obj, 50, CO_NOFREE, mod_consts[8], mod_consts[150], NULL, 0, 0, 0);
    codeobj_b067facd3b4500d3e185f1adaf50e20e = MAKE_CODEOBJECT(module_filename_obj, 286, CO_NOFREE, mod_consts[31], mod_consts[150], NULL, 0, 0, 0);
    codeobj_390e0d2c1f0a89462d2b6e4f63d3fdc4 = MAKE_CODEOBJECT(module_filename_obj, 32, CO_NOFREE, mod_consts[12], mod_consts[150], NULL, 0, 0, 0);
    codeobj_91103067dcd586dfb73a9300382a2bf7 = MAKE_CODEOBJECT(module_filename_obj, 178, CO_NOFREE, mod_consts[107], mod_consts[150], NULL, 0, 0, 0);
    codeobj_db8cc6926c2108e877484bc63b18157c = MAKE_CODEOBJECT(module_filename_obj, 168, CO_NOFREE, mod_consts[18], mod_consts[150], NULL, 0, 0, 0);
    codeobj_59493ff7ab18b1206e1b870f33336d38 = MAKE_CODEOBJECT(module_filename_obj, 305, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[151], NULL, 3, 0, 0);
    codeobj_201187fceac633fe0ddb8cf64eed80a7 = MAKE_CODEOBJECT(module_filename_obj, 161, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[152], NULL, 2, 0, 0);
    codeobj_13ccd233d3c49c0deeb5578d5d988a97 = MAKE_CODEOBJECT(module_filename_obj, 53, CO_OPTIMIZED | CO_NEWLOCALS | CO_VARARGS | CO_NOFREE, mod_consts[2], mod_consts[153], NULL, 3, 0, 0);
    codeobj_789fae4f29ca82fe44ad83242d8d23ec = MAKE_CODEOBJECT(module_filename_obj, 253, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[154], NULL, 3, 0, 0);
    codeobj_99eacdb6046efa487fa94a7a939ebbc3 = MAKE_CODEOBJECT(module_filename_obj, 23, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[155], NULL, 3, 0, 0);
    codeobj_df4920181a454a81b8219ced28b38ee7 = MAKE_CODEOBJECT(module_filename_obj, 35, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[156], NULL, 4, 0, 0);
    codeobj_6b4910f8e728dcfd0803ea56207884a7 = MAKE_CODEOBJECT(module_filename_obj, 87, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[157], NULL, 4, 0, 0);
    codeobj_c4818851a6696683b8f85d2a009b0859 = MAKE_CODEOBJECT(module_filename_obj, 98, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[158], NULL, 4, 0, 0);
    codeobj_98ee57df943e033264d984d9d9bfbb6d = MAKE_CODEOBJECT(module_filename_obj, 266, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[159], NULL, 3, 0, 0);
    codeobj_71824b5007f999f47c76f01b04b4a7ec = MAKE_CODEOBJECT(module_filename_obj, 171, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[160], NULL, 2, 0, 0);
    codeobj_2a30d9dcbcedcf3e6be926983f3c62ce = MAKE_CODEOBJECT(module_filename_obj, 291, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[2], mod_consts[160], NULL, 2, 0, 0);
    codeobj_9efec6d4ef9615db68fe3db532fafbec = MAKE_CODEOBJECT(module_filename_obj, 27, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[64], mod_consts[161], NULL, 1, 0, 0);
    codeobj_195b6cb8f13f233cfa7abbadc4ef4ae7 = MAKE_CODEOBJECT(module_filename_obj, 39, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[64], mod_consts[161], NULL, 1, 0, 0);
    codeobj_7163e59336e1ab5575db0a821461ddb9 = MAKE_CODEOBJECT(module_filename_obj, 256, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[133], mod_consts[161], NULL, 1, 0, 0);
    codeobj_208f80491e1b413f20da0e11d5be6e91 = MAKE_CODEOBJECT(module_filename_obj, 273, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[133], mod_consts[161], NULL, 1, 0, 0);
}

// The module function declarations.
NUITKA_CROSS_MODULE PyObject *impl___main__$$$function__5_complex_call_helper_pos_star_list(PyObject **python_pars);


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__10___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__11___repr__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__12___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__13___repr__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__14___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__15___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__1___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__2___reduce__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__3___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__4___reduce__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__5___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__6___init__(PyObject *defaults);


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__7___init__(PyObject *defaults);


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__8___init__();


static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__9___init__();


// The module function definitions.
static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__1___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_pool = python_pars[1];
    PyObject *par_message = python_pars[2];
    struct Nuitka_FrameObject *frame_99eacdb6046efa487fa94a7a939ebbc3;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    bool tmp_result;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_frame_99eacdb6046efa487fa94a7a939ebbc3 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_99eacdb6046efa487fa94a7a939ebbc3)) {
        Py_XDECREF(cache_frame_99eacdb6046efa487fa94a7a939ebbc3);

#if _DEBUG_REFCOUNTS
        if (cache_frame_99eacdb6046efa487fa94a7a939ebbc3 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_99eacdb6046efa487fa94a7a939ebbc3 = MAKE_FUNCTION_FRAME(codeobj_99eacdb6046efa487fa94a7a939ebbc3, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_99eacdb6046efa487fa94a7a939ebbc3->m_type_description == NULL);
    frame_99eacdb6046efa487fa94a7a939ebbc3 = cache_frame_99eacdb6046efa487fa94a7a939ebbc3;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_99eacdb6046efa487fa94a7a939ebbc3);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_99eacdb6046efa487fa94a7a939ebbc3) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        CHECK_OBJECT(par_pool);
        tmp_assattr_name_1 = par_pool;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[0], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 24;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_tuple_element_1;
        tmp_expression_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_expression_name_1 == NULL)) {
            tmp_expression_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 25;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[2]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 25;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_args_element_name_1 = par_self;
        tmp_left_name_1 = mod_consts[3];
        CHECK_OBJECT(par_pool);
        tmp_tuple_element_1 = par_pool;
        tmp_right_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_right_name_1, 0, tmp_tuple_element_1);
        CHECK_OBJECT(par_message);
        tmp_tuple_element_1 = par_message;
        PyTuple_SET_ITEM0(tmp_right_name_1, 1, tmp_tuple_element_1);
        tmp_args_element_name_2 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_args_element_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 25;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
        frame_99eacdb6046efa487fa94a7a939ebbc3->m_frame.f_lineno = 25;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_call_result_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_called_name_1);
        Py_DECREF(tmp_args_element_name_2);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 25;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_99eacdb6046efa487fa94a7a939ebbc3);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_99eacdb6046efa487fa94a7a939ebbc3);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_99eacdb6046efa487fa94a7a939ebbc3, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_99eacdb6046efa487fa94a7a939ebbc3->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_99eacdb6046efa487fa94a7a939ebbc3, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_99eacdb6046efa487fa94a7a939ebbc3,
        type_description_1,
        par_self,
        par_pool,
        par_message
    );


    // Release cached frame if used for exception.
    if (frame_99eacdb6046efa487fa94a7a939ebbc3 == cache_frame_99eacdb6046efa487fa94a7a939ebbc3) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_99eacdb6046efa487fa94a7a939ebbc3);
        cache_frame_99eacdb6046efa487fa94a7a939ebbc3 = NULL;
    }

    assertFrameObject(frame_99eacdb6046efa487fa94a7a939ebbc3);

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
    Py_XDECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_pool);
    Py_DECREF(par_pool);
    par_pool = NULL;
    CHECK_OBJECT(par_message);
    Py_DECREF(par_message);
    par_message = NULL;
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

    Py_XDECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_pool);
    Py_DECREF(par_pool);
    par_pool = NULL;
    CHECK_OBJECT(par_message);
    Py_DECREF(par_message);
    par_message = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__2___reduce__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    struct Nuitka_FrameObject *frame_9efec6d4ef9615db68fe3db532fafbec;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_9efec6d4ef9615db68fe3db532fafbec = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_9efec6d4ef9615db68fe3db532fafbec)) {
        Py_XDECREF(cache_frame_9efec6d4ef9615db68fe3db532fafbec);

#if _DEBUG_REFCOUNTS
        if (cache_frame_9efec6d4ef9615db68fe3db532fafbec == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_9efec6d4ef9615db68fe3db532fafbec = MAKE_FUNCTION_FRAME(codeobj_9efec6d4ef9615db68fe3db532fafbec, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_9efec6d4ef9615db68fe3db532fafbec->m_type_description == NULL);
    frame_9efec6d4ef9615db68fe3db532fafbec = cache_frame_9efec6d4ef9615db68fe3db532fafbec;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_9efec6d4ef9615db68fe3db532fafbec);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_9efec6d4ef9615db68fe3db532fafbec) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_expression_name_1;
        CHECK_OBJECT(par_self);
        tmp_expression_name_1 = par_self;
        tmp_tuple_element_1 = LOOKUP_ATTRIBUTE_CLASS_SLOT(tmp_expression_name_1);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;
            type_description_1 = "o";
            goto frame_exception_exit_1;
        }
        tmp_return_value = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_return_value, 0, tmp_tuple_element_1);
        tmp_tuple_element_1 = mod_consts[4];
        PyTuple_SET_ITEM0(tmp_return_value, 1, tmp_tuple_element_1);
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_9efec6d4ef9615db68fe3db532fafbec);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_9efec6d4ef9615db68fe3db532fafbec);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_9efec6d4ef9615db68fe3db532fafbec);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_9efec6d4ef9615db68fe3db532fafbec, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_9efec6d4ef9615db68fe3db532fafbec->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_9efec6d4ef9615db68fe3db532fafbec, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_9efec6d4ef9615db68fe3db532fafbec,
        type_description_1,
        par_self
    );


    // Release cached frame if used for exception.
    if (frame_9efec6d4ef9615db68fe3db532fafbec == cache_frame_9efec6d4ef9615db68fe3db532fafbec) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_9efec6d4ef9615db68fe3db532fafbec);
        cache_frame_9efec6d4ef9615db68fe3db532fafbec = NULL;
    }

    assertFrameObject(frame_9efec6d4ef9615db68fe3db532fafbec);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    Py_XDECREF(par_self);
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

    Py_XDECREF(par_self);
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__3___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_pool = python_pars[1];
    PyObject *par_url = python_pars[2];
    PyObject *par_message = python_pars[3];
    struct Nuitka_FrameObject *frame_df4920181a454a81b8219ced28b38ee7;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    bool tmp_result;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_frame_df4920181a454a81b8219ced28b38ee7 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_df4920181a454a81b8219ced28b38ee7)) {
        Py_XDECREF(cache_frame_df4920181a454a81b8219ced28b38ee7);

#if _DEBUG_REFCOUNTS
        if (cache_frame_df4920181a454a81b8219ced28b38ee7 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_df4920181a454a81b8219ced28b38ee7 = MAKE_FUNCTION_FRAME(codeobj_df4920181a454a81b8219ced28b38ee7, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_df4920181a454a81b8219ced28b38ee7->m_type_description == NULL);
    frame_df4920181a454a81b8219ced28b38ee7 = cache_frame_df4920181a454a81b8219ced28b38ee7;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_df4920181a454a81b8219ced28b38ee7);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_df4920181a454a81b8219ced28b38ee7) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        CHECK_OBJECT(par_url);
        tmp_assattr_name_1 = par_url;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[5], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 36;
            type_description_1 = "oooo";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        tmp_called_instance_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[6]);

        if (unlikely(tmp_called_instance_1 == NULL)) {
            tmp_called_instance_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[6]);
        }

        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 37;
            type_description_1 = "oooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_args_element_name_1 = par_self;
        CHECK_OBJECT(par_pool);
        tmp_args_element_name_2 = par_pool;
        CHECK_OBJECT(par_message);
        tmp_args_element_name_3 = par_message;
        frame_df4920181a454a81b8219ced28b38ee7->m_frame.f_lineno = 37;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2, tmp_args_element_name_3};
            tmp_call_result_1 = CALL_METHOD_WITH_ARGS3(
                tmp_called_instance_1,
                mod_consts[2],
                call_args
            );
        }

        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 37;
            type_description_1 = "oooo";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_df4920181a454a81b8219ced28b38ee7);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_df4920181a454a81b8219ced28b38ee7);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_df4920181a454a81b8219ced28b38ee7, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_df4920181a454a81b8219ced28b38ee7->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_df4920181a454a81b8219ced28b38ee7, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_df4920181a454a81b8219ced28b38ee7,
        type_description_1,
        par_self,
        par_pool,
        par_url,
        par_message
    );


    // Release cached frame if used for exception.
    if (frame_df4920181a454a81b8219ced28b38ee7 == cache_frame_df4920181a454a81b8219ced28b38ee7) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_df4920181a454a81b8219ced28b38ee7);
        cache_frame_df4920181a454a81b8219ced28b38ee7 = NULL;
    }

    assertFrameObject(frame_df4920181a454a81b8219ced28b38ee7);

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
    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_pool);
    par_pool = NULL;
    CHECK_OBJECT(par_url);
    Py_DECREF(par_url);
    par_url = NULL;
    Py_XDECREF(par_message);
    par_message = NULL;
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

    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_pool);
    par_pool = NULL;
    CHECK_OBJECT(par_url);
    Py_DECREF(par_url);
    par_url = NULL;
    Py_XDECREF(par_message);
    par_message = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__4___reduce__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    struct Nuitka_FrameObject *frame_195b6cb8f13f233cfa7abbadc4ef4ae7;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7 = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7)) {
        Py_XDECREF(cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7);

#if _DEBUG_REFCOUNTS
        if (cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7 = MAKE_FUNCTION_FRAME(codeobj_195b6cb8f13f233cfa7abbadc4ef4ae7, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7->m_type_description == NULL);
    frame_195b6cb8f13f233cfa7abbadc4ef4ae7 = cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_195b6cb8f13f233cfa7abbadc4ef4ae7);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_195b6cb8f13f233cfa7abbadc4ef4ae7) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_expression_name_1;
        CHECK_OBJECT(par_self);
        tmp_expression_name_1 = par_self;
        tmp_tuple_element_1 = LOOKUP_ATTRIBUTE_CLASS_SLOT(tmp_expression_name_1);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 41;
            type_description_1 = "o";
            goto frame_exception_exit_1;
        }
        tmp_return_value = PyTuple_New(2);
        {
            PyObject *tmp_tuple_element_2;
            PyTuple_SET_ITEM(tmp_return_value, 0, tmp_tuple_element_1);
            tmp_tuple_element_2 = Py_None;
            tmp_tuple_element_1 = PyTuple_New(3);
            {
                PyObject *tmp_expression_name_2;
                PyTuple_SET_ITEM0(tmp_tuple_element_1, 0, tmp_tuple_element_2);
                if (par_self == NULL) {

                    FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[7]);
                    exception_tb = NULL;
                    NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
                    CHAIN_EXCEPTION(exception_value);

                    exception_lineno = 41;
                    type_description_1 = "o";
                    goto tuple_build_exception_2;
                }

                tmp_expression_name_2 = par_self;
                tmp_tuple_element_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[5]);
                if (tmp_tuple_element_2 == NULL) {
                    assert(ERROR_OCCURRED());

                    FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                    exception_lineno = 41;
                    type_description_1 = "o";
                    goto tuple_build_exception_2;
                }
                PyTuple_SET_ITEM(tmp_tuple_element_1, 1, tmp_tuple_element_2);
                tmp_tuple_element_2 = Py_None;
                PyTuple_SET_ITEM0(tmp_tuple_element_1, 2, tmp_tuple_element_2);
            }
            goto tuple_build_noexception_1;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_2:;
            Py_DECREF(tmp_tuple_element_1);
            goto tuple_build_exception_1;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_1:;
            PyTuple_SET_ITEM(tmp_return_value, 1, tmp_tuple_element_1);
        }
        goto tuple_build_noexception_2;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_return_value);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_2:;
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_195b6cb8f13f233cfa7abbadc4ef4ae7);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_195b6cb8f13f233cfa7abbadc4ef4ae7);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_195b6cb8f13f233cfa7abbadc4ef4ae7);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_195b6cb8f13f233cfa7abbadc4ef4ae7, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_195b6cb8f13f233cfa7abbadc4ef4ae7->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_195b6cb8f13f233cfa7abbadc4ef4ae7, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_195b6cb8f13f233cfa7abbadc4ef4ae7,
        type_description_1,
        par_self
    );


    // Release cached frame if used for exception.
    if (frame_195b6cb8f13f233cfa7abbadc4ef4ae7 == cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7);
        cache_frame_195b6cb8f13f233cfa7abbadc4ef4ae7 = NULL;
    }

    assertFrameObject(frame_195b6cb8f13f233cfa7abbadc4ef4ae7);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    Py_XDECREF(par_self);
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

    Py_XDECREF(par_self);
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__5___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_message = python_pars[1];
    PyObject *par_error = python_pars[2];
    PyObject *par_args = python_pars[3];
    struct Nuitka_FrameObject *frame_13ccd233d3c49c0deeb5578d5d988a97;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_13ccd233d3c49c0deeb5578d5d988a97 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_13ccd233d3c49c0deeb5578d5d988a97)) {
        Py_XDECREF(cache_frame_13ccd233d3c49c0deeb5578d5d988a97);

#if _DEBUG_REFCOUNTS
        if (cache_frame_13ccd233d3c49c0deeb5578d5d988a97 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_13ccd233d3c49c0deeb5578d5d988a97 = MAKE_FUNCTION_FRAME(codeobj_13ccd233d3c49c0deeb5578d5d988a97, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_13ccd233d3c49c0deeb5578d5d988a97->m_type_description == NULL);
    frame_13ccd233d3c49c0deeb5578d5d988a97 = cache_frame_13ccd233d3c49c0deeb5578d5d988a97;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_13ccd233d3c49c0deeb5578d5d988a97);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_13ccd233d3c49c0deeb5578d5d988a97) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_dircall_arg1_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_type_arg_name_1;
        PyObject *tmp_object_arg_name_1;
        PyObject *tmp_dircall_arg2_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_dircall_arg3_1;
        PyObject *tmp_call_result_1;
        tmp_type_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[8]);

        if (unlikely(tmp_type_arg_name_1 == NULL)) {
            tmp_type_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[8]);
        }

        if (tmp_type_arg_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 54;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_object_arg_name_1 = par_self;
        tmp_expression_name_1 = BUILTIN_SUPER2(tmp_type_arg_name_1, tmp_object_arg_name_1);
        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 54;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        tmp_dircall_arg1_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[2]);
        Py_DECREF(tmp_expression_name_1);
        if (tmp_dircall_arg1_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 54;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_message);
        tmp_tuple_element_1 = par_message;
        tmp_dircall_arg2_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_dircall_arg2_1, 0, tmp_tuple_element_1);
        CHECK_OBJECT(par_error);
        tmp_tuple_element_1 = par_error;
        PyTuple_SET_ITEM0(tmp_dircall_arg2_1, 1, tmp_tuple_element_1);
        CHECK_OBJECT(par_args);
        tmp_dircall_arg3_1 = par_args;
        Py_INCREF(tmp_dircall_arg3_1);

        {
            PyObject *dir_call_args[] = {tmp_dircall_arg1_1, tmp_dircall_arg2_1, tmp_dircall_arg3_1};
            tmp_call_result_1 = impl___main__$$$function__5_complex_call_helper_pos_star_list(dir_call_args);
        }
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 54;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        CHECK_OBJECT(par_error);
        tmp_assattr_name_1 = par_error;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[9], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 55;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_13ccd233d3c49c0deeb5578d5d988a97);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_13ccd233d3c49c0deeb5578d5d988a97);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_13ccd233d3c49c0deeb5578d5d988a97, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_13ccd233d3c49c0deeb5578d5d988a97->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_13ccd233d3c49c0deeb5578d5d988a97, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_13ccd233d3c49c0deeb5578d5d988a97,
        type_description_1,
        par_self,
        par_message,
        par_error,
        par_args,
        NULL
    );


    // Release cached frame if used for exception.
    if (frame_13ccd233d3c49c0deeb5578d5d988a97 == cache_frame_13ccd233d3c49c0deeb5578d5d988a97) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_13ccd233d3c49c0deeb5578d5d988a97);
        cache_frame_13ccd233d3c49c0deeb5578d5d988a97 = NULL;
    }

    assertFrameObject(frame_13ccd233d3c49c0deeb5578d5d988a97);

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
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_message);
    Py_DECREF(par_message);
    par_message = NULL;
    CHECK_OBJECT(par_error);
    Py_DECREF(par_error);
    par_error = NULL;
    CHECK_OBJECT(par_args);
    Py_DECREF(par_args);
    par_args = NULL;
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
    CHECK_OBJECT(par_message);
    Py_DECREF(par_message);
    par_message = NULL;
    CHECK_OBJECT(par_error);
    Py_DECREF(par_error);
    par_error = NULL;
    CHECK_OBJECT(par_args);
    Py_DECREF(par_args);
    par_args = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__6___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_pool = python_pars[1];
    PyObject *par_url = python_pars[2];
    PyObject *par_reason = python_pars[3];
    PyObject *var_message = NULL;
    struct Nuitka_FrameObject *frame_6b4910f8e728dcfd0803ea56207884a7;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    bool tmp_result;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_frame_6b4910f8e728dcfd0803ea56207884a7 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_6b4910f8e728dcfd0803ea56207884a7)) {
        Py_XDECREF(cache_frame_6b4910f8e728dcfd0803ea56207884a7);

#if _DEBUG_REFCOUNTS
        if (cache_frame_6b4910f8e728dcfd0803ea56207884a7 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_6b4910f8e728dcfd0803ea56207884a7 = MAKE_FUNCTION_FRAME(codeobj_6b4910f8e728dcfd0803ea56207884a7, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_6b4910f8e728dcfd0803ea56207884a7->m_type_description == NULL);
    frame_6b4910f8e728dcfd0803ea56207884a7 = cache_frame_6b4910f8e728dcfd0803ea56207884a7;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_6b4910f8e728dcfd0803ea56207884a7);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_6b4910f8e728dcfd0803ea56207884a7) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        CHECK_OBJECT(par_reason);
        tmp_assattr_name_1 = par_reason;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[10], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 88;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_tuple_element_1;
        tmp_left_name_1 = mod_consts[11];
        CHECK_OBJECT(par_url);
        tmp_tuple_element_1 = par_url;
        tmp_right_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_right_name_1, 0, tmp_tuple_element_1);
        CHECK_OBJECT(par_reason);
        tmp_tuple_element_1 = par_reason;
        PyTuple_SET_ITEM0(tmp_right_name_1, 1, tmp_tuple_element_1);
        tmp_assign_source_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 90;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        assert(var_message == NULL);
        var_message = tmp_assign_source_1;
    }
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        PyObject *tmp_args_element_name_4;
        tmp_called_instance_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[12]);

        if (unlikely(tmp_called_instance_1 == NULL)) {
            tmp_called_instance_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
        }

        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 92;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_args_element_name_1 = par_self;
        CHECK_OBJECT(par_pool);
        tmp_args_element_name_2 = par_pool;
        CHECK_OBJECT(par_url);
        tmp_args_element_name_3 = par_url;
        CHECK_OBJECT(var_message);
        tmp_args_element_name_4 = var_message;
        frame_6b4910f8e728dcfd0803ea56207884a7->m_frame.f_lineno = 92;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2, tmp_args_element_name_3, tmp_args_element_name_4};
            tmp_call_result_1 = CALL_METHOD_WITH_ARGS4(
                tmp_called_instance_1,
                mod_consts[2],
                call_args
            );
        }

        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 92;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_6b4910f8e728dcfd0803ea56207884a7);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_6b4910f8e728dcfd0803ea56207884a7);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_6b4910f8e728dcfd0803ea56207884a7, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_6b4910f8e728dcfd0803ea56207884a7->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_6b4910f8e728dcfd0803ea56207884a7, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_6b4910f8e728dcfd0803ea56207884a7,
        type_description_1,
        par_self,
        par_pool,
        par_url,
        par_reason,
        var_message
    );


    // Release cached frame if used for exception.
    if (frame_6b4910f8e728dcfd0803ea56207884a7 == cache_frame_6b4910f8e728dcfd0803ea56207884a7) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_6b4910f8e728dcfd0803ea56207884a7);
        cache_frame_6b4910f8e728dcfd0803ea56207884a7 = NULL;
    }

    assertFrameObject(frame_6b4910f8e728dcfd0803ea56207884a7);

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
    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_pool);
    par_pool = NULL;
    Py_XDECREF(par_url);
    par_url = NULL;
    CHECK_OBJECT(par_reason);
    Py_DECREF(par_reason);
    par_reason = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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

    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_pool);
    par_pool = NULL;
    Py_XDECREF(par_url);
    par_url = NULL;
    CHECK_OBJECT(par_reason);
    Py_DECREF(par_reason);
    par_reason = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__7___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_pool = python_pars[1];
    PyObject *par_url = python_pars[2];
    PyObject *par_retries = python_pars[3];
    PyObject *var_message = NULL;
    struct Nuitka_FrameObject *frame_c4818851a6696683b8f85d2a009b0859;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_c4818851a6696683b8f85d2a009b0859 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_c4818851a6696683b8f85d2a009b0859)) {
        Py_XDECREF(cache_frame_c4818851a6696683b8f85d2a009b0859);

#if _DEBUG_REFCOUNTS
        if (cache_frame_c4818851a6696683b8f85d2a009b0859 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_c4818851a6696683b8f85d2a009b0859 = MAKE_FUNCTION_FRAME(codeobj_c4818851a6696683b8f85d2a009b0859, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_c4818851a6696683b8f85d2a009b0859->m_type_description == NULL);
    frame_c4818851a6696683b8f85d2a009b0859 = cache_frame_c4818851a6696683b8f85d2a009b0859;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_c4818851a6696683b8f85d2a009b0859);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_c4818851a6696683b8f85d2a009b0859) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        tmp_left_name_1 = mod_consts[13];
        CHECK_OBJECT(par_url);
        tmp_right_name_1 = par_url;
        tmp_assign_source_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_OBJECT(tmp_left_name_1, tmp_right_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 99;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        assert(var_message == NULL);
        var_message = tmp_assign_source_1;
    }
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        PyObject *tmp_args_element_name_4;
        tmp_expression_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[12]);

        if (unlikely(tmp_expression_name_1 == NULL)) {
            tmp_expression_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
        }

        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 100;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[2]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 100;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_args_element_name_1 = par_self;
        CHECK_OBJECT(par_pool);
        tmp_args_element_name_2 = par_pool;
        if (par_url == NULL) {
            Py_DECREF(tmp_called_name_1);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[5]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 100;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }

        tmp_args_element_name_3 = par_url;
        CHECK_OBJECT(var_message);
        tmp_args_element_name_4 = var_message;
        frame_c4818851a6696683b8f85d2a009b0859->m_frame.f_lineno = 100;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2, tmp_args_element_name_3, tmp_args_element_name_4};
            tmp_call_result_1 = CALL_FUNCTION_WITH_ARGS4(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_called_name_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 100;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        CHECK_OBJECT(par_retries);
        tmp_assattr_name_1 = par_retries;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[7]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 101;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }

        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[14], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 101;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c4818851a6696683b8f85d2a009b0859);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c4818851a6696683b8f85d2a009b0859);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_c4818851a6696683b8f85d2a009b0859, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_c4818851a6696683b8f85d2a009b0859->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_c4818851a6696683b8f85d2a009b0859, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_c4818851a6696683b8f85d2a009b0859,
        type_description_1,
        par_self,
        par_pool,
        par_url,
        par_retries,
        var_message
    );


    // Release cached frame if used for exception.
    if (frame_c4818851a6696683b8f85d2a009b0859 == cache_frame_c4818851a6696683b8f85d2a009b0859) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_c4818851a6696683b8f85d2a009b0859);
        cache_frame_c4818851a6696683b8f85d2a009b0859 = NULL;
    }

    assertFrameObject(frame_c4818851a6696683b8f85d2a009b0859);

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
    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_pool);
    par_pool = NULL;
    Py_XDECREF(par_url);
    par_url = NULL;
    CHECK_OBJECT(par_retries);
    Py_DECREF(par_retries);
    par_retries = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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

    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_pool);
    par_pool = NULL;
    Py_XDECREF(par_url);
    par_url = NULL;
    CHECK_OBJECT(par_retries);
    Py_DECREF(par_retries);
    par_retries = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__8___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_location = python_pars[1];
    PyObject *var_message = NULL;
    struct Nuitka_FrameObject *frame_201187fceac633fe0ddb8cf64eed80a7;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_201187fceac633fe0ddb8cf64eed80a7 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_201187fceac633fe0ddb8cf64eed80a7)) {
        Py_XDECREF(cache_frame_201187fceac633fe0ddb8cf64eed80a7);

#if _DEBUG_REFCOUNTS
        if (cache_frame_201187fceac633fe0ddb8cf64eed80a7 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_201187fceac633fe0ddb8cf64eed80a7 = MAKE_FUNCTION_FRAME(codeobj_201187fceac633fe0ddb8cf64eed80a7, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_201187fceac633fe0ddb8cf64eed80a7->m_type_description == NULL);
    frame_201187fceac633fe0ddb8cf64eed80a7 = cache_frame_201187fceac633fe0ddb8cf64eed80a7;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_201187fceac633fe0ddb8cf64eed80a7);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_201187fceac633fe0ddb8cf64eed80a7) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        tmp_left_name_1 = mod_consts[15];
        CHECK_OBJECT(par_location);
        tmp_right_name_1 = par_location;
        tmp_assign_source_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_OBJECT(tmp_left_name_1, tmp_right_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 162;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
        assert(var_message == NULL);
        var_message = tmp_assign_source_1;
    }
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        tmp_called_instance_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_called_instance_1 == NULL)) {
            tmp_called_instance_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_args_element_name_1 = par_self;
        CHECK_OBJECT(var_message);
        tmp_args_element_name_2 = var_message;
        frame_201187fceac633fe0ddb8cf64eed80a7->m_frame.f_lineno = 163;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_call_result_1 = CALL_METHOD_WITH_ARGS2(
                tmp_called_instance_1,
                mod_consts[2],
                call_args
            );
        }

        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 163;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        if (par_location == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[16]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 165;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }

        tmp_assattr_name_1 = par_location;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[7]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 165;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }

        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[16], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 165;
            type_description_1 = "ooo";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_201187fceac633fe0ddb8cf64eed80a7);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_201187fceac633fe0ddb8cf64eed80a7);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_201187fceac633fe0ddb8cf64eed80a7, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_201187fceac633fe0ddb8cf64eed80a7->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_201187fceac633fe0ddb8cf64eed80a7, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_201187fceac633fe0ddb8cf64eed80a7,
        type_description_1,
        par_self,
        par_location,
        var_message
    );


    // Release cached frame if used for exception.
    if (frame_201187fceac633fe0ddb8cf64eed80a7 == cache_frame_201187fceac633fe0ddb8cf64eed80a7) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_201187fceac633fe0ddb8cf64eed80a7);
        cache_frame_201187fceac633fe0ddb8cf64eed80a7 = NULL;
    }

    assertFrameObject(frame_201187fceac633fe0ddb8cf64eed80a7);

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
    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_location);
    par_location = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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

    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_location);
    par_location = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__9___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_scheme = python_pars[1];
    PyObject *var_message = NULL;
    struct Nuitka_FrameObject *frame_71824b5007f999f47c76f01b04b4a7ec;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_71824b5007f999f47c76f01b04b4a7ec = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_71824b5007f999f47c76f01b04b4a7ec)) {
        Py_XDECREF(cache_frame_71824b5007f999f47c76f01b04b4a7ec);

#if _DEBUG_REFCOUNTS
        if (cache_frame_71824b5007f999f47c76f01b04b4a7ec == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_71824b5007f999f47c76f01b04b4a7ec = MAKE_FUNCTION_FRAME(codeobj_71824b5007f999f47c76f01b04b4a7ec, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_71824b5007f999f47c76f01b04b4a7ec->m_type_description == NULL);
    frame_71824b5007f999f47c76f01b04b4a7ec = cache_frame_71824b5007f999f47c76f01b04b4a7ec;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_71824b5007f999f47c76f01b04b4a7ec);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_71824b5007f999f47c76f01b04b4a7ec) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        tmp_left_name_1 = mod_consts[17];
        CHECK_OBJECT(par_scheme);
        tmp_right_name_1 = par_scheme;
        tmp_assign_source_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_OBJECT(tmp_left_name_1, tmp_right_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 172;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        assert(var_message == NULL);
        var_message = tmp_assign_source_1;
    }
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_type_arg_name_1;
        PyObject *tmp_object_arg_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        tmp_type_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[18]);

        if (unlikely(tmp_type_arg_name_1 == NULL)) {
            tmp_type_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[18]);
        }

        if (tmp_type_arg_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 173;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_object_arg_name_1 = par_self;
        tmp_called_instance_1 = BUILTIN_SUPER2(tmp_type_arg_name_1, tmp_object_arg_name_1);
        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 173;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(var_message);
        tmp_args_element_name_1 = var_message;
        frame_71824b5007f999f47c76f01b04b4a7ec->m_frame.f_lineno = 173;
        tmp_call_result_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[2], tmp_args_element_name_1);
        Py_DECREF(tmp_called_instance_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 173;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        if (par_scheme == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 175;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }

        tmp_assattr_name_1 = par_scheme;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[19], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 175;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_71824b5007f999f47c76f01b04b4a7ec);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_71824b5007f999f47c76f01b04b4a7ec);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_71824b5007f999f47c76f01b04b4a7ec, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_71824b5007f999f47c76f01b04b4a7ec->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_71824b5007f999f47c76f01b04b4a7ec, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_71824b5007f999f47c76f01b04b4a7ec,
        type_description_1,
        par_self,
        par_scheme,
        var_message,
        NULL
    );


    // Release cached frame if used for exception.
    if (frame_71824b5007f999f47c76f01b04b4a7ec == cache_frame_71824b5007f999f47c76f01b04b4a7ec) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_71824b5007f999f47c76f01b04b4a7ec);
        cache_frame_71824b5007f999f47c76f01b04b4a7ec = NULL;
    }

    assertFrameObject(frame_71824b5007f999f47c76f01b04b4a7ec);

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
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_scheme);
    par_scheme = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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
    Py_XDECREF(par_scheme);
    par_scheme = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__10___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_partial = python_pars[1];
    PyObject *par_expected = python_pars[2];
    struct Nuitka_FrameObject *frame_789fae4f29ca82fe44ad83242d8d23ec;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_789fae4f29ca82fe44ad83242d8d23ec = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_789fae4f29ca82fe44ad83242d8d23ec)) {
        Py_XDECREF(cache_frame_789fae4f29ca82fe44ad83242d8d23ec);

#if _DEBUG_REFCOUNTS
        if (cache_frame_789fae4f29ca82fe44ad83242d8d23ec == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_789fae4f29ca82fe44ad83242d8d23ec = MAKE_FUNCTION_FRAME(codeobj_789fae4f29ca82fe44ad83242d8d23ec, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_789fae4f29ca82fe44ad83242d8d23ec->m_type_description == NULL);
    frame_789fae4f29ca82fe44ad83242d8d23ec = cache_frame_789fae4f29ca82fe44ad83242d8d23ec;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_789fae4f29ca82fe44ad83242d8d23ec);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_789fae4f29ca82fe44ad83242d8d23ec) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_type_arg_name_1;
        PyObject *tmp_object_arg_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        tmp_type_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[20]);

        if (unlikely(tmp_type_arg_name_1 == NULL)) {
            tmp_type_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[20]);
        }

        if (tmp_type_arg_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 254;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_object_arg_name_1 = par_self;
        tmp_called_instance_1 = BUILTIN_SUPER2(tmp_type_arg_name_1, tmp_object_arg_name_1);
        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 254;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_partial);
        tmp_args_element_name_1 = par_partial;
        CHECK_OBJECT(par_expected);
        tmp_args_element_name_2 = par_expected;
        frame_789fae4f29ca82fe44ad83242d8d23ec->m_frame.f_lineno = 254;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_call_result_1 = CALL_METHOD_WITH_ARGS2(
                tmp_called_instance_1,
                mod_consts[2],
                call_args
            );
        }

        Py_DECREF(tmp_called_instance_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 254;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_789fae4f29ca82fe44ad83242d8d23ec);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_789fae4f29ca82fe44ad83242d8d23ec);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_789fae4f29ca82fe44ad83242d8d23ec, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_789fae4f29ca82fe44ad83242d8d23ec->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_789fae4f29ca82fe44ad83242d8d23ec, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_789fae4f29ca82fe44ad83242d8d23ec,
        type_description_1,
        par_self,
        par_partial,
        par_expected,
        NULL
    );


    // Release cached frame if used for exception.
    if (frame_789fae4f29ca82fe44ad83242d8d23ec == cache_frame_789fae4f29ca82fe44ad83242d8d23ec) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_789fae4f29ca82fe44ad83242d8d23ec);
        cache_frame_789fae4f29ca82fe44ad83242d8d23ec = NULL;
    }

    assertFrameObject(frame_789fae4f29ca82fe44ad83242d8d23ec);

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
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_partial);
    par_partial = NULL;
    Py_XDECREF(par_expected);
    par_expected = NULL;
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
    Py_XDECREF(par_partial);
    par_partial = NULL;
    Py_XDECREF(par_expected);
    par_expected = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__11___repr__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    struct Nuitka_FrameObject *frame_7163e59336e1ab5575db0a821461ddb9;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_7163e59336e1ab5575db0a821461ddb9 = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_7163e59336e1ab5575db0a821461ddb9)) {
        Py_XDECREF(cache_frame_7163e59336e1ab5575db0a821461ddb9);

#if _DEBUG_REFCOUNTS
        if (cache_frame_7163e59336e1ab5575db0a821461ddb9 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_7163e59336e1ab5575db0a821461ddb9 = MAKE_FUNCTION_FRAME(codeobj_7163e59336e1ab5575db0a821461ddb9, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_7163e59336e1ab5575db0a821461ddb9->m_type_description == NULL);
    frame_7163e59336e1ab5575db0a821461ddb9 = cache_frame_7163e59336e1ab5575db0a821461ddb9;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_7163e59336e1ab5575db0a821461ddb9);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_7163e59336e1ab5575db0a821461ddb9) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_expression_name_1;
        tmp_left_name_1 = mod_consts[21];
        CHECK_OBJECT(par_self);
        tmp_expression_name_1 = par_self;
        tmp_tuple_element_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[22]);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 258;
            type_description_1 = "o";
            goto frame_exception_exit_1;
        }
        tmp_right_name_1 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_2;
            PyTuple_SET_ITEM(tmp_right_name_1, 0, tmp_tuple_element_1);
            if (par_self == NULL) {

                FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[7]);
                exception_tb = NULL;
                NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
                CHAIN_EXCEPTION(exception_value);

                exception_lineno = 259;
                type_description_1 = "o";
                goto tuple_build_exception_1;
            }

            tmp_expression_name_2 = par_self;
            tmp_tuple_element_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[23]);
            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 259;
                type_description_1 = "o";
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
        tmp_return_value = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 257;
            type_description_1 = "o";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_7163e59336e1ab5575db0a821461ddb9);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_7163e59336e1ab5575db0a821461ddb9);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_7163e59336e1ab5575db0a821461ddb9);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_7163e59336e1ab5575db0a821461ddb9, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_7163e59336e1ab5575db0a821461ddb9->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_7163e59336e1ab5575db0a821461ddb9, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_7163e59336e1ab5575db0a821461ddb9,
        type_description_1,
        par_self
    );


    // Release cached frame if used for exception.
    if (frame_7163e59336e1ab5575db0a821461ddb9 == cache_frame_7163e59336e1ab5575db0a821461ddb9) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_7163e59336e1ab5575db0a821461ddb9);
        cache_frame_7163e59336e1ab5575db0a821461ddb9 = NULL;
    }

    assertFrameObject(frame_7163e59336e1ab5575db0a821461ddb9);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    Py_XDECREF(par_self);
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

    Py_XDECREF(par_self);
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__12___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_response = python_pars[1];
    PyObject *par_length = python_pars[2];
    struct Nuitka_FrameObject *frame_98ee57df943e033264d984d9d9bfbb6d;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_98ee57df943e033264d984d9d9bfbb6d = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_98ee57df943e033264d984d9d9bfbb6d)) {
        Py_XDECREF(cache_frame_98ee57df943e033264d984d9d9bfbb6d);

#if _DEBUG_REFCOUNTS
        if (cache_frame_98ee57df943e033264d984d9d9bfbb6d == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_98ee57df943e033264d984d9d9bfbb6d = MAKE_FUNCTION_FRAME(codeobj_98ee57df943e033264d984d9d9bfbb6d, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_98ee57df943e033264d984d9d9bfbb6d->m_type_description == NULL);
    frame_98ee57df943e033264d984d9d9bfbb6d = cache_frame_98ee57df943e033264d984d9d9bfbb6d;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_98ee57df943e033264d984d9d9bfbb6d);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_98ee57df943e033264d984d9d9bfbb6d) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_type_arg_name_1;
        PyObject *tmp_object_arg_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_expression_name_2;
        tmp_type_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[24]);

        if (unlikely(tmp_type_arg_name_1 == NULL)) {
            tmp_type_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[24]);
        }

        if (tmp_type_arg_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 267;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_object_arg_name_1 = par_self;
        tmp_expression_name_1 = BUILTIN_SUPER2(tmp_type_arg_name_1, tmp_object_arg_name_1);
        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 267;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[2]);
        Py_DECREF(tmp_expression_name_1);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 267;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_response);
        tmp_called_instance_1 = par_response;
        frame_98ee57df943e033264d984d9d9bfbb6d->m_frame.f_lineno = 268;
        tmp_args_element_name_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_1, mod_consts[25]);
        if (tmp_args_element_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 268;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        if (par_response == NULL) {
            Py_DECREF(tmp_called_name_1);
            Py_DECREF(tmp_args_element_name_1);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[26]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 268;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_2 = par_response;
        tmp_args_element_name_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[27]);
        if (tmp_args_element_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);
            Py_DECREF(tmp_args_element_name_1);

            exception_lineno = 268;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        frame_98ee57df943e033264d984d9d9bfbb6d->m_frame.f_lineno = 267;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_call_result_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_called_name_1);
        Py_DECREF(tmp_args_element_name_1);
        Py_DECREF(tmp_args_element_name_2);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 267;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        if (par_response == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[26]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 270;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }

        tmp_assattr_name_1 = par_response;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[26], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 270;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_2;
        PyObject *tmp_assattr_target_2;
        CHECK_OBJECT(par_length);
        tmp_assattr_name_2 = par_length;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_2 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[28], tmp_assattr_name_2);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 271;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_98ee57df943e033264d984d9d9bfbb6d);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_98ee57df943e033264d984d9d9bfbb6d);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_98ee57df943e033264d984d9d9bfbb6d, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_98ee57df943e033264d984d9d9bfbb6d->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_98ee57df943e033264d984d9d9bfbb6d, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_98ee57df943e033264d984d9d9bfbb6d,
        type_description_1,
        par_self,
        par_response,
        par_length,
        NULL
    );


    // Release cached frame if used for exception.
    if (frame_98ee57df943e033264d984d9d9bfbb6d == cache_frame_98ee57df943e033264d984d9d9bfbb6d) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_98ee57df943e033264d984d9d9bfbb6d);
        cache_frame_98ee57df943e033264d984d9d9bfbb6d = NULL;
    }

    assertFrameObject(frame_98ee57df943e033264d984d9d9bfbb6d);

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
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_response);
    par_response = NULL;
    CHECK_OBJECT(par_length);
    Py_DECREF(par_length);
    par_length = NULL;
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
    Py_XDECREF(par_response);
    par_response = NULL;
    CHECK_OBJECT(par_length);
    Py_DECREF(par_length);
    par_length = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__13___repr__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    struct Nuitka_FrameObject *frame_208f80491e1b413f20da0e11d5be6e91;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_208f80491e1b413f20da0e11d5be6e91 = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_208f80491e1b413f20da0e11d5be6e91)) {
        Py_XDECREF(cache_frame_208f80491e1b413f20da0e11d5be6e91);

#if _DEBUG_REFCOUNTS
        if (cache_frame_208f80491e1b413f20da0e11d5be6e91 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_208f80491e1b413f20da0e11d5be6e91 = MAKE_FUNCTION_FRAME(codeobj_208f80491e1b413f20da0e11d5be6e91, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_208f80491e1b413f20da0e11d5be6e91->m_type_description == NULL);
    frame_208f80491e1b413f20da0e11d5be6e91 = cache_frame_208f80491e1b413f20da0e11d5be6e91;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_208f80491e1b413f20da0e11d5be6e91);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_208f80491e1b413f20da0e11d5be6e91) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_expression_name_1;
        tmp_left_name_1 = mod_consts[29];
        CHECK_OBJECT(par_self);
        tmp_expression_name_1 = par_self;
        tmp_tuple_element_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[28]);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 275;
            type_description_1 = "o";
            goto frame_exception_exit_1;
        }
        tmp_right_name_1 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_2;
            PyTuple_SET_ITEM(tmp_right_name_1, 0, tmp_tuple_element_1);
            if (par_self == NULL) {

                FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[7]);
                exception_tb = NULL;
                NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
                CHAIN_EXCEPTION(exception_value);

                exception_lineno = 276;
                type_description_1 = "o";
                goto tuple_build_exception_1;
            }

            tmp_expression_name_2 = par_self;
            tmp_tuple_element_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[22]);
            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 276;
                type_description_1 = "o";
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
        tmp_return_value = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 274;
            type_description_1 = "o";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_208f80491e1b413f20da0e11d5be6e91);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_208f80491e1b413f20da0e11d5be6e91);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_208f80491e1b413f20da0e11d5be6e91);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_208f80491e1b413f20da0e11d5be6e91, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_208f80491e1b413f20da0e11d5be6e91->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_208f80491e1b413f20da0e11d5be6e91, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_208f80491e1b413f20da0e11d5be6e91,
        type_description_1,
        par_self
    );


    // Release cached frame if used for exception.
    if (frame_208f80491e1b413f20da0e11d5be6e91 == cache_frame_208f80491e1b413f20da0e11d5be6e91) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_208f80491e1b413f20da0e11d5be6e91);
        cache_frame_208f80491e1b413f20da0e11d5be6e91 = NULL;
    }

    assertFrameObject(frame_208f80491e1b413f20da0e11d5be6e91);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    Py_XDECREF(par_self);
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

    Py_XDECREF(par_self);
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__14___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_scheme = python_pars[1];
    PyObject *var_message = NULL;
    struct Nuitka_FrameObject *frame_2a30d9dcbcedcf3e6be926983f3c62ce;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce)) {
        Py_XDECREF(cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce);

#if _DEBUG_REFCOUNTS
        if (cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce = MAKE_FUNCTION_FRAME(codeobj_2a30d9dcbcedcf3e6be926983f3c62ce, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce->m_type_description == NULL);
    frame_2a30d9dcbcedcf3e6be926983f3c62ce = cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_2a30d9dcbcedcf3e6be926983f3c62ce);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_2a30d9dcbcedcf3e6be926983f3c62ce) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        tmp_left_name_1 = mod_consts[30];
        CHECK_OBJECT(par_scheme);
        tmp_right_name_1 = par_scheme;
        tmp_assign_source_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_OBJECT(tmp_left_name_1, tmp_right_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 292;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        assert(var_message == NULL);
        var_message = tmp_assign_source_1;
    }
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_type_arg_name_1;
        PyObject *tmp_object_arg_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        tmp_type_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[31]);

        if (unlikely(tmp_type_arg_name_1 == NULL)) {
            tmp_type_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[31]);
        }

        if (tmp_type_arg_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 293;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_object_arg_name_1 = par_self;
        tmp_called_instance_1 = BUILTIN_SUPER2(tmp_type_arg_name_1, tmp_object_arg_name_1);
        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 293;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(var_message);
        tmp_args_element_name_1 = var_message;
        frame_2a30d9dcbcedcf3e6be926983f3c62ce->m_frame.f_lineno = 293;
        tmp_call_result_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[2], tmp_args_element_name_1);
        Py_DECREF(tmp_called_instance_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 293;
            type_description_1 = "oooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_2a30d9dcbcedcf3e6be926983f3c62ce);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_2a30d9dcbcedcf3e6be926983f3c62ce);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_2a30d9dcbcedcf3e6be926983f3c62ce, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_2a30d9dcbcedcf3e6be926983f3c62ce->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_2a30d9dcbcedcf3e6be926983f3c62ce, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_2a30d9dcbcedcf3e6be926983f3c62ce,
        type_description_1,
        par_self,
        par_scheme,
        var_message,
        NULL
    );


    // Release cached frame if used for exception.
    if (frame_2a30d9dcbcedcf3e6be926983f3c62ce == cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce);
        cache_frame_2a30d9dcbcedcf3e6be926983f3c62ce = NULL;
    }

    assertFrameObject(frame_2a30d9dcbcedcf3e6be926983f3c62ce);

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
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_scheme);
    par_scheme = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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
    Py_XDECREF(par_scheme);
    par_scheme = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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


static PyObject *impl_pip$_vendor$urllib3$exceptions$$$function__15___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_defects = python_pars[1];
    PyObject *par_unparsed_data = python_pars[2];
    PyObject *var_message = NULL;
    struct Nuitka_FrameObject *frame_59493ff7ab18b1206e1b870f33336d38;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_frame_59493ff7ab18b1206e1b870f33336d38 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_59493ff7ab18b1206e1b870f33336d38)) {
        Py_XDECREF(cache_frame_59493ff7ab18b1206e1b870f33336d38);

#if _DEBUG_REFCOUNTS
        if (cache_frame_59493ff7ab18b1206e1b870f33336d38 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_59493ff7ab18b1206e1b870f33336d38 = MAKE_FUNCTION_FRAME(codeobj_59493ff7ab18b1206e1b870f33336d38, module_pip$_vendor$urllib3$exceptions, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_59493ff7ab18b1206e1b870f33336d38->m_type_description == NULL);
    frame_59493ff7ab18b1206e1b870f33336d38 = cache_frame_59493ff7ab18b1206e1b870f33336d38;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_59493ff7ab18b1206e1b870f33336d38);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_59493ff7ab18b1206e1b870f33336d38) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_tuple_element_1;
        int tmp_or_left_truth_1;
        PyObject *tmp_or_left_value_1;
        PyObject *tmp_or_right_value_1;
        tmp_left_name_1 = mod_consts[32];
        CHECK_OBJECT(par_defects);
        tmp_or_left_value_1 = par_defects;
        tmp_or_left_truth_1 = CHECK_IF_TRUE(tmp_or_left_value_1);
        if (tmp_or_left_truth_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 306;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        if (tmp_or_left_truth_1 == 1) {
            goto or_left_1;
        } else {
            goto or_right_1;
        }
        or_right_1:;
        tmp_or_right_value_1 = mod_consts[33];
        tmp_tuple_element_1 = tmp_or_right_value_1;
        goto or_end_1;
        or_left_1:;
        tmp_tuple_element_1 = tmp_or_left_value_1;
        or_end_1:;
        tmp_right_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_right_name_1, 0, tmp_tuple_element_1);
        CHECK_OBJECT(par_unparsed_data);
        tmp_tuple_element_1 = par_unparsed_data;
        PyTuple_SET_ITEM0(tmp_right_name_1, 1, tmp_tuple_element_1);
        tmp_assign_source_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 306;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        assert(var_message == NULL);
        var_message = tmp_assign_source_1;
    }
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_type_arg_name_1;
        PyObject *tmp_object_arg_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        tmp_type_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[34]);

        if (unlikely(tmp_type_arg_name_1 == NULL)) {
            tmp_type_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[34]);
        }

        if (tmp_type_arg_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 307;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_object_arg_name_1 = par_self;
        tmp_called_instance_1 = BUILTIN_SUPER2(tmp_type_arg_name_1, tmp_object_arg_name_1);
        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 307;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(var_message);
        tmp_args_element_name_1 = var_message;
        frame_59493ff7ab18b1206e1b870f33336d38->m_frame.f_lineno = 307;
        tmp_call_result_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[2], tmp_args_element_name_1);
        Py_DECREF(tmp_called_instance_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 307;
            type_description_1 = "ooooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_59493ff7ab18b1206e1b870f33336d38);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_59493ff7ab18b1206e1b870f33336d38);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_59493ff7ab18b1206e1b870f33336d38, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_59493ff7ab18b1206e1b870f33336d38->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_59493ff7ab18b1206e1b870f33336d38, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_59493ff7ab18b1206e1b870f33336d38,
        type_description_1,
        par_self,
        par_defects,
        par_unparsed_data,
        var_message,
        NULL
    );


    // Release cached frame if used for exception.
    if (frame_59493ff7ab18b1206e1b870f33336d38 == cache_frame_59493ff7ab18b1206e1b870f33336d38) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_59493ff7ab18b1206e1b870f33336d38);
        cache_frame_59493ff7ab18b1206e1b870f33336d38 = NULL;
    }

    assertFrameObject(frame_59493ff7ab18b1206e1b870f33336d38);

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
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    CHECK_OBJECT(par_defects);
    Py_DECREF(par_defects);
    par_defects = NULL;
    CHECK_OBJECT(par_unparsed_data);
    Py_DECREF(par_unparsed_data);
    par_unparsed_data = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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
    CHECK_OBJECT(par_defects);
    Py_DECREF(par_defects);
    par_defects = NULL;
    CHECK_OBJECT(par_unparsed_data);
    Py_DECREF(par_unparsed_data);
    par_unparsed_data = NULL;
    Py_XDECREF(var_message);
    var_message = NULL;
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



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__10___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__10___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[132],
#endif
        codeobj_789fae4f29ca82fe44ad83242d8d23ec,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__11___repr__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__11___repr__,
        mod_consts[133],
#if PYTHON_VERSION >= 0x300
        mod_consts[134],
#endif
        codeobj_7163e59336e1ab5575db0a821461ddb9,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__12___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__12___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[136],
#endif
        codeobj_98ee57df943e033264d984d9d9bfbb6d,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__13___repr__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__13___repr__,
        mod_consts[133],
#if PYTHON_VERSION >= 0x300
        mod_consts[137],
#endif
        codeobj_208f80491e1b413f20da0e11d5be6e91,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__14___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__14___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[141],
#endif
        codeobj_2a30d9dcbcedcf3e6be926983f3c62ce,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__15___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__15___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[146],
#endif
        codeobj_59493ff7ab18b1206e1b870f33336d38,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__1___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__1___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[63],
#endif
        codeobj_99eacdb6046efa487fa94a7a939ebbc3,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__2___reduce__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__2___reduce__,
        mod_consts[64],
#if PYTHON_VERSION >= 0x300
        mod_consts[65],
#endif
        codeobj_9efec6d4ef9615db68fe3db532fafbec,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__3___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__3___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[67],
#endif
        codeobj_df4920181a454a81b8219ced28b38ee7,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__4___reduce__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__4___reduce__,
        mod_consts[64],
#if PYTHON_VERSION >= 0x300
        mod_consts[68],
#endif
        codeobj_195b6cb8f13f233cfa7abbadc4ef4ae7,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__5___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__5___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[72],
#endif
        codeobj_13ccd233d3c49c0deeb5578d5d988a97,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__6___init__(PyObject *defaults) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__6___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[81],
#endif
        codeobj_6b4910f8e728dcfd0803ea56207884a7,
        defaults,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__7___init__(PyObject *defaults) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__7___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[85],
#endif
        codeobj_c4818851a6696683b8f85d2a009b0859,
        defaults,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__8___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__8___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[104],
#endif
        codeobj_201187fceac633fe0ddb8cf64eed80a7,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__9___init__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$urllib3$exceptions$$$function__9___init__,
        mod_consts[2],
#if PYTHON_VERSION >= 0x300
        mod_consts[106],
#endif
        codeobj_71824b5007f999f47c76f01b04b4a7ec,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$urllib3$exceptions,
        NULL,
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

function_impl_code functable_pip$_vendor$urllib3$exceptions[] = {
    impl_pip$_vendor$urllib3$exceptions$$$function__1___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__2___reduce__,
    impl_pip$_vendor$urllib3$exceptions$$$function__3___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__4___reduce__,
    impl_pip$_vendor$urllib3$exceptions$$$function__5___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__6___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__7___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__8___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__9___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__10___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__11___repr__,
    impl_pip$_vendor$urllib3$exceptions$$$function__12___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__13___repr__,
    impl_pip$_vendor$urllib3$exceptions$$$function__14___init__,
    impl_pip$_vendor$urllib3$exceptions$$$function__15___init__,
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

    function_impl_code *current = functable_pip$_vendor$urllib3$exceptions;
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

    if (offset > sizeof(functable_pip$_vendor$urllib3$exceptions) || offset < 0) {
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
        functable_pip$_vendor$urllib3$exceptions[offset],
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
        module_pip$_vendor$urllib3$exceptions,
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
PyObject *modulecode_pip$_vendor$urllib3$exceptions(PyObject *module, struct Nuitka_MetaPathBasedLoaderEntry const *module_entry) {
    module_pip$_vendor$urllib3$exceptions = module;

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
    PRINT_STRING("pip._vendor.urllib3.exceptions: Calling setupMetaPathBasedLoader().\n");
#endif
    setupMetaPathBasedLoader();

#if PYTHON_VERSION >= 0x300
    patchInspectModule();
#endif

#endif

    /* The constants only used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.urllib3.exceptions: Calling createModuleConstants().\n");
#endif
    createModuleConstants();

    /* The code objects used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.urllib3.exceptions: Calling createModuleCodeObjects().\n");
#endif
    createModuleCodeObjects();

    // PRINT_STRING("in initpip$_vendor$urllib3$exceptions\n");

    // Create the module object first. There are no methods initially, all are
    // added dynamically in actual code only.  Also no "__doc__" is initially
    // set at this time, as it could not contain NUL characters this way, they
    // are instead set in early module code.  No "self" for modules, we have no
    // use for it.

    moduledict_pip$_vendor$urllib3$exceptions = MODULE_DICT(module_pip$_vendor$urllib3$exceptions);

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
    registerDillPluginTables(module_entry->name, &_method_def_reduce_compiled_function, &_method_def_create_compiled_function);
#endif

    // Set "__compiled__" to what version information we have.
    UPDATE_STRING_DICT0(
        moduledict_pip$_vendor$urllib3$exceptions,
        (Nuitka_StringObject *)const_str_plain___compiled__,
        Nuitka_dunder_compiled_value
    );

    // Update "__package__" value to what it ought to be.
    {
#if 0
        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$urllib3$exceptions,
            (Nuitka_StringObject *)const_str_plain___package__,
            const_str_empty
        );
#elif 0
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___name__);

        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$urllib3$exceptions,
            (Nuitka_StringObject *)const_str_plain___package__,
            module_name
        );
#else

#if PYTHON_VERSION < 0x300
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___name__);
        char const *module_name_cstr = PyString_AS_STRING(module_name);

        char const *last_dot = strrchr(module_name_cstr, '.');

        if (last_dot != NULL) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$urllib3$exceptions,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyString_FromStringAndSize(module_name_cstr, last_dot - module_name_cstr)
            );
        }
#else
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___name__);
        Py_ssize_t dot_index = PyUnicode_Find(module_name, const_str_dot, 0, PyUnicode_GetLength(module_name), -1);

        if (dot_index != -1) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$urllib3$exceptions,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyUnicode_Substring(module_name, 0, dot_index)
            );
        }
#endif
#endif
    }

    CHECK_OBJECT(module_pip$_vendor$urllib3$exceptions);

    // For deep importing of a module we need to have "__builtins__", so we set
    // it ourselves in the same way than CPython does. Note: This must be done
    // before the frame object is allocated, or else it may fail.

    if (GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___builtins__) == NULL) {
        PyObject *value = (PyObject *)builtin_module;

        // Check if main module, not a dict then but the module itself.
#if !defined(_NUITKA_EXE) || !0
        value = PyModule_GetDict(value);
#endif

        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___builtins__, value);
    }

#if PYTHON_VERSION >= 0x300
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___loader__, (PyObject *)&Nuitka_Loader_Type);
#endif

#if PYTHON_VERSION >= 0x340
// Set the "__spec__" value

#if 0
    // Main modules just get "None" as spec.
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___spec__, Py_None);
#else
    // Other modules get a "ModuleSpec" from the standard mechanism.
    {
        PyObject *bootstrap_module = getImportLibBootstrapModule();
        CHECK_OBJECT(bootstrap_module);

        PyObject *_spec_from_module = PyObject_GetAttrString(bootstrap_module, "_spec_from_module");
        CHECK_OBJECT(_spec_from_module);

        PyObject *spec_value = CALL_FUNCTION_WITH_SINGLE_ARG(_spec_from_module, module_pip$_vendor$urllib3$exceptions);
        Py_DECREF(_spec_from_module);

        // We can assume this to never fail, or else we are in trouble anyway.
        // CHECK_OBJECT(spec_value);

        if (spec_value == NULL) {
            PyErr_PrintEx(0);
            abort();
        }

// Mark the execution in the "__spec__" value.
        SET_ATTRIBUTE(spec_value, const_str_plain__initializing, Py_True);

        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)const_str_plain___spec__, spec_value);
    }
#endif
#endif

    // Temp variables if any
    PyObject *outline_0_var___class__ = NULL;
    PyObject *outline_1_var___class__ = NULL;
    PyObject *outline_2_var___class__ = NULL;
    PyObject *outline_3_var___class__ = NULL;
    PyObject *outline_4_var___class__ = NULL;
    PyObject *outline_5_var___class__ = NULL;
    PyObject *outline_6_var___class__ = NULL;
    PyObject *outline_7_var___class__ = NULL;
    PyObject *outline_8_var___class__ = NULL;
    PyObject *outline_9_var___class__ = NULL;
    PyObject *outline_10_var___class__ = NULL;
    PyObject *outline_11_var___class__ = NULL;
    PyObject *outline_12_var___class__ = NULL;
    PyObject *outline_13_var___class__ = NULL;
    PyObject *outline_14_var___class__ = NULL;
    PyObject *outline_15_var___class__ = NULL;
    PyObject *outline_16_var___class__ = NULL;
    PyObject *outline_17_var___class__ = NULL;
    PyObject *outline_18_var___class__ = NULL;
    PyObject *outline_19_var___class__ = NULL;
    PyObject *outline_20_var___class__ = NULL;
    PyObject *outline_21_var___class__ = NULL;
    PyObject *outline_22_var___class__ = NULL;
    PyObject *outline_23_var___class__ = NULL;
    PyObject *outline_24_var___class__ = NULL;
    PyObject *outline_25_var___class__ = NULL;
    PyObject *outline_26_var___class__ = NULL;
    PyObject *outline_27_var___class__ = NULL;
    PyObject *outline_28_var___class__ = NULL;
    PyObject *outline_29_var___class__ = NULL;
    PyObject *outline_30_var___class__ = NULL;
    PyObject *outline_31_var___class__ = NULL;
    PyObject *outline_32_var___class__ = NULL;
    PyObject *outline_33_var___class__ = NULL;
    PyObject *outline_34_var___class__ = NULL;
    PyObject *outline_35_var___class__ = NULL;
    PyObject *outline_36_var___class__ = NULL;
    PyObject *tmp_class_creation_10__bases = NULL;
    PyObject *tmp_class_creation_10__class_decl_dict = NULL;
    PyObject *tmp_class_creation_10__metaclass = NULL;
    PyObject *tmp_class_creation_10__prepared = NULL;
    PyObject *tmp_class_creation_11__bases = NULL;
    PyObject *tmp_class_creation_11__class_decl_dict = NULL;
    PyObject *tmp_class_creation_11__metaclass = NULL;
    PyObject *tmp_class_creation_11__prepared = NULL;
    PyObject *tmp_class_creation_12__bases = NULL;
    PyObject *tmp_class_creation_12__class_decl_dict = NULL;
    PyObject *tmp_class_creation_12__metaclass = NULL;
    PyObject *tmp_class_creation_12__prepared = NULL;
    PyObject *tmp_class_creation_13__bases = NULL;
    PyObject *tmp_class_creation_13__class_decl_dict = NULL;
    PyObject *tmp_class_creation_13__metaclass = NULL;
    PyObject *tmp_class_creation_13__prepared = NULL;
    PyObject *tmp_class_creation_14__bases = NULL;
    PyObject *tmp_class_creation_14__class_decl_dict = NULL;
    PyObject *tmp_class_creation_14__metaclass = NULL;
    PyObject *tmp_class_creation_14__prepared = NULL;
    PyObject *tmp_class_creation_15__bases = NULL;
    PyObject *tmp_class_creation_15__class_decl_dict = NULL;
    PyObject *tmp_class_creation_15__metaclass = NULL;
    PyObject *tmp_class_creation_15__prepared = NULL;
    PyObject *tmp_class_creation_16__bases = NULL;
    PyObject *tmp_class_creation_16__class_decl_dict = NULL;
    PyObject *tmp_class_creation_16__metaclass = NULL;
    PyObject *tmp_class_creation_16__prepared = NULL;
    PyObject *tmp_class_creation_17__bases = NULL;
    PyObject *tmp_class_creation_17__class_decl_dict = NULL;
    PyObject *tmp_class_creation_17__metaclass = NULL;
    PyObject *tmp_class_creation_17__prepared = NULL;
    PyObject *tmp_class_creation_18__bases = NULL;
    PyObject *tmp_class_creation_18__class_decl_dict = NULL;
    PyObject *tmp_class_creation_18__metaclass = NULL;
    PyObject *tmp_class_creation_18__prepared = NULL;
    PyObject *tmp_class_creation_19__bases = NULL;
    PyObject *tmp_class_creation_19__class_decl_dict = NULL;
    PyObject *tmp_class_creation_19__metaclass = NULL;
    PyObject *tmp_class_creation_19__prepared = NULL;
    PyObject *tmp_class_creation_1__bases = NULL;
    PyObject *tmp_class_creation_1__class_decl_dict = NULL;
    PyObject *tmp_class_creation_1__metaclass = NULL;
    PyObject *tmp_class_creation_1__prepared = NULL;
    PyObject *tmp_class_creation_20__bases = NULL;
    PyObject *tmp_class_creation_20__class_decl_dict = NULL;
    PyObject *tmp_class_creation_20__metaclass = NULL;
    PyObject *tmp_class_creation_20__prepared = NULL;
    PyObject *tmp_class_creation_21__bases = NULL;
    PyObject *tmp_class_creation_21__class_decl_dict = NULL;
    PyObject *tmp_class_creation_21__metaclass = NULL;
    PyObject *tmp_class_creation_21__prepared = NULL;
    PyObject *tmp_class_creation_22__bases = NULL;
    PyObject *tmp_class_creation_22__class_decl_dict = NULL;
    PyObject *tmp_class_creation_22__metaclass = NULL;
    PyObject *tmp_class_creation_22__prepared = NULL;
    PyObject *tmp_class_creation_23__bases = NULL;
    PyObject *tmp_class_creation_23__class_decl_dict = NULL;
    PyObject *tmp_class_creation_23__metaclass = NULL;
    PyObject *tmp_class_creation_23__prepared = NULL;
    PyObject *tmp_class_creation_24__bases = NULL;
    PyObject *tmp_class_creation_24__class_decl_dict = NULL;
    PyObject *tmp_class_creation_24__metaclass = NULL;
    PyObject *tmp_class_creation_24__prepared = NULL;
    PyObject *tmp_class_creation_25__bases = NULL;
    PyObject *tmp_class_creation_25__class_decl_dict = NULL;
    PyObject *tmp_class_creation_25__metaclass = NULL;
    PyObject *tmp_class_creation_25__prepared = NULL;
    PyObject *tmp_class_creation_26__bases = NULL;
    PyObject *tmp_class_creation_26__class_decl_dict = NULL;
    PyObject *tmp_class_creation_26__metaclass = NULL;
    PyObject *tmp_class_creation_26__prepared = NULL;
    PyObject *tmp_class_creation_27__bases = NULL;
    PyObject *tmp_class_creation_27__class_decl_dict = NULL;
    PyObject *tmp_class_creation_27__metaclass = NULL;
    PyObject *tmp_class_creation_27__prepared = NULL;
    PyObject *tmp_class_creation_28__bases = NULL;
    PyObject *tmp_class_creation_28__class_decl_dict = NULL;
    PyObject *tmp_class_creation_28__metaclass = NULL;
    PyObject *tmp_class_creation_28__prepared = NULL;
    PyObject *tmp_class_creation_29__bases = NULL;
    PyObject *tmp_class_creation_29__class_decl_dict = NULL;
    PyObject *tmp_class_creation_29__metaclass = NULL;
    PyObject *tmp_class_creation_29__prepared = NULL;
    PyObject *tmp_class_creation_2__bases = NULL;
    PyObject *tmp_class_creation_2__class_decl_dict = NULL;
    PyObject *tmp_class_creation_2__metaclass = NULL;
    PyObject *tmp_class_creation_2__prepared = NULL;
    PyObject *tmp_class_creation_30__bases = NULL;
    PyObject *tmp_class_creation_30__class_decl_dict = NULL;
    PyObject *tmp_class_creation_30__metaclass = NULL;
    PyObject *tmp_class_creation_30__prepared = NULL;
    PyObject *tmp_class_creation_31__bases = NULL;
    PyObject *tmp_class_creation_31__class_decl_dict = NULL;
    PyObject *tmp_class_creation_31__metaclass = NULL;
    PyObject *tmp_class_creation_31__prepared = NULL;
    PyObject *tmp_class_creation_32__bases = NULL;
    PyObject *tmp_class_creation_32__class_decl_dict = NULL;
    PyObject *tmp_class_creation_32__metaclass = NULL;
    PyObject *tmp_class_creation_32__prepared = NULL;
    PyObject *tmp_class_creation_33__bases = NULL;
    PyObject *tmp_class_creation_33__class_decl_dict = NULL;
    PyObject *tmp_class_creation_33__metaclass = NULL;
    PyObject *tmp_class_creation_33__prepared = NULL;
    PyObject *tmp_class_creation_34__bases = NULL;
    PyObject *tmp_class_creation_34__class_decl_dict = NULL;
    PyObject *tmp_class_creation_34__metaclass = NULL;
    PyObject *tmp_class_creation_34__prepared = NULL;
    PyObject *tmp_class_creation_35__bases = NULL;
    PyObject *tmp_class_creation_35__class_decl_dict = NULL;
    PyObject *tmp_class_creation_35__metaclass = NULL;
    PyObject *tmp_class_creation_35__prepared = NULL;
    PyObject *tmp_class_creation_36__bases = NULL;
    PyObject *tmp_class_creation_36__class_decl_dict = NULL;
    PyObject *tmp_class_creation_36__metaclass = NULL;
    PyObject *tmp_class_creation_36__prepared = NULL;
    PyObject *tmp_class_creation_37__bases = NULL;
    PyObject *tmp_class_creation_37__class_decl_dict = NULL;
    PyObject *tmp_class_creation_37__metaclass = NULL;
    PyObject *tmp_class_creation_37__prepared = NULL;
    PyObject *tmp_class_creation_3__bases = NULL;
    PyObject *tmp_class_creation_3__class_decl_dict = NULL;
    PyObject *tmp_class_creation_3__metaclass = NULL;
    PyObject *tmp_class_creation_3__prepared = NULL;
    PyObject *tmp_class_creation_4__bases = NULL;
    PyObject *tmp_class_creation_4__class_decl_dict = NULL;
    PyObject *tmp_class_creation_4__metaclass = NULL;
    PyObject *tmp_class_creation_4__prepared = NULL;
    PyObject *tmp_class_creation_5__bases = NULL;
    PyObject *tmp_class_creation_5__class_decl_dict = NULL;
    PyObject *tmp_class_creation_5__metaclass = NULL;
    PyObject *tmp_class_creation_5__prepared = NULL;
    PyObject *tmp_class_creation_6__bases = NULL;
    PyObject *tmp_class_creation_6__class_decl_dict = NULL;
    PyObject *tmp_class_creation_6__metaclass = NULL;
    PyObject *tmp_class_creation_6__prepared = NULL;
    PyObject *tmp_class_creation_7__bases = NULL;
    PyObject *tmp_class_creation_7__class_decl_dict = NULL;
    PyObject *tmp_class_creation_7__metaclass = NULL;
    PyObject *tmp_class_creation_7__prepared = NULL;
    PyObject *tmp_class_creation_8__bases = NULL;
    PyObject *tmp_class_creation_8__class_decl_dict = NULL;
    PyObject *tmp_class_creation_8__metaclass = NULL;
    PyObject *tmp_class_creation_8__prepared = NULL;
    PyObject *tmp_class_creation_9__bases = NULL;
    PyObject *tmp_class_creation_9__class_decl_dict = NULL;
    PyObject *tmp_class_creation_9__metaclass = NULL;
    PyObject *tmp_class_creation_9__prepared = NULL;
    struct Nuitka_FrameObject *frame_12850fd7f94e4dfb545b718c84e504f7;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    bool tmp_result;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    int tmp_res;
    PyObject *tmp_dictdel_dict;
    PyObject *tmp_dictdel_key;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8 = NULL;
    PyObject *tmp_dictset_value;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14 = NULL;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20 = NULL;
    struct Nuitka_FrameObject *frame_174b7ceb75edad879b3d9d167edc3442_2;
    NUITKA_MAY_BE_UNUSED char const *type_description_2 = NULL;
    static struct Nuitka_FrameObject *cache_frame_174b7ceb75edad879b3d9d167edc3442_2 = NULL;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32 = NULL;
    struct Nuitka_FrameObject *frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3;
    NUITKA_MAY_BE_UNUSED char const *type_description_3 = NULL;
    static struct Nuitka_FrameObject *cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3 = NULL;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44 = NULL;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50 = NULL;
    struct Nuitka_FrameObject *frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4;
    NUITKA_MAY_BE_UNUSED char const *type_description_4 = NULL;
    static struct Nuitka_FrameObject *cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4 = NULL;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58 = NULL;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64 = NULL;
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
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77 = NULL;
    struct Nuitka_FrameObject *frame_d4612ad00762592c3f3b527f730e5d41_5;
    NUITKA_MAY_BE_UNUSED char const *type_description_5 = NULL;
    static struct Nuitka_FrameObject *cache_frame_d4612ad00762592c3f3b527f730e5d41_5 = NULL;
    PyObject *exception_keeper_type_25;
    PyObject *exception_keeper_value_25;
    PyTracebackObject *exception_keeper_tb_25;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_25;
    PyObject *exception_keeper_type_26;
    PyObject *exception_keeper_value_26;
    PyTracebackObject *exception_keeper_tb_26;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_26;
    PyObject *exception_keeper_type_27;
    PyObject *exception_keeper_value_27;
    PyTracebackObject *exception_keeper_tb_27;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_27;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95 = NULL;
    struct Nuitka_FrameObject *frame_c61a422c602595da887bdeb6ad5c1789_6;
    NUITKA_MAY_BE_UNUSED char const *type_description_6 = NULL;
    static struct Nuitka_FrameObject *cache_frame_c61a422c602595da887bdeb6ad5c1789_6 = NULL;
    PyObject *exception_keeper_type_28;
    PyObject *exception_keeper_value_28;
    PyTracebackObject *exception_keeper_tb_28;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_28;
    PyObject *exception_keeper_type_29;
    PyObject *exception_keeper_value_29;
    PyTracebackObject *exception_keeper_tb_29;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_29;
    PyObject *exception_keeper_type_30;
    PyObject *exception_keeper_value_30;
    PyTracebackObject *exception_keeper_tb_30;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_30;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104 = NULL;
    PyObject *exception_keeper_type_31;
    PyObject *exception_keeper_value_31;
    PyTracebackObject *exception_keeper_tb_31;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_31;
    PyObject *exception_keeper_type_32;
    PyObject *exception_keeper_value_32;
    PyTracebackObject *exception_keeper_tb_32;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_32;
    PyObject *exception_keeper_type_33;
    PyObject *exception_keeper_value_33;
    PyTracebackObject *exception_keeper_tb_33;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_33;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110 = NULL;
    PyObject *exception_keeper_type_34;
    PyObject *exception_keeper_value_34;
    PyTracebackObject *exception_keeper_tb_34;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_34;
    PyObject *exception_keeper_type_35;
    PyObject *exception_keeper_value_35;
    PyTracebackObject *exception_keeper_tb_35;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_35;
    PyObject *exception_keeper_type_36;
    PyObject *exception_keeper_value_36;
    PyTracebackObject *exception_keeper_tb_36;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_36;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120 = NULL;
    PyObject *exception_keeper_type_37;
    PyObject *exception_keeper_value_37;
    PyTracebackObject *exception_keeper_tb_37;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_37;
    PyObject *exception_keeper_type_38;
    PyObject *exception_keeper_value_38;
    PyTracebackObject *exception_keeper_tb_38;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_38;
    PyObject *exception_keeper_type_39;
    PyObject *exception_keeper_value_39;
    PyTracebackObject *exception_keeper_tb_39;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_39;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128 = NULL;
    PyObject *exception_keeper_type_40;
    PyObject *exception_keeper_value_40;
    PyTracebackObject *exception_keeper_tb_40;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_40;
    PyObject *exception_keeper_type_41;
    PyObject *exception_keeper_value_41;
    PyTracebackObject *exception_keeper_tb_41;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_41;
    PyObject *exception_keeper_type_42;
    PyObject *exception_keeper_value_42;
    PyTracebackObject *exception_keeper_tb_42;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_42;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134 = NULL;
    PyObject *exception_keeper_type_43;
    PyObject *exception_keeper_value_43;
    PyTracebackObject *exception_keeper_tb_43;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_43;
    PyObject *exception_keeper_type_44;
    PyObject *exception_keeper_value_44;
    PyTracebackObject *exception_keeper_tb_44;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_44;
    PyObject *exception_keeper_type_45;
    PyObject *exception_keeper_value_45;
    PyTracebackObject *exception_keeper_tb_45;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_45;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140 = NULL;
    PyObject *exception_keeper_type_46;
    PyObject *exception_keeper_value_46;
    PyTracebackObject *exception_keeper_tb_46;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_46;
    PyObject *exception_keeper_type_47;
    PyObject *exception_keeper_value_47;
    PyTracebackObject *exception_keeper_tb_47;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_47;
    PyObject *exception_keeper_type_48;
    PyObject *exception_keeper_value_48;
    PyTracebackObject *exception_keeper_tb_48;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_48;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146 = NULL;
    PyObject *exception_keeper_type_49;
    PyObject *exception_keeper_value_49;
    PyTracebackObject *exception_keeper_tb_49;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_49;
    PyObject *exception_keeper_type_50;
    PyObject *exception_keeper_value_50;
    PyTracebackObject *exception_keeper_tb_50;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_50;
    PyObject *exception_keeper_type_51;
    PyObject *exception_keeper_value_51;
    PyTracebackObject *exception_keeper_tb_51;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_51;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152 = NULL;
    PyObject *exception_keeper_type_52;
    PyObject *exception_keeper_value_52;
    PyTracebackObject *exception_keeper_tb_52;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_52;
    PyObject *exception_keeper_type_53;
    PyObject *exception_keeper_value_53;
    PyTracebackObject *exception_keeper_tb_53;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_53;
    PyObject *exception_keeper_type_54;
    PyObject *exception_keeper_value_54;
    PyTracebackObject *exception_keeper_tb_54;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_54;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158 = NULL;
    struct Nuitka_FrameObject *frame_066d2c927324a67686dc7baef2a7a942_7;
    NUITKA_MAY_BE_UNUSED char const *type_description_7 = NULL;
    static struct Nuitka_FrameObject *cache_frame_066d2c927324a67686dc7baef2a7a942_7 = NULL;
    PyObject *exception_keeper_type_55;
    PyObject *exception_keeper_value_55;
    PyTracebackObject *exception_keeper_tb_55;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_55;
    PyObject *exception_keeper_type_56;
    PyObject *exception_keeper_value_56;
    PyTracebackObject *exception_keeper_tb_56;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_56;
    PyObject *exception_keeper_type_57;
    PyObject *exception_keeper_value_57;
    PyTracebackObject *exception_keeper_tb_57;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_57;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168 = NULL;
    struct Nuitka_FrameObject *frame_db8cc6926c2108e877484bc63b18157c_8;
    NUITKA_MAY_BE_UNUSED char const *type_description_8 = NULL;
    static struct Nuitka_FrameObject *cache_frame_db8cc6926c2108e877484bc63b18157c_8 = NULL;
    PyObject *exception_keeper_type_58;
    PyObject *exception_keeper_value_58;
    PyTracebackObject *exception_keeper_tb_58;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_58;
    PyObject *exception_keeper_type_59;
    PyObject *exception_keeper_value_59;
    PyTracebackObject *exception_keeper_tb_59;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_59;
    PyObject *exception_keeper_type_60;
    PyObject *exception_keeper_value_60;
    PyTracebackObject *exception_keeper_tb_60;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_60;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178 = NULL;
    struct Nuitka_FrameObject *frame_91103067dcd586dfb73a9300382a2bf7_9;
    NUITKA_MAY_BE_UNUSED char const *type_description_9 = NULL;
    static struct Nuitka_FrameObject *cache_frame_91103067dcd586dfb73a9300382a2bf7_9 = NULL;
    PyObject *exception_keeper_type_61;
    PyObject *exception_keeper_value_61;
    PyTracebackObject *exception_keeper_tb_61;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_61;
    PyObject *exception_keeper_type_62;
    PyObject *exception_keeper_value_62;
    PyTracebackObject *exception_keeper_tb_62;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_62;
    PyObject *exception_keeper_type_63;
    PyObject *exception_keeper_value_63;
    PyTracebackObject *exception_keeper_tb_63;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_63;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185 = NULL;
    PyObject *exception_keeper_type_64;
    PyObject *exception_keeper_value_64;
    PyTracebackObject *exception_keeper_tb_64;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_64;
    PyObject *exception_keeper_type_65;
    PyObject *exception_keeper_value_65;
    PyTracebackObject *exception_keeper_tb_65;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_65;
    PyObject *exception_keeper_type_66;
    PyObject *exception_keeper_value_66;
    PyTracebackObject *exception_keeper_tb_66;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_66;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191 = NULL;
    PyObject *exception_keeper_type_67;
    PyObject *exception_keeper_value_67;
    PyTracebackObject *exception_keeper_tb_67;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_67;
    PyObject *exception_keeper_type_68;
    PyObject *exception_keeper_value_68;
    PyTracebackObject *exception_keeper_tb_68;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_68;
    PyObject *exception_keeper_type_69;
    PyObject *exception_keeper_value_69;
    PyTracebackObject *exception_keeper_tb_69;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_69;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197 = NULL;
    PyObject *exception_keeper_type_70;
    PyObject *exception_keeper_value_70;
    PyTracebackObject *exception_keeper_tb_70;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_70;
    PyObject *exception_keeper_type_71;
    PyObject *exception_keeper_value_71;
    PyTracebackObject *exception_keeper_tb_71;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_71;
    PyObject *exception_keeper_type_72;
    PyObject *exception_keeper_value_72;
    PyTracebackObject *exception_keeper_tb_72;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_72;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203 = NULL;
    PyObject *exception_keeper_type_73;
    PyObject *exception_keeper_value_73;
    PyTracebackObject *exception_keeper_tb_73;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_73;
    PyObject *exception_keeper_type_74;
    PyObject *exception_keeper_value_74;
    PyTracebackObject *exception_keeper_tb_74;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_74;
    PyObject *exception_keeper_type_75;
    PyObject *exception_keeper_value_75;
    PyTracebackObject *exception_keeper_tb_75;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_75;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209 = NULL;
    PyObject *exception_keeper_type_76;
    PyObject *exception_keeper_value_76;
    PyTracebackObject *exception_keeper_tb_76;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_76;
    PyObject *exception_keeper_type_77;
    PyObject *exception_keeper_value_77;
    PyTracebackObject *exception_keeper_tb_77;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_77;
    PyObject *exception_keeper_type_78;
    PyObject *exception_keeper_value_78;
    PyTracebackObject *exception_keeper_tb_78;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_78;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215 = NULL;
    PyObject *exception_keeper_type_79;
    PyObject *exception_keeper_value_79;
    PyTracebackObject *exception_keeper_tb_79;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_79;
    PyObject *exception_keeper_type_80;
    PyObject *exception_keeper_value_80;
    PyTracebackObject *exception_keeper_tb_80;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_80;
    PyObject *exception_keeper_type_81;
    PyObject *exception_keeper_value_81;
    PyTracebackObject *exception_keeper_tb_81;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_81;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221 = NULL;
    PyObject *exception_keeper_type_82;
    PyObject *exception_keeper_value_82;
    PyTracebackObject *exception_keeper_tb_82;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_82;
    PyObject *exception_keeper_type_83;
    PyObject *exception_keeper_value_83;
    PyTracebackObject *exception_keeper_tb_83;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_83;
    PyObject *exception_keeper_type_84;
    PyObject *exception_keeper_value_84;
    PyTracebackObject *exception_keeper_tb_84;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_84;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230 = NULL;
    PyObject *exception_keeper_type_85;
    PyObject *exception_keeper_value_85;
    PyTracebackObject *exception_keeper_tb_85;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_85;
    PyObject *exception_keeper_type_86;
    PyObject *exception_keeper_value_86;
    PyTracebackObject *exception_keeper_tb_86;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_86;
    PyObject *exception_keeper_type_87;
    PyObject *exception_keeper_value_87;
    PyTracebackObject *exception_keeper_tb_87;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_87;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236 = NULL;
    PyObject *exception_keeper_type_88;
    PyObject *exception_keeper_value_88;
    PyTracebackObject *exception_keeper_tb_88;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_88;
    PyObject *exception_keeper_type_89;
    PyObject *exception_keeper_value_89;
    PyTracebackObject *exception_keeper_tb_89;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_89;
    PyObject *exception_keeper_type_90;
    PyObject *exception_keeper_value_90;
    PyTracebackObject *exception_keeper_tb_90;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_90;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245 = NULL;
    struct Nuitka_FrameObject *frame_90ad096779fac202a1b1540ce0977ce0_10;
    NUITKA_MAY_BE_UNUSED char const *type_description_10 = NULL;
    static struct Nuitka_FrameObject *cache_frame_90ad096779fac202a1b1540ce0977ce0_10 = NULL;
    PyObject *exception_keeper_type_91;
    PyObject *exception_keeper_value_91;
    PyTracebackObject *exception_keeper_tb_91;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_91;
    PyObject *exception_keeper_type_92;
    PyObject *exception_keeper_value_92;
    PyTracebackObject *exception_keeper_tb_92;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_92;
    PyObject *exception_keeper_type_93;
    PyObject *exception_keeper_value_93;
    PyTracebackObject *exception_keeper_tb_93;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_93;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263 = NULL;
    struct Nuitka_FrameObject *frame_546b5f9419c19af80309bdfac001c04f_11;
    NUITKA_MAY_BE_UNUSED char const *type_description_11 = NULL;
    static struct Nuitka_FrameObject *cache_frame_546b5f9419c19af80309bdfac001c04f_11 = NULL;
    PyObject *exception_keeper_type_94;
    PyObject *exception_keeper_value_94;
    PyTracebackObject *exception_keeper_tb_94;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_94;
    PyObject *exception_keeper_type_95;
    PyObject *exception_keeper_value_95;
    PyTracebackObject *exception_keeper_tb_95;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_95;
    PyObject *exception_keeper_type_96;
    PyObject *exception_keeper_value_96;
    PyTracebackObject *exception_keeper_tb_96;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_96;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280 = NULL;
    PyObject *exception_keeper_type_97;
    PyObject *exception_keeper_value_97;
    PyTracebackObject *exception_keeper_tb_97;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_97;
    PyObject *exception_keeper_type_98;
    PyObject *exception_keeper_value_98;
    PyTracebackObject *exception_keeper_tb_98;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_98;
    PyObject *exception_keeper_type_99;
    PyObject *exception_keeper_value_99;
    PyTracebackObject *exception_keeper_tb_99;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_99;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286 = NULL;
    struct Nuitka_FrameObject *frame_b067facd3b4500d3e185f1adaf50e20e_12;
    NUITKA_MAY_BE_UNUSED char const *type_description_12 = NULL;
    static struct Nuitka_FrameObject *cache_frame_b067facd3b4500d3e185f1adaf50e20e_12 = NULL;
    PyObject *exception_keeper_type_100;
    PyObject *exception_keeper_value_100;
    PyTracebackObject *exception_keeper_tb_100;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_100;
    PyObject *exception_keeper_type_101;
    PyObject *exception_keeper_value_101;
    PyTracebackObject *exception_keeper_tb_101;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_101;
    PyObject *exception_keeper_type_102;
    PyObject *exception_keeper_value_102;
    PyTracebackObject *exception_keeper_tb_102;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_102;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296 = NULL;
    PyObject *exception_keeper_type_103;
    PyObject *exception_keeper_value_103;
    PyTracebackObject *exception_keeper_tb_103;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_103;
    PyObject *exception_keeper_type_104;
    PyObject *exception_keeper_value_104;
    PyTracebackObject *exception_keeper_tb_104;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_104;
    PyObject *exception_keeper_type_105;
    PyObject *exception_keeper_value_105;
    PyTracebackObject *exception_keeper_tb_105;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_105;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302 = NULL;
    struct Nuitka_FrameObject *frame_741c7aa688cb6408fa2c7e94c2e034af_13;
    NUITKA_MAY_BE_UNUSED char const *type_description_13 = NULL;
    static struct Nuitka_FrameObject *cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13 = NULL;
    PyObject *exception_keeper_type_106;
    PyObject *exception_keeper_value_106;
    PyTracebackObject *exception_keeper_tb_106;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_106;
    PyObject *exception_keeper_type_107;
    PyObject *exception_keeper_value_107;
    PyTracebackObject *exception_keeper_tb_107;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_107;
    PyObject *exception_keeper_type_108;
    PyObject *exception_keeper_value_108;
    PyTracebackObject *exception_keeper_tb_108;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_108;
    PyObject *locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310 = NULL;
    PyObject *exception_keeper_type_109;
    PyObject *exception_keeper_value_109;
    PyTracebackObject *exception_keeper_tb_109;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_109;
    PyObject *exception_keeper_type_110;
    PyObject *exception_keeper_value_110;
    PyTracebackObject *exception_keeper_tb_110;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_110;
    PyObject *exception_keeper_type_111;
    PyObject *exception_keeper_value_111;
    PyTracebackObject *exception_keeper_tb_111;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_111;

    // Module code.
    {
        PyObject *tmp_assign_source_1;
        tmp_assign_source_1 = Py_None;
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[35], tmp_assign_source_1);
    }
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = mod_consts[36];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[37], tmp_assign_source_2);
    }
    // Frame without reuse.
    frame_12850fd7f94e4dfb545b718c84e504f7 = MAKE_MODULE_FRAME(codeobj_12850fd7f94e4dfb545b718c84e504f7, module_pip$_vendor$urllib3$exceptions);

    // Push the new frame as the currently active one, and we should be exclusively
    // owning it.
    pushFrameStack(frame_12850fd7f94e4dfb545b718c84e504f7);
    assert(Py_REFCNT(frame_12850fd7f94e4dfb545b718c84e504f7) == 2);

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_assattr_name_1 = mod_consts[36];
        tmp_assattr_target_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[38]);

        if (unlikely(tmp_assattr_target_1 == NULL)) {
            tmp_assattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
        }

        assert(!(tmp_assattr_target_1 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[39], tmp_assattr_name_1);
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
        tmp_assattr_target_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[38]);

        if (unlikely(tmp_assattr_target_2 == NULL)) {
            tmp_assattr_target_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[38]);
        }

        assert(!(tmp_assattr_target_2 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[40], tmp_assattr_name_2);
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
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[41], tmp_assign_source_3);
    }
    {
        PyObject *tmp_assign_source_4;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 1;
        {
            PyObject *hard_module = IMPORT_HARD___FUTURE__();
            tmp_assign_source_4 = LOOKUP_ATTRIBUTE(hard_module, mod_consts[42]);
        }

        assert(!(tmp_assign_source_4 == NULL));
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[42], tmp_assign_source_4);
    }
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_import_name_from_1;
        PyObject *tmp_name_name_1;
        PyObject *tmp_globals_arg_name_1;
        PyObject *tmp_locals_arg_name_1;
        PyObject *tmp_fromlist_name_1;
        PyObject *tmp_level_name_1;
        tmp_name_name_1 = mod_consts[43];
        tmp_globals_arg_name_1 = (PyObject *)moduledict_pip$_vendor$urllib3$exceptions;
        tmp_locals_arg_name_1 = Py_None;
        tmp_fromlist_name_1 = mod_consts[44];
        tmp_level_name_1 = mod_consts[45];
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 3;
        tmp_import_name_from_1 = IMPORT_MODULE5(tmp_name_name_1, tmp_globals_arg_name_1, tmp_locals_arg_name_1, tmp_fromlist_name_1, tmp_level_name_1);
        if (tmp_import_name_from_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_1)) {
            tmp_assign_source_5 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_1,
                (PyObject *)moduledict_pip$_vendor$urllib3$exceptions,
                mod_consts[20],
                mod_consts[46]
            );
        } else {
            tmp_assign_source_5 = IMPORT_NAME(tmp_import_name_from_1, mod_consts[20]);
        }

        Py_DECREF(tmp_import_name_from_1);
        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[47], tmp_assign_source_5);
    }
    {
        PyObject *tmp_assign_source_6;
        tmp_assign_source_6 = mod_consts[48];
        assert(tmp_class_creation_1__bases == NULL);
        Py_INCREF(tmp_assign_source_6);
        tmp_class_creation_1__bases = tmp_assign_source_6;
    }
    {
        PyObject *tmp_assign_source_7;
        tmp_assign_source_7 = PyDict_New();
        assert(tmp_class_creation_1__class_decl_dict == NULL);
        tmp_class_creation_1__class_decl_dict = tmp_assign_source_7;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_8;
        PyObject *tmp_metaclass_name_1;
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_key_name_1;
        PyObject *tmp_dict_arg_name_1;
        PyObject *tmp_dict_arg_name_2;
        PyObject *tmp_key_name_2;
        PyObject *tmp_type_arg_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_subscript_name_1;
        PyObject *tmp_bases_name_1;
        tmp_key_name_1 = mod_consts[49];
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
        tmp_key_name_2 = mod_consts[49];
        tmp_metaclass_name_1 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_2, tmp_key_name_2);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        goto condexpr_end_1;
        condexpr_false_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_expression_name_1 = tmp_class_creation_1__bases;
        tmp_subscript_name_1 = mod_consts[46];
        tmp_type_arg_1 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_1, tmp_subscript_name_1, 0);
        if (tmp_type_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        tmp_metaclass_name_1 = BUILTIN_TYPE1(tmp_type_arg_1);
        Py_DECREF(tmp_type_arg_1);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        condexpr_end_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_bases_name_1 = tmp_class_creation_1__bases;
        tmp_assign_source_8 = SELECT_METACLASS(tmp_metaclass_name_1, tmp_bases_name_1);
        Py_DECREF(tmp_metaclass_name_1);
        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        assert(tmp_class_creation_1__metaclass == NULL);
        tmp_class_creation_1__metaclass = tmp_assign_source_8;
    }
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_key_name_3;
        PyObject *tmp_dict_arg_name_3;
        tmp_key_name_3 = mod_consts[49];
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
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 8;

        goto try_except_handler_1;
    }
    branch_no_1:;
    {
        nuitka_bool tmp_condition_result_3;
        PyObject *tmp_expression_name_2;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_2 = tmp_class_creation_1__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_2, mod_consts[50]);
        tmp_condition_result_3 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_3 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_assign_source_9;
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_3;
        PyObject *tmp_args_name_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_kwargs_name_1;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_3 = tmp_class_creation_1__metaclass;
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_3, mod_consts[50]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        tmp_tuple_element_1 = mod_consts[1];
        tmp_args_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_1, 0, tmp_tuple_element_1);
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_tuple_element_1 = tmp_class_creation_1__bases;
        PyTuple_SET_ITEM0(tmp_args_name_1, 1, tmp_tuple_element_1);
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_kwargs_name_1 = tmp_class_creation_1__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 8;
        tmp_assign_source_9 = CALL_FUNCTION(tmp_called_name_1, tmp_args_name_1, tmp_kwargs_name_1);
        Py_DECREF(tmp_called_name_1);
        Py_DECREF(tmp_args_name_1);
        if (tmp_assign_source_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_9;
    }
    {
        nuitka_bool tmp_condition_result_4;
        PyObject *tmp_operand_name_1;
        PyObject *tmp_expression_name_4;
        CHECK_OBJECT(tmp_class_creation_1__prepared);
        tmp_expression_name_4 = tmp_class_creation_1__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_4, mod_consts[51]);
        tmp_operand_name_1 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

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
        PyObject *tmp_tuple_element_2;
        PyObject *tmp_getattr_target_1;
        PyObject *tmp_getattr_attr_1;
        PyObject *tmp_getattr_default_1;
        tmp_raise_type_1 = PyExc_TypeError;
        tmp_left_name_1 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_getattr_target_1 = tmp_class_creation_1__metaclass;
        tmp_getattr_attr_1 = mod_consts[53];
        tmp_getattr_default_1 = mod_consts[54];
        tmp_tuple_element_2 = BUILTIN_GETATTR(tmp_getattr_target_1, tmp_getattr_attr_1, tmp_getattr_default_1);
        if (tmp_tuple_element_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        tmp_right_name_1 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_5;
            PyObject *tmp_type_arg_2;
            PyTuple_SET_ITEM(tmp_right_name_1, 0, tmp_tuple_element_2);
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_type_arg_2 = tmp_class_creation_1__prepared;
            tmp_expression_name_5 = BUILTIN_TYPE1(tmp_type_arg_2);
            assert(!(tmp_expression_name_5 == NULL));
            tmp_tuple_element_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_5, mod_consts[53]);
            Py_DECREF(tmp_expression_name_5);
            if (tmp_tuple_element_2 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 8;

                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_right_name_1, 1, tmp_tuple_element_2);
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


            exception_lineno = 8;

            goto try_except_handler_1;
        }
        exception_type = tmp_raise_type_1;
        Py_INCREF(tmp_raise_type_1);
        exception_value = tmp_raise_value_1;
        exception_lineno = 8;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_1;
    }
    branch_no_3:;
    goto branch_end_2;
    branch_no_2:;
    {
        PyObject *tmp_assign_source_10;
        tmp_assign_source_10 = PyDict_New();
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_10;
    }
    branch_end_2:;
    {
        PyObject *tmp_assign_source_11;
        {
            PyObject *tmp_set_locals_1;
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_set_locals_1 = tmp_class_creation_1__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8 = tmp_set_locals_1;
            Py_INCREF(tmp_set_locals_1);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_3;
        }
        tmp_dictset_value = mod_consts[57];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_3;
        }
        tmp_dictset_value = mod_consts[1];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 8;

            goto try_except_handler_3;
        }
        {
            PyObject *tmp_assign_source_12;
            PyObject *tmp_called_name_2;
            PyObject *tmp_args_name_2;
            PyObject *tmp_tuple_element_3;
            PyObject *tmp_kwargs_name_2;
            CHECK_OBJECT(tmp_class_creation_1__metaclass);
            tmp_called_name_2 = tmp_class_creation_1__metaclass;
            tmp_tuple_element_3 = mod_consts[1];
            tmp_args_name_2 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_2, 0, tmp_tuple_element_3);
            CHECK_OBJECT(tmp_class_creation_1__bases);
            tmp_tuple_element_3 = tmp_class_creation_1__bases;
            PyTuple_SET_ITEM0(tmp_args_name_2, 1, tmp_tuple_element_3);
            tmp_tuple_element_3 = locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8;
            PyTuple_SET_ITEM0(tmp_args_name_2, 2, tmp_tuple_element_3);
            CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
            tmp_kwargs_name_2 = tmp_class_creation_1__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 8;
            tmp_assign_source_12 = CALL_FUNCTION(tmp_called_name_2, tmp_args_name_2, tmp_kwargs_name_2);
            Py_DECREF(tmp_args_name_2);
            if (tmp_assign_source_12 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 8;

                goto try_except_handler_3;
            }
            assert(outline_0_var___class__ == NULL);
            outline_0_var___class__ = tmp_assign_source_12;
        }
        CHECK_OBJECT(outline_0_var___class__);
        tmp_assign_source_11 = outline_0_var___class__;
        Py_INCREF(tmp_assign_source_11);
        goto try_return_handler_3;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_3:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8);
        locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8 = NULL;
        goto try_return_handler_2;
        // Exception handler code:
        try_except_handler_3:;
        exception_keeper_type_1 = exception_type;
        exception_keeper_value_1 = exception_value;
        exception_keeper_tb_1 = exception_tb;
        exception_keeper_lineno_1 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8);
        locals_pip$_vendor$urllib3$exceptions$$$class__1_HTTPError_8 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_1;
        exception_value = exception_keeper_value_1;
        exception_tb = exception_keeper_tb_1;
        exception_lineno = exception_keeper_lineno_1;

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
        exception_keeper_type_2 = exception_type;
        exception_keeper_value_2 = exception_value;
        exception_keeper_tb_2 = exception_tb;
        exception_keeper_lineno_2 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_2;
        exception_value = exception_keeper_value_2;
        exception_tb = exception_keeper_tb_2;
        exception_lineno = exception_keeper_lineno_2;

        goto outline_exception_1;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_1:;
        exception_lineno = 8;
        goto try_except_handler_1;
        outline_result_1:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1], tmp_assign_source_11);
    }
    goto try_end_1;
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

    Py_XDECREF(tmp_class_creation_1__bases);
    tmp_class_creation_1__bases = NULL;
    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_3;
    exception_value = exception_keeper_value_3;
    exception_tb = exception_keeper_tb_3;
    exception_lineno = exception_keeper_lineno_3;

    goto frame_exception_exit_1;
    // End of try:
    try_end_1:;
    Py_XDECREF(tmp_class_creation_1__bases);
    tmp_class_creation_1__bases = NULL;
    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_1__prepared);
    Py_DECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_13;
        PyObject *tmp_tuple_element_4;
        tmp_tuple_element_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[59]);

        if (unlikely(tmp_tuple_element_4 == NULL)) {
            tmp_tuple_element_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[59]);
        }

        if (tmp_tuple_element_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        tmp_assign_source_13 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_13, 0, tmp_tuple_element_4);
        assert(tmp_class_creation_2__bases == NULL);
        tmp_class_creation_2__bases = tmp_assign_source_13;
    }
    {
        PyObject *tmp_assign_source_14;
        tmp_assign_source_14 = PyDict_New();
        assert(tmp_class_creation_2__class_decl_dict == NULL);
        tmp_class_creation_2__class_decl_dict = tmp_assign_source_14;
    }
    {
        PyObject *tmp_assign_source_15;
        PyObject *tmp_metaclass_name_2;
        nuitka_bool tmp_condition_result_5;
        PyObject *tmp_key_name_4;
        PyObject *tmp_dict_arg_name_4;
        PyObject *tmp_dict_arg_name_5;
        PyObject *tmp_key_name_5;
        PyObject *tmp_type_arg_3;
        PyObject *tmp_expression_name_6;
        PyObject *tmp_subscript_name_2;
        PyObject *tmp_bases_name_2;
        tmp_key_name_4 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_4 = tmp_class_creation_2__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_4, tmp_key_name_4);
        assert(!(tmp_res == -1));
        tmp_condition_result_5 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_5 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_2;
        } else {
            goto condexpr_false_2;
        }
        condexpr_true_2:;
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_5 = tmp_class_creation_2__class_decl_dict;
        tmp_key_name_5 = mod_consts[49];
        tmp_metaclass_name_2 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_5, tmp_key_name_5);
        if (tmp_metaclass_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        goto condexpr_end_2;
        condexpr_false_2:;
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_expression_name_6 = tmp_class_creation_2__bases;
        tmp_subscript_name_2 = mod_consts[46];
        tmp_type_arg_3 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_6, tmp_subscript_name_2, 0);
        if (tmp_type_arg_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        tmp_metaclass_name_2 = BUILTIN_TYPE1(tmp_type_arg_3);
        Py_DECREF(tmp_type_arg_3);
        if (tmp_metaclass_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        condexpr_end_2:;
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_bases_name_2 = tmp_class_creation_2__bases;
        tmp_assign_source_15 = SELECT_METACLASS(tmp_metaclass_name_2, tmp_bases_name_2);
        Py_DECREF(tmp_metaclass_name_2);
        if (tmp_assign_source_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        assert(tmp_class_creation_2__metaclass == NULL);
        tmp_class_creation_2__metaclass = tmp_assign_source_15;
    }
    {
        nuitka_bool tmp_condition_result_6;
        PyObject *tmp_key_name_6;
        PyObject *tmp_dict_arg_name_6;
        tmp_key_name_6 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_dict_arg_name_6 = tmp_class_creation_2__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_6, tmp_key_name_6);
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
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 14;

        goto try_except_handler_4;
    }
    branch_no_4:;
    {
        nuitka_bool tmp_condition_result_7;
        PyObject *tmp_expression_name_7;
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_expression_name_7 = tmp_class_creation_2__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_7, mod_consts[50]);
        tmp_condition_result_7 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_7 == NUITKA_BOOL_TRUE) {
            goto branch_yes_5;
        } else {
            goto branch_no_5;
        }
    }
    branch_yes_5:;
    {
        PyObject *tmp_assign_source_16;
        PyObject *tmp_called_name_3;
        PyObject *tmp_expression_name_8;
        PyObject *tmp_args_name_3;
        PyObject *tmp_tuple_element_5;
        PyObject *tmp_kwargs_name_3;
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_expression_name_8 = tmp_class_creation_2__metaclass;
        tmp_called_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_8, mod_consts[50]);
        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        tmp_tuple_element_5 = mod_consts[60];
        tmp_args_name_3 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_3, 0, tmp_tuple_element_5);
        CHECK_OBJECT(tmp_class_creation_2__bases);
        tmp_tuple_element_5 = tmp_class_creation_2__bases;
        PyTuple_SET_ITEM0(tmp_args_name_3, 1, tmp_tuple_element_5);
        CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
        tmp_kwargs_name_3 = tmp_class_creation_2__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 14;
        tmp_assign_source_16 = CALL_FUNCTION(tmp_called_name_3, tmp_args_name_3, tmp_kwargs_name_3);
        Py_DECREF(tmp_called_name_3);
        Py_DECREF(tmp_args_name_3);
        if (tmp_assign_source_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        assert(tmp_class_creation_2__prepared == NULL);
        tmp_class_creation_2__prepared = tmp_assign_source_16;
    }
    {
        nuitka_bool tmp_condition_result_8;
        PyObject *tmp_operand_name_2;
        PyObject *tmp_expression_name_9;
        CHECK_OBJECT(tmp_class_creation_2__prepared);
        tmp_expression_name_9 = tmp_class_creation_2__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_9, mod_consts[51]);
        tmp_operand_name_2 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
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
        PyObject *tmp_tuple_element_6;
        PyObject *tmp_getattr_target_2;
        PyObject *tmp_getattr_attr_2;
        PyObject *tmp_getattr_default_2;
        tmp_raise_type_2 = PyExc_TypeError;
        tmp_left_name_2 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_2__metaclass);
        tmp_getattr_target_2 = tmp_class_creation_2__metaclass;
        tmp_getattr_attr_2 = mod_consts[53];
        tmp_getattr_default_2 = mod_consts[54];
        tmp_tuple_element_6 = BUILTIN_GETATTR(tmp_getattr_target_2, tmp_getattr_attr_2, tmp_getattr_default_2);
        if (tmp_tuple_element_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        tmp_right_name_2 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_10;
            PyObject *tmp_type_arg_4;
            PyTuple_SET_ITEM(tmp_right_name_2, 0, tmp_tuple_element_6);
            CHECK_OBJECT(tmp_class_creation_2__prepared);
            tmp_type_arg_4 = tmp_class_creation_2__prepared;
            tmp_expression_name_10 = BUILTIN_TYPE1(tmp_type_arg_4);
            assert(!(tmp_expression_name_10 == NULL));
            tmp_tuple_element_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_10, mod_consts[53]);
            Py_DECREF(tmp_expression_name_10);
            if (tmp_tuple_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 14;

                goto tuple_build_exception_2;
            }
            PyTuple_SET_ITEM(tmp_right_name_2, 1, tmp_tuple_element_6);
        }
        goto tuple_build_noexception_2;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_2:;
        Py_DECREF(tmp_right_name_2);
        goto try_except_handler_4;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_2:;
        tmp_raise_value_2 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_2, tmp_right_name_2);
        Py_DECREF(tmp_right_name_2);
        if (tmp_raise_value_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_4;
        }
        exception_type = tmp_raise_type_2;
        Py_INCREF(tmp_raise_type_2);
        exception_value = tmp_raise_value_2;
        exception_lineno = 14;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_4;
    }
    branch_no_6:;
    goto branch_end_5;
    branch_no_5:;
    {
        PyObject *tmp_assign_source_17;
        tmp_assign_source_17 = PyDict_New();
        assert(tmp_class_creation_2__prepared == NULL);
        tmp_class_creation_2__prepared = tmp_assign_source_17;
    }
    branch_end_5:;
    {
        PyObject *tmp_assign_source_18;
        {
            PyObject *tmp_set_locals_2;
            CHECK_OBJECT(tmp_class_creation_2__prepared);
            tmp_set_locals_2 = tmp_class_creation_2__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14 = tmp_set_locals_2;
            Py_INCREF(tmp_set_locals_2);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_6;
        }
        tmp_dictset_value = mod_consts[61];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_6;
        }
        tmp_dictset_value = mod_consts[60];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto try_except_handler_6;
        }
        {
            PyObject *tmp_assign_source_19;
            PyObject *tmp_called_name_4;
            PyObject *tmp_args_name_4;
            PyObject *tmp_tuple_element_7;
            PyObject *tmp_kwargs_name_4;
            CHECK_OBJECT(tmp_class_creation_2__metaclass);
            tmp_called_name_4 = tmp_class_creation_2__metaclass;
            tmp_tuple_element_7 = mod_consts[60];
            tmp_args_name_4 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_4, 0, tmp_tuple_element_7);
            CHECK_OBJECT(tmp_class_creation_2__bases);
            tmp_tuple_element_7 = tmp_class_creation_2__bases;
            PyTuple_SET_ITEM0(tmp_args_name_4, 1, tmp_tuple_element_7);
            tmp_tuple_element_7 = locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14;
            PyTuple_SET_ITEM0(tmp_args_name_4, 2, tmp_tuple_element_7);
            CHECK_OBJECT(tmp_class_creation_2__class_decl_dict);
            tmp_kwargs_name_4 = tmp_class_creation_2__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 14;
            tmp_assign_source_19 = CALL_FUNCTION(tmp_called_name_4, tmp_args_name_4, tmp_kwargs_name_4);
            Py_DECREF(tmp_args_name_4);
            if (tmp_assign_source_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 14;

                goto try_except_handler_6;
            }
            assert(outline_1_var___class__ == NULL);
            outline_1_var___class__ = tmp_assign_source_19;
        }
        CHECK_OBJECT(outline_1_var___class__);
        tmp_assign_source_18 = outline_1_var___class__;
        Py_INCREF(tmp_assign_source_18);
        goto try_return_handler_6;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_6:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14);
        locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14 = NULL;
        goto try_return_handler_5;
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

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14);
        locals_pip$_vendor$urllib3$exceptions$$$class__2_HTTPWarning_14 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_4;
        exception_value = exception_keeper_value_4;
        exception_tb = exception_keeper_tb_4;
        exception_lineno = exception_keeper_lineno_4;

        goto try_except_handler_5;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_5:;
        CHECK_OBJECT(outline_1_var___class__);
        Py_DECREF(outline_1_var___class__);
        outline_1_var___class__ = NULL;
        goto outline_result_2;
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

        // Re-raise.
        exception_type = exception_keeper_type_5;
        exception_value = exception_keeper_value_5;
        exception_tb = exception_keeper_tb_5;
        exception_lineno = exception_keeper_lineno_5;

        goto outline_exception_2;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_2:;
        exception_lineno = 14;
        goto try_except_handler_4;
        outline_result_2:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[60], tmp_assign_source_18);
    }
    goto try_end_2;
    // Exception handler code:
    try_except_handler_4:;
    exception_keeper_type_6 = exception_type;
    exception_keeper_value_6 = exception_value;
    exception_keeper_tb_6 = exception_tb;
    exception_keeper_lineno_6 = exception_lineno;
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
    exception_type = exception_keeper_type_6;
    exception_value = exception_keeper_value_6;
    exception_tb = exception_keeper_tb_6;
    exception_lineno = exception_keeper_lineno_6;

    goto frame_exception_exit_1;
    // End of try:
    try_end_2:;
    Py_XDECREF(tmp_class_creation_2__bases);
    tmp_class_creation_2__bases = NULL;
    Py_XDECREF(tmp_class_creation_2__class_decl_dict);
    tmp_class_creation_2__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_2__metaclass);
    tmp_class_creation_2__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_2__prepared);
    Py_DECREF(tmp_class_creation_2__prepared);
    tmp_class_creation_2__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_20;
        PyObject *tmp_tuple_element_8;
        tmp_tuple_element_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_8 == NULL)) {
            tmp_tuple_element_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        tmp_assign_source_20 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_20, 0, tmp_tuple_element_8);
        assert(tmp_class_creation_3__bases == NULL);
        tmp_class_creation_3__bases = tmp_assign_source_20;
    }
    {
        PyObject *tmp_assign_source_21;
        tmp_assign_source_21 = PyDict_New();
        assert(tmp_class_creation_3__class_decl_dict == NULL);
        tmp_class_creation_3__class_decl_dict = tmp_assign_source_21;
    }
    {
        PyObject *tmp_assign_source_22;
        PyObject *tmp_metaclass_name_3;
        nuitka_bool tmp_condition_result_9;
        PyObject *tmp_key_name_7;
        PyObject *tmp_dict_arg_name_7;
        PyObject *tmp_dict_arg_name_8;
        PyObject *tmp_key_name_8;
        PyObject *tmp_type_arg_5;
        PyObject *tmp_expression_name_11;
        PyObject *tmp_subscript_name_3;
        PyObject *tmp_bases_name_3;
        tmp_key_name_7 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_3__class_decl_dict);
        tmp_dict_arg_name_7 = tmp_class_creation_3__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_7, tmp_key_name_7);
        assert(!(tmp_res == -1));
        tmp_condition_result_9 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_9 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_3;
        } else {
            goto condexpr_false_3;
        }
        condexpr_true_3:;
        CHECK_OBJECT(tmp_class_creation_3__class_decl_dict);
        tmp_dict_arg_name_8 = tmp_class_creation_3__class_decl_dict;
        tmp_key_name_8 = mod_consts[49];
        tmp_metaclass_name_3 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_8, tmp_key_name_8);
        if (tmp_metaclass_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        goto condexpr_end_3;
        condexpr_false_3:;
        CHECK_OBJECT(tmp_class_creation_3__bases);
        tmp_expression_name_11 = tmp_class_creation_3__bases;
        tmp_subscript_name_3 = mod_consts[46];
        tmp_type_arg_5 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_11, tmp_subscript_name_3, 0);
        if (tmp_type_arg_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        tmp_metaclass_name_3 = BUILTIN_TYPE1(tmp_type_arg_5);
        Py_DECREF(tmp_type_arg_5);
        if (tmp_metaclass_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        condexpr_end_3:;
        CHECK_OBJECT(tmp_class_creation_3__bases);
        tmp_bases_name_3 = tmp_class_creation_3__bases;
        tmp_assign_source_22 = SELECT_METACLASS(tmp_metaclass_name_3, tmp_bases_name_3);
        Py_DECREF(tmp_metaclass_name_3);
        if (tmp_assign_source_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        assert(tmp_class_creation_3__metaclass == NULL);
        tmp_class_creation_3__metaclass = tmp_assign_source_22;
    }
    {
        nuitka_bool tmp_condition_result_10;
        PyObject *tmp_key_name_9;
        PyObject *tmp_dict_arg_name_9;
        tmp_key_name_9 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_3__class_decl_dict);
        tmp_dict_arg_name_9 = tmp_class_creation_3__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_9, tmp_key_name_9);
        assert(!(tmp_res == -1));
        tmp_condition_result_10 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_10 == NUITKA_BOOL_TRUE) {
            goto branch_yes_7;
        } else {
            goto branch_no_7;
        }
    }
    branch_yes_7:;
    CHECK_OBJECT(tmp_class_creation_3__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_3__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 20;

        goto try_except_handler_7;
    }
    branch_no_7:;
    {
        nuitka_bool tmp_condition_result_11;
        PyObject *tmp_expression_name_12;
        CHECK_OBJECT(tmp_class_creation_3__metaclass);
        tmp_expression_name_12 = tmp_class_creation_3__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_12, mod_consts[50]);
        tmp_condition_result_11 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_11 == NUITKA_BOOL_TRUE) {
            goto branch_yes_8;
        } else {
            goto branch_no_8;
        }
    }
    branch_yes_8:;
    {
        PyObject *tmp_assign_source_23;
        PyObject *tmp_called_name_5;
        PyObject *tmp_expression_name_13;
        PyObject *tmp_args_name_5;
        PyObject *tmp_tuple_element_9;
        PyObject *tmp_kwargs_name_5;
        CHECK_OBJECT(tmp_class_creation_3__metaclass);
        tmp_expression_name_13 = tmp_class_creation_3__metaclass;
        tmp_called_name_5 = LOOKUP_ATTRIBUTE(tmp_expression_name_13, mod_consts[50]);
        if (tmp_called_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        tmp_tuple_element_9 = mod_consts[6];
        tmp_args_name_5 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_5, 0, tmp_tuple_element_9);
        CHECK_OBJECT(tmp_class_creation_3__bases);
        tmp_tuple_element_9 = tmp_class_creation_3__bases;
        PyTuple_SET_ITEM0(tmp_args_name_5, 1, tmp_tuple_element_9);
        CHECK_OBJECT(tmp_class_creation_3__class_decl_dict);
        tmp_kwargs_name_5 = tmp_class_creation_3__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 20;
        tmp_assign_source_23 = CALL_FUNCTION(tmp_called_name_5, tmp_args_name_5, tmp_kwargs_name_5);
        Py_DECREF(tmp_called_name_5);
        Py_DECREF(tmp_args_name_5);
        if (tmp_assign_source_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        assert(tmp_class_creation_3__prepared == NULL);
        tmp_class_creation_3__prepared = tmp_assign_source_23;
    }
    {
        nuitka_bool tmp_condition_result_12;
        PyObject *tmp_operand_name_3;
        PyObject *tmp_expression_name_14;
        CHECK_OBJECT(tmp_class_creation_3__prepared);
        tmp_expression_name_14 = tmp_class_creation_3__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_14, mod_consts[51]);
        tmp_operand_name_3 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_3);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        tmp_condition_result_12 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_12 == NUITKA_BOOL_TRUE) {
            goto branch_yes_9;
        } else {
            goto branch_no_9;
        }
    }
    branch_yes_9:;
    {
        PyObject *tmp_raise_type_3;
        PyObject *tmp_raise_value_3;
        PyObject *tmp_left_name_3;
        PyObject *tmp_right_name_3;
        PyObject *tmp_tuple_element_10;
        PyObject *tmp_getattr_target_3;
        PyObject *tmp_getattr_attr_3;
        PyObject *tmp_getattr_default_3;
        tmp_raise_type_3 = PyExc_TypeError;
        tmp_left_name_3 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_3__metaclass);
        tmp_getattr_target_3 = tmp_class_creation_3__metaclass;
        tmp_getattr_attr_3 = mod_consts[53];
        tmp_getattr_default_3 = mod_consts[54];
        tmp_tuple_element_10 = BUILTIN_GETATTR(tmp_getattr_target_3, tmp_getattr_attr_3, tmp_getattr_default_3);
        if (tmp_tuple_element_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        tmp_right_name_3 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_15;
            PyObject *tmp_type_arg_6;
            PyTuple_SET_ITEM(tmp_right_name_3, 0, tmp_tuple_element_10);
            CHECK_OBJECT(tmp_class_creation_3__prepared);
            tmp_type_arg_6 = tmp_class_creation_3__prepared;
            tmp_expression_name_15 = BUILTIN_TYPE1(tmp_type_arg_6);
            assert(!(tmp_expression_name_15 == NULL));
            tmp_tuple_element_10 = LOOKUP_ATTRIBUTE(tmp_expression_name_15, mod_consts[53]);
            Py_DECREF(tmp_expression_name_15);
            if (tmp_tuple_element_10 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 20;

                goto tuple_build_exception_3;
            }
            PyTuple_SET_ITEM(tmp_right_name_3, 1, tmp_tuple_element_10);
        }
        goto tuple_build_noexception_3;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_3:;
        Py_DECREF(tmp_right_name_3);
        goto try_except_handler_7;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_3:;
        tmp_raise_value_3 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_3, tmp_right_name_3);
        Py_DECREF(tmp_right_name_3);
        if (tmp_raise_value_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_7;
        }
        exception_type = tmp_raise_type_3;
        Py_INCREF(tmp_raise_type_3);
        exception_value = tmp_raise_value_3;
        exception_lineno = 20;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_7;
    }
    branch_no_9:;
    goto branch_end_8;
    branch_no_8:;
    {
        PyObject *tmp_assign_source_24;
        tmp_assign_source_24 = PyDict_New();
        assert(tmp_class_creation_3__prepared == NULL);
        tmp_class_creation_3__prepared = tmp_assign_source_24;
    }
    branch_end_8:;
    {
        PyObject *tmp_assign_source_25;
        {
            PyObject *tmp_set_locals_3;
            CHECK_OBJECT(tmp_class_creation_3__prepared);
            tmp_set_locals_3 = tmp_class_creation_3__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20 = tmp_set_locals_3;
            Py_INCREF(tmp_set_locals_3);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_9;
        }
        tmp_dictset_value = mod_consts[62];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_9;
        }
        tmp_dictset_value = mod_consts[6];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 20;

            goto try_except_handler_9;
        }
        if (isFrameUnusable(cache_frame_174b7ceb75edad879b3d9d167edc3442_2)) {
            Py_XDECREF(cache_frame_174b7ceb75edad879b3d9d167edc3442_2);

#if _DEBUG_REFCOUNTS
            if (cache_frame_174b7ceb75edad879b3d9d167edc3442_2 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_174b7ceb75edad879b3d9d167edc3442_2 = MAKE_FUNCTION_FRAME(codeobj_174b7ceb75edad879b3d9d167edc3442, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_174b7ceb75edad879b3d9d167edc3442_2->m_type_description == NULL);
        frame_174b7ceb75edad879b3d9d167edc3442_2 = cache_frame_174b7ceb75edad879b3d9d167edc3442_2;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_174b7ceb75edad879b3d9d167edc3442_2);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_174b7ceb75edad879b3d9d167edc3442_2) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__1___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 23;
            type_description_2 = "o";
            goto frame_exception_exit_2;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__2___reduce__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20, mod_consts[64], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 27;
            type_description_2 = "o";
            goto frame_exception_exit_2;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_174b7ceb75edad879b3d9d167edc3442_2);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_1;

        frame_exception_exit_2:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_174b7ceb75edad879b3d9d167edc3442_2);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_174b7ceb75edad879b3d9d167edc3442_2, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_174b7ceb75edad879b3d9d167edc3442_2->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_174b7ceb75edad879b3d9d167edc3442_2, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_174b7ceb75edad879b3d9d167edc3442_2,
            type_description_2,
            outline_2_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_174b7ceb75edad879b3d9d167edc3442_2 == cache_frame_174b7ceb75edad879b3d9d167edc3442_2) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_174b7ceb75edad879b3d9d167edc3442_2);
            cache_frame_174b7ceb75edad879b3d9d167edc3442_2 = NULL;
        }

        assertFrameObject(frame_174b7ceb75edad879b3d9d167edc3442_2);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_1;

        frame_no_exception_1:;
        goto skip_nested_handling_1;
        nested_frame_exit_1:;

        goto try_except_handler_9;
        skip_nested_handling_1:;
        {
            PyObject *tmp_assign_source_26;
            PyObject *tmp_called_name_6;
            PyObject *tmp_args_name_6;
            PyObject *tmp_tuple_element_11;
            PyObject *tmp_kwargs_name_6;
            CHECK_OBJECT(tmp_class_creation_3__metaclass);
            tmp_called_name_6 = tmp_class_creation_3__metaclass;
            tmp_tuple_element_11 = mod_consts[6];
            tmp_args_name_6 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_6, 0, tmp_tuple_element_11);
            CHECK_OBJECT(tmp_class_creation_3__bases);
            tmp_tuple_element_11 = tmp_class_creation_3__bases;
            PyTuple_SET_ITEM0(tmp_args_name_6, 1, tmp_tuple_element_11);
            tmp_tuple_element_11 = locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20;
            PyTuple_SET_ITEM0(tmp_args_name_6, 2, tmp_tuple_element_11);
            CHECK_OBJECT(tmp_class_creation_3__class_decl_dict);
            tmp_kwargs_name_6 = tmp_class_creation_3__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 20;
            tmp_assign_source_26 = CALL_FUNCTION(tmp_called_name_6, tmp_args_name_6, tmp_kwargs_name_6);
            Py_DECREF(tmp_args_name_6);
            if (tmp_assign_source_26 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 20;

                goto try_except_handler_9;
            }
            assert(outline_2_var___class__ == NULL);
            outline_2_var___class__ = tmp_assign_source_26;
        }
        CHECK_OBJECT(outline_2_var___class__);
        tmp_assign_source_25 = outline_2_var___class__;
        Py_INCREF(tmp_assign_source_25);
        goto try_return_handler_9;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_9:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20);
        locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20 = NULL;
        goto try_return_handler_8;
        // Exception handler code:
        try_except_handler_9:;
        exception_keeper_type_7 = exception_type;
        exception_keeper_value_7 = exception_value;
        exception_keeper_tb_7 = exception_tb;
        exception_keeper_lineno_7 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20);
        locals_pip$_vendor$urllib3$exceptions$$$class__3_PoolError_20 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_7;
        exception_value = exception_keeper_value_7;
        exception_tb = exception_keeper_tb_7;
        exception_lineno = exception_keeper_lineno_7;

        goto try_except_handler_8;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_8:;
        CHECK_OBJECT(outline_2_var___class__);
        Py_DECREF(outline_2_var___class__);
        outline_2_var___class__ = NULL;
        goto outline_result_3;
        // Exception handler code:
        try_except_handler_8:;
        exception_keeper_type_8 = exception_type;
        exception_keeper_value_8 = exception_value;
        exception_keeper_tb_8 = exception_tb;
        exception_keeper_lineno_8 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_8;
        exception_value = exception_keeper_value_8;
        exception_tb = exception_keeper_tb_8;
        exception_lineno = exception_keeper_lineno_8;

        goto outline_exception_3;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_3:;
        exception_lineno = 20;
        goto try_except_handler_7;
        outline_result_3:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[6], tmp_assign_source_25);
    }
    goto try_end_3;
    // Exception handler code:
    try_except_handler_7:;
    exception_keeper_type_9 = exception_type;
    exception_keeper_value_9 = exception_value;
    exception_keeper_tb_9 = exception_tb;
    exception_keeper_lineno_9 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_3__bases);
    tmp_class_creation_3__bases = NULL;
    Py_XDECREF(tmp_class_creation_3__class_decl_dict);
    tmp_class_creation_3__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_3__metaclass);
    tmp_class_creation_3__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_3__prepared);
    tmp_class_creation_3__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_9;
    exception_value = exception_keeper_value_9;
    exception_tb = exception_keeper_tb_9;
    exception_lineno = exception_keeper_lineno_9;

    goto frame_exception_exit_1;
    // End of try:
    try_end_3:;
    Py_XDECREF(tmp_class_creation_3__bases);
    tmp_class_creation_3__bases = NULL;
    Py_XDECREF(tmp_class_creation_3__class_decl_dict);
    tmp_class_creation_3__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_3__metaclass);
    tmp_class_creation_3__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_3__prepared);
    Py_DECREF(tmp_class_creation_3__prepared);
    tmp_class_creation_3__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_27;
        PyObject *tmp_tuple_element_12;
        tmp_tuple_element_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[6]);

        if (unlikely(tmp_tuple_element_12 == NULL)) {
            tmp_tuple_element_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[6]);
        }

        if (tmp_tuple_element_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        tmp_assign_source_27 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_27, 0, tmp_tuple_element_12);
        assert(tmp_class_creation_4__bases == NULL);
        tmp_class_creation_4__bases = tmp_assign_source_27;
    }
    {
        PyObject *tmp_assign_source_28;
        tmp_assign_source_28 = PyDict_New();
        assert(tmp_class_creation_4__class_decl_dict == NULL);
        tmp_class_creation_4__class_decl_dict = tmp_assign_source_28;
    }
    {
        PyObject *tmp_assign_source_29;
        PyObject *tmp_metaclass_name_4;
        nuitka_bool tmp_condition_result_13;
        PyObject *tmp_key_name_10;
        PyObject *tmp_dict_arg_name_10;
        PyObject *tmp_dict_arg_name_11;
        PyObject *tmp_key_name_11;
        PyObject *tmp_type_arg_7;
        PyObject *tmp_expression_name_16;
        PyObject *tmp_subscript_name_4;
        PyObject *tmp_bases_name_4;
        tmp_key_name_10 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_4__class_decl_dict);
        tmp_dict_arg_name_10 = tmp_class_creation_4__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_10, tmp_key_name_10);
        assert(!(tmp_res == -1));
        tmp_condition_result_13 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_13 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_4;
        } else {
            goto condexpr_false_4;
        }
        condexpr_true_4:;
        CHECK_OBJECT(tmp_class_creation_4__class_decl_dict);
        tmp_dict_arg_name_11 = tmp_class_creation_4__class_decl_dict;
        tmp_key_name_11 = mod_consts[49];
        tmp_metaclass_name_4 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_11, tmp_key_name_11);
        if (tmp_metaclass_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        goto condexpr_end_4;
        condexpr_false_4:;
        CHECK_OBJECT(tmp_class_creation_4__bases);
        tmp_expression_name_16 = tmp_class_creation_4__bases;
        tmp_subscript_name_4 = mod_consts[46];
        tmp_type_arg_7 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_16, tmp_subscript_name_4, 0);
        if (tmp_type_arg_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        tmp_metaclass_name_4 = BUILTIN_TYPE1(tmp_type_arg_7);
        Py_DECREF(tmp_type_arg_7);
        if (tmp_metaclass_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        condexpr_end_4:;
        CHECK_OBJECT(tmp_class_creation_4__bases);
        tmp_bases_name_4 = tmp_class_creation_4__bases;
        tmp_assign_source_29 = SELECT_METACLASS(tmp_metaclass_name_4, tmp_bases_name_4);
        Py_DECREF(tmp_metaclass_name_4);
        if (tmp_assign_source_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        assert(tmp_class_creation_4__metaclass == NULL);
        tmp_class_creation_4__metaclass = tmp_assign_source_29;
    }
    {
        nuitka_bool tmp_condition_result_14;
        PyObject *tmp_key_name_12;
        PyObject *tmp_dict_arg_name_12;
        tmp_key_name_12 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_4__class_decl_dict);
        tmp_dict_arg_name_12 = tmp_class_creation_4__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_12, tmp_key_name_12);
        assert(!(tmp_res == -1));
        tmp_condition_result_14 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_14 == NUITKA_BOOL_TRUE) {
            goto branch_yes_10;
        } else {
            goto branch_no_10;
        }
    }
    branch_yes_10:;
    CHECK_OBJECT(tmp_class_creation_4__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_4__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 32;

        goto try_except_handler_10;
    }
    branch_no_10:;
    {
        nuitka_bool tmp_condition_result_15;
        PyObject *tmp_expression_name_17;
        CHECK_OBJECT(tmp_class_creation_4__metaclass);
        tmp_expression_name_17 = tmp_class_creation_4__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_17, mod_consts[50]);
        tmp_condition_result_15 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_15 == NUITKA_BOOL_TRUE) {
            goto branch_yes_11;
        } else {
            goto branch_no_11;
        }
    }
    branch_yes_11:;
    {
        PyObject *tmp_assign_source_30;
        PyObject *tmp_called_name_7;
        PyObject *tmp_expression_name_18;
        PyObject *tmp_args_name_7;
        PyObject *tmp_tuple_element_13;
        PyObject *tmp_kwargs_name_7;
        CHECK_OBJECT(tmp_class_creation_4__metaclass);
        tmp_expression_name_18 = tmp_class_creation_4__metaclass;
        tmp_called_name_7 = LOOKUP_ATTRIBUTE(tmp_expression_name_18, mod_consts[50]);
        if (tmp_called_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        tmp_tuple_element_13 = mod_consts[12];
        tmp_args_name_7 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_7, 0, tmp_tuple_element_13);
        CHECK_OBJECT(tmp_class_creation_4__bases);
        tmp_tuple_element_13 = tmp_class_creation_4__bases;
        PyTuple_SET_ITEM0(tmp_args_name_7, 1, tmp_tuple_element_13);
        CHECK_OBJECT(tmp_class_creation_4__class_decl_dict);
        tmp_kwargs_name_7 = tmp_class_creation_4__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 32;
        tmp_assign_source_30 = CALL_FUNCTION(tmp_called_name_7, tmp_args_name_7, tmp_kwargs_name_7);
        Py_DECREF(tmp_called_name_7);
        Py_DECREF(tmp_args_name_7);
        if (tmp_assign_source_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        assert(tmp_class_creation_4__prepared == NULL);
        tmp_class_creation_4__prepared = tmp_assign_source_30;
    }
    {
        nuitka_bool tmp_condition_result_16;
        PyObject *tmp_operand_name_4;
        PyObject *tmp_expression_name_19;
        CHECK_OBJECT(tmp_class_creation_4__prepared);
        tmp_expression_name_19 = tmp_class_creation_4__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_19, mod_consts[51]);
        tmp_operand_name_4 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_4);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        tmp_condition_result_16 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_16 == NUITKA_BOOL_TRUE) {
            goto branch_yes_12;
        } else {
            goto branch_no_12;
        }
    }
    branch_yes_12:;
    {
        PyObject *tmp_raise_type_4;
        PyObject *tmp_raise_value_4;
        PyObject *tmp_left_name_4;
        PyObject *tmp_right_name_4;
        PyObject *tmp_tuple_element_14;
        PyObject *tmp_getattr_target_4;
        PyObject *tmp_getattr_attr_4;
        PyObject *tmp_getattr_default_4;
        tmp_raise_type_4 = PyExc_TypeError;
        tmp_left_name_4 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_4__metaclass);
        tmp_getattr_target_4 = tmp_class_creation_4__metaclass;
        tmp_getattr_attr_4 = mod_consts[53];
        tmp_getattr_default_4 = mod_consts[54];
        tmp_tuple_element_14 = BUILTIN_GETATTR(tmp_getattr_target_4, tmp_getattr_attr_4, tmp_getattr_default_4);
        if (tmp_tuple_element_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        tmp_right_name_4 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_20;
            PyObject *tmp_type_arg_8;
            PyTuple_SET_ITEM(tmp_right_name_4, 0, tmp_tuple_element_14);
            CHECK_OBJECT(tmp_class_creation_4__prepared);
            tmp_type_arg_8 = tmp_class_creation_4__prepared;
            tmp_expression_name_20 = BUILTIN_TYPE1(tmp_type_arg_8);
            assert(!(tmp_expression_name_20 == NULL));
            tmp_tuple_element_14 = LOOKUP_ATTRIBUTE(tmp_expression_name_20, mod_consts[53]);
            Py_DECREF(tmp_expression_name_20);
            if (tmp_tuple_element_14 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 32;

                goto tuple_build_exception_4;
            }
            PyTuple_SET_ITEM(tmp_right_name_4, 1, tmp_tuple_element_14);
        }
        goto tuple_build_noexception_4;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_4:;
        Py_DECREF(tmp_right_name_4);
        goto try_except_handler_10;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_4:;
        tmp_raise_value_4 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_4, tmp_right_name_4);
        Py_DECREF(tmp_right_name_4);
        if (tmp_raise_value_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_10;
        }
        exception_type = tmp_raise_type_4;
        Py_INCREF(tmp_raise_type_4);
        exception_value = tmp_raise_value_4;
        exception_lineno = 32;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_10;
    }
    branch_no_12:;
    goto branch_end_11;
    branch_no_11:;
    {
        PyObject *tmp_assign_source_31;
        tmp_assign_source_31 = PyDict_New();
        assert(tmp_class_creation_4__prepared == NULL);
        tmp_class_creation_4__prepared = tmp_assign_source_31;
    }
    branch_end_11:;
    {
        PyObject *tmp_assign_source_32;
        {
            PyObject *tmp_set_locals_4;
            CHECK_OBJECT(tmp_class_creation_4__prepared);
            tmp_set_locals_4 = tmp_class_creation_4__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32 = tmp_set_locals_4;
            Py_INCREF(tmp_set_locals_4);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_12;
        }
        tmp_dictset_value = mod_consts[66];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_12;
        }
        tmp_dictset_value = mod_consts[12];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto try_except_handler_12;
        }
        if (isFrameUnusable(cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3)) {
            Py_XDECREF(cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3);

#if _DEBUG_REFCOUNTS
            if (cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3 = MAKE_FUNCTION_FRAME(codeobj_390e0d2c1f0a89462d2b6e4f63d3fdc4, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3->m_type_description == NULL);
        frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3 = cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__3___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 35;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__4___reduce__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32, mod_consts[64], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 39;
            type_description_2 = "o";
            goto frame_exception_exit_3;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_2;

        frame_exception_exit_3:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3,
            type_description_2,
            outline_3_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3 == cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3);
            cache_frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3 = NULL;
        }

        assertFrameObject(frame_390e0d2c1f0a89462d2b6e4f63d3fdc4_3);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_2;

        frame_no_exception_2:;
        goto skip_nested_handling_2;
        nested_frame_exit_2:;

        goto try_except_handler_12;
        skip_nested_handling_2:;
        {
            PyObject *tmp_assign_source_33;
            PyObject *tmp_called_name_8;
            PyObject *tmp_args_name_8;
            PyObject *tmp_tuple_element_15;
            PyObject *tmp_kwargs_name_8;
            CHECK_OBJECT(tmp_class_creation_4__metaclass);
            tmp_called_name_8 = tmp_class_creation_4__metaclass;
            tmp_tuple_element_15 = mod_consts[12];
            tmp_args_name_8 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_8, 0, tmp_tuple_element_15);
            CHECK_OBJECT(tmp_class_creation_4__bases);
            tmp_tuple_element_15 = tmp_class_creation_4__bases;
            PyTuple_SET_ITEM0(tmp_args_name_8, 1, tmp_tuple_element_15);
            tmp_tuple_element_15 = locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32;
            PyTuple_SET_ITEM0(tmp_args_name_8, 2, tmp_tuple_element_15);
            CHECK_OBJECT(tmp_class_creation_4__class_decl_dict);
            tmp_kwargs_name_8 = tmp_class_creation_4__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 32;
            tmp_assign_source_33 = CALL_FUNCTION(tmp_called_name_8, tmp_args_name_8, tmp_kwargs_name_8);
            Py_DECREF(tmp_args_name_8);
            if (tmp_assign_source_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 32;

                goto try_except_handler_12;
            }
            assert(outline_3_var___class__ == NULL);
            outline_3_var___class__ = tmp_assign_source_33;
        }
        CHECK_OBJECT(outline_3_var___class__);
        tmp_assign_source_32 = outline_3_var___class__;
        Py_INCREF(tmp_assign_source_32);
        goto try_return_handler_12;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_12:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32);
        locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32 = NULL;
        goto try_return_handler_11;
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

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32);
        locals_pip$_vendor$urllib3$exceptions$$$class__4_RequestError_32 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_10;
        exception_value = exception_keeper_value_10;
        exception_tb = exception_keeper_tb_10;
        exception_lineno = exception_keeper_lineno_10;

        goto try_except_handler_11;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_11:;
        CHECK_OBJECT(outline_3_var___class__);
        Py_DECREF(outline_3_var___class__);
        outline_3_var___class__ = NULL;
        goto outline_result_4;
        // Exception handler code:
        try_except_handler_11:;
        exception_keeper_type_11 = exception_type;
        exception_keeper_value_11 = exception_value;
        exception_keeper_tb_11 = exception_tb;
        exception_keeper_lineno_11 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_11;
        exception_value = exception_keeper_value_11;
        exception_tb = exception_keeper_tb_11;
        exception_lineno = exception_keeper_lineno_11;

        goto outline_exception_4;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_4:;
        exception_lineno = 32;
        goto try_except_handler_10;
        outline_result_4:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[12], tmp_assign_source_32);
    }
    goto try_end_4;
    // Exception handler code:
    try_except_handler_10:;
    exception_keeper_type_12 = exception_type;
    exception_keeper_value_12 = exception_value;
    exception_keeper_tb_12 = exception_tb;
    exception_keeper_lineno_12 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_4__bases);
    tmp_class_creation_4__bases = NULL;
    Py_XDECREF(tmp_class_creation_4__class_decl_dict);
    tmp_class_creation_4__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_4__metaclass);
    tmp_class_creation_4__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_4__prepared);
    tmp_class_creation_4__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_12;
    exception_value = exception_keeper_value_12;
    exception_tb = exception_keeper_tb_12;
    exception_lineno = exception_keeper_lineno_12;

    goto frame_exception_exit_1;
    // End of try:
    try_end_4:;
    Py_XDECREF(tmp_class_creation_4__bases);
    tmp_class_creation_4__bases = NULL;
    Py_XDECREF(tmp_class_creation_4__class_decl_dict);
    tmp_class_creation_4__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_4__metaclass);
    tmp_class_creation_4__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_4__prepared);
    Py_DECREF(tmp_class_creation_4__prepared);
    tmp_class_creation_4__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_34;
        PyObject *tmp_tuple_element_16;
        tmp_tuple_element_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_16 == NULL)) {
            tmp_tuple_element_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        tmp_assign_source_34 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_34, 0, tmp_tuple_element_16);
        assert(tmp_class_creation_5__bases == NULL);
        tmp_class_creation_5__bases = tmp_assign_source_34;
    }
    {
        PyObject *tmp_assign_source_35;
        tmp_assign_source_35 = PyDict_New();
        assert(tmp_class_creation_5__class_decl_dict == NULL);
        tmp_class_creation_5__class_decl_dict = tmp_assign_source_35;
    }
    {
        PyObject *tmp_assign_source_36;
        PyObject *tmp_metaclass_name_5;
        nuitka_bool tmp_condition_result_17;
        PyObject *tmp_key_name_13;
        PyObject *tmp_dict_arg_name_13;
        PyObject *tmp_dict_arg_name_14;
        PyObject *tmp_key_name_14;
        PyObject *tmp_type_arg_9;
        PyObject *tmp_expression_name_21;
        PyObject *tmp_subscript_name_5;
        PyObject *tmp_bases_name_5;
        tmp_key_name_13 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_5__class_decl_dict);
        tmp_dict_arg_name_13 = tmp_class_creation_5__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_13, tmp_key_name_13);
        assert(!(tmp_res == -1));
        tmp_condition_result_17 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_17 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_5;
        } else {
            goto condexpr_false_5;
        }
        condexpr_true_5:;
        CHECK_OBJECT(tmp_class_creation_5__class_decl_dict);
        tmp_dict_arg_name_14 = tmp_class_creation_5__class_decl_dict;
        tmp_key_name_14 = mod_consts[49];
        tmp_metaclass_name_5 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_14, tmp_key_name_14);
        if (tmp_metaclass_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        goto condexpr_end_5;
        condexpr_false_5:;
        CHECK_OBJECT(tmp_class_creation_5__bases);
        tmp_expression_name_21 = tmp_class_creation_5__bases;
        tmp_subscript_name_5 = mod_consts[46];
        tmp_type_arg_9 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_21, tmp_subscript_name_5, 0);
        if (tmp_type_arg_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        tmp_metaclass_name_5 = BUILTIN_TYPE1(tmp_type_arg_9);
        Py_DECREF(tmp_type_arg_9);
        if (tmp_metaclass_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        condexpr_end_5:;
        CHECK_OBJECT(tmp_class_creation_5__bases);
        tmp_bases_name_5 = tmp_class_creation_5__bases;
        tmp_assign_source_36 = SELECT_METACLASS(tmp_metaclass_name_5, tmp_bases_name_5);
        Py_DECREF(tmp_metaclass_name_5);
        if (tmp_assign_source_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        assert(tmp_class_creation_5__metaclass == NULL);
        tmp_class_creation_5__metaclass = tmp_assign_source_36;
    }
    {
        nuitka_bool tmp_condition_result_18;
        PyObject *tmp_key_name_15;
        PyObject *tmp_dict_arg_name_15;
        tmp_key_name_15 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_5__class_decl_dict);
        tmp_dict_arg_name_15 = tmp_class_creation_5__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_15, tmp_key_name_15);
        assert(!(tmp_res == -1));
        tmp_condition_result_18 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_18 == NUITKA_BOOL_TRUE) {
            goto branch_yes_13;
        } else {
            goto branch_no_13;
        }
    }
    branch_yes_13:;
    CHECK_OBJECT(tmp_class_creation_5__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_5__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 44;

        goto try_except_handler_13;
    }
    branch_no_13:;
    {
        nuitka_bool tmp_condition_result_19;
        PyObject *tmp_expression_name_22;
        CHECK_OBJECT(tmp_class_creation_5__metaclass);
        tmp_expression_name_22 = tmp_class_creation_5__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_22, mod_consts[50]);
        tmp_condition_result_19 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_19 == NUITKA_BOOL_TRUE) {
            goto branch_yes_14;
        } else {
            goto branch_no_14;
        }
    }
    branch_yes_14:;
    {
        PyObject *tmp_assign_source_37;
        PyObject *tmp_called_name_9;
        PyObject *tmp_expression_name_23;
        PyObject *tmp_args_name_9;
        PyObject *tmp_tuple_element_17;
        PyObject *tmp_kwargs_name_9;
        CHECK_OBJECT(tmp_class_creation_5__metaclass);
        tmp_expression_name_23 = tmp_class_creation_5__metaclass;
        tmp_called_name_9 = LOOKUP_ATTRIBUTE(tmp_expression_name_23, mod_consts[50]);
        if (tmp_called_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        tmp_tuple_element_17 = mod_consts[69];
        tmp_args_name_9 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_9, 0, tmp_tuple_element_17);
        CHECK_OBJECT(tmp_class_creation_5__bases);
        tmp_tuple_element_17 = tmp_class_creation_5__bases;
        PyTuple_SET_ITEM0(tmp_args_name_9, 1, tmp_tuple_element_17);
        CHECK_OBJECT(tmp_class_creation_5__class_decl_dict);
        tmp_kwargs_name_9 = tmp_class_creation_5__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 44;
        tmp_assign_source_37 = CALL_FUNCTION(tmp_called_name_9, tmp_args_name_9, tmp_kwargs_name_9);
        Py_DECREF(tmp_called_name_9);
        Py_DECREF(tmp_args_name_9);
        if (tmp_assign_source_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        assert(tmp_class_creation_5__prepared == NULL);
        tmp_class_creation_5__prepared = tmp_assign_source_37;
    }
    {
        nuitka_bool tmp_condition_result_20;
        PyObject *tmp_operand_name_5;
        PyObject *tmp_expression_name_24;
        CHECK_OBJECT(tmp_class_creation_5__prepared);
        tmp_expression_name_24 = tmp_class_creation_5__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_24, mod_consts[51]);
        tmp_operand_name_5 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_5);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        tmp_condition_result_20 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_20 == NUITKA_BOOL_TRUE) {
            goto branch_yes_15;
        } else {
            goto branch_no_15;
        }
    }
    branch_yes_15:;
    {
        PyObject *tmp_raise_type_5;
        PyObject *tmp_raise_value_5;
        PyObject *tmp_left_name_5;
        PyObject *tmp_right_name_5;
        PyObject *tmp_tuple_element_18;
        PyObject *tmp_getattr_target_5;
        PyObject *tmp_getattr_attr_5;
        PyObject *tmp_getattr_default_5;
        tmp_raise_type_5 = PyExc_TypeError;
        tmp_left_name_5 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_5__metaclass);
        tmp_getattr_target_5 = tmp_class_creation_5__metaclass;
        tmp_getattr_attr_5 = mod_consts[53];
        tmp_getattr_default_5 = mod_consts[54];
        tmp_tuple_element_18 = BUILTIN_GETATTR(tmp_getattr_target_5, tmp_getattr_attr_5, tmp_getattr_default_5);
        if (tmp_tuple_element_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        tmp_right_name_5 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_25;
            PyObject *tmp_type_arg_10;
            PyTuple_SET_ITEM(tmp_right_name_5, 0, tmp_tuple_element_18);
            CHECK_OBJECT(tmp_class_creation_5__prepared);
            tmp_type_arg_10 = tmp_class_creation_5__prepared;
            tmp_expression_name_25 = BUILTIN_TYPE1(tmp_type_arg_10);
            assert(!(tmp_expression_name_25 == NULL));
            tmp_tuple_element_18 = LOOKUP_ATTRIBUTE(tmp_expression_name_25, mod_consts[53]);
            Py_DECREF(tmp_expression_name_25);
            if (tmp_tuple_element_18 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 44;

                goto tuple_build_exception_5;
            }
            PyTuple_SET_ITEM(tmp_right_name_5, 1, tmp_tuple_element_18);
        }
        goto tuple_build_noexception_5;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_5:;
        Py_DECREF(tmp_right_name_5);
        goto try_except_handler_13;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_5:;
        tmp_raise_value_5 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_5, tmp_right_name_5);
        Py_DECREF(tmp_right_name_5);
        if (tmp_raise_value_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_13;
        }
        exception_type = tmp_raise_type_5;
        Py_INCREF(tmp_raise_type_5);
        exception_value = tmp_raise_value_5;
        exception_lineno = 44;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_13;
    }
    branch_no_15:;
    goto branch_end_14;
    branch_no_14:;
    {
        PyObject *tmp_assign_source_38;
        tmp_assign_source_38 = PyDict_New();
        assert(tmp_class_creation_5__prepared == NULL);
        tmp_class_creation_5__prepared = tmp_assign_source_38;
    }
    branch_end_14:;
    {
        PyObject *tmp_assign_source_39;
        {
            PyObject *tmp_set_locals_5;
            CHECK_OBJECT(tmp_class_creation_5__prepared);
            tmp_set_locals_5 = tmp_class_creation_5__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44 = tmp_set_locals_5;
            Py_INCREF(tmp_set_locals_5);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_15;
        }
        tmp_dictset_value = mod_consts[70];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_15;
        }
        tmp_dictset_value = mod_consts[69];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 44;

            goto try_except_handler_15;
        }
        {
            PyObject *tmp_assign_source_40;
            PyObject *tmp_called_name_10;
            PyObject *tmp_args_name_10;
            PyObject *tmp_tuple_element_19;
            PyObject *tmp_kwargs_name_10;
            CHECK_OBJECT(tmp_class_creation_5__metaclass);
            tmp_called_name_10 = tmp_class_creation_5__metaclass;
            tmp_tuple_element_19 = mod_consts[69];
            tmp_args_name_10 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_10, 0, tmp_tuple_element_19);
            CHECK_OBJECT(tmp_class_creation_5__bases);
            tmp_tuple_element_19 = tmp_class_creation_5__bases;
            PyTuple_SET_ITEM0(tmp_args_name_10, 1, tmp_tuple_element_19);
            tmp_tuple_element_19 = locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44;
            PyTuple_SET_ITEM0(tmp_args_name_10, 2, tmp_tuple_element_19);
            CHECK_OBJECT(tmp_class_creation_5__class_decl_dict);
            tmp_kwargs_name_10 = tmp_class_creation_5__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 44;
            tmp_assign_source_40 = CALL_FUNCTION(tmp_called_name_10, tmp_args_name_10, tmp_kwargs_name_10);
            Py_DECREF(tmp_args_name_10);
            if (tmp_assign_source_40 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 44;

                goto try_except_handler_15;
            }
            assert(outline_4_var___class__ == NULL);
            outline_4_var___class__ = tmp_assign_source_40;
        }
        CHECK_OBJECT(outline_4_var___class__);
        tmp_assign_source_39 = outline_4_var___class__;
        Py_INCREF(tmp_assign_source_39);
        goto try_return_handler_15;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_15:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44);
        locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44 = NULL;
        goto try_return_handler_14;
        // Exception handler code:
        try_except_handler_15:;
        exception_keeper_type_13 = exception_type;
        exception_keeper_value_13 = exception_value;
        exception_keeper_tb_13 = exception_tb;
        exception_keeper_lineno_13 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44);
        locals_pip$_vendor$urllib3$exceptions$$$class__5_SSLError_44 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_13;
        exception_value = exception_keeper_value_13;
        exception_tb = exception_keeper_tb_13;
        exception_lineno = exception_keeper_lineno_13;

        goto try_except_handler_14;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_14:;
        CHECK_OBJECT(outline_4_var___class__);
        Py_DECREF(outline_4_var___class__);
        outline_4_var___class__ = NULL;
        goto outline_result_5;
        // Exception handler code:
        try_except_handler_14:;
        exception_keeper_type_14 = exception_type;
        exception_keeper_value_14 = exception_value;
        exception_keeper_tb_14 = exception_tb;
        exception_keeper_lineno_14 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_14;
        exception_value = exception_keeper_value_14;
        exception_tb = exception_keeper_tb_14;
        exception_lineno = exception_keeper_lineno_14;

        goto outline_exception_5;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_5:;
        exception_lineno = 44;
        goto try_except_handler_13;
        outline_result_5:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[69], tmp_assign_source_39);
    }
    goto try_end_5;
    // Exception handler code:
    try_except_handler_13:;
    exception_keeper_type_15 = exception_type;
    exception_keeper_value_15 = exception_value;
    exception_keeper_tb_15 = exception_tb;
    exception_keeper_lineno_15 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_5__bases);
    tmp_class_creation_5__bases = NULL;
    Py_XDECREF(tmp_class_creation_5__class_decl_dict);
    tmp_class_creation_5__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_5__metaclass);
    tmp_class_creation_5__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_5__prepared);
    tmp_class_creation_5__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_15;
    exception_value = exception_keeper_value_15;
    exception_tb = exception_keeper_tb_15;
    exception_lineno = exception_keeper_lineno_15;

    goto frame_exception_exit_1;
    // End of try:
    try_end_5:;
    Py_XDECREF(tmp_class_creation_5__bases);
    tmp_class_creation_5__bases = NULL;
    Py_XDECREF(tmp_class_creation_5__class_decl_dict);
    tmp_class_creation_5__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_5__metaclass);
    tmp_class_creation_5__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_5__prepared);
    Py_DECREF(tmp_class_creation_5__prepared);
    tmp_class_creation_5__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_41;
        PyObject *tmp_tuple_element_20;
        tmp_tuple_element_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_20 == NULL)) {
            tmp_tuple_element_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        tmp_assign_source_41 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_41, 0, tmp_tuple_element_20);
        assert(tmp_class_creation_6__bases == NULL);
        tmp_class_creation_6__bases = tmp_assign_source_41;
    }
    {
        PyObject *tmp_assign_source_42;
        tmp_assign_source_42 = PyDict_New();
        assert(tmp_class_creation_6__class_decl_dict == NULL);
        tmp_class_creation_6__class_decl_dict = tmp_assign_source_42;
    }
    {
        PyObject *tmp_assign_source_43;
        PyObject *tmp_metaclass_name_6;
        nuitka_bool tmp_condition_result_21;
        PyObject *tmp_key_name_16;
        PyObject *tmp_dict_arg_name_16;
        PyObject *tmp_dict_arg_name_17;
        PyObject *tmp_key_name_17;
        PyObject *tmp_type_arg_11;
        PyObject *tmp_expression_name_26;
        PyObject *tmp_subscript_name_6;
        PyObject *tmp_bases_name_6;
        tmp_key_name_16 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_6__class_decl_dict);
        tmp_dict_arg_name_16 = tmp_class_creation_6__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_16, tmp_key_name_16);
        assert(!(tmp_res == -1));
        tmp_condition_result_21 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_21 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_6;
        } else {
            goto condexpr_false_6;
        }
        condexpr_true_6:;
        CHECK_OBJECT(tmp_class_creation_6__class_decl_dict);
        tmp_dict_arg_name_17 = tmp_class_creation_6__class_decl_dict;
        tmp_key_name_17 = mod_consts[49];
        tmp_metaclass_name_6 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_17, tmp_key_name_17);
        if (tmp_metaclass_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        goto condexpr_end_6;
        condexpr_false_6:;
        CHECK_OBJECT(tmp_class_creation_6__bases);
        tmp_expression_name_26 = tmp_class_creation_6__bases;
        tmp_subscript_name_6 = mod_consts[46];
        tmp_type_arg_11 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_26, tmp_subscript_name_6, 0);
        if (tmp_type_arg_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        tmp_metaclass_name_6 = BUILTIN_TYPE1(tmp_type_arg_11);
        Py_DECREF(tmp_type_arg_11);
        if (tmp_metaclass_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        condexpr_end_6:;
        CHECK_OBJECT(tmp_class_creation_6__bases);
        tmp_bases_name_6 = tmp_class_creation_6__bases;
        tmp_assign_source_43 = SELECT_METACLASS(tmp_metaclass_name_6, tmp_bases_name_6);
        Py_DECREF(tmp_metaclass_name_6);
        if (tmp_assign_source_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        assert(tmp_class_creation_6__metaclass == NULL);
        tmp_class_creation_6__metaclass = tmp_assign_source_43;
    }
    {
        nuitka_bool tmp_condition_result_22;
        PyObject *tmp_key_name_18;
        PyObject *tmp_dict_arg_name_18;
        tmp_key_name_18 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_6__class_decl_dict);
        tmp_dict_arg_name_18 = tmp_class_creation_6__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_18, tmp_key_name_18);
        assert(!(tmp_res == -1));
        tmp_condition_result_22 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_22 == NUITKA_BOOL_TRUE) {
            goto branch_yes_16;
        } else {
            goto branch_no_16;
        }
    }
    branch_yes_16:;
    CHECK_OBJECT(tmp_class_creation_6__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_6__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 50;

        goto try_except_handler_16;
    }
    branch_no_16:;
    {
        nuitka_bool tmp_condition_result_23;
        PyObject *tmp_expression_name_27;
        CHECK_OBJECT(tmp_class_creation_6__metaclass);
        tmp_expression_name_27 = tmp_class_creation_6__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_27, mod_consts[50]);
        tmp_condition_result_23 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_23 == NUITKA_BOOL_TRUE) {
            goto branch_yes_17;
        } else {
            goto branch_no_17;
        }
    }
    branch_yes_17:;
    {
        PyObject *tmp_assign_source_44;
        PyObject *tmp_called_name_11;
        PyObject *tmp_expression_name_28;
        PyObject *tmp_args_name_11;
        PyObject *tmp_tuple_element_21;
        PyObject *tmp_kwargs_name_11;
        CHECK_OBJECT(tmp_class_creation_6__metaclass);
        tmp_expression_name_28 = tmp_class_creation_6__metaclass;
        tmp_called_name_11 = LOOKUP_ATTRIBUTE(tmp_expression_name_28, mod_consts[50]);
        if (tmp_called_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        tmp_tuple_element_21 = mod_consts[8];
        tmp_args_name_11 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_11, 0, tmp_tuple_element_21);
        CHECK_OBJECT(tmp_class_creation_6__bases);
        tmp_tuple_element_21 = tmp_class_creation_6__bases;
        PyTuple_SET_ITEM0(tmp_args_name_11, 1, tmp_tuple_element_21);
        CHECK_OBJECT(tmp_class_creation_6__class_decl_dict);
        tmp_kwargs_name_11 = tmp_class_creation_6__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 50;
        tmp_assign_source_44 = CALL_FUNCTION(tmp_called_name_11, tmp_args_name_11, tmp_kwargs_name_11);
        Py_DECREF(tmp_called_name_11);
        Py_DECREF(tmp_args_name_11);
        if (tmp_assign_source_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        assert(tmp_class_creation_6__prepared == NULL);
        tmp_class_creation_6__prepared = tmp_assign_source_44;
    }
    {
        nuitka_bool tmp_condition_result_24;
        PyObject *tmp_operand_name_6;
        PyObject *tmp_expression_name_29;
        CHECK_OBJECT(tmp_class_creation_6__prepared);
        tmp_expression_name_29 = tmp_class_creation_6__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_29, mod_consts[51]);
        tmp_operand_name_6 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_6);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        tmp_condition_result_24 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_24 == NUITKA_BOOL_TRUE) {
            goto branch_yes_18;
        } else {
            goto branch_no_18;
        }
    }
    branch_yes_18:;
    {
        PyObject *tmp_raise_type_6;
        PyObject *tmp_raise_value_6;
        PyObject *tmp_left_name_6;
        PyObject *tmp_right_name_6;
        PyObject *tmp_tuple_element_22;
        PyObject *tmp_getattr_target_6;
        PyObject *tmp_getattr_attr_6;
        PyObject *tmp_getattr_default_6;
        tmp_raise_type_6 = PyExc_TypeError;
        tmp_left_name_6 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_6__metaclass);
        tmp_getattr_target_6 = tmp_class_creation_6__metaclass;
        tmp_getattr_attr_6 = mod_consts[53];
        tmp_getattr_default_6 = mod_consts[54];
        tmp_tuple_element_22 = BUILTIN_GETATTR(tmp_getattr_target_6, tmp_getattr_attr_6, tmp_getattr_default_6);
        if (tmp_tuple_element_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        tmp_right_name_6 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_30;
            PyObject *tmp_type_arg_12;
            PyTuple_SET_ITEM(tmp_right_name_6, 0, tmp_tuple_element_22);
            CHECK_OBJECT(tmp_class_creation_6__prepared);
            tmp_type_arg_12 = tmp_class_creation_6__prepared;
            tmp_expression_name_30 = BUILTIN_TYPE1(tmp_type_arg_12);
            assert(!(tmp_expression_name_30 == NULL));
            tmp_tuple_element_22 = LOOKUP_ATTRIBUTE(tmp_expression_name_30, mod_consts[53]);
            Py_DECREF(tmp_expression_name_30);
            if (tmp_tuple_element_22 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 50;

                goto tuple_build_exception_6;
            }
            PyTuple_SET_ITEM(tmp_right_name_6, 1, tmp_tuple_element_22);
        }
        goto tuple_build_noexception_6;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_6:;
        Py_DECREF(tmp_right_name_6);
        goto try_except_handler_16;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_6:;
        tmp_raise_value_6 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_6, tmp_right_name_6);
        Py_DECREF(tmp_right_name_6);
        if (tmp_raise_value_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_16;
        }
        exception_type = tmp_raise_type_6;
        Py_INCREF(tmp_raise_type_6);
        exception_value = tmp_raise_value_6;
        exception_lineno = 50;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_16;
    }
    branch_no_18:;
    goto branch_end_17;
    branch_no_17:;
    {
        PyObject *tmp_assign_source_45;
        tmp_assign_source_45 = PyDict_New();
        assert(tmp_class_creation_6__prepared == NULL);
        tmp_class_creation_6__prepared = tmp_assign_source_45;
    }
    branch_end_17:;
    {
        PyObject *tmp_assign_source_46;
        {
            PyObject *tmp_set_locals_6;
            CHECK_OBJECT(tmp_class_creation_6__prepared);
            tmp_set_locals_6 = tmp_class_creation_6__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50 = tmp_set_locals_6;
            Py_INCREF(tmp_set_locals_6);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_18;
        }
        tmp_dictset_value = mod_consts[71];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_18;
        }
        tmp_dictset_value = mod_consts[8];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 50;

            goto try_except_handler_18;
        }
        if (isFrameUnusable(cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4)) {
            Py_XDECREF(cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4);

#if _DEBUG_REFCOUNTS
            if (cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4 = MAKE_FUNCTION_FRAME(codeobj_ab51bb3d6332d4b560ddcc31e7b3fc22, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4->m_type_description == NULL);
        frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4 = cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__5___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 53;
            type_description_2 = "o";
            goto frame_exception_exit_4;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_3;

        frame_exception_exit_4:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4,
            type_description_2,
            outline_5_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4 == cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4);
            cache_frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4 = NULL;
        }

        assertFrameObject(frame_ab51bb3d6332d4b560ddcc31e7b3fc22_4);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_3;

        frame_no_exception_3:;
        goto skip_nested_handling_3;
        nested_frame_exit_3:;

        goto try_except_handler_18;
        skip_nested_handling_3:;
        {
            PyObject *tmp_assign_source_47;
            PyObject *tmp_called_name_12;
            PyObject *tmp_args_name_12;
            PyObject *tmp_tuple_element_23;
            PyObject *tmp_kwargs_name_12;
            CHECK_OBJECT(tmp_class_creation_6__metaclass);
            tmp_called_name_12 = tmp_class_creation_6__metaclass;
            tmp_tuple_element_23 = mod_consts[8];
            tmp_args_name_12 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_12, 0, tmp_tuple_element_23);
            CHECK_OBJECT(tmp_class_creation_6__bases);
            tmp_tuple_element_23 = tmp_class_creation_6__bases;
            PyTuple_SET_ITEM0(tmp_args_name_12, 1, tmp_tuple_element_23);
            tmp_tuple_element_23 = locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50;
            PyTuple_SET_ITEM0(tmp_args_name_12, 2, tmp_tuple_element_23);
            CHECK_OBJECT(tmp_class_creation_6__class_decl_dict);
            tmp_kwargs_name_12 = tmp_class_creation_6__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 50;
            tmp_assign_source_47 = CALL_FUNCTION(tmp_called_name_12, tmp_args_name_12, tmp_kwargs_name_12);
            Py_DECREF(tmp_args_name_12);
            if (tmp_assign_source_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 50;

                goto try_except_handler_18;
            }
            assert(outline_5_var___class__ == NULL);
            outline_5_var___class__ = tmp_assign_source_47;
        }
        CHECK_OBJECT(outline_5_var___class__);
        tmp_assign_source_46 = outline_5_var___class__;
        Py_INCREF(tmp_assign_source_46);
        goto try_return_handler_18;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_18:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50);
        locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50 = NULL;
        goto try_return_handler_17;
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

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50);
        locals_pip$_vendor$urllib3$exceptions$$$class__6_ProxyError_50 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_16;
        exception_value = exception_keeper_value_16;
        exception_tb = exception_keeper_tb_16;
        exception_lineno = exception_keeper_lineno_16;

        goto try_except_handler_17;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_17:;
        CHECK_OBJECT(outline_5_var___class__);
        Py_DECREF(outline_5_var___class__);
        outline_5_var___class__ = NULL;
        goto outline_result_6;
        // Exception handler code:
        try_except_handler_17:;
        exception_keeper_type_17 = exception_type;
        exception_keeper_value_17 = exception_value;
        exception_keeper_tb_17 = exception_tb;
        exception_keeper_lineno_17 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_17;
        exception_value = exception_keeper_value_17;
        exception_tb = exception_keeper_tb_17;
        exception_lineno = exception_keeper_lineno_17;

        goto outline_exception_6;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_6:;
        exception_lineno = 50;
        goto try_except_handler_16;
        outline_result_6:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[8], tmp_assign_source_46);
    }
    goto try_end_6;
    // Exception handler code:
    try_except_handler_16:;
    exception_keeper_type_18 = exception_type;
    exception_keeper_value_18 = exception_value;
    exception_keeper_tb_18 = exception_tb;
    exception_keeper_lineno_18 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_6__bases);
    tmp_class_creation_6__bases = NULL;
    Py_XDECREF(tmp_class_creation_6__class_decl_dict);
    tmp_class_creation_6__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_6__metaclass);
    tmp_class_creation_6__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_6__prepared);
    tmp_class_creation_6__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_18;
    exception_value = exception_keeper_value_18;
    exception_tb = exception_keeper_tb_18;
    exception_lineno = exception_keeper_lineno_18;

    goto frame_exception_exit_1;
    // End of try:
    try_end_6:;
    Py_XDECREF(tmp_class_creation_6__bases);
    tmp_class_creation_6__bases = NULL;
    Py_XDECREF(tmp_class_creation_6__class_decl_dict);
    tmp_class_creation_6__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_6__metaclass);
    tmp_class_creation_6__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_6__prepared);
    Py_DECREF(tmp_class_creation_6__prepared);
    tmp_class_creation_6__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_48;
        PyObject *tmp_tuple_element_24;
        tmp_tuple_element_24 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_24 == NULL)) {
            tmp_tuple_element_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        tmp_assign_source_48 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_48, 0, tmp_tuple_element_24);
        assert(tmp_class_creation_7__bases == NULL);
        tmp_class_creation_7__bases = tmp_assign_source_48;
    }
    {
        PyObject *tmp_assign_source_49;
        tmp_assign_source_49 = PyDict_New();
        assert(tmp_class_creation_7__class_decl_dict == NULL);
        tmp_class_creation_7__class_decl_dict = tmp_assign_source_49;
    }
    {
        PyObject *tmp_assign_source_50;
        PyObject *tmp_metaclass_name_7;
        nuitka_bool tmp_condition_result_25;
        PyObject *tmp_key_name_19;
        PyObject *tmp_dict_arg_name_19;
        PyObject *tmp_dict_arg_name_20;
        PyObject *tmp_key_name_20;
        PyObject *tmp_type_arg_13;
        PyObject *tmp_expression_name_31;
        PyObject *tmp_subscript_name_7;
        PyObject *tmp_bases_name_7;
        tmp_key_name_19 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_7__class_decl_dict);
        tmp_dict_arg_name_19 = tmp_class_creation_7__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_19, tmp_key_name_19);
        assert(!(tmp_res == -1));
        tmp_condition_result_25 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_25 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_7;
        } else {
            goto condexpr_false_7;
        }
        condexpr_true_7:;
        CHECK_OBJECT(tmp_class_creation_7__class_decl_dict);
        tmp_dict_arg_name_20 = tmp_class_creation_7__class_decl_dict;
        tmp_key_name_20 = mod_consts[49];
        tmp_metaclass_name_7 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_20, tmp_key_name_20);
        if (tmp_metaclass_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        goto condexpr_end_7;
        condexpr_false_7:;
        CHECK_OBJECT(tmp_class_creation_7__bases);
        tmp_expression_name_31 = tmp_class_creation_7__bases;
        tmp_subscript_name_7 = mod_consts[46];
        tmp_type_arg_13 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_31, tmp_subscript_name_7, 0);
        if (tmp_type_arg_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        tmp_metaclass_name_7 = BUILTIN_TYPE1(tmp_type_arg_13);
        Py_DECREF(tmp_type_arg_13);
        if (tmp_metaclass_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        condexpr_end_7:;
        CHECK_OBJECT(tmp_class_creation_7__bases);
        tmp_bases_name_7 = tmp_class_creation_7__bases;
        tmp_assign_source_50 = SELECT_METACLASS(tmp_metaclass_name_7, tmp_bases_name_7);
        Py_DECREF(tmp_metaclass_name_7);
        if (tmp_assign_source_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        assert(tmp_class_creation_7__metaclass == NULL);
        tmp_class_creation_7__metaclass = tmp_assign_source_50;
    }
    {
        nuitka_bool tmp_condition_result_26;
        PyObject *tmp_key_name_21;
        PyObject *tmp_dict_arg_name_21;
        tmp_key_name_21 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_7__class_decl_dict);
        tmp_dict_arg_name_21 = tmp_class_creation_7__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_21, tmp_key_name_21);
        assert(!(tmp_res == -1));
        tmp_condition_result_26 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_26 == NUITKA_BOOL_TRUE) {
            goto branch_yes_19;
        } else {
            goto branch_no_19;
        }
    }
    branch_yes_19:;
    CHECK_OBJECT(tmp_class_creation_7__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_7__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 58;

        goto try_except_handler_19;
    }
    branch_no_19:;
    {
        nuitka_bool tmp_condition_result_27;
        PyObject *tmp_expression_name_32;
        CHECK_OBJECT(tmp_class_creation_7__metaclass);
        tmp_expression_name_32 = tmp_class_creation_7__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_32, mod_consts[50]);
        tmp_condition_result_27 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_27 == NUITKA_BOOL_TRUE) {
            goto branch_yes_20;
        } else {
            goto branch_no_20;
        }
    }
    branch_yes_20:;
    {
        PyObject *tmp_assign_source_51;
        PyObject *tmp_called_name_13;
        PyObject *tmp_expression_name_33;
        PyObject *tmp_args_name_13;
        PyObject *tmp_tuple_element_25;
        PyObject *tmp_kwargs_name_13;
        CHECK_OBJECT(tmp_class_creation_7__metaclass);
        tmp_expression_name_33 = tmp_class_creation_7__metaclass;
        tmp_called_name_13 = LOOKUP_ATTRIBUTE(tmp_expression_name_33, mod_consts[50]);
        if (tmp_called_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        tmp_tuple_element_25 = mod_consts[73];
        tmp_args_name_13 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_13, 0, tmp_tuple_element_25);
        CHECK_OBJECT(tmp_class_creation_7__bases);
        tmp_tuple_element_25 = tmp_class_creation_7__bases;
        PyTuple_SET_ITEM0(tmp_args_name_13, 1, tmp_tuple_element_25);
        CHECK_OBJECT(tmp_class_creation_7__class_decl_dict);
        tmp_kwargs_name_13 = tmp_class_creation_7__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 58;
        tmp_assign_source_51 = CALL_FUNCTION(tmp_called_name_13, tmp_args_name_13, tmp_kwargs_name_13);
        Py_DECREF(tmp_called_name_13);
        Py_DECREF(tmp_args_name_13);
        if (tmp_assign_source_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        assert(tmp_class_creation_7__prepared == NULL);
        tmp_class_creation_7__prepared = tmp_assign_source_51;
    }
    {
        nuitka_bool tmp_condition_result_28;
        PyObject *tmp_operand_name_7;
        PyObject *tmp_expression_name_34;
        CHECK_OBJECT(tmp_class_creation_7__prepared);
        tmp_expression_name_34 = tmp_class_creation_7__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_34, mod_consts[51]);
        tmp_operand_name_7 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_7);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        tmp_condition_result_28 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_28 == NUITKA_BOOL_TRUE) {
            goto branch_yes_21;
        } else {
            goto branch_no_21;
        }
    }
    branch_yes_21:;
    {
        PyObject *tmp_raise_type_7;
        PyObject *tmp_raise_value_7;
        PyObject *tmp_left_name_7;
        PyObject *tmp_right_name_7;
        PyObject *tmp_tuple_element_26;
        PyObject *tmp_getattr_target_7;
        PyObject *tmp_getattr_attr_7;
        PyObject *tmp_getattr_default_7;
        tmp_raise_type_7 = PyExc_TypeError;
        tmp_left_name_7 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_7__metaclass);
        tmp_getattr_target_7 = tmp_class_creation_7__metaclass;
        tmp_getattr_attr_7 = mod_consts[53];
        tmp_getattr_default_7 = mod_consts[54];
        tmp_tuple_element_26 = BUILTIN_GETATTR(tmp_getattr_target_7, tmp_getattr_attr_7, tmp_getattr_default_7);
        if (tmp_tuple_element_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        tmp_right_name_7 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_35;
            PyObject *tmp_type_arg_14;
            PyTuple_SET_ITEM(tmp_right_name_7, 0, tmp_tuple_element_26);
            CHECK_OBJECT(tmp_class_creation_7__prepared);
            tmp_type_arg_14 = tmp_class_creation_7__prepared;
            tmp_expression_name_35 = BUILTIN_TYPE1(tmp_type_arg_14);
            assert(!(tmp_expression_name_35 == NULL));
            tmp_tuple_element_26 = LOOKUP_ATTRIBUTE(tmp_expression_name_35, mod_consts[53]);
            Py_DECREF(tmp_expression_name_35);
            if (tmp_tuple_element_26 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 58;

                goto tuple_build_exception_7;
            }
            PyTuple_SET_ITEM(tmp_right_name_7, 1, tmp_tuple_element_26);
        }
        goto tuple_build_noexception_7;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_7:;
        Py_DECREF(tmp_right_name_7);
        goto try_except_handler_19;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_7:;
        tmp_raise_value_7 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_7, tmp_right_name_7);
        Py_DECREF(tmp_right_name_7);
        if (tmp_raise_value_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_19;
        }
        exception_type = tmp_raise_type_7;
        Py_INCREF(tmp_raise_type_7);
        exception_value = tmp_raise_value_7;
        exception_lineno = 58;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_19;
    }
    branch_no_21:;
    goto branch_end_20;
    branch_no_20:;
    {
        PyObject *tmp_assign_source_52;
        tmp_assign_source_52 = PyDict_New();
        assert(tmp_class_creation_7__prepared == NULL);
        tmp_class_creation_7__prepared = tmp_assign_source_52;
    }
    branch_end_20:;
    {
        PyObject *tmp_assign_source_53;
        {
            PyObject *tmp_set_locals_7;
            CHECK_OBJECT(tmp_class_creation_7__prepared);
            tmp_set_locals_7 = tmp_class_creation_7__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58 = tmp_set_locals_7;
            Py_INCREF(tmp_set_locals_7);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_21;
        }
        tmp_dictset_value = mod_consts[74];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_21;
        }
        tmp_dictset_value = mod_consts[73];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 58;

            goto try_except_handler_21;
        }
        {
            PyObject *tmp_assign_source_54;
            PyObject *tmp_called_name_14;
            PyObject *tmp_args_name_14;
            PyObject *tmp_tuple_element_27;
            PyObject *tmp_kwargs_name_14;
            CHECK_OBJECT(tmp_class_creation_7__metaclass);
            tmp_called_name_14 = tmp_class_creation_7__metaclass;
            tmp_tuple_element_27 = mod_consts[73];
            tmp_args_name_14 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_14, 0, tmp_tuple_element_27);
            CHECK_OBJECT(tmp_class_creation_7__bases);
            tmp_tuple_element_27 = tmp_class_creation_7__bases;
            PyTuple_SET_ITEM0(tmp_args_name_14, 1, tmp_tuple_element_27);
            tmp_tuple_element_27 = locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58;
            PyTuple_SET_ITEM0(tmp_args_name_14, 2, tmp_tuple_element_27);
            CHECK_OBJECT(tmp_class_creation_7__class_decl_dict);
            tmp_kwargs_name_14 = tmp_class_creation_7__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 58;
            tmp_assign_source_54 = CALL_FUNCTION(tmp_called_name_14, tmp_args_name_14, tmp_kwargs_name_14);
            Py_DECREF(tmp_args_name_14);
            if (tmp_assign_source_54 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 58;

                goto try_except_handler_21;
            }
            assert(outline_6_var___class__ == NULL);
            outline_6_var___class__ = tmp_assign_source_54;
        }
        CHECK_OBJECT(outline_6_var___class__);
        tmp_assign_source_53 = outline_6_var___class__;
        Py_INCREF(tmp_assign_source_53);
        goto try_return_handler_21;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_21:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58);
        locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58 = NULL;
        goto try_return_handler_20;
        // Exception handler code:
        try_except_handler_21:;
        exception_keeper_type_19 = exception_type;
        exception_keeper_value_19 = exception_value;
        exception_keeper_tb_19 = exception_tb;
        exception_keeper_lineno_19 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58);
        locals_pip$_vendor$urllib3$exceptions$$$class__7_DecodeError_58 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_19;
        exception_value = exception_keeper_value_19;
        exception_tb = exception_keeper_tb_19;
        exception_lineno = exception_keeper_lineno_19;

        goto try_except_handler_20;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_20:;
        CHECK_OBJECT(outline_6_var___class__);
        Py_DECREF(outline_6_var___class__);
        outline_6_var___class__ = NULL;
        goto outline_result_7;
        // Exception handler code:
        try_except_handler_20:;
        exception_keeper_type_20 = exception_type;
        exception_keeper_value_20 = exception_value;
        exception_keeper_tb_20 = exception_tb;
        exception_keeper_lineno_20 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_20;
        exception_value = exception_keeper_value_20;
        exception_tb = exception_keeper_tb_20;
        exception_lineno = exception_keeper_lineno_20;

        goto outline_exception_7;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_7:;
        exception_lineno = 58;
        goto try_except_handler_19;
        outline_result_7:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[73], tmp_assign_source_53);
    }
    goto try_end_7;
    // Exception handler code:
    try_except_handler_19:;
    exception_keeper_type_21 = exception_type;
    exception_keeper_value_21 = exception_value;
    exception_keeper_tb_21 = exception_tb;
    exception_keeper_lineno_21 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_7__bases);
    tmp_class_creation_7__bases = NULL;
    Py_XDECREF(tmp_class_creation_7__class_decl_dict);
    tmp_class_creation_7__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_7__metaclass);
    tmp_class_creation_7__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_7__prepared);
    tmp_class_creation_7__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_21;
    exception_value = exception_keeper_value_21;
    exception_tb = exception_keeper_tb_21;
    exception_lineno = exception_keeper_lineno_21;

    goto frame_exception_exit_1;
    // End of try:
    try_end_7:;
    Py_XDECREF(tmp_class_creation_7__bases);
    tmp_class_creation_7__bases = NULL;
    Py_XDECREF(tmp_class_creation_7__class_decl_dict);
    tmp_class_creation_7__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_7__metaclass);
    tmp_class_creation_7__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_7__prepared);
    Py_DECREF(tmp_class_creation_7__prepared);
    tmp_class_creation_7__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_55;
        PyObject *tmp_tuple_element_28;
        tmp_tuple_element_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_28 == NULL)) {
            tmp_tuple_element_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        tmp_assign_source_55 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_55, 0, tmp_tuple_element_28);
        assert(tmp_class_creation_8__bases == NULL);
        tmp_class_creation_8__bases = tmp_assign_source_55;
    }
    {
        PyObject *tmp_assign_source_56;
        tmp_assign_source_56 = PyDict_New();
        assert(tmp_class_creation_8__class_decl_dict == NULL);
        tmp_class_creation_8__class_decl_dict = tmp_assign_source_56;
    }
    {
        PyObject *tmp_assign_source_57;
        PyObject *tmp_metaclass_name_8;
        nuitka_bool tmp_condition_result_29;
        PyObject *tmp_key_name_22;
        PyObject *tmp_dict_arg_name_22;
        PyObject *tmp_dict_arg_name_23;
        PyObject *tmp_key_name_23;
        PyObject *tmp_type_arg_15;
        PyObject *tmp_expression_name_36;
        PyObject *tmp_subscript_name_8;
        PyObject *tmp_bases_name_8;
        tmp_key_name_22 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_8__class_decl_dict);
        tmp_dict_arg_name_22 = tmp_class_creation_8__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_22, tmp_key_name_22);
        assert(!(tmp_res == -1));
        tmp_condition_result_29 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_29 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_8;
        } else {
            goto condexpr_false_8;
        }
        condexpr_true_8:;
        CHECK_OBJECT(tmp_class_creation_8__class_decl_dict);
        tmp_dict_arg_name_23 = tmp_class_creation_8__class_decl_dict;
        tmp_key_name_23 = mod_consts[49];
        tmp_metaclass_name_8 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_23, tmp_key_name_23);
        if (tmp_metaclass_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        goto condexpr_end_8;
        condexpr_false_8:;
        CHECK_OBJECT(tmp_class_creation_8__bases);
        tmp_expression_name_36 = tmp_class_creation_8__bases;
        tmp_subscript_name_8 = mod_consts[46];
        tmp_type_arg_15 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_36, tmp_subscript_name_8, 0);
        if (tmp_type_arg_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        tmp_metaclass_name_8 = BUILTIN_TYPE1(tmp_type_arg_15);
        Py_DECREF(tmp_type_arg_15);
        if (tmp_metaclass_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        condexpr_end_8:;
        CHECK_OBJECT(tmp_class_creation_8__bases);
        tmp_bases_name_8 = tmp_class_creation_8__bases;
        tmp_assign_source_57 = SELECT_METACLASS(tmp_metaclass_name_8, tmp_bases_name_8);
        Py_DECREF(tmp_metaclass_name_8);
        if (tmp_assign_source_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        assert(tmp_class_creation_8__metaclass == NULL);
        tmp_class_creation_8__metaclass = tmp_assign_source_57;
    }
    {
        nuitka_bool tmp_condition_result_30;
        PyObject *tmp_key_name_24;
        PyObject *tmp_dict_arg_name_24;
        tmp_key_name_24 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_8__class_decl_dict);
        tmp_dict_arg_name_24 = tmp_class_creation_8__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_24, tmp_key_name_24);
        assert(!(tmp_res == -1));
        tmp_condition_result_30 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_30 == NUITKA_BOOL_TRUE) {
            goto branch_yes_22;
        } else {
            goto branch_no_22;
        }
    }
    branch_yes_22:;
    CHECK_OBJECT(tmp_class_creation_8__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_8__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 64;

        goto try_except_handler_22;
    }
    branch_no_22:;
    {
        nuitka_bool tmp_condition_result_31;
        PyObject *tmp_expression_name_37;
        CHECK_OBJECT(tmp_class_creation_8__metaclass);
        tmp_expression_name_37 = tmp_class_creation_8__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_37, mod_consts[50]);
        tmp_condition_result_31 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_31 == NUITKA_BOOL_TRUE) {
            goto branch_yes_23;
        } else {
            goto branch_no_23;
        }
    }
    branch_yes_23:;
    {
        PyObject *tmp_assign_source_58;
        PyObject *tmp_called_name_15;
        PyObject *tmp_expression_name_38;
        PyObject *tmp_args_name_15;
        PyObject *tmp_tuple_element_29;
        PyObject *tmp_kwargs_name_15;
        CHECK_OBJECT(tmp_class_creation_8__metaclass);
        tmp_expression_name_38 = tmp_class_creation_8__metaclass;
        tmp_called_name_15 = LOOKUP_ATTRIBUTE(tmp_expression_name_38, mod_consts[50]);
        if (tmp_called_name_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        tmp_tuple_element_29 = mod_consts[75];
        tmp_args_name_15 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_15, 0, tmp_tuple_element_29);
        CHECK_OBJECT(tmp_class_creation_8__bases);
        tmp_tuple_element_29 = tmp_class_creation_8__bases;
        PyTuple_SET_ITEM0(tmp_args_name_15, 1, tmp_tuple_element_29);
        CHECK_OBJECT(tmp_class_creation_8__class_decl_dict);
        tmp_kwargs_name_15 = tmp_class_creation_8__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 64;
        tmp_assign_source_58 = CALL_FUNCTION(tmp_called_name_15, tmp_args_name_15, tmp_kwargs_name_15);
        Py_DECREF(tmp_called_name_15);
        Py_DECREF(tmp_args_name_15);
        if (tmp_assign_source_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        assert(tmp_class_creation_8__prepared == NULL);
        tmp_class_creation_8__prepared = tmp_assign_source_58;
    }
    {
        nuitka_bool tmp_condition_result_32;
        PyObject *tmp_operand_name_8;
        PyObject *tmp_expression_name_39;
        CHECK_OBJECT(tmp_class_creation_8__prepared);
        tmp_expression_name_39 = tmp_class_creation_8__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_39, mod_consts[51]);
        tmp_operand_name_8 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_8);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        tmp_condition_result_32 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_32 == NUITKA_BOOL_TRUE) {
            goto branch_yes_24;
        } else {
            goto branch_no_24;
        }
    }
    branch_yes_24:;
    {
        PyObject *tmp_raise_type_8;
        PyObject *tmp_raise_value_8;
        PyObject *tmp_left_name_8;
        PyObject *tmp_right_name_8;
        PyObject *tmp_tuple_element_30;
        PyObject *tmp_getattr_target_8;
        PyObject *tmp_getattr_attr_8;
        PyObject *tmp_getattr_default_8;
        tmp_raise_type_8 = PyExc_TypeError;
        tmp_left_name_8 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_8__metaclass);
        tmp_getattr_target_8 = tmp_class_creation_8__metaclass;
        tmp_getattr_attr_8 = mod_consts[53];
        tmp_getattr_default_8 = mod_consts[54];
        tmp_tuple_element_30 = BUILTIN_GETATTR(tmp_getattr_target_8, tmp_getattr_attr_8, tmp_getattr_default_8);
        if (tmp_tuple_element_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        tmp_right_name_8 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_40;
            PyObject *tmp_type_arg_16;
            PyTuple_SET_ITEM(tmp_right_name_8, 0, tmp_tuple_element_30);
            CHECK_OBJECT(tmp_class_creation_8__prepared);
            tmp_type_arg_16 = tmp_class_creation_8__prepared;
            tmp_expression_name_40 = BUILTIN_TYPE1(tmp_type_arg_16);
            assert(!(tmp_expression_name_40 == NULL));
            tmp_tuple_element_30 = LOOKUP_ATTRIBUTE(tmp_expression_name_40, mod_consts[53]);
            Py_DECREF(tmp_expression_name_40);
            if (tmp_tuple_element_30 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 64;

                goto tuple_build_exception_8;
            }
            PyTuple_SET_ITEM(tmp_right_name_8, 1, tmp_tuple_element_30);
        }
        goto tuple_build_noexception_8;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_8:;
        Py_DECREF(tmp_right_name_8);
        goto try_except_handler_22;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_8:;
        tmp_raise_value_8 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_8, tmp_right_name_8);
        Py_DECREF(tmp_right_name_8);
        if (tmp_raise_value_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_22;
        }
        exception_type = tmp_raise_type_8;
        Py_INCREF(tmp_raise_type_8);
        exception_value = tmp_raise_value_8;
        exception_lineno = 64;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_22;
    }
    branch_no_24:;
    goto branch_end_23;
    branch_no_23:;
    {
        PyObject *tmp_assign_source_59;
        tmp_assign_source_59 = PyDict_New();
        assert(tmp_class_creation_8__prepared == NULL);
        tmp_class_creation_8__prepared = tmp_assign_source_59;
    }
    branch_end_23:;
    {
        PyObject *tmp_assign_source_60;
        {
            PyObject *tmp_set_locals_8;
            CHECK_OBJECT(tmp_class_creation_8__prepared);
            tmp_set_locals_8 = tmp_class_creation_8__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64 = tmp_set_locals_8;
            Py_INCREF(tmp_set_locals_8);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_24;
        }
        tmp_dictset_value = mod_consts[76];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_24;
        }
        tmp_dictset_value = mod_consts[75];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 64;

            goto try_except_handler_24;
        }
        {
            PyObject *tmp_assign_source_61;
            PyObject *tmp_called_name_16;
            PyObject *tmp_args_name_16;
            PyObject *tmp_tuple_element_31;
            PyObject *tmp_kwargs_name_16;
            CHECK_OBJECT(tmp_class_creation_8__metaclass);
            tmp_called_name_16 = tmp_class_creation_8__metaclass;
            tmp_tuple_element_31 = mod_consts[75];
            tmp_args_name_16 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_16, 0, tmp_tuple_element_31);
            CHECK_OBJECT(tmp_class_creation_8__bases);
            tmp_tuple_element_31 = tmp_class_creation_8__bases;
            PyTuple_SET_ITEM0(tmp_args_name_16, 1, tmp_tuple_element_31);
            tmp_tuple_element_31 = locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64;
            PyTuple_SET_ITEM0(tmp_args_name_16, 2, tmp_tuple_element_31);
            CHECK_OBJECT(tmp_class_creation_8__class_decl_dict);
            tmp_kwargs_name_16 = tmp_class_creation_8__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 64;
            tmp_assign_source_61 = CALL_FUNCTION(tmp_called_name_16, tmp_args_name_16, tmp_kwargs_name_16);
            Py_DECREF(tmp_args_name_16);
            if (tmp_assign_source_61 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 64;

                goto try_except_handler_24;
            }
            assert(outline_7_var___class__ == NULL);
            outline_7_var___class__ = tmp_assign_source_61;
        }
        CHECK_OBJECT(outline_7_var___class__);
        tmp_assign_source_60 = outline_7_var___class__;
        Py_INCREF(tmp_assign_source_60);
        goto try_return_handler_24;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_24:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64);
        locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64 = NULL;
        goto try_return_handler_23;
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

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64);
        locals_pip$_vendor$urllib3$exceptions$$$class__8_ProtocolError_64 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_22;
        exception_value = exception_keeper_value_22;
        exception_tb = exception_keeper_tb_22;
        exception_lineno = exception_keeper_lineno_22;

        goto try_except_handler_23;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_23:;
        CHECK_OBJECT(outline_7_var___class__);
        Py_DECREF(outline_7_var___class__);
        outline_7_var___class__ = NULL;
        goto outline_result_8;
        // Exception handler code:
        try_except_handler_23:;
        exception_keeper_type_23 = exception_type;
        exception_keeper_value_23 = exception_value;
        exception_keeper_tb_23 = exception_tb;
        exception_keeper_lineno_23 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_23;
        exception_value = exception_keeper_value_23;
        exception_tb = exception_keeper_tb_23;
        exception_lineno = exception_keeper_lineno_23;

        goto outline_exception_8;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_8:;
        exception_lineno = 64;
        goto try_except_handler_22;
        outline_result_8:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[75], tmp_assign_source_60);
    }
    goto try_end_8;
    // Exception handler code:
    try_except_handler_22:;
    exception_keeper_type_24 = exception_type;
    exception_keeper_value_24 = exception_value;
    exception_keeper_tb_24 = exception_tb;
    exception_keeper_lineno_24 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_8__bases);
    tmp_class_creation_8__bases = NULL;
    Py_XDECREF(tmp_class_creation_8__class_decl_dict);
    tmp_class_creation_8__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_8__metaclass);
    tmp_class_creation_8__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_8__prepared);
    tmp_class_creation_8__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_24;
    exception_value = exception_keeper_value_24;
    exception_tb = exception_keeper_tb_24;
    exception_lineno = exception_keeper_lineno_24;

    goto frame_exception_exit_1;
    // End of try:
    try_end_8:;
    Py_XDECREF(tmp_class_creation_8__bases);
    tmp_class_creation_8__bases = NULL;
    Py_XDECREF(tmp_class_creation_8__class_decl_dict);
    tmp_class_creation_8__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_8__metaclass);
    tmp_class_creation_8__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_8__prepared);
    Py_DECREF(tmp_class_creation_8__prepared);
    tmp_class_creation_8__prepared = NULL;
    {
        PyObject *tmp_assign_source_62;
        tmp_assign_source_62 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[75]);

        if (unlikely(tmp_assign_source_62 == NULL)) {
            tmp_assign_source_62 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[75]);
        }

        if (tmp_assign_source_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 71;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[77], tmp_assign_source_62);
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_63;
        PyObject *tmp_tuple_element_32;
        tmp_tuple_element_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[12]);

        if (unlikely(tmp_tuple_element_32 == NULL)) {
            tmp_tuple_element_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
        }

        if (tmp_tuple_element_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        tmp_assign_source_63 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_63, 0, tmp_tuple_element_32);
        assert(tmp_class_creation_9__bases == NULL);
        tmp_class_creation_9__bases = tmp_assign_source_63;
    }
    {
        PyObject *tmp_assign_source_64;
        tmp_assign_source_64 = PyDict_New();
        assert(tmp_class_creation_9__class_decl_dict == NULL);
        tmp_class_creation_9__class_decl_dict = tmp_assign_source_64;
    }
    {
        PyObject *tmp_assign_source_65;
        PyObject *tmp_metaclass_name_9;
        nuitka_bool tmp_condition_result_33;
        PyObject *tmp_key_name_25;
        PyObject *tmp_dict_arg_name_25;
        PyObject *tmp_dict_arg_name_26;
        PyObject *tmp_key_name_26;
        PyObject *tmp_type_arg_17;
        PyObject *tmp_expression_name_41;
        PyObject *tmp_subscript_name_9;
        PyObject *tmp_bases_name_9;
        tmp_key_name_25 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_9__class_decl_dict);
        tmp_dict_arg_name_25 = tmp_class_creation_9__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_25, tmp_key_name_25);
        assert(!(tmp_res == -1));
        tmp_condition_result_33 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_33 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_9;
        } else {
            goto condexpr_false_9;
        }
        condexpr_true_9:;
        CHECK_OBJECT(tmp_class_creation_9__class_decl_dict);
        tmp_dict_arg_name_26 = tmp_class_creation_9__class_decl_dict;
        tmp_key_name_26 = mod_consts[49];
        tmp_metaclass_name_9 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_26, tmp_key_name_26);
        if (tmp_metaclass_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        goto condexpr_end_9;
        condexpr_false_9:;
        CHECK_OBJECT(tmp_class_creation_9__bases);
        tmp_expression_name_41 = tmp_class_creation_9__bases;
        tmp_subscript_name_9 = mod_consts[46];
        tmp_type_arg_17 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_41, tmp_subscript_name_9, 0);
        if (tmp_type_arg_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        tmp_metaclass_name_9 = BUILTIN_TYPE1(tmp_type_arg_17);
        Py_DECREF(tmp_type_arg_17);
        if (tmp_metaclass_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        condexpr_end_9:;
        CHECK_OBJECT(tmp_class_creation_9__bases);
        tmp_bases_name_9 = tmp_class_creation_9__bases;
        tmp_assign_source_65 = SELECT_METACLASS(tmp_metaclass_name_9, tmp_bases_name_9);
        Py_DECREF(tmp_metaclass_name_9);
        if (tmp_assign_source_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        assert(tmp_class_creation_9__metaclass == NULL);
        tmp_class_creation_9__metaclass = tmp_assign_source_65;
    }
    {
        nuitka_bool tmp_condition_result_34;
        PyObject *tmp_key_name_27;
        PyObject *tmp_dict_arg_name_27;
        tmp_key_name_27 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_9__class_decl_dict);
        tmp_dict_arg_name_27 = tmp_class_creation_9__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_27, tmp_key_name_27);
        assert(!(tmp_res == -1));
        tmp_condition_result_34 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_34 == NUITKA_BOOL_TRUE) {
            goto branch_yes_25;
        } else {
            goto branch_no_25;
        }
    }
    branch_yes_25:;
    CHECK_OBJECT(tmp_class_creation_9__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_9__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 77;

        goto try_except_handler_25;
    }
    branch_no_25:;
    {
        nuitka_bool tmp_condition_result_35;
        PyObject *tmp_expression_name_42;
        CHECK_OBJECT(tmp_class_creation_9__metaclass);
        tmp_expression_name_42 = tmp_class_creation_9__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_42, mod_consts[50]);
        tmp_condition_result_35 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_35 == NUITKA_BOOL_TRUE) {
            goto branch_yes_26;
        } else {
            goto branch_no_26;
        }
    }
    branch_yes_26:;
    {
        PyObject *tmp_assign_source_66;
        PyObject *tmp_called_name_17;
        PyObject *tmp_expression_name_43;
        PyObject *tmp_args_name_17;
        PyObject *tmp_tuple_element_33;
        PyObject *tmp_kwargs_name_17;
        CHECK_OBJECT(tmp_class_creation_9__metaclass);
        tmp_expression_name_43 = tmp_class_creation_9__metaclass;
        tmp_called_name_17 = LOOKUP_ATTRIBUTE(tmp_expression_name_43, mod_consts[50]);
        if (tmp_called_name_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        tmp_tuple_element_33 = mod_consts[78];
        tmp_args_name_17 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_17, 0, tmp_tuple_element_33);
        CHECK_OBJECT(tmp_class_creation_9__bases);
        tmp_tuple_element_33 = tmp_class_creation_9__bases;
        PyTuple_SET_ITEM0(tmp_args_name_17, 1, tmp_tuple_element_33);
        CHECK_OBJECT(tmp_class_creation_9__class_decl_dict);
        tmp_kwargs_name_17 = tmp_class_creation_9__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 77;
        tmp_assign_source_66 = CALL_FUNCTION(tmp_called_name_17, tmp_args_name_17, tmp_kwargs_name_17);
        Py_DECREF(tmp_called_name_17);
        Py_DECREF(tmp_args_name_17);
        if (tmp_assign_source_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        assert(tmp_class_creation_9__prepared == NULL);
        tmp_class_creation_9__prepared = tmp_assign_source_66;
    }
    {
        nuitka_bool tmp_condition_result_36;
        PyObject *tmp_operand_name_9;
        PyObject *tmp_expression_name_44;
        CHECK_OBJECT(tmp_class_creation_9__prepared);
        tmp_expression_name_44 = tmp_class_creation_9__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_44, mod_consts[51]);
        tmp_operand_name_9 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_9);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        tmp_condition_result_36 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_36 == NUITKA_BOOL_TRUE) {
            goto branch_yes_27;
        } else {
            goto branch_no_27;
        }
    }
    branch_yes_27:;
    {
        PyObject *tmp_raise_type_9;
        PyObject *tmp_raise_value_9;
        PyObject *tmp_left_name_9;
        PyObject *tmp_right_name_9;
        PyObject *tmp_tuple_element_34;
        PyObject *tmp_getattr_target_9;
        PyObject *tmp_getattr_attr_9;
        PyObject *tmp_getattr_default_9;
        tmp_raise_type_9 = PyExc_TypeError;
        tmp_left_name_9 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_9__metaclass);
        tmp_getattr_target_9 = tmp_class_creation_9__metaclass;
        tmp_getattr_attr_9 = mod_consts[53];
        tmp_getattr_default_9 = mod_consts[54];
        tmp_tuple_element_34 = BUILTIN_GETATTR(tmp_getattr_target_9, tmp_getattr_attr_9, tmp_getattr_default_9);
        if (tmp_tuple_element_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        tmp_right_name_9 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_45;
            PyObject *tmp_type_arg_18;
            PyTuple_SET_ITEM(tmp_right_name_9, 0, tmp_tuple_element_34);
            CHECK_OBJECT(tmp_class_creation_9__prepared);
            tmp_type_arg_18 = tmp_class_creation_9__prepared;
            tmp_expression_name_45 = BUILTIN_TYPE1(tmp_type_arg_18);
            assert(!(tmp_expression_name_45 == NULL));
            tmp_tuple_element_34 = LOOKUP_ATTRIBUTE(tmp_expression_name_45, mod_consts[53]);
            Py_DECREF(tmp_expression_name_45);
            if (tmp_tuple_element_34 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 77;

                goto tuple_build_exception_9;
            }
            PyTuple_SET_ITEM(tmp_right_name_9, 1, tmp_tuple_element_34);
        }
        goto tuple_build_noexception_9;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_9:;
        Py_DECREF(tmp_right_name_9);
        goto try_except_handler_25;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_9:;
        tmp_raise_value_9 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_9, tmp_right_name_9);
        Py_DECREF(tmp_right_name_9);
        if (tmp_raise_value_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_25;
        }
        exception_type = tmp_raise_type_9;
        Py_INCREF(tmp_raise_type_9);
        exception_value = tmp_raise_value_9;
        exception_lineno = 77;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_25;
    }
    branch_no_27:;
    goto branch_end_26;
    branch_no_26:;
    {
        PyObject *tmp_assign_source_67;
        tmp_assign_source_67 = PyDict_New();
        assert(tmp_class_creation_9__prepared == NULL);
        tmp_class_creation_9__prepared = tmp_assign_source_67;
    }
    branch_end_26:;
    {
        PyObject *tmp_assign_source_68;
        {
            PyObject *tmp_set_locals_9;
            CHECK_OBJECT(tmp_class_creation_9__prepared);
            tmp_set_locals_9 = tmp_class_creation_9__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77 = tmp_set_locals_9;
            Py_INCREF(tmp_set_locals_9);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_27;
        }
        tmp_dictset_value = mod_consts[79];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_27;
        }
        tmp_dictset_value = mod_consts[78];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 77;

            goto try_except_handler_27;
        }
        if (isFrameUnusable(cache_frame_d4612ad00762592c3f3b527f730e5d41_5)) {
            Py_XDECREF(cache_frame_d4612ad00762592c3f3b527f730e5d41_5);

#if _DEBUG_REFCOUNTS
            if (cache_frame_d4612ad00762592c3f3b527f730e5d41_5 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_d4612ad00762592c3f3b527f730e5d41_5 = MAKE_FUNCTION_FRAME(codeobj_d4612ad00762592c3f3b527f730e5d41, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_d4612ad00762592c3f3b527f730e5d41_5->m_type_description == NULL);
        frame_d4612ad00762592c3f3b527f730e5d41_5 = cache_frame_d4612ad00762592c3f3b527f730e5d41_5;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_d4612ad00762592c3f3b527f730e5d41_5);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_d4612ad00762592c3f3b527f730e5d41_5) == 2); // Frame stack

        // Framed code:
        {
            PyObject *tmp_defaults_1;
            tmp_defaults_1 = mod_consts[80];
            Py_INCREF(tmp_defaults_1);


            tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__6___init__(tmp_defaults_1);

            tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77, mod_consts[2], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 87;
                type_description_2 = "o";
                goto frame_exception_exit_5;
            }
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_d4612ad00762592c3f3b527f730e5d41_5);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_4;

        frame_exception_exit_5:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_d4612ad00762592c3f3b527f730e5d41_5);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_d4612ad00762592c3f3b527f730e5d41_5, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_d4612ad00762592c3f3b527f730e5d41_5->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_d4612ad00762592c3f3b527f730e5d41_5, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_d4612ad00762592c3f3b527f730e5d41_5,
            type_description_2,
            outline_8_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_d4612ad00762592c3f3b527f730e5d41_5 == cache_frame_d4612ad00762592c3f3b527f730e5d41_5) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_d4612ad00762592c3f3b527f730e5d41_5);
            cache_frame_d4612ad00762592c3f3b527f730e5d41_5 = NULL;
        }

        assertFrameObject(frame_d4612ad00762592c3f3b527f730e5d41_5);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_4;

        frame_no_exception_4:;
        goto skip_nested_handling_4;
        nested_frame_exit_4:;

        goto try_except_handler_27;
        skip_nested_handling_4:;
        {
            PyObject *tmp_assign_source_69;
            PyObject *tmp_called_name_18;
            PyObject *tmp_args_name_18;
            PyObject *tmp_tuple_element_35;
            PyObject *tmp_kwargs_name_18;
            CHECK_OBJECT(tmp_class_creation_9__metaclass);
            tmp_called_name_18 = tmp_class_creation_9__metaclass;
            tmp_tuple_element_35 = mod_consts[78];
            tmp_args_name_18 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_18, 0, tmp_tuple_element_35);
            CHECK_OBJECT(tmp_class_creation_9__bases);
            tmp_tuple_element_35 = tmp_class_creation_9__bases;
            PyTuple_SET_ITEM0(tmp_args_name_18, 1, tmp_tuple_element_35);
            tmp_tuple_element_35 = locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77;
            PyTuple_SET_ITEM0(tmp_args_name_18, 2, tmp_tuple_element_35);
            CHECK_OBJECT(tmp_class_creation_9__class_decl_dict);
            tmp_kwargs_name_18 = tmp_class_creation_9__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 77;
            tmp_assign_source_69 = CALL_FUNCTION(tmp_called_name_18, tmp_args_name_18, tmp_kwargs_name_18);
            Py_DECREF(tmp_args_name_18);
            if (tmp_assign_source_69 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 77;

                goto try_except_handler_27;
            }
            assert(outline_8_var___class__ == NULL);
            outline_8_var___class__ = tmp_assign_source_69;
        }
        CHECK_OBJECT(outline_8_var___class__);
        tmp_assign_source_68 = outline_8_var___class__;
        Py_INCREF(tmp_assign_source_68);
        goto try_return_handler_27;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_27:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77);
        locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77 = NULL;
        goto try_return_handler_26;
        // Exception handler code:
        try_except_handler_27:;
        exception_keeper_type_25 = exception_type;
        exception_keeper_value_25 = exception_value;
        exception_keeper_tb_25 = exception_tb;
        exception_keeper_lineno_25 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77);
        locals_pip$_vendor$urllib3$exceptions$$$class__9_MaxRetryError_77 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_25;
        exception_value = exception_keeper_value_25;
        exception_tb = exception_keeper_tb_25;
        exception_lineno = exception_keeper_lineno_25;

        goto try_except_handler_26;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_26:;
        CHECK_OBJECT(outline_8_var___class__);
        Py_DECREF(outline_8_var___class__);
        outline_8_var___class__ = NULL;
        goto outline_result_9;
        // Exception handler code:
        try_except_handler_26:;
        exception_keeper_type_26 = exception_type;
        exception_keeper_value_26 = exception_value;
        exception_keeper_tb_26 = exception_tb;
        exception_keeper_lineno_26 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_26;
        exception_value = exception_keeper_value_26;
        exception_tb = exception_keeper_tb_26;
        exception_lineno = exception_keeper_lineno_26;

        goto outline_exception_9;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_9:;
        exception_lineno = 77;
        goto try_except_handler_25;
        outline_result_9:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[78], tmp_assign_source_68);
    }
    goto try_end_9;
    // Exception handler code:
    try_except_handler_25:;
    exception_keeper_type_27 = exception_type;
    exception_keeper_value_27 = exception_value;
    exception_keeper_tb_27 = exception_tb;
    exception_keeper_lineno_27 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_9__bases);
    tmp_class_creation_9__bases = NULL;
    Py_XDECREF(tmp_class_creation_9__class_decl_dict);
    tmp_class_creation_9__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_9__metaclass);
    tmp_class_creation_9__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_9__prepared);
    tmp_class_creation_9__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_27;
    exception_value = exception_keeper_value_27;
    exception_tb = exception_keeper_tb_27;
    exception_lineno = exception_keeper_lineno_27;

    goto frame_exception_exit_1;
    // End of try:
    try_end_9:;
    Py_XDECREF(tmp_class_creation_9__bases);
    tmp_class_creation_9__bases = NULL;
    Py_XDECREF(tmp_class_creation_9__class_decl_dict);
    tmp_class_creation_9__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_9__metaclass);
    tmp_class_creation_9__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_9__prepared);
    Py_DECREF(tmp_class_creation_9__prepared);
    tmp_class_creation_9__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_70;
        PyObject *tmp_tuple_element_36;
        tmp_tuple_element_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[12]);

        if (unlikely(tmp_tuple_element_36 == NULL)) {
            tmp_tuple_element_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
        }

        if (tmp_tuple_element_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        tmp_assign_source_70 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_70, 0, tmp_tuple_element_36);
        assert(tmp_class_creation_10__bases == NULL);
        tmp_class_creation_10__bases = tmp_assign_source_70;
    }
    {
        PyObject *tmp_assign_source_71;
        tmp_assign_source_71 = PyDict_New();
        assert(tmp_class_creation_10__class_decl_dict == NULL);
        tmp_class_creation_10__class_decl_dict = tmp_assign_source_71;
    }
    {
        PyObject *tmp_assign_source_72;
        PyObject *tmp_metaclass_name_10;
        nuitka_bool tmp_condition_result_37;
        PyObject *tmp_key_name_28;
        PyObject *tmp_dict_arg_name_28;
        PyObject *tmp_dict_arg_name_29;
        PyObject *tmp_key_name_29;
        PyObject *tmp_type_arg_19;
        PyObject *tmp_expression_name_46;
        PyObject *tmp_subscript_name_10;
        PyObject *tmp_bases_name_10;
        tmp_key_name_28 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_10__class_decl_dict);
        tmp_dict_arg_name_28 = tmp_class_creation_10__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_28, tmp_key_name_28);
        assert(!(tmp_res == -1));
        tmp_condition_result_37 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_37 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_10;
        } else {
            goto condexpr_false_10;
        }
        condexpr_true_10:;
        CHECK_OBJECT(tmp_class_creation_10__class_decl_dict);
        tmp_dict_arg_name_29 = tmp_class_creation_10__class_decl_dict;
        tmp_key_name_29 = mod_consts[49];
        tmp_metaclass_name_10 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_29, tmp_key_name_29);
        if (tmp_metaclass_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        goto condexpr_end_10;
        condexpr_false_10:;
        CHECK_OBJECT(tmp_class_creation_10__bases);
        tmp_expression_name_46 = tmp_class_creation_10__bases;
        tmp_subscript_name_10 = mod_consts[46];
        tmp_type_arg_19 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_46, tmp_subscript_name_10, 0);
        if (tmp_type_arg_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        tmp_metaclass_name_10 = BUILTIN_TYPE1(tmp_type_arg_19);
        Py_DECREF(tmp_type_arg_19);
        if (tmp_metaclass_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        condexpr_end_10:;
        CHECK_OBJECT(tmp_class_creation_10__bases);
        tmp_bases_name_10 = tmp_class_creation_10__bases;
        tmp_assign_source_72 = SELECT_METACLASS(tmp_metaclass_name_10, tmp_bases_name_10);
        Py_DECREF(tmp_metaclass_name_10);
        if (tmp_assign_source_72 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        assert(tmp_class_creation_10__metaclass == NULL);
        tmp_class_creation_10__metaclass = tmp_assign_source_72;
    }
    {
        nuitka_bool tmp_condition_result_38;
        PyObject *tmp_key_name_30;
        PyObject *tmp_dict_arg_name_30;
        tmp_key_name_30 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_10__class_decl_dict);
        tmp_dict_arg_name_30 = tmp_class_creation_10__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_30, tmp_key_name_30);
        assert(!(tmp_res == -1));
        tmp_condition_result_38 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_38 == NUITKA_BOOL_TRUE) {
            goto branch_yes_28;
        } else {
            goto branch_no_28;
        }
    }
    branch_yes_28:;
    CHECK_OBJECT(tmp_class_creation_10__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_10__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 95;

        goto try_except_handler_28;
    }
    branch_no_28:;
    {
        nuitka_bool tmp_condition_result_39;
        PyObject *tmp_expression_name_47;
        CHECK_OBJECT(tmp_class_creation_10__metaclass);
        tmp_expression_name_47 = tmp_class_creation_10__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_47, mod_consts[50]);
        tmp_condition_result_39 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_39 == NUITKA_BOOL_TRUE) {
            goto branch_yes_29;
        } else {
            goto branch_no_29;
        }
    }
    branch_yes_29:;
    {
        PyObject *tmp_assign_source_73;
        PyObject *tmp_called_name_19;
        PyObject *tmp_expression_name_48;
        PyObject *tmp_args_name_19;
        PyObject *tmp_tuple_element_37;
        PyObject *tmp_kwargs_name_19;
        CHECK_OBJECT(tmp_class_creation_10__metaclass);
        tmp_expression_name_48 = tmp_class_creation_10__metaclass;
        tmp_called_name_19 = LOOKUP_ATTRIBUTE(tmp_expression_name_48, mod_consts[50]);
        if (tmp_called_name_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        tmp_tuple_element_37 = mod_consts[82];
        tmp_args_name_19 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_19, 0, tmp_tuple_element_37);
        CHECK_OBJECT(tmp_class_creation_10__bases);
        tmp_tuple_element_37 = tmp_class_creation_10__bases;
        PyTuple_SET_ITEM0(tmp_args_name_19, 1, tmp_tuple_element_37);
        CHECK_OBJECT(tmp_class_creation_10__class_decl_dict);
        tmp_kwargs_name_19 = tmp_class_creation_10__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 95;
        tmp_assign_source_73 = CALL_FUNCTION(tmp_called_name_19, tmp_args_name_19, tmp_kwargs_name_19);
        Py_DECREF(tmp_called_name_19);
        Py_DECREF(tmp_args_name_19);
        if (tmp_assign_source_73 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        assert(tmp_class_creation_10__prepared == NULL);
        tmp_class_creation_10__prepared = tmp_assign_source_73;
    }
    {
        nuitka_bool tmp_condition_result_40;
        PyObject *tmp_operand_name_10;
        PyObject *tmp_expression_name_49;
        CHECK_OBJECT(tmp_class_creation_10__prepared);
        tmp_expression_name_49 = tmp_class_creation_10__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_49, mod_consts[51]);
        tmp_operand_name_10 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_10);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        tmp_condition_result_40 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_40 == NUITKA_BOOL_TRUE) {
            goto branch_yes_30;
        } else {
            goto branch_no_30;
        }
    }
    branch_yes_30:;
    {
        PyObject *tmp_raise_type_10;
        PyObject *tmp_raise_value_10;
        PyObject *tmp_left_name_10;
        PyObject *tmp_right_name_10;
        PyObject *tmp_tuple_element_38;
        PyObject *tmp_getattr_target_10;
        PyObject *tmp_getattr_attr_10;
        PyObject *tmp_getattr_default_10;
        tmp_raise_type_10 = PyExc_TypeError;
        tmp_left_name_10 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_10__metaclass);
        tmp_getattr_target_10 = tmp_class_creation_10__metaclass;
        tmp_getattr_attr_10 = mod_consts[53];
        tmp_getattr_default_10 = mod_consts[54];
        tmp_tuple_element_38 = BUILTIN_GETATTR(tmp_getattr_target_10, tmp_getattr_attr_10, tmp_getattr_default_10);
        if (tmp_tuple_element_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        tmp_right_name_10 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_50;
            PyObject *tmp_type_arg_20;
            PyTuple_SET_ITEM(tmp_right_name_10, 0, tmp_tuple_element_38);
            CHECK_OBJECT(tmp_class_creation_10__prepared);
            tmp_type_arg_20 = tmp_class_creation_10__prepared;
            tmp_expression_name_50 = BUILTIN_TYPE1(tmp_type_arg_20);
            assert(!(tmp_expression_name_50 == NULL));
            tmp_tuple_element_38 = LOOKUP_ATTRIBUTE(tmp_expression_name_50, mod_consts[53]);
            Py_DECREF(tmp_expression_name_50);
            if (tmp_tuple_element_38 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 95;

                goto tuple_build_exception_10;
            }
            PyTuple_SET_ITEM(tmp_right_name_10, 1, tmp_tuple_element_38);
        }
        goto tuple_build_noexception_10;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_10:;
        Py_DECREF(tmp_right_name_10);
        goto try_except_handler_28;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_10:;
        tmp_raise_value_10 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_10, tmp_right_name_10);
        Py_DECREF(tmp_right_name_10);
        if (tmp_raise_value_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_28;
        }
        exception_type = tmp_raise_type_10;
        Py_INCREF(tmp_raise_type_10);
        exception_value = tmp_raise_value_10;
        exception_lineno = 95;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_28;
    }
    branch_no_30:;
    goto branch_end_29;
    branch_no_29:;
    {
        PyObject *tmp_assign_source_74;
        tmp_assign_source_74 = PyDict_New();
        assert(tmp_class_creation_10__prepared == NULL);
        tmp_class_creation_10__prepared = tmp_assign_source_74;
    }
    branch_end_29:;
    {
        PyObject *tmp_assign_source_75;
        {
            PyObject *tmp_set_locals_10;
            CHECK_OBJECT(tmp_class_creation_10__prepared);
            tmp_set_locals_10 = tmp_class_creation_10__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95 = tmp_set_locals_10;
            Py_INCREF(tmp_set_locals_10);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_30;
        }
        tmp_dictset_value = mod_consts[83];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_30;
        }
        tmp_dictset_value = mod_consts[82];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 95;

            goto try_except_handler_30;
        }
        if (isFrameUnusable(cache_frame_c61a422c602595da887bdeb6ad5c1789_6)) {
            Py_XDECREF(cache_frame_c61a422c602595da887bdeb6ad5c1789_6);

#if _DEBUG_REFCOUNTS
            if (cache_frame_c61a422c602595da887bdeb6ad5c1789_6 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_c61a422c602595da887bdeb6ad5c1789_6 = MAKE_FUNCTION_FRAME(codeobj_c61a422c602595da887bdeb6ad5c1789, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_c61a422c602595da887bdeb6ad5c1789_6->m_type_description == NULL);
        frame_c61a422c602595da887bdeb6ad5c1789_6 = cache_frame_c61a422c602595da887bdeb6ad5c1789_6;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_c61a422c602595da887bdeb6ad5c1789_6);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_c61a422c602595da887bdeb6ad5c1789_6) == 2); // Frame stack

        // Framed code:
        {
            PyObject *tmp_defaults_2;
            tmp_defaults_2 = mod_consts[84];
            Py_INCREF(tmp_defaults_2);


            tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__7___init__(tmp_defaults_2);

            tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95, mod_consts[2], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 98;
                type_description_2 = "o";
                goto frame_exception_exit_6;
            }
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_c61a422c602595da887bdeb6ad5c1789_6);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_5;

        frame_exception_exit_6:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_c61a422c602595da887bdeb6ad5c1789_6);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_c61a422c602595da887bdeb6ad5c1789_6, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_c61a422c602595da887bdeb6ad5c1789_6->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_c61a422c602595da887bdeb6ad5c1789_6, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_c61a422c602595da887bdeb6ad5c1789_6,
            type_description_2,
            outline_9_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_c61a422c602595da887bdeb6ad5c1789_6 == cache_frame_c61a422c602595da887bdeb6ad5c1789_6) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_c61a422c602595da887bdeb6ad5c1789_6);
            cache_frame_c61a422c602595da887bdeb6ad5c1789_6 = NULL;
        }

        assertFrameObject(frame_c61a422c602595da887bdeb6ad5c1789_6);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_5;

        frame_no_exception_5:;
        goto skip_nested_handling_5;
        nested_frame_exit_5:;

        goto try_except_handler_30;
        skip_nested_handling_5:;
        {
            PyObject *tmp_assign_source_76;
            PyObject *tmp_called_name_20;
            PyObject *tmp_args_name_20;
            PyObject *tmp_tuple_element_39;
            PyObject *tmp_kwargs_name_20;
            CHECK_OBJECT(tmp_class_creation_10__metaclass);
            tmp_called_name_20 = tmp_class_creation_10__metaclass;
            tmp_tuple_element_39 = mod_consts[82];
            tmp_args_name_20 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_20, 0, tmp_tuple_element_39);
            CHECK_OBJECT(tmp_class_creation_10__bases);
            tmp_tuple_element_39 = tmp_class_creation_10__bases;
            PyTuple_SET_ITEM0(tmp_args_name_20, 1, tmp_tuple_element_39);
            tmp_tuple_element_39 = locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95;
            PyTuple_SET_ITEM0(tmp_args_name_20, 2, tmp_tuple_element_39);
            CHECK_OBJECT(tmp_class_creation_10__class_decl_dict);
            tmp_kwargs_name_20 = tmp_class_creation_10__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 95;
            tmp_assign_source_76 = CALL_FUNCTION(tmp_called_name_20, tmp_args_name_20, tmp_kwargs_name_20);
            Py_DECREF(tmp_args_name_20);
            if (tmp_assign_source_76 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 95;

                goto try_except_handler_30;
            }
            assert(outline_9_var___class__ == NULL);
            outline_9_var___class__ = tmp_assign_source_76;
        }
        CHECK_OBJECT(outline_9_var___class__);
        tmp_assign_source_75 = outline_9_var___class__;
        Py_INCREF(tmp_assign_source_75);
        goto try_return_handler_30;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_30:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95);
        locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95 = NULL;
        goto try_return_handler_29;
        // Exception handler code:
        try_except_handler_30:;
        exception_keeper_type_28 = exception_type;
        exception_keeper_value_28 = exception_value;
        exception_keeper_tb_28 = exception_tb;
        exception_keeper_lineno_28 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95);
        locals_pip$_vendor$urllib3$exceptions$$$class__10_HostChangedError_95 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_28;
        exception_value = exception_keeper_value_28;
        exception_tb = exception_keeper_tb_28;
        exception_lineno = exception_keeper_lineno_28;

        goto try_except_handler_29;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_29:;
        CHECK_OBJECT(outline_9_var___class__);
        Py_DECREF(outline_9_var___class__);
        outline_9_var___class__ = NULL;
        goto outline_result_10;
        // Exception handler code:
        try_except_handler_29:;
        exception_keeper_type_29 = exception_type;
        exception_keeper_value_29 = exception_value;
        exception_keeper_tb_29 = exception_tb;
        exception_keeper_lineno_29 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_29;
        exception_value = exception_keeper_value_29;
        exception_tb = exception_keeper_tb_29;
        exception_lineno = exception_keeper_lineno_29;

        goto outline_exception_10;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_10:;
        exception_lineno = 95;
        goto try_except_handler_28;
        outline_result_10:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[82], tmp_assign_source_75);
    }
    goto try_end_10;
    // Exception handler code:
    try_except_handler_28:;
    exception_keeper_type_30 = exception_type;
    exception_keeper_value_30 = exception_value;
    exception_keeper_tb_30 = exception_tb;
    exception_keeper_lineno_30 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_10__bases);
    tmp_class_creation_10__bases = NULL;
    Py_XDECREF(tmp_class_creation_10__class_decl_dict);
    tmp_class_creation_10__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_10__metaclass);
    tmp_class_creation_10__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_10__prepared);
    tmp_class_creation_10__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_30;
    exception_value = exception_keeper_value_30;
    exception_tb = exception_keeper_tb_30;
    exception_lineno = exception_keeper_lineno_30;

    goto frame_exception_exit_1;
    // End of try:
    try_end_10:;
    Py_XDECREF(tmp_class_creation_10__bases);
    tmp_class_creation_10__bases = NULL;
    Py_XDECREF(tmp_class_creation_10__class_decl_dict);
    tmp_class_creation_10__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_10__metaclass);
    tmp_class_creation_10__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_10__prepared);
    Py_DECREF(tmp_class_creation_10__prepared);
    tmp_class_creation_10__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_77;
        PyObject *tmp_tuple_element_40;
        tmp_tuple_element_40 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_40 == NULL)) {
            tmp_tuple_element_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        tmp_assign_source_77 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_77, 0, tmp_tuple_element_40);
        assert(tmp_class_creation_11__bases == NULL);
        tmp_class_creation_11__bases = tmp_assign_source_77;
    }
    {
        PyObject *tmp_assign_source_78;
        tmp_assign_source_78 = PyDict_New();
        assert(tmp_class_creation_11__class_decl_dict == NULL);
        tmp_class_creation_11__class_decl_dict = tmp_assign_source_78;
    }
    {
        PyObject *tmp_assign_source_79;
        PyObject *tmp_metaclass_name_11;
        nuitka_bool tmp_condition_result_41;
        PyObject *tmp_key_name_31;
        PyObject *tmp_dict_arg_name_31;
        PyObject *tmp_dict_arg_name_32;
        PyObject *tmp_key_name_32;
        PyObject *tmp_type_arg_21;
        PyObject *tmp_expression_name_51;
        PyObject *tmp_subscript_name_11;
        PyObject *tmp_bases_name_11;
        tmp_key_name_31 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_11__class_decl_dict);
        tmp_dict_arg_name_31 = tmp_class_creation_11__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_31, tmp_key_name_31);
        assert(!(tmp_res == -1));
        tmp_condition_result_41 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_41 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_11;
        } else {
            goto condexpr_false_11;
        }
        condexpr_true_11:;
        CHECK_OBJECT(tmp_class_creation_11__class_decl_dict);
        tmp_dict_arg_name_32 = tmp_class_creation_11__class_decl_dict;
        tmp_key_name_32 = mod_consts[49];
        tmp_metaclass_name_11 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_32, tmp_key_name_32);
        if (tmp_metaclass_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        goto condexpr_end_11;
        condexpr_false_11:;
        CHECK_OBJECT(tmp_class_creation_11__bases);
        tmp_expression_name_51 = tmp_class_creation_11__bases;
        tmp_subscript_name_11 = mod_consts[46];
        tmp_type_arg_21 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_51, tmp_subscript_name_11, 0);
        if (tmp_type_arg_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        tmp_metaclass_name_11 = BUILTIN_TYPE1(tmp_type_arg_21);
        Py_DECREF(tmp_type_arg_21);
        if (tmp_metaclass_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        condexpr_end_11:;
        CHECK_OBJECT(tmp_class_creation_11__bases);
        tmp_bases_name_11 = tmp_class_creation_11__bases;
        tmp_assign_source_79 = SELECT_METACLASS(tmp_metaclass_name_11, tmp_bases_name_11);
        Py_DECREF(tmp_metaclass_name_11);
        if (tmp_assign_source_79 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        assert(tmp_class_creation_11__metaclass == NULL);
        tmp_class_creation_11__metaclass = tmp_assign_source_79;
    }
    {
        nuitka_bool tmp_condition_result_42;
        PyObject *tmp_key_name_33;
        PyObject *tmp_dict_arg_name_33;
        tmp_key_name_33 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_11__class_decl_dict);
        tmp_dict_arg_name_33 = tmp_class_creation_11__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_33, tmp_key_name_33);
        assert(!(tmp_res == -1));
        tmp_condition_result_42 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_42 == NUITKA_BOOL_TRUE) {
            goto branch_yes_31;
        } else {
            goto branch_no_31;
        }
    }
    branch_yes_31:;
    CHECK_OBJECT(tmp_class_creation_11__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_11__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 104;

        goto try_except_handler_31;
    }
    branch_no_31:;
    {
        nuitka_bool tmp_condition_result_43;
        PyObject *tmp_expression_name_52;
        CHECK_OBJECT(tmp_class_creation_11__metaclass);
        tmp_expression_name_52 = tmp_class_creation_11__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_52, mod_consts[50]);
        tmp_condition_result_43 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_43 == NUITKA_BOOL_TRUE) {
            goto branch_yes_32;
        } else {
            goto branch_no_32;
        }
    }
    branch_yes_32:;
    {
        PyObject *tmp_assign_source_80;
        PyObject *tmp_called_name_21;
        PyObject *tmp_expression_name_53;
        PyObject *tmp_args_name_21;
        PyObject *tmp_tuple_element_41;
        PyObject *tmp_kwargs_name_21;
        CHECK_OBJECT(tmp_class_creation_11__metaclass);
        tmp_expression_name_53 = tmp_class_creation_11__metaclass;
        tmp_called_name_21 = LOOKUP_ATTRIBUTE(tmp_expression_name_53, mod_consts[50]);
        if (tmp_called_name_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        tmp_tuple_element_41 = mod_consts[86];
        tmp_args_name_21 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_21, 0, tmp_tuple_element_41);
        CHECK_OBJECT(tmp_class_creation_11__bases);
        tmp_tuple_element_41 = tmp_class_creation_11__bases;
        PyTuple_SET_ITEM0(tmp_args_name_21, 1, tmp_tuple_element_41);
        CHECK_OBJECT(tmp_class_creation_11__class_decl_dict);
        tmp_kwargs_name_21 = tmp_class_creation_11__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 104;
        tmp_assign_source_80 = CALL_FUNCTION(tmp_called_name_21, tmp_args_name_21, tmp_kwargs_name_21);
        Py_DECREF(tmp_called_name_21);
        Py_DECREF(tmp_args_name_21);
        if (tmp_assign_source_80 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        assert(tmp_class_creation_11__prepared == NULL);
        tmp_class_creation_11__prepared = tmp_assign_source_80;
    }
    {
        nuitka_bool tmp_condition_result_44;
        PyObject *tmp_operand_name_11;
        PyObject *tmp_expression_name_54;
        CHECK_OBJECT(tmp_class_creation_11__prepared);
        tmp_expression_name_54 = tmp_class_creation_11__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_54, mod_consts[51]);
        tmp_operand_name_11 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_11);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        tmp_condition_result_44 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_44 == NUITKA_BOOL_TRUE) {
            goto branch_yes_33;
        } else {
            goto branch_no_33;
        }
    }
    branch_yes_33:;
    {
        PyObject *tmp_raise_type_11;
        PyObject *tmp_raise_value_11;
        PyObject *tmp_left_name_11;
        PyObject *tmp_right_name_11;
        PyObject *tmp_tuple_element_42;
        PyObject *tmp_getattr_target_11;
        PyObject *tmp_getattr_attr_11;
        PyObject *tmp_getattr_default_11;
        tmp_raise_type_11 = PyExc_TypeError;
        tmp_left_name_11 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_11__metaclass);
        tmp_getattr_target_11 = tmp_class_creation_11__metaclass;
        tmp_getattr_attr_11 = mod_consts[53];
        tmp_getattr_default_11 = mod_consts[54];
        tmp_tuple_element_42 = BUILTIN_GETATTR(tmp_getattr_target_11, tmp_getattr_attr_11, tmp_getattr_default_11);
        if (tmp_tuple_element_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        tmp_right_name_11 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_55;
            PyObject *tmp_type_arg_22;
            PyTuple_SET_ITEM(tmp_right_name_11, 0, tmp_tuple_element_42);
            CHECK_OBJECT(tmp_class_creation_11__prepared);
            tmp_type_arg_22 = tmp_class_creation_11__prepared;
            tmp_expression_name_55 = BUILTIN_TYPE1(tmp_type_arg_22);
            assert(!(tmp_expression_name_55 == NULL));
            tmp_tuple_element_42 = LOOKUP_ATTRIBUTE(tmp_expression_name_55, mod_consts[53]);
            Py_DECREF(tmp_expression_name_55);
            if (tmp_tuple_element_42 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 104;

                goto tuple_build_exception_11;
            }
            PyTuple_SET_ITEM(tmp_right_name_11, 1, tmp_tuple_element_42);
        }
        goto tuple_build_noexception_11;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_11:;
        Py_DECREF(tmp_right_name_11);
        goto try_except_handler_31;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_11:;
        tmp_raise_value_11 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_11, tmp_right_name_11);
        Py_DECREF(tmp_right_name_11);
        if (tmp_raise_value_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_31;
        }
        exception_type = tmp_raise_type_11;
        Py_INCREF(tmp_raise_type_11);
        exception_value = tmp_raise_value_11;
        exception_lineno = 104;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_31;
    }
    branch_no_33:;
    goto branch_end_32;
    branch_no_32:;
    {
        PyObject *tmp_assign_source_81;
        tmp_assign_source_81 = PyDict_New();
        assert(tmp_class_creation_11__prepared == NULL);
        tmp_class_creation_11__prepared = tmp_assign_source_81;
    }
    branch_end_32:;
    {
        PyObject *tmp_assign_source_82;
        {
            PyObject *tmp_set_locals_11;
            CHECK_OBJECT(tmp_class_creation_11__prepared);
            tmp_set_locals_11 = tmp_class_creation_11__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104 = tmp_set_locals_11;
            Py_INCREF(tmp_set_locals_11);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_33;
        }
        tmp_dictset_value = mod_consts[87];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_33;
        }
        tmp_dictset_value = mod_consts[86];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 104;

            goto try_except_handler_33;
        }
        {
            PyObject *tmp_assign_source_83;
            PyObject *tmp_called_name_22;
            PyObject *tmp_args_name_22;
            PyObject *tmp_tuple_element_43;
            PyObject *tmp_kwargs_name_22;
            CHECK_OBJECT(tmp_class_creation_11__metaclass);
            tmp_called_name_22 = tmp_class_creation_11__metaclass;
            tmp_tuple_element_43 = mod_consts[86];
            tmp_args_name_22 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_22, 0, tmp_tuple_element_43);
            CHECK_OBJECT(tmp_class_creation_11__bases);
            tmp_tuple_element_43 = tmp_class_creation_11__bases;
            PyTuple_SET_ITEM0(tmp_args_name_22, 1, tmp_tuple_element_43);
            tmp_tuple_element_43 = locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104;
            PyTuple_SET_ITEM0(tmp_args_name_22, 2, tmp_tuple_element_43);
            CHECK_OBJECT(tmp_class_creation_11__class_decl_dict);
            tmp_kwargs_name_22 = tmp_class_creation_11__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 104;
            tmp_assign_source_83 = CALL_FUNCTION(tmp_called_name_22, tmp_args_name_22, tmp_kwargs_name_22);
            Py_DECREF(tmp_args_name_22);
            if (tmp_assign_source_83 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 104;

                goto try_except_handler_33;
            }
            assert(outline_10_var___class__ == NULL);
            outline_10_var___class__ = tmp_assign_source_83;
        }
        CHECK_OBJECT(outline_10_var___class__);
        tmp_assign_source_82 = outline_10_var___class__;
        Py_INCREF(tmp_assign_source_82);
        goto try_return_handler_33;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_33:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104);
        locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104 = NULL;
        goto try_return_handler_32;
        // Exception handler code:
        try_except_handler_33:;
        exception_keeper_type_31 = exception_type;
        exception_keeper_value_31 = exception_value;
        exception_keeper_tb_31 = exception_tb;
        exception_keeper_lineno_31 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104);
        locals_pip$_vendor$urllib3$exceptions$$$class__11_TimeoutStateError_104 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_31;
        exception_value = exception_keeper_value_31;
        exception_tb = exception_keeper_tb_31;
        exception_lineno = exception_keeper_lineno_31;

        goto try_except_handler_32;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_32:;
        CHECK_OBJECT(outline_10_var___class__);
        Py_DECREF(outline_10_var___class__);
        outline_10_var___class__ = NULL;
        goto outline_result_11;
        // Exception handler code:
        try_except_handler_32:;
        exception_keeper_type_32 = exception_type;
        exception_keeper_value_32 = exception_value;
        exception_keeper_tb_32 = exception_tb;
        exception_keeper_lineno_32 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_32;
        exception_value = exception_keeper_value_32;
        exception_tb = exception_keeper_tb_32;
        exception_lineno = exception_keeper_lineno_32;

        goto outline_exception_11;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_11:;
        exception_lineno = 104;
        goto try_except_handler_31;
        outline_result_11:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[86], tmp_assign_source_82);
    }
    goto try_end_11;
    // Exception handler code:
    try_except_handler_31:;
    exception_keeper_type_33 = exception_type;
    exception_keeper_value_33 = exception_value;
    exception_keeper_tb_33 = exception_tb;
    exception_keeper_lineno_33 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_11__bases);
    tmp_class_creation_11__bases = NULL;
    Py_XDECREF(tmp_class_creation_11__class_decl_dict);
    tmp_class_creation_11__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_11__metaclass);
    tmp_class_creation_11__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_11__prepared);
    tmp_class_creation_11__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_33;
    exception_value = exception_keeper_value_33;
    exception_tb = exception_keeper_tb_33;
    exception_lineno = exception_keeper_lineno_33;

    goto frame_exception_exit_1;
    // End of try:
    try_end_11:;
    Py_XDECREF(tmp_class_creation_11__bases);
    tmp_class_creation_11__bases = NULL;
    Py_XDECREF(tmp_class_creation_11__class_decl_dict);
    tmp_class_creation_11__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_11__metaclass);
    tmp_class_creation_11__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_11__prepared);
    Py_DECREF(tmp_class_creation_11__prepared);
    tmp_class_creation_11__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_84;
        PyObject *tmp_tuple_element_44;
        tmp_tuple_element_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_44 == NULL)) {
            tmp_tuple_element_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        tmp_assign_source_84 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_84, 0, tmp_tuple_element_44);
        assert(tmp_class_creation_12__bases == NULL);
        tmp_class_creation_12__bases = tmp_assign_source_84;
    }
    {
        PyObject *tmp_assign_source_85;
        tmp_assign_source_85 = PyDict_New();
        assert(tmp_class_creation_12__class_decl_dict == NULL);
        tmp_class_creation_12__class_decl_dict = tmp_assign_source_85;
    }
    {
        PyObject *tmp_assign_source_86;
        PyObject *tmp_metaclass_name_12;
        nuitka_bool tmp_condition_result_45;
        PyObject *tmp_key_name_34;
        PyObject *tmp_dict_arg_name_34;
        PyObject *tmp_dict_arg_name_35;
        PyObject *tmp_key_name_35;
        PyObject *tmp_type_arg_23;
        PyObject *tmp_expression_name_56;
        PyObject *tmp_subscript_name_12;
        PyObject *tmp_bases_name_12;
        tmp_key_name_34 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_12__class_decl_dict);
        tmp_dict_arg_name_34 = tmp_class_creation_12__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_34, tmp_key_name_34);
        assert(!(tmp_res == -1));
        tmp_condition_result_45 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_45 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_12;
        } else {
            goto condexpr_false_12;
        }
        condexpr_true_12:;
        CHECK_OBJECT(tmp_class_creation_12__class_decl_dict);
        tmp_dict_arg_name_35 = tmp_class_creation_12__class_decl_dict;
        tmp_key_name_35 = mod_consts[49];
        tmp_metaclass_name_12 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_35, tmp_key_name_35);
        if (tmp_metaclass_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        goto condexpr_end_12;
        condexpr_false_12:;
        CHECK_OBJECT(tmp_class_creation_12__bases);
        tmp_expression_name_56 = tmp_class_creation_12__bases;
        tmp_subscript_name_12 = mod_consts[46];
        tmp_type_arg_23 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_56, tmp_subscript_name_12, 0);
        if (tmp_type_arg_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        tmp_metaclass_name_12 = BUILTIN_TYPE1(tmp_type_arg_23);
        Py_DECREF(tmp_type_arg_23);
        if (tmp_metaclass_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        condexpr_end_12:;
        CHECK_OBJECT(tmp_class_creation_12__bases);
        tmp_bases_name_12 = tmp_class_creation_12__bases;
        tmp_assign_source_86 = SELECT_METACLASS(tmp_metaclass_name_12, tmp_bases_name_12);
        Py_DECREF(tmp_metaclass_name_12);
        if (tmp_assign_source_86 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        assert(tmp_class_creation_12__metaclass == NULL);
        tmp_class_creation_12__metaclass = tmp_assign_source_86;
    }
    {
        nuitka_bool tmp_condition_result_46;
        PyObject *tmp_key_name_36;
        PyObject *tmp_dict_arg_name_36;
        tmp_key_name_36 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_12__class_decl_dict);
        tmp_dict_arg_name_36 = tmp_class_creation_12__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_36, tmp_key_name_36);
        assert(!(tmp_res == -1));
        tmp_condition_result_46 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_46 == NUITKA_BOOL_TRUE) {
            goto branch_yes_34;
        } else {
            goto branch_no_34;
        }
    }
    branch_yes_34:;
    CHECK_OBJECT(tmp_class_creation_12__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_12__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 110;

        goto try_except_handler_34;
    }
    branch_no_34:;
    {
        nuitka_bool tmp_condition_result_47;
        PyObject *tmp_expression_name_57;
        CHECK_OBJECT(tmp_class_creation_12__metaclass);
        tmp_expression_name_57 = tmp_class_creation_12__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_57, mod_consts[50]);
        tmp_condition_result_47 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_47 == NUITKA_BOOL_TRUE) {
            goto branch_yes_35;
        } else {
            goto branch_no_35;
        }
    }
    branch_yes_35:;
    {
        PyObject *tmp_assign_source_87;
        PyObject *tmp_called_name_23;
        PyObject *tmp_expression_name_58;
        PyObject *tmp_args_name_23;
        PyObject *tmp_tuple_element_45;
        PyObject *tmp_kwargs_name_23;
        CHECK_OBJECT(tmp_class_creation_12__metaclass);
        tmp_expression_name_58 = tmp_class_creation_12__metaclass;
        tmp_called_name_23 = LOOKUP_ATTRIBUTE(tmp_expression_name_58, mod_consts[50]);
        if (tmp_called_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        tmp_tuple_element_45 = mod_consts[88];
        tmp_args_name_23 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_23, 0, tmp_tuple_element_45);
        CHECK_OBJECT(tmp_class_creation_12__bases);
        tmp_tuple_element_45 = tmp_class_creation_12__bases;
        PyTuple_SET_ITEM0(tmp_args_name_23, 1, tmp_tuple_element_45);
        CHECK_OBJECT(tmp_class_creation_12__class_decl_dict);
        tmp_kwargs_name_23 = tmp_class_creation_12__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 110;
        tmp_assign_source_87 = CALL_FUNCTION(tmp_called_name_23, tmp_args_name_23, tmp_kwargs_name_23);
        Py_DECREF(tmp_called_name_23);
        Py_DECREF(tmp_args_name_23);
        if (tmp_assign_source_87 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        assert(tmp_class_creation_12__prepared == NULL);
        tmp_class_creation_12__prepared = tmp_assign_source_87;
    }
    {
        nuitka_bool tmp_condition_result_48;
        PyObject *tmp_operand_name_12;
        PyObject *tmp_expression_name_59;
        CHECK_OBJECT(tmp_class_creation_12__prepared);
        tmp_expression_name_59 = tmp_class_creation_12__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_59, mod_consts[51]);
        tmp_operand_name_12 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_12);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        tmp_condition_result_48 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_48 == NUITKA_BOOL_TRUE) {
            goto branch_yes_36;
        } else {
            goto branch_no_36;
        }
    }
    branch_yes_36:;
    {
        PyObject *tmp_raise_type_12;
        PyObject *tmp_raise_value_12;
        PyObject *tmp_left_name_12;
        PyObject *tmp_right_name_12;
        PyObject *tmp_tuple_element_46;
        PyObject *tmp_getattr_target_12;
        PyObject *tmp_getattr_attr_12;
        PyObject *tmp_getattr_default_12;
        tmp_raise_type_12 = PyExc_TypeError;
        tmp_left_name_12 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_12__metaclass);
        tmp_getattr_target_12 = tmp_class_creation_12__metaclass;
        tmp_getattr_attr_12 = mod_consts[53];
        tmp_getattr_default_12 = mod_consts[54];
        tmp_tuple_element_46 = BUILTIN_GETATTR(tmp_getattr_target_12, tmp_getattr_attr_12, tmp_getattr_default_12);
        if (tmp_tuple_element_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        tmp_right_name_12 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_60;
            PyObject *tmp_type_arg_24;
            PyTuple_SET_ITEM(tmp_right_name_12, 0, tmp_tuple_element_46);
            CHECK_OBJECT(tmp_class_creation_12__prepared);
            tmp_type_arg_24 = tmp_class_creation_12__prepared;
            tmp_expression_name_60 = BUILTIN_TYPE1(tmp_type_arg_24);
            assert(!(tmp_expression_name_60 == NULL));
            tmp_tuple_element_46 = LOOKUP_ATTRIBUTE(tmp_expression_name_60, mod_consts[53]);
            Py_DECREF(tmp_expression_name_60);
            if (tmp_tuple_element_46 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 110;

                goto tuple_build_exception_12;
            }
            PyTuple_SET_ITEM(tmp_right_name_12, 1, tmp_tuple_element_46);
        }
        goto tuple_build_noexception_12;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_12:;
        Py_DECREF(tmp_right_name_12);
        goto try_except_handler_34;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_12:;
        tmp_raise_value_12 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_12, tmp_right_name_12);
        Py_DECREF(tmp_right_name_12);
        if (tmp_raise_value_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_34;
        }
        exception_type = tmp_raise_type_12;
        Py_INCREF(tmp_raise_type_12);
        exception_value = tmp_raise_value_12;
        exception_lineno = 110;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_34;
    }
    branch_no_36:;
    goto branch_end_35;
    branch_no_35:;
    {
        PyObject *tmp_assign_source_88;
        tmp_assign_source_88 = PyDict_New();
        assert(tmp_class_creation_12__prepared == NULL);
        tmp_class_creation_12__prepared = tmp_assign_source_88;
    }
    branch_end_35:;
    {
        PyObject *tmp_assign_source_89;
        {
            PyObject *tmp_set_locals_12;
            CHECK_OBJECT(tmp_class_creation_12__prepared);
            tmp_set_locals_12 = tmp_class_creation_12__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110 = tmp_set_locals_12;
            Py_INCREF(tmp_set_locals_12);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_36;
        }
        tmp_dictset_value = mod_consts[89];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_36;
        }
        tmp_dictset_value = mod_consts[88];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 110;

            goto try_except_handler_36;
        }
        {
            PyObject *tmp_assign_source_90;
            PyObject *tmp_called_name_24;
            PyObject *tmp_args_name_24;
            PyObject *tmp_tuple_element_47;
            PyObject *tmp_kwargs_name_24;
            CHECK_OBJECT(tmp_class_creation_12__metaclass);
            tmp_called_name_24 = tmp_class_creation_12__metaclass;
            tmp_tuple_element_47 = mod_consts[88];
            tmp_args_name_24 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_24, 0, tmp_tuple_element_47);
            CHECK_OBJECT(tmp_class_creation_12__bases);
            tmp_tuple_element_47 = tmp_class_creation_12__bases;
            PyTuple_SET_ITEM0(tmp_args_name_24, 1, tmp_tuple_element_47);
            tmp_tuple_element_47 = locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110;
            PyTuple_SET_ITEM0(tmp_args_name_24, 2, tmp_tuple_element_47);
            CHECK_OBJECT(tmp_class_creation_12__class_decl_dict);
            tmp_kwargs_name_24 = tmp_class_creation_12__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 110;
            tmp_assign_source_90 = CALL_FUNCTION(tmp_called_name_24, tmp_args_name_24, tmp_kwargs_name_24);
            Py_DECREF(tmp_args_name_24);
            if (tmp_assign_source_90 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 110;

                goto try_except_handler_36;
            }
            assert(outline_11_var___class__ == NULL);
            outline_11_var___class__ = tmp_assign_source_90;
        }
        CHECK_OBJECT(outline_11_var___class__);
        tmp_assign_source_89 = outline_11_var___class__;
        Py_INCREF(tmp_assign_source_89);
        goto try_return_handler_36;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_36:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110);
        locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110 = NULL;
        goto try_return_handler_35;
        // Exception handler code:
        try_except_handler_36:;
        exception_keeper_type_34 = exception_type;
        exception_keeper_value_34 = exception_value;
        exception_keeper_tb_34 = exception_tb;
        exception_keeper_lineno_34 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110);
        locals_pip$_vendor$urllib3$exceptions$$$class__12_TimeoutError_110 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_34;
        exception_value = exception_keeper_value_34;
        exception_tb = exception_keeper_tb_34;
        exception_lineno = exception_keeper_lineno_34;

        goto try_except_handler_35;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_35:;
        CHECK_OBJECT(outline_11_var___class__);
        Py_DECREF(outline_11_var___class__);
        outline_11_var___class__ = NULL;
        goto outline_result_12;
        // Exception handler code:
        try_except_handler_35:;
        exception_keeper_type_35 = exception_type;
        exception_keeper_value_35 = exception_value;
        exception_keeper_tb_35 = exception_tb;
        exception_keeper_lineno_35 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_35;
        exception_value = exception_keeper_value_35;
        exception_tb = exception_keeper_tb_35;
        exception_lineno = exception_keeper_lineno_35;

        goto outline_exception_12;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_12:;
        exception_lineno = 110;
        goto try_except_handler_34;
        outline_result_12:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[88], tmp_assign_source_89);
    }
    goto try_end_12;
    // Exception handler code:
    try_except_handler_34:;
    exception_keeper_type_36 = exception_type;
    exception_keeper_value_36 = exception_value;
    exception_keeper_tb_36 = exception_tb;
    exception_keeper_lineno_36 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_12__bases);
    tmp_class_creation_12__bases = NULL;
    Py_XDECREF(tmp_class_creation_12__class_decl_dict);
    tmp_class_creation_12__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_12__metaclass);
    tmp_class_creation_12__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_12__prepared);
    tmp_class_creation_12__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_36;
    exception_value = exception_keeper_value_36;
    exception_tb = exception_keeper_tb_36;
    exception_lineno = exception_keeper_lineno_36;

    goto frame_exception_exit_1;
    // End of try:
    try_end_12:;
    Py_XDECREF(tmp_class_creation_12__bases);
    tmp_class_creation_12__bases = NULL;
    Py_XDECREF(tmp_class_creation_12__class_decl_dict);
    tmp_class_creation_12__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_12__metaclass);
    tmp_class_creation_12__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_12__prepared);
    Py_DECREF(tmp_class_creation_12__prepared);
    tmp_class_creation_12__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_91;
        PyObject *tmp_tuple_element_48;
        tmp_tuple_element_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[88]);

        if (unlikely(tmp_tuple_element_48 == NULL)) {
            tmp_tuple_element_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[88]);
        }

        if (tmp_tuple_element_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        tmp_assign_source_91 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_assign_source_91, 0, tmp_tuple_element_48);
        tmp_tuple_element_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[12]);

        if (unlikely(tmp_tuple_element_48 == NULL)) {
            tmp_tuple_element_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
        }

        if (tmp_tuple_element_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto tuple_build_exception_13;
        }
        PyTuple_SET_ITEM0(tmp_assign_source_91, 1, tmp_tuple_element_48);
        goto tuple_build_noexception_13;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_13:;
        Py_DECREF(tmp_assign_source_91);
        goto try_except_handler_37;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_13:;
        assert(tmp_class_creation_13__bases == NULL);
        tmp_class_creation_13__bases = tmp_assign_source_91;
    }
    {
        PyObject *tmp_assign_source_92;
        tmp_assign_source_92 = PyDict_New();
        assert(tmp_class_creation_13__class_decl_dict == NULL);
        tmp_class_creation_13__class_decl_dict = tmp_assign_source_92;
    }
    {
        PyObject *tmp_assign_source_93;
        PyObject *tmp_metaclass_name_13;
        nuitka_bool tmp_condition_result_49;
        PyObject *tmp_key_name_37;
        PyObject *tmp_dict_arg_name_37;
        PyObject *tmp_dict_arg_name_38;
        PyObject *tmp_key_name_38;
        PyObject *tmp_type_arg_25;
        PyObject *tmp_expression_name_61;
        PyObject *tmp_subscript_name_13;
        PyObject *tmp_bases_name_13;
        tmp_key_name_37 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_13__class_decl_dict);
        tmp_dict_arg_name_37 = tmp_class_creation_13__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_37, tmp_key_name_37);
        assert(!(tmp_res == -1));
        tmp_condition_result_49 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_49 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_13;
        } else {
            goto condexpr_false_13;
        }
        condexpr_true_13:;
        CHECK_OBJECT(tmp_class_creation_13__class_decl_dict);
        tmp_dict_arg_name_38 = tmp_class_creation_13__class_decl_dict;
        tmp_key_name_38 = mod_consts[49];
        tmp_metaclass_name_13 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_38, tmp_key_name_38);
        if (tmp_metaclass_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        goto condexpr_end_13;
        condexpr_false_13:;
        CHECK_OBJECT(tmp_class_creation_13__bases);
        tmp_expression_name_61 = tmp_class_creation_13__bases;
        tmp_subscript_name_13 = mod_consts[46];
        tmp_type_arg_25 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_61, tmp_subscript_name_13, 0);
        if (tmp_type_arg_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        tmp_metaclass_name_13 = BUILTIN_TYPE1(tmp_type_arg_25);
        Py_DECREF(tmp_type_arg_25);
        if (tmp_metaclass_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        condexpr_end_13:;
        CHECK_OBJECT(tmp_class_creation_13__bases);
        tmp_bases_name_13 = tmp_class_creation_13__bases;
        tmp_assign_source_93 = SELECT_METACLASS(tmp_metaclass_name_13, tmp_bases_name_13);
        Py_DECREF(tmp_metaclass_name_13);
        if (tmp_assign_source_93 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        assert(tmp_class_creation_13__metaclass == NULL);
        tmp_class_creation_13__metaclass = tmp_assign_source_93;
    }
    {
        nuitka_bool tmp_condition_result_50;
        PyObject *tmp_key_name_39;
        PyObject *tmp_dict_arg_name_39;
        tmp_key_name_39 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_13__class_decl_dict);
        tmp_dict_arg_name_39 = tmp_class_creation_13__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_39, tmp_key_name_39);
        assert(!(tmp_res == -1));
        tmp_condition_result_50 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_50 == NUITKA_BOOL_TRUE) {
            goto branch_yes_37;
        } else {
            goto branch_no_37;
        }
    }
    branch_yes_37:;
    CHECK_OBJECT(tmp_class_creation_13__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_13__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 120;

        goto try_except_handler_37;
    }
    branch_no_37:;
    {
        nuitka_bool tmp_condition_result_51;
        PyObject *tmp_expression_name_62;
        CHECK_OBJECT(tmp_class_creation_13__metaclass);
        tmp_expression_name_62 = tmp_class_creation_13__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_62, mod_consts[50]);
        tmp_condition_result_51 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_51 == NUITKA_BOOL_TRUE) {
            goto branch_yes_38;
        } else {
            goto branch_no_38;
        }
    }
    branch_yes_38:;
    {
        PyObject *tmp_assign_source_94;
        PyObject *tmp_called_name_25;
        PyObject *tmp_expression_name_63;
        PyObject *tmp_args_name_25;
        PyObject *tmp_tuple_element_49;
        PyObject *tmp_kwargs_name_25;
        CHECK_OBJECT(tmp_class_creation_13__metaclass);
        tmp_expression_name_63 = tmp_class_creation_13__metaclass;
        tmp_called_name_25 = LOOKUP_ATTRIBUTE(tmp_expression_name_63, mod_consts[50]);
        if (tmp_called_name_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        tmp_tuple_element_49 = mod_consts[90];
        tmp_args_name_25 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_25, 0, tmp_tuple_element_49);
        CHECK_OBJECT(tmp_class_creation_13__bases);
        tmp_tuple_element_49 = tmp_class_creation_13__bases;
        PyTuple_SET_ITEM0(tmp_args_name_25, 1, tmp_tuple_element_49);
        CHECK_OBJECT(tmp_class_creation_13__class_decl_dict);
        tmp_kwargs_name_25 = tmp_class_creation_13__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 120;
        tmp_assign_source_94 = CALL_FUNCTION(tmp_called_name_25, tmp_args_name_25, tmp_kwargs_name_25);
        Py_DECREF(tmp_called_name_25);
        Py_DECREF(tmp_args_name_25);
        if (tmp_assign_source_94 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        assert(tmp_class_creation_13__prepared == NULL);
        tmp_class_creation_13__prepared = tmp_assign_source_94;
    }
    {
        nuitka_bool tmp_condition_result_52;
        PyObject *tmp_operand_name_13;
        PyObject *tmp_expression_name_64;
        CHECK_OBJECT(tmp_class_creation_13__prepared);
        tmp_expression_name_64 = tmp_class_creation_13__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_64, mod_consts[51]);
        tmp_operand_name_13 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_13);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        tmp_condition_result_52 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_52 == NUITKA_BOOL_TRUE) {
            goto branch_yes_39;
        } else {
            goto branch_no_39;
        }
    }
    branch_yes_39:;
    {
        PyObject *tmp_raise_type_13;
        PyObject *tmp_raise_value_13;
        PyObject *tmp_left_name_13;
        PyObject *tmp_right_name_13;
        PyObject *tmp_tuple_element_50;
        PyObject *tmp_getattr_target_13;
        PyObject *tmp_getattr_attr_13;
        PyObject *tmp_getattr_default_13;
        tmp_raise_type_13 = PyExc_TypeError;
        tmp_left_name_13 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_13__metaclass);
        tmp_getattr_target_13 = tmp_class_creation_13__metaclass;
        tmp_getattr_attr_13 = mod_consts[53];
        tmp_getattr_default_13 = mod_consts[54];
        tmp_tuple_element_50 = BUILTIN_GETATTR(tmp_getattr_target_13, tmp_getattr_attr_13, tmp_getattr_default_13);
        if (tmp_tuple_element_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        tmp_right_name_13 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_65;
            PyObject *tmp_type_arg_26;
            PyTuple_SET_ITEM(tmp_right_name_13, 0, tmp_tuple_element_50);
            CHECK_OBJECT(tmp_class_creation_13__prepared);
            tmp_type_arg_26 = tmp_class_creation_13__prepared;
            tmp_expression_name_65 = BUILTIN_TYPE1(tmp_type_arg_26);
            assert(!(tmp_expression_name_65 == NULL));
            tmp_tuple_element_50 = LOOKUP_ATTRIBUTE(tmp_expression_name_65, mod_consts[53]);
            Py_DECREF(tmp_expression_name_65);
            if (tmp_tuple_element_50 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 120;

                goto tuple_build_exception_14;
            }
            PyTuple_SET_ITEM(tmp_right_name_13, 1, tmp_tuple_element_50);
        }
        goto tuple_build_noexception_14;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_14:;
        Py_DECREF(tmp_right_name_13);
        goto try_except_handler_37;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_14:;
        tmp_raise_value_13 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_13, tmp_right_name_13);
        Py_DECREF(tmp_right_name_13);
        if (tmp_raise_value_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_37;
        }
        exception_type = tmp_raise_type_13;
        Py_INCREF(tmp_raise_type_13);
        exception_value = tmp_raise_value_13;
        exception_lineno = 120;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_37;
    }
    branch_no_39:;
    goto branch_end_38;
    branch_no_38:;
    {
        PyObject *tmp_assign_source_95;
        tmp_assign_source_95 = PyDict_New();
        assert(tmp_class_creation_13__prepared == NULL);
        tmp_class_creation_13__prepared = tmp_assign_source_95;
    }
    branch_end_38:;
    {
        PyObject *tmp_assign_source_96;
        {
            PyObject *tmp_set_locals_13;
            CHECK_OBJECT(tmp_class_creation_13__prepared);
            tmp_set_locals_13 = tmp_class_creation_13__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120 = tmp_set_locals_13;
            Py_INCREF(tmp_set_locals_13);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_39;
        }
        tmp_dictset_value = mod_consts[91];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_39;
        }
        tmp_dictset_value = mod_consts[90];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 120;

            goto try_except_handler_39;
        }
        {
            PyObject *tmp_assign_source_97;
            PyObject *tmp_called_name_26;
            PyObject *tmp_args_name_26;
            PyObject *tmp_tuple_element_51;
            PyObject *tmp_kwargs_name_26;
            CHECK_OBJECT(tmp_class_creation_13__metaclass);
            tmp_called_name_26 = tmp_class_creation_13__metaclass;
            tmp_tuple_element_51 = mod_consts[90];
            tmp_args_name_26 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_26, 0, tmp_tuple_element_51);
            CHECK_OBJECT(tmp_class_creation_13__bases);
            tmp_tuple_element_51 = tmp_class_creation_13__bases;
            PyTuple_SET_ITEM0(tmp_args_name_26, 1, tmp_tuple_element_51);
            tmp_tuple_element_51 = locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120;
            PyTuple_SET_ITEM0(tmp_args_name_26, 2, tmp_tuple_element_51);
            CHECK_OBJECT(tmp_class_creation_13__class_decl_dict);
            tmp_kwargs_name_26 = tmp_class_creation_13__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 120;
            tmp_assign_source_97 = CALL_FUNCTION(tmp_called_name_26, tmp_args_name_26, tmp_kwargs_name_26);
            Py_DECREF(tmp_args_name_26);
            if (tmp_assign_source_97 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 120;

                goto try_except_handler_39;
            }
            assert(outline_12_var___class__ == NULL);
            outline_12_var___class__ = tmp_assign_source_97;
        }
        CHECK_OBJECT(outline_12_var___class__);
        tmp_assign_source_96 = outline_12_var___class__;
        Py_INCREF(tmp_assign_source_96);
        goto try_return_handler_39;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_39:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120);
        locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120 = NULL;
        goto try_return_handler_38;
        // Exception handler code:
        try_except_handler_39:;
        exception_keeper_type_37 = exception_type;
        exception_keeper_value_37 = exception_value;
        exception_keeper_tb_37 = exception_tb;
        exception_keeper_lineno_37 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120);
        locals_pip$_vendor$urllib3$exceptions$$$class__13_ReadTimeoutError_120 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_37;
        exception_value = exception_keeper_value_37;
        exception_tb = exception_keeper_tb_37;
        exception_lineno = exception_keeper_lineno_37;

        goto try_except_handler_38;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_38:;
        CHECK_OBJECT(outline_12_var___class__);
        Py_DECREF(outline_12_var___class__);
        outline_12_var___class__ = NULL;
        goto outline_result_13;
        // Exception handler code:
        try_except_handler_38:;
        exception_keeper_type_38 = exception_type;
        exception_keeper_value_38 = exception_value;
        exception_keeper_tb_38 = exception_tb;
        exception_keeper_lineno_38 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_38;
        exception_value = exception_keeper_value_38;
        exception_tb = exception_keeper_tb_38;
        exception_lineno = exception_keeper_lineno_38;

        goto outline_exception_13;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_13:;
        exception_lineno = 120;
        goto try_except_handler_37;
        outline_result_13:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[90], tmp_assign_source_96);
    }
    goto try_end_13;
    // Exception handler code:
    try_except_handler_37:;
    exception_keeper_type_39 = exception_type;
    exception_keeper_value_39 = exception_value;
    exception_keeper_tb_39 = exception_tb;
    exception_keeper_lineno_39 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_13__bases);
    tmp_class_creation_13__bases = NULL;
    Py_XDECREF(tmp_class_creation_13__class_decl_dict);
    tmp_class_creation_13__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_13__metaclass);
    tmp_class_creation_13__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_13__prepared);
    tmp_class_creation_13__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_39;
    exception_value = exception_keeper_value_39;
    exception_tb = exception_keeper_tb_39;
    exception_lineno = exception_keeper_lineno_39;

    goto frame_exception_exit_1;
    // End of try:
    try_end_13:;
    Py_XDECREF(tmp_class_creation_13__bases);
    tmp_class_creation_13__bases = NULL;
    Py_XDECREF(tmp_class_creation_13__class_decl_dict);
    tmp_class_creation_13__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_13__metaclass);
    tmp_class_creation_13__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_13__prepared);
    Py_DECREF(tmp_class_creation_13__prepared);
    tmp_class_creation_13__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_98;
        PyObject *tmp_tuple_element_52;
        tmp_tuple_element_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[88]);

        if (unlikely(tmp_tuple_element_52 == NULL)) {
            tmp_tuple_element_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[88]);
        }

        if (tmp_tuple_element_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        tmp_assign_source_98 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_98, 0, tmp_tuple_element_52);
        assert(tmp_class_creation_14__bases == NULL);
        tmp_class_creation_14__bases = tmp_assign_source_98;
    }
    {
        PyObject *tmp_assign_source_99;
        tmp_assign_source_99 = PyDict_New();
        assert(tmp_class_creation_14__class_decl_dict == NULL);
        tmp_class_creation_14__class_decl_dict = tmp_assign_source_99;
    }
    {
        PyObject *tmp_assign_source_100;
        PyObject *tmp_metaclass_name_14;
        nuitka_bool tmp_condition_result_53;
        PyObject *tmp_key_name_40;
        PyObject *tmp_dict_arg_name_40;
        PyObject *tmp_dict_arg_name_41;
        PyObject *tmp_key_name_41;
        PyObject *tmp_type_arg_27;
        PyObject *tmp_expression_name_66;
        PyObject *tmp_subscript_name_14;
        PyObject *tmp_bases_name_14;
        tmp_key_name_40 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_14__class_decl_dict);
        tmp_dict_arg_name_40 = tmp_class_creation_14__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_40, tmp_key_name_40);
        assert(!(tmp_res == -1));
        tmp_condition_result_53 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_53 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_14;
        } else {
            goto condexpr_false_14;
        }
        condexpr_true_14:;
        CHECK_OBJECT(tmp_class_creation_14__class_decl_dict);
        tmp_dict_arg_name_41 = tmp_class_creation_14__class_decl_dict;
        tmp_key_name_41 = mod_consts[49];
        tmp_metaclass_name_14 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_41, tmp_key_name_41);
        if (tmp_metaclass_name_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        goto condexpr_end_14;
        condexpr_false_14:;
        CHECK_OBJECT(tmp_class_creation_14__bases);
        tmp_expression_name_66 = tmp_class_creation_14__bases;
        tmp_subscript_name_14 = mod_consts[46];
        tmp_type_arg_27 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_66, tmp_subscript_name_14, 0);
        if (tmp_type_arg_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        tmp_metaclass_name_14 = BUILTIN_TYPE1(tmp_type_arg_27);
        Py_DECREF(tmp_type_arg_27);
        if (tmp_metaclass_name_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        condexpr_end_14:;
        CHECK_OBJECT(tmp_class_creation_14__bases);
        tmp_bases_name_14 = tmp_class_creation_14__bases;
        tmp_assign_source_100 = SELECT_METACLASS(tmp_metaclass_name_14, tmp_bases_name_14);
        Py_DECREF(tmp_metaclass_name_14);
        if (tmp_assign_source_100 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        assert(tmp_class_creation_14__metaclass == NULL);
        tmp_class_creation_14__metaclass = tmp_assign_source_100;
    }
    {
        nuitka_bool tmp_condition_result_54;
        PyObject *tmp_key_name_42;
        PyObject *tmp_dict_arg_name_42;
        tmp_key_name_42 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_14__class_decl_dict);
        tmp_dict_arg_name_42 = tmp_class_creation_14__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_42, tmp_key_name_42);
        assert(!(tmp_res == -1));
        tmp_condition_result_54 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_54 == NUITKA_BOOL_TRUE) {
            goto branch_yes_40;
        } else {
            goto branch_no_40;
        }
    }
    branch_yes_40:;
    CHECK_OBJECT(tmp_class_creation_14__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_14__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 128;

        goto try_except_handler_40;
    }
    branch_no_40:;
    {
        nuitka_bool tmp_condition_result_55;
        PyObject *tmp_expression_name_67;
        CHECK_OBJECT(tmp_class_creation_14__metaclass);
        tmp_expression_name_67 = tmp_class_creation_14__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_67, mod_consts[50]);
        tmp_condition_result_55 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_55 == NUITKA_BOOL_TRUE) {
            goto branch_yes_41;
        } else {
            goto branch_no_41;
        }
    }
    branch_yes_41:;
    {
        PyObject *tmp_assign_source_101;
        PyObject *tmp_called_name_27;
        PyObject *tmp_expression_name_68;
        PyObject *tmp_args_name_27;
        PyObject *tmp_tuple_element_53;
        PyObject *tmp_kwargs_name_27;
        CHECK_OBJECT(tmp_class_creation_14__metaclass);
        tmp_expression_name_68 = tmp_class_creation_14__metaclass;
        tmp_called_name_27 = LOOKUP_ATTRIBUTE(tmp_expression_name_68, mod_consts[50]);
        if (tmp_called_name_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        tmp_tuple_element_53 = mod_consts[92];
        tmp_args_name_27 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_27, 0, tmp_tuple_element_53);
        CHECK_OBJECT(tmp_class_creation_14__bases);
        tmp_tuple_element_53 = tmp_class_creation_14__bases;
        PyTuple_SET_ITEM0(tmp_args_name_27, 1, tmp_tuple_element_53);
        CHECK_OBJECT(tmp_class_creation_14__class_decl_dict);
        tmp_kwargs_name_27 = tmp_class_creation_14__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 128;
        tmp_assign_source_101 = CALL_FUNCTION(tmp_called_name_27, tmp_args_name_27, tmp_kwargs_name_27);
        Py_DECREF(tmp_called_name_27);
        Py_DECREF(tmp_args_name_27);
        if (tmp_assign_source_101 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        assert(tmp_class_creation_14__prepared == NULL);
        tmp_class_creation_14__prepared = tmp_assign_source_101;
    }
    {
        nuitka_bool tmp_condition_result_56;
        PyObject *tmp_operand_name_14;
        PyObject *tmp_expression_name_69;
        CHECK_OBJECT(tmp_class_creation_14__prepared);
        tmp_expression_name_69 = tmp_class_creation_14__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_69, mod_consts[51]);
        tmp_operand_name_14 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_14);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        tmp_condition_result_56 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_56 == NUITKA_BOOL_TRUE) {
            goto branch_yes_42;
        } else {
            goto branch_no_42;
        }
    }
    branch_yes_42:;
    {
        PyObject *tmp_raise_type_14;
        PyObject *tmp_raise_value_14;
        PyObject *tmp_left_name_14;
        PyObject *tmp_right_name_14;
        PyObject *tmp_tuple_element_54;
        PyObject *tmp_getattr_target_14;
        PyObject *tmp_getattr_attr_14;
        PyObject *tmp_getattr_default_14;
        tmp_raise_type_14 = PyExc_TypeError;
        tmp_left_name_14 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_14__metaclass);
        tmp_getattr_target_14 = tmp_class_creation_14__metaclass;
        tmp_getattr_attr_14 = mod_consts[53];
        tmp_getattr_default_14 = mod_consts[54];
        tmp_tuple_element_54 = BUILTIN_GETATTR(tmp_getattr_target_14, tmp_getattr_attr_14, tmp_getattr_default_14);
        if (tmp_tuple_element_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        tmp_right_name_14 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_70;
            PyObject *tmp_type_arg_28;
            PyTuple_SET_ITEM(tmp_right_name_14, 0, tmp_tuple_element_54);
            CHECK_OBJECT(tmp_class_creation_14__prepared);
            tmp_type_arg_28 = tmp_class_creation_14__prepared;
            tmp_expression_name_70 = BUILTIN_TYPE1(tmp_type_arg_28);
            assert(!(tmp_expression_name_70 == NULL));
            tmp_tuple_element_54 = LOOKUP_ATTRIBUTE(tmp_expression_name_70, mod_consts[53]);
            Py_DECREF(tmp_expression_name_70);
            if (tmp_tuple_element_54 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 128;

                goto tuple_build_exception_15;
            }
            PyTuple_SET_ITEM(tmp_right_name_14, 1, tmp_tuple_element_54);
        }
        goto tuple_build_noexception_15;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_15:;
        Py_DECREF(tmp_right_name_14);
        goto try_except_handler_40;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_15:;
        tmp_raise_value_14 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_14, tmp_right_name_14);
        Py_DECREF(tmp_right_name_14);
        if (tmp_raise_value_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_40;
        }
        exception_type = tmp_raise_type_14;
        Py_INCREF(tmp_raise_type_14);
        exception_value = tmp_raise_value_14;
        exception_lineno = 128;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_40;
    }
    branch_no_42:;
    goto branch_end_41;
    branch_no_41:;
    {
        PyObject *tmp_assign_source_102;
        tmp_assign_source_102 = PyDict_New();
        assert(tmp_class_creation_14__prepared == NULL);
        tmp_class_creation_14__prepared = tmp_assign_source_102;
    }
    branch_end_41:;
    {
        PyObject *tmp_assign_source_103;
        {
            PyObject *tmp_set_locals_14;
            CHECK_OBJECT(tmp_class_creation_14__prepared);
            tmp_set_locals_14 = tmp_class_creation_14__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128 = tmp_set_locals_14;
            Py_INCREF(tmp_set_locals_14);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_42;
        }
        tmp_dictset_value = mod_consts[93];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_42;
        }
        tmp_dictset_value = mod_consts[92];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto try_except_handler_42;
        }
        {
            PyObject *tmp_assign_source_104;
            PyObject *tmp_called_name_28;
            PyObject *tmp_args_name_28;
            PyObject *tmp_tuple_element_55;
            PyObject *tmp_kwargs_name_28;
            CHECK_OBJECT(tmp_class_creation_14__metaclass);
            tmp_called_name_28 = tmp_class_creation_14__metaclass;
            tmp_tuple_element_55 = mod_consts[92];
            tmp_args_name_28 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_28, 0, tmp_tuple_element_55);
            CHECK_OBJECT(tmp_class_creation_14__bases);
            tmp_tuple_element_55 = tmp_class_creation_14__bases;
            PyTuple_SET_ITEM0(tmp_args_name_28, 1, tmp_tuple_element_55);
            tmp_tuple_element_55 = locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128;
            PyTuple_SET_ITEM0(tmp_args_name_28, 2, tmp_tuple_element_55);
            CHECK_OBJECT(tmp_class_creation_14__class_decl_dict);
            tmp_kwargs_name_28 = tmp_class_creation_14__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 128;
            tmp_assign_source_104 = CALL_FUNCTION(tmp_called_name_28, tmp_args_name_28, tmp_kwargs_name_28);
            Py_DECREF(tmp_args_name_28);
            if (tmp_assign_source_104 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 128;

                goto try_except_handler_42;
            }
            assert(outline_13_var___class__ == NULL);
            outline_13_var___class__ = tmp_assign_source_104;
        }
        CHECK_OBJECT(outline_13_var___class__);
        tmp_assign_source_103 = outline_13_var___class__;
        Py_INCREF(tmp_assign_source_103);
        goto try_return_handler_42;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_42:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128);
        locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128 = NULL;
        goto try_return_handler_41;
        // Exception handler code:
        try_except_handler_42:;
        exception_keeper_type_40 = exception_type;
        exception_keeper_value_40 = exception_value;
        exception_keeper_tb_40 = exception_tb;
        exception_keeper_lineno_40 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128);
        locals_pip$_vendor$urllib3$exceptions$$$class__14_ConnectTimeoutError_128 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_40;
        exception_value = exception_keeper_value_40;
        exception_tb = exception_keeper_tb_40;
        exception_lineno = exception_keeper_lineno_40;

        goto try_except_handler_41;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_41:;
        CHECK_OBJECT(outline_13_var___class__);
        Py_DECREF(outline_13_var___class__);
        outline_13_var___class__ = NULL;
        goto outline_result_14;
        // Exception handler code:
        try_except_handler_41:;
        exception_keeper_type_41 = exception_type;
        exception_keeper_value_41 = exception_value;
        exception_keeper_tb_41 = exception_tb;
        exception_keeper_lineno_41 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_41;
        exception_value = exception_keeper_value_41;
        exception_tb = exception_keeper_tb_41;
        exception_lineno = exception_keeper_lineno_41;

        goto outline_exception_14;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_14:;
        exception_lineno = 128;
        goto try_except_handler_40;
        outline_result_14:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[92], tmp_assign_source_103);
    }
    goto try_end_14;
    // Exception handler code:
    try_except_handler_40:;
    exception_keeper_type_42 = exception_type;
    exception_keeper_value_42 = exception_value;
    exception_keeper_tb_42 = exception_tb;
    exception_keeper_lineno_42 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_14__bases);
    tmp_class_creation_14__bases = NULL;
    Py_XDECREF(tmp_class_creation_14__class_decl_dict);
    tmp_class_creation_14__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_14__metaclass);
    tmp_class_creation_14__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_14__prepared);
    tmp_class_creation_14__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_42;
    exception_value = exception_keeper_value_42;
    exception_tb = exception_keeper_tb_42;
    exception_lineno = exception_keeper_lineno_42;

    goto frame_exception_exit_1;
    // End of try:
    try_end_14:;
    Py_XDECREF(tmp_class_creation_14__bases);
    tmp_class_creation_14__bases = NULL;
    Py_XDECREF(tmp_class_creation_14__class_decl_dict);
    tmp_class_creation_14__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_14__metaclass);
    tmp_class_creation_14__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_14__prepared);
    Py_DECREF(tmp_class_creation_14__prepared);
    tmp_class_creation_14__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_105;
        PyObject *tmp_tuple_element_56;
        tmp_tuple_element_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[92]);

        if (unlikely(tmp_tuple_element_56 == NULL)) {
            tmp_tuple_element_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[92]);
        }

        if (tmp_tuple_element_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        tmp_assign_source_105 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_assign_source_105, 0, tmp_tuple_element_56);
        tmp_tuple_element_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[6]);

        if (unlikely(tmp_tuple_element_56 == NULL)) {
            tmp_tuple_element_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[6]);
        }

        if (tmp_tuple_element_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto tuple_build_exception_16;
        }
        PyTuple_SET_ITEM0(tmp_assign_source_105, 1, tmp_tuple_element_56);
        goto tuple_build_noexception_16;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_16:;
        Py_DECREF(tmp_assign_source_105);
        goto try_except_handler_43;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_16:;
        assert(tmp_class_creation_15__bases == NULL);
        tmp_class_creation_15__bases = tmp_assign_source_105;
    }
    {
        PyObject *tmp_assign_source_106;
        tmp_assign_source_106 = PyDict_New();
        assert(tmp_class_creation_15__class_decl_dict == NULL);
        tmp_class_creation_15__class_decl_dict = tmp_assign_source_106;
    }
    {
        PyObject *tmp_assign_source_107;
        PyObject *tmp_metaclass_name_15;
        nuitka_bool tmp_condition_result_57;
        PyObject *tmp_key_name_43;
        PyObject *tmp_dict_arg_name_43;
        PyObject *tmp_dict_arg_name_44;
        PyObject *tmp_key_name_44;
        PyObject *tmp_type_arg_29;
        PyObject *tmp_expression_name_71;
        PyObject *tmp_subscript_name_15;
        PyObject *tmp_bases_name_15;
        tmp_key_name_43 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_15__class_decl_dict);
        tmp_dict_arg_name_43 = tmp_class_creation_15__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_43, tmp_key_name_43);
        assert(!(tmp_res == -1));
        tmp_condition_result_57 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_57 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_15;
        } else {
            goto condexpr_false_15;
        }
        condexpr_true_15:;
        CHECK_OBJECT(tmp_class_creation_15__class_decl_dict);
        tmp_dict_arg_name_44 = tmp_class_creation_15__class_decl_dict;
        tmp_key_name_44 = mod_consts[49];
        tmp_metaclass_name_15 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_44, tmp_key_name_44);
        if (tmp_metaclass_name_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        goto condexpr_end_15;
        condexpr_false_15:;
        CHECK_OBJECT(tmp_class_creation_15__bases);
        tmp_expression_name_71 = tmp_class_creation_15__bases;
        tmp_subscript_name_15 = mod_consts[46];
        tmp_type_arg_29 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_71, tmp_subscript_name_15, 0);
        if (tmp_type_arg_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        tmp_metaclass_name_15 = BUILTIN_TYPE1(tmp_type_arg_29);
        Py_DECREF(tmp_type_arg_29);
        if (tmp_metaclass_name_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        condexpr_end_15:;
        CHECK_OBJECT(tmp_class_creation_15__bases);
        tmp_bases_name_15 = tmp_class_creation_15__bases;
        tmp_assign_source_107 = SELECT_METACLASS(tmp_metaclass_name_15, tmp_bases_name_15);
        Py_DECREF(tmp_metaclass_name_15);
        if (tmp_assign_source_107 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        assert(tmp_class_creation_15__metaclass == NULL);
        tmp_class_creation_15__metaclass = tmp_assign_source_107;
    }
    {
        nuitka_bool tmp_condition_result_58;
        PyObject *tmp_key_name_45;
        PyObject *tmp_dict_arg_name_45;
        tmp_key_name_45 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_15__class_decl_dict);
        tmp_dict_arg_name_45 = tmp_class_creation_15__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_45, tmp_key_name_45);
        assert(!(tmp_res == -1));
        tmp_condition_result_58 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_58 == NUITKA_BOOL_TRUE) {
            goto branch_yes_43;
        } else {
            goto branch_no_43;
        }
    }
    branch_yes_43:;
    CHECK_OBJECT(tmp_class_creation_15__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_15__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 134;

        goto try_except_handler_43;
    }
    branch_no_43:;
    {
        nuitka_bool tmp_condition_result_59;
        PyObject *tmp_expression_name_72;
        CHECK_OBJECT(tmp_class_creation_15__metaclass);
        tmp_expression_name_72 = tmp_class_creation_15__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_72, mod_consts[50]);
        tmp_condition_result_59 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_59 == NUITKA_BOOL_TRUE) {
            goto branch_yes_44;
        } else {
            goto branch_no_44;
        }
    }
    branch_yes_44:;
    {
        PyObject *tmp_assign_source_108;
        PyObject *tmp_called_name_29;
        PyObject *tmp_expression_name_73;
        PyObject *tmp_args_name_29;
        PyObject *tmp_tuple_element_57;
        PyObject *tmp_kwargs_name_29;
        CHECK_OBJECT(tmp_class_creation_15__metaclass);
        tmp_expression_name_73 = tmp_class_creation_15__metaclass;
        tmp_called_name_29 = LOOKUP_ATTRIBUTE(tmp_expression_name_73, mod_consts[50]);
        if (tmp_called_name_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        tmp_tuple_element_57 = mod_consts[94];
        tmp_args_name_29 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_29, 0, tmp_tuple_element_57);
        CHECK_OBJECT(tmp_class_creation_15__bases);
        tmp_tuple_element_57 = tmp_class_creation_15__bases;
        PyTuple_SET_ITEM0(tmp_args_name_29, 1, tmp_tuple_element_57);
        CHECK_OBJECT(tmp_class_creation_15__class_decl_dict);
        tmp_kwargs_name_29 = tmp_class_creation_15__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 134;
        tmp_assign_source_108 = CALL_FUNCTION(tmp_called_name_29, tmp_args_name_29, tmp_kwargs_name_29);
        Py_DECREF(tmp_called_name_29);
        Py_DECREF(tmp_args_name_29);
        if (tmp_assign_source_108 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        assert(tmp_class_creation_15__prepared == NULL);
        tmp_class_creation_15__prepared = tmp_assign_source_108;
    }
    {
        nuitka_bool tmp_condition_result_60;
        PyObject *tmp_operand_name_15;
        PyObject *tmp_expression_name_74;
        CHECK_OBJECT(tmp_class_creation_15__prepared);
        tmp_expression_name_74 = tmp_class_creation_15__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_74, mod_consts[51]);
        tmp_operand_name_15 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_15);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        tmp_condition_result_60 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_60 == NUITKA_BOOL_TRUE) {
            goto branch_yes_45;
        } else {
            goto branch_no_45;
        }
    }
    branch_yes_45:;
    {
        PyObject *tmp_raise_type_15;
        PyObject *tmp_raise_value_15;
        PyObject *tmp_left_name_15;
        PyObject *tmp_right_name_15;
        PyObject *tmp_tuple_element_58;
        PyObject *tmp_getattr_target_15;
        PyObject *tmp_getattr_attr_15;
        PyObject *tmp_getattr_default_15;
        tmp_raise_type_15 = PyExc_TypeError;
        tmp_left_name_15 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_15__metaclass);
        tmp_getattr_target_15 = tmp_class_creation_15__metaclass;
        tmp_getattr_attr_15 = mod_consts[53];
        tmp_getattr_default_15 = mod_consts[54];
        tmp_tuple_element_58 = BUILTIN_GETATTR(tmp_getattr_target_15, tmp_getattr_attr_15, tmp_getattr_default_15);
        if (tmp_tuple_element_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        tmp_right_name_15 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_75;
            PyObject *tmp_type_arg_30;
            PyTuple_SET_ITEM(tmp_right_name_15, 0, tmp_tuple_element_58);
            CHECK_OBJECT(tmp_class_creation_15__prepared);
            tmp_type_arg_30 = tmp_class_creation_15__prepared;
            tmp_expression_name_75 = BUILTIN_TYPE1(tmp_type_arg_30);
            assert(!(tmp_expression_name_75 == NULL));
            tmp_tuple_element_58 = LOOKUP_ATTRIBUTE(tmp_expression_name_75, mod_consts[53]);
            Py_DECREF(tmp_expression_name_75);
            if (tmp_tuple_element_58 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 134;

                goto tuple_build_exception_17;
            }
            PyTuple_SET_ITEM(tmp_right_name_15, 1, tmp_tuple_element_58);
        }
        goto tuple_build_noexception_17;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_17:;
        Py_DECREF(tmp_right_name_15);
        goto try_except_handler_43;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_17:;
        tmp_raise_value_15 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_15, tmp_right_name_15);
        Py_DECREF(tmp_right_name_15);
        if (tmp_raise_value_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_43;
        }
        exception_type = tmp_raise_type_15;
        Py_INCREF(tmp_raise_type_15);
        exception_value = tmp_raise_value_15;
        exception_lineno = 134;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_43;
    }
    branch_no_45:;
    goto branch_end_44;
    branch_no_44:;
    {
        PyObject *tmp_assign_source_109;
        tmp_assign_source_109 = PyDict_New();
        assert(tmp_class_creation_15__prepared == NULL);
        tmp_class_creation_15__prepared = tmp_assign_source_109;
    }
    branch_end_44:;
    {
        PyObject *tmp_assign_source_110;
        {
            PyObject *tmp_set_locals_15;
            CHECK_OBJECT(tmp_class_creation_15__prepared);
            tmp_set_locals_15 = tmp_class_creation_15__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134 = tmp_set_locals_15;
            Py_INCREF(tmp_set_locals_15);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_45;
        }
        tmp_dictset_value = mod_consts[95];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_45;
        }
        tmp_dictset_value = mod_consts[94];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 134;

            goto try_except_handler_45;
        }
        {
            PyObject *tmp_assign_source_111;
            PyObject *tmp_called_name_30;
            PyObject *tmp_args_name_30;
            PyObject *tmp_tuple_element_59;
            PyObject *tmp_kwargs_name_30;
            CHECK_OBJECT(tmp_class_creation_15__metaclass);
            tmp_called_name_30 = tmp_class_creation_15__metaclass;
            tmp_tuple_element_59 = mod_consts[94];
            tmp_args_name_30 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_30, 0, tmp_tuple_element_59);
            CHECK_OBJECT(tmp_class_creation_15__bases);
            tmp_tuple_element_59 = tmp_class_creation_15__bases;
            PyTuple_SET_ITEM0(tmp_args_name_30, 1, tmp_tuple_element_59);
            tmp_tuple_element_59 = locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134;
            PyTuple_SET_ITEM0(tmp_args_name_30, 2, tmp_tuple_element_59);
            CHECK_OBJECT(tmp_class_creation_15__class_decl_dict);
            tmp_kwargs_name_30 = tmp_class_creation_15__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 134;
            tmp_assign_source_111 = CALL_FUNCTION(tmp_called_name_30, tmp_args_name_30, tmp_kwargs_name_30);
            Py_DECREF(tmp_args_name_30);
            if (tmp_assign_source_111 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 134;

                goto try_except_handler_45;
            }
            assert(outline_14_var___class__ == NULL);
            outline_14_var___class__ = tmp_assign_source_111;
        }
        CHECK_OBJECT(outline_14_var___class__);
        tmp_assign_source_110 = outline_14_var___class__;
        Py_INCREF(tmp_assign_source_110);
        goto try_return_handler_45;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_45:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134);
        locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134 = NULL;
        goto try_return_handler_44;
        // Exception handler code:
        try_except_handler_45:;
        exception_keeper_type_43 = exception_type;
        exception_keeper_value_43 = exception_value;
        exception_keeper_tb_43 = exception_tb;
        exception_keeper_lineno_43 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134);
        locals_pip$_vendor$urllib3$exceptions$$$class__15_NewConnectionError_134 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_43;
        exception_value = exception_keeper_value_43;
        exception_tb = exception_keeper_tb_43;
        exception_lineno = exception_keeper_lineno_43;

        goto try_except_handler_44;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_44:;
        CHECK_OBJECT(outline_14_var___class__);
        Py_DECREF(outline_14_var___class__);
        outline_14_var___class__ = NULL;
        goto outline_result_15;
        // Exception handler code:
        try_except_handler_44:;
        exception_keeper_type_44 = exception_type;
        exception_keeper_value_44 = exception_value;
        exception_keeper_tb_44 = exception_tb;
        exception_keeper_lineno_44 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_44;
        exception_value = exception_keeper_value_44;
        exception_tb = exception_keeper_tb_44;
        exception_lineno = exception_keeper_lineno_44;

        goto outline_exception_15;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_15:;
        exception_lineno = 134;
        goto try_except_handler_43;
        outline_result_15:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[94], tmp_assign_source_110);
    }
    goto try_end_15;
    // Exception handler code:
    try_except_handler_43:;
    exception_keeper_type_45 = exception_type;
    exception_keeper_value_45 = exception_value;
    exception_keeper_tb_45 = exception_tb;
    exception_keeper_lineno_45 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_15__bases);
    tmp_class_creation_15__bases = NULL;
    Py_XDECREF(tmp_class_creation_15__class_decl_dict);
    tmp_class_creation_15__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_15__metaclass);
    tmp_class_creation_15__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_15__prepared);
    tmp_class_creation_15__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_45;
    exception_value = exception_keeper_value_45;
    exception_tb = exception_keeper_tb_45;
    exception_lineno = exception_keeper_lineno_45;

    goto frame_exception_exit_1;
    // End of try:
    try_end_15:;
    Py_XDECREF(tmp_class_creation_15__bases);
    tmp_class_creation_15__bases = NULL;
    Py_XDECREF(tmp_class_creation_15__class_decl_dict);
    tmp_class_creation_15__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_15__metaclass);
    tmp_class_creation_15__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_15__prepared);
    Py_DECREF(tmp_class_creation_15__prepared);
    tmp_class_creation_15__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_112;
        PyObject *tmp_tuple_element_60;
        tmp_tuple_element_60 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[6]);

        if (unlikely(tmp_tuple_element_60 == NULL)) {
            tmp_tuple_element_60 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[6]);
        }

        if (tmp_tuple_element_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        tmp_assign_source_112 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_112, 0, tmp_tuple_element_60);
        assert(tmp_class_creation_16__bases == NULL);
        tmp_class_creation_16__bases = tmp_assign_source_112;
    }
    {
        PyObject *tmp_assign_source_113;
        tmp_assign_source_113 = PyDict_New();
        assert(tmp_class_creation_16__class_decl_dict == NULL);
        tmp_class_creation_16__class_decl_dict = tmp_assign_source_113;
    }
    {
        PyObject *tmp_assign_source_114;
        PyObject *tmp_metaclass_name_16;
        nuitka_bool tmp_condition_result_61;
        PyObject *tmp_key_name_46;
        PyObject *tmp_dict_arg_name_46;
        PyObject *tmp_dict_arg_name_47;
        PyObject *tmp_key_name_47;
        PyObject *tmp_type_arg_31;
        PyObject *tmp_expression_name_76;
        PyObject *tmp_subscript_name_16;
        PyObject *tmp_bases_name_16;
        tmp_key_name_46 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_16__class_decl_dict);
        tmp_dict_arg_name_46 = tmp_class_creation_16__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_46, tmp_key_name_46);
        assert(!(tmp_res == -1));
        tmp_condition_result_61 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_61 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_16;
        } else {
            goto condexpr_false_16;
        }
        condexpr_true_16:;
        CHECK_OBJECT(tmp_class_creation_16__class_decl_dict);
        tmp_dict_arg_name_47 = tmp_class_creation_16__class_decl_dict;
        tmp_key_name_47 = mod_consts[49];
        tmp_metaclass_name_16 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_47, tmp_key_name_47);
        if (tmp_metaclass_name_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        goto condexpr_end_16;
        condexpr_false_16:;
        CHECK_OBJECT(tmp_class_creation_16__bases);
        tmp_expression_name_76 = tmp_class_creation_16__bases;
        tmp_subscript_name_16 = mod_consts[46];
        tmp_type_arg_31 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_76, tmp_subscript_name_16, 0);
        if (tmp_type_arg_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        tmp_metaclass_name_16 = BUILTIN_TYPE1(tmp_type_arg_31);
        Py_DECREF(tmp_type_arg_31);
        if (tmp_metaclass_name_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        condexpr_end_16:;
        CHECK_OBJECT(tmp_class_creation_16__bases);
        tmp_bases_name_16 = tmp_class_creation_16__bases;
        tmp_assign_source_114 = SELECT_METACLASS(tmp_metaclass_name_16, tmp_bases_name_16);
        Py_DECREF(tmp_metaclass_name_16);
        if (tmp_assign_source_114 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        assert(tmp_class_creation_16__metaclass == NULL);
        tmp_class_creation_16__metaclass = tmp_assign_source_114;
    }
    {
        nuitka_bool tmp_condition_result_62;
        PyObject *tmp_key_name_48;
        PyObject *tmp_dict_arg_name_48;
        tmp_key_name_48 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_16__class_decl_dict);
        tmp_dict_arg_name_48 = tmp_class_creation_16__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_48, tmp_key_name_48);
        assert(!(tmp_res == -1));
        tmp_condition_result_62 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_62 == NUITKA_BOOL_TRUE) {
            goto branch_yes_46;
        } else {
            goto branch_no_46;
        }
    }
    branch_yes_46:;
    CHECK_OBJECT(tmp_class_creation_16__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_16__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 140;

        goto try_except_handler_46;
    }
    branch_no_46:;
    {
        nuitka_bool tmp_condition_result_63;
        PyObject *tmp_expression_name_77;
        CHECK_OBJECT(tmp_class_creation_16__metaclass);
        tmp_expression_name_77 = tmp_class_creation_16__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_77, mod_consts[50]);
        tmp_condition_result_63 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_63 == NUITKA_BOOL_TRUE) {
            goto branch_yes_47;
        } else {
            goto branch_no_47;
        }
    }
    branch_yes_47:;
    {
        PyObject *tmp_assign_source_115;
        PyObject *tmp_called_name_31;
        PyObject *tmp_expression_name_78;
        PyObject *tmp_args_name_31;
        PyObject *tmp_tuple_element_61;
        PyObject *tmp_kwargs_name_31;
        CHECK_OBJECT(tmp_class_creation_16__metaclass);
        tmp_expression_name_78 = tmp_class_creation_16__metaclass;
        tmp_called_name_31 = LOOKUP_ATTRIBUTE(tmp_expression_name_78, mod_consts[50]);
        if (tmp_called_name_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        tmp_tuple_element_61 = mod_consts[96];
        tmp_args_name_31 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_31, 0, tmp_tuple_element_61);
        CHECK_OBJECT(tmp_class_creation_16__bases);
        tmp_tuple_element_61 = tmp_class_creation_16__bases;
        PyTuple_SET_ITEM0(tmp_args_name_31, 1, tmp_tuple_element_61);
        CHECK_OBJECT(tmp_class_creation_16__class_decl_dict);
        tmp_kwargs_name_31 = tmp_class_creation_16__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 140;
        tmp_assign_source_115 = CALL_FUNCTION(tmp_called_name_31, tmp_args_name_31, tmp_kwargs_name_31);
        Py_DECREF(tmp_called_name_31);
        Py_DECREF(tmp_args_name_31);
        if (tmp_assign_source_115 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        assert(tmp_class_creation_16__prepared == NULL);
        tmp_class_creation_16__prepared = tmp_assign_source_115;
    }
    {
        nuitka_bool tmp_condition_result_64;
        PyObject *tmp_operand_name_16;
        PyObject *tmp_expression_name_79;
        CHECK_OBJECT(tmp_class_creation_16__prepared);
        tmp_expression_name_79 = tmp_class_creation_16__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_79, mod_consts[51]);
        tmp_operand_name_16 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_16);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        tmp_condition_result_64 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_64 == NUITKA_BOOL_TRUE) {
            goto branch_yes_48;
        } else {
            goto branch_no_48;
        }
    }
    branch_yes_48:;
    {
        PyObject *tmp_raise_type_16;
        PyObject *tmp_raise_value_16;
        PyObject *tmp_left_name_16;
        PyObject *tmp_right_name_16;
        PyObject *tmp_tuple_element_62;
        PyObject *tmp_getattr_target_16;
        PyObject *tmp_getattr_attr_16;
        PyObject *tmp_getattr_default_16;
        tmp_raise_type_16 = PyExc_TypeError;
        tmp_left_name_16 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_16__metaclass);
        tmp_getattr_target_16 = tmp_class_creation_16__metaclass;
        tmp_getattr_attr_16 = mod_consts[53];
        tmp_getattr_default_16 = mod_consts[54];
        tmp_tuple_element_62 = BUILTIN_GETATTR(tmp_getattr_target_16, tmp_getattr_attr_16, tmp_getattr_default_16);
        if (tmp_tuple_element_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        tmp_right_name_16 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_80;
            PyObject *tmp_type_arg_32;
            PyTuple_SET_ITEM(tmp_right_name_16, 0, tmp_tuple_element_62);
            CHECK_OBJECT(tmp_class_creation_16__prepared);
            tmp_type_arg_32 = tmp_class_creation_16__prepared;
            tmp_expression_name_80 = BUILTIN_TYPE1(tmp_type_arg_32);
            assert(!(tmp_expression_name_80 == NULL));
            tmp_tuple_element_62 = LOOKUP_ATTRIBUTE(tmp_expression_name_80, mod_consts[53]);
            Py_DECREF(tmp_expression_name_80);
            if (tmp_tuple_element_62 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;

                goto tuple_build_exception_18;
            }
            PyTuple_SET_ITEM(tmp_right_name_16, 1, tmp_tuple_element_62);
        }
        goto tuple_build_noexception_18;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_18:;
        Py_DECREF(tmp_right_name_16);
        goto try_except_handler_46;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_18:;
        tmp_raise_value_16 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_16, tmp_right_name_16);
        Py_DECREF(tmp_right_name_16);
        if (tmp_raise_value_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_46;
        }
        exception_type = tmp_raise_type_16;
        Py_INCREF(tmp_raise_type_16);
        exception_value = tmp_raise_value_16;
        exception_lineno = 140;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_46;
    }
    branch_no_48:;
    goto branch_end_47;
    branch_no_47:;
    {
        PyObject *tmp_assign_source_116;
        tmp_assign_source_116 = PyDict_New();
        assert(tmp_class_creation_16__prepared == NULL);
        tmp_class_creation_16__prepared = tmp_assign_source_116;
    }
    branch_end_47:;
    {
        PyObject *tmp_assign_source_117;
        {
            PyObject *tmp_set_locals_16;
            CHECK_OBJECT(tmp_class_creation_16__prepared);
            tmp_set_locals_16 = tmp_class_creation_16__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140 = tmp_set_locals_16;
            Py_INCREF(tmp_set_locals_16);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_48;
        }
        tmp_dictset_value = mod_consts[97];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_48;
        }
        tmp_dictset_value = mod_consts[96];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 140;

            goto try_except_handler_48;
        }
        {
            PyObject *tmp_assign_source_118;
            PyObject *tmp_called_name_32;
            PyObject *tmp_args_name_32;
            PyObject *tmp_tuple_element_63;
            PyObject *tmp_kwargs_name_32;
            CHECK_OBJECT(tmp_class_creation_16__metaclass);
            tmp_called_name_32 = tmp_class_creation_16__metaclass;
            tmp_tuple_element_63 = mod_consts[96];
            tmp_args_name_32 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_32, 0, tmp_tuple_element_63);
            CHECK_OBJECT(tmp_class_creation_16__bases);
            tmp_tuple_element_63 = tmp_class_creation_16__bases;
            PyTuple_SET_ITEM0(tmp_args_name_32, 1, tmp_tuple_element_63);
            tmp_tuple_element_63 = locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140;
            PyTuple_SET_ITEM0(tmp_args_name_32, 2, tmp_tuple_element_63);
            CHECK_OBJECT(tmp_class_creation_16__class_decl_dict);
            tmp_kwargs_name_32 = tmp_class_creation_16__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 140;
            tmp_assign_source_118 = CALL_FUNCTION(tmp_called_name_32, tmp_args_name_32, tmp_kwargs_name_32);
            Py_DECREF(tmp_args_name_32);
            if (tmp_assign_source_118 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;

                goto try_except_handler_48;
            }
            assert(outline_15_var___class__ == NULL);
            outline_15_var___class__ = tmp_assign_source_118;
        }
        CHECK_OBJECT(outline_15_var___class__);
        tmp_assign_source_117 = outline_15_var___class__;
        Py_INCREF(tmp_assign_source_117);
        goto try_return_handler_48;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_48:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140);
        locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140 = NULL;
        goto try_return_handler_47;
        // Exception handler code:
        try_except_handler_48:;
        exception_keeper_type_46 = exception_type;
        exception_keeper_value_46 = exception_value;
        exception_keeper_tb_46 = exception_tb;
        exception_keeper_lineno_46 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140);
        locals_pip$_vendor$urllib3$exceptions$$$class__16_EmptyPoolError_140 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_46;
        exception_value = exception_keeper_value_46;
        exception_tb = exception_keeper_tb_46;
        exception_lineno = exception_keeper_lineno_46;

        goto try_except_handler_47;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_47:;
        CHECK_OBJECT(outline_15_var___class__);
        Py_DECREF(outline_15_var___class__);
        outline_15_var___class__ = NULL;
        goto outline_result_16;
        // Exception handler code:
        try_except_handler_47:;
        exception_keeper_type_47 = exception_type;
        exception_keeper_value_47 = exception_value;
        exception_keeper_tb_47 = exception_tb;
        exception_keeper_lineno_47 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_47;
        exception_value = exception_keeper_value_47;
        exception_tb = exception_keeper_tb_47;
        exception_lineno = exception_keeper_lineno_47;

        goto outline_exception_16;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_16:;
        exception_lineno = 140;
        goto try_except_handler_46;
        outline_result_16:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[96], tmp_assign_source_117);
    }
    goto try_end_16;
    // Exception handler code:
    try_except_handler_46:;
    exception_keeper_type_48 = exception_type;
    exception_keeper_value_48 = exception_value;
    exception_keeper_tb_48 = exception_tb;
    exception_keeper_lineno_48 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_16__bases);
    tmp_class_creation_16__bases = NULL;
    Py_XDECREF(tmp_class_creation_16__class_decl_dict);
    tmp_class_creation_16__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_16__metaclass);
    tmp_class_creation_16__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_16__prepared);
    tmp_class_creation_16__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_48;
    exception_value = exception_keeper_value_48;
    exception_tb = exception_keeper_tb_48;
    exception_lineno = exception_keeper_lineno_48;

    goto frame_exception_exit_1;
    // End of try:
    try_end_16:;
    Py_XDECREF(tmp_class_creation_16__bases);
    tmp_class_creation_16__bases = NULL;
    Py_XDECREF(tmp_class_creation_16__class_decl_dict);
    tmp_class_creation_16__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_16__metaclass);
    tmp_class_creation_16__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_16__prepared);
    Py_DECREF(tmp_class_creation_16__prepared);
    tmp_class_creation_16__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_119;
        PyObject *tmp_tuple_element_64;
        tmp_tuple_element_64 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[6]);

        if (unlikely(tmp_tuple_element_64 == NULL)) {
            tmp_tuple_element_64 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[6]);
        }

        if (tmp_tuple_element_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        tmp_assign_source_119 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_119, 0, tmp_tuple_element_64);
        assert(tmp_class_creation_17__bases == NULL);
        tmp_class_creation_17__bases = tmp_assign_source_119;
    }
    {
        PyObject *tmp_assign_source_120;
        tmp_assign_source_120 = PyDict_New();
        assert(tmp_class_creation_17__class_decl_dict == NULL);
        tmp_class_creation_17__class_decl_dict = tmp_assign_source_120;
    }
    {
        PyObject *tmp_assign_source_121;
        PyObject *tmp_metaclass_name_17;
        nuitka_bool tmp_condition_result_65;
        PyObject *tmp_key_name_49;
        PyObject *tmp_dict_arg_name_49;
        PyObject *tmp_dict_arg_name_50;
        PyObject *tmp_key_name_50;
        PyObject *tmp_type_arg_33;
        PyObject *tmp_expression_name_81;
        PyObject *tmp_subscript_name_17;
        PyObject *tmp_bases_name_17;
        tmp_key_name_49 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_17__class_decl_dict);
        tmp_dict_arg_name_49 = tmp_class_creation_17__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_49, tmp_key_name_49);
        assert(!(tmp_res == -1));
        tmp_condition_result_65 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_65 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_17;
        } else {
            goto condexpr_false_17;
        }
        condexpr_true_17:;
        CHECK_OBJECT(tmp_class_creation_17__class_decl_dict);
        tmp_dict_arg_name_50 = tmp_class_creation_17__class_decl_dict;
        tmp_key_name_50 = mod_consts[49];
        tmp_metaclass_name_17 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_50, tmp_key_name_50);
        if (tmp_metaclass_name_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        goto condexpr_end_17;
        condexpr_false_17:;
        CHECK_OBJECT(tmp_class_creation_17__bases);
        tmp_expression_name_81 = tmp_class_creation_17__bases;
        tmp_subscript_name_17 = mod_consts[46];
        tmp_type_arg_33 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_81, tmp_subscript_name_17, 0);
        if (tmp_type_arg_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        tmp_metaclass_name_17 = BUILTIN_TYPE1(tmp_type_arg_33);
        Py_DECREF(tmp_type_arg_33);
        if (tmp_metaclass_name_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        condexpr_end_17:;
        CHECK_OBJECT(tmp_class_creation_17__bases);
        tmp_bases_name_17 = tmp_class_creation_17__bases;
        tmp_assign_source_121 = SELECT_METACLASS(tmp_metaclass_name_17, tmp_bases_name_17);
        Py_DECREF(tmp_metaclass_name_17);
        if (tmp_assign_source_121 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        assert(tmp_class_creation_17__metaclass == NULL);
        tmp_class_creation_17__metaclass = tmp_assign_source_121;
    }
    {
        nuitka_bool tmp_condition_result_66;
        PyObject *tmp_key_name_51;
        PyObject *tmp_dict_arg_name_51;
        tmp_key_name_51 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_17__class_decl_dict);
        tmp_dict_arg_name_51 = tmp_class_creation_17__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_51, tmp_key_name_51);
        assert(!(tmp_res == -1));
        tmp_condition_result_66 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_66 == NUITKA_BOOL_TRUE) {
            goto branch_yes_49;
        } else {
            goto branch_no_49;
        }
    }
    branch_yes_49:;
    CHECK_OBJECT(tmp_class_creation_17__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_17__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 146;

        goto try_except_handler_49;
    }
    branch_no_49:;
    {
        nuitka_bool tmp_condition_result_67;
        PyObject *tmp_expression_name_82;
        CHECK_OBJECT(tmp_class_creation_17__metaclass);
        tmp_expression_name_82 = tmp_class_creation_17__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_82, mod_consts[50]);
        tmp_condition_result_67 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_67 == NUITKA_BOOL_TRUE) {
            goto branch_yes_50;
        } else {
            goto branch_no_50;
        }
    }
    branch_yes_50:;
    {
        PyObject *tmp_assign_source_122;
        PyObject *tmp_called_name_33;
        PyObject *tmp_expression_name_83;
        PyObject *tmp_args_name_33;
        PyObject *tmp_tuple_element_65;
        PyObject *tmp_kwargs_name_33;
        CHECK_OBJECT(tmp_class_creation_17__metaclass);
        tmp_expression_name_83 = tmp_class_creation_17__metaclass;
        tmp_called_name_33 = LOOKUP_ATTRIBUTE(tmp_expression_name_83, mod_consts[50]);
        if (tmp_called_name_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        tmp_tuple_element_65 = mod_consts[98];
        tmp_args_name_33 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_33, 0, tmp_tuple_element_65);
        CHECK_OBJECT(tmp_class_creation_17__bases);
        tmp_tuple_element_65 = tmp_class_creation_17__bases;
        PyTuple_SET_ITEM0(tmp_args_name_33, 1, tmp_tuple_element_65);
        CHECK_OBJECT(tmp_class_creation_17__class_decl_dict);
        tmp_kwargs_name_33 = tmp_class_creation_17__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 146;
        tmp_assign_source_122 = CALL_FUNCTION(tmp_called_name_33, tmp_args_name_33, tmp_kwargs_name_33);
        Py_DECREF(tmp_called_name_33);
        Py_DECREF(tmp_args_name_33);
        if (tmp_assign_source_122 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        assert(tmp_class_creation_17__prepared == NULL);
        tmp_class_creation_17__prepared = tmp_assign_source_122;
    }
    {
        nuitka_bool tmp_condition_result_68;
        PyObject *tmp_operand_name_17;
        PyObject *tmp_expression_name_84;
        CHECK_OBJECT(tmp_class_creation_17__prepared);
        tmp_expression_name_84 = tmp_class_creation_17__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_84, mod_consts[51]);
        tmp_operand_name_17 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_17);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        tmp_condition_result_68 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_68 == NUITKA_BOOL_TRUE) {
            goto branch_yes_51;
        } else {
            goto branch_no_51;
        }
    }
    branch_yes_51:;
    {
        PyObject *tmp_raise_type_17;
        PyObject *tmp_raise_value_17;
        PyObject *tmp_left_name_17;
        PyObject *tmp_right_name_17;
        PyObject *tmp_tuple_element_66;
        PyObject *tmp_getattr_target_17;
        PyObject *tmp_getattr_attr_17;
        PyObject *tmp_getattr_default_17;
        tmp_raise_type_17 = PyExc_TypeError;
        tmp_left_name_17 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_17__metaclass);
        tmp_getattr_target_17 = tmp_class_creation_17__metaclass;
        tmp_getattr_attr_17 = mod_consts[53];
        tmp_getattr_default_17 = mod_consts[54];
        tmp_tuple_element_66 = BUILTIN_GETATTR(tmp_getattr_target_17, tmp_getattr_attr_17, tmp_getattr_default_17);
        if (tmp_tuple_element_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        tmp_right_name_17 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_85;
            PyObject *tmp_type_arg_34;
            PyTuple_SET_ITEM(tmp_right_name_17, 0, tmp_tuple_element_66);
            CHECK_OBJECT(tmp_class_creation_17__prepared);
            tmp_type_arg_34 = tmp_class_creation_17__prepared;
            tmp_expression_name_85 = BUILTIN_TYPE1(tmp_type_arg_34);
            assert(!(tmp_expression_name_85 == NULL));
            tmp_tuple_element_66 = LOOKUP_ATTRIBUTE(tmp_expression_name_85, mod_consts[53]);
            Py_DECREF(tmp_expression_name_85);
            if (tmp_tuple_element_66 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 146;

                goto tuple_build_exception_19;
            }
            PyTuple_SET_ITEM(tmp_right_name_17, 1, tmp_tuple_element_66);
        }
        goto tuple_build_noexception_19;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_19:;
        Py_DECREF(tmp_right_name_17);
        goto try_except_handler_49;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_19:;
        tmp_raise_value_17 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_17, tmp_right_name_17);
        Py_DECREF(tmp_right_name_17);
        if (tmp_raise_value_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_49;
        }
        exception_type = tmp_raise_type_17;
        Py_INCREF(tmp_raise_type_17);
        exception_value = tmp_raise_value_17;
        exception_lineno = 146;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_49;
    }
    branch_no_51:;
    goto branch_end_50;
    branch_no_50:;
    {
        PyObject *tmp_assign_source_123;
        tmp_assign_source_123 = PyDict_New();
        assert(tmp_class_creation_17__prepared == NULL);
        tmp_class_creation_17__prepared = tmp_assign_source_123;
    }
    branch_end_50:;
    {
        PyObject *tmp_assign_source_124;
        {
            PyObject *tmp_set_locals_17;
            CHECK_OBJECT(tmp_class_creation_17__prepared);
            tmp_set_locals_17 = tmp_class_creation_17__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146 = tmp_set_locals_17;
            Py_INCREF(tmp_set_locals_17);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_51;
        }
        tmp_dictset_value = mod_consts[99];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_51;
        }
        tmp_dictset_value = mod_consts[98];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 146;

            goto try_except_handler_51;
        }
        {
            PyObject *tmp_assign_source_125;
            PyObject *tmp_called_name_34;
            PyObject *tmp_args_name_34;
            PyObject *tmp_tuple_element_67;
            PyObject *tmp_kwargs_name_34;
            CHECK_OBJECT(tmp_class_creation_17__metaclass);
            tmp_called_name_34 = tmp_class_creation_17__metaclass;
            tmp_tuple_element_67 = mod_consts[98];
            tmp_args_name_34 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_34, 0, tmp_tuple_element_67);
            CHECK_OBJECT(tmp_class_creation_17__bases);
            tmp_tuple_element_67 = tmp_class_creation_17__bases;
            PyTuple_SET_ITEM0(tmp_args_name_34, 1, tmp_tuple_element_67);
            tmp_tuple_element_67 = locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146;
            PyTuple_SET_ITEM0(tmp_args_name_34, 2, tmp_tuple_element_67);
            CHECK_OBJECT(tmp_class_creation_17__class_decl_dict);
            tmp_kwargs_name_34 = tmp_class_creation_17__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 146;
            tmp_assign_source_125 = CALL_FUNCTION(tmp_called_name_34, tmp_args_name_34, tmp_kwargs_name_34);
            Py_DECREF(tmp_args_name_34);
            if (tmp_assign_source_125 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 146;

                goto try_except_handler_51;
            }
            assert(outline_16_var___class__ == NULL);
            outline_16_var___class__ = tmp_assign_source_125;
        }
        CHECK_OBJECT(outline_16_var___class__);
        tmp_assign_source_124 = outline_16_var___class__;
        Py_INCREF(tmp_assign_source_124);
        goto try_return_handler_51;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_51:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146);
        locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146 = NULL;
        goto try_return_handler_50;
        // Exception handler code:
        try_except_handler_51:;
        exception_keeper_type_49 = exception_type;
        exception_keeper_value_49 = exception_value;
        exception_keeper_tb_49 = exception_tb;
        exception_keeper_lineno_49 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146);
        locals_pip$_vendor$urllib3$exceptions$$$class__17_ClosedPoolError_146 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_49;
        exception_value = exception_keeper_value_49;
        exception_tb = exception_keeper_tb_49;
        exception_lineno = exception_keeper_lineno_49;

        goto try_except_handler_50;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_50:;
        CHECK_OBJECT(outline_16_var___class__);
        Py_DECREF(outline_16_var___class__);
        outline_16_var___class__ = NULL;
        goto outline_result_17;
        // Exception handler code:
        try_except_handler_50:;
        exception_keeper_type_50 = exception_type;
        exception_keeper_value_50 = exception_value;
        exception_keeper_tb_50 = exception_tb;
        exception_keeper_lineno_50 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_50;
        exception_value = exception_keeper_value_50;
        exception_tb = exception_keeper_tb_50;
        exception_lineno = exception_keeper_lineno_50;

        goto outline_exception_17;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_17:;
        exception_lineno = 146;
        goto try_except_handler_49;
        outline_result_17:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[98], tmp_assign_source_124);
    }
    goto try_end_17;
    // Exception handler code:
    try_except_handler_49:;
    exception_keeper_type_51 = exception_type;
    exception_keeper_value_51 = exception_value;
    exception_keeper_tb_51 = exception_tb;
    exception_keeper_lineno_51 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_17__bases);
    tmp_class_creation_17__bases = NULL;
    Py_XDECREF(tmp_class_creation_17__class_decl_dict);
    tmp_class_creation_17__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_17__metaclass);
    tmp_class_creation_17__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_17__prepared);
    tmp_class_creation_17__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_51;
    exception_value = exception_keeper_value_51;
    exception_tb = exception_keeper_tb_51;
    exception_lineno = exception_keeper_lineno_51;

    goto frame_exception_exit_1;
    // End of try:
    try_end_17:;
    Py_XDECREF(tmp_class_creation_17__bases);
    tmp_class_creation_17__bases = NULL;
    Py_XDECREF(tmp_class_creation_17__class_decl_dict);
    tmp_class_creation_17__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_17__metaclass);
    tmp_class_creation_17__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_17__prepared);
    Py_DECREF(tmp_class_creation_17__prepared);
    tmp_class_creation_17__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_126;
        PyObject *tmp_tuple_element_68;
        tmp_tuple_element_68 = PyExc_ValueError;
        tmp_assign_source_126 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_assign_source_126, 0, tmp_tuple_element_68);
        tmp_tuple_element_68 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_68 == NULL)) {
            tmp_tuple_element_68 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_68 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto tuple_build_exception_20;
        }
        PyTuple_SET_ITEM0(tmp_assign_source_126, 1, tmp_tuple_element_68);
        goto tuple_build_noexception_20;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_20:;
        Py_DECREF(tmp_assign_source_126);
        goto try_except_handler_52;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_20:;
        assert(tmp_class_creation_18__bases == NULL);
        tmp_class_creation_18__bases = tmp_assign_source_126;
    }
    {
        PyObject *tmp_assign_source_127;
        tmp_assign_source_127 = PyDict_New();
        assert(tmp_class_creation_18__class_decl_dict == NULL);
        tmp_class_creation_18__class_decl_dict = tmp_assign_source_127;
    }
    {
        PyObject *tmp_assign_source_128;
        PyObject *tmp_metaclass_name_18;
        nuitka_bool tmp_condition_result_69;
        PyObject *tmp_key_name_52;
        PyObject *tmp_dict_arg_name_52;
        PyObject *tmp_dict_arg_name_53;
        PyObject *tmp_key_name_53;
        PyObject *tmp_type_arg_35;
        PyObject *tmp_expression_name_86;
        PyObject *tmp_subscript_name_18;
        PyObject *tmp_bases_name_18;
        tmp_key_name_52 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_18__class_decl_dict);
        tmp_dict_arg_name_52 = tmp_class_creation_18__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_52, tmp_key_name_52);
        assert(!(tmp_res == -1));
        tmp_condition_result_69 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_69 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_18;
        } else {
            goto condexpr_false_18;
        }
        condexpr_true_18:;
        CHECK_OBJECT(tmp_class_creation_18__class_decl_dict);
        tmp_dict_arg_name_53 = tmp_class_creation_18__class_decl_dict;
        tmp_key_name_53 = mod_consts[49];
        tmp_metaclass_name_18 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_53, tmp_key_name_53);
        if (tmp_metaclass_name_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        goto condexpr_end_18;
        condexpr_false_18:;
        CHECK_OBJECT(tmp_class_creation_18__bases);
        tmp_expression_name_86 = tmp_class_creation_18__bases;
        tmp_subscript_name_18 = mod_consts[46];
        tmp_type_arg_35 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_86, tmp_subscript_name_18, 0);
        if (tmp_type_arg_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        tmp_metaclass_name_18 = BUILTIN_TYPE1(tmp_type_arg_35);
        Py_DECREF(tmp_type_arg_35);
        if (tmp_metaclass_name_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        condexpr_end_18:;
        CHECK_OBJECT(tmp_class_creation_18__bases);
        tmp_bases_name_18 = tmp_class_creation_18__bases;
        tmp_assign_source_128 = SELECT_METACLASS(tmp_metaclass_name_18, tmp_bases_name_18);
        Py_DECREF(tmp_metaclass_name_18);
        if (tmp_assign_source_128 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        assert(tmp_class_creation_18__metaclass == NULL);
        tmp_class_creation_18__metaclass = tmp_assign_source_128;
    }
    {
        nuitka_bool tmp_condition_result_70;
        PyObject *tmp_key_name_54;
        PyObject *tmp_dict_arg_name_54;
        tmp_key_name_54 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_18__class_decl_dict);
        tmp_dict_arg_name_54 = tmp_class_creation_18__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_54, tmp_key_name_54);
        assert(!(tmp_res == -1));
        tmp_condition_result_70 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_70 == NUITKA_BOOL_TRUE) {
            goto branch_yes_52;
        } else {
            goto branch_no_52;
        }
    }
    branch_yes_52:;
    CHECK_OBJECT(tmp_class_creation_18__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_18__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 152;

        goto try_except_handler_52;
    }
    branch_no_52:;
    {
        nuitka_bool tmp_condition_result_71;
        PyObject *tmp_expression_name_87;
        CHECK_OBJECT(tmp_class_creation_18__metaclass);
        tmp_expression_name_87 = tmp_class_creation_18__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_87, mod_consts[50]);
        tmp_condition_result_71 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_71 == NUITKA_BOOL_TRUE) {
            goto branch_yes_53;
        } else {
            goto branch_no_53;
        }
    }
    branch_yes_53:;
    {
        PyObject *tmp_assign_source_129;
        PyObject *tmp_called_name_35;
        PyObject *tmp_expression_name_88;
        PyObject *tmp_args_name_35;
        PyObject *tmp_tuple_element_69;
        PyObject *tmp_kwargs_name_35;
        CHECK_OBJECT(tmp_class_creation_18__metaclass);
        tmp_expression_name_88 = tmp_class_creation_18__metaclass;
        tmp_called_name_35 = LOOKUP_ATTRIBUTE(tmp_expression_name_88, mod_consts[50]);
        if (tmp_called_name_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        tmp_tuple_element_69 = mod_consts[100];
        tmp_args_name_35 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_35, 0, tmp_tuple_element_69);
        CHECK_OBJECT(tmp_class_creation_18__bases);
        tmp_tuple_element_69 = tmp_class_creation_18__bases;
        PyTuple_SET_ITEM0(tmp_args_name_35, 1, tmp_tuple_element_69);
        CHECK_OBJECT(tmp_class_creation_18__class_decl_dict);
        tmp_kwargs_name_35 = tmp_class_creation_18__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 152;
        tmp_assign_source_129 = CALL_FUNCTION(tmp_called_name_35, tmp_args_name_35, tmp_kwargs_name_35);
        Py_DECREF(tmp_called_name_35);
        Py_DECREF(tmp_args_name_35);
        if (tmp_assign_source_129 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        assert(tmp_class_creation_18__prepared == NULL);
        tmp_class_creation_18__prepared = tmp_assign_source_129;
    }
    {
        nuitka_bool tmp_condition_result_72;
        PyObject *tmp_operand_name_18;
        PyObject *tmp_expression_name_89;
        CHECK_OBJECT(tmp_class_creation_18__prepared);
        tmp_expression_name_89 = tmp_class_creation_18__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_89, mod_consts[51]);
        tmp_operand_name_18 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_18);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        tmp_condition_result_72 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_72 == NUITKA_BOOL_TRUE) {
            goto branch_yes_54;
        } else {
            goto branch_no_54;
        }
    }
    branch_yes_54:;
    {
        PyObject *tmp_raise_type_18;
        PyObject *tmp_raise_value_18;
        PyObject *tmp_left_name_18;
        PyObject *tmp_right_name_18;
        PyObject *tmp_tuple_element_70;
        PyObject *tmp_getattr_target_18;
        PyObject *tmp_getattr_attr_18;
        PyObject *tmp_getattr_default_18;
        tmp_raise_type_18 = PyExc_TypeError;
        tmp_left_name_18 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_18__metaclass);
        tmp_getattr_target_18 = tmp_class_creation_18__metaclass;
        tmp_getattr_attr_18 = mod_consts[53];
        tmp_getattr_default_18 = mod_consts[54];
        tmp_tuple_element_70 = BUILTIN_GETATTR(tmp_getattr_target_18, tmp_getattr_attr_18, tmp_getattr_default_18);
        if (tmp_tuple_element_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        tmp_right_name_18 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_90;
            PyObject *tmp_type_arg_36;
            PyTuple_SET_ITEM(tmp_right_name_18, 0, tmp_tuple_element_70);
            CHECK_OBJECT(tmp_class_creation_18__prepared);
            tmp_type_arg_36 = tmp_class_creation_18__prepared;
            tmp_expression_name_90 = BUILTIN_TYPE1(tmp_type_arg_36);
            assert(!(tmp_expression_name_90 == NULL));
            tmp_tuple_element_70 = LOOKUP_ATTRIBUTE(tmp_expression_name_90, mod_consts[53]);
            Py_DECREF(tmp_expression_name_90);
            if (tmp_tuple_element_70 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 152;

                goto tuple_build_exception_21;
            }
            PyTuple_SET_ITEM(tmp_right_name_18, 1, tmp_tuple_element_70);
        }
        goto tuple_build_noexception_21;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_21:;
        Py_DECREF(tmp_right_name_18);
        goto try_except_handler_52;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_21:;
        tmp_raise_value_18 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_18, tmp_right_name_18);
        Py_DECREF(tmp_right_name_18);
        if (tmp_raise_value_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_52;
        }
        exception_type = tmp_raise_type_18;
        Py_INCREF(tmp_raise_type_18);
        exception_value = tmp_raise_value_18;
        exception_lineno = 152;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_52;
    }
    branch_no_54:;
    goto branch_end_53;
    branch_no_53:;
    {
        PyObject *tmp_assign_source_130;
        tmp_assign_source_130 = PyDict_New();
        assert(tmp_class_creation_18__prepared == NULL);
        tmp_class_creation_18__prepared = tmp_assign_source_130;
    }
    branch_end_53:;
    {
        PyObject *tmp_assign_source_131;
        {
            PyObject *tmp_set_locals_18;
            CHECK_OBJECT(tmp_class_creation_18__prepared);
            tmp_set_locals_18 = tmp_class_creation_18__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152 = tmp_set_locals_18;
            Py_INCREF(tmp_set_locals_18);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_54;
        }
        tmp_dictset_value = mod_consts[101];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_54;
        }
        tmp_dictset_value = mod_consts[100];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 152;

            goto try_except_handler_54;
        }
        {
            PyObject *tmp_assign_source_132;
            PyObject *tmp_called_name_36;
            PyObject *tmp_args_name_36;
            PyObject *tmp_tuple_element_71;
            PyObject *tmp_kwargs_name_36;
            CHECK_OBJECT(tmp_class_creation_18__metaclass);
            tmp_called_name_36 = tmp_class_creation_18__metaclass;
            tmp_tuple_element_71 = mod_consts[100];
            tmp_args_name_36 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_36, 0, tmp_tuple_element_71);
            CHECK_OBJECT(tmp_class_creation_18__bases);
            tmp_tuple_element_71 = tmp_class_creation_18__bases;
            PyTuple_SET_ITEM0(tmp_args_name_36, 1, tmp_tuple_element_71);
            tmp_tuple_element_71 = locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152;
            PyTuple_SET_ITEM0(tmp_args_name_36, 2, tmp_tuple_element_71);
            CHECK_OBJECT(tmp_class_creation_18__class_decl_dict);
            tmp_kwargs_name_36 = tmp_class_creation_18__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 152;
            tmp_assign_source_132 = CALL_FUNCTION(tmp_called_name_36, tmp_args_name_36, tmp_kwargs_name_36);
            Py_DECREF(tmp_args_name_36);
            if (tmp_assign_source_132 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 152;

                goto try_except_handler_54;
            }
            assert(outline_17_var___class__ == NULL);
            outline_17_var___class__ = tmp_assign_source_132;
        }
        CHECK_OBJECT(outline_17_var___class__);
        tmp_assign_source_131 = outline_17_var___class__;
        Py_INCREF(tmp_assign_source_131);
        goto try_return_handler_54;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_54:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152);
        locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152 = NULL;
        goto try_return_handler_53;
        // Exception handler code:
        try_except_handler_54:;
        exception_keeper_type_52 = exception_type;
        exception_keeper_value_52 = exception_value;
        exception_keeper_tb_52 = exception_tb;
        exception_keeper_lineno_52 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152);
        locals_pip$_vendor$urllib3$exceptions$$$class__18_LocationValueError_152 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_52;
        exception_value = exception_keeper_value_52;
        exception_tb = exception_keeper_tb_52;
        exception_lineno = exception_keeper_lineno_52;

        goto try_except_handler_53;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_53:;
        CHECK_OBJECT(outline_17_var___class__);
        Py_DECREF(outline_17_var___class__);
        outline_17_var___class__ = NULL;
        goto outline_result_18;
        // Exception handler code:
        try_except_handler_53:;
        exception_keeper_type_53 = exception_type;
        exception_keeper_value_53 = exception_value;
        exception_keeper_tb_53 = exception_tb;
        exception_keeper_lineno_53 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_53;
        exception_value = exception_keeper_value_53;
        exception_tb = exception_keeper_tb_53;
        exception_lineno = exception_keeper_lineno_53;

        goto outline_exception_18;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_18:;
        exception_lineno = 152;
        goto try_except_handler_52;
        outline_result_18:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[100], tmp_assign_source_131);
    }
    goto try_end_18;
    // Exception handler code:
    try_except_handler_52:;
    exception_keeper_type_54 = exception_type;
    exception_keeper_value_54 = exception_value;
    exception_keeper_tb_54 = exception_tb;
    exception_keeper_lineno_54 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_18__bases);
    tmp_class_creation_18__bases = NULL;
    Py_XDECREF(tmp_class_creation_18__class_decl_dict);
    tmp_class_creation_18__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_18__metaclass);
    tmp_class_creation_18__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_18__prepared);
    tmp_class_creation_18__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_54;
    exception_value = exception_keeper_value_54;
    exception_tb = exception_keeper_tb_54;
    exception_lineno = exception_keeper_lineno_54;

    goto frame_exception_exit_1;
    // End of try:
    try_end_18:;
    Py_XDECREF(tmp_class_creation_18__bases);
    tmp_class_creation_18__bases = NULL;
    Py_XDECREF(tmp_class_creation_18__class_decl_dict);
    tmp_class_creation_18__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_18__metaclass);
    tmp_class_creation_18__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_18__prepared);
    Py_DECREF(tmp_class_creation_18__prepared);
    tmp_class_creation_18__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_133;
        PyObject *tmp_tuple_element_72;
        tmp_tuple_element_72 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[100]);

        if (unlikely(tmp_tuple_element_72 == NULL)) {
            tmp_tuple_element_72 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[100]);
        }

        if (tmp_tuple_element_72 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        tmp_assign_source_133 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_133, 0, tmp_tuple_element_72);
        assert(tmp_class_creation_19__bases == NULL);
        tmp_class_creation_19__bases = tmp_assign_source_133;
    }
    {
        PyObject *tmp_assign_source_134;
        tmp_assign_source_134 = PyDict_New();
        assert(tmp_class_creation_19__class_decl_dict == NULL);
        tmp_class_creation_19__class_decl_dict = tmp_assign_source_134;
    }
    {
        PyObject *tmp_assign_source_135;
        PyObject *tmp_metaclass_name_19;
        nuitka_bool tmp_condition_result_73;
        PyObject *tmp_key_name_55;
        PyObject *tmp_dict_arg_name_55;
        PyObject *tmp_dict_arg_name_56;
        PyObject *tmp_key_name_56;
        PyObject *tmp_type_arg_37;
        PyObject *tmp_expression_name_91;
        PyObject *tmp_subscript_name_19;
        PyObject *tmp_bases_name_19;
        tmp_key_name_55 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_19__class_decl_dict);
        tmp_dict_arg_name_55 = tmp_class_creation_19__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_55, tmp_key_name_55);
        assert(!(tmp_res == -1));
        tmp_condition_result_73 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_73 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_19;
        } else {
            goto condexpr_false_19;
        }
        condexpr_true_19:;
        CHECK_OBJECT(tmp_class_creation_19__class_decl_dict);
        tmp_dict_arg_name_56 = tmp_class_creation_19__class_decl_dict;
        tmp_key_name_56 = mod_consts[49];
        tmp_metaclass_name_19 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_56, tmp_key_name_56);
        if (tmp_metaclass_name_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        goto condexpr_end_19;
        condexpr_false_19:;
        CHECK_OBJECT(tmp_class_creation_19__bases);
        tmp_expression_name_91 = tmp_class_creation_19__bases;
        tmp_subscript_name_19 = mod_consts[46];
        tmp_type_arg_37 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_91, tmp_subscript_name_19, 0);
        if (tmp_type_arg_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        tmp_metaclass_name_19 = BUILTIN_TYPE1(tmp_type_arg_37);
        Py_DECREF(tmp_type_arg_37);
        if (tmp_metaclass_name_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        condexpr_end_19:;
        CHECK_OBJECT(tmp_class_creation_19__bases);
        tmp_bases_name_19 = tmp_class_creation_19__bases;
        tmp_assign_source_135 = SELECT_METACLASS(tmp_metaclass_name_19, tmp_bases_name_19);
        Py_DECREF(tmp_metaclass_name_19);
        if (tmp_assign_source_135 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        assert(tmp_class_creation_19__metaclass == NULL);
        tmp_class_creation_19__metaclass = tmp_assign_source_135;
    }
    {
        nuitka_bool tmp_condition_result_74;
        PyObject *tmp_key_name_57;
        PyObject *tmp_dict_arg_name_57;
        tmp_key_name_57 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_19__class_decl_dict);
        tmp_dict_arg_name_57 = tmp_class_creation_19__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_57, tmp_key_name_57);
        assert(!(tmp_res == -1));
        tmp_condition_result_74 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_74 == NUITKA_BOOL_TRUE) {
            goto branch_yes_55;
        } else {
            goto branch_no_55;
        }
    }
    branch_yes_55:;
    CHECK_OBJECT(tmp_class_creation_19__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_19__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 158;

        goto try_except_handler_55;
    }
    branch_no_55:;
    {
        nuitka_bool tmp_condition_result_75;
        PyObject *tmp_expression_name_92;
        CHECK_OBJECT(tmp_class_creation_19__metaclass);
        tmp_expression_name_92 = tmp_class_creation_19__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_92, mod_consts[50]);
        tmp_condition_result_75 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_75 == NUITKA_BOOL_TRUE) {
            goto branch_yes_56;
        } else {
            goto branch_no_56;
        }
    }
    branch_yes_56:;
    {
        PyObject *tmp_assign_source_136;
        PyObject *tmp_called_name_37;
        PyObject *tmp_expression_name_93;
        PyObject *tmp_args_name_37;
        PyObject *tmp_tuple_element_73;
        PyObject *tmp_kwargs_name_37;
        CHECK_OBJECT(tmp_class_creation_19__metaclass);
        tmp_expression_name_93 = tmp_class_creation_19__metaclass;
        tmp_called_name_37 = LOOKUP_ATTRIBUTE(tmp_expression_name_93, mod_consts[50]);
        if (tmp_called_name_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        tmp_tuple_element_73 = mod_consts[102];
        tmp_args_name_37 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_37, 0, tmp_tuple_element_73);
        CHECK_OBJECT(tmp_class_creation_19__bases);
        tmp_tuple_element_73 = tmp_class_creation_19__bases;
        PyTuple_SET_ITEM0(tmp_args_name_37, 1, tmp_tuple_element_73);
        CHECK_OBJECT(tmp_class_creation_19__class_decl_dict);
        tmp_kwargs_name_37 = tmp_class_creation_19__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 158;
        tmp_assign_source_136 = CALL_FUNCTION(tmp_called_name_37, tmp_args_name_37, tmp_kwargs_name_37);
        Py_DECREF(tmp_called_name_37);
        Py_DECREF(tmp_args_name_37);
        if (tmp_assign_source_136 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        assert(tmp_class_creation_19__prepared == NULL);
        tmp_class_creation_19__prepared = tmp_assign_source_136;
    }
    {
        nuitka_bool tmp_condition_result_76;
        PyObject *tmp_operand_name_19;
        PyObject *tmp_expression_name_94;
        CHECK_OBJECT(tmp_class_creation_19__prepared);
        tmp_expression_name_94 = tmp_class_creation_19__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_94, mod_consts[51]);
        tmp_operand_name_19 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_19);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        tmp_condition_result_76 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_76 == NUITKA_BOOL_TRUE) {
            goto branch_yes_57;
        } else {
            goto branch_no_57;
        }
    }
    branch_yes_57:;
    {
        PyObject *tmp_raise_type_19;
        PyObject *tmp_raise_value_19;
        PyObject *tmp_left_name_19;
        PyObject *tmp_right_name_19;
        PyObject *tmp_tuple_element_74;
        PyObject *tmp_getattr_target_19;
        PyObject *tmp_getattr_attr_19;
        PyObject *tmp_getattr_default_19;
        tmp_raise_type_19 = PyExc_TypeError;
        tmp_left_name_19 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_19__metaclass);
        tmp_getattr_target_19 = tmp_class_creation_19__metaclass;
        tmp_getattr_attr_19 = mod_consts[53];
        tmp_getattr_default_19 = mod_consts[54];
        tmp_tuple_element_74 = BUILTIN_GETATTR(tmp_getattr_target_19, tmp_getattr_attr_19, tmp_getattr_default_19);
        if (tmp_tuple_element_74 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        tmp_right_name_19 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_95;
            PyObject *tmp_type_arg_38;
            PyTuple_SET_ITEM(tmp_right_name_19, 0, tmp_tuple_element_74);
            CHECK_OBJECT(tmp_class_creation_19__prepared);
            tmp_type_arg_38 = tmp_class_creation_19__prepared;
            tmp_expression_name_95 = BUILTIN_TYPE1(tmp_type_arg_38);
            assert(!(tmp_expression_name_95 == NULL));
            tmp_tuple_element_74 = LOOKUP_ATTRIBUTE(tmp_expression_name_95, mod_consts[53]);
            Py_DECREF(tmp_expression_name_95);
            if (tmp_tuple_element_74 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 158;

                goto tuple_build_exception_22;
            }
            PyTuple_SET_ITEM(tmp_right_name_19, 1, tmp_tuple_element_74);
        }
        goto tuple_build_noexception_22;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_22:;
        Py_DECREF(tmp_right_name_19);
        goto try_except_handler_55;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_22:;
        tmp_raise_value_19 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_19, tmp_right_name_19);
        Py_DECREF(tmp_right_name_19);
        if (tmp_raise_value_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_55;
        }
        exception_type = tmp_raise_type_19;
        Py_INCREF(tmp_raise_type_19);
        exception_value = tmp_raise_value_19;
        exception_lineno = 158;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_55;
    }
    branch_no_57:;
    goto branch_end_56;
    branch_no_56:;
    {
        PyObject *tmp_assign_source_137;
        tmp_assign_source_137 = PyDict_New();
        assert(tmp_class_creation_19__prepared == NULL);
        tmp_class_creation_19__prepared = tmp_assign_source_137;
    }
    branch_end_56:;
    {
        PyObject *tmp_assign_source_138;
        {
            PyObject *tmp_set_locals_19;
            CHECK_OBJECT(tmp_class_creation_19__prepared);
            tmp_set_locals_19 = tmp_class_creation_19__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158 = tmp_set_locals_19;
            Py_INCREF(tmp_set_locals_19);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_57;
        }
        tmp_dictset_value = mod_consts[103];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_57;
        }
        tmp_dictset_value = mod_consts[102];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 158;

            goto try_except_handler_57;
        }
        if (isFrameUnusable(cache_frame_066d2c927324a67686dc7baef2a7a942_7)) {
            Py_XDECREF(cache_frame_066d2c927324a67686dc7baef2a7a942_7);

#if _DEBUG_REFCOUNTS
            if (cache_frame_066d2c927324a67686dc7baef2a7a942_7 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_066d2c927324a67686dc7baef2a7a942_7 = MAKE_FUNCTION_FRAME(codeobj_066d2c927324a67686dc7baef2a7a942, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_066d2c927324a67686dc7baef2a7a942_7->m_type_description == NULL);
        frame_066d2c927324a67686dc7baef2a7a942_7 = cache_frame_066d2c927324a67686dc7baef2a7a942_7;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_066d2c927324a67686dc7baef2a7a942_7);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_066d2c927324a67686dc7baef2a7a942_7) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__8___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 161;
            type_description_2 = "o";
            goto frame_exception_exit_7;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_066d2c927324a67686dc7baef2a7a942_7);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_6;

        frame_exception_exit_7:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_066d2c927324a67686dc7baef2a7a942_7);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_066d2c927324a67686dc7baef2a7a942_7, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_066d2c927324a67686dc7baef2a7a942_7->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_066d2c927324a67686dc7baef2a7a942_7, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_066d2c927324a67686dc7baef2a7a942_7,
            type_description_2,
            outline_18_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_066d2c927324a67686dc7baef2a7a942_7 == cache_frame_066d2c927324a67686dc7baef2a7a942_7) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_066d2c927324a67686dc7baef2a7a942_7);
            cache_frame_066d2c927324a67686dc7baef2a7a942_7 = NULL;
        }

        assertFrameObject(frame_066d2c927324a67686dc7baef2a7a942_7);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_6;

        frame_no_exception_6:;
        goto skip_nested_handling_6;
        nested_frame_exit_6:;

        goto try_except_handler_57;
        skip_nested_handling_6:;
        {
            PyObject *tmp_assign_source_139;
            PyObject *tmp_called_name_38;
            PyObject *tmp_args_name_38;
            PyObject *tmp_tuple_element_75;
            PyObject *tmp_kwargs_name_38;
            CHECK_OBJECT(tmp_class_creation_19__metaclass);
            tmp_called_name_38 = tmp_class_creation_19__metaclass;
            tmp_tuple_element_75 = mod_consts[102];
            tmp_args_name_38 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_38, 0, tmp_tuple_element_75);
            CHECK_OBJECT(tmp_class_creation_19__bases);
            tmp_tuple_element_75 = tmp_class_creation_19__bases;
            PyTuple_SET_ITEM0(tmp_args_name_38, 1, tmp_tuple_element_75);
            tmp_tuple_element_75 = locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158;
            PyTuple_SET_ITEM0(tmp_args_name_38, 2, tmp_tuple_element_75);
            CHECK_OBJECT(tmp_class_creation_19__class_decl_dict);
            tmp_kwargs_name_38 = tmp_class_creation_19__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 158;
            tmp_assign_source_139 = CALL_FUNCTION(tmp_called_name_38, tmp_args_name_38, tmp_kwargs_name_38);
            Py_DECREF(tmp_args_name_38);
            if (tmp_assign_source_139 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 158;

                goto try_except_handler_57;
            }
            assert(outline_18_var___class__ == NULL);
            outline_18_var___class__ = tmp_assign_source_139;
        }
        CHECK_OBJECT(outline_18_var___class__);
        tmp_assign_source_138 = outline_18_var___class__;
        Py_INCREF(tmp_assign_source_138);
        goto try_return_handler_57;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_57:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158);
        locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158 = NULL;
        goto try_return_handler_56;
        // Exception handler code:
        try_except_handler_57:;
        exception_keeper_type_55 = exception_type;
        exception_keeper_value_55 = exception_value;
        exception_keeper_tb_55 = exception_tb;
        exception_keeper_lineno_55 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158);
        locals_pip$_vendor$urllib3$exceptions$$$class__19_LocationParseError_158 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_55;
        exception_value = exception_keeper_value_55;
        exception_tb = exception_keeper_tb_55;
        exception_lineno = exception_keeper_lineno_55;

        goto try_except_handler_56;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_56:;
        CHECK_OBJECT(outline_18_var___class__);
        Py_DECREF(outline_18_var___class__);
        outline_18_var___class__ = NULL;
        goto outline_result_19;
        // Exception handler code:
        try_except_handler_56:;
        exception_keeper_type_56 = exception_type;
        exception_keeper_value_56 = exception_value;
        exception_keeper_tb_56 = exception_tb;
        exception_keeper_lineno_56 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_56;
        exception_value = exception_keeper_value_56;
        exception_tb = exception_keeper_tb_56;
        exception_lineno = exception_keeper_lineno_56;

        goto outline_exception_19;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_19:;
        exception_lineno = 158;
        goto try_except_handler_55;
        outline_result_19:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[102], tmp_assign_source_138);
    }
    goto try_end_19;
    // Exception handler code:
    try_except_handler_55:;
    exception_keeper_type_57 = exception_type;
    exception_keeper_value_57 = exception_value;
    exception_keeper_tb_57 = exception_tb;
    exception_keeper_lineno_57 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_19__bases);
    tmp_class_creation_19__bases = NULL;
    Py_XDECREF(tmp_class_creation_19__class_decl_dict);
    tmp_class_creation_19__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_19__metaclass);
    tmp_class_creation_19__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_19__prepared);
    tmp_class_creation_19__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_57;
    exception_value = exception_keeper_value_57;
    exception_tb = exception_keeper_tb_57;
    exception_lineno = exception_keeper_lineno_57;

    goto frame_exception_exit_1;
    // End of try:
    try_end_19:;
    Py_XDECREF(tmp_class_creation_19__bases);
    tmp_class_creation_19__bases = NULL;
    Py_XDECREF(tmp_class_creation_19__class_decl_dict);
    tmp_class_creation_19__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_19__metaclass);
    tmp_class_creation_19__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_19__prepared);
    Py_DECREF(tmp_class_creation_19__prepared);
    tmp_class_creation_19__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_140;
        PyObject *tmp_tuple_element_76;
        tmp_tuple_element_76 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[100]);

        if (unlikely(tmp_tuple_element_76 == NULL)) {
            tmp_tuple_element_76 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[100]);
        }

        if (tmp_tuple_element_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        tmp_assign_source_140 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_140, 0, tmp_tuple_element_76);
        assert(tmp_class_creation_20__bases == NULL);
        tmp_class_creation_20__bases = tmp_assign_source_140;
    }
    {
        PyObject *tmp_assign_source_141;
        tmp_assign_source_141 = PyDict_New();
        assert(tmp_class_creation_20__class_decl_dict == NULL);
        tmp_class_creation_20__class_decl_dict = tmp_assign_source_141;
    }
    {
        PyObject *tmp_assign_source_142;
        PyObject *tmp_metaclass_name_20;
        nuitka_bool tmp_condition_result_77;
        PyObject *tmp_key_name_58;
        PyObject *tmp_dict_arg_name_58;
        PyObject *tmp_dict_arg_name_59;
        PyObject *tmp_key_name_59;
        PyObject *tmp_type_arg_39;
        PyObject *tmp_expression_name_96;
        PyObject *tmp_subscript_name_20;
        PyObject *tmp_bases_name_20;
        tmp_key_name_58 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_20__class_decl_dict);
        tmp_dict_arg_name_58 = tmp_class_creation_20__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_58, tmp_key_name_58);
        assert(!(tmp_res == -1));
        tmp_condition_result_77 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_77 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_20;
        } else {
            goto condexpr_false_20;
        }
        condexpr_true_20:;
        CHECK_OBJECT(tmp_class_creation_20__class_decl_dict);
        tmp_dict_arg_name_59 = tmp_class_creation_20__class_decl_dict;
        tmp_key_name_59 = mod_consts[49];
        tmp_metaclass_name_20 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_59, tmp_key_name_59);
        if (tmp_metaclass_name_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        goto condexpr_end_20;
        condexpr_false_20:;
        CHECK_OBJECT(tmp_class_creation_20__bases);
        tmp_expression_name_96 = tmp_class_creation_20__bases;
        tmp_subscript_name_20 = mod_consts[46];
        tmp_type_arg_39 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_96, tmp_subscript_name_20, 0);
        if (tmp_type_arg_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        tmp_metaclass_name_20 = BUILTIN_TYPE1(tmp_type_arg_39);
        Py_DECREF(tmp_type_arg_39);
        if (tmp_metaclass_name_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        condexpr_end_20:;
        CHECK_OBJECT(tmp_class_creation_20__bases);
        tmp_bases_name_20 = tmp_class_creation_20__bases;
        tmp_assign_source_142 = SELECT_METACLASS(tmp_metaclass_name_20, tmp_bases_name_20);
        Py_DECREF(tmp_metaclass_name_20);
        if (tmp_assign_source_142 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        assert(tmp_class_creation_20__metaclass == NULL);
        tmp_class_creation_20__metaclass = tmp_assign_source_142;
    }
    {
        nuitka_bool tmp_condition_result_78;
        PyObject *tmp_key_name_60;
        PyObject *tmp_dict_arg_name_60;
        tmp_key_name_60 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_20__class_decl_dict);
        tmp_dict_arg_name_60 = tmp_class_creation_20__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_60, tmp_key_name_60);
        assert(!(tmp_res == -1));
        tmp_condition_result_78 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_78 == NUITKA_BOOL_TRUE) {
            goto branch_yes_58;
        } else {
            goto branch_no_58;
        }
    }
    branch_yes_58:;
    CHECK_OBJECT(tmp_class_creation_20__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_20__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 168;

        goto try_except_handler_58;
    }
    branch_no_58:;
    {
        nuitka_bool tmp_condition_result_79;
        PyObject *tmp_expression_name_97;
        CHECK_OBJECT(tmp_class_creation_20__metaclass);
        tmp_expression_name_97 = tmp_class_creation_20__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_97, mod_consts[50]);
        tmp_condition_result_79 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_79 == NUITKA_BOOL_TRUE) {
            goto branch_yes_59;
        } else {
            goto branch_no_59;
        }
    }
    branch_yes_59:;
    {
        PyObject *tmp_assign_source_143;
        PyObject *tmp_called_name_39;
        PyObject *tmp_expression_name_98;
        PyObject *tmp_args_name_39;
        PyObject *tmp_tuple_element_77;
        PyObject *tmp_kwargs_name_39;
        CHECK_OBJECT(tmp_class_creation_20__metaclass);
        tmp_expression_name_98 = tmp_class_creation_20__metaclass;
        tmp_called_name_39 = LOOKUP_ATTRIBUTE(tmp_expression_name_98, mod_consts[50]);
        if (tmp_called_name_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        tmp_tuple_element_77 = mod_consts[18];
        tmp_args_name_39 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_39, 0, tmp_tuple_element_77);
        CHECK_OBJECT(tmp_class_creation_20__bases);
        tmp_tuple_element_77 = tmp_class_creation_20__bases;
        PyTuple_SET_ITEM0(tmp_args_name_39, 1, tmp_tuple_element_77);
        CHECK_OBJECT(tmp_class_creation_20__class_decl_dict);
        tmp_kwargs_name_39 = tmp_class_creation_20__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 168;
        tmp_assign_source_143 = CALL_FUNCTION(tmp_called_name_39, tmp_args_name_39, tmp_kwargs_name_39);
        Py_DECREF(tmp_called_name_39);
        Py_DECREF(tmp_args_name_39);
        if (tmp_assign_source_143 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        assert(tmp_class_creation_20__prepared == NULL);
        tmp_class_creation_20__prepared = tmp_assign_source_143;
    }
    {
        nuitka_bool tmp_condition_result_80;
        PyObject *tmp_operand_name_20;
        PyObject *tmp_expression_name_99;
        CHECK_OBJECT(tmp_class_creation_20__prepared);
        tmp_expression_name_99 = tmp_class_creation_20__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_99, mod_consts[51]);
        tmp_operand_name_20 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_20);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        tmp_condition_result_80 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_80 == NUITKA_BOOL_TRUE) {
            goto branch_yes_60;
        } else {
            goto branch_no_60;
        }
    }
    branch_yes_60:;
    {
        PyObject *tmp_raise_type_20;
        PyObject *tmp_raise_value_20;
        PyObject *tmp_left_name_20;
        PyObject *tmp_right_name_20;
        PyObject *tmp_tuple_element_78;
        PyObject *tmp_getattr_target_20;
        PyObject *tmp_getattr_attr_20;
        PyObject *tmp_getattr_default_20;
        tmp_raise_type_20 = PyExc_TypeError;
        tmp_left_name_20 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_20__metaclass);
        tmp_getattr_target_20 = tmp_class_creation_20__metaclass;
        tmp_getattr_attr_20 = mod_consts[53];
        tmp_getattr_default_20 = mod_consts[54];
        tmp_tuple_element_78 = BUILTIN_GETATTR(tmp_getattr_target_20, tmp_getattr_attr_20, tmp_getattr_default_20);
        if (tmp_tuple_element_78 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        tmp_right_name_20 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_100;
            PyObject *tmp_type_arg_40;
            PyTuple_SET_ITEM(tmp_right_name_20, 0, tmp_tuple_element_78);
            CHECK_OBJECT(tmp_class_creation_20__prepared);
            tmp_type_arg_40 = tmp_class_creation_20__prepared;
            tmp_expression_name_100 = BUILTIN_TYPE1(tmp_type_arg_40);
            assert(!(tmp_expression_name_100 == NULL));
            tmp_tuple_element_78 = LOOKUP_ATTRIBUTE(tmp_expression_name_100, mod_consts[53]);
            Py_DECREF(tmp_expression_name_100);
            if (tmp_tuple_element_78 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 168;

                goto tuple_build_exception_23;
            }
            PyTuple_SET_ITEM(tmp_right_name_20, 1, tmp_tuple_element_78);
        }
        goto tuple_build_noexception_23;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_23:;
        Py_DECREF(tmp_right_name_20);
        goto try_except_handler_58;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_23:;
        tmp_raise_value_20 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_20, tmp_right_name_20);
        Py_DECREF(tmp_right_name_20);
        if (tmp_raise_value_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_58;
        }
        exception_type = tmp_raise_type_20;
        Py_INCREF(tmp_raise_type_20);
        exception_value = tmp_raise_value_20;
        exception_lineno = 168;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_58;
    }
    branch_no_60:;
    goto branch_end_59;
    branch_no_59:;
    {
        PyObject *tmp_assign_source_144;
        tmp_assign_source_144 = PyDict_New();
        assert(tmp_class_creation_20__prepared == NULL);
        tmp_class_creation_20__prepared = tmp_assign_source_144;
    }
    branch_end_59:;
    {
        PyObject *tmp_assign_source_145;
        {
            PyObject *tmp_set_locals_20;
            CHECK_OBJECT(tmp_class_creation_20__prepared);
            tmp_set_locals_20 = tmp_class_creation_20__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168 = tmp_set_locals_20;
            Py_INCREF(tmp_set_locals_20);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_60;
        }
        tmp_dictset_value = mod_consts[105];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_60;
        }
        tmp_dictset_value = mod_consts[18];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 168;

            goto try_except_handler_60;
        }
        if (isFrameUnusable(cache_frame_db8cc6926c2108e877484bc63b18157c_8)) {
            Py_XDECREF(cache_frame_db8cc6926c2108e877484bc63b18157c_8);

#if _DEBUG_REFCOUNTS
            if (cache_frame_db8cc6926c2108e877484bc63b18157c_8 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_db8cc6926c2108e877484bc63b18157c_8 = MAKE_FUNCTION_FRAME(codeobj_db8cc6926c2108e877484bc63b18157c, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_db8cc6926c2108e877484bc63b18157c_8->m_type_description == NULL);
        frame_db8cc6926c2108e877484bc63b18157c_8 = cache_frame_db8cc6926c2108e877484bc63b18157c_8;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_db8cc6926c2108e877484bc63b18157c_8);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_db8cc6926c2108e877484bc63b18157c_8) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__9___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 171;
            type_description_2 = "o";
            goto frame_exception_exit_8;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_db8cc6926c2108e877484bc63b18157c_8);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_7;

        frame_exception_exit_8:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_db8cc6926c2108e877484bc63b18157c_8);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_db8cc6926c2108e877484bc63b18157c_8, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_db8cc6926c2108e877484bc63b18157c_8->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_db8cc6926c2108e877484bc63b18157c_8, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_db8cc6926c2108e877484bc63b18157c_8,
            type_description_2,
            outline_19_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_db8cc6926c2108e877484bc63b18157c_8 == cache_frame_db8cc6926c2108e877484bc63b18157c_8) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_db8cc6926c2108e877484bc63b18157c_8);
            cache_frame_db8cc6926c2108e877484bc63b18157c_8 = NULL;
        }

        assertFrameObject(frame_db8cc6926c2108e877484bc63b18157c_8);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_7;

        frame_no_exception_7:;
        goto skip_nested_handling_7;
        nested_frame_exit_7:;

        goto try_except_handler_60;
        skip_nested_handling_7:;
        {
            PyObject *tmp_assign_source_146;
            PyObject *tmp_called_name_40;
            PyObject *tmp_args_name_40;
            PyObject *tmp_tuple_element_79;
            PyObject *tmp_kwargs_name_40;
            CHECK_OBJECT(tmp_class_creation_20__metaclass);
            tmp_called_name_40 = tmp_class_creation_20__metaclass;
            tmp_tuple_element_79 = mod_consts[18];
            tmp_args_name_40 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_40, 0, tmp_tuple_element_79);
            CHECK_OBJECT(tmp_class_creation_20__bases);
            tmp_tuple_element_79 = tmp_class_creation_20__bases;
            PyTuple_SET_ITEM0(tmp_args_name_40, 1, tmp_tuple_element_79);
            tmp_tuple_element_79 = locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168;
            PyTuple_SET_ITEM0(tmp_args_name_40, 2, tmp_tuple_element_79);
            CHECK_OBJECT(tmp_class_creation_20__class_decl_dict);
            tmp_kwargs_name_40 = tmp_class_creation_20__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 168;
            tmp_assign_source_146 = CALL_FUNCTION(tmp_called_name_40, tmp_args_name_40, tmp_kwargs_name_40);
            Py_DECREF(tmp_args_name_40);
            if (tmp_assign_source_146 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 168;

                goto try_except_handler_60;
            }
            assert(outline_19_var___class__ == NULL);
            outline_19_var___class__ = tmp_assign_source_146;
        }
        CHECK_OBJECT(outline_19_var___class__);
        tmp_assign_source_145 = outline_19_var___class__;
        Py_INCREF(tmp_assign_source_145);
        goto try_return_handler_60;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_60:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168);
        locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168 = NULL;
        goto try_return_handler_59;
        // Exception handler code:
        try_except_handler_60:;
        exception_keeper_type_58 = exception_type;
        exception_keeper_value_58 = exception_value;
        exception_keeper_tb_58 = exception_tb;
        exception_keeper_lineno_58 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168);
        locals_pip$_vendor$urllib3$exceptions$$$class__20_URLSchemeUnknown_168 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_58;
        exception_value = exception_keeper_value_58;
        exception_tb = exception_keeper_tb_58;
        exception_lineno = exception_keeper_lineno_58;

        goto try_except_handler_59;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_59:;
        CHECK_OBJECT(outline_19_var___class__);
        Py_DECREF(outline_19_var___class__);
        outline_19_var___class__ = NULL;
        goto outline_result_20;
        // Exception handler code:
        try_except_handler_59:;
        exception_keeper_type_59 = exception_type;
        exception_keeper_value_59 = exception_value;
        exception_keeper_tb_59 = exception_tb;
        exception_keeper_lineno_59 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_59;
        exception_value = exception_keeper_value_59;
        exception_tb = exception_keeper_tb_59;
        exception_lineno = exception_keeper_lineno_59;

        goto outline_exception_20;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_20:;
        exception_lineno = 168;
        goto try_except_handler_58;
        outline_result_20:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[18], tmp_assign_source_145);
    }
    goto try_end_20;
    // Exception handler code:
    try_except_handler_58:;
    exception_keeper_type_60 = exception_type;
    exception_keeper_value_60 = exception_value;
    exception_keeper_tb_60 = exception_tb;
    exception_keeper_lineno_60 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_20__bases);
    tmp_class_creation_20__bases = NULL;
    Py_XDECREF(tmp_class_creation_20__class_decl_dict);
    tmp_class_creation_20__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_20__metaclass);
    tmp_class_creation_20__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_20__prepared);
    tmp_class_creation_20__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_60;
    exception_value = exception_keeper_value_60;
    exception_tb = exception_keeper_tb_60;
    exception_lineno = exception_keeper_lineno_60;

    goto frame_exception_exit_1;
    // End of try:
    try_end_20:;
    Py_XDECREF(tmp_class_creation_20__bases);
    tmp_class_creation_20__bases = NULL;
    Py_XDECREF(tmp_class_creation_20__class_decl_dict);
    tmp_class_creation_20__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_20__metaclass);
    tmp_class_creation_20__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_20__prepared);
    Py_DECREF(tmp_class_creation_20__prepared);
    tmp_class_creation_20__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_147;
        PyObject *tmp_tuple_element_80;
        tmp_tuple_element_80 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_80 == NULL)) {
            tmp_tuple_element_80 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_80 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        tmp_assign_source_147 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_147, 0, tmp_tuple_element_80);
        assert(tmp_class_creation_21__bases == NULL);
        tmp_class_creation_21__bases = tmp_assign_source_147;
    }
    {
        PyObject *tmp_assign_source_148;
        tmp_assign_source_148 = PyDict_New();
        assert(tmp_class_creation_21__class_decl_dict == NULL);
        tmp_class_creation_21__class_decl_dict = tmp_assign_source_148;
    }
    {
        PyObject *tmp_assign_source_149;
        PyObject *tmp_metaclass_name_21;
        nuitka_bool tmp_condition_result_81;
        PyObject *tmp_key_name_61;
        PyObject *tmp_dict_arg_name_61;
        PyObject *tmp_dict_arg_name_62;
        PyObject *tmp_key_name_62;
        PyObject *tmp_type_arg_41;
        PyObject *tmp_expression_name_101;
        PyObject *tmp_subscript_name_21;
        PyObject *tmp_bases_name_21;
        tmp_key_name_61 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_21__class_decl_dict);
        tmp_dict_arg_name_61 = tmp_class_creation_21__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_61, tmp_key_name_61);
        assert(!(tmp_res == -1));
        tmp_condition_result_81 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_81 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_21;
        } else {
            goto condexpr_false_21;
        }
        condexpr_true_21:;
        CHECK_OBJECT(tmp_class_creation_21__class_decl_dict);
        tmp_dict_arg_name_62 = tmp_class_creation_21__class_decl_dict;
        tmp_key_name_62 = mod_consts[49];
        tmp_metaclass_name_21 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_62, tmp_key_name_62);
        if (tmp_metaclass_name_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        goto condexpr_end_21;
        condexpr_false_21:;
        CHECK_OBJECT(tmp_class_creation_21__bases);
        tmp_expression_name_101 = tmp_class_creation_21__bases;
        tmp_subscript_name_21 = mod_consts[46];
        tmp_type_arg_41 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_101, tmp_subscript_name_21, 0);
        if (tmp_type_arg_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        tmp_metaclass_name_21 = BUILTIN_TYPE1(tmp_type_arg_41);
        Py_DECREF(tmp_type_arg_41);
        if (tmp_metaclass_name_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        condexpr_end_21:;
        CHECK_OBJECT(tmp_class_creation_21__bases);
        tmp_bases_name_21 = tmp_class_creation_21__bases;
        tmp_assign_source_149 = SELECT_METACLASS(tmp_metaclass_name_21, tmp_bases_name_21);
        Py_DECREF(tmp_metaclass_name_21);
        if (tmp_assign_source_149 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        assert(tmp_class_creation_21__metaclass == NULL);
        tmp_class_creation_21__metaclass = tmp_assign_source_149;
    }
    {
        nuitka_bool tmp_condition_result_82;
        PyObject *tmp_key_name_63;
        PyObject *tmp_dict_arg_name_63;
        tmp_key_name_63 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_21__class_decl_dict);
        tmp_dict_arg_name_63 = tmp_class_creation_21__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_63, tmp_key_name_63);
        assert(!(tmp_res == -1));
        tmp_condition_result_82 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_82 == NUITKA_BOOL_TRUE) {
            goto branch_yes_61;
        } else {
            goto branch_no_61;
        }
    }
    branch_yes_61:;
    CHECK_OBJECT(tmp_class_creation_21__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_21__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 178;

        goto try_except_handler_61;
    }
    branch_no_61:;
    {
        nuitka_bool tmp_condition_result_83;
        PyObject *tmp_expression_name_102;
        CHECK_OBJECT(tmp_class_creation_21__metaclass);
        tmp_expression_name_102 = tmp_class_creation_21__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_102, mod_consts[50]);
        tmp_condition_result_83 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_83 == NUITKA_BOOL_TRUE) {
            goto branch_yes_62;
        } else {
            goto branch_no_62;
        }
    }
    branch_yes_62:;
    {
        PyObject *tmp_assign_source_150;
        PyObject *tmp_called_name_41;
        PyObject *tmp_expression_name_103;
        PyObject *tmp_args_name_41;
        PyObject *tmp_tuple_element_81;
        PyObject *tmp_kwargs_name_41;
        CHECK_OBJECT(tmp_class_creation_21__metaclass);
        tmp_expression_name_103 = tmp_class_creation_21__metaclass;
        tmp_called_name_41 = LOOKUP_ATTRIBUTE(tmp_expression_name_103, mod_consts[50]);
        if (tmp_called_name_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        tmp_tuple_element_81 = mod_consts[107];
        tmp_args_name_41 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_41, 0, tmp_tuple_element_81);
        CHECK_OBJECT(tmp_class_creation_21__bases);
        tmp_tuple_element_81 = tmp_class_creation_21__bases;
        PyTuple_SET_ITEM0(tmp_args_name_41, 1, tmp_tuple_element_81);
        CHECK_OBJECT(tmp_class_creation_21__class_decl_dict);
        tmp_kwargs_name_41 = tmp_class_creation_21__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 178;
        tmp_assign_source_150 = CALL_FUNCTION(tmp_called_name_41, tmp_args_name_41, tmp_kwargs_name_41);
        Py_DECREF(tmp_called_name_41);
        Py_DECREF(tmp_args_name_41);
        if (tmp_assign_source_150 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        assert(tmp_class_creation_21__prepared == NULL);
        tmp_class_creation_21__prepared = tmp_assign_source_150;
    }
    {
        nuitka_bool tmp_condition_result_84;
        PyObject *tmp_operand_name_21;
        PyObject *tmp_expression_name_104;
        CHECK_OBJECT(tmp_class_creation_21__prepared);
        tmp_expression_name_104 = tmp_class_creation_21__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_104, mod_consts[51]);
        tmp_operand_name_21 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_21);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        tmp_condition_result_84 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_84 == NUITKA_BOOL_TRUE) {
            goto branch_yes_63;
        } else {
            goto branch_no_63;
        }
    }
    branch_yes_63:;
    {
        PyObject *tmp_raise_type_21;
        PyObject *tmp_raise_value_21;
        PyObject *tmp_left_name_21;
        PyObject *tmp_right_name_21;
        PyObject *tmp_tuple_element_82;
        PyObject *tmp_getattr_target_21;
        PyObject *tmp_getattr_attr_21;
        PyObject *tmp_getattr_default_21;
        tmp_raise_type_21 = PyExc_TypeError;
        tmp_left_name_21 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_21__metaclass);
        tmp_getattr_target_21 = tmp_class_creation_21__metaclass;
        tmp_getattr_attr_21 = mod_consts[53];
        tmp_getattr_default_21 = mod_consts[54];
        tmp_tuple_element_82 = BUILTIN_GETATTR(tmp_getattr_target_21, tmp_getattr_attr_21, tmp_getattr_default_21);
        if (tmp_tuple_element_82 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        tmp_right_name_21 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_105;
            PyObject *tmp_type_arg_42;
            PyTuple_SET_ITEM(tmp_right_name_21, 0, tmp_tuple_element_82);
            CHECK_OBJECT(tmp_class_creation_21__prepared);
            tmp_type_arg_42 = tmp_class_creation_21__prepared;
            tmp_expression_name_105 = BUILTIN_TYPE1(tmp_type_arg_42);
            assert(!(tmp_expression_name_105 == NULL));
            tmp_tuple_element_82 = LOOKUP_ATTRIBUTE(tmp_expression_name_105, mod_consts[53]);
            Py_DECREF(tmp_expression_name_105);
            if (tmp_tuple_element_82 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 178;

                goto tuple_build_exception_24;
            }
            PyTuple_SET_ITEM(tmp_right_name_21, 1, tmp_tuple_element_82);
        }
        goto tuple_build_noexception_24;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_24:;
        Py_DECREF(tmp_right_name_21);
        goto try_except_handler_61;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_24:;
        tmp_raise_value_21 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_21, tmp_right_name_21);
        Py_DECREF(tmp_right_name_21);
        if (tmp_raise_value_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_61;
        }
        exception_type = tmp_raise_type_21;
        Py_INCREF(tmp_raise_type_21);
        exception_value = tmp_raise_value_21;
        exception_lineno = 178;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_61;
    }
    branch_no_63:;
    goto branch_end_62;
    branch_no_62:;
    {
        PyObject *tmp_assign_source_151;
        tmp_assign_source_151 = PyDict_New();
        assert(tmp_class_creation_21__prepared == NULL);
        tmp_class_creation_21__prepared = tmp_assign_source_151;
    }
    branch_end_62:;
    {
        PyObject *tmp_assign_source_152;
        {
            PyObject *tmp_set_locals_21;
            CHECK_OBJECT(tmp_class_creation_21__prepared);
            tmp_set_locals_21 = tmp_class_creation_21__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178 = tmp_set_locals_21;
            Py_INCREF(tmp_set_locals_21);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_63;
        }
        tmp_dictset_value = mod_consts[108];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_63;
        }
        tmp_dictset_value = mod_consts[107];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 178;

            goto try_except_handler_63;
        }
        if (isFrameUnusable(cache_frame_91103067dcd586dfb73a9300382a2bf7_9)) {
            Py_XDECREF(cache_frame_91103067dcd586dfb73a9300382a2bf7_9);

#if _DEBUG_REFCOUNTS
            if (cache_frame_91103067dcd586dfb73a9300382a2bf7_9 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_91103067dcd586dfb73a9300382a2bf7_9 = MAKE_FUNCTION_FRAME(codeobj_91103067dcd586dfb73a9300382a2bf7, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_91103067dcd586dfb73a9300382a2bf7_9->m_type_description == NULL);
        frame_91103067dcd586dfb73a9300382a2bf7_9 = cache_frame_91103067dcd586dfb73a9300382a2bf7_9;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_91103067dcd586dfb73a9300382a2bf7_9);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_91103067dcd586dfb73a9300382a2bf7_9) == 2); // Frame stack

        // Framed code:
        tmp_dictset_value = mod_consts[109];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178, mod_consts[110], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 181;
            type_description_2 = "o";
            goto frame_exception_exit_9;
        }
        tmp_dictset_value = mod_consts[111];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178, mod_consts[112], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 182;
            type_description_2 = "o";
            goto frame_exception_exit_9;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_91103067dcd586dfb73a9300382a2bf7_9);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_8;

        frame_exception_exit_9:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_91103067dcd586dfb73a9300382a2bf7_9);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_91103067dcd586dfb73a9300382a2bf7_9, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_91103067dcd586dfb73a9300382a2bf7_9->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_91103067dcd586dfb73a9300382a2bf7_9, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_91103067dcd586dfb73a9300382a2bf7_9,
            type_description_2,
            outline_20_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_91103067dcd586dfb73a9300382a2bf7_9 == cache_frame_91103067dcd586dfb73a9300382a2bf7_9) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_91103067dcd586dfb73a9300382a2bf7_9);
            cache_frame_91103067dcd586dfb73a9300382a2bf7_9 = NULL;
        }

        assertFrameObject(frame_91103067dcd586dfb73a9300382a2bf7_9);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_8;

        frame_no_exception_8:;
        goto skip_nested_handling_8;
        nested_frame_exit_8:;

        goto try_except_handler_63;
        skip_nested_handling_8:;
        {
            PyObject *tmp_assign_source_153;
            PyObject *tmp_called_name_42;
            PyObject *tmp_args_name_42;
            PyObject *tmp_tuple_element_83;
            PyObject *tmp_kwargs_name_42;
            CHECK_OBJECT(tmp_class_creation_21__metaclass);
            tmp_called_name_42 = tmp_class_creation_21__metaclass;
            tmp_tuple_element_83 = mod_consts[107];
            tmp_args_name_42 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_42, 0, tmp_tuple_element_83);
            CHECK_OBJECT(tmp_class_creation_21__bases);
            tmp_tuple_element_83 = tmp_class_creation_21__bases;
            PyTuple_SET_ITEM0(tmp_args_name_42, 1, tmp_tuple_element_83);
            tmp_tuple_element_83 = locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178;
            PyTuple_SET_ITEM0(tmp_args_name_42, 2, tmp_tuple_element_83);
            CHECK_OBJECT(tmp_class_creation_21__class_decl_dict);
            tmp_kwargs_name_42 = tmp_class_creation_21__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 178;
            tmp_assign_source_153 = CALL_FUNCTION(tmp_called_name_42, tmp_args_name_42, tmp_kwargs_name_42);
            Py_DECREF(tmp_args_name_42);
            if (tmp_assign_source_153 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 178;

                goto try_except_handler_63;
            }
            assert(outline_20_var___class__ == NULL);
            outline_20_var___class__ = tmp_assign_source_153;
        }
        CHECK_OBJECT(outline_20_var___class__);
        tmp_assign_source_152 = outline_20_var___class__;
        Py_INCREF(tmp_assign_source_152);
        goto try_return_handler_63;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_63:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178);
        locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178 = NULL;
        goto try_return_handler_62;
        // Exception handler code:
        try_except_handler_63:;
        exception_keeper_type_61 = exception_type;
        exception_keeper_value_61 = exception_value;
        exception_keeper_tb_61 = exception_tb;
        exception_keeper_lineno_61 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178);
        locals_pip$_vendor$urllib3$exceptions$$$class__21_ResponseError_178 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_61;
        exception_value = exception_keeper_value_61;
        exception_tb = exception_keeper_tb_61;
        exception_lineno = exception_keeper_lineno_61;

        goto try_except_handler_62;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_62:;
        CHECK_OBJECT(outline_20_var___class__);
        Py_DECREF(outline_20_var___class__);
        outline_20_var___class__ = NULL;
        goto outline_result_21;
        // Exception handler code:
        try_except_handler_62:;
        exception_keeper_type_62 = exception_type;
        exception_keeper_value_62 = exception_value;
        exception_keeper_tb_62 = exception_tb;
        exception_keeper_lineno_62 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_62;
        exception_value = exception_keeper_value_62;
        exception_tb = exception_keeper_tb_62;
        exception_lineno = exception_keeper_lineno_62;

        goto outline_exception_21;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_21:;
        exception_lineno = 178;
        goto try_except_handler_61;
        outline_result_21:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[107], tmp_assign_source_152);
    }
    goto try_end_21;
    // Exception handler code:
    try_except_handler_61:;
    exception_keeper_type_63 = exception_type;
    exception_keeper_value_63 = exception_value;
    exception_keeper_tb_63 = exception_tb;
    exception_keeper_lineno_63 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_21__bases);
    tmp_class_creation_21__bases = NULL;
    Py_XDECREF(tmp_class_creation_21__class_decl_dict);
    tmp_class_creation_21__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_21__metaclass);
    tmp_class_creation_21__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_21__prepared);
    tmp_class_creation_21__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_63;
    exception_value = exception_keeper_value_63;
    exception_tb = exception_keeper_tb_63;
    exception_lineno = exception_keeper_lineno_63;

    goto frame_exception_exit_1;
    // End of try:
    try_end_21:;
    Py_XDECREF(tmp_class_creation_21__bases);
    tmp_class_creation_21__bases = NULL;
    Py_XDECREF(tmp_class_creation_21__class_decl_dict);
    tmp_class_creation_21__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_21__metaclass);
    tmp_class_creation_21__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_21__prepared);
    Py_DECREF(tmp_class_creation_21__prepared);
    tmp_class_creation_21__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_154;
        PyObject *tmp_tuple_element_84;
        tmp_tuple_element_84 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[60]);

        if (unlikely(tmp_tuple_element_84 == NULL)) {
            tmp_tuple_element_84 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[60]);
        }

        if (tmp_tuple_element_84 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        tmp_assign_source_154 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_154, 0, tmp_tuple_element_84);
        assert(tmp_class_creation_22__bases == NULL);
        tmp_class_creation_22__bases = tmp_assign_source_154;
    }
    {
        PyObject *tmp_assign_source_155;
        tmp_assign_source_155 = PyDict_New();
        assert(tmp_class_creation_22__class_decl_dict == NULL);
        tmp_class_creation_22__class_decl_dict = tmp_assign_source_155;
    }
    {
        PyObject *tmp_assign_source_156;
        PyObject *tmp_metaclass_name_22;
        nuitka_bool tmp_condition_result_85;
        PyObject *tmp_key_name_64;
        PyObject *tmp_dict_arg_name_64;
        PyObject *tmp_dict_arg_name_65;
        PyObject *tmp_key_name_65;
        PyObject *tmp_type_arg_43;
        PyObject *tmp_expression_name_106;
        PyObject *tmp_subscript_name_22;
        PyObject *tmp_bases_name_22;
        tmp_key_name_64 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_22__class_decl_dict);
        tmp_dict_arg_name_64 = tmp_class_creation_22__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_64, tmp_key_name_64);
        assert(!(tmp_res == -1));
        tmp_condition_result_85 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_85 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_22;
        } else {
            goto condexpr_false_22;
        }
        condexpr_true_22:;
        CHECK_OBJECT(tmp_class_creation_22__class_decl_dict);
        tmp_dict_arg_name_65 = tmp_class_creation_22__class_decl_dict;
        tmp_key_name_65 = mod_consts[49];
        tmp_metaclass_name_22 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_65, tmp_key_name_65);
        if (tmp_metaclass_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        goto condexpr_end_22;
        condexpr_false_22:;
        CHECK_OBJECT(tmp_class_creation_22__bases);
        tmp_expression_name_106 = tmp_class_creation_22__bases;
        tmp_subscript_name_22 = mod_consts[46];
        tmp_type_arg_43 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_106, tmp_subscript_name_22, 0);
        if (tmp_type_arg_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        tmp_metaclass_name_22 = BUILTIN_TYPE1(tmp_type_arg_43);
        Py_DECREF(tmp_type_arg_43);
        if (tmp_metaclass_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        condexpr_end_22:;
        CHECK_OBJECT(tmp_class_creation_22__bases);
        tmp_bases_name_22 = tmp_class_creation_22__bases;
        tmp_assign_source_156 = SELECT_METACLASS(tmp_metaclass_name_22, tmp_bases_name_22);
        Py_DECREF(tmp_metaclass_name_22);
        if (tmp_assign_source_156 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        assert(tmp_class_creation_22__metaclass == NULL);
        tmp_class_creation_22__metaclass = tmp_assign_source_156;
    }
    {
        nuitka_bool tmp_condition_result_86;
        PyObject *tmp_key_name_66;
        PyObject *tmp_dict_arg_name_66;
        tmp_key_name_66 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_22__class_decl_dict);
        tmp_dict_arg_name_66 = tmp_class_creation_22__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_66, tmp_key_name_66);
        assert(!(tmp_res == -1));
        tmp_condition_result_86 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_86 == NUITKA_BOOL_TRUE) {
            goto branch_yes_64;
        } else {
            goto branch_no_64;
        }
    }
    branch_yes_64:;
    CHECK_OBJECT(tmp_class_creation_22__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_22__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 185;

        goto try_except_handler_64;
    }
    branch_no_64:;
    {
        nuitka_bool tmp_condition_result_87;
        PyObject *tmp_expression_name_107;
        CHECK_OBJECT(tmp_class_creation_22__metaclass);
        tmp_expression_name_107 = tmp_class_creation_22__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_107, mod_consts[50]);
        tmp_condition_result_87 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_87 == NUITKA_BOOL_TRUE) {
            goto branch_yes_65;
        } else {
            goto branch_no_65;
        }
    }
    branch_yes_65:;
    {
        PyObject *tmp_assign_source_157;
        PyObject *tmp_called_name_43;
        PyObject *tmp_expression_name_108;
        PyObject *tmp_args_name_43;
        PyObject *tmp_tuple_element_85;
        PyObject *tmp_kwargs_name_43;
        CHECK_OBJECT(tmp_class_creation_22__metaclass);
        tmp_expression_name_108 = tmp_class_creation_22__metaclass;
        tmp_called_name_43 = LOOKUP_ATTRIBUTE(tmp_expression_name_108, mod_consts[50]);
        if (tmp_called_name_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        tmp_tuple_element_85 = mod_consts[113];
        tmp_args_name_43 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_43, 0, tmp_tuple_element_85);
        CHECK_OBJECT(tmp_class_creation_22__bases);
        tmp_tuple_element_85 = tmp_class_creation_22__bases;
        PyTuple_SET_ITEM0(tmp_args_name_43, 1, tmp_tuple_element_85);
        CHECK_OBJECT(tmp_class_creation_22__class_decl_dict);
        tmp_kwargs_name_43 = tmp_class_creation_22__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 185;
        tmp_assign_source_157 = CALL_FUNCTION(tmp_called_name_43, tmp_args_name_43, tmp_kwargs_name_43);
        Py_DECREF(tmp_called_name_43);
        Py_DECREF(tmp_args_name_43);
        if (tmp_assign_source_157 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        assert(tmp_class_creation_22__prepared == NULL);
        tmp_class_creation_22__prepared = tmp_assign_source_157;
    }
    {
        nuitka_bool tmp_condition_result_88;
        PyObject *tmp_operand_name_22;
        PyObject *tmp_expression_name_109;
        CHECK_OBJECT(tmp_class_creation_22__prepared);
        tmp_expression_name_109 = tmp_class_creation_22__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_109, mod_consts[51]);
        tmp_operand_name_22 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_22);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        tmp_condition_result_88 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_88 == NUITKA_BOOL_TRUE) {
            goto branch_yes_66;
        } else {
            goto branch_no_66;
        }
    }
    branch_yes_66:;
    {
        PyObject *tmp_raise_type_22;
        PyObject *tmp_raise_value_22;
        PyObject *tmp_left_name_22;
        PyObject *tmp_right_name_22;
        PyObject *tmp_tuple_element_86;
        PyObject *tmp_getattr_target_22;
        PyObject *tmp_getattr_attr_22;
        PyObject *tmp_getattr_default_22;
        tmp_raise_type_22 = PyExc_TypeError;
        tmp_left_name_22 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_22__metaclass);
        tmp_getattr_target_22 = tmp_class_creation_22__metaclass;
        tmp_getattr_attr_22 = mod_consts[53];
        tmp_getattr_default_22 = mod_consts[54];
        tmp_tuple_element_86 = BUILTIN_GETATTR(tmp_getattr_target_22, tmp_getattr_attr_22, tmp_getattr_default_22);
        if (tmp_tuple_element_86 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        tmp_right_name_22 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_110;
            PyObject *tmp_type_arg_44;
            PyTuple_SET_ITEM(tmp_right_name_22, 0, tmp_tuple_element_86);
            CHECK_OBJECT(tmp_class_creation_22__prepared);
            tmp_type_arg_44 = tmp_class_creation_22__prepared;
            tmp_expression_name_110 = BUILTIN_TYPE1(tmp_type_arg_44);
            assert(!(tmp_expression_name_110 == NULL));
            tmp_tuple_element_86 = LOOKUP_ATTRIBUTE(tmp_expression_name_110, mod_consts[53]);
            Py_DECREF(tmp_expression_name_110);
            if (tmp_tuple_element_86 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 185;

                goto tuple_build_exception_25;
            }
            PyTuple_SET_ITEM(tmp_right_name_22, 1, tmp_tuple_element_86);
        }
        goto tuple_build_noexception_25;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_25:;
        Py_DECREF(tmp_right_name_22);
        goto try_except_handler_64;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_25:;
        tmp_raise_value_22 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_22, tmp_right_name_22);
        Py_DECREF(tmp_right_name_22);
        if (tmp_raise_value_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_64;
        }
        exception_type = tmp_raise_type_22;
        Py_INCREF(tmp_raise_type_22);
        exception_value = tmp_raise_value_22;
        exception_lineno = 185;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_64;
    }
    branch_no_66:;
    goto branch_end_65;
    branch_no_65:;
    {
        PyObject *tmp_assign_source_158;
        tmp_assign_source_158 = PyDict_New();
        assert(tmp_class_creation_22__prepared == NULL);
        tmp_class_creation_22__prepared = tmp_assign_source_158;
    }
    branch_end_65:;
    {
        PyObject *tmp_assign_source_159;
        {
            PyObject *tmp_set_locals_22;
            CHECK_OBJECT(tmp_class_creation_22__prepared);
            tmp_set_locals_22 = tmp_class_creation_22__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185 = tmp_set_locals_22;
            Py_INCREF(tmp_set_locals_22);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_66;
        }
        tmp_dictset_value = mod_consts[114];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_66;
        }
        tmp_dictset_value = mod_consts[113];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 185;

            goto try_except_handler_66;
        }
        {
            PyObject *tmp_assign_source_160;
            PyObject *tmp_called_name_44;
            PyObject *tmp_args_name_44;
            PyObject *tmp_tuple_element_87;
            PyObject *tmp_kwargs_name_44;
            CHECK_OBJECT(tmp_class_creation_22__metaclass);
            tmp_called_name_44 = tmp_class_creation_22__metaclass;
            tmp_tuple_element_87 = mod_consts[113];
            tmp_args_name_44 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_44, 0, tmp_tuple_element_87);
            CHECK_OBJECT(tmp_class_creation_22__bases);
            tmp_tuple_element_87 = tmp_class_creation_22__bases;
            PyTuple_SET_ITEM0(tmp_args_name_44, 1, tmp_tuple_element_87);
            tmp_tuple_element_87 = locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185;
            PyTuple_SET_ITEM0(tmp_args_name_44, 2, tmp_tuple_element_87);
            CHECK_OBJECT(tmp_class_creation_22__class_decl_dict);
            tmp_kwargs_name_44 = tmp_class_creation_22__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 185;
            tmp_assign_source_160 = CALL_FUNCTION(tmp_called_name_44, tmp_args_name_44, tmp_kwargs_name_44);
            Py_DECREF(tmp_args_name_44);
            if (tmp_assign_source_160 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 185;

                goto try_except_handler_66;
            }
            assert(outline_21_var___class__ == NULL);
            outline_21_var___class__ = tmp_assign_source_160;
        }
        CHECK_OBJECT(outline_21_var___class__);
        tmp_assign_source_159 = outline_21_var___class__;
        Py_INCREF(tmp_assign_source_159);
        goto try_return_handler_66;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_66:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185);
        locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185 = NULL;
        goto try_return_handler_65;
        // Exception handler code:
        try_except_handler_66:;
        exception_keeper_type_64 = exception_type;
        exception_keeper_value_64 = exception_value;
        exception_keeper_tb_64 = exception_tb;
        exception_keeper_lineno_64 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185);
        locals_pip$_vendor$urllib3$exceptions$$$class__22_SecurityWarning_185 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_64;
        exception_value = exception_keeper_value_64;
        exception_tb = exception_keeper_tb_64;
        exception_lineno = exception_keeper_lineno_64;

        goto try_except_handler_65;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_65:;
        CHECK_OBJECT(outline_21_var___class__);
        Py_DECREF(outline_21_var___class__);
        outline_21_var___class__ = NULL;
        goto outline_result_22;
        // Exception handler code:
        try_except_handler_65:;
        exception_keeper_type_65 = exception_type;
        exception_keeper_value_65 = exception_value;
        exception_keeper_tb_65 = exception_tb;
        exception_keeper_lineno_65 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_65;
        exception_value = exception_keeper_value_65;
        exception_tb = exception_keeper_tb_65;
        exception_lineno = exception_keeper_lineno_65;

        goto outline_exception_22;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_22:;
        exception_lineno = 185;
        goto try_except_handler_64;
        outline_result_22:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[113], tmp_assign_source_159);
    }
    goto try_end_22;
    // Exception handler code:
    try_except_handler_64:;
    exception_keeper_type_66 = exception_type;
    exception_keeper_value_66 = exception_value;
    exception_keeper_tb_66 = exception_tb;
    exception_keeper_lineno_66 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_22__bases);
    tmp_class_creation_22__bases = NULL;
    Py_XDECREF(tmp_class_creation_22__class_decl_dict);
    tmp_class_creation_22__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_22__metaclass);
    tmp_class_creation_22__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_22__prepared);
    tmp_class_creation_22__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_66;
    exception_value = exception_keeper_value_66;
    exception_tb = exception_keeper_tb_66;
    exception_lineno = exception_keeper_lineno_66;

    goto frame_exception_exit_1;
    // End of try:
    try_end_22:;
    Py_XDECREF(tmp_class_creation_22__bases);
    tmp_class_creation_22__bases = NULL;
    Py_XDECREF(tmp_class_creation_22__class_decl_dict);
    tmp_class_creation_22__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_22__metaclass);
    tmp_class_creation_22__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_22__prepared);
    Py_DECREF(tmp_class_creation_22__prepared);
    tmp_class_creation_22__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_161;
        PyObject *tmp_tuple_element_88;
        tmp_tuple_element_88 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[113]);

        if (unlikely(tmp_tuple_element_88 == NULL)) {
            tmp_tuple_element_88 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[113]);
        }

        if (tmp_tuple_element_88 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        tmp_assign_source_161 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_161, 0, tmp_tuple_element_88);
        assert(tmp_class_creation_23__bases == NULL);
        tmp_class_creation_23__bases = tmp_assign_source_161;
    }
    {
        PyObject *tmp_assign_source_162;
        tmp_assign_source_162 = PyDict_New();
        assert(tmp_class_creation_23__class_decl_dict == NULL);
        tmp_class_creation_23__class_decl_dict = tmp_assign_source_162;
    }
    {
        PyObject *tmp_assign_source_163;
        PyObject *tmp_metaclass_name_23;
        nuitka_bool tmp_condition_result_89;
        PyObject *tmp_key_name_67;
        PyObject *tmp_dict_arg_name_67;
        PyObject *tmp_dict_arg_name_68;
        PyObject *tmp_key_name_68;
        PyObject *tmp_type_arg_45;
        PyObject *tmp_expression_name_111;
        PyObject *tmp_subscript_name_23;
        PyObject *tmp_bases_name_23;
        tmp_key_name_67 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_23__class_decl_dict);
        tmp_dict_arg_name_67 = tmp_class_creation_23__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_67, tmp_key_name_67);
        assert(!(tmp_res == -1));
        tmp_condition_result_89 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_89 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_23;
        } else {
            goto condexpr_false_23;
        }
        condexpr_true_23:;
        CHECK_OBJECT(tmp_class_creation_23__class_decl_dict);
        tmp_dict_arg_name_68 = tmp_class_creation_23__class_decl_dict;
        tmp_key_name_68 = mod_consts[49];
        tmp_metaclass_name_23 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_68, tmp_key_name_68);
        if (tmp_metaclass_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        goto condexpr_end_23;
        condexpr_false_23:;
        CHECK_OBJECT(tmp_class_creation_23__bases);
        tmp_expression_name_111 = tmp_class_creation_23__bases;
        tmp_subscript_name_23 = mod_consts[46];
        tmp_type_arg_45 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_111, tmp_subscript_name_23, 0);
        if (tmp_type_arg_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        tmp_metaclass_name_23 = BUILTIN_TYPE1(tmp_type_arg_45);
        Py_DECREF(tmp_type_arg_45);
        if (tmp_metaclass_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        condexpr_end_23:;
        CHECK_OBJECT(tmp_class_creation_23__bases);
        tmp_bases_name_23 = tmp_class_creation_23__bases;
        tmp_assign_source_163 = SELECT_METACLASS(tmp_metaclass_name_23, tmp_bases_name_23);
        Py_DECREF(tmp_metaclass_name_23);
        if (tmp_assign_source_163 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        assert(tmp_class_creation_23__metaclass == NULL);
        tmp_class_creation_23__metaclass = tmp_assign_source_163;
    }
    {
        nuitka_bool tmp_condition_result_90;
        PyObject *tmp_key_name_69;
        PyObject *tmp_dict_arg_name_69;
        tmp_key_name_69 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_23__class_decl_dict);
        tmp_dict_arg_name_69 = tmp_class_creation_23__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_69, tmp_key_name_69);
        assert(!(tmp_res == -1));
        tmp_condition_result_90 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_90 == NUITKA_BOOL_TRUE) {
            goto branch_yes_67;
        } else {
            goto branch_no_67;
        }
    }
    branch_yes_67:;
    CHECK_OBJECT(tmp_class_creation_23__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_23__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 191;

        goto try_except_handler_67;
    }
    branch_no_67:;
    {
        nuitka_bool tmp_condition_result_91;
        PyObject *tmp_expression_name_112;
        CHECK_OBJECT(tmp_class_creation_23__metaclass);
        tmp_expression_name_112 = tmp_class_creation_23__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_112, mod_consts[50]);
        tmp_condition_result_91 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_91 == NUITKA_BOOL_TRUE) {
            goto branch_yes_68;
        } else {
            goto branch_no_68;
        }
    }
    branch_yes_68:;
    {
        PyObject *tmp_assign_source_164;
        PyObject *tmp_called_name_45;
        PyObject *tmp_expression_name_113;
        PyObject *tmp_args_name_45;
        PyObject *tmp_tuple_element_89;
        PyObject *tmp_kwargs_name_45;
        CHECK_OBJECT(tmp_class_creation_23__metaclass);
        tmp_expression_name_113 = tmp_class_creation_23__metaclass;
        tmp_called_name_45 = LOOKUP_ATTRIBUTE(tmp_expression_name_113, mod_consts[50]);
        if (tmp_called_name_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        tmp_tuple_element_89 = mod_consts[115];
        tmp_args_name_45 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_45, 0, tmp_tuple_element_89);
        CHECK_OBJECT(tmp_class_creation_23__bases);
        tmp_tuple_element_89 = tmp_class_creation_23__bases;
        PyTuple_SET_ITEM0(tmp_args_name_45, 1, tmp_tuple_element_89);
        CHECK_OBJECT(tmp_class_creation_23__class_decl_dict);
        tmp_kwargs_name_45 = tmp_class_creation_23__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 191;
        tmp_assign_source_164 = CALL_FUNCTION(tmp_called_name_45, tmp_args_name_45, tmp_kwargs_name_45);
        Py_DECREF(tmp_called_name_45);
        Py_DECREF(tmp_args_name_45);
        if (tmp_assign_source_164 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        assert(tmp_class_creation_23__prepared == NULL);
        tmp_class_creation_23__prepared = tmp_assign_source_164;
    }
    {
        nuitka_bool tmp_condition_result_92;
        PyObject *tmp_operand_name_23;
        PyObject *tmp_expression_name_114;
        CHECK_OBJECT(tmp_class_creation_23__prepared);
        tmp_expression_name_114 = tmp_class_creation_23__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_114, mod_consts[51]);
        tmp_operand_name_23 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_23);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        tmp_condition_result_92 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_92 == NUITKA_BOOL_TRUE) {
            goto branch_yes_69;
        } else {
            goto branch_no_69;
        }
    }
    branch_yes_69:;
    {
        PyObject *tmp_raise_type_23;
        PyObject *tmp_raise_value_23;
        PyObject *tmp_left_name_23;
        PyObject *tmp_right_name_23;
        PyObject *tmp_tuple_element_90;
        PyObject *tmp_getattr_target_23;
        PyObject *tmp_getattr_attr_23;
        PyObject *tmp_getattr_default_23;
        tmp_raise_type_23 = PyExc_TypeError;
        tmp_left_name_23 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_23__metaclass);
        tmp_getattr_target_23 = tmp_class_creation_23__metaclass;
        tmp_getattr_attr_23 = mod_consts[53];
        tmp_getattr_default_23 = mod_consts[54];
        tmp_tuple_element_90 = BUILTIN_GETATTR(tmp_getattr_target_23, tmp_getattr_attr_23, tmp_getattr_default_23);
        if (tmp_tuple_element_90 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        tmp_right_name_23 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_115;
            PyObject *tmp_type_arg_46;
            PyTuple_SET_ITEM(tmp_right_name_23, 0, tmp_tuple_element_90);
            CHECK_OBJECT(tmp_class_creation_23__prepared);
            tmp_type_arg_46 = tmp_class_creation_23__prepared;
            tmp_expression_name_115 = BUILTIN_TYPE1(tmp_type_arg_46);
            assert(!(tmp_expression_name_115 == NULL));
            tmp_tuple_element_90 = LOOKUP_ATTRIBUTE(tmp_expression_name_115, mod_consts[53]);
            Py_DECREF(tmp_expression_name_115);
            if (tmp_tuple_element_90 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 191;

                goto tuple_build_exception_26;
            }
            PyTuple_SET_ITEM(tmp_right_name_23, 1, tmp_tuple_element_90);
        }
        goto tuple_build_noexception_26;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_26:;
        Py_DECREF(tmp_right_name_23);
        goto try_except_handler_67;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_26:;
        tmp_raise_value_23 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_23, tmp_right_name_23);
        Py_DECREF(tmp_right_name_23);
        if (tmp_raise_value_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_67;
        }
        exception_type = tmp_raise_type_23;
        Py_INCREF(tmp_raise_type_23);
        exception_value = tmp_raise_value_23;
        exception_lineno = 191;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_67;
    }
    branch_no_69:;
    goto branch_end_68;
    branch_no_68:;
    {
        PyObject *tmp_assign_source_165;
        tmp_assign_source_165 = PyDict_New();
        assert(tmp_class_creation_23__prepared == NULL);
        tmp_class_creation_23__prepared = tmp_assign_source_165;
    }
    branch_end_68:;
    {
        PyObject *tmp_assign_source_166;
        {
            PyObject *tmp_set_locals_23;
            CHECK_OBJECT(tmp_class_creation_23__prepared);
            tmp_set_locals_23 = tmp_class_creation_23__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191 = tmp_set_locals_23;
            Py_INCREF(tmp_set_locals_23);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_69;
        }
        tmp_dictset_value = mod_consts[116];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_69;
        }
        tmp_dictset_value = mod_consts[115];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 191;

            goto try_except_handler_69;
        }
        {
            PyObject *tmp_assign_source_167;
            PyObject *tmp_called_name_46;
            PyObject *tmp_args_name_46;
            PyObject *tmp_tuple_element_91;
            PyObject *tmp_kwargs_name_46;
            CHECK_OBJECT(tmp_class_creation_23__metaclass);
            tmp_called_name_46 = tmp_class_creation_23__metaclass;
            tmp_tuple_element_91 = mod_consts[115];
            tmp_args_name_46 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_46, 0, tmp_tuple_element_91);
            CHECK_OBJECT(tmp_class_creation_23__bases);
            tmp_tuple_element_91 = tmp_class_creation_23__bases;
            PyTuple_SET_ITEM0(tmp_args_name_46, 1, tmp_tuple_element_91);
            tmp_tuple_element_91 = locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191;
            PyTuple_SET_ITEM0(tmp_args_name_46, 2, tmp_tuple_element_91);
            CHECK_OBJECT(tmp_class_creation_23__class_decl_dict);
            tmp_kwargs_name_46 = tmp_class_creation_23__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 191;
            tmp_assign_source_167 = CALL_FUNCTION(tmp_called_name_46, tmp_args_name_46, tmp_kwargs_name_46);
            Py_DECREF(tmp_args_name_46);
            if (tmp_assign_source_167 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 191;

                goto try_except_handler_69;
            }
            assert(outline_22_var___class__ == NULL);
            outline_22_var___class__ = tmp_assign_source_167;
        }
        CHECK_OBJECT(outline_22_var___class__);
        tmp_assign_source_166 = outline_22_var___class__;
        Py_INCREF(tmp_assign_source_166);
        goto try_return_handler_69;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_69:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191);
        locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191 = NULL;
        goto try_return_handler_68;
        // Exception handler code:
        try_except_handler_69:;
        exception_keeper_type_67 = exception_type;
        exception_keeper_value_67 = exception_value;
        exception_keeper_tb_67 = exception_tb;
        exception_keeper_lineno_67 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191);
        locals_pip$_vendor$urllib3$exceptions$$$class__23_SubjectAltNameWarning_191 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_67;
        exception_value = exception_keeper_value_67;
        exception_tb = exception_keeper_tb_67;
        exception_lineno = exception_keeper_lineno_67;

        goto try_except_handler_68;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_68:;
        CHECK_OBJECT(outline_22_var___class__);
        Py_DECREF(outline_22_var___class__);
        outline_22_var___class__ = NULL;
        goto outline_result_23;
        // Exception handler code:
        try_except_handler_68:;
        exception_keeper_type_68 = exception_type;
        exception_keeper_value_68 = exception_value;
        exception_keeper_tb_68 = exception_tb;
        exception_keeper_lineno_68 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_68;
        exception_value = exception_keeper_value_68;
        exception_tb = exception_keeper_tb_68;
        exception_lineno = exception_keeper_lineno_68;

        goto outline_exception_23;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_23:;
        exception_lineno = 191;
        goto try_except_handler_67;
        outline_result_23:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[115], tmp_assign_source_166);
    }
    goto try_end_23;
    // Exception handler code:
    try_except_handler_67:;
    exception_keeper_type_69 = exception_type;
    exception_keeper_value_69 = exception_value;
    exception_keeper_tb_69 = exception_tb;
    exception_keeper_lineno_69 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_23__bases);
    tmp_class_creation_23__bases = NULL;
    Py_XDECREF(tmp_class_creation_23__class_decl_dict);
    tmp_class_creation_23__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_23__metaclass);
    tmp_class_creation_23__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_23__prepared);
    tmp_class_creation_23__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_69;
    exception_value = exception_keeper_value_69;
    exception_tb = exception_keeper_tb_69;
    exception_lineno = exception_keeper_lineno_69;

    goto frame_exception_exit_1;
    // End of try:
    try_end_23:;
    Py_XDECREF(tmp_class_creation_23__bases);
    tmp_class_creation_23__bases = NULL;
    Py_XDECREF(tmp_class_creation_23__class_decl_dict);
    tmp_class_creation_23__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_23__metaclass);
    tmp_class_creation_23__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_23__prepared);
    Py_DECREF(tmp_class_creation_23__prepared);
    tmp_class_creation_23__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_168;
        PyObject *tmp_tuple_element_92;
        tmp_tuple_element_92 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[113]);

        if (unlikely(tmp_tuple_element_92 == NULL)) {
            tmp_tuple_element_92 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[113]);
        }

        if (tmp_tuple_element_92 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        tmp_assign_source_168 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_168, 0, tmp_tuple_element_92);
        assert(tmp_class_creation_24__bases == NULL);
        tmp_class_creation_24__bases = tmp_assign_source_168;
    }
    {
        PyObject *tmp_assign_source_169;
        tmp_assign_source_169 = PyDict_New();
        assert(tmp_class_creation_24__class_decl_dict == NULL);
        tmp_class_creation_24__class_decl_dict = tmp_assign_source_169;
    }
    {
        PyObject *tmp_assign_source_170;
        PyObject *tmp_metaclass_name_24;
        nuitka_bool tmp_condition_result_93;
        PyObject *tmp_key_name_70;
        PyObject *tmp_dict_arg_name_70;
        PyObject *tmp_dict_arg_name_71;
        PyObject *tmp_key_name_71;
        PyObject *tmp_type_arg_47;
        PyObject *tmp_expression_name_116;
        PyObject *tmp_subscript_name_24;
        PyObject *tmp_bases_name_24;
        tmp_key_name_70 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_24__class_decl_dict);
        tmp_dict_arg_name_70 = tmp_class_creation_24__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_70, tmp_key_name_70);
        assert(!(tmp_res == -1));
        tmp_condition_result_93 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_93 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_24;
        } else {
            goto condexpr_false_24;
        }
        condexpr_true_24:;
        CHECK_OBJECT(tmp_class_creation_24__class_decl_dict);
        tmp_dict_arg_name_71 = tmp_class_creation_24__class_decl_dict;
        tmp_key_name_71 = mod_consts[49];
        tmp_metaclass_name_24 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_71, tmp_key_name_71);
        if (tmp_metaclass_name_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        goto condexpr_end_24;
        condexpr_false_24:;
        CHECK_OBJECT(tmp_class_creation_24__bases);
        tmp_expression_name_116 = tmp_class_creation_24__bases;
        tmp_subscript_name_24 = mod_consts[46];
        tmp_type_arg_47 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_116, tmp_subscript_name_24, 0);
        if (tmp_type_arg_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        tmp_metaclass_name_24 = BUILTIN_TYPE1(tmp_type_arg_47);
        Py_DECREF(tmp_type_arg_47);
        if (tmp_metaclass_name_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        condexpr_end_24:;
        CHECK_OBJECT(tmp_class_creation_24__bases);
        tmp_bases_name_24 = tmp_class_creation_24__bases;
        tmp_assign_source_170 = SELECT_METACLASS(tmp_metaclass_name_24, tmp_bases_name_24);
        Py_DECREF(tmp_metaclass_name_24);
        if (tmp_assign_source_170 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        assert(tmp_class_creation_24__metaclass == NULL);
        tmp_class_creation_24__metaclass = tmp_assign_source_170;
    }
    {
        nuitka_bool tmp_condition_result_94;
        PyObject *tmp_key_name_72;
        PyObject *tmp_dict_arg_name_72;
        tmp_key_name_72 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_24__class_decl_dict);
        tmp_dict_arg_name_72 = tmp_class_creation_24__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_72, tmp_key_name_72);
        assert(!(tmp_res == -1));
        tmp_condition_result_94 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_94 == NUITKA_BOOL_TRUE) {
            goto branch_yes_70;
        } else {
            goto branch_no_70;
        }
    }
    branch_yes_70:;
    CHECK_OBJECT(tmp_class_creation_24__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_24__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 197;

        goto try_except_handler_70;
    }
    branch_no_70:;
    {
        nuitka_bool tmp_condition_result_95;
        PyObject *tmp_expression_name_117;
        CHECK_OBJECT(tmp_class_creation_24__metaclass);
        tmp_expression_name_117 = tmp_class_creation_24__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_117, mod_consts[50]);
        tmp_condition_result_95 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_95 == NUITKA_BOOL_TRUE) {
            goto branch_yes_71;
        } else {
            goto branch_no_71;
        }
    }
    branch_yes_71:;
    {
        PyObject *tmp_assign_source_171;
        PyObject *tmp_called_name_47;
        PyObject *tmp_expression_name_118;
        PyObject *tmp_args_name_47;
        PyObject *tmp_tuple_element_93;
        PyObject *tmp_kwargs_name_47;
        CHECK_OBJECT(tmp_class_creation_24__metaclass);
        tmp_expression_name_118 = tmp_class_creation_24__metaclass;
        tmp_called_name_47 = LOOKUP_ATTRIBUTE(tmp_expression_name_118, mod_consts[50]);
        if (tmp_called_name_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        tmp_tuple_element_93 = mod_consts[117];
        tmp_args_name_47 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_47, 0, tmp_tuple_element_93);
        CHECK_OBJECT(tmp_class_creation_24__bases);
        tmp_tuple_element_93 = tmp_class_creation_24__bases;
        PyTuple_SET_ITEM0(tmp_args_name_47, 1, tmp_tuple_element_93);
        CHECK_OBJECT(tmp_class_creation_24__class_decl_dict);
        tmp_kwargs_name_47 = tmp_class_creation_24__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 197;
        tmp_assign_source_171 = CALL_FUNCTION(tmp_called_name_47, tmp_args_name_47, tmp_kwargs_name_47);
        Py_DECREF(tmp_called_name_47);
        Py_DECREF(tmp_args_name_47);
        if (tmp_assign_source_171 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        assert(tmp_class_creation_24__prepared == NULL);
        tmp_class_creation_24__prepared = tmp_assign_source_171;
    }
    {
        nuitka_bool tmp_condition_result_96;
        PyObject *tmp_operand_name_24;
        PyObject *tmp_expression_name_119;
        CHECK_OBJECT(tmp_class_creation_24__prepared);
        tmp_expression_name_119 = tmp_class_creation_24__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_119, mod_consts[51]);
        tmp_operand_name_24 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_24);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        tmp_condition_result_96 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_96 == NUITKA_BOOL_TRUE) {
            goto branch_yes_72;
        } else {
            goto branch_no_72;
        }
    }
    branch_yes_72:;
    {
        PyObject *tmp_raise_type_24;
        PyObject *tmp_raise_value_24;
        PyObject *tmp_left_name_24;
        PyObject *tmp_right_name_24;
        PyObject *tmp_tuple_element_94;
        PyObject *tmp_getattr_target_24;
        PyObject *tmp_getattr_attr_24;
        PyObject *tmp_getattr_default_24;
        tmp_raise_type_24 = PyExc_TypeError;
        tmp_left_name_24 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_24__metaclass);
        tmp_getattr_target_24 = tmp_class_creation_24__metaclass;
        tmp_getattr_attr_24 = mod_consts[53];
        tmp_getattr_default_24 = mod_consts[54];
        tmp_tuple_element_94 = BUILTIN_GETATTR(tmp_getattr_target_24, tmp_getattr_attr_24, tmp_getattr_default_24);
        if (tmp_tuple_element_94 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        tmp_right_name_24 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_120;
            PyObject *tmp_type_arg_48;
            PyTuple_SET_ITEM(tmp_right_name_24, 0, tmp_tuple_element_94);
            CHECK_OBJECT(tmp_class_creation_24__prepared);
            tmp_type_arg_48 = tmp_class_creation_24__prepared;
            tmp_expression_name_120 = BUILTIN_TYPE1(tmp_type_arg_48);
            assert(!(tmp_expression_name_120 == NULL));
            tmp_tuple_element_94 = LOOKUP_ATTRIBUTE(tmp_expression_name_120, mod_consts[53]);
            Py_DECREF(tmp_expression_name_120);
            if (tmp_tuple_element_94 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 197;

                goto tuple_build_exception_27;
            }
            PyTuple_SET_ITEM(tmp_right_name_24, 1, tmp_tuple_element_94);
        }
        goto tuple_build_noexception_27;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_27:;
        Py_DECREF(tmp_right_name_24);
        goto try_except_handler_70;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_27:;
        tmp_raise_value_24 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_24, tmp_right_name_24);
        Py_DECREF(tmp_right_name_24);
        if (tmp_raise_value_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_70;
        }
        exception_type = tmp_raise_type_24;
        Py_INCREF(tmp_raise_type_24);
        exception_value = tmp_raise_value_24;
        exception_lineno = 197;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_70;
    }
    branch_no_72:;
    goto branch_end_71;
    branch_no_71:;
    {
        PyObject *tmp_assign_source_172;
        tmp_assign_source_172 = PyDict_New();
        assert(tmp_class_creation_24__prepared == NULL);
        tmp_class_creation_24__prepared = tmp_assign_source_172;
    }
    branch_end_71:;
    {
        PyObject *tmp_assign_source_173;
        {
            PyObject *tmp_set_locals_24;
            CHECK_OBJECT(tmp_class_creation_24__prepared);
            tmp_set_locals_24 = tmp_class_creation_24__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197 = tmp_set_locals_24;
            Py_INCREF(tmp_set_locals_24);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_72;
        }
        tmp_dictset_value = mod_consts[118];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_72;
        }
        tmp_dictset_value = mod_consts[117];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 197;

            goto try_except_handler_72;
        }
        {
            PyObject *tmp_assign_source_174;
            PyObject *tmp_called_name_48;
            PyObject *tmp_args_name_48;
            PyObject *tmp_tuple_element_95;
            PyObject *tmp_kwargs_name_48;
            CHECK_OBJECT(tmp_class_creation_24__metaclass);
            tmp_called_name_48 = tmp_class_creation_24__metaclass;
            tmp_tuple_element_95 = mod_consts[117];
            tmp_args_name_48 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_48, 0, tmp_tuple_element_95);
            CHECK_OBJECT(tmp_class_creation_24__bases);
            tmp_tuple_element_95 = tmp_class_creation_24__bases;
            PyTuple_SET_ITEM0(tmp_args_name_48, 1, tmp_tuple_element_95);
            tmp_tuple_element_95 = locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197;
            PyTuple_SET_ITEM0(tmp_args_name_48, 2, tmp_tuple_element_95);
            CHECK_OBJECT(tmp_class_creation_24__class_decl_dict);
            tmp_kwargs_name_48 = tmp_class_creation_24__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 197;
            tmp_assign_source_174 = CALL_FUNCTION(tmp_called_name_48, tmp_args_name_48, tmp_kwargs_name_48);
            Py_DECREF(tmp_args_name_48);
            if (tmp_assign_source_174 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 197;

                goto try_except_handler_72;
            }
            assert(outline_23_var___class__ == NULL);
            outline_23_var___class__ = tmp_assign_source_174;
        }
        CHECK_OBJECT(outline_23_var___class__);
        tmp_assign_source_173 = outline_23_var___class__;
        Py_INCREF(tmp_assign_source_173);
        goto try_return_handler_72;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_72:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197);
        locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197 = NULL;
        goto try_return_handler_71;
        // Exception handler code:
        try_except_handler_72:;
        exception_keeper_type_70 = exception_type;
        exception_keeper_value_70 = exception_value;
        exception_keeper_tb_70 = exception_tb;
        exception_keeper_lineno_70 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197);
        locals_pip$_vendor$urllib3$exceptions$$$class__24_InsecureRequestWarning_197 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_70;
        exception_value = exception_keeper_value_70;
        exception_tb = exception_keeper_tb_70;
        exception_lineno = exception_keeper_lineno_70;

        goto try_except_handler_71;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_71:;
        CHECK_OBJECT(outline_23_var___class__);
        Py_DECREF(outline_23_var___class__);
        outline_23_var___class__ = NULL;
        goto outline_result_24;
        // Exception handler code:
        try_except_handler_71:;
        exception_keeper_type_71 = exception_type;
        exception_keeper_value_71 = exception_value;
        exception_keeper_tb_71 = exception_tb;
        exception_keeper_lineno_71 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_71;
        exception_value = exception_keeper_value_71;
        exception_tb = exception_keeper_tb_71;
        exception_lineno = exception_keeper_lineno_71;

        goto outline_exception_24;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_24:;
        exception_lineno = 197;
        goto try_except_handler_70;
        outline_result_24:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[117], tmp_assign_source_173);
    }
    goto try_end_24;
    // Exception handler code:
    try_except_handler_70:;
    exception_keeper_type_72 = exception_type;
    exception_keeper_value_72 = exception_value;
    exception_keeper_tb_72 = exception_tb;
    exception_keeper_lineno_72 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_24__bases);
    tmp_class_creation_24__bases = NULL;
    Py_XDECREF(tmp_class_creation_24__class_decl_dict);
    tmp_class_creation_24__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_24__metaclass);
    tmp_class_creation_24__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_24__prepared);
    tmp_class_creation_24__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_72;
    exception_value = exception_keeper_value_72;
    exception_tb = exception_keeper_tb_72;
    exception_lineno = exception_keeper_lineno_72;

    goto frame_exception_exit_1;
    // End of try:
    try_end_24:;
    Py_XDECREF(tmp_class_creation_24__bases);
    tmp_class_creation_24__bases = NULL;
    Py_XDECREF(tmp_class_creation_24__class_decl_dict);
    tmp_class_creation_24__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_24__metaclass);
    tmp_class_creation_24__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_24__prepared);
    Py_DECREF(tmp_class_creation_24__prepared);
    tmp_class_creation_24__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_175;
        PyObject *tmp_tuple_element_96;
        tmp_tuple_element_96 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[113]);

        if (unlikely(tmp_tuple_element_96 == NULL)) {
            tmp_tuple_element_96 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[113]);
        }

        if (tmp_tuple_element_96 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        tmp_assign_source_175 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_175, 0, tmp_tuple_element_96);
        assert(tmp_class_creation_25__bases == NULL);
        tmp_class_creation_25__bases = tmp_assign_source_175;
    }
    {
        PyObject *tmp_assign_source_176;
        tmp_assign_source_176 = PyDict_New();
        assert(tmp_class_creation_25__class_decl_dict == NULL);
        tmp_class_creation_25__class_decl_dict = tmp_assign_source_176;
    }
    {
        PyObject *tmp_assign_source_177;
        PyObject *tmp_metaclass_name_25;
        nuitka_bool tmp_condition_result_97;
        PyObject *tmp_key_name_73;
        PyObject *tmp_dict_arg_name_73;
        PyObject *tmp_dict_arg_name_74;
        PyObject *tmp_key_name_74;
        PyObject *tmp_type_arg_49;
        PyObject *tmp_expression_name_121;
        PyObject *tmp_subscript_name_25;
        PyObject *tmp_bases_name_25;
        tmp_key_name_73 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_25__class_decl_dict);
        tmp_dict_arg_name_73 = tmp_class_creation_25__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_73, tmp_key_name_73);
        assert(!(tmp_res == -1));
        tmp_condition_result_97 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_97 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_25;
        } else {
            goto condexpr_false_25;
        }
        condexpr_true_25:;
        CHECK_OBJECT(tmp_class_creation_25__class_decl_dict);
        tmp_dict_arg_name_74 = tmp_class_creation_25__class_decl_dict;
        tmp_key_name_74 = mod_consts[49];
        tmp_metaclass_name_25 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_74, tmp_key_name_74);
        if (tmp_metaclass_name_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        goto condexpr_end_25;
        condexpr_false_25:;
        CHECK_OBJECT(tmp_class_creation_25__bases);
        tmp_expression_name_121 = tmp_class_creation_25__bases;
        tmp_subscript_name_25 = mod_consts[46];
        tmp_type_arg_49 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_121, tmp_subscript_name_25, 0);
        if (tmp_type_arg_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        tmp_metaclass_name_25 = BUILTIN_TYPE1(tmp_type_arg_49);
        Py_DECREF(tmp_type_arg_49);
        if (tmp_metaclass_name_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        condexpr_end_25:;
        CHECK_OBJECT(tmp_class_creation_25__bases);
        tmp_bases_name_25 = tmp_class_creation_25__bases;
        tmp_assign_source_177 = SELECT_METACLASS(tmp_metaclass_name_25, tmp_bases_name_25);
        Py_DECREF(tmp_metaclass_name_25);
        if (tmp_assign_source_177 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        assert(tmp_class_creation_25__metaclass == NULL);
        tmp_class_creation_25__metaclass = tmp_assign_source_177;
    }
    {
        nuitka_bool tmp_condition_result_98;
        PyObject *tmp_key_name_75;
        PyObject *tmp_dict_arg_name_75;
        tmp_key_name_75 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_25__class_decl_dict);
        tmp_dict_arg_name_75 = tmp_class_creation_25__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_75, tmp_key_name_75);
        assert(!(tmp_res == -1));
        tmp_condition_result_98 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_98 == NUITKA_BOOL_TRUE) {
            goto branch_yes_73;
        } else {
            goto branch_no_73;
        }
    }
    branch_yes_73:;
    CHECK_OBJECT(tmp_class_creation_25__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_25__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 203;

        goto try_except_handler_73;
    }
    branch_no_73:;
    {
        nuitka_bool tmp_condition_result_99;
        PyObject *tmp_expression_name_122;
        CHECK_OBJECT(tmp_class_creation_25__metaclass);
        tmp_expression_name_122 = tmp_class_creation_25__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_122, mod_consts[50]);
        tmp_condition_result_99 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_99 == NUITKA_BOOL_TRUE) {
            goto branch_yes_74;
        } else {
            goto branch_no_74;
        }
    }
    branch_yes_74:;
    {
        PyObject *tmp_assign_source_178;
        PyObject *tmp_called_name_49;
        PyObject *tmp_expression_name_123;
        PyObject *tmp_args_name_49;
        PyObject *tmp_tuple_element_97;
        PyObject *tmp_kwargs_name_49;
        CHECK_OBJECT(tmp_class_creation_25__metaclass);
        tmp_expression_name_123 = tmp_class_creation_25__metaclass;
        tmp_called_name_49 = LOOKUP_ATTRIBUTE(tmp_expression_name_123, mod_consts[50]);
        if (tmp_called_name_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        tmp_tuple_element_97 = mod_consts[119];
        tmp_args_name_49 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_49, 0, tmp_tuple_element_97);
        CHECK_OBJECT(tmp_class_creation_25__bases);
        tmp_tuple_element_97 = tmp_class_creation_25__bases;
        PyTuple_SET_ITEM0(tmp_args_name_49, 1, tmp_tuple_element_97);
        CHECK_OBJECT(tmp_class_creation_25__class_decl_dict);
        tmp_kwargs_name_49 = tmp_class_creation_25__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 203;
        tmp_assign_source_178 = CALL_FUNCTION(tmp_called_name_49, tmp_args_name_49, tmp_kwargs_name_49);
        Py_DECREF(tmp_called_name_49);
        Py_DECREF(tmp_args_name_49);
        if (tmp_assign_source_178 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        assert(tmp_class_creation_25__prepared == NULL);
        tmp_class_creation_25__prepared = tmp_assign_source_178;
    }
    {
        nuitka_bool tmp_condition_result_100;
        PyObject *tmp_operand_name_25;
        PyObject *tmp_expression_name_124;
        CHECK_OBJECT(tmp_class_creation_25__prepared);
        tmp_expression_name_124 = tmp_class_creation_25__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_124, mod_consts[51]);
        tmp_operand_name_25 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_25);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        tmp_condition_result_100 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_100 == NUITKA_BOOL_TRUE) {
            goto branch_yes_75;
        } else {
            goto branch_no_75;
        }
    }
    branch_yes_75:;
    {
        PyObject *tmp_raise_type_25;
        PyObject *tmp_raise_value_25;
        PyObject *tmp_left_name_25;
        PyObject *tmp_right_name_25;
        PyObject *tmp_tuple_element_98;
        PyObject *tmp_getattr_target_25;
        PyObject *tmp_getattr_attr_25;
        PyObject *tmp_getattr_default_25;
        tmp_raise_type_25 = PyExc_TypeError;
        tmp_left_name_25 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_25__metaclass);
        tmp_getattr_target_25 = tmp_class_creation_25__metaclass;
        tmp_getattr_attr_25 = mod_consts[53];
        tmp_getattr_default_25 = mod_consts[54];
        tmp_tuple_element_98 = BUILTIN_GETATTR(tmp_getattr_target_25, tmp_getattr_attr_25, tmp_getattr_default_25);
        if (tmp_tuple_element_98 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        tmp_right_name_25 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_125;
            PyObject *tmp_type_arg_50;
            PyTuple_SET_ITEM(tmp_right_name_25, 0, tmp_tuple_element_98);
            CHECK_OBJECT(tmp_class_creation_25__prepared);
            tmp_type_arg_50 = tmp_class_creation_25__prepared;
            tmp_expression_name_125 = BUILTIN_TYPE1(tmp_type_arg_50);
            assert(!(tmp_expression_name_125 == NULL));
            tmp_tuple_element_98 = LOOKUP_ATTRIBUTE(tmp_expression_name_125, mod_consts[53]);
            Py_DECREF(tmp_expression_name_125);
            if (tmp_tuple_element_98 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 203;

                goto tuple_build_exception_28;
            }
            PyTuple_SET_ITEM(tmp_right_name_25, 1, tmp_tuple_element_98);
        }
        goto tuple_build_noexception_28;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_28:;
        Py_DECREF(tmp_right_name_25);
        goto try_except_handler_73;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_28:;
        tmp_raise_value_25 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_25, tmp_right_name_25);
        Py_DECREF(tmp_right_name_25);
        if (tmp_raise_value_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_73;
        }
        exception_type = tmp_raise_type_25;
        Py_INCREF(tmp_raise_type_25);
        exception_value = tmp_raise_value_25;
        exception_lineno = 203;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_73;
    }
    branch_no_75:;
    goto branch_end_74;
    branch_no_74:;
    {
        PyObject *tmp_assign_source_179;
        tmp_assign_source_179 = PyDict_New();
        assert(tmp_class_creation_25__prepared == NULL);
        tmp_class_creation_25__prepared = tmp_assign_source_179;
    }
    branch_end_74:;
    {
        PyObject *tmp_assign_source_180;
        {
            PyObject *tmp_set_locals_25;
            CHECK_OBJECT(tmp_class_creation_25__prepared);
            tmp_set_locals_25 = tmp_class_creation_25__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203 = tmp_set_locals_25;
            Py_INCREF(tmp_set_locals_25);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_75;
        }
        tmp_dictset_value = mod_consts[120];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_75;
        }
        tmp_dictset_value = mod_consts[119];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 203;

            goto try_except_handler_75;
        }
        {
            PyObject *tmp_assign_source_181;
            PyObject *tmp_called_name_50;
            PyObject *tmp_args_name_50;
            PyObject *tmp_tuple_element_99;
            PyObject *tmp_kwargs_name_50;
            CHECK_OBJECT(tmp_class_creation_25__metaclass);
            tmp_called_name_50 = tmp_class_creation_25__metaclass;
            tmp_tuple_element_99 = mod_consts[119];
            tmp_args_name_50 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_50, 0, tmp_tuple_element_99);
            CHECK_OBJECT(tmp_class_creation_25__bases);
            tmp_tuple_element_99 = tmp_class_creation_25__bases;
            PyTuple_SET_ITEM0(tmp_args_name_50, 1, tmp_tuple_element_99);
            tmp_tuple_element_99 = locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203;
            PyTuple_SET_ITEM0(tmp_args_name_50, 2, tmp_tuple_element_99);
            CHECK_OBJECT(tmp_class_creation_25__class_decl_dict);
            tmp_kwargs_name_50 = tmp_class_creation_25__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 203;
            tmp_assign_source_181 = CALL_FUNCTION(tmp_called_name_50, tmp_args_name_50, tmp_kwargs_name_50);
            Py_DECREF(tmp_args_name_50);
            if (tmp_assign_source_181 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 203;

                goto try_except_handler_75;
            }
            assert(outline_24_var___class__ == NULL);
            outline_24_var___class__ = tmp_assign_source_181;
        }
        CHECK_OBJECT(outline_24_var___class__);
        tmp_assign_source_180 = outline_24_var___class__;
        Py_INCREF(tmp_assign_source_180);
        goto try_return_handler_75;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_75:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203);
        locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203 = NULL;
        goto try_return_handler_74;
        // Exception handler code:
        try_except_handler_75:;
        exception_keeper_type_73 = exception_type;
        exception_keeper_value_73 = exception_value;
        exception_keeper_tb_73 = exception_tb;
        exception_keeper_lineno_73 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203);
        locals_pip$_vendor$urllib3$exceptions$$$class__25_SystemTimeWarning_203 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_73;
        exception_value = exception_keeper_value_73;
        exception_tb = exception_keeper_tb_73;
        exception_lineno = exception_keeper_lineno_73;

        goto try_except_handler_74;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_74:;
        CHECK_OBJECT(outline_24_var___class__);
        Py_DECREF(outline_24_var___class__);
        outline_24_var___class__ = NULL;
        goto outline_result_25;
        // Exception handler code:
        try_except_handler_74:;
        exception_keeper_type_74 = exception_type;
        exception_keeper_value_74 = exception_value;
        exception_keeper_tb_74 = exception_tb;
        exception_keeper_lineno_74 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_74;
        exception_value = exception_keeper_value_74;
        exception_tb = exception_keeper_tb_74;
        exception_lineno = exception_keeper_lineno_74;

        goto outline_exception_25;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_25:;
        exception_lineno = 203;
        goto try_except_handler_73;
        outline_result_25:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[119], tmp_assign_source_180);
    }
    goto try_end_25;
    // Exception handler code:
    try_except_handler_73:;
    exception_keeper_type_75 = exception_type;
    exception_keeper_value_75 = exception_value;
    exception_keeper_tb_75 = exception_tb;
    exception_keeper_lineno_75 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_25__bases);
    tmp_class_creation_25__bases = NULL;
    Py_XDECREF(tmp_class_creation_25__class_decl_dict);
    tmp_class_creation_25__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_25__metaclass);
    tmp_class_creation_25__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_25__prepared);
    tmp_class_creation_25__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_75;
    exception_value = exception_keeper_value_75;
    exception_tb = exception_keeper_tb_75;
    exception_lineno = exception_keeper_lineno_75;

    goto frame_exception_exit_1;
    // End of try:
    try_end_25:;
    Py_XDECREF(tmp_class_creation_25__bases);
    tmp_class_creation_25__bases = NULL;
    Py_XDECREF(tmp_class_creation_25__class_decl_dict);
    tmp_class_creation_25__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_25__metaclass);
    tmp_class_creation_25__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_25__prepared);
    Py_DECREF(tmp_class_creation_25__prepared);
    tmp_class_creation_25__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_182;
        PyObject *tmp_tuple_element_100;
        tmp_tuple_element_100 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[113]);

        if (unlikely(tmp_tuple_element_100 == NULL)) {
            tmp_tuple_element_100 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[113]);
        }

        if (tmp_tuple_element_100 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        tmp_assign_source_182 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_182, 0, tmp_tuple_element_100);
        assert(tmp_class_creation_26__bases == NULL);
        tmp_class_creation_26__bases = tmp_assign_source_182;
    }
    {
        PyObject *tmp_assign_source_183;
        tmp_assign_source_183 = PyDict_New();
        assert(tmp_class_creation_26__class_decl_dict == NULL);
        tmp_class_creation_26__class_decl_dict = tmp_assign_source_183;
    }
    {
        PyObject *tmp_assign_source_184;
        PyObject *tmp_metaclass_name_26;
        nuitka_bool tmp_condition_result_101;
        PyObject *tmp_key_name_76;
        PyObject *tmp_dict_arg_name_76;
        PyObject *tmp_dict_arg_name_77;
        PyObject *tmp_key_name_77;
        PyObject *tmp_type_arg_51;
        PyObject *tmp_expression_name_126;
        PyObject *tmp_subscript_name_26;
        PyObject *tmp_bases_name_26;
        tmp_key_name_76 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_26__class_decl_dict);
        tmp_dict_arg_name_76 = tmp_class_creation_26__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_76, tmp_key_name_76);
        assert(!(tmp_res == -1));
        tmp_condition_result_101 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_101 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_26;
        } else {
            goto condexpr_false_26;
        }
        condexpr_true_26:;
        CHECK_OBJECT(tmp_class_creation_26__class_decl_dict);
        tmp_dict_arg_name_77 = tmp_class_creation_26__class_decl_dict;
        tmp_key_name_77 = mod_consts[49];
        tmp_metaclass_name_26 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_77, tmp_key_name_77);
        if (tmp_metaclass_name_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        goto condexpr_end_26;
        condexpr_false_26:;
        CHECK_OBJECT(tmp_class_creation_26__bases);
        tmp_expression_name_126 = tmp_class_creation_26__bases;
        tmp_subscript_name_26 = mod_consts[46];
        tmp_type_arg_51 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_126, tmp_subscript_name_26, 0);
        if (tmp_type_arg_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        tmp_metaclass_name_26 = BUILTIN_TYPE1(tmp_type_arg_51);
        Py_DECREF(tmp_type_arg_51);
        if (tmp_metaclass_name_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        condexpr_end_26:;
        CHECK_OBJECT(tmp_class_creation_26__bases);
        tmp_bases_name_26 = tmp_class_creation_26__bases;
        tmp_assign_source_184 = SELECT_METACLASS(tmp_metaclass_name_26, tmp_bases_name_26);
        Py_DECREF(tmp_metaclass_name_26);
        if (tmp_assign_source_184 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        assert(tmp_class_creation_26__metaclass == NULL);
        tmp_class_creation_26__metaclass = tmp_assign_source_184;
    }
    {
        nuitka_bool tmp_condition_result_102;
        PyObject *tmp_key_name_78;
        PyObject *tmp_dict_arg_name_78;
        tmp_key_name_78 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_26__class_decl_dict);
        tmp_dict_arg_name_78 = tmp_class_creation_26__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_78, tmp_key_name_78);
        assert(!(tmp_res == -1));
        tmp_condition_result_102 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_102 == NUITKA_BOOL_TRUE) {
            goto branch_yes_76;
        } else {
            goto branch_no_76;
        }
    }
    branch_yes_76:;
    CHECK_OBJECT(tmp_class_creation_26__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_26__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 209;

        goto try_except_handler_76;
    }
    branch_no_76:;
    {
        nuitka_bool tmp_condition_result_103;
        PyObject *tmp_expression_name_127;
        CHECK_OBJECT(tmp_class_creation_26__metaclass);
        tmp_expression_name_127 = tmp_class_creation_26__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_127, mod_consts[50]);
        tmp_condition_result_103 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_103 == NUITKA_BOOL_TRUE) {
            goto branch_yes_77;
        } else {
            goto branch_no_77;
        }
    }
    branch_yes_77:;
    {
        PyObject *tmp_assign_source_185;
        PyObject *tmp_called_name_51;
        PyObject *tmp_expression_name_128;
        PyObject *tmp_args_name_51;
        PyObject *tmp_tuple_element_101;
        PyObject *tmp_kwargs_name_51;
        CHECK_OBJECT(tmp_class_creation_26__metaclass);
        tmp_expression_name_128 = tmp_class_creation_26__metaclass;
        tmp_called_name_51 = LOOKUP_ATTRIBUTE(tmp_expression_name_128, mod_consts[50]);
        if (tmp_called_name_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        tmp_tuple_element_101 = mod_consts[121];
        tmp_args_name_51 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_51, 0, tmp_tuple_element_101);
        CHECK_OBJECT(tmp_class_creation_26__bases);
        tmp_tuple_element_101 = tmp_class_creation_26__bases;
        PyTuple_SET_ITEM0(tmp_args_name_51, 1, tmp_tuple_element_101);
        CHECK_OBJECT(tmp_class_creation_26__class_decl_dict);
        tmp_kwargs_name_51 = tmp_class_creation_26__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 209;
        tmp_assign_source_185 = CALL_FUNCTION(tmp_called_name_51, tmp_args_name_51, tmp_kwargs_name_51);
        Py_DECREF(tmp_called_name_51);
        Py_DECREF(tmp_args_name_51);
        if (tmp_assign_source_185 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        assert(tmp_class_creation_26__prepared == NULL);
        tmp_class_creation_26__prepared = tmp_assign_source_185;
    }
    {
        nuitka_bool tmp_condition_result_104;
        PyObject *tmp_operand_name_26;
        PyObject *tmp_expression_name_129;
        CHECK_OBJECT(tmp_class_creation_26__prepared);
        tmp_expression_name_129 = tmp_class_creation_26__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_129, mod_consts[51]);
        tmp_operand_name_26 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_26);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        tmp_condition_result_104 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_104 == NUITKA_BOOL_TRUE) {
            goto branch_yes_78;
        } else {
            goto branch_no_78;
        }
    }
    branch_yes_78:;
    {
        PyObject *tmp_raise_type_26;
        PyObject *tmp_raise_value_26;
        PyObject *tmp_left_name_26;
        PyObject *tmp_right_name_26;
        PyObject *tmp_tuple_element_102;
        PyObject *tmp_getattr_target_26;
        PyObject *tmp_getattr_attr_26;
        PyObject *tmp_getattr_default_26;
        tmp_raise_type_26 = PyExc_TypeError;
        tmp_left_name_26 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_26__metaclass);
        tmp_getattr_target_26 = tmp_class_creation_26__metaclass;
        tmp_getattr_attr_26 = mod_consts[53];
        tmp_getattr_default_26 = mod_consts[54];
        tmp_tuple_element_102 = BUILTIN_GETATTR(tmp_getattr_target_26, tmp_getattr_attr_26, tmp_getattr_default_26);
        if (tmp_tuple_element_102 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        tmp_right_name_26 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_130;
            PyObject *tmp_type_arg_52;
            PyTuple_SET_ITEM(tmp_right_name_26, 0, tmp_tuple_element_102);
            CHECK_OBJECT(tmp_class_creation_26__prepared);
            tmp_type_arg_52 = tmp_class_creation_26__prepared;
            tmp_expression_name_130 = BUILTIN_TYPE1(tmp_type_arg_52);
            assert(!(tmp_expression_name_130 == NULL));
            tmp_tuple_element_102 = LOOKUP_ATTRIBUTE(tmp_expression_name_130, mod_consts[53]);
            Py_DECREF(tmp_expression_name_130);
            if (tmp_tuple_element_102 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 209;

                goto tuple_build_exception_29;
            }
            PyTuple_SET_ITEM(tmp_right_name_26, 1, tmp_tuple_element_102);
        }
        goto tuple_build_noexception_29;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_29:;
        Py_DECREF(tmp_right_name_26);
        goto try_except_handler_76;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_29:;
        tmp_raise_value_26 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_26, tmp_right_name_26);
        Py_DECREF(tmp_right_name_26);
        if (tmp_raise_value_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_76;
        }
        exception_type = tmp_raise_type_26;
        Py_INCREF(tmp_raise_type_26);
        exception_value = tmp_raise_value_26;
        exception_lineno = 209;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_76;
    }
    branch_no_78:;
    goto branch_end_77;
    branch_no_77:;
    {
        PyObject *tmp_assign_source_186;
        tmp_assign_source_186 = PyDict_New();
        assert(tmp_class_creation_26__prepared == NULL);
        tmp_class_creation_26__prepared = tmp_assign_source_186;
    }
    branch_end_77:;
    {
        PyObject *tmp_assign_source_187;
        {
            PyObject *tmp_set_locals_26;
            CHECK_OBJECT(tmp_class_creation_26__prepared);
            tmp_set_locals_26 = tmp_class_creation_26__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209 = tmp_set_locals_26;
            Py_INCREF(tmp_set_locals_26);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_78;
        }
        tmp_dictset_value = mod_consts[122];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_78;
        }
        tmp_dictset_value = mod_consts[121];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 209;

            goto try_except_handler_78;
        }
        {
            PyObject *tmp_assign_source_188;
            PyObject *tmp_called_name_52;
            PyObject *tmp_args_name_52;
            PyObject *tmp_tuple_element_103;
            PyObject *tmp_kwargs_name_52;
            CHECK_OBJECT(tmp_class_creation_26__metaclass);
            tmp_called_name_52 = tmp_class_creation_26__metaclass;
            tmp_tuple_element_103 = mod_consts[121];
            tmp_args_name_52 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_52, 0, tmp_tuple_element_103);
            CHECK_OBJECT(tmp_class_creation_26__bases);
            tmp_tuple_element_103 = tmp_class_creation_26__bases;
            PyTuple_SET_ITEM0(tmp_args_name_52, 1, tmp_tuple_element_103);
            tmp_tuple_element_103 = locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209;
            PyTuple_SET_ITEM0(tmp_args_name_52, 2, tmp_tuple_element_103);
            CHECK_OBJECT(tmp_class_creation_26__class_decl_dict);
            tmp_kwargs_name_52 = tmp_class_creation_26__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 209;
            tmp_assign_source_188 = CALL_FUNCTION(tmp_called_name_52, tmp_args_name_52, tmp_kwargs_name_52);
            Py_DECREF(tmp_args_name_52);
            if (tmp_assign_source_188 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 209;

                goto try_except_handler_78;
            }
            assert(outline_25_var___class__ == NULL);
            outline_25_var___class__ = tmp_assign_source_188;
        }
        CHECK_OBJECT(outline_25_var___class__);
        tmp_assign_source_187 = outline_25_var___class__;
        Py_INCREF(tmp_assign_source_187);
        goto try_return_handler_78;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_78:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209);
        locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209 = NULL;
        goto try_return_handler_77;
        // Exception handler code:
        try_except_handler_78:;
        exception_keeper_type_76 = exception_type;
        exception_keeper_value_76 = exception_value;
        exception_keeper_tb_76 = exception_tb;
        exception_keeper_lineno_76 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209);
        locals_pip$_vendor$urllib3$exceptions$$$class__26_InsecurePlatformWarning_209 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_76;
        exception_value = exception_keeper_value_76;
        exception_tb = exception_keeper_tb_76;
        exception_lineno = exception_keeper_lineno_76;

        goto try_except_handler_77;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_77:;
        CHECK_OBJECT(outline_25_var___class__);
        Py_DECREF(outline_25_var___class__);
        outline_25_var___class__ = NULL;
        goto outline_result_26;
        // Exception handler code:
        try_except_handler_77:;
        exception_keeper_type_77 = exception_type;
        exception_keeper_value_77 = exception_value;
        exception_keeper_tb_77 = exception_tb;
        exception_keeper_lineno_77 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_77;
        exception_value = exception_keeper_value_77;
        exception_tb = exception_keeper_tb_77;
        exception_lineno = exception_keeper_lineno_77;

        goto outline_exception_26;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_26:;
        exception_lineno = 209;
        goto try_except_handler_76;
        outline_result_26:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[121], tmp_assign_source_187);
    }
    goto try_end_26;
    // Exception handler code:
    try_except_handler_76:;
    exception_keeper_type_78 = exception_type;
    exception_keeper_value_78 = exception_value;
    exception_keeper_tb_78 = exception_tb;
    exception_keeper_lineno_78 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_26__bases);
    tmp_class_creation_26__bases = NULL;
    Py_XDECREF(tmp_class_creation_26__class_decl_dict);
    tmp_class_creation_26__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_26__metaclass);
    tmp_class_creation_26__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_26__prepared);
    tmp_class_creation_26__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_78;
    exception_value = exception_keeper_value_78;
    exception_tb = exception_keeper_tb_78;
    exception_lineno = exception_keeper_lineno_78;

    goto frame_exception_exit_1;
    // End of try:
    try_end_26:;
    Py_XDECREF(tmp_class_creation_26__bases);
    tmp_class_creation_26__bases = NULL;
    Py_XDECREF(tmp_class_creation_26__class_decl_dict);
    tmp_class_creation_26__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_26__metaclass);
    tmp_class_creation_26__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_26__prepared);
    Py_DECREF(tmp_class_creation_26__prepared);
    tmp_class_creation_26__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_189;
        PyObject *tmp_tuple_element_104;
        tmp_tuple_element_104 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[60]);

        if (unlikely(tmp_tuple_element_104 == NULL)) {
            tmp_tuple_element_104 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[60]);
        }

        if (tmp_tuple_element_104 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        tmp_assign_source_189 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_189, 0, tmp_tuple_element_104);
        assert(tmp_class_creation_27__bases == NULL);
        tmp_class_creation_27__bases = tmp_assign_source_189;
    }
    {
        PyObject *tmp_assign_source_190;
        tmp_assign_source_190 = PyDict_New();
        assert(tmp_class_creation_27__class_decl_dict == NULL);
        tmp_class_creation_27__class_decl_dict = tmp_assign_source_190;
    }
    {
        PyObject *tmp_assign_source_191;
        PyObject *tmp_metaclass_name_27;
        nuitka_bool tmp_condition_result_105;
        PyObject *tmp_key_name_79;
        PyObject *tmp_dict_arg_name_79;
        PyObject *tmp_dict_arg_name_80;
        PyObject *tmp_key_name_80;
        PyObject *tmp_type_arg_53;
        PyObject *tmp_expression_name_131;
        PyObject *tmp_subscript_name_27;
        PyObject *tmp_bases_name_27;
        tmp_key_name_79 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_27__class_decl_dict);
        tmp_dict_arg_name_79 = tmp_class_creation_27__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_79, tmp_key_name_79);
        assert(!(tmp_res == -1));
        tmp_condition_result_105 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_105 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_27;
        } else {
            goto condexpr_false_27;
        }
        condexpr_true_27:;
        CHECK_OBJECT(tmp_class_creation_27__class_decl_dict);
        tmp_dict_arg_name_80 = tmp_class_creation_27__class_decl_dict;
        tmp_key_name_80 = mod_consts[49];
        tmp_metaclass_name_27 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_80, tmp_key_name_80);
        if (tmp_metaclass_name_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        goto condexpr_end_27;
        condexpr_false_27:;
        CHECK_OBJECT(tmp_class_creation_27__bases);
        tmp_expression_name_131 = tmp_class_creation_27__bases;
        tmp_subscript_name_27 = mod_consts[46];
        tmp_type_arg_53 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_131, tmp_subscript_name_27, 0);
        if (tmp_type_arg_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        tmp_metaclass_name_27 = BUILTIN_TYPE1(tmp_type_arg_53);
        Py_DECREF(tmp_type_arg_53);
        if (tmp_metaclass_name_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        condexpr_end_27:;
        CHECK_OBJECT(tmp_class_creation_27__bases);
        tmp_bases_name_27 = tmp_class_creation_27__bases;
        tmp_assign_source_191 = SELECT_METACLASS(tmp_metaclass_name_27, tmp_bases_name_27);
        Py_DECREF(tmp_metaclass_name_27);
        if (tmp_assign_source_191 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        assert(tmp_class_creation_27__metaclass == NULL);
        tmp_class_creation_27__metaclass = tmp_assign_source_191;
    }
    {
        nuitka_bool tmp_condition_result_106;
        PyObject *tmp_key_name_81;
        PyObject *tmp_dict_arg_name_81;
        tmp_key_name_81 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_27__class_decl_dict);
        tmp_dict_arg_name_81 = tmp_class_creation_27__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_81, tmp_key_name_81);
        assert(!(tmp_res == -1));
        tmp_condition_result_106 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_106 == NUITKA_BOOL_TRUE) {
            goto branch_yes_79;
        } else {
            goto branch_no_79;
        }
    }
    branch_yes_79:;
    CHECK_OBJECT(tmp_class_creation_27__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_27__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 215;

        goto try_except_handler_79;
    }
    branch_no_79:;
    {
        nuitka_bool tmp_condition_result_107;
        PyObject *tmp_expression_name_132;
        CHECK_OBJECT(tmp_class_creation_27__metaclass);
        tmp_expression_name_132 = tmp_class_creation_27__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_132, mod_consts[50]);
        tmp_condition_result_107 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_107 == NUITKA_BOOL_TRUE) {
            goto branch_yes_80;
        } else {
            goto branch_no_80;
        }
    }
    branch_yes_80:;
    {
        PyObject *tmp_assign_source_192;
        PyObject *tmp_called_name_53;
        PyObject *tmp_expression_name_133;
        PyObject *tmp_args_name_53;
        PyObject *tmp_tuple_element_105;
        PyObject *tmp_kwargs_name_53;
        CHECK_OBJECT(tmp_class_creation_27__metaclass);
        tmp_expression_name_133 = tmp_class_creation_27__metaclass;
        tmp_called_name_53 = LOOKUP_ATTRIBUTE(tmp_expression_name_133, mod_consts[50]);
        if (tmp_called_name_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        tmp_tuple_element_105 = mod_consts[123];
        tmp_args_name_53 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_53, 0, tmp_tuple_element_105);
        CHECK_OBJECT(tmp_class_creation_27__bases);
        tmp_tuple_element_105 = tmp_class_creation_27__bases;
        PyTuple_SET_ITEM0(tmp_args_name_53, 1, tmp_tuple_element_105);
        CHECK_OBJECT(tmp_class_creation_27__class_decl_dict);
        tmp_kwargs_name_53 = tmp_class_creation_27__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 215;
        tmp_assign_source_192 = CALL_FUNCTION(tmp_called_name_53, tmp_args_name_53, tmp_kwargs_name_53);
        Py_DECREF(tmp_called_name_53);
        Py_DECREF(tmp_args_name_53);
        if (tmp_assign_source_192 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        assert(tmp_class_creation_27__prepared == NULL);
        tmp_class_creation_27__prepared = tmp_assign_source_192;
    }
    {
        nuitka_bool tmp_condition_result_108;
        PyObject *tmp_operand_name_27;
        PyObject *tmp_expression_name_134;
        CHECK_OBJECT(tmp_class_creation_27__prepared);
        tmp_expression_name_134 = tmp_class_creation_27__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_134, mod_consts[51]);
        tmp_operand_name_27 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_27);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        tmp_condition_result_108 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_108 == NUITKA_BOOL_TRUE) {
            goto branch_yes_81;
        } else {
            goto branch_no_81;
        }
    }
    branch_yes_81:;
    {
        PyObject *tmp_raise_type_27;
        PyObject *tmp_raise_value_27;
        PyObject *tmp_left_name_27;
        PyObject *tmp_right_name_27;
        PyObject *tmp_tuple_element_106;
        PyObject *tmp_getattr_target_27;
        PyObject *tmp_getattr_attr_27;
        PyObject *tmp_getattr_default_27;
        tmp_raise_type_27 = PyExc_TypeError;
        tmp_left_name_27 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_27__metaclass);
        tmp_getattr_target_27 = tmp_class_creation_27__metaclass;
        tmp_getattr_attr_27 = mod_consts[53];
        tmp_getattr_default_27 = mod_consts[54];
        tmp_tuple_element_106 = BUILTIN_GETATTR(tmp_getattr_target_27, tmp_getattr_attr_27, tmp_getattr_default_27);
        if (tmp_tuple_element_106 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        tmp_right_name_27 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_135;
            PyObject *tmp_type_arg_54;
            PyTuple_SET_ITEM(tmp_right_name_27, 0, tmp_tuple_element_106);
            CHECK_OBJECT(tmp_class_creation_27__prepared);
            tmp_type_arg_54 = tmp_class_creation_27__prepared;
            tmp_expression_name_135 = BUILTIN_TYPE1(tmp_type_arg_54);
            assert(!(tmp_expression_name_135 == NULL));
            tmp_tuple_element_106 = LOOKUP_ATTRIBUTE(tmp_expression_name_135, mod_consts[53]);
            Py_DECREF(tmp_expression_name_135);
            if (tmp_tuple_element_106 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 215;

                goto tuple_build_exception_30;
            }
            PyTuple_SET_ITEM(tmp_right_name_27, 1, tmp_tuple_element_106);
        }
        goto tuple_build_noexception_30;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_30:;
        Py_DECREF(tmp_right_name_27);
        goto try_except_handler_79;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_30:;
        tmp_raise_value_27 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_27, tmp_right_name_27);
        Py_DECREF(tmp_right_name_27);
        if (tmp_raise_value_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_79;
        }
        exception_type = tmp_raise_type_27;
        Py_INCREF(tmp_raise_type_27);
        exception_value = tmp_raise_value_27;
        exception_lineno = 215;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_79;
    }
    branch_no_81:;
    goto branch_end_80;
    branch_no_80:;
    {
        PyObject *tmp_assign_source_193;
        tmp_assign_source_193 = PyDict_New();
        assert(tmp_class_creation_27__prepared == NULL);
        tmp_class_creation_27__prepared = tmp_assign_source_193;
    }
    branch_end_80:;
    {
        PyObject *tmp_assign_source_194;
        {
            PyObject *tmp_set_locals_27;
            CHECK_OBJECT(tmp_class_creation_27__prepared);
            tmp_set_locals_27 = tmp_class_creation_27__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215 = tmp_set_locals_27;
            Py_INCREF(tmp_set_locals_27);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_81;
        }
        tmp_dictset_value = mod_consts[124];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_81;
        }
        tmp_dictset_value = mod_consts[123];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 215;

            goto try_except_handler_81;
        }
        {
            PyObject *tmp_assign_source_195;
            PyObject *tmp_called_name_54;
            PyObject *tmp_args_name_54;
            PyObject *tmp_tuple_element_107;
            PyObject *tmp_kwargs_name_54;
            CHECK_OBJECT(tmp_class_creation_27__metaclass);
            tmp_called_name_54 = tmp_class_creation_27__metaclass;
            tmp_tuple_element_107 = mod_consts[123];
            tmp_args_name_54 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_54, 0, tmp_tuple_element_107);
            CHECK_OBJECT(tmp_class_creation_27__bases);
            tmp_tuple_element_107 = tmp_class_creation_27__bases;
            PyTuple_SET_ITEM0(tmp_args_name_54, 1, tmp_tuple_element_107);
            tmp_tuple_element_107 = locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215;
            PyTuple_SET_ITEM0(tmp_args_name_54, 2, tmp_tuple_element_107);
            CHECK_OBJECT(tmp_class_creation_27__class_decl_dict);
            tmp_kwargs_name_54 = tmp_class_creation_27__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 215;
            tmp_assign_source_195 = CALL_FUNCTION(tmp_called_name_54, tmp_args_name_54, tmp_kwargs_name_54);
            Py_DECREF(tmp_args_name_54);
            if (tmp_assign_source_195 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 215;

                goto try_except_handler_81;
            }
            assert(outline_26_var___class__ == NULL);
            outline_26_var___class__ = tmp_assign_source_195;
        }
        CHECK_OBJECT(outline_26_var___class__);
        tmp_assign_source_194 = outline_26_var___class__;
        Py_INCREF(tmp_assign_source_194);
        goto try_return_handler_81;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_81:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215);
        locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215 = NULL;
        goto try_return_handler_80;
        // Exception handler code:
        try_except_handler_81:;
        exception_keeper_type_79 = exception_type;
        exception_keeper_value_79 = exception_value;
        exception_keeper_tb_79 = exception_tb;
        exception_keeper_lineno_79 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215);
        locals_pip$_vendor$urllib3$exceptions$$$class__27_SNIMissingWarning_215 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_79;
        exception_value = exception_keeper_value_79;
        exception_tb = exception_keeper_tb_79;
        exception_lineno = exception_keeper_lineno_79;

        goto try_except_handler_80;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_80:;
        CHECK_OBJECT(outline_26_var___class__);
        Py_DECREF(outline_26_var___class__);
        outline_26_var___class__ = NULL;
        goto outline_result_27;
        // Exception handler code:
        try_except_handler_80:;
        exception_keeper_type_80 = exception_type;
        exception_keeper_value_80 = exception_value;
        exception_keeper_tb_80 = exception_tb;
        exception_keeper_lineno_80 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_80;
        exception_value = exception_keeper_value_80;
        exception_tb = exception_keeper_tb_80;
        exception_lineno = exception_keeper_lineno_80;

        goto outline_exception_27;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_27:;
        exception_lineno = 215;
        goto try_except_handler_79;
        outline_result_27:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[123], tmp_assign_source_194);
    }
    goto try_end_27;
    // Exception handler code:
    try_except_handler_79:;
    exception_keeper_type_81 = exception_type;
    exception_keeper_value_81 = exception_value;
    exception_keeper_tb_81 = exception_tb;
    exception_keeper_lineno_81 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_27__bases);
    tmp_class_creation_27__bases = NULL;
    Py_XDECREF(tmp_class_creation_27__class_decl_dict);
    tmp_class_creation_27__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_27__metaclass);
    tmp_class_creation_27__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_27__prepared);
    tmp_class_creation_27__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_81;
    exception_value = exception_keeper_value_81;
    exception_tb = exception_keeper_tb_81;
    exception_lineno = exception_keeper_lineno_81;

    goto frame_exception_exit_1;
    // End of try:
    try_end_27:;
    Py_XDECREF(tmp_class_creation_27__bases);
    tmp_class_creation_27__bases = NULL;
    Py_XDECREF(tmp_class_creation_27__class_decl_dict);
    tmp_class_creation_27__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_27__metaclass);
    tmp_class_creation_27__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_27__prepared);
    Py_DECREF(tmp_class_creation_27__prepared);
    tmp_class_creation_27__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_196;
        PyObject *tmp_tuple_element_108;
        tmp_tuple_element_108 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[60]);

        if (unlikely(tmp_tuple_element_108 == NULL)) {
            tmp_tuple_element_108 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[60]);
        }

        if (tmp_tuple_element_108 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        tmp_assign_source_196 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_196, 0, tmp_tuple_element_108);
        assert(tmp_class_creation_28__bases == NULL);
        tmp_class_creation_28__bases = tmp_assign_source_196;
    }
    {
        PyObject *tmp_assign_source_197;
        tmp_assign_source_197 = PyDict_New();
        assert(tmp_class_creation_28__class_decl_dict == NULL);
        tmp_class_creation_28__class_decl_dict = tmp_assign_source_197;
    }
    {
        PyObject *tmp_assign_source_198;
        PyObject *tmp_metaclass_name_28;
        nuitka_bool tmp_condition_result_109;
        PyObject *tmp_key_name_82;
        PyObject *tmp_dict_arg_name_82;
        PyObject *tmp_dict_arg_name_83;
        PyObject *tmp_key_name_83;
        PyObject *tmp_type_arg_55;
        PyObject *tmp_expression_name_136;
        PyObject *tmp_subscript_name_28;
        PyObject *tmp_bases_name_28;
        tmp_key_name_82 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_28__class_decl_dict);
        tmp_dict_arg_name_82 = tmp_class_creation_28__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_82, tmp_key_name_82);
        assert(!(tmp_res == -1));
        tmp_condition_result_109 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_109 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_28;
        } else {
            goto condexpr_false_28;
        }
        condexpr_true_28:;
        CHECK_OBJECT(tmp_class_creation_28__class_decl_dict);
        tmp_dict_arg_name_83 = tmp_class_creation_28__class_decl_dict;
        tmp_key_name_83 = mod_consts[49];
        tmp_metaclass_name_28 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_83, tmp_key_name_83);
        if (tmp_metaclass_name_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        goto condexpr_end_28;
        condexpr_false_28:;
        CHECK_OBJECT(tmp_class_creation_28__bases);
        tmp_expression_name_136 = tmp_class_creation_28__bases;
        tmp_subscript_name_28 = mod_consts[46];
        tmp_type_arg_55 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_136, tmp_subscript_name_28, 0);
        if (tmp_type_arg_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        tmp_metaclass_name_28 = BUILTIN_TYPE1(tmp_type_arg_55);
        Py_DECREF(tmp_type_arg_55);
        if (tmp_metaclass_name_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        condexpr_end_28:;
        CHECK_OBJECT(tmp_class_creation_28__bases);
        tmp_bases_name_28 = tmp_class_creation_28__bases;
        tmp_assign_source_198 = SELECT_METACLASS(tmp_metaclass_name_28, tmp_bases_name_28);
        Py_DECREF(tmp_metaclass_name_28);
        if (tmp_assign_source_198 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        assert(tmp_class_creation_28__metaclass == NULL);
        tmp_class_creation_28__metaclass = tmp_assign_source_198;
    }
    {
        nuitka_bool tmp_condition_result_110;
        PyObject *tmp_key_name_84;
        PyObject *tmp_dict_arg_name_84;
        tmp_key_name_84 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_28__class_decl_dict);
        tmp_dict_arg_name_84 = tmp_class_creation_28__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_84, tmp_key_name_84);
        assert(!(tmp_res == -1));
        tmp_condition_result_110 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_110 == NUITKA_BOOL_TRUE) {
            goto branch_yes_82;
        } else {
            goto branch_no_82;
        }
    }
    branch_yes_82:;
    CHECK_OBJECT(tmp_class_creation_28__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_28__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 221;

        goto try_except_handler_82;
    }
    branch_no_82:;
    {
        nuitka_bool tmp_condition_result_111;
        PyObject *tmp_expression_name_137;
        CHECK_OBJECT(tmp_class_creation_28__metaclass);
        tmp_expression_name_137 = tmp_class_creation_28__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_137, mod_consts[50]);
        tmp_condition_result_111 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_111 == NUITKA_BOOL_TRUE) {
            goto branch_yes_83;
        } else {
            goto branch_no_83;
        }
    }
    branch_yes_83:;
    {
        PyObject *tmp_assign_source_199;
        PyObject *tmp_called_name_55;
        PyObject *tmp_expression_name_138;
        PyObject *tmp_args_name_55;
        PyObject *tmp_tuple_element_109;
        PyObject *tmp_kwargs_name_55;
        CHECK_OBJECT(tmp_class_creation_28__metaclass);
        tmp_expression_name_138 = tmp_class_creation_28__metaclass;
        tmp_called_name_55 = LOOKUP_ATTRIBUTE(tmp_expression_name_138, mod_consts[50]);
        if (tmp_called_name_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        tmp_tuple_element_109 = mod_consts[125];
        tmp_args_name_55 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_55, 0, tmp_tuple_element_109);
        CHECK_OBJECT(tmp_class_creation_28__bases);
        tmp_tuple_element_109 = tmp_class_creation_28__bases;
        PyTuple_SET_ITEM0(tmp_args_name_55, 1, tmp_tuple_element_109);
        CHECK_OBJECT(tmp_class_creation_28__class_decl_dict);
        tmp_kwargs_name_55 = tmp_class_creation_28__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 221;
        tmp_assign_source_199 = CALL_FUNCTION(tmp_called_name_55, tmp_args_name_55, tmp_kwargs_name_55);
        Py_DECREF(tmp_called_name_55);
        Py_DECREF(tmp_args_name_55);
        if (tmp_assign_source_199 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        assert(tmp_class_creation_28__prepared == NULL);
        tmp_class_creation_28__prepared = tmp_assign_source_199;
    }
    {
        nuitka_bool tmp_condition_result_112;
        PyObject *tmp_operand_name_28;
        PyObject *tmp_expression_name_139;
        CHECK_OBJECT(tmp_class_creation_28__prepared);
        tmp_expression_name_139 = tmp_class_creation_28__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_139, mod_consts[51]);
        tmp_operand_name_28 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_28);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        tmp_condition_result_112 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_112 == NUITKA_BOOL_TRUE) {
            goto branch_yes_84;
        } else {
            goto branch_no_84;
        }
    }
    branch_yes_84:;
    {
        PyObject *tmp_raise_type_28;
        PyObject *tmp_raise_value_28;
        PyObject *tmp_left_name_28;
        PyObject *tmp_right_name_28;
        PyObject *tmp_tuple_element_110;
        PyObject *tmp_getattr_target_28;
        PyObject *tmp_getattr_attr_28;
        PyObject *tmp_getattr_default_28;
        tmp_raise_type_28 = PyExc_TypeError;
        tmp_left_name_28 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_28__metaclass);
        tmp_getattr_target_28 = tmp_class_creation_28__metaclass;
        tmp_getattr_attr_28 = mod_consts[53];
        tmp_getattr_default_28 = mod_consts[54];
        tmp_tuple_element_110 = BUILTIN_GETATTR(tmp_getattr_target_28, tmp_getattr_attr_28, tmp_getattr_default_28);
        if (tmp_tuple_element_110 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        tmp_right_name_28 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_140;
            PyObject *tmp_type_arg_56;
            PyTuple_SET_ITEM(tmp_right_name_28, 0, tmp_tuple_element_110);
            CHECK_OBJECT(tmp_class_creation_28__prepared);
            tmp_type_arg_56 = tmp_class_creation_28__prepared;
            tmp_expression_name_140 = BUILTIN_TYPE1(tmp_type_arg_56);
            assert(!(tmp_expression_name_140 == NULL));
            tmp_tuple_element_110 = LOOKUP_ATTRIBUTE(tmp_expression_name_140, mod_consts[53]);
            Py_DECREF(tmp_expression_name_140);
            if (tmp_tuple_element_110 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 221;

                goto tuple_build_exception_31;
            }
            PyTuple_SET_ITEM(tmp_right_name_28, 1, tmp_tuple_element_110);
        }
        goto tuple_build_noexception_31;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_31:;
        Py_DECREF(tmp_right_name_28);
        goto try_except_handler_82;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_31:;
        tmp_raise_value_28 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_28, tmp_right_name_28);
        Py_DECREF(tmp_right_name_28);
        if (tmp_raise_value_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_82;
        }
        exception_type = tmp_raise_type_28;
        Py_INCREF(tmp_raise_type_28);
        exception_value = tmp_raise_value_28;
        exception_lineno = 221;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_82;
    }
    branch_no_84:;
    goto branch_end_83;
    branch_no_83:;
    {
        PyObject *tmp_assign_source_200;
        tmp_assign_source_200 = PyDict_New();
        assert(tmp_class_creation_28__prepared == NULL);
        tmp_class_creation_28__prepared = tmp_assign_source_200;
    }
    branch_end_83:;
    {
        PyObject *tmp_assign_source_201;
        {
            PyObject *tmp_set_locals_28;
            CHECK_OBJECT(tmp_class_creation_28__prepared);
            tmp_set_locals_28 = tmp_class_creation_28__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221 = tmp_set_locals_28;
            Py_INCREF(tmp_set_locals_28);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_84;
        }
        tmp_dictset_value = mod_consts[126];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_84;
        }
        tmp_dictset_value = mod_consts[125];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 221;

            goto try_except_handler_84;
        }
        {
            PyObject *tmp_assign_source_202;
            PyObject *tmp_called_name_56;
            PyObject *tmp_args_name_56;
            PyObject *tmp_tuple_element_111;
            PyObject *tmp_kwargs_name_56;
            CHECK_OBJECT(tmp_class_creation_28__metaclass);
            tmp_called_name_56 = tmp_class_creation_28__metaclass;
            tmp_tuple_element_111 = mod_consts[125];
            tmp_args_name_56 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_56, 0, tmp_tuple_element_111);
            CHECK_OBJECT(tmp_class_creation_28__bases);
            tmp_tuple_element_111 = tmp_class_creation_28__bases;
            PyTuple_SET_ITEM0(tmp_args_name_56, 1, tmp_tuple_element_111);
            tmp_tuple_element_111 = locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221;
            PyTuple_SET_ITEM0(tmp_args_name_56, 2, tmp_tuple_element_111);
            CHECK_OBJECT(tmp_class_creation_28__class_decl_dict);
            tmp_kwargs_name_56 = tmp_class_creation_28__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 221;
            tmp_assign_source_202 = CALL_FUNCTION(tmp_called_name_56, tmp_args_name_56, tmp_kwargs_name_56);
            Py_DECREF(tmp_args_name_56);
            if (tmp_assign_source_202 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 221;

                goto try_except_handler_84;
            }
            assert(outline_27_var___class__ == NULL);
            outline_27_var___class__ = tmp_assign_source_202;
        }
        CHECK_OBJECT(outline_27_var___class__);
        tmp_assign_source_201 = outline_27_var___class__;
        Py_INCREF(tmp_assign_source_201);
        goto try_return_handler_84;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_84:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221);
        locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221 = NULL;
        goto try_return_handler_83;
        // Exception handler code:
        try_except_handler_84:;
        exception_keeper_type_82 = exception_type;
        exception_keeper_value_82 = exception_value;
        exception_keeper_tb_82 = exception_tb;
        exception_keeper_lineno_82 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221);
        locals_pip$_vendor$urllib3$exceptions$$$class__28_DependencyWarning_221 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_82;
        exception_value = exception_keeper_value_82;
        exception_tb = exception_keeper_tb_82;
        exception_lineno = exception_keeper_lineno_82;

        goto try_except_handler_83;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_83:;
        CHECK_OBJECT(outline_27_var___class__);
        Py_DECREF(outline_27_var___class__);
        outline_27_var___class__ = NULL;
        goto outline_result_28;
        // Exception handler code:
        try_except_handler_83:;
        exception_keeper_type_83 = exception_type;
        exception_keeper_value_83 = exception_value;
        exception_keeper_tb_83 = exception_tb;
        exception_keeper_lineno_83 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_83;
        exception_value = exception_keeper_value_83;
        exception_tb = exception_keeper_tb_83;
        exception_lineno = exception_keeper_lineno_83;

        goto outline_exception_28;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_28:;
        exception_lineno = 221;
        goto try_except_handler_82;
        outline_result_28:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[125], tmp_assign_source_201);
    }
    goto try_end_28;
    // Exception handler code:
    try_except_handler_82:;
    exception_keeper_type_84 = exception_type;
    exception_keeper_value_84 = exception_value;
    exception_keeper_tb_84 = exception_tb;
    exception_keeper_lineno_84 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_28__bases);
    tmp_class_creation_28__bases = NULL;
    Py_XDECREF(tmp_class_creation_28__class_decl_dict);
    tmp_class_creation_28__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_28__metaclass);
    tmp_class_creation_28__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_28__prepared);
    tmp_class_creation_28__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_84;
    exception_value = exception_keeper_value_84;
    exception_tb = exception_keeper_tb_84;
    exception_lineno = exception_keeper_lineno_84;

    goto frame_exception_exit_1;
    // End of try:
    try_end_28:;
    Py_XDECREF(tmp_class_creation_28__bases);
    tmp_class_creation_28__bases = NULL;
    Py_XDECREF(tmp_class_creation_28__class_decl_dict);
    tmp_class_creation_28__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_28__metaclass);
    tmp_class_creation_28__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_28__prepared);
    Py_DECREF(tmp_class_creation_28__prepared);
    tmp_class_creation_28__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_203;
        PyObject *tmp_tuple_element_112;
        tmp_tuple_element_112 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[75]);

        if (unlikely(tmp_tuple_element_112 == NULL)) {
            tmp_tuple_element_112 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[75]);
        }

        if (tmp_tuple_element_112 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        tmp_assign_source_203 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_assign_source_203, 0, tmp_tuple_element_112);
        tmp_tuple_element_112 = PyExc_ValueError;
        PyTuple_SET_ITEM0(tmp_assign_source_203, 1, tmp_tuple_element_112);
        assert(tmp_class_creation_29__bases == NULL);
        tmp_class_creation_29__bases = tmp_assign_source_203;
    }
    {
        PyObject *tmp_assign_source_204;
        tmp_assign_source_204 = PyDict_New();
        assert(tmp_class_creation_29__class_decl_dict == NULL);
        tmp_class_creation_29__class_decl_dict = tmp_assign_source_204;
    }
    {
        PyObject *tmp_assign_source_205;
        PyObject *tmp_metaclass_name_29;
        nuitka_bool tmp_condition_result_113;
        PyObject *tmp_key_name_85;
        PyObject *tmp_dict_arg_name_85;
        PyObject *tmp_dict_arg_name_86;
        PyObject *tmp_key_name_86;
        PyObject *tmp_type_arg_57;
        PyObject *tmp_expression_name_141;
        PyObject *tmp_subscript_name_29;
        PyObject *tmp_bases_name_29;
        tmp_key_name_85 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_29__class_decl_dict);
        tmp_dict_arg_name_85 = tmp_class_creation_29__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_85, tmp_key_name_85);
        assert(!(tmp_res == -1));
        tmp_condition_result_113 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_113 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_29;
        } else {
            goto condexpr_false_29;
        }
        condexpr_true_29:;
        CHECK_OBJECT(tmp_class_creation_29__class_decl_dict);
        tmp_dict_arg_name_86 = tmp_class_creation_29__class_decl_dict;
        tmp_key_name_86 = mod_consts[49];
        tmp_metaclass_name_29 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_86, tmp_key_name_86);
        if (tmp_metaclass_name_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        goto condexpr_end_29;
        condexpr_false_29:;
        CHECK_OBJECT(tmp_class_creation_29__bases);
        tmp_expression_name_141 = tmp_class_creation_29__bases;
        tmp_subscript_name_29 = mod_consts[46];
        tmp_type_arg_57 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_141, tmp_subscript_name_29, 0);
        if (tmp_type_arg_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        tmp_metaclass_name_29 = BUILTIN_TYPE1(tmp_type_arg_57);
        Py_DECREF(tmp_type_arg_57);
        if (tmp_metaclass_name_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        condexpr_end_29:;
        CHECK_OBJECT(tmp_class_creation_29__bases);
        tmp_bases_name_29 = tmp_class_creation_29__bases;
        tmp_assign_source_205 = SELECT_METACLASS(tmp_metaclass_name_29, tmp_bases_name_29);
        Py_DECREF(tmp_metaclass_name_29);
        if (tmp_assign_source_205 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        assert(tmp_class_creation_29__metaclass == NULL);
        tmp_class_creation_29__metaclass = tmp_assign_source_205;
    }
    {
        nuitka_bool tmp_condition_result_114;
        PyObject *tmp_key_name_87;
        PyObject *tmp_dict_arg_name_87;
        tmp_key_name_87 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_29__class_decl_dict);
        tmp_dict_arg_name_87 = tmp_class_creation_29__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_87, tmp_key_name_87);
        assert(!(tmp_res == -1));
        tmp_condition_result_114 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_114 == NUITKA_BOOL_TRUE) {
            goto branch_yes_85;
        } else {
            goto branch_no_85;
        }
    }
    branch_yes_85:;
    CHECK_OBJECT(tmp_class_creation_29__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_29__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 230;

        goto try_except_handler_85;
    }
    branch_no_85:;
    {
        nuitka_bool tmp_condition_result_115;
        PyObject *tmp_expression_name_142;
        CHECK_OBJECT(tmp_class_creation_29__metaclass);
        tmp_expression_name_142 = tmp_class_creation_29__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_142, mod_consts[50]);
        tmp_condition_result_115 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_115 == NUITKA_BOOL_TRUE) {
            goto branch_yes_86;
        } else {
            goto branch_no_86;
        }
    }
    branch_yes_86:;
    {
        PyObject *tmp_assign_source_206;
        PyObject *tmp_called_name_57;
        PyObject *tmp_expression_name_143;
        PyObject *tmp_args_name_57;
        PyObject *tmp_tuple_element_113;
        PyObject *tmp_kwargs_name_57;
        CHECK_OBJECT(tmp_class_creation_29__metaclass);
        tmp_expression_name_143 = tmp_class_creation_29__metaclass;
        tmp_called_name_57 = LOOKUP_ATTRIBUTE(tmp_expression_name_143, mod_consts[50]);
        if (tmp_called_name_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        tmp_tuple_element_113 = mod_consts[127];
        tmp_args_name_57 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_57, 0, tmp_tuple_element_113);
        CHECK_OBJECT(tmp_class_creation_29__bases);
        tmp_tuple_element_113 = tmp_class_creation_29__bases;
        PyTuple_SET_ITEM0(tmp_args_name_57, 1, tmp_tuple_element_113);
        CHECK_OBJECT(tmp_class_creation_29__class_decl_dict);
        tmp_kwargs_name_57 = tmp_class_creation_29__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 230;
        tmp_assign_source_206 = CALL_FUNCTION(tmp_called_name_57, tmp_args_name_57, tmp_kwargs_name_57);
        Py_DECREF(tmp_called_name_57);
        Py_DECREF(tmp_args_name_57);
        if (tmp_assign_source_206 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        assert(tmp_class_creation_29__prepared == NULL);
        tmp_class_creation_29__prepared = tmp_assign_source_206;
    }
    {
        nuitka_bool tmp_condition_result_116;
        PyObject *tmp_operand_name_29;
        PyObject *tmp_expression_name_144;
        CHECK_OBJECT(tmp_class_creation_29__prepared);
        tmp_expression_name_144 = tmp_class_creation_29__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_144, mod_consts[51]);
        tmp_operand_name_29 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_29);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        tmp_condition_result_116 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_116 == NUITKA_BOOL_TRUE) {
            goto branch_yes_87;
        } else {
            goto branch_no_87;
        }
    }
    branch_yes_87:;
    {
        PyObject *tmp_raise_type_29;
        PyObject *tmp_raise_value_29;
        PyObject *tmp_left_name_29;
        PyObject *tmp_right_name_29;
        PyObject *tmp_tuple_element_114;
        PyObject *tmp_getattr_target_29;
        PyObject *tmp_getattr_attr_29;
        PyObject *tmp_getattr_default_29;
        tmp_raise_type_29 = PyExc_TypeError;
        tmp_left_name_29 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_29__metaclass);
        tmp_getattr_target_29 = tmp_class_creation_29__metaclass;
        tmp_getattr_attr_29 = mod_consts[53];
        tmp_getattr_default_29 = mod_consts[54];
        tmp_tuple_element_114 = BUILTIN_GETATTR(tmp_getattr_target_29, tmp_getattr_attr_29, tmp_getattr_default_29);
        if (tmp_tuple_element_114 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        tmp_right_name_29 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_145;
            PyObject *tmp_type_arg_58;
            PyTuple_SET_ITEM(tmp_right_name_29, 0, tmp_tuple_element_114);
            CHECK_OBJECT(tmp_class_creation_29__prepared);
            tmp_type_arg_58 = tmp_class_creation_29__prepared;
            tmp_expression_name_145 = BUILTIN_TYPE1(tmp_type_arg_58);
            assert(!(tmp_expression_name_145 == NULL));
            tmp_tuple_element_114 = LOOKUP_ATTRIBUTE(tmp_expression_name_145, mod_consts[53]);
            Py_DECREF(tmp_expression_name_145);
            if (tmp_tuple_element_114 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 230;

                goto tuple_build_exception_32;
            }
            PyTuple_SET_ITEM(tmp_right_name_29, 1, tmp_tuple_element_114);
        }
        goto tuple_build_noexception_32;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_32:;
        Py_DECREF(tmp_right_name_29);
        goto try_except_handler_85;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_32:;
        tmp_raise_value_29 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_29, tmp_right_name_29);
        Py_DECREF(tmp_right_name_29);
        if (tmp_raise_value_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_85;
        }
        exception_type = tmp_raise_type_29;
        Py_INCREF(tmp_raise_type_29);
        exception_value = tmp_raise_value_29;
        exception_lineno = 230;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_85;
    }
    branch_no_87:;
    goto branch_end_86;
    branch_no_86:;
    {
        PyObject *tmp_assign_source_207;
        tmp_assign_source_207 = PyDict_New();
        assert(tmp_class_creation_29__prepared == NULL);
        tmp_class_creation_29__prepared = tmp_assign_source_207;
    }
    branch_end_86:;
    {
        PyObject *tmp_assign_source_208;
        {
            PyObject *tmp_set_locals_29;
            CHECK_OBJECT(tmp_class_creation_29__prepared);
            tmp_set_locals_29 = tmp_class_creation_29__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230 = tmp_set_locals_29;
            Py_INCREF(tmp_set_locals_29);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_87;
        }
        tmp_dictset_value = mod_consts[128];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_87;
        }
        tmp_dictset_value = mod_consts[127];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 230;

            goto try_except_handler_87;
        }
        {
            PyObject *tmp_assign_source_209;
            PyObject *tmp_called_name_58;
            PyObject *tmp_args_name_58;
            PyObject *tmp_tuple_element_115;
            PyObject *tmp_kwargs_name_58;
            CHECK_OBJECT(tmp_class_creation_29__metaclass);
            tmp_called_name_58 = tmp_class_creation_29__metaclass;
            tmp_tuple_element_115 = mod_consts[127];
            tmp_args_name_58 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_58, 0, tmp_tuple_element_115);
            CHECK_OBJECT(tmp_class_creation_29__bases);
            tmp_tuple_element_115 = tmp_class_creation_29__bases;
            PyTuple_SET_ITEM0(tmp_args_name_58, 1, tmp_tuple_element_115);
            tmp_tuple_element_115 = locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230;
            PyTuple_SET_ITEM0(tmp_args_name_58, 2, tmp_tuple_element_115);
            CHECK_OBJECT(tmp_class_creation_29__class_decl_dict);
            tmp_kwargs_name_58 = tmp_class_creation_29__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 230;
            tmp_assign_source_209 = CALL_FUNCTION(tmp_called_name_58, tmp_args_name_58, tmp_kwargs_name_58);
            Py_DECREF(tmp_args_name_58);
            if (tmp_assign_source_209 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 230;

                goto try_except_handler_87;
            }
            assert(outline_28_var___class__ == NULL);
            outline_28_var___class__ = tmp_assign_source_209;
        }
        CHECK_OBJECT(outline_28_var___class__);
        tmp_assign_source_208 = outline_28_var___class__;
        Py_INCREF(tmp_assign_source_208);
        goto try_return_handler_87;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_87:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230);
        locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230 = NULL;
        goto try_return_handler_86;
        // Exception handler code:
        try_except_handler_87:;
        exception_keeper_type_85 = exception_type;
        exception_keeper_value_85 = exception_value;
        exception_keeper_tb_85 = exception_tb;
        exception_keeper_lineno_85 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230);
        locals_pip$_vendor$urllib3$exceptions$$$class__29_ResponseNotChunked_230 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_85;
        exception_value = exception_keeper_value_85;
        exception_tb = exception_keeper_tb_85;
        exception_lineno = exception_keeper_lineno_85;

        goto try_except_handler_86;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_86:;
        CHECK_OBJECT(outline_28_var___class__);
        Py_DECREF(outline_28_var___class__);
        outline_28_var___class__ = NULL;
        goto outline_result_29;
        // Exception handler code:
        try_except_handler_86:;
        exception_keeper_type_86 = exception_type;
        exception_keeper_value_86 = exception_value;
        exception_keeper_tb_86 = exception_tb;
        exception_keeper_lineno_86 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_86;
        exception_value = exception_keeper_value_86;
        exception_tb = exception_keeper_tb_86;
        exception_lineno = exception_keeper_lineno_86;

        goto outline_exception_29;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_29:;
        exception_lineno = 230;
        goto try_except_handler_85;
        outline_result_29:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[127], tmp_assign_source_208);
    }
    goto try_end_29;
    // Exception handler code:
    try_except_handler_85:;
    exception_keeper_type_87 = exception_type;
    exception_keeper_value_87 = exception_value;
    exception_keeper_tb_87 = exception_tb;
    exception_keeper_lineno_87 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_29__bases);
    tmp_class_creation_29__bases = NULL;
    Py_XDECREF(tmp_class_creation_29__class_decl_dict);
    tmp_class_creation_29__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_29__metaclass);
    tmp_class_creation_29__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_29__prepared);
    tmp_class_creation_29__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_87;
    exception_value = exception_keeper_value_87;
    exception_tb = exception_keeper_tb_87;
    exception_lineno = exception_keeper_lineno_87;

    goto frame_exception_exit_1;
    // End of try:
    try_end_29:;
    Py_XDECREF(tmp_class_creation_29__bases);
    tmp_class_creation_29__bases = NULL;
    Py_XDECREF(tmp_class_creation_29__class_decl_dict);
    tmp_class_creation_29__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_29__metaclass);
    tmp_class_creation_29__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_29__prepared);
    Py_DECREF(tmp_class_creation_29__prepared);
    tmp_class_creation_29__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_210;
        PyObject *tmp_tuple_element_116;
        tmp_tuple_element_116 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_116 == NULL)) {
            tmp_tuple_element_116 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_116 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        tmp_assign_source_210 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_210, 0, tmp_tuple_element_116);
        assert(tmp_class_creation_30__bases == NULL);
        tmp_class_creation_30__bases = tmp_assign_source_210;
    }
    {
        PyObject *tmp_assign_source_211;
        tmp_assign_source_211 = PyDict_New();
        assert(tmp_class_creation_30__class_decl_dict == NULL);
        tmp_class_creation_30__class_decl_dict = tmp_assign_source_211;
    }
    {
        PyObject *tmp_assign_source_212;
        PyObject *tmp_metaclass_name_30;
        nuitka_bool tmp_condition_result_117;
        PyObject *tmp_key_name_88;
        PyObject *tmp_dict_arg_name_88;
        PyObject *tmp_dict_arg_name_89;
        PyObject *tmp_key_name_89;
        PyObject *tmp_type_arg_59;
        PyObject *tmp_expression_name_146;
        PyObject *tmp_subscript_name_30;
        PyObject *tmp_bases_name_30;
        tmp_key_name_88 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_30__class_decl_dict);
        tmp_dict_arg_name_88 = tmp_class_creation_30__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_88, tmp_key_name_88);
        assert(!(tmp_res == -1));
        tmp_condition_result_117 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_117 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_30;
        } else {
            goto condexpr_false_30;
        }
        condexpr_true_30:;
        CHECK_OBJECT(tmp_class_creation_30__class_decl_dict);
        tmp_dict_arg_name_89 = tmp_class_creation_30__class_decl_dict;
        tmp_key_name_89 = mod_consts[49];
        tmp_metaclass_name_30 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_89, tmp_key_name_89);
        if (tmp_metaclass_name_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        goto condexpr_end_30;
        condexpr_false_30:;
        CHECK_OBJECT(tmp_class_creation_30__bases);
        tmp_expression_name_146 = tmp_class_creation_30__bases;
        tmp_subscript_name_30 = mod_consts[46];
        tmp_type_arg_59 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_146, tmp_subscript_name_30, 0);
        if (tmp_type_arg_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        tmp_metaclass_name_30 = BUILTIN_TYPE1(tmp_type_arg_59);
        Py_DECREF(tmp_type_arg_59);
        if (tmp_metaclass_name_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        condexpr_end_30:;
        CHECK_OBJECT(tmp_class_creation_30__bases);
        tmp_bases_name_30 = tmp_class_creation_30__bases;
        tmp_assign_source_212 = SELECT_METACLASS(tmp_metaclass_name_30, tmp_bases_name_30);
        Py_DECREF(tmp_metaclass_name_30);
        if (tmp_assign_source_212 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        assert(tmp_class_creation_30__metaclass == NULL);
        tmp_class_creation_30__metaclass = tmp_assign_source_212;
    }
    {
        nuitka_bool tmp_condition_result_118;
        PyObject *tmp_key_name_90;
        PyObject *tmp_dict_arg_name_90;
        tmp_key_name_90 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_30__class_decl_dict);
        tmp_dict_arg_name_90 = tmp_class_creation_30__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_90, tmp_key_name_90);
        assert(!(tmp_res == -1));
        tmp_condition_result_118 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_118 == NUITKA_BOOL_TRUE) {
            goto branch_yes_88;
        } else {
            goto branch_no_88;
        }
    }
    branch_yes_88:;
    CHECK_OBJECT(tmp_class_creation_30__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_30__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 236;

        goto try_except_handler_88;
    }
    branch_no_88:;
    {
        nuitka_bool tmp_condition_result_119;
        PyObject *tmp_expression_name_147;
        CHECK_OBJECT(tmp_class_creation_30__metaclass);
        tmp_expression_name_147 = tmp_class_creation_30__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_147, mod_consts[50]);
        tmp_condition_result_119 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_119 == NUITKA_BOOL_TRUE) {
            goto branch_yes_89;
        } else {
            goto branch_no_89;
        }
    }
    branch_yes_89:;
    {
        PyObject *tmp_assign_source_213;
        PyObject *tmp_called_name_59;
        PyObject *tmp_expression_name_148;
        PyObject *tmp_args_name_59;
        PyObject *tmp_tuple_element_117;
        PyObject *tmp_kwargs_name_59;
        CHECK_OBJECT(tmp_class_creation_30__metaclass);
        tmp_expression_name_148 = tmp_class_creation_30__metaclass;
        tmp_called_name_59 = LOOKUP_ATTRIBUTE(tmp_expression_name_148, mod_consts[50]);
        if (tmp_called_name_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        tmp_tuple_element_117 = mod_consts[129];
        tmp_args_name_59 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_59, 0, tmp_tuple_element_117);
        CHECK_OBJECT(tmp_class_creation_30__bases);
        tmp_tuple_element_117 = tmp_class_creation_30__bases;
        PyTuple_SET_ITEM0(tmp_args_name_59, 1, tmp_tuple_element_117);
        CHECK_OBJECT(tmp_class_creation_30__class_decl_dict);
        tmp_kwargs_name_59 = tmp_class_creation_30__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 236;
        tmp_assign_source_213 = CALL_FUNCTION(tmp_called_name_59, tmp_args_name_59, tmp_kwargs_name_59);
        Py_DECREF(tmp_called_name_59);
        Py_DECREF(tmp_args_name_59);
        if (tmp_assign_source_213 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        assert(tmp_class_creation_30__prepared == NULL);
        tmp_class_creation_30__prepared = tmp_assign_source_213;
    }
    {
        nuitka_bool tmp_condition_result_120;
        PyObject *tmp_operand_name_30;
        PyObject *tmp_expression_name_149;
        CHECK_OBJECT(tmp_class_creation_30__prepared);
        tmp_expression_name_149 = tmp_class_creation_30__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_149, mod_consts[51]);
        tmp_operand_name_30 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_30);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        tmp_condition_result_120 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_120 == NUITKA_BOOL_TRUE) {
            goto branch_yes_90;
        } else {
            goto branch_no_90;
        }
    }
    branch_yes_90:;
    {
        PyObject *tmp_raise_type_30;
        PyObject *tmp_raise_value_30;
        PyObject *tmp_left_name_30;
        PyObject *tmp_right_name_30;
        PyObject *tmp_tuple_element_118;
        PyObject *tmp_getattr_target_30;
        PyObject *tmp_getattr_attr_30;
        PyObject *tmp_getattr_default_30;
        tmp_raise_type_30 = PyExc_TypeError;
        tmp_left_name_30 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_30__metaclass);
        tmp_getattr_target_30 = tmp_class_creation_30__metaclass;
        tmp_getattr_attr_30 = mod_consts[53];
        tmp_getattr_default_30 = mod_consts[54];
        tmp_tuple_element_118 = BUILTIN_GETATTR(tmp_getattr_target_30, tmp_getattr_attr_30, tmp_getattr_default_30);
        if (tmp_tuple_element_118 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        tmp_right_name_30 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_150;
            PyObject *tmp_type_arg_60;
            PyTuple_SET_ITEM(tmp_right_name_30, 0, tmp_tuple_element_118);
            CHECK_OBJECT(tmp_class_creation_30__prepared);
            tmp_type_arg_60 = tmp_class_creation_30__prepared;
            tmp_expression_name_150 = BUILTIN_TYPE1(tmp_type_arg_60);
            assert(!(tmp_expression_name_150 == NULL));
            tmp_tuple_element_118 = LOOKUP_ATTRIBUTE(tmp_expression_name_150, mod_consts[53]);
            Py_DECREF(tmp_expression_name_150);
            if (tmp_tuple_element_118 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 236;

                goto tuple_build_exception_33;
            }
            PyTuple_SET_ITEM(tmp_right_name_30, 1, tmp_tuple_element_118);
        }
        goto tuple_build_noexception_33;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_33:;
        Py_DECREF(tmp_right_name_30);
        goto try_except_handler_88;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_33:;
        tmp_raise_value_30 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_30, tmp_right_name_30);
        Py_DECREF(tmp_right_name_30);
        if (tmp_raise_value_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_88;
        }
        exception_type = tmp_raise_type_30;
        Py_INCREF(tmp_raise_type_30);
        exception_value = tmp_raise_value_30;
        exception_lineno = 236;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_88;
    }
    branch_no_90:;
    goto branch_end_89;
    branch_no_89:;
    {
        PyObject *tmp_assign_source_214;
        tmp_assign_source_214 = PyDict_New();
        assert(tmp_class_creation_30__prepared == NULL);
        tmp_class_creation_30__prepared = tmp_assign_source_214;
    }
    branch_end_89:;
    {
        PyObject *tmp_assign_source_215;
        {
            PyObject *tmp_set_locals_30;
            CHECK_OBJECT(tmp_class_creation_30__prepared);
            tmp_set_locals_30 = tmp_class_creation_30__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236 = tmp_set_locals_30;
            Py_INCREF(tmp_set_locals_30);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_90;
        }
        tmp_dictset_value = mod_consts[130];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_90;
        }
        tmp_dictset_value = mod_consts[129];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 236;

            goto try_except_handler_90;
        }
        {
            PyObject *tmp_assign_source_216;
            PyObject *tmp_called_name_60;
            PyObject *tmp_args_name_60;
            PyObject *tmp_tuple_element_119;
            PyObject *tmp_kwargs_name_60;
            CHECK_OBJECT(tmp_class_creation_30__metaclass);
            tmp_called_name_60 = tmp_class_creation_30__metaclass;
            tmp_tuple_element_119 = mod_consts[129];
            tmp_args_name_60 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_60, 0, tmp_tuple_element_119);
            CHECK_OBJECT(tmp_class_creation_30__bases);
            tmp_tuple_element_119 = tmp_class_creation_30__bases;
            PyTuple_SET_ITEM0(tmp_args_name_60, 1, tmp_tuple_element_119);
            tmp_tuple_element_119 = locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236;
            PyTuple_SET_ITEM0(tmp_args_name_60, 2, tmp_tuple_element_119);
            CHECK_OBJECT(tmp_class_creation_30__class_decl_dict);
            tmp_kwargs_name_60 = tmp_class_creation_30__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 236;
            tmp_assign_source_216 = CALL_FUNCTION(tmp_called_name_60, tmp_args_name_60, tmp_kwargs_name_60);
            Py_DECREF(tmp_args_name_60);
            if (tmp_assign_source_216 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 236;

                goto try_except_handler_90;
            }
            assert(outline_29_var___class__ == NULL);
            outline_29_var___class__ = tmp_assign_source_216;
        }
        CHECK_OBJECT(outline_29_var___class__);
        tmp_assign_source_215 = outline_29_var___class__;
        Py_INCREF(tmp_assign_source_215);
        goto try_return_handler_90;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_90:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236);
        locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236 = NULL;
        goto try_return_handler_89;
        // Exception handler code:
        try_except_handler_90:;
        exception_keeper_type_88 = exception_type;
        exception_keeper_value_88 = exception_value;
        exception_keeper_tb_88 = exception_tb;
        exception_keeper_lineno_88 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236);
        locals_pip$_vendor$urllib3$exceptions$$$class__30_BodyNotHttplibCompatible_236 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_88;
        exception_value = exception_keeper_value_88;
        exception_tb = exception_keeper_tb_88;
        exception_lineno = exception_keeper_lineno_88;

        goto try_except_handler_89;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_89:;
        CHECK_OBJECT(outline_29_var___class__);
        Py_DECREF(outline_29_var___class__);
        outline_29_var___class__ = NULL;
        goto outline_result_30;
        // Exception handler code:
        try_except_handler_89:;
        exception_keeper_type_89 = exception_type;
        exception_keeper_value_89 = exception_value;
        exception_keeper_tb_89 = exception_tb;
        exception_keeper_lineno_89 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_89;
        exception_value = exception_keeper_value_89;
        exception_tb = exception_keeper_tb_89;
        exception_lineno = exception_keeper_lineno_89;

        goto outline_exception_30;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_30:;
        exception_lineno = 236;
        goto try_except_handler_88;
        outline_result_30:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[129], tmp_assign_source_215);
    }
    goto try_end_30;
    // Exception handler code:
    try_except_handler_88:;
    exception_keeper_type_90 = exception_type;
    exception_keeper_value_90 = exception_value;
    exception_keeper_tb_90 = exception_tb;
    exception_keeper_lineno_90 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_30__bases);
    tmp_class_creation_30__bases = NULL;
    Py_XDECREF(tmp_class_creation_30__class_decl_dict);
    tmp_class_creation_30__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_30__metaclass);
    tmp_class_creation_30__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_30__prepared);
    tmp_class_creation_30__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_90;
    exception_value = exception_keeper_value_90;
    exception_tb = exception_keeper_tb_90;
    exception_lineno = exception_keeper_lineno_90;

    goto frame_exception_exit_1;
    // End of try:
    try_end_30:;
    Py_XDECREF(tmp_class_creation_30__bases);
    tmp_class_creation_30__bases = NULL;
    Py_XDECREF(tmp_class_creation_30__class_decl_dict);
    tmp_class_creation_30__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_30__metaclass);
    tmp_class_creation_30__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_30__prepared);
    Py_DECREF(tmp_class_creation_30__prepared);
    tmp_class_creation_30__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_217;
        PyObject *tmp_tuple_element_120;
        tmp_tuple_element_120 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_120 == NULL)) {
            tmp_tuple_element_120 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_120 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        tmp_assign_source_217 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_assign_source_217, 0, tmp_tuple_element_120);
        tmp_tuple_element_120 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[47]);

        if (unlikely(tmp_tuple_element_120 == NULL)) {
            tmp_tuple_element_120 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[47]);
        }

        if (tmp_tuple_element_120 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto tuple_build_exception_34;
        }
        PyTuple_SET_ITEM0(tmp_assign_source_217, 1, tmp_tuple_element_120);
        goto tuple_build_noexception_34;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_34:;
        Py_DECREF(tmp_assign_source_217);
        goto try_except_handler_91;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_34:;
        assert(tmp_class_creation_31__bases == NULL);
        tmp_class_creation_31__bases = tmp_assign_source_217;
    }
    {
        PyObject *tmp_assign_source_218;
        tmp_assign_source_218 = PyDict_New();
        assert(tmp_class_creation_31__class_decl_dict == NULL);
        tmp_class_creation_31__class_decl_dict = tmp_assign_source_218;
    }
    {
        PyObject *tmp_assign_source_219;
        PyObject *tmp_metaclass_name_31;
        nuitka_bool tmp_condition_result_121;
        PyObject *tmp_key_name_91;
        PyObject *tmp_dict_arg_name_91;
        PyObject *tmp_dict_arg_name_92;
        PyObject *tmp_key_name_92;
        PyObject *tmp_type_arg_61;
        PyObject *tmp_expression_name_151;
        PyObject *tmp_subscript_name_31;
        PyObject *tmp_bases_name_31;
        tmp_key_name_91 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_31__class_decl_dict);
        tmp_dict_arg_name_91 = tmp_class_creation_31__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_91, tmp_key_name_91);
        assert(!(tmp_res == -1));
        tmp_condition_result_121 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_121 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_31;
        } else {
            goto condexpr_false_31;
        }
        condexpr_true_31:;
        CHECK_OBJECT(tmp_class_creation_31__class_decl_dict);
        tmp_dict_arg_name_92 = tmp_class_creation_31__class_decl_dict;
        tmp_key_name_92 = mod_consts[49];
        tmp_metaclass_name_31 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_92, tmp_key_name_92);
        if (tmp_metaclass_name_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        goto condexpr_end_31;
        condexpr_false_31:;
        CHECK_OBJECT(tmp_class_creation_31__bases);
        tmp_expression_name_151 = tmp_class_creation_31__bases;
        tmp_subscript_name_31 = mod_consts[46];
        tmp_type_arg_61 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_151, tmp_subscript_name_31, 0);
        if (tmp_type_arg_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        tmp_metaclass_name_31 = BUILTIN_TYPE1(tmp_type_arg_61);
        Py_DECREF(tmp_type_arg_61);
        if (tmp_metaclass_name_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        condexpr_end_31:;
        CHECK_OBJECT(tmp_class_creation_31__bases);
        tmp_bases_name_31 = tmp_class_creation_31__bases;
        tmp_assign_source_219 = SELECT_METACLASS(tmp_metaclass_name_31, tmp_bases_name_31);
        Py_DECREF(tmp_metaclass_name_31);
        if (tmp_assign_source_219 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        assert(tmp_class_creation_31__metaclass == NULL);
        tmp_class_creation_31__metaclass = tmp_assign_source_219;
    }
    {
        nuitka_bool tmp_condition_result_122;
        PyObject *tmp_key_name_93;
        PyObject *tmp_dict_arg_name_93;
        tmp_key_name_93 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_31__class_decl_dict);
        tmp_dict_arg_name_93 = tmp_class_creation_31__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_93, tmp_key_name_93);
        assert(!(tmp_res == -1));
        tmp_condition_result_122 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_122 == NUITKA_BOOL_TRUE) {
            goto branch_yes_91;
        } else {
            goto branch_no_91;
        }
    }
    branch_yes_91:;
    CHECK_OBJECT(tmp_class_creation_31__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_31__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 245;

        goto try_except_handler_91;
    }
    branch_no_91:;
    {
        nuitka_bool tmp_condition_result_123;
        PyObject *tmp_expression_name_152;
        CHECK_OBJECT(tmp_class_creation_31__metaclass);
        tmp_expression_name_152 = tmp_class_creation_31__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_152, mod_consts[50]);
        tmp_condition_result_123 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_123 == NUITKA_BOOL_TRUE) {
            goto branch_yes_92;
        } else {
            goto branch_no_92;
        }
    }
    branch_yes_92:;
    {
        PyObject *tmp_assign_source_220;
        PyObject *tmp_called_name_61;
        PyObject *tmp_expression_name_153;
        PyObject *tmp_args_name_61;
        PyObject *tmp_tuple_element_121;
        PyObject *tmp_kwargs_name_61;
        CHECK_OBJECT(tmp_class_creation_31__metaclass);
        tmp_expression_name_153 = tmp_class_creation_31__metaclass;
        tmp_called_name_61 = LOOKUP_ATTRIBUTE(tmp_expression_name_153, mod_consts[50]);
        if (tmp_called_name_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        tmp_tuple_element_121 = mod_consts[20];
        tmp_args_name_61 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_61, 0, tmp_tuple_element_121);
        CHECK_OBJECT(tmp_class_creation_31__bases);
        tmp_tuple_element_121 = tmp_class_creation_31__bases;
        PyTuple_SET_ITEM0(tmp_args_name_61, 1, tmp_tuple_element_121);
        CHECK_OBJECT(tmp_class_creation_31__class_decl_dict);
        tmp_kwargs_name_61 = tmp_class_creation_31__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 245;
        tmp_assign_source_220 = CALL_FUNCTION(tmp_called_name_61, tmp_args_name_61, tmp_kwargs_name_61);
        Py_DECREF(tmp_called_name_61);
        Py_DECREF(tmp_args_name_61);
        if (tmp_assign_source_220 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        assert(tmp_class_creation_31__prepared == NULL);
        tmp_class_creation_31__prepared = tmp_assign_source_220;
    }
    {
        nuitka_bool tmp_condition_result_124;
        PyObject *tmp_operand_name_31;
        PyObject *tmp_expression_name_154;
        CHECK_OBJECT(tmp_class_creation_31__prepared);
        tmp_expression_name_154 = tmp_class_creation_31__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_154, mod_consts[51]);
        tmp_operand_name_31 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_31);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        tmp_condition_result_124 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_124 == NUITKA_BOOL_TRUE) {
            goto branch_yes_93;
        } else {
            goto branch_no_93;
        }
    }
    branch_yes_93:;
    {
        PyObject *tmp_raise_type_31;
        PyObject *tmp_raise_value_31;
        PyObject *tmp_left_name_31;
        PyObject *tmp_right_name_31;
        PyObject *tmp_tuple_element_122;
        PyObject *tmp_getattr_target_31;
        PyObject *tmp_getattr_attr_31;
        PyObject *tmp_getattr_default_31;
        tmp_raise_type_31 = PyExc_TypeError;
        tmp_left_name_31 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_31__metaclass);
        tmp_getattr_target_31 = tmp_class_creation_31__metaclass;
        tmp_getattr_attr_31 = mod_consts[53];
        tmp_getattr_default_31 = mod_consts[54];
        tmp_tuple_element_122 = BUILTIN_GETATTR(tmp_getattr_target_31, tmp_getattr_attr_31, tmp_getattr_default_31);
        if (tmp_tuple_element_122 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        tmp_right_name_31 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_155;
            PyObject *tmp_type_arg_62;
            PyTuple_SET_ITEM(tmp_right_name_31, 0, tmp_tuple_element_122);
            CHECK_OBJECT(tmp_class_creation_31__prepared);
            tmp_type_arg_62 = tmp_class_creation_31__prepared;
            tmp_expression_name_155 = BUILTIN_TYPE1(tmp_type_arg_62);
            assert(!(tmp_expression_name_155 == NULL));
            tmp_tuple_element_122 = LOOKUP_ATTRIBUTE(tmp_expression_name_155, mod_consts[53]);
            Py_DECREF(tmp_expression_name_155);
            if (tmp_tuple_element_122 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 245;

                goto tuple_build_exception_35;
            }
            PyTuple_SET_ITEM(tmp_right_name_31, 1, tmp_tuple_element_122);
        }
        goto tuple_build_noexception_35;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_35:;
        Py_DECREF(tmp_right_name_31);
        goto try_except_handler_91;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_35:;
        tmp_raise_value_31 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_31, tmp_right_name_31);
        Py_DECREF(tmp_right_name_31);
        if (tmp_raise_value_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_91;
        }
        exception_type = tmp_raise_type_31;
        Py_INCREF(tmp_raise_type_31);
        exception_value = tmp_raise_value_31;
        exception_lineno = 245;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_91;
    }
    branch_no_93:;
    goto branch_end_92;
    branch_no_92:;
    {
        PyObject *tmp_assign_source_221;
        tmp_assign_source_221 = PyDict_New();
        assert(tmp_class_creation_31__prepared == NULL);
        tmp_class_creation_31__prepared = tmp_assign_source_221;
    }
    branch_end_92:;
    {
        PyObject *tmp_assign_source_222;
        {
            PyObject *tmp_set_locals_31;
            CHECK_OBJECT(tmp_class_creation_31__prepared);
            tmp_set_locals_31 = tmp_class_creation_31__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245 = tmp_set_locals_31;
            Py_INCREF(tmp_set_locals_31);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_93;
        }
        tmp_dictset_value = mod_consts[131];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_93;
        }
        tmp_dictset_value = mod_consts[20];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 245;

            goto try_except_handler_93;
        }
        if (isFrameUnusable(cache_frame_90ad096779fac202a1b1540ce0977ce0_10)) {
            Py_XDECREF(cache_frame_90ad096779fac202a1b1540ce0977ce0_10);

#if _DEBUG_REFCOUNTS
            if (cache_frame_90ad096779fac202a1b1540ce0977ce0_10 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_90ad096779fac202a1b1540ce0977ce0_10 = MAKE_FUNCTION_FRAME(codeobj_90ad096779fac202a1b1540ce0977ce0, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_90ad096779fac202a1b1540ce0977ce0_10->m_type_description == NULL);
        frame_90ad096779fac202a1b1540ce0977ce0_10 = cache_frame_90ad096779fac202a1b1540ce0977ce0_10;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_90ad096779fac202a1b1540ce0977ce0_10);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_90ad096779fac202a1b1540ce0977ce0_10) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__10___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 253;
            type_description_2 = "o";
            goto frame_exception_exit_10;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__11___repr__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245, mod_consts[133], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 256;
            type_description_2 = "o";
            goto frame_exception_exit_10;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_90ad096779fac202a1b1540ce0977ce0_10);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_9;

        frame_exception_exit_10:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_90ad096779fac202a1b1540ce0977ce0_10);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_90ad096779fac202a1b1540ce0977ce0_10, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_90ad096779fac202a1b1540ce0977ce0_10->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_90ad096779fac202a1b1540ce0977ce0_10, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_90ad096779fac202a1b1540ce0977ce0_10,
            type_description_2,
            outline_30_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_90ad096779fac202a1b1540ce0977ce0_10 == cache_frame_90ad096779fac202a1b1540ce0977ce0_10) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_90ad096779fac202a1b1540ce0977ce0_10);
            cache_frame_90ad096779fac202a1b1540ce0977ce0_10 = NULL;
        }

        assertFrameObject(frame_90ad096779fac202a1b1540ce0977ce0_10);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_9;

        frame_no_exception_9:;
        goto skip_nested_handling_9;
        nested_frame_exit_9:;

        goto try_except_handler_93;
        skip_nested_handling_9:;
        {
            PyObject *tmp_assign_source_223;
            PyObject *tmp_called_name_62;
            PyObject *tmp_args_name_62;
            PyObject *tmp_tuple_element_123;
            PyObject *tmp_kwargs_name_62;
            CHECK_OBJECT(tmp_class_creation_31__metaclass);
            tmp_called_name_62 = tmp_class_creation_31__metaclass;
            tmp_tuple_element_123 = mod_consts[20];
            tmp_args_name_62 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_62, 0, tmp_tuple_element_123);
            CHECK_OBJECT(tmp_class_creation_31__bases);
            tmp_tuple_element_123 = tmp_class_creation_31__bases;
            PyTuple_SET_ITEM0(tmp_args_name_62, 1, tmp_tuple_element_123);
            tmp_tuple_element_123 = locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245;
            PyTuple_SET_ITEM0(tmp_args_name_62, 2, tmp_tuple_element_123);
            CHECK_OBJECT(tmp_class_creation_31__class_decl_dict);
            tmp_kwargs_name_62 = tmp_class_creation_31__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 245;
            tmp_assign_source_223 = CALL_FUNCTION(tmp_called_name_62, tmp_args_name_62, tmp_kwargs_name_62);
            Py_DECREF(tmp_args_name_62);
            if (tmp_assign_source_223 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 245;

                goto try_except_handler_93;
            }
            assert(outline_30_var___class__ == NULL);
            outline_30_var___class__ = tmp_assign_source_223;
        }
        CHECK_OBJECT(outline_30_var___class__);
        tmp_assign_source_222 = outline_30_var___class__;
        Py_INCREF(tmp_assign_source_222);
        goto try_return_handler_93;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_93:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245);
        locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245 = NULL;
        goto try_return_handler_92;
        // Exception handler code:
        try_except_handler_93:;
        exception_keeper_type_91 = exception_type;
        exception_keeper_value_91 = exception_value;
        exception_keeper_tb_91 = exception_tb;
        exception_keeper_lineno_91 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245);
        locals_pip$_vendor$urllib3$exceptions$$$class__31_IncompleteRead_245 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_91;
        exception_value = exception_keeper_value_91;
        exception_tb = exception_keeper_tb_91;
        exception_lineno = exception_keeper_lineno_91;

        goto try_except_handler_92;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_92:;
        CHECK_OBJECT(outline_30_var___class__);
        Py_DECREF(outline_30_var___class__);
        outline_30_var___class__ = NULL;
        goto outline_result_31;
        // Exception handler code:
        try_except_handler_92:;
        exception_keeper_type_92 = exception_type;
        exception_keeper_value_92 = exception_value;
        exception_keeper_tb_92 = exception_tb;
        exception_keeper_lineno_92 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_92;
        exception_value = exception_keeper_value_92;
        exception_tb = exception_keeper_tb_92;
        exception_lineno = exception_keeper_lineno_92;

        goto outline_exception_31;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_31:;
        exception_lineno = 245;
        goto try_except_handler_91;
        outline_result_31:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[20], tmp_assign_source_222);
    }
    goto try_end_31;
    // Exception handler code:
    try_except_handler_91:;
    exception_keeper_type_93 = exception_type;
    exception_keeper_value_93 = exception_value;
    exception_keeper_tb_93 = exception_tb;
    exception_keeper_lineno_93 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_31__bases);
    tmp_class_creation_31__bases = NULL;
    Py_XDECREF(tmp_class_creation_31__class_decl_dict);
    tmp_class_creation_31__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_31__metaclass);
    tmp_class_creation_31__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_31__prepared);
    tmp_class_creation_31__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_93;
    exception_value = exception_keeper_value_93;
    exception_tb = exception_keeper_tb_93;
    exception_lineno = exception_keeper_lineno_93;

    goto frame_exception_exit_1;
    // End of try:
    try_end_31:;
    Py_XDECREF(tmp_class_creation_31__bases);
    tmp_class_creation_31__bases = NULL;
    Py_XDECREF(tmp_class_creation_31__class_decl_dict);
    tmp_class_creation_31__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_31__metaclass);
    tmp_class_creation_31__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_31__prepared);
    Py_DECREF(tmp_class_creation_31__prepared);
    tmp_class_creation_31__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_224;
        PyObject *tmp_tuple_element_124;
        tmp_tuple_element_124 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_124 == NULL)) {
            tmp_tuple_element_124 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_124 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        tmp_assign_source_224 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_assign_source_224, 0, tmp_tuple_element_124);
        tmp_tuple_element_124 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[47]);

        if (unlikely(tmp_tuple_element_124 == NULL)) {
            tmp_tuple_element_124 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[47]);
        }

        if (tmp_tuple_element_124 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto tuple_build_exception_36;
        }
        PyTuple_SET_ITEM0(tmp_assign_source_224, 1, tmp_tuple_element_124);
        goto tuple_build_noexception_36;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_36:;
        Py_DECREF(tmp_assign_source_224);
        goto try_except_handler_94;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_36:;
        assert(tmp_class_creation_32__bases == NULL);
        tmp_class_creation_32__bases = tmp_assign_source_224;
    }
    {
        PyObject *tmp_assign_source_225;
        tmp_assign_source_225 = PyDict_New();
        assert(tmp_class_creation_32__class_decl_dict == NULL);
        tmp_class_creation_32__class_decl_dict = tmp_assign_source_225;
    }
    {
        PyObject *tmp_assign_source_226;
        PyObject *tmp_metaclass_name_32;
        nuitka_bool tmp_condition_result_125;
        PyObject *tmp_key_name_94;
        PyObject *tmp_dict_arg_name_94;
        PyObject *tmp_dict_arg_name_95;
        PyObject *tmp_key_name_95;
        PyObject *tmp_type_arg_63;
        PyObject *tmp_expression_name_156;
        PyObject *tmp_subscript_name_32;
        PyObject *tmp_bases_name_32;
        tmp_key_name_94 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_32__class_decl_dict);
        tmp_dict_arg_name_94 = tmp_class_creation_32__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_94, tmp_key_name_94);
        assert(!(tmp_res == -1));
        tmp_condition_result_125 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_125 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_32;
        } else {
            goto condexpr_false_32;
        }
        condexpr_true_32:;
        CHECK_OBJECT(tmp_class_creation_32__class_decl_dict);
        tmp_dict_arg_name_95 = tmp_class_creation_32__class_decl_dict;
        tmp_key_name_95 = mod_consts[49];
        tmp_metaclass_name_32 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_95, tmp_key_name_95);
        if (tmp_metaclass_name_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        goto condexpr_end_32;
        condexpr_false_32:;
        CHECK_OBJECT(tmp_class_creation_32__bases);
        tmp_expression_name_156 = tmp_class_creation_32__bases;
        tmp_subscript_name_32 = mod_consts[46];
        tmp_type_arg_63 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_156, tmp_subscript_name_32, 0);
        if (tmp_type_arg_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        tmp_metaclass_name_32 = BUILTIN_TYPE1(tmp_type_arg_63);
        Py_DECREF(tmp_type_arg_63);
        if (tmp_metaclass_name_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        condexpr_end_32:;
        CHECK_OBJECT(tmp_class_creation_32__bases);
        tmp_bases_name_32 = tmp_class_creation_32__bases;
        tmp_assign_source_226 = SELECT_METACLASS(tmp_metaclass_name_32, tmp_bases_name_32);
        Py_DECREF(tmp_metaclass_name_32);
        if (tmp_assign_source_226 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        assert(tmp_class_creation_32__metaclass == NULL);
        tmp_class_creation_32__metaclass = tmp_assign_source_226;
    }
    {
        nuitka_bool tmp_condition_result_126;
        PyObject *tmp_key_name_96;
        PyObject *tmp_dict_arg_name_96;
        tmp_key_name_96 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_32__class_decl_dict);
        tmp_dict_arg_name_96 = tmp_class_creation_32__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_96, tmp_key_name_96);
        assert(!(tmp_res == -1));
        tmp_condition_result_126 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_126 == NUITKA_BOOL_TRUE) {
            goto branch_yes_94;
        } else {
            goto branch_no_94;
        }
    }
    branch_yes_94:;
    CHECK_OBJECT(tmp_class_creation_32__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_32__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 263;

        goto try_except_handler_94;
    }
    branch_no_94:;
    {
        nuitka_bool tmp_condition_result_127;
        PyObject *tmp_expression_name_157;
        CHECK_OBJECT(tmp_class_creation_32__metaclass);
        tmp_expression_name_157 = tmp_class_creation_32__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_157, mod_consts[50]);
        tmp_condition_result_127 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_127 == NUITKA_BOOL_TRUE) {
            goto branch_yes_95;
        } else {
            goto branch_no_95;
        }
    }
    branch_yes_95:;
    {
        PyObject *tmp_assign_source_227;
        PyObject *tmp_called_name_63;
        PyObject *tmp_expression_name_158;
        PyObject *tmp_args_name_63;
        PyObject *tmp_tuple_element_125;
        PyObject *tmp_kwargs_name_63;
        CHECK_OBJECT(tmp_class_creation_32__metaclass);
        tmp_expression_name_158 = tmp_class_creation_32__metaclass;
        tmp_called_name_63 = LOOKUP_ATTRIBUTE(tmp_expression_name_158, mod_consts[50]);
        if (tmp_called_name_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        tmp_tuple_element_125 = mod_consts[24];
        tmp_args_name_63 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_63, 0, tmp_tuple_element_125);
        CHECK_OBJECT(tmp_class_creation_32__bases);
        tmp_tuple_element_125 = tmp_class_creation_32__bases;
        PyTuple_SET_ITEM0(tmp_args_name_63, 1, tmp_tuple_element_125);
        CHECK_OBJECT(tmp_class_creation_32__class_decl_dict);
        tmp_kwargs_name_63 = tmp_class_creation_32__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 263;
        tmp_assign_source_227 = CALL_FUNCTION(tmp_called_name_63, tmp_args_name_63, tmp_kwargs_name_63);
        Py_DECREF(tmp_called_name_63);
        Py_DECREF(tmp_args_name_63);
        if (tmp_assign_source_227 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        assert(tmp_class_creation_32__prepared == NULL);
        tmp_class_creation_32__prepared = tmp_assign_source_227;
    }
    {
        nuitka_bool tmp_condition_result_128;
        PyObject *tmp_operand_name_32;
        PyObject *tmp_expression_name_159;
        CHECK_OBJECT(tmp_class_creation_32__prepared);
        tmp_expression_name_159 = tmp_class_creation_32__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_159, mod_consts[51]);
        tmp_operand_name_32 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_32);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        tmp_condition_result_128 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_128 == NUITKA_BOOL_TRUE) {
            goto branch_yes_96;
        } else {
            goto branch_no_96;
        }
    }
    branch_yes_96:;
    {
        PyObject *tmp_raise_type_32;
        PyObject *tmp_raise_value_32;
        PyObject *tmp_left_name_32;
        PyObject *tmp_right_name_32;
        PyObject *tmp_tuple_element_126;
        PyObject *tmp_getattr_target_32;
        PyObject *tmp_getattr_attr_32;
        PyObject *tmp_getattr_default_32;
        tmp_raise_type_32 = PyExc_TypeError;
        tmp_left_name_32 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_32__metaclass);
        tmp_getattr_target_32 = tmp_class_creation_32__metaclass;
        tmp_getattr_attr_32 = mod_consts[53];
        tmp_getattr_default_32 = mod_consts[54];
        tmp_tuple_element_126 = BUILTIN_GETATTR(tmp_getattr_target_32, tmp_getattr_attr_32, tmp_getattr_default_32);
        if (tmp_tuple_element_126 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        tmp_right_name_32 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_160;
            PyObject *tmp_type_arg_64;
            PyTuple_SET_ITEM(tmp_right_name_32, 0, tmp_tuple_element_126);
            CHECK_OBJECT(tmp_class_creation_32__prepared);
            tmp_type_arg_64 = tmp_class_creation_32__prepared;
            tmp_expression_name_160 = BUILTIN_TYPE1(tmp_type_arg_64);
            assert(!(tmp_expression_name_160 == NULL));
            tmp_tuple_element_126 = LOOKUP_ATTRIBUTE(tmp_expression_name_160, mod_consts[53]);
            Py_DECREF(tmp_expression_name_160);
            if (tmp_tuple_element_126 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 263;

                goto tuple_build_exception_37;
            }
            PyTuple_SET_ITEM(tmp_right_name_32, 1, tmp_tuple_element_126);
        }
        goto tuple_build_noexception_37;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_37:;
        Py_DECREF(tmp_right_name_32);
        goto try_except_handler_94;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_37:;
        tmp_raise_value_32 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_32, tmp_right_name_32);
        Py_DECREF(tmp_right_name_32);
        if (tmp_raise_value_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_94;
        }
        exception_type = tmp_raise_type_32;
        Py_INCREF(tmp_raise_type_32);
        exception_value = tmp_raise_value_32;
        exception_lineno = 263;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_94;
    }
    branch_no_96:;
    goto branch_end_95;
    branch_no_95:;
    {
        PyObject *tmp_assign_source_228;
        tmp_assign_source_228 = PyDict_New();
        assert(tmp_class_creation_32__prepared == NULL);
        tmp_class_creation_32__prepared = tmp_assign_source_228;
    }
    branch_end_95:;
    {
        PyObject *tmp_assign_source_229;
        {
            PyObject *tmp_set_locals_32;
            CHECK_OBJECT(tmp_class_creation_32__prepared);
            tmp_set_locals_32 = tmp_class_creation_32__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263 = tmp_set_locals_32;
            Py_INCREF(tmp_set_locals_32);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_96;
        }
        tmp_dictset_value = mod_consts[135];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_96;
        }
        tmp_dictset_value = mod_consts[24];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 263;

            goto try_except_handler_96;
        }
        if (isFrameUnusable(cache_frame_546b5f9419c19af80309bdfac001c04f_11)) {
            Py_XDECREF(cache_frame_546b5f9419c19af80309bdfac001c04f_11);

#if _DEBUG_REFCOUNTS
            if (cache_frame_546b5f9419c19af80309bdfac001c04f_11 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_546b5f9419c19af80309bdfac001c04f_11 = MAKE_FUNCTION_FRAME(codeobj_546b5f9419c19af80309bdfac001c04f, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_546b5f9419c19af80309bdfac001c04f_11->m_type_description == NULL);
        frame_546b5f9419c19af80309bdfac001c04f_11 = cache_frame_546b5f9419c19af80309bdfac001c04f_11;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_546b5f9419c19af80309bdfac001c04f_11);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_546b5f9419c19af80309bdfac001c04f_11) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__12___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 266;
            type_description_2 = "o";
            goto frame_exception_exit_11;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__13___repr__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263, mod_consts[133], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 273;
            type_description_2 = "o";
            goto frame_exception_exit_11;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_546b5f9419c19af80309bdfac001c04f_11);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_10;

        frame_exception_exit_11:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_546b5f9419c19af80309bdfac001c04f_11);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_546b5f9419c19af80309bdfac001c04f_11, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_546b5f9419c19af80309bdfac001c04f_11->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_546b5f9419c19af80309bdfac001c04f_11, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_546b5f9419c19af80309bdfac001c04f_11,
            type_description_2,
            outline_31_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_546b5f9419c19af80309bdfac001c04f_11 == cache_frame_546b5f9419c19af80309bdfac001c04f_11) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_546b5f9419c19af80309bdfac001c04f_11);
            cache_frame_546b5f9419c19af80309bdfac001c04f_11 = NULL;
        }

        assertFrameObject(frame_546b5f9419c19af80309bdfac001c04f_11);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_10;

        frame_no_exception_10:;
        goto skip_nested_handling_10;
        nested_frame_exit_10:;

        goto try_except_handler_96;
        skip_nested_handling_10:;
        {
            PyObject *tmp_assign_source_230;
            PyObject *tmp_called_name_64;
            PyObject *tmp_args_name_64;
            PyObject *tmp_tuple_element_127;
            PyObject *tmp_kwargs_name_64;
            CHECK_OBJECT(tmp_class_creation_32__metaclass);
            tmp_called_name_64 = tmp_class_creation_32__metaclass;
            tmp_tuple_element_127 = mod_consts[24];
            tmp_args_name_64 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_64, 0, tmp_tuple_element_127);
            CHECK_OBJECT(tmp_class_creation_32__bases);
            tmp_tuple_element_127 = tmp_class_creation_32__bases;
            PyTuple_SET_ITEM0(tmp_args_name_64, 1, tmp_tuple_element_127);
            tmp_tuple_element_127 = locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263;
            PyTuple_SET_ITEM0(tmp_args_name_64, 2, tmp_tuple_element_127);
            CHECK_OBJECT(tmp_class_creation_32__class_decl_dict);
            tmp_kwargs_name_64 = tmp_class_creation_32__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 263;
            tmp_assign_source_230 = CALL_FUNCTION(tmp_called_name_64, tmp_args_name_64, tmp_kwargs_name_64);
            Py_DECREF(tmp_args_name_64);
            if (tmp_assign_source_230 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 263;

                goto try_except_handler_96;
            }
            assert(outline_31_var___class__ == NULL);
            outline_31_var___class__ = tmp_assign_source_230;
        }
        CHECK_OBJECT(outline_31_var___class__);
        tmp_assign_source_229 = outline_31_var___class__;
        Py_INCREF(tmp_assign_source_229);
        goto try_return_handler_96;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_96:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263);
        locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263 = NULL;
        goto try_return_handler_95;
        // Exception handler code:
        try_except_handler_96:;
        exception_keeper_type_94 = exception_type;
        exception_keeper_value_94 = exception_value;
        exception_keeper_tb_94 = exception_tb;
        exception_keeper_lineno_94 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263);
        locals_pip$_vendor$urllib3$exceptions$$$class__32_InvalidChunkLength_263 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_94;
        exception_value = exception_keeper_value_94;
        exception_tb = exception_keeper_tb_94;
        exception_lineno = exception_keeper_lineno_94;

        goto try_except_handler_95;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_95:;
        CHECK_OBJECT(outline_31_var___class__);
        Py_DECREF(outline_31_var___class__);
        outline_31_var___class__ = NULL;
        goto outline_result_32;
        // Exception handler code:
        try_except_handler_95:;
        exception_keeper_type_95 = exception_type;
        exception_keeper_value_95 = exception_value;
        exception_keeper_tb_95 = exception_tb;
        exception_keeper_lineno_95 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_95;
        exception_value = exception_keeper_value_95;
        exception_tb = exception_keeper_tb_95;
        exception_lineno = exception_keeper_lineno_95;

        goto outline_exception_32;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_32:;
        exception_lineno = 263;
        goto try_except_handler_94;
        outline_result_32:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[24], tmp_assign_source_229);
    }
    goto try_end_32;
    // Exception handler code:
    try_except_handler_94:;
    exception_keeper_type_96 = exception_type;
    exception_keeper_value_96 = exception_value;
    exception_keeper_tb_96 = exception_tb;
    exception_keeper_lineno_96 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_32__bases);
    tmp_class_creation_32__bases = NULL;
    Py_XDECREF(tmp_class_creation_32__class_decl_dict);
    tmp_class_creation_32__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_32__metaclass);
    tmp_class_creation_32__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_32__prepared);
    tmp_class_creation_32__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_96;
    exception_value = exception_keeper_value_96;
    exception_tb = exception_keeper_tb_96;
    exception_lineno = exception_keeper_lineno_96;

    goto frame_exception_exit_1;
    // End of try:
    try_end_32:;
    Py_XDECREF(tmp_class_creation_32__bases);
    tmp_class_creation_32__bases = NULL;
    Py_XDECREF(tmp_class_creation_32__class_decl_dict);
    tmp_class_creation_32__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_32__metaclass);
    tmp_class_creation_32__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_32__prepared);
    Py_DECREF(tmp_class_creation_32__prepared);
    tmp_class_creation_32__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_231;
        PyObject *tmp_tuple_element_128;
        tmp_tuple_element_128 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_128 == NULL)) {
            tmp_tuple_element_128 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_128 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        tmp_assign_source_231 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_231, 0, tmp_tuple_element_128);
        assert(tmp_class_creation_33__bases == NULL);
        tmp_class_creation_33__bases = tmp_assign_source_231;
    }
    {
        PyObject *tmp_assign_source_232;
        tmp_assign_source_232 = PyDict_New();
        assert(tmp_class_creation_33__class_decl_dict == NULL);
        tmp_class_creation_33__class_decl_dict = tmp_assign_source_232;
    }
    {
        PyObject *tmp_assign_source_233;
        PyObject *tmp_metaclass_name_33;
        nuitka_bool tmp_condition_result_129;
        PyObject *tmp_key_name_97;
        PyObject *tmp_dict_arg_name_97;
        PyObject *tmp_dict_arg_name_98;
        PyObject *tmp_key_name_98;
        PyObject *tmp_type_arg_65;
        PyObject *tmp_expression_name_161;
        PyObject *tmp_subscript_name_33;
        PyObject *tmp_bases_name_33;
        tmp_key_name_97 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_33__class_decl_dict);
        tmp_dict_arg_name_97 = tmp_class_creation_33__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_97, tmp_key_name_97);
        assert(!(tmp_res == -1));
        tmp_condition_result_129 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_129 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_33;
        } else {
            goto condexpr_false_33;
        }
        condexpr_true_33:;
        CHECK_OBJECT(tmp_class_creation_33__class_decl_dict);
        tmp_dict_arg_name_98 = tmp_class_creation_33__class_decl_dict;
        tmp_key_name_98 = mod_consts[49];
        tmp_metaclass_name_33 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_98, tmp_key_name_98);
        if (tmp_metaclass_name_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        goto condexpr_end_33;
        condexpr_false_33:;
        CHECK_OBJECT(tmp_class_creation_33__bases);
        tmp_expression_name_161 = tmp_class_creation_33__bases;
        tmp_subscript_name_33 = mod_consts[46];
        tmp_type_arg_65 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_161, tmp_subscript_name_33, 0);
        if (tmp_type_arg_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        tmp_metaclass_name_33 = BUILTIN_TYPE1(tmp_type_arg_65);
        Py_DECREF(tmp_type_arg_65);
        if (tmp_metaclass_name_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        condexpr_end_33:;
        CHECK_OBJECT(tmp_class_creation_33__bases);
        tmp_bases_name_33 = tmp_class_creation_33__bases;
        tmp_assign_source_233 = SELECT_METACLASS(tmp_metaclass_name_33, tmp_bases_name_33);
        Py_DECREF(tmp_metaclass_name_33);
        if (tmp_assign_source_233 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        assert(tmp_class_creation_33__metaclass == NULL);
        tmp_class_creation_33__metaclass = tmp_assign_source_233;
    }
    {
        nuitka_bool tmp_condition_result_130;
        PyObject *tmp_key_name_99;
        PyObject *tmp_dict_arg_name_99;
        tmp_key_name_99 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_33__class_decl_dict);
        tmp_dict_arg_name_99 = tmp_class_creation_33__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_99, tmp_key_name_99);
        assert(!(tmp_res == -1));
        tmp_condition_result_130 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_130 == NUITKA_BOOL_TRUE) {
            goto branch_yes_97;
        } else {
            goto branch_no_97;
        }
    }
    branch_yes_97:;
    CHECK_OBJECT(tmp_class_creation_33__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_33__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 280;

        goto try_except_handler_97;
    }
    branch_no_97:;
    {
        nuitka_bool tmp_condition_result_131;
        PyObject *tmp_expression_name_162;
        CHECK_OBJECT(tmp_class_creation_33__metaclass);
        tmp_expression_name_162 = tmp_class_creation_33__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_162, mod_consts[50]);
        tmp_condition_result_131 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_131 == NUITKA_BOOL_TRUE) {
            goto branch_yes_98;
        } else {
            goto branch_no_98;
        }
    }
    branch_yes_98:;
    {
        PyObject *tmp_assign_source_234;
        PyObject *tmp_called_name_65;
        PyObject *tmp_expression_name_163;
        PyObject *tmp_args_name_65;
        PyObject *tmp_tuple_element_129;
        PyObject *tmp_kwargs_name_65;
        CHECK_OBJECT(tmp_class_creation_33__metaclass);
        tmp_expression_name_163 = tmp_class_creation_33__metaclass;
        tmp_called_name_65 = LOOKUP_ATTRIBUTE(tmp_expression_name_163, mod_consts[50]);
        if (tmp_called_name_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        tmp_tuple_element_129 = mod_consts[138];
        tmp_args_name_65 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_65, 0, tmp_tuple_element_129);
        CHECK_OBJECT(tmp_class_creation_33__bases);
        tmp_tuple_element_129 = tmp_class_creation_33__bases;
        PyTuple_SET_ITEM0(tmp_args_name_65, 1, tmp_tuple_element_129);
        CHECK_OBJECT(tmp_class_creation_33__class_decl_dict);
        tmp_kwargs_name_65 = tmp_class_creation_33__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 280;
        tmp_assign_source_234 = CALL_FUNCTION(tmp_called_name_65, tmp_args_name_65, tmp_kwargs_name_65);
        Py_DECREF(tmp_called_name_65);
        Py_DECREF(tmp_args_name_65);
        if (tmp_assign_source_234 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        assert(tmp_class_creation_33__prepared == NULL);
        tmp_class_creation_33__prepared = tmp_assign_source_234;
    }
    {
        nuitka_bool tmp_condition_result_132;
        PyObject *tmp_operand_name_33;
        PyObject *tmp_expression_name_164;
        CHECK_OBJECT(tmp_class_creation_33__prepared);
        tmp_expression_name_164 = tmp_class_creation_33__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_164, mod_consts[51]);
        tmp_operand_name_33 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_33);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        tmp_condition_result_132 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_132 == NUITKA_BOOL_TRUE) {
            goto branch_yes_99;
        } else {
            goto branch_no_99;
        }
    }
    branch_yes_99:;
    {
        PyObject *tmp_raise_type_33;
        PyObject *tmp_raise_value_33;
        PyObject *tmp_left_name_33;
        PyObject *tmp_right_name_33;
        PyObject *tmp_tuple_element_130;
        PyObject *tmp_getattr_target_33;
        PyObject *tmp_getattr_attr_33;
        PyObject *tmp_getattr_default_33;
        tmp_raise_type_33 = PyExc_TypeError;
        tmp_left_name_33 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_33__metaclass);
        tmp_getattr_target_33 = tmp_class_creation_33__metaclass;
        tmp_getattr_attr_33 = mod_consts[53];
        tmp_getattr_default_33 = mod_consts[54];
        tmp_tuple_element_130 = BUILTIN_GETATTR(tmp_getattr_target_33, tmp_getattr_attr_33, tmp_getattr_default_33);
        if (tmp_tuple_element_130 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        tmp_right_name_33 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_165;
            PyObject *tmp_type_arg_66;
            PyTuple_SET_ITEM(tmp_right_name_33, 0, tmp_tuple_element_130);
            CHECK_OBJECT(tmp_class_creation_33__prepared);
            tmp_type_arg_66 = tmp_class_creation_33__prepared;
            tmp_expression_name_165 = BUILTIN_TYPE1(tmp_type_arg_66);
            assert(!(tmp_expression_name_165 == NULL));
            tmp_tuple_element_130 = LOOKUP_ATTRIBUTE(tmp_expression_name_165, mod_consts[53]);
            Py_DECREF(tmp_expression_name_165);
            if (tmp_tuple_element_130 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 280;

                goto tuple_build_exception_38;
            }
            PyTuple_SET_ITEM(tmp_right_name_33, 1, tmp_tuple_element_130);
        }
        goto tuple_build_noexception_38;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_38:;
        Py_DECREF(tmp_right_name_33);
        goto try_except_handler_97;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_38:;
        tmp_raise_value_33 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_33, tmp_right_name_33);
        Py_DECREF(tmp_right_name_33);
        if (tmp_raise_value_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_97;
        }
        exception_type = tmp_raise_type_33;
        Py_INCREF(tmp_raise_type_33);
        exception_value = tmp_raise_value_33;
        exception_lineno = 280;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_97;
    }
    branch_no_99:;
    goto branch_end_98;
    branch_no_98:;
    {
        PyObject *tmp_assign_source_235;
        tmp_assign_source_235 = PyDict_New();
        assert(tmp_class_creation_33__prepared == NULL);
        tmp_class_creation_33__prepared = tmp_assign_source_235;
    }
    branch_end_98:;
    {
        PyObject *tmp_assign_source_236;
        {
            PyObject *tmp_set_locals_33;
            CHECK_OBJECT(tmp_class_creation_33__prepared);
            tmp_set_locals_33 = tmp_class_creation_33__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280 = tmp_set_locals_33;
            Py_INCREF(tmp_set_locals_33);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_99;
        }
        tmp_dictset_value = mod_consts[139];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_99;
        }
        tmp_dictset_value = mod_consts[138];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 280;

            goto try_except_handler_99;
        }
        {
            PyObject *tmp_assign_source_237;
            PyObject *tmp_called_name_66;
            PyObject *tmp_args_name_66;
            PyObject *tmp_tuple_element_131;
            PyObject *tmp_kwargs_name_66;
            CHECK_OBJECT(tmp_class_creation_33__metaclass);
            tmp_called_name_66 = tmp_class_creation_33__metaclass;
            tmp_tuple_element_131 = mod_consts[138];
            tmp_args_name_66 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_66, 0, tmp_tuple_element_131);
            CHECK_OBJECT(tmp_class_creation_33__bases);
            tmp_tuple_element_131 = tmp_class_creation_33__bases;
            PyTuple_SET_ITEM0(tmp_args_name_66, 1, tmp_tuple_element_131);
            tmp_tuple_element_131 = locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280;
            PyTuple_SET_ITEM0(tmp_args_name_66, 2, tmp_tuple_element_131);
            CHECK_OBJECT(tmp_class_creation_33__class_decl_dict);
            tmp_kwargs_name_66 = tmp_class_creation_33__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 280;
            tmp_assign_source_237 = CALL_FUNCTION(tmp_called_name_66, tmp_args_name_66, tmp_kwargs_name_66);
            Py_DECREF(tmp_args_name_66);
            if (tmp_assign_source_237 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 280;

                goto try_except_handler_99;
            }
            assert(outline_32_var___class__ == NULL);
            outline_32_var___class__ = tmp_assign_source_237;
        }
        CHECK_OBJECT(outline_32_var___class__);
        tmp_assign_source_236 = outline_32_var___class__;
        Py_INCREF(tmp_assign_source_236);
        goto try_return_handler_99;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_99:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280);
        locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280 = NULL;
        goto try_return_handler_98;
        // Exception handler code:
        try_except_handler_99:;
        exception_keeper_type_97 = exception_type;
        exception_keeper_value_97 = exception_value;
        exception_keeper_tb_97 = exception_tb;
        exception_keeper_lineno_97 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280);
        locals_pip$_vendor$urllib3$exceptions$$$class__33_InvalidHeader_280 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_97;
        exception_value = exception_keeper_value_97;
        exception_tb = exception_keeper_tb_97;
        exception_lineno = exception_keeper_lineno_97;

        goto try_except_handler_98;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_98:;
        CHECK_OBJECT(outline_32_var___class__);
        Py_DECREF(outline_32_var___class__);
        outline_32_var___class__ = NULL;
        goto outline_result_33;
        // Exception handler code:
        try_except_handler_98:;
        exception_keeper_type_98 = exception_type;
        exception_keeper_value_98 = exception_value;
        exception_keeper_tb_98 = exception_tb;
        exception_keeper_lineno_98 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_98;
        exception_value = exception_keeper_value_98;
        exception_tb = exception_keeper_tb_98;
        exception_lineno = exception_keeper_lineno_98;

        goto outline_exception_33;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_33:;
        exception_lineno = 280;
        goto try_except_handler_97;
        outline_result_33:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[138], tmp_assign_source_236);
    }
    goto try_end_33;
    // Exception handler code:
    try_except_handler_97:;
    exception_keeper_type_99 = exception_type;
    exception_keeper_value_99 = exception_value;
    exception_keeper_tb_99 = exception_tb;
    exception_keeper_lineno_99 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_33__bases);
    tmp_class_creation_33__bases = NULL;
    Py_XDECREF(tmp_class_creation_33__class_decl_dict);
    tmp_class_creation_33__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_33__metaclass);
    tmp_class_creation_33__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_33__prepared);
    tmp_class_creation_33__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_99;
    exception_value = exception_keeper_value_99;
    exception_tb = exception_keeper_tb_99;
    exception_lineno = exception_keeper_lineno_99;

    goto frame_exception_exit_1;
    // End of try:
    try_end_33:;
    Py_XDECREF(tmp_class_creation_33__bases);
    tmp_class_creation_33__bases = NULL;
    Py_XDECREF(tmp_class_creation_33__class_decl_dict);
    tmp_class_creation_33__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_33__metaclass);
    tmp_class_creation_33__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_33__prepared);
    Py_DECREF(tmp_class_creation_33__prepared);
    tmp_class_creation_33__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_238;
        PyObject *tmp_tuple_element_132;
        tmp_tuple_element_132 = PyExc_AssertionError;
        tmp_assign_source_238 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_assign_source_238, 0, tmp_tuple_element_132);
        tmp_tuple_element_132 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[18]);

        if (unlikely(tmp_tuple_element_132 == NULL)) {
            tmp_tuple_element_132 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[18]);
        }

        if (tmp_tuple_element_132 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto tuple_build_exception_39;
        }
        PyTuple_SET_ITEM0(tmp_assign_source_238, 1, tmp_tuple_element_132);
        goto tuple_build_noexception_39;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_39:;
        Py_DECREF(tmp_assign_source_238);
        goto try_except_handler_100;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_39:;
        assert(tmp_class_creation_34__bases == NULL);
        tmp_class_creation_34__bases = tmp_assign_source_238;
    }
    {
        PyObject *tmp_assign_source_239;
        tmp_assign_source_239 = PyDict_New();
        assert(tmp_class_creation_34__class_decl_dict == NULL);
        tmp_class_creation_34__class_decl_dict = tmp_assign_source_239;
    }
    {
        PyObject *tmp_assign_source_240;
        PyObject *tmp_metaclass_name_34;
        nuitka_bool tmp_condition_result_133;
        PyObject *tmp_key_name_100;
        PyObject *tmp_dict_arg_name_100;
        PyObject *tmp_dict_arg_name_101;
        PyObject *tmp_key_name_101;
        PyObject *tmp_type_arg_67;
        PyObject *tmp_expression_name_166;
        PyObject *tmp_subscript_name_34;
        PyObject *tmp_bases_name_34;
        tmp_key_name_100 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_34__class_decl_dict);
        tmp_dict_arg_name_100 = tmp_class_creation_34__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_100, tmp_key_name_100);
        assert(!(tmp_res == -1));
        tmp_condition_result_133 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_133 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_34;
        } else {
            goto condexpr_false_34;
        }
        condexpr_true_34:;
        CHECK_OBJECT(tmp_class_creation_34__class_decl_dict);
        tmp_dict_arg_name_101 = tmp_class_creation_34__class_decl_dict;
        tmp_key_name_101 = mod_consts[49];
        tmp_metaclass_name_34 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_101, tmp_key_name_101);
        if (tmp_metaclass_name_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        goto condexpr_end_34;
        condexpr_false_34:;
        CHECK_OBJECT(tmp_class_creation_34__bases);
        tmp_expression_name_166 = tmp_class_creation_34__bases;
        tmp_subscript_name_34 = mod_consts[46];
        tmp_type_arg_67 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_166, tmp_subscript_name_34, 0);
        if (tmp_type_arg_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        tmp_metaclass_name_34 = BUILTIN_TYPE1(tmp_type_arg_67);
        Py_DECREF(tmp_type_arg_67);
        if (tmp_metaclass_name_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        condexpr_end_34:;
        CHECK_OBJECT(tmp_class_creation_34__bases);
        tmp_bases_name_34 = tmp_class_creation_34__bases;
        tmp_assign_source_240 = SELECT_METACLASS(tmp_metaclass_name_34, tmp_bases_name_34);
        Py_DECREF(tmp_metaclass_name_34);
        if (tmp_assign_source_240 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        assert(tmp_class_creation_34__metaclass == NULL);
        tmp_class_creation_34__metaclass = tmp_assign_source_240;
    }
    {
        nuitka_bool tmp_condition_result_134;
        PyObject *tmp_key_name_102;
        PyObject *tmp_dict_arg_name_102;
        tmp_key_name_102 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_34__class_decl_dict);
        tmp_dict_arg_name_102 = tmp_class_creation_34__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_102, tmp_key_name_102);
        assert(!(tmp_res == -1));
        tmp_condition_result_134 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_134 == NUITKA_BOOL_TRUE) {
            goto branch_yes_100;
        } else {
            goto branch_no_100;
        }
    }
    branch_yes_100:;
    CHECK_OBJECT(tmp_class_creation_34__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_34__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 286;

        goto try_except_handler_100;
    }
    branch_no_100:;
    {
        nuitka_bool tmp_condition_result_135;
        PyObject *tmp_expression_name_167;
        CHECK_OBJECT(tmp_class_creation_34__metaclass);
        tmp_expression_name_167 = tmp_class_creation_34__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_167, mod_consts[50]);
        tmp_condition_result_135 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_135 == NUITKA_BOOL_TRUE) {
            goto branch_yes_101;
        } else {
            goto branch_no_101;
        }
    }
    branch_yes_101:;
    {
        PyObject *tmp_assign_source_241;
        PyObject *tmp_called_name_67;
        PyObject *tmp_expression_name_168;
        PyObject *tmp_args_name_67;
        PyObject *tmp_tuple_element_133;
        PyObject *tmp_kwargs_name_67;
        CHECK_OBJECT(tmp_class_creation_34__metaclass);
        tmp_expression_name_168 = tmp_class_creation_34__metaclass;
        tmp_called_name_67 = LOOKUP_ATTRIBUTE(tmp_expression_name_168, mod_consts[50]);
        if (tmp_called_name_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        tmp_tuple_element_133 = mod_consts[31];
        tmp_args_name_67 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_67, 0, tmp_tuple_element_133);
        CHECK_OBJECT(tmp_class_creation_34__bases);
        tmp_tuple_element_133 = tmp_class_creation_34__bases;
        PyTuple_SET_ITEM0(tmp_args_name_67, 1, tmp_tuple_element_133);
        CHECK_OBJECT(tmp_class_creation_34__class_decl_dict);
        tmp_kwargs_name_67 = tmp_class_creation_34__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 286;
        tmp_assign_source_241 = CALL_FUNCTION(tmp_called_name_67, tmp_args_name_67, tmp_kwargs_name_67);
        Py_DECREF(tmp_called_name_67);
        Py_DECREF(tmp_args_name_67);
        if (tmp_assign_source_241 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        assert(tmp_class_creation_34__prepared == NULL);
        tmp_class_creation_34__prepared = tmp_assign_source_241;
    }
    {
        nuitka_bool tmp_condition_result_136;
        PyObject *tmp_operand_name_34;
        PyObject *tmp_expression_name_169;
        CHECK_OBJECT(tmp_class_creation_34__prepared);
        tmp_expression_name_169 = tmp_class_creation_34__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_169, mod_consts[51]);
        tmp_operand_name_34 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_34);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        tmp_condition_result_136 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_136 == NUITKA_BOOL_TRUE) {
            goto branch_yes_102;
        } else {
            goto branch_no_102;
        }
    }
    branch_yes_102:;
    {
        PyObject *tmp_raise_type_34;
        PyObject *tmp_raise_value_34;
        PyObject *tmp_left_name_34;
        PyObject *tmp_right_name_34;
        PyObject *tmp_tuple_element_134;
        PyObject *tmp_getattr_target_34;
        PyObject *tmp_getattr_attr_34;
        PyObject *tmp_getattr_default_34;
        tmp_raise_type_34 = PyExc_TypeError;
        tmp_left_name_34 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_34__metaclass);
        tmp_getattr_target_34 = tmp_class_creation_34__metaclass;
        tmp_getattr_attr_34 = mod_consts[53];
        tmp_getattr_default_34 = mod_consts[54];
        tmp_tuple_element_134 = BUILTIN_GETATTR(tmp_getattr_target_34, tmp_getattr_attr_34, tmp_getattr_default_34);
        if (tmp_tuple_element_134 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        tmp_right_name_34 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_170;
            PyObject *tmp_type_arg_68;
            PyTuple_SET_ITEM(tmp_right_name_34, 0, tmp_tuple_element_134);
            CHECK_OBJECT(tmp_class_creation_34__prepared);
            tmp_type_arg_68 = tmp_class_creation_34__prepared;
            tmp_expression_name_170 = BUILTIN_TYPE1(tmp_type_arg_68);
            assert(!(tmp_expression_name_170 == NULL));
            tmp_tuple_element_134 = LOOKUP_ATTRIBUTE(tmp_expression_name_170, mod_consts[53]);
            Py_DECREF(tmp_expression_name_170);
            if (tmp_tuple_element_134 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 286;

                goto tuple_build_exception_40;
            }
            PyTuple_SET_ITEM(tmp_right_name_34, 1, tmp_tuple_element_134);
        }
        goto tuple_build_noexception_40;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_40:;
        Py_DECREF(tmp_right_name_34);
        goto try_except_handler_100;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_40:;
        tmp_raise_value_34 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_34, tmp_right_name_34);
        Py_DECREF(tmp_right_name_34);
        if (tmp_raise_value_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_100;
        }
        exception_type = tmp_raise_type_34;
        Py_INCREF(tmp_raise_type_34);
        exception_value = tmp_raise_value_34;
        exception_lineno = 286;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_100;
    }
    branch_no_102:;
    goto branch_end_101;
    branch_no_101:;
    {
        PyObject *tmp_assign_source_242;
        tmp_assign_source_242 = PyDict_New();
        assert(tmp_class_creation_34__prepared == NULL);
        tmp_class_creation_34__prepared = tmp_assign_source_242;
    }
    branch_end_101:;
    {
        PyObject *tmp_assign_source_243;
        {
            PyObject *tmp_set_locals_34;
            CHECK_OBJECT(tmp_class_creation_34__prepared);
            tmp_set_locals_34 = tmp_class_creation_34__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286 = tmp_set_locals_34;
            Py_INCREF(tmp_set_locals_34);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_102;
        }
        tmp_dictset_value = mod_consts[140];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_102;
        }
        tmp_dictset_value = mod_consts[31];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 286;

            goto try_except_handler_102;
        }
        if (isFrameUnusable(cache_frame_b067facd3b4500d3e185f1adaf50e20e_12)) {
            Py_XDECREF(cache_frame_b067facd3b4500d3e185f1adaf50e20e_12);

#if _DEBUG_REFCOUNTS
            if (cache_frame_b067facd3b4500d3e185f1adaf50e20e_12 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_b067facd3b4500d3e185f1adaf50e20e_12 = MAKE_FUNCTION_FRAME(codeobj_b067facd3b4500d3e185f1adaf50e20e, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_b067facd3b4500d3e185f1adaf50e20e_12->m_type_description == NULL);
        frame_b067facd3b4500d3e185f1adaf50e20e_12 = cache_frame_b067facd3b4500d3e185f1adaf50e20e_12;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_b067facd3b4500d3e185f1adaf50e20e_12);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_b067facd3b4500d3e185f1adaf50e20e_12) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__14___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 291;
            type_description_2 = "o";
            goto frame_exception_exit_12;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_b067facd3b4500d3e185f1adaf50e20e_12);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_11;

        frame_exception_exit_12:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_b067facd3b4500d3e185f1adaf50e20e_12);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_b067facd3b4500d3e185f1adaf50e20e_12, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_b067facd3b4500d3e185f1adaf50e20e_12->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_b067facd3b4500d3e185f1adaf50e20e_12, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_b067facd3b4500d3e185f1adaf50e20e_12,
            type_description_2,
            outline_33_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_b067facd3b4500d3e185f1adaf50e20e_12 == cache_frame_b067facd3b4500d3e185f1adaf50e20e_12) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_b067facd3b4500d3e185f1adaf50e20e_12);
            cache_frame_b067facd3b4500d3e185f1adaf50e20e_12 = NULL;
        }

        assertFrameObject(frame_b067facd3b4500d3e185f1adaf50e20e_12);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_11;

        frame_no_exception_11:;
        goto skip_nested_handling_11;
        nested_frame_exit_11:;

        goto try_except_handler_102;
        skip_nested_handling_11:;
        {
            PyObject *tmp_assign_source_244;
            PyObject *tmp_called_name_68;
            PyObject *tmp_args_name_68;
            PyObject *tmp_tuple_element_135;
            PyObject *tmp_kwargs_name_68;
            CHECK_OBJECT(tmp_class_creation_34__metaclass);
            tmp_called_name_68 = tmp_class_creation_34__metaclass;
            tmp_tuple_element_135 = mod_consts[31];
            tmp_args_name_68 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_68, 0, tmp_tuple_element_135);
            CHECK_OBJECT(tmp_class_creation_34__bases);
            tmp_tuple_element_135 = tmp_class_creation_34__bases;
            PyTuple_SET_ITEM0(tmp_args_name_68, 1, tmp_tuple_element_135);
            tmp_tuple_element_135 = locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286;
            PyTuple_SET_ITEM0(tmp_args_name_68, 2, tmp_tuple_element_135);
            CHECK_OBJECT(tmp_class_creation_34__class_decl_dict);
            tmp_kwargs_name_68 = tmp_class_creation_34__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 286;
            tmp_assign_source_244 = CALL_FUNCTION(tmp_called_name_68, tmp_args_name_68, tmp_kwargs_name_68);
            Py_DECREF(tmp_args_name_68);
            if (tmp_assign_source_244 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 286;

                goto try_except_handler_102;
            }
            assert(outline_33_var___class__ == NULL);
            outline_33_var___class__ = tmp_assign_source_244;
        }
        CHECK_OBJECT(outline_33_var___class__);
        tmp_assign_source_243 = outline_33_var___class__;
        Py_INCREF(tmp_assign_source_243);
        goto try_return_handler_102;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_102:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286);
        locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286 = NULL;
        goto try_return_handler_101;
        // Exception handler code:
        try_except_handler_102:;
        exception_keeper_type_100 = exception_type;
        exception_keeper_value_100 = exception_value;
        exception_keeper_tb_100 = exception_tb;
        exception_keeper_lineno_100 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286);
        locals_pip$_vendor$urllib3$exceptions$$$class__34_ProxySchemeUnknown_286 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_100;
        exception_value = exception_keeper_value_100;
        exception_tb = exception_keeper_tb_100;
        exception_lineno = exception_keeper_lineno_100;

        goto try_except_handler_101;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_101:;
        CHECK_OBJECT(outline_33_var___class__);
        Py_DECREF(outline_33_var___class__);
        outline_33_var___class__ = NULL;
        goto outline_result_34;
        // Exception handler code:
        try_except_handler_101:;
        exception_keeper_type_101 = exception_type;
        exception_keeper_value_101 = exception_value;
        exception_keeper_tb_101 = exception_tb;
        exception_keeper_lineno_101 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_101;
        exception_value = exception_keeper_value_101;
        exception_tb = exception_keeper_tb_101;
        exception_lineno = exception_keeper_lineno_101;

        goto outline_exception_34;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_34:;
        exception_lineno = 286;
        goto try_except_handler_100;
        outline_result_34:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[31], tmp_assign_source_243);
    }
    goto try_end_34;
    // Exception handler code:
    try_except_handler_100:;
    exception_keeper_type_102 = exception_type;
    exception_keeper_value_102 = exception_value;
    exception_keeper_tb_102 = exception_tb;
    exception_keeper_lineno_102 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_34__bases);
    tmp_class_creation_34__bases = NULL;
    Py_XDECREF(tmp_class_creation_34__class_decl_dict);
    tmp_class_creation_34__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_34__metaclass);
    tmp_class_creation_34__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_34__prepared);
    tmp_class_creation_34__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_102;
    exception_value = exception_keeper_value_102;
    exception_tb = exception_keeper_tb_102;
    exception_lineno = exception_keeper_lineno_102;

    goto frame_exception_exit_1;
    // End of try:
    try_end_34:;
    Py_XDECREF(tmp_class_creation_34__bases);
    tmp_class_creation_34__bases = NULL;
    Py_XDECREF(tmp_class_creation_34__class_decl_dict);
    tmp_class_creation_34__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_34__metaclass);
    tmp_class_creation_34__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_34__prepared);
    Py_DECREF(tmp_class_creation_34__prepared);
    tmp_class_creation_34__prepared = NULL;
    {
        PyObject *tmp_assign_source_245;
        tmp_assign_source_245 = mod_consts[142];
        assert(tmp_class_creation_35__bases == NULL);
        Py_INCREF(tmp_assign_source_245);
        tmp_class_creation_35__bases = tmp_assign_source_245;
    }
    {
        PyObject *tmp_assign_source_246;
        tmp_assign_source_246 = PyDict_New();
        assert(tmp_class_creation_35__class_decl_dict == NULL);
        tmp_class_creation_35__class_decl_dict = tmp_assign_source_246;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_247;
        PyObject *tmp_metaclass_name_35;
        nuitka_bool tmp_condition_result_137;
        PyObject *tmp_key_name_103;
        PyObject *tmp_dict_arg_name_103;
        PyObject *tmp_dict_arg_name_104;
        PyObject *tmp_key_name_104;
        PyObject *tmp_type_arg_69;
        PyObject *tmp_expression_name_171;
        PyObject *tmp_subscript_name_35;
        PyObject *tmp_bases_name_35;
        tmp_key_name_103 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_35__class_decl_dict);
        tmp_dict_arg_name_103 = tmp_class_creation_35__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_103, tmp_key_name_103);
        assert(!(tmp_res == -1));
        tmp_condition_result_137 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_137 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_35;
        } else {
            goto condexpr_false_35;
        }
        condexpr_true_35:;
        CHECK_OBJECT(tmp_class_creation_35__class_decl_dict);
        tmp_dict_arg_name_104 = tmp_class_creation_35__class_decl_dict;
        tmp_key_name_104 = mod_consts[49];
        tmp_metaclass_name_35 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_104, tmp_key_name_104);
        if (tmp_metaclass_name_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        goto condexpr_end_35;
        condexpr_false_35:;
        CHECK_OBJECT(tmp_class_creation_35__bases);
        tmp_expression_name_171 = tmp_class_creation_35__bases;
        tmp_subscript_name_35 = mod_consts[46];
        tmp_type_arg_69 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_171, tmp_subscript_name_35, 0);
        if (tmp_type_arg_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        tmp_metaclass_name_35 = BUILTIN_TYPE1(tmp_type_arg_69);
        Py_DECREF(tmp_type_arg_69);
        if (tmp_metaclass_name_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        condexpr_end_35:;
        CHECK_OBJECT(tmp_class_creation_35__bases);
        tmp_bases_name_35 = tmp_class_creation_35__bases;
        tmp_assign_source_247 = SELECT_METACLASS(tmp_metaclass_name_35, tmp_bases_name_35);
        Py_DECREF(tmp_metaclass_name_35);
        if (tmp_assign_source_247 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        assert(tmp_class_creation_35__metaclass == NULL);
        tmp_class_creation_35__metaclass = tmp_assign_source_247;
    }
    {
        nuitka_bool tmp_condition_result_138;
        PyObject *tmp_key_name_105;
        PyObject *tmp_dict_arg_name_105;
        tmp_key_name_105 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_35__class_decl_dict);
        tmp_dict_arg_name_105 = tmp_class_creation_35__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_105, tmp_key_name_105);
        assert(!(tmp_res == -1));
        tmp_condition_result_138 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_138 == NUITKA_BOOL_TRUE) {
            goto branch_yes_103;
        } else {
            goto branch_no_103;
        }
    }
    branch_yes_103:;
    CHECK_OBJECT(tmp_class_creation_35__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_35__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 296;

        goto try_except_handler_103;
    }
    branch_no_103:;
    {
        nuitka_bool tmp_condition_result_139;
        PyObject *tmp_expression_name_172;
        CHECK_OBJECT(tmp_class_creation_35__metaclass);
        tmp_expression_name_172 = tmp_class_creation_35__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_172, mod_consts[50]);
        tmp_condition_result_139 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_139 == NUITKA_BOOL_TRUE) {
            goto branch_yes_104;
        } else {
            goto branch_no_104;
        }
    }
    branch_yes_104:;
    {
        PyObject *tmp_assign_source_248;
        PyObject *tmp_called_name_69;
        PyObject *tmp_expression_name_173;
        PyObject *tmp_args_name_69;
        PyObject *tmp_tuple_element_136;
        PyObject *tmp_kwargs_name_69;
        CHECK_OBJECT(tmp_class_creation_35__metaclass);
        tmp_expression_name_173 = tmp_class_creation_35__metaclass;
        tmp_called_name_69 = LOOKUP_ATTRIBUTE(tmp_expression_name_173, mod_consts[50]);
        if (tmp_called_name_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        tmp_tuple_element_136 = mod_consts[143];
        tmp_args_name_69 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_69, 0, tmp_tuple_element_136);
        CHECK_OBJECT(tmp_class_creation_35__bases);
        tmp_tuple_element_136 = tmp_class_creation_35__bases;
        PyTuple_SET_ITEM0(tmp_args_name_69, 1, tmp_tuple_element_136);
        CHECK_OBJECT(tmp_class_creation_35__class_decl_dict);
        tmp_kwargs_name_69 = tmp_class_creation_35__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 296;
        tmp_assign_source_248 = CALL_FUNCTION(tmp_called_name_69, tmp_args_name_69, tmp_kwargs_name_69);
        Py_DECREF(tmp_called_name_69);
        Py_DECREF(tmp_args_name_69);
        if (tmp_assign_source_248 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        assert(tmp_class_creation_35__prepared == NULL);
        tmp_class_creation_35__prepared = tmp_assign_source_248;
    }
    {
        nuitka_bool tmp_condition_result_140;
        PyObject *tmp_operand_name_35;
        PyObject *tmp_expression_name_174;
        CHECK_OBJECT(tmp_class_creation_35__prepared);
        tmp_expression_name_174 = tmp_class_creation_35__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_174, mod_consts[51]);
        tmp_operand_name_35 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_35);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        tmp_condition_result_140 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_140 == NUITKA_BOOL_TRUE) {
            goto branch_yes_105;
        } else {
            goto branch_no_105;
        }
    }
    branch_yes_105:;
    {
        PyObject *tmp_raise_type_35;
        PyObject *tmp_raise_value_35;
        PyObject *tmp_left_name_35;
        PyObject *tmp_right_name_35;
        PyObject *tmp_tuple_element_137;
        PyObject *tmp_getattr_target_35;
        PyObject *tmp_getattr_attr_35;
        PyObject *tmp_getattr_default_35;
        tmp_raise_type_35 = PyExc_TypeError;
        tmp_left_name_35 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_35__metaclass);
        tmp_getattr_target_35 = tmp_class_creation_35__metaclass;
        tmp_getattr_attr_35 = mod_consts[53];
        tmp_getattr_default_35 = mod_consts[54];
        tmp_tuple_element_137 = BUILTIN_GETATTR(tmp_getattr_target_35, tmp_getattr_attr_35, tmp_getattr_default_35);
        if (tmp_tuple_element_137 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        tmp_right_name_35 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_175;
            PyObject *tmp_type_arg_70;
            PyTuple_SET_ITEM(tmp_right_name_35, 0, tmp_tuple_element_137);
            CHECK_OBJECT(tmp_class_creation_35__prepared);
            tmp_type_arg_70 = tmp_class_creation_35__prepared;
            tmp_expression_name_175 = BUILTIN_TYPE1(tmp_type_arg_70);
            assert(!(tmp_expression_name_175 == NULL));
            tmp_tuple_element_137 = LOOKUP_ATTRIBUTE(tmp_expression_name_175, mod_consts[53]);
            Py_DECREF(tmp_expression_name_175);
            if (tmp_tuple_element_137 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 296;

                goto tuple_build_exception_41;
            }
            PyTuple_SET_ITEM(tmp_right_name_35, 1, tmp_tuple_element_137);
        }
        goto tuple_build_noexception_41;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_41:;
        Py_DECREF(tmp_right_name_35);
        goto try_except_handler_103;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_41:;
        tmp_raise_value_35 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_35, tmp_right_name_35);
        Py_DECREF(tmp_right_name_35);
        if (tmp_raise_value_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_103;
        }
        exception_type = tmp_raise_type_35;
        Py_INCREF(tmp_raise_type_35);
        exception_value = tmp_raise_value_35;
        exception_lineno = 296;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_103;
    }
    branch_no_105:;
    goto branch_end_104;
    branch_no_104:;
    {
        PyObject *tmp_assign_source_249;
        tmp_assign_source_249 = PyDict_New();
        assert(tmp_class_creation_35__prepared == NULL);
        tmp_class_creation_35__prepared = tmp_assign_source_249;
    }
    branch_end_104:;
    {
        PyObject *tmp_assign_source_250;
        {
            PyObject *tmp_set_locals_35;
            CHECK_OBJECT(tmp_class_creation_35__prepared);
            tmp_set_locals_35 = tmp_class_creation_35__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296 = tmp_set_locals_35;
            Py_INCREF(tmp_set_locals_35);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_105;
        }
        tmp_dictset_value = mod_consts[144];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_105;
        }
        tmp_dictset_value = mod_consts[143];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 296;

            goto try_except_handler_105;
        }
        {
            PyObject *tmp_assign_source_251;
            PyObject *tmp_called_name_70;
            PyObject *tmp_args_name_70;
            PyObject *tmp_tuple_element_138;
            PyObject *tmp_kwargs_name_70;
            CHECK_OBJECT(tmp_class_creation_35__metaclass);
            tmp_called_name_70 = tmp_class_creation_35__metaclass;
            tmp_tuple_element_138 = mod_consts[143];
            tmp_args_name_70 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_70, 0, tmp_tuple_element_138);
            CHECK_OBJECT(tmp_class_creation_35__bases);
            tmp_tuple_element_138 = tmp_class_creation_35__bases;
            PyTuple_SET_ITEM0(tmp_args_name_70, 1, tmp_tuple_element_138);
            tmp_tuple_element_138 = locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296;
            PyTuple_SET_ITEM0(tmp_args_name_70, 2, tmp_tuple_element_138);
            CHECK_OBJECT(tmp_class_creation_35__class_decl_dict);
            tmp_kwargs_name_70 = tmp_class_creation_35__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 296;
            tmp_assign_source_251 = CALL_FUNCTION(tmp_called_name_70, tmp_args_name_70, tmp_kwargs_name_70);
            Py_DECREF(tmp_args_name_70);
            if (tmp_assign_source_251 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 296;

                goto try_except_handler_105;
            }
            assert(outline_34_var___class__ == NULL);
            outline_34_var___class__ = tmp_assign_source_251;
        }
        CHECK_OBJECT(outline_34_var___class__);
        tmp_assign_source_250 = outline_34_var___class__;
        Py_INCREF(tmp_assign_source_250);
        goto try_return_handler_105;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_105:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296);
        locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296 = NULL;
        goto try_return_handler_104;
        // Exception handler code:
        try_except_handler_105:;
        exception_keeper_type_103 = exception_type;
        exception_keeper_value_103 = exception_value;
        exception_keeper_tb_103 = exception_tb;
        exception_keeper_lineno_103 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296);
        locals_pip$_vendor$urllib3$exceptions$$$class__35_ProxySchemeUnsupported_296 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_103;
        exception_value = exception_keeper_value_103;
        exception_tb = exception_keeper_tb_103;
        exception_lineno = exception_keeper_lineno_103;

        goto try_except_handler_104;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_104:;
        CHECK_OBJECT(outline_34_var___class__);
        Py_DECREF(outline_34_var___class__);
        outline_34_var___class__ = NULL;
        goto outline_result_35;
        // Exception handler code:
        try_except_handler_104:;
        exception_keeper_type_104 = exception_type;
        exception_keeper_value_104 = exception_value;
        exception_keeper_tb_104 = exception_tb;
        exception_keeper_lineno_104 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_104;
        exception_value = exception_keeper_value_104;
        exception_tb = exception_keeper_tb_104;
        exception_lineno = exception_keeper_lineno_104;

        goto outline_exception_35;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_35:;
        exception_lineno = 296;
        goto try_except_handler_103;
        outline_result_35:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[143], tmp_assign_source_250);
    }
    goto try_end_35;
    // Exception handler code:
    try_except_handler_103:;
    exception_keeper_type_105 = exception_type;
    exception_keeper_value_105 = exception_value;
    exception_keeper_tb_105 = exception_tb;
    exception_keeper_lineno_105 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_35__bases);
    tmp_class_creation_35__bases = NULL;
    Py_XDECREF(tmp_class_creation_35__class_decl_dict);
    tmp_class_creation_35__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_35__metaclass);
    tmp_class_creation_35__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_35__prepared);
    tmp_class_creation_35__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_105;
    exception_value = exception_keeper_value_105;
    exception_tb = exception_keeper_tb_105;
    exception_lineno = exception_keeper_lineno_105;

    goto frame_exception_exit_1;
    // End of try:
    try_end_35:;
    Py_XDECREF(tmp_class_creation_35__bases);
    tmp_class_creation_35__bases = NULL;
    Py_XDECREF(tmp_class_creation_35__class_decl_dict);
    tmp_class_creation_35__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_35__metaclass);
    tmp_class_creation_35__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_35__prepared);
    Py_DECREF(tmp_class_creation_35__prepared);
    tmp_class_creation_35__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_252;
        PyObject *tmp_tuple_element_139;
        tmp_tuple_element_139 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_139 == NULL)) {
            tmp_tuple_element_139 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_139 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        tmp_assign_source_252 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_252, 0, tmp_tuple_element_139);
        assert(tmp_class_creation_36__bases == NULL);
        tmp_class_creation_36__bases = tmp_assign_source_252;
    }
    {
        PyObject *tmp_assign_source_253;
        tmp_assign_source_253 = PyDict_New();
        assert(tmp_class_creation_36__class_decl_dict == NULL);
        tmp_class_creation_36__class_decl_dict = tmp_assign_source_253;
    }
    {
        PyObject *tmp_assign_source_254;
        PyObject *tmp_metaclass_name_36;
        nuitka_bool tmp_condition_result_141;
        PyObject *tmp_key_name_106;
        PyObject *tmp_dict_arg_name_106;
        PyObject *tmp_dict_arg_name_107;
        PyObject *tmp_key_name_107;
        PyObject *tmp_type_arg_71;
        PyObject *tmp_expression_name_176;
        PyObject *tmp_subscript_name_36;
        PyObject *tmp_bases_name_36;
        tmp_key_name_106 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_36__class_decl_dict);
        tmp_dict_arg_name_106 = tmp_class_creation_36__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_106, tmp_key_name_106);
        assert(!(tmp_res == -1));
        tmp_condition_result_141 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_141 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_36;
        } else {
            goto condexpr_false_36;
        }
        condexpr_true_36:;
        CHECK_OBJECT(tmp_class_creation_36__class_decl_dict);
        tmp_dict_arg_name_107 = tmp_class_creation_36__class_decl_dict;
        tmp_key_name_107 = mod_consts[49];
        tmp_metaclass_name_36 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_107, tmp_key_name_107);
        if (tmp_metaclass_name_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        goto condexpr_end_36;
        condexpr_false_36:;
        CHECK_OBJECT(tmp_class_creation_36__bases);
        tmp_expression_name_176 = tmp_class_creation_36__bases;
        tmp_subscript_name_36 = mod_consts[46];
        tmp_type_arg_71 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_176, tmp_subscript_name_36, 0);
        if (tmp_type_arg_71 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        tmp_metaclass_name_36 = BUILTIN_TYPE1(tmp_type_arg_71);
        Py_DECREF(tmp_type_arg_71);
        if (tmp_metaclass_name_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        condexpr_end_36:;
        CHECK_OBJECT(tmp_class_creation_36__bases);
        tmp_bases_name_36 = tmp_class_creation_36__bases;
        tmp_assign_source_254 = SELECT_METACLASS(tmp_metaclass_name_36, tmp_bases_name_36);
        Py_DECREF(tmp_metaclass_name_36);
        if (tmp_assign_source_254 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        assert(tmp_class_creation_36__metaclass == NULL);
        tmp_class_creation_36__metaclass = tmp_assign_source_254;
    }
    {
        nuitka_bool tmp_condition_result_142;
        PyObject *tmp_key_name_108;
        PyObject *tmp_dict_arg_name_108;
        tmp_key_name_108 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_36__class_decl_dict);
        tmp_dict_arg_name_108 = tmp_class_creation_36__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_108, tmp_key_name_108);
        assert(!(tmp_res == -1));
        tmp_condition_result_142 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_142 == NUITKA_BOOL_TRUE) {
            goto branch_yes_106;
        } else {
            goto branch_no_106;
        }
    }
    branch_yes_106:;
    CHECK_OBJECT(tmp_class_creation_36__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_36__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 302;

        goto try_except_handler_106;
    }
    branch_no_106:;
    {
        nuitka_bool tmp_condition_result_143;
        PyObject *tmp_expression_name_177;
        CHECK_OBJECT(tmp_class_creation_36__metaclass);
        tmp_expression_name_177 = tmp_class_creation_36__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_177, mod_consts[50]);
        tmp_condition_result_143 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_143 == NUITKA_BOOL_TRUE) {
            goto branch_yes_107;
        } else {
            goto branch_no_107;
        }
    }
    branch_yes_107:;
    {
        PyObject *tmp_assign_source_255;
        PyObject *tmp_called_name_71;
        PyObject *tmp_expression_name_178;
        PyObject *tmp_args_name_71;
        PyObject *tmp_tuple_element_140;
        PyObject *tmp_kwargs_name_71;
        CHECK_OBJECT(tmp_class_creation_36__metaclass);
        tmp_expression_name_178 = tmp_class_creation_36__metaclass;
        tmp_called_name_71 = LOOKUP_ATTRIBUTE(tmp_expression_name_178, mod_consts[50]);
        if (tmp_called_name_71 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        tmp_tuple_element_140 = mod_consts[34];
        tmp_args_name_71 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_71, 0, tmp_tuple_element_140);
        CHECK_OBJECT(tmp_class_creation_36__bases);
        tmp_tuple_element_140 = tmp_class_creation_36__bases;
        PyTuple_SET_ITEM0(tmp_args_name_71, 1, tmp_tuple_element_140);
        CHECK_OBJECT(tmp_class_creation_36__class_decl_dict);
        tmp_kwargs_name_71 = tmp_class_creation_36__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 302;
        tmp_assign_source_255 = CALL_FUNCTION(tmp_called_name_71, tmp_args_name_71, tmp_kwargs_name_71);
        Py_DECREF(tmp_called_name_71);
        Py_DECREF(tmp_args_name_71);
        if (tmp_assign_source_255 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        assert(tmp_class_creation_36__prepared == NULL);
        tmp_class_creation_36__prepared = tmp_assign_source_255;
    }
    {
        nuitka_bool tmp_condition_result_144;
        PyObject *tmp_operand_name_36;
        PyObject *tmp_expression_name_179;
        CHECK_OBJECT(tmp_class_creation_36__prepared);
        tmp_expression_name_179 = tmp_class_creation_36__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_179, mod_consts[51]);
        tmp_operand_name_36 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_36);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        tmp_condition_result_144 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_144 == NUITKA_BOOL_TRUE) {
            goto branch_yes_108;
        } else {
            goto branch_no_108;
        }
    }
    branch_yes_108:;
    {
        PyObject *tmp_raise_type_36;
        PyObject *tmp_raise_value_36;
        PyObject *tmp_left_name_36;
        PyObject *tmp_right_name_36;
        PyObject *tmp_tuple_element_141;
        PyObject *tmp_getattr_target_36;
        PyObject *tmp_getattr_attr_36;
        PyObject *tmp_getattr_default_36;
        tmp_raise_type_36 = PyExc_TypeError;
        tmp_left_name_36 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_36__metaclass);
        tmp_getattr_target_36 = tmp_class_creation_36__metaclass;
        tmp_getattr_attr_36 = mod_consts[53];
        tmp_getattr_default_36 = mod_consts[54];
        tmp_tuple_element_141 = BUILTIN_GETATTR(tmp_getattr_target_36, tmp_getattr_attr_36, tmp_getattr_default_36);
        if (tmp_tuple_element_141 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        tmp_right_name_36 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_180;
            PyObject *tmp_type_arg_72;
            PyTuple_SET_ITEM(tmp_right_name_36, 0, tmp_tuple_element_141);
            CHECK_OBJECT(tmp_class_creation_36__prepared);
            tmp_type_arg_72 = tmp_class_creation_36__prepared;
            tmp_expression_name_180 = BUILTIN_TYPE1(tmp_type_arg_72);
            assert(!(tmp_expression_name_180 == NULL));
            tmp_tuple_element_141 = LOOKUP_ATTRIBUTE(tmp_expression_name_180, mod_consts[53]);
            Py_DECREF(tmp_expression_name_180);
            if (tmp_tuple_element_141 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 302;

                goto tuple_build_exception_42;
            }
            PyTuple_SET_ITEM(tmp_right_name_36, 1, tmp_tuple_element_141);
        }
        goto tuple_build_noexception_42;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_42:;
        Py_DECREF(tmp_right_name_36);
        goto try_except_handler_106;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_42:;
        tmp_raise_value_36 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_36, tmp_right_name_36);
        Py_DECREF(tmp_right_name_36);
        if (tmp_raise_value_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_106;
        }
        exception_type = tmp_raise_type_36;
        Py_INCREF(tmp_raise_type_36);
        exception_value = tmp_raise_value_36;
        exception_lineno = 302;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_106;
    }
    branch_no_108:;
    goto branch_end_107;
    branch_no_107:;
    {
        PyObject *tmp_assign_source_256;
        tmp_assign_source_256 = PyDict_New();
        assert(tmp_class_creation_36__prepared == NULL);
        tmp_class_creation_36__prepared = tmp_assign_source_256;
    }
    branch_end_107:;
    {
        PyObject *tmp_assign_source_257;
        {
            PyObject *tmp_set_locals_36;
            CHECK_OBJECT(tmp_class_creation_36__prepared);
            tmp_set_locals_36 = tmp_class_creation_36__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302 = tmp_set_locals_36;
            Py_INCREF(tmp_set_locals_36);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_108;
        }
        tmp_dictset_value = mod_consts[145];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_108;
        }
        tmp_dictset_value = mod_consts[34];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 302;

            goto try_except_handler_108;
        }
        if (isFrameUnusable(cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13)) {
            Py_XDECREF(cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13);

#if _DEBUG_REFCOUNTS
            if (cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13 = MAKE_FUNCTION_FRAME(codeobj_741c7aa688cb6408fa2c7e94c2e034af, module_pip$_vendor$urllib3$exceptions, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13->m_type_description == NULL);
        frame_741c7aa688cb6408fa2c7e94c2e034af_13 = cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_741c7aa688cb6408fa2c7e94c2e034af_13);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_741c7aa688cb6408fa2c7e94c2e034af_13) == 2); // Frame stack

        // Framed code:


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$urllib3$exceptions$$$function__15___init__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302, mod_consts[2], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 305;
            type_description_2 = "o";
            goto frame_exception_exit_13;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_741c7aa688cb6408fa2c7e94c2e034af_13);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_12;

        frame_exception_exit_13:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_741c7aa688cb6408fa2c7e94c2e034af_13);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_741c7aa688cb6408fa2c7e94c2e034af_13, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_741c7aa688cb6408fa2c7e94c2e034af_13->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_741c7aa688cb6408fa2c7e94c2e034af_13, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_741c7aa688cb6408fa2c7e94c2e034af_13,
            type_description_2,
            outline_35_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_741c7aa688cb6408fa2c7e94c2e034af_13 == cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13);
            cache_frame_741c7aa688cb6408fa2c7e94c2e034af_13 = NULL;
        }

        assertFrameObject(frame_741c7aa688cb6408fa2c7e94c2e034af_13);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_12;

        frame_no_exception_12:;
        goto skip_nested_handling_12;
        nested_frame_exit_12:;

        goto try_except_handler_108;
        skip_nested_handling_12:;
        {
            PyObject *tmp_assign_source_258;
            PyObject *tmp_called_name_72;
            PyObject *tmp_args_name_72;
            PyObject *tmp_tuple_element_142;
            PyObject *tmp_kwargs_name_72;
            CHECK_OBJECT(tmp_class_creation_36__metaclass);
            tmp_called_name_72 = tmp_class_creation_36__metaclass;
            tmp_tuple_element_142 = mod_consts[34];
            tmp_args_name_72 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_72, 0, tmp_tuple_element_142);
            CHECK_OBJECT(tmp_class_creation_36__bases);
            tmp_tuple_element_142 = tmp_class_creation_36__bases;
            PyTuple_SET_ITEM0(tmp_args_name_72, 1, tmp_tuple_element_142);
            tmp_tuple_element_142 = locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302;
            PyTuple_SET_ITEM0(tmp_args_name_72, 2, tmp_tuple_element_142);
            CHECK_OBJECT(tmp_class_creation_36__class_decl_dict);
            tmp_kwargs_name_72 = tmp_class_creation_36__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 302;
            tmp_assign_source_258 = CALL_FUNCTION(tmp_called_name_72, tmp_args_name_72, tmp_kwargs_name_72);
            Py_DECREF(tmp_args_name_72);
            if (tmp_assign_source_258 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 302;

                goto try_except_handler_108;
            }
            assert(outline_35_var___class__ == NULL);
            outline_35_var___class__ = tmp_assign_source_258;
        }
        CHECK_OBJECT(outline_35_var___class__);
        tmp_assign_source_257 = outline_35_var___class__;
        Py_INCREF(tmp_assign_source_257);
        goto try_return_handler_108;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_108:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302);
        locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302 = NULL;
        goto try_return_handler_107;
        // Exception handler code:
        try_except_handler_108:;
        exception_keeper_type_106 = exception_type;
        exception_keeper_value_106 = exception_value;
        exception_keeper_tb_106 = exception_tb;
        exception_keeper_lineno_106 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302);
        locals_pip$_vendor$urllib3$exceptions$$$class__36_HeaderParsingError_302 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_106;
        exception_value = exception_keeper_value_106;
        exception_tb = exception_keeper_tb_106;
        exception_lineno = exception_keeper_lineno_106;

        goto try_except_handler_107;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_107:;
        CHECK_OBJECT(outline_35_var___class__);
        Py_DECREF(outline_35_var___class__);
        outline_35_var___class__ = NULL;
        goto outline_result_36;
        // Exception handler code:
        try_except_handler_107:;
        exception_keeper_type_107 = exception_type;
        exception_keeper_value_107 = exception_value;
        exception_keeper_tb_107 = exception_tb;
        exception_keeper_lineno_107 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_107;
        exception_value = exception_keeper_value_107;
        exception_tb = exception_keeper_tb_107;
        exception_lineno = exception_keeper_lineno_107;

        goto outline_exception_36;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_36:;
        exception_lineno = 302;
        goto try_except_handler_106;
        outline_result_36:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[34], tmp_assign_source_257);
    }
    goto try_end_36;
    // Exception handler code:
    try_except_handler_106:;
    exception_keeper_type_108 = exception_type;
    exception_keeper_value_108 = exception_value;
    exception_keeper_tb_108 = exception_tb;
    exception_keeper_lineno_108 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_36__bases);
    tmp_class_creation_36__bases = NULL;
    Py_XDECREF(tmp_class_creation_36__class_decl_dict);
    tmp_class_creation_36__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_36__metaclass);
    tmp_class_creation_36__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_36__prepared);
    tmp_class_creation_36__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_108;
    exception_value = exception_keeper_value_108;
    exception_tb = exception_keeper_tb_108;
    exception_lineno = exception_keeper_lineno_108;

    goto frame_exception_exit_1;
    // End of try:
    try_end_36:;
    Py_XDECREF(tmp_class_creation_36__bases);
    tmp_class_creation_36__bases = NULL;
    Py_XDECREF(tmp_class_creation_36__class_decl_dict);
    tmp_class_creation_36__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_36__metaclass);
    tmp_class_creation_36__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_36__prepared);
    Py_DECREF(tmp_class_creation_36__prepared);
    tmp_class_creation_36__prepared = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_259;
        PyObject *tmp_tuple_element_143;
        tmp_tuple_element_143 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[1]);

        if (unlikely(tmp_tuple_element_143 == NULL)) {
            tmp_tuple_element_143 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[1]);
        }

        if (tmp_tuple_element_143 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        tmp_assign_source_259 = PyTuple_New(1);
        PyTuple_SET_ITEM0(tmp_assign_source_259, 0, tmp_tuple_element_143);
        assert(tmp_class_creation_37__bases == NULL);
        tmp_class_creation_37__bases = tmp_assign_source_259;
    }
    {
        PyObject *tmp_assign_source_260;
        tmp_assign_source_260 = PyDict_New();
        assert(tmp_class_creation_37__class_decl_dict == NULL);
        tmp_class_creation_37__class_decl_dict = tmp_assign_source_260;
    }
    {
        PyObject *tmp_assign_source_261;
        PyObject *tmp_metaclass_name_37;
        nuitka_bool tmp_condition_result_145;
        PyObject *tmp_key_name_109;
        PyObject *tmp_dict_arg_name_109;
        PyObject *tmp_dict_arg_name_110;
        PyObject *tmp_key_name_110;
        PyObject *tmp_type_arg_73;
        PyObject *tmp_expression_name_181;
        PyObject *tmp_subscript_name_37;
        PyObject *tmp_bases_name_37;
        tmp_key_name_109 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_37__class_decl_dict);
        tmp_dict_arg_name_109 = tmp_class_creation_37__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_109, tmp_key_name_109);
        assert(!(tmp_res == -1));
        tmp_condition_result_145 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_145 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_37;
        } else {
            goto condexpr_false_37;
        }
        condexpr_true_37:;
        CHECK_OBJECT(tmp_class_creation_37__class_decl_dict);
        tmp_dict_arg_name_110 = tmp_class_creation_37__class_decl_dict;
        tmp_key_name_110 = mod_consts[49];
        tmp_metaclass_name_37 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_110, tmp_key_name_110);
        if (tmp_metaclass_name_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        goto condexpr_end_37;
        condexpr_false_37:;
        CHECK_OBJECT(tmp_class_creation_37__bases);
        tmp_expression_name_181 = tmp_class_creation_37__bases;
        tmp_subscript_name_37 = mod_consts[46];
        tmp_type_arg_73 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_181, tmp_subscript_name_37, 0);
        if (tmp_type_arg_73 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        tmp_metaclass_name_37 = BUILTIN_TYPE1(tmp_type_arg_73);
        Py_DECREF(tmp_type_arg_73);
        if (tmp_metaclass_name_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        condexpr_end_37:;
        CHECK_OBJECT(tmp_class_creation_37__bases);
        tmp_bases_name_37 = tmp_class_creation_37__bases;
        tmp_assign_source_261 = SELECT_METACLASS(tmp_metaclass_name_37, tmp_bases_name_37);
        Py_DECREF(tmp_metaclass_name_37);
        if (tmp_assign_source_261 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        assert(tmp_class_creation_37__metaclass == NULL);
        tmp_class_creation_37__metaclass = tmp_assign_source_261;
    }
    {
        nuitka_bool tmp_condition_result_146;
        PyObject *tmp_key_name_111;
        PyObject *tmp_dict_arg_name_111;
        tmp_key_name_111 = mod_consts[49];
        CHECK_OBJECT(tmp_class_creation_37__class_decl_dict);
        tmp_dict_arg_name_111 = tmp_class_creation_37__class_decl_dict;
        tmp_res = DICT_HAS_ITEM(tmp_dict_arg_name_111, tmp_key_name_111);
        assert(!(tmp_res == -1));
        tmp_condition_result_146 = (tmp_res != 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_146 == NUITKA_BOOL_TRUE) {
            goto branch_yes_109;
        } else {
            goto branch_no_109;
        }
    }
    branch_yes_109:;
    CHECK_OBJECT(tmp_class_creation_37__class_decl_dict);
    tmp_dictdel_dict = tmp_class_creation_37__class_decl_dict;
    tmp_dictdel_key = mod_consts[49];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 310;

        goto try_except_handler_109;
    }
    branch_no_109:;
    {
        nuitka_bool tmp_condition_result_147;
        PyObject *tmp_expression_name_182;
        CHECK_OBJECT(tmp_class_creation_37__metaclass);
        tmp_expression_name_182 = tmp_class_creation_37__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_182, mod_consts[50]);
        tmp_condition_result_147 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_147 == NUITKA_BOOL_TRUE) {
            goto branch_yes_110;
        } else {
            goto branch_no_110;
        }
    }
    branch_yes_110:;
    {
        PyObject *tmp_assign_source_262;
        PyObject *tmp_called_name_73;
        PyObject *tmp_expression_name_183;
        PyObject *tmp_args_name_73;
        PyObject *tmp_tuple_element_144;
        PyObject *tmp_kwargs_name_73;
        CHECK_OBJECT(tmp_class_creation_37__metaclass);
        tmp_expression_name_183 = tmp_class_creation_37__metaclass;
        tmp_called_name_73 = LOOKUP_ATTRIBUTE(tmp_expression_name_183, mod_consts[50]);
        if (tmp_called_name_73 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        tmp_tuple_element_144 = mod_consts[147];
        tmp_args_name_73 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_73, 0, tmp_tuple_element_144);
        CHECK_OBJECT(tmp_class_creation_37__bases);
        tmp_tuple_element_144 = tmp_class_creation_37__bases;
        PyTuple_SET_ITEM0(tmp_args_name_73, 1, tmp_tuple_element_144);
        CHECK_OBJECT(tmp_class_creation_37__class_decl_dict);
        tmp_kwargs_name_73 = tmp_class_creation_37__class_decl_dict;
        frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 310;
        tmp_assign_source_262 = CALL_FUNCTION(tmp_called_name_73, tmp_args_name_73, tmp_kwargs_name_73);
        Py_DECREF(tmp_called_name_73);
        Py_DECREF(tmp_args_name_73);
        if (tmp_assign_source_262 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        assert(tmp_class_creation_37__prepared == NULL);
        tmp_class_creation_37__prepared = tmp_assign_source_262;
    }
    {
        nuitka_bool tmp_condition_result_148;
        PyObject *tmp_operand_name_37;
        PyObject *tmp_expression_name_184;
        CHECK_OBJECT(tmp_class_creation_37__prepared);
        tmp_expression_name_184 = tmp_class_creation_37__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_184, mod_consts[51]);
        tmp_operand_name_37 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_37);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        tmp_condition_result_148 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_148 == NUITKA_BOOL_TRUE) {
            goto branch_yes_111;
        } else {
            goto branch_no_111;
        }
    }
    branch_yes_111:;
    {
        PyObject *tmp_raise_type_37;
        PyObject *tmp_raise_value_37;
        PyObject *tmp_left_name_37;
        PyObject *tmp_right_name_37;
        PyObject *tmp_tuple_element_145;
        PyObject *tmp_getattr_target_37;
        PyObject *tmp_getattr_attr_37;
        PyObject *tmp_getattr_default_37;
        tmp_raise_type_37 = PyExc_TypeError;
        tmp_left_name_37 = mod_consts[52];
        CHECK_OBJECT(tmp_class_creation_37__metaclass);
        tmp_getattr_target_37 = tmp_class_creation_37__metaclass;
        tmp_getattr_attr_37 = mod_consts[53];
        tmp_getattr_default_37 = mod_consts[54];
        tmp_tuple_element_145 = BUILTIN_GETATTR(tmp_getattr_target_37, tmp_getattr_attr_37, tmp_getattr_default_37);
        if (tmp_tuple_element_145 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        tmp_right_name_37 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_185;
            PyObject *tmp_type_arg_74;
            PyTuple_SET_ITEM(tmp_right_name_37, 0, tmp_tuple_element_145);
            CHECK_OBJECT(tmp_class_creation_37__prepared);
            tmp_type_arg_74 = tmp_class_creation_37__prepared;
            tmp_expression_name_185 = BUILTIN_TYPE1(tmp_type_arg_74);
            assert(!(tmp_expression_name_185 == NULL));
            tmp_tuple_element_145 = LOOKUP_ATTRIBUTE(tmp_expression_name_185, mod_consts[53]);
            Py_DECREF(tmp_expression_name_185);
            if (tmp_tuple_element_145 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 310;

                goto tuple_build_exception_43;
            }
            PyTuple_SET_ITEM(tmp_right_name_37, 1, tmp_tuple_element_145);
        }
        goto tuple_build_noexception_43;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_43:;
        Py_DECREF(tmp_right_name_37);
        goto try_except_handler_109;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_43:;
        tmp_raise_value_37 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_37, tmp_right_name_37);
        Py_DECREF(tmp_right_name_37);
        if (tmp_raise_value_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_109;
        }
        exception_type = tmp_raise_type_37;
        Py_INCREF(tmp_raise_type_37);
        exception_value = tmp_raise_value_37;
        exception_lineno = 310;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_109;
    }
    branch_no_111:;
    goto branch_end_110;
    branch_no_110:;
    {
        PyObject *tmp_assign_source_263;
        tmp_assign_source_263 = PyDict_New();
        assert(tmp_class_creation_37__prepared == NULL);
        tmp_class_creation_37__prepared = tmp_assign_source_263;
    }
    branch_end_110:;
    {
        PyObject *tmp_assign_source_264;
        {
            PyObject *tmp_set_locals_37;
            CHECK_OBJECT(tmp_class_creation_37__prepared);
            tmp_set_locals_37 = tmp_class_creation_37__prepared;
            locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310 = tmp_set_locals_37;
            Py_INCREF(tmp_set_locals_37);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[55];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310, mod_consts[56], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_111;
        }
        tmp_dictset_value = mod_consts[148];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310, mod_consts[35], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_111;
        }
        tmp_dictset_value = mod_consts[147];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310, mod_consts[58], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 310;

            goto try_except_handler_111;
        }
        {
            PyObject *tmp_assign_source_265;
            PyObject *tmp_called_name_74;
            PyObject *tmp_args_name_74;
            PyObject *tmp_tuple_element_146;
            PyObject *tmp_kwargs_name_74;
            CHECK_OBJECT(tmp_class_creation_37__metaclass);
            tmp_called_name_74 = tmp_class_creation_37__metaclass;
            tmp_tuple_element_146 = mod_consts[147];
            tmp_args_name_74 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_74, 0, tmp_tuple_element_146);
            CHECK_OBJECT(tmp_class_creation_37__bases);
            tmp_tuple_element_146 = tmp_class_creation_37__bases;
            PyTuple_SET_ITEM0(tmp_args_name_74, 1, tmp_tuple_element_146);
            tmp_tuple_element_146 = locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310;
            PyTuple_SET_ITEM0(tmp_args_name_74, 2, tmp_tuple_element_146);
            CHECK_OBJECT(tmp_class_creation_37__class_decl_dict);
            tmp_kwargs_name_74 = tmp_class_creation_37__class_decl_dict;
            frame_12850fd7f94e4dfb545b718c84e504f7->m_frame.f_lineno = 310;
            tmp_assign_source_265 = CALL_FUNCTION(tmp_called_name_74, tmp_args_name_74, tmp_kwargs_name_74);
            Py_DECREF(tmp_args_name_74);
            if (tmp_assign_source_265 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 310;

                goto try_except_handler_111;
            }
            assert(outline_36_var___class__ == NULL);
            outline_36_var___class__ = tmp_assign_source_265;
        }
        CHECK_OBJECT(outline_36_var___class__);
        tmp_assign_source_264 = outline_36_var___class__;
        Py_INCREF(tmp_assign_source_264);
        goto try_return_handler_111;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_111:;
        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310);
        locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310 = NULL;
        goto try_return_handler_110;
        // Exception handler code:
        try_except_handler_111:;
        exception_keeper_type_109 = exception_type;
        exception_keeper_value_109 = exception_value;
        exception_keeper_tb_109 = exception_tb;
        exception_keeper_lineno_109 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        Py_DECREF(locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310);
        locals_pip$_vendor$urllib3$exceptions$$$class__37_UnrewindableBodyError_310 = NULL;
        // Re-raise.
        exception_type = exception_keeper_type_109;
        exception_value = exception_keeper_value_109;
        exception_tb = exception_keeper_tb_109;
        exception_lineno = exception_keeper_lineno_109;

        goto try_except_handler_110;
        // End of try:
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_110:;
        CHECK_OBJECT(outline_36_var___class__);
        Py_DECREF(outline_36_var___class__);
        outline_36_var___class__ = NULL;
        goto outline_result_37;
        // Exception handler code:
        try_except_handler_110:;
        exception_keeper_type_110 = exception_type;
        exception_keeper_value_110 = exception_value;
        exception_keeper_tb_110 = exception_tb;
        exception_keeper_lineno_110 = exception_lineno;
        exception_type = NULL;
        exception_value = NULL;
        exception_tb = NULL;
        exception_lineno = 0;

        // Re-raise.
        exception_type = exception_keeper_type_110;
        exception_value = exception_keeper_value_110;
        exception_tb = exception_keeper_tb_110;
        exception_lineno = exception_keeper_lineno_110;

        goto outline_exception_37;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_37:;
        exception_lineno = 310;
        goto try_except_handler_109;
        outline_result_37:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$urllib3$exceptions, (Nuitka_StringObject *)mod_consts[147], tmp_assign_source_264);
    }
    goto try_end_37;
    // Exception handler code:
    try_except_handler_109:;
    exception_keeper_type_111 = exception_type;
    exception_keeper_value_111 = exception_value;
    exception_keeper_tb_111 = exception_tb;
    exception_keeper_lineno_111 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_class_creation_37__bases);
    tmp_class_creation_37__bases = NULL;
    Py_XDECREF(tmp_class_creation_37__class_decl_dict);
    tmp_class_creation_37__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_37__metaclass);
    tmp_class_creation_37__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_37__prepared);
    tmp_class_creation_37__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_111;
    exception_value = exception_keeper_value_111;
    exception_tb = exception_keeper_tb_111;
    exception_lineno = exception_keeper_lineno_111;

    goto frame_exception_exit_1;
    // End of try:
    try_end_37:;

    // Restore frame exception if necessary.
#if 0
    RESTORE_FRAME_EXCEPTION(frame_12850fd7f94e4dfb545b718c84e504f7);
#endif
    popFrameStack();

    assertFrameObject(frame_12850fd7f94e4dfb545b718c84e504f7);

    goto frame_no_exception_13;

    frame_exception_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_12850fd7f94e4dfb545b718c84e504f7);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_12850fd7f94e4dfb545b718c84e504f7, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_12850fd7f94e4dfb545b718c84e504f7->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_12850fd7f94e4dfb545b718c84e504f7, exception_lineno);
    }

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto module_exception_exit;

    frame_no_exception_13:;
    Py_XDECREF(tmp_class_creation_37__bases);
    tmp_class_creation_37__bases = NULL;
    Py_XDECREF(tmp_class_creation_37__class_decl_dict);
    tmp_class_creation_37__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_37__metaclass);
    tmp_class_creation_37__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_37__prepared);
    Py_DECREF(tmp_class_creation_37__prepared);
    tmp_class_creation_37__prepared = NULL;

    return module_pip$_vendor$urllib3$exceptions;
    module_exception_exit:
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);
    return NULL;
}

