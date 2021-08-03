/* Generated code for Python module 'numpy.core._add_newdocs'
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

/* The "module_numpy$core$_add_newdocs" is a Python object pointer of module type.
 *
 * Note: For full compatibility with CPython, every module variable access
 * needs to go through it except for cases where the module cannot possibly
 * have changed in the mean time.
 */

PyObject *module_numpy$core$_add_newdocs;
PyDictObject *moduledict_numpy$core$_add_newdocs;

/* The declarations of module constants used, if any. */
static PyObject *mod_consts[354];

static PyObject *module_filename_obj = NULL;

/* Indicator if this modules private constants were created yet. */
static bool constants_created = false;

/* Function to create module private constants. */
static void createModuleConstants(void) {
    if (constants_created == false) {
        loadConstantsBlob(&mod_consts[0], UNTRANSLATE("numpy.core._add_newdocs"));
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
void checkModuleConstants_numpy$core$_add_newdocs(void) {
    // The module may not have been used at all, then ignore this.
    if (constants_created == false) return;

    checkConstantsBlob(&mod_consts[0], "numpy.core._add_newdocs");
}
#endif

// The module code objects.
static PyCodeObject *codeobj_c220f471a98c826733824301ef8c9428;
static PyCodeObject *codeobj_f88dc747b4ef6e9a81080b2f8f093fda;
static PyCodeObject *codeobj_837e186af4984df94dc0b36608112861;
static PyCodeObject *codeobj_e2f83e1870731c26347d011a360b3353;
static PyCodeObject *codeobj_8c22d20285be3f462dc517c5631a011a;
static PyCodeObject *codeobj_6bb5e4f9f1973cbee91a038767b6b70f;

static void createModuleCodeObjects(void) {
    module_filename_obj = mod_consts[31]; CHECK_OBJECT(module_filename_obj);
    codeobj_c220f471a98c826733824301ef8c9428 = MAKE_CODEOBJECT(module_filename_obj, 6714, CO_GENERATOR | CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[25], mod_consts[347], NULL, 1, 0, 0);
    codeobj_f88dc747b4ef6e9a81080b2f8f093fda = MAKE_CODEOBJECT(module_filename_obj, 6715, CO_GENERATOR | CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[25], mod_consts[348], NULL, 1, 0, 0);
    codeobj_837e186af4984df94dc0b36608112861 = MAKE_CODEOBJECT(module_filename_obj, 1, CO_NOFREE, mod_consts[349], NULL, NULL, 0, 0, 0);
    codeobj_e2f83e1870731c26347d011a360b3353 = MAKE_CODEOBJECT(module_filename_obj, 6709, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[297], mod_consts[350], NULL, 3, 0, 0);
    codeobj_8c22d20285be3f462dc517c5631a011a = MAKE_CODEOBJECT(module_filename_obj, 6673, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE, mod_consts[295], mod_consts[351], NULL, 1, 0, 0);
    codeobj_6bb5e4f9f1973cbee91a038767b6b70f = MAKE_CODEOBJECT(module_filename_obj, 6674, CO_GENERATOR | CO_OPTIMIZED | CO_NEWLOCALS, mod_consts[0], mod_consts[352], mod_consts[353], 0, 0, 0);
}

// The module function declarations.
static PyObject *MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen(struct Nuitka_CellObject **closure);


static PyObject *MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr(struct Nuitka_CellObject **closure);


static PyObject *MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr(struct Nuitka_CellObject **closure);


static PyObject *MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases();


static PyObject *MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen(struct Nuitka_CellObject **closure);


static PyObject *MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type();


// The module function definitions.
static PyObject *impl_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    struct Nuitka_CellObject *par_aliases = Nuitka_Cell_New1(python_pars[0]);
    PyObject *var_type_aliases_gen = NULL;
    struct Nuitka_FrameObject *frame_8c22d20285be3f462dc517c5631a011a;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    static struct Nuitka_FrameObject *cache_frame_8c22d20285be3f462dc517c5631a011a = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    {
        PyObject *tmp_assign_source_1;
        struct Nuitka_CellObject *tmp_closure_1[1];

        tmp_closure_1[0] = par_aliases;
        Py_INCREF(tmp_closure_1[0]);

        tmp_assign_source_1 = MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen(tmp_closure_1);

        assert(var_type_aliases_gen == NULL);
        var_type_aliases_gen = tmp_assign_source_1;
    }
    // Tried code:
    if (isFrameUnusable(cache_frame_8c22d20285be3f462dc517c5631a011a)) {
        Py_XDECREF(cache_frame_8c22d20285be3f462dc517c5631a011a);

#if _DEBUG_REFCOUNTS
        if (cache_frame_8c22d20285be3f462dc517c5631a011a == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_8c22d20285be3f462dc517c5631a011a = MAKE_FUNCTION_FRAME(codeobj_8c22d20285be3f462dc517c5631a011a, module_numpy$core$_add_newdocs, sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_8c22d20285be3f462dc517c5631a011a->m_type_description == NULL);
    frame_8c22d20285be3f462dc517c5631a011a = cache_frame_8c22d20285be3f462dc517c5631a011a;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_8c22d20285be3f462dc517c5631a011a);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_8c22d20285be3f462dc517c5631a011a) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_list_arg_1;
        PyObject *tmp_called_name_1;
        CHECK_OBJECT(var_type_aliases_gen);
        tmp_called_name_1 = var_type_aliases_gen;
        frame_8c22d20285be3f462dc517c5631a011a->m_frame.f_lineno = 6683;
        tmp_list_arg_1 = CALL_FUNCTION_NO_ARGS(tmp_called_name_1);
        if (tmp_list_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6683;
            type_description_1 = "co";
            goto frame_exception_exit_1;
        }
        tmp_return_value = MAKE_LIST(tmp_list_arg_1);
        Py_DECREF(tmp_list_arg_1);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6683;
            type_description_1 = "co";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_8c22d20285be3f462dc517c5631a011a);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_8c22d20285be3f462dc517c5631a011a);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_8c22d20285be3f462dc517c5631a011a);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_8c22d20285be3f462dc517c5631a011a, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_8c22d20285be3f462dc517c5631a011a->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_8c22d20285be3f462dc517c5631a011a, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_8c22d20285be3f462dc517c5631a011a,
        type_description_1,
        par_aliases,
        var_type_aliases_gen
    );


    // Release cached frame if used for exception.
    if (frame_8c22d20285be3f462dc517c5631a011a == cache_frame_8c22d20285be3f462dc517c5631a011a) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_8c22d20285be3f462dc517c5631a011a);
        cache_frame_8c22d20285be3f462dc517c5631a011a = NULL;
    }

    assertFrameObject(frame_8c22d20285be3f462dc517c5631a011a);

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_aliases);
    Py_DECREF(par_aliases);
    par_aliases = NULL;
    CHECK_OBJECT(var_type_aliases_gen);
    Py_DECREF(var_type_aliases_gen);
    var_type_aliases_gen = NULL;
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

    CHECK_OBJECT(par_aliases);
    Py_DECREF(par_aliases);
    par_aliases = NULL;
    CHECK_OBJECT(var_type_aliases_gen);
    Py_DECREF(var_type_aliases_gen);
    var_type_aliases_gen = NULL;
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


static PyObject *impl_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *tmp_return_value = NULL;

    // Actual function body.
    {
        struct Nuitka_CellObject *tmp_closure_1[1];

        tmp_closure_1[0] = self->m_closure[0];
        Py_INCREF(tmp_closure_1[0]);

        tmp_return_value = MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen(tmp_closure_1);

        goto function_return_exit;
    }

    NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
    return NULL;


function_return_exit:
   // Function cleanup code if any.


   // Actual function exit with return value, making sure we did not make
   // the error status worse despite non-NULL return.
   CHECK_OBJECT(tmp_return_value);
   assert(had_error || !ERROR_OCCURRED());
   return tmp_return_value;
}



struct numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen_locals {
    PyObject *var_alias;
    PyObject *var_doc;
    PyObject *var_alias_type;
    PyObject *tmp_for_loop_1__for_iterator;
    PyObject *tmp_for_loop_1__iter_value;
    nuitka_bool tmp_try_except_1__unhandled_indicator;
    PyObject *tmp_tuple_unpack_1__element_1;
    PyObject *tmp_tuple_unpack_1__element_2;
    PyObject *tmp_tuple_unpack_1__source_iter;
    char const *type_description_1;
    PyObject *exception_type;
    PyObject *exception_value;
    PyTracebackObject *exception_tb;
    int exception_lineno;
    PyObject *tmp_iterator_attempt;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    int exception_keeper_lineno_1;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    int exception_keeper_lineno_2;
    PyObject *exception_keeper_type_3;
    PyObject *exception_keeper_value_3;
    PyTracebackObject *exception_keeper_tb_3;
    int exception_keeper_lineno_3;
    PyObject *exception_preserved_type_1;
    PyObject *exception_preserved_value_1;
    PyTracebackObject *exception_preserved_tb_1;
    int tmp_res;
    bool tmp_result;
    PyObject *exception_keeper_type_4;
    PyObject *exception_keeper_value_4;
    PyTracebackObject *exception_keeper_tb_4;
    int exception_keeper_lineno_4;
    char yield_tmps[1024];
    PyObject *exception_keeper_type_5;
    PyObject *exception_keeper_value_5;
    PyTracebackObject *exception_keeper_tb_5;
    int exception_keeper_lineno_5;
    PyObject *exception_keeper_type_6;
    PyObject *exception_keeper_value_6;
    PyTracebackObject *exception_keeper_tb_6;
    int exception_keeper_lineno_6;
};

static PyObject *numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen_context(struct Nuitka_GeneratorObject *generator, PyObject *yield_return_value) {
    CHECK_OBJECT(generator);
    assert(Nuitka_Generator_Check((PyObject *)generator));
    CHECK_OBJECT_X(yield_return_value);

    // Heap access if used.
    struct numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen_locals *generator_heap = (struct numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen_locals *)generator->m_heap_storage;

    // Dispatch to yield based on return label index:
    switch(generator->m_yield_return_index) {
    case 1: goto yield_return_1;
    }

    // Local variable initialization
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_m_frame = NULL;
    generator_heap->var_alias = NULL;
    generator_heap->var_doc = NULL;
    generator_heap->var_alias_type = NULL;
    generator_heap->tmp_for_loop_1__for_iterator = NULL;
    generator_heap->tmp_for_loop_1__iter_value = NULL;
    generator_heap->tmp_try_except_1__unhandled_indicator = NUITKA_BOOL_UNASSIGNED;
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;
    generator_heap->tmp_tuple_unpack_1__source_iter = NULL;
    generator_heap->type_description_1 = NULL;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    // Actual generator function body.
    // Tried code:
    if (isFrameUnusable(cache_m_frame)) {
        Py_XDECREF(cache_m_frame);

#if _DEBUG_REFCOUNTS
        if (cache_m_frame == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_m_frame = MAKE_FUNCTION_FRAME(codeobj_6bb5e4f9f1973cbee91a038767b6b70f, module_numpy$core$_add_newdocs, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    generator->m_frame = cache_m_frame;

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    Py_INCREF(generator->m_frame);
    assert(Py_REFCNT(generator->m_frame) == 2); // Frame stack

#if PYTHON_VERSION >= 0x340
    generator->m_frame->m_frame.f_gen = (PyObject *)generator;
#endif

    assert(generator->m_frame->m_frame.f_back == NULL);
    Py_CLEAR(generator->m_frame->m_frame.f_back);

    generator->m_frame->m_frame.f_back = PyThreadState_GET()->frame;
    Py_INCREF(generator->m_frame->m_frame.f_back);

    PyThreadState_GET()->frame = &generator->m_frame->m_frame;
    Py_INCREF(generator->m_frame);

    Nuitka_Frame_MarkAsExecuting(generator->m_frame);

#if PYTHON_VERSION >= 0x300
    // Accept currently existing exception as the one to publish again when we
    // yield or yield from.
    {
        PyThreadState *thread_state = PyThreadState_GET();

        EXC_TYPE_F(generator) = EXC_TYPE(thread_state);
        if (EXC_TYPE_F(generator) == Py_None) EXC_TYPE_F(generator) = NULL;
        Py_XINCREF(EXC_TYPE_F(generator));
        EXC_VALUE_F(generator) = EXC_VALUE(thread_state);
        Py_XINCREF(EXC_VALUE_F(generator));
        EXC_TRACEBACK_F(generator) = EXC_TRACEBACK(thread_state);
        Py_XINCREF(EXC_TRACEBACK_F(generator));
    }

#endif

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_iter_arg_1;
        if (Nuitka_Cell_GET(generator->m_closure[0]) == NULL) {

            FORMAT_UNBOUND_CLOSURE_ERROR(&generator_heap->exception_type, &generator_heap->exception_value, mod_consts[2]);
            generator_heap->exception_tb = NULL;
            NORMALIZE_EXCEPTION(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            CHAIN_EXCEPTION(generator_heap->exception_value);

            generator_heap->exception_lineno = 6675;
            generator_heap->type_description_1 = "oooc";
            goto frame_exception_exit_1;
        }

        tmp_iter_arg_1 = Nuitka_Cell_GET(generator->m_closure[0]);
        tmp_assign_source_1 = MAKE_ITERATOR(tmp_iter_arg_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6675;
            generator_heap->type_description_1 = "oooc";
            goto frame_exception_exit_1;
        }
        assert(generator_heap->tmp_for_loop_1__for_iterator == NULL);
        generator_heap->tmp_for_loop_1__for_iterator = tmp_assign_source_1;
    }
    // Tried code:
    loop_start_1:;
    {
        PyObject *tmp_next_source_1;
        PyObject *tmp_assign_source_2;
        CHECK_OBJECT(generator_heap->tmp_for_loop_1__for_iterator);
        tmp_next_source_1 = generator_heap->tmp_for_loop_1__for_iterator;
        tmp_assign_source_2 = ITERATOR_NEXT(tmp_next_source_1);
        if (tmp_assign_source_2 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_1;
            } else {

                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
                generator_heap->type_description_1 = "oooc";
                generator_heap->exception_lineno = 6675;
                goto try_except_handler_2;
            }
        }

        {
            PyObject *old = generator_heap->tmp_for_loop_1__iter_value;
            generator_heap->tmp_for_loop_1__iter_value = tmp_assign_source_2;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_3;
        PyObject *tmp_iter_arg_2;
        CHECK_OBJECT(generator_heap->tmp_for_loop_1__iter_value);
        tmp_iter_arg_2 = generator_heap->tmp_for_loop_1__iter_value;
        tmp_assign_source_3 = MAKE_ITERATOR(tmp_iter_arg_2);
        if (tmp_assign_source_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6675;
            generator_heap->type_description_1 = "oooc";
            goto try_except_handler_3;
        }
        {
            PyObject *old = generator_heap->tmp_tuple_unpack_1__source_iter;
            generator_heap->tmp_tuple_unpack_1__source_iter = tmp_assign_source_3;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_4;
        PyObject *tmp_unpack_1;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__source_iter);
        tmp_unpack_1 = generator_heap->tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_4 = UNPACK_NEXT(tmp_unpack_1, 0, 2);
        if (tmp_assign_source_4 == NULL) {
            if (!ERROR_OCCURRED()) {
                generator_heap->exception_type = PyExc_StopIteration;
                Py_INCREF(generator_heap->exception_type);
                generator_heap->exception_value = NULL;
                generator_heap->exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            }


            generator_heap->type_description_1 = "oooc";
            generator_heap->exception_lineno = 6675;
            goto try_except_handler_4;
        }
        {
            PyObject *old = generator_heap->tmp_tuple_unpack_1__element_1;
            generator_heap->tmp_tuple_unpack_1__element_1 = tmp_assign_source_4;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_unpack_2;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__source_iter);
        tmp_unpack_2 = generator_heap->tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_5 = UNPACK_NEXT(tmp_unpack_2, 1, 2);
        if (tmp_assign_source_5 == NULL) {
            if (!ERROR_OCCURRED()) {
                generator_heap->exception_type = PyExc_StopIteration;
                Py_INCREF(generator_heap->exception_type);
                generator_heap->exception_value = NULL;
                generator_heap->exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            }


            generator_heap->type_description_1 = "oooc";
            generator_heap->exception_lineno = 6675;
            goto try_except_handler_4;
        }
        {
            PyObject *old = generator_heap->tmp_tuple_unpack_1__element_2;
            generator_heap->tmp_tuple_unpack_1__element_2 = tmp_assign_source_5;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_iterator_name_1;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__source_iter);
        tmp_iterator_name_1 = generator_heap->tmp_tuple_unpack_1__source_iter;
        // Check if iterator has left-over elements.
        CHECK_OBJECT(tmp_iterator_name_1); assert(HAS_ITERNEXT(tmp_iterator_name_1));

        generator_heap->tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_1)->tp_iternext)(tmp_iterator_name_1);

        if (likely(generator_heap->tmp_iterator_attempt == NULL)) {
            PyObject *error = GET_ERROR_OCCURRED();

            if (error != NULL) {
                if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                    CLEAR_ERROR_OCCURRED();
                } else {
                    FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);

                    generator_heap->type_description_1 = "oooc";
                    generator_heap->exception_lineno = 6675;
                    goto try_except_handler_4;
                }
            }
        } else {
            Py_DECREF(generator_heap->tmp_iterator_attempt);

            generator_heap->exception_type = PyExc_ValueError;
            Py_INCREF(PyExc_ValueError);
            generator_heap->exception_value = mod_consts[3];
            Py_INCREF(generator_heap->exception_value);
            generator_heap->exception_tb = NULL;

            generator_heap->type_description_1 = "oooc";
            generator_heap->exception_lineno = 6675;
            goto try_except_handler_4;
        }
    }
    goto try_end_1;
    // Exception handler code:
    try_except_handler_4:;
    generator_heap->exception_keeper_type_1 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_1 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_1 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_1 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__source_iter);
    generator_heap->tmp_tuple_unpack_1__source_iter = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_1;
    generator_heap->exception_value = generator_heap->exception_keeper_value_1;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_1;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_1;

    goto try_except_handler_3;
    // End of try:
    try_end_1:;
    goto try_end_2;
    // Exception handler code:
    try_except_handler_3:;
    generator_heap->exception_keeper_type_2 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_2 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_2 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_2 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_1);
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_2);
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_2;
    generator_heap->exception_value = generator_heap->exception_keeper_value_2;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_2;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_2;

    goto try_except_handler_2;
    // End of try:
    try_end_2:;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__source_iter);
    generator_heap->tmp_tuple_unpack_1__source_iter = NULL;
    {
        PyObject *tmp_assign_source_6;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__element_1);
        tmp_assign_source_6 = generator_heap->tmp_tuple_unpack_1__element_1;
        {
            PyObject *old = generator_heap->var_alias;
            generator_heap->var_alias = tmp_assign_source_6;
            Py_INCREF(generator_heap->var_alias);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_1);
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;

    {
        PyObject *tmp_assign_source_7;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__element_2);
        tmp_assign_source_7 = generator_heap->tmp_tuple_unpack_1__element_2;
        {
            PyObject *old = generator_heap->var_doc;
            generator_heap->var_doc = tmp_assign_source_7;
            Py_INCREF(generator_heap->var_doc);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_2);
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;

    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_1);
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_2);
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;
    {
        nuitka_bool tmp_assign_source_8;
        tmp_assign_source_8 = NUITKA_BOOL_TRUE;
        generator_heap->tmp_try_except_1__unhandled_indicator = tmp_assign_source_8;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_9;
        PyObject *tmp_getattr_target_1;
        PyObject *tmp_getattr_attr_1;
        tmp_getattr_target_1 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[4]);

        if (unlikely(tmp_getattr_target_1 == NULL)) {
            tmp_getattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[4]);
        }

        if (tmp_getattr_target_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6677;
            generator_heap->type_description_1 = "oooc";
            goto try_except_handler_5;
        }
        CHECK_OBJECT(generator_heap->var_alias);
        tmp_getattr_attr_1 = generator_heap->var_alias;
        tmp_assign_source_9 = BUILTIN_GETATTR(tmp_getattr_target_1, tmp_getattr_attr_1, NULL);
        if (tmp_assign_source_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6677;
            generator_heap->type_description_1 = "oooc";
            goto try_except_handler_5;
        }
        {
            PyObject *old = generator_heap->var_alias_type;
            generator_heap->var_alias_type = tmp_assign_source_9;
            Py_XDECREF(old);
        }

    }
    goto try_end_3;
    // Exception handler code:
    try_except_handler_5:;
    generator_heap->exception_keeper_type_3 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_3 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_3 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_3 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    {
        nuitka_bool tmp_assign_source_10;
        tmp_assign_source_10 = NUITKA_BOOL_FALSE;
        generator_heap->tmp_try_except_1__unhandled_indicator = tmp_assign_source_10;
    }
    // Preserve existing published exception id 1.
    GET_CURRENT_EXCEPTION(&generator_heap->exception_preserved_type_1, &generator_heap->exception_preserved_value_1, &generator_heap->exception_preserved_tb_1);

    if (generator_heap->exception_keeper_tb_3 == NULL) {
        generator_heap->exception_keeper_tb_3 = MAKE_TRACEBACK(generator->m_frame, generator_heap->exception_keeper_lineno_3);
    } else if (generator_heap->exception_keeper_lineno_3 != 0) {
        generator_heap->exception_keeper_tb_3 = ADD_TRACEBACK(generator_heap->exception_keeper_tb_3, generator->m_frame, generator_heap->exception_keeper_lineno_3);
    }

    NORMALIZE_EXCEPTION(&generator_heap->exception_keeper_type_3, &generator_heap->exception_keeper_value_3, &generator_heap->exception_keeper_tb_3);
    ATTACH_TRACEBACK_TO_EXCEPTION_VALUE(generator_heap->exception_keeper_value_3, generator_heap->exception_keeper_tb_3);
    PUBLISH_EXCEPTION(&generator_heap->exception_keeper_type_3, &generator_heap->exception_keeper_value_3, &generator_heap->exception_keeper_tb_3);
    // Tried code:
    {
        bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        tmp_compexpr_left_1 = EXC_TYPE(PyThreadState_GET());
        tmp_compexpr_right_1 = PyExc_AttributeError;
        generator_heap->tmp_res = EXCEPTION_MATCH_BOOL(tmp_compexpr_left_1, tmp_compexpr_right_1);
        assert(!(generator_heap->tmp_res == -1));
        tmp_condition_result_1 = (generator_heap->tmp_res == 0) ? true : false;
        if (tmp_condition_result_1 != false) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    generator_heap->tmp_result = RERAISE_EXCEPTION(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
    if (unlikely(generator_heap->tmp_result == false)) {
        generator_heap->exception_lineno = 6676;
    }

    if (generator_heap->exception_tb && generator_heap->exception_tb->tb_frame == &generator->m_frame->m_frame) generator->m_frame->m_frame.f_lineno = generator_heap->exception_tb->tb_lineno;
    generator_heap->type_description_1 = "oooc";
    goto try_except_handler_6;
    branch_no_1:;
    goto try_end_4;
    // Exception handler code:
    try_except_handler_6:;
    generator_heap->exception_keeper_type_4 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_4 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_4 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_4 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    // Restore previous exception id 1.
    SET_CURRENT_EXCEPTION(generator_heap->exception_preserved_type_1, generator_heap->exception_preserved_value_1, generator_heap->exception_preserved_tb_1);

    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_4;
    generator_heap->exception_value = generator_heap->exception_keeper_value_4;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_4;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_4;

    goto try_except_handler_2;
    // End of try:
    try_end_4:;
    // Restore previous exception id 1.
    SET_CURRENT_EXCEPTION(generator_heap->exception_preserved_type_1, generator_heap->exception_preserved_value_1, generator_heap->exception_preserved_tb_1);

    goto try_end_3;
    NUITKA_CANNOT_GET_HERE("exception handler codes exits in all cases");
    return NULL;
    // End of try:
    try_end_3:;
    {
        bool tmp_condition_result_2;
        nuitka_bool tmp_compexpr_left_2;
        nuitka_bool tmp_compexpr_right_2;
        assert(generator_heap->tmp_try_except_1__unhandled_indicator != NUITKA_BOOL_UNASSIGNED);
        tmp_compexpr_left_2 = generator_heap->tmp_try_except_1__unhandled_indicator;
        tmp_compexpr_right_2 = NUITKA_BOOL_TRUE;
        tmp_condition_result_2 = (tmp_compexpr_left_2 == tmp_compexpr_right_2) ? true : false;
        if (tmp_condition_result_2 != false) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_expression_name_1;
        PyObject *tmp_tuple_element_1;
        NUITKA_MAY_BE_UNUSED PyObject *tmp_yield_result_1;
        if (generator_heap->var_alias_type == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&generator_heap->exception_type, &generator_heap->exception_value, mod_consts[5]);
            generator_heap->exception_tb = NULL;
            NORMALIZE_EXCEPTION(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            CHAIN_EXCEPTION(generator_heap->exception_value);

            generator_heap->exception_lineno = 6682;
            generator_heap->type_description_1 = "oooc";
            goto try_except_handler_2;
        }

        tmp_tuple_element_1 = generator_heap->var_alias_type;
        tmp_expression_name_1 = PyTuple_New(3);
        PyTuple_SET_ITEM0(tmp_expression_name_1, 0, tmp_tuple_element_1);
        CHECK_OBJECT(generator_heap->var_alias);
        tmp_tuple_element_1 = generator_heap->var_alias;
        PyTuple_SET_ITEM0(tmp_expression_name_1, 1, tmp_tuple_element_1);
        CHECK_OBJECT(generator_heap->var_doc);
        tmp_tuple_element_1 = generator_heap->var_doc;
        PyTuple_SET_ITEM0(tmp_expression_name_1, 2, tmp_tuple_element_1);
        Nuitka_PreserveHeap(generator_heap->yield_tmps, &tmp_tuple_element_1, sizeof(PyObject *), NULL);
        generator->m_yield_return_index = 1;
        return tmp_expression_name_1;
        yield_return_1:
        Nuitka_RestoreHeap(generator_heap->yield_tmps, &tmp_tuple_element_1, sizeof(PyObject *), NULL);
        if (yield_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6682;
            generator_heap->type_description_1 = "oooc";
            goto try_except_handler_2;
        }
        tmp_yield_result_1 = yield_return_value;
    }
    branch_no_2:;
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


        generator_heap->exception_lineno = 6675;
        generator_heap->type_description_1 = "oooc";
        goto try_except_handler_2;
    }
    goto loop_start_1;
    loop_end_1:;
    goto try_end_5;
    // Exception handler code:
    try_except_handler_2:;
    generator_heap->exception_keeper_type_5 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_5 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_5 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_5 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->tmp_for_loop_1__iter_value);
    generator_heap->tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(generator_heap->tmp_for_loop_1__for_iterator);
    generator_heap->tmp_for_loop_1__for_iterator = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_5;
    generator_heap->exception_value = generator_heap->exception_keeper_value_5;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_5;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_5;

    goto frame_exception_exit_1;
    // End of try:
    try_end_5:;

    Nuitka_Frame_MarkAsNotExecuting(generator->m_frame);

#if PYTHON_VERSION >= 0x300
    Py_CLEAR(EXC_TYPE_F(generator));
    Py_CLEAR(EXC_VALUE_F(generator));
    Py_CLEAR(EXC_TRACEBACK_F(generator));
#endif

    // Allow re-use of the frame again.
    Py_DECREF(generator->m_frame);
    goto frame_no_exception_1;

    frame_exception_exit_1:;

    // If it's not an exit exception, consider and create a traceback for it.
    if (!EXCEPTION_MATCH_GENERATOR(generator_heap->exception_type)) {
        if (generator_heap->exception_tb == NULL) {
            generator_heap->exception_tb = MAKE_TRACEBACK(generator->m_frame, generator_heap->exception_lineno);
        } else if (generator_heap->exception_tb->tb_frame != &generator->m_frame->m_frame) {
            generator_heap->exception_tb = ADD_TRACEBACK(generator_heap->exception_tb, generator->m_frame, generator_heap->exception_lineno);
        }

        Nuitka_Frame_AttachLocals(
            generator->m_frame,
            generator_heap->type_description_1,
            generator_heap->var_alias,
            generator_heap->var_doc,
            generator_heap->var_alias_type,
            generator->m_closure[0]
        );


        // Release cached frame if used for exception.
        if (generator->m_frame == cache_m_frame) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_m_frame);
            cache_m_frame = NULL;
        }

        assertFrameObject(generator->m_frame);
    }

#if PYTHON_VERSION >= 0x300
    Py_CLEAR(EXC_TYPE_F(generator));
    Py_CLEAR(EXC_VALUE_F(generator));
    Py_CLEAR(EXC_TRACEBACK_F(generator));
#endif

    Py_DECREF(generator->m_frame);

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    goto try_end_6;
    // Exception handler code:
    try_except_handler_1:;
    generator_heap->exception_keeper_type_6 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_6 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_6 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_6 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->var_alias);
    generator_heap->var_alias = NULL;
    Py_XDECREF(generator_heap->var_doc);
    generator_heap->var_doc = NULL;
    Py_XDECREF(generator_heap->var_alias_type);
    generator_heap->var_alias_type = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_6;
    generator_heap->exception_value = generator_heap->exception_keeper_value_6;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_6;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_6;

    goto function_exception_exit;
    // End of try:
    try_end_6:;
    Py_XDECREF(generator_heap->tmp_for_loop_1__iter_value);
    generator_heap->tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(generator_heap->tmp_for_loop_1__for_iterator);
    generator_heap->tmp_for_loop_1__for_iterator = NULL;
    Py_XDECREF(generator_heap->var_alias);
    generator_heap->var_alias = NULL;
    Py_XDECREF(generator_heap->var_doc);
    generator_heap->var_doc = NULL;
    Py_XDECREF(generator_heap->var_alias_type);
    generator_heap->var_alias_type = NULL;


    return NULL;

    function_exception_exit:
    assert(generator_heap->exception_type);
    RESTORE_ERROR_OCCURRED(generator_heap->exception_type, generator_heap->exception_value, generator_heap->exception_tb);

    return NULL;

}

static PyObject *MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen(struct Nuitka_CellObject **closure) {
    return Nuitka_Generator_New(
        numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen_context,
        module_numpy$core$_add_newdocs,
        mod_consts[0],
#if PYTHON_VERSION >= 0x350
        mod_consts[1],
#endif
        codeobj_6bb5e4f9f1973cbee91a038767b6b70f,
        closure,
        1,
        sizeof(struct numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen$$$genobj__1_type_aliases_gen_locals)
    );
}


static PyObject *impl_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_obj = python_pars[0];
    PyObject *par_fixed_aliases = python_pars[1];
    PyObject *par_doc = python_pars[2];
    struct Nuitka_CellObject *var_o = Nuitka_Cell_Empty();
    PyObject *var_character_code = NULL;
    PyObject *var_canonical_name_doc = NULL;
    PyObject *var_alias_doc = NULL;
    PyObject *var_docstring = NULL;
    PyObject *tmp_genexpr_1__$0 = NULL;
    PyObject *tmp_genexpr_2__$0 = NULL;
    struct Nuitka_FrameObject *frame_e2f83e1870731c26347d011a360b3353;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    int tmp_res;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_frame_e2f83e1870731c26347d011a360b3353 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_e2f83e1870731c26347d011a360b3353)) {
        Py_XDECREF(cache_frame_e2f83e1870731c26347d011a360b3353);

#if _DEBUG_REFCOUNTS
        if (cache_frame_e2f83e1870731c26347d011a360b3353 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_e2f83e1870731c26347d011a360b3353 = MAKE_FUNCTION_FRAME(codeobj_e2f83e1870731c26347d011a360b3353, module_numpy$core$_add_newdocs, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_e2f83e1870731c26347d011a360b3353->m_type_description == NULL);
    frame_e2f83e1870731c26347d011a360b3353 = cache_frame_e2f83e1870731c26347d011a360b3353;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_e2f83e1870731c26347d011a360b3353);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_e2f83e1870731c26347d011a360b3353) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_getattr_target_1;
        PyObject *tmp_getattr_attr_1;
        tmp_getattr_target_1 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[4]);

        if (unlikely(tmp_getattr_target_1 == NULL)) {
            tmp_getattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[4]);
        }

        if (tmp_getattr_target_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6710;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_obj);
        tmp_getattr_attr_1 = par_obj;
        tmp_assign_source_1 = BUILTIN_GETATTR(tmp_getattr_target_1, tmp_getattr_attr_1, NULL);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6710;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        assert(Nuitka_Cell_GET(var_o) == NULL);
        PyCell_SET(var_o, tmp_assign_source_1);

    }
    {
        PyObject *tmp_assign_source_2;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_called_name_1;
        PyObject *tmp_args_element_name_1;
        tmp_called_name_1 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[6]);

        if (unlikely(tmp_called_name_1 == NULL)) {
            tmp_called_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[6]);
        }

        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6712;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(Nuitka_Cell_GET(var_o));
        tmp_args_element_name_1 = Nuitka_Cell_GET(var_o);
        frame_e2f83e1870731c26347d011a360b3353->m_frame.f_lineno = 6712;
        tmp_expression_name_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_1, tmp_args_element_name_1);
        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6712;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[7]);
        Py_DECREF(tmp_expression_name_1);
        if (tmp_assign_source_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6712;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        assert(var_character_code == NULL);
        var_character_code = tmp_assign_source_2;
    }
    {
        PyObject *tmp_assign_source_3;
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_2;
        CHECK_OBJECT(par_obj);
        tmp_compexpr_left_1 = par_obj;
        if (Nuitka_Cell_GET(var_o) == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[8]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 6713;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_2 = Nuitka_Cell_GET(var_o);
        tmp_compexpr_right_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[9]);
        if (tmp_compexpr_right_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6713;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_1 = RICH_COMPARE_EQ_NBOOL_OBJECT_OBJECT(tmp_compexpr_left_1, tmp_compexpr_right_1);
        Py_DECREF(tmp_compexpr_right_1);
        if (tmp_condition_result_1 == NUITKA_BOOL_EXCEPTION) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6713;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto condexpr_true_1;
        } else {
            goto condexpr_false_1;
        }
        assert(tmp_condition_result_1 != NUITKA_BOOL_UNASSIGNED);
        condexpr_true_1:;
        tmp_assign_source_3 = mod_consts[10];
        Py_INCREF(tmp_assign_source_3);
        goto condexpr_end_1;
        condexpr_false_1:;
        tmp_called_instance_1 = mod_consts[11];
        CHECK_OBJECT(par_obj);
        tmp_args_element_name_2 = par_obj;
        frame_e2f83e1870731c26347d011a360b3353->m_frame.f_lineno = 6713;
        tmp_assign_source_3 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[12], tmp_args_element_name_2);
        if (tmp_assign_source_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6713;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        condexpr_end_1:;
        assert(var_canonical_name_doc == NULL);
        var_canonical_name_doc = tmp_assign_source_3;
    }
    {
        PyObject *tmp_assign_source_4;
        PyObject *tmp_called_name_2;
        PyObject *tmp_expression_name_3;
        PyObject *tmp_args_element_name_3;
        tmp_expression_name_3 = mod_consts[10];
        tmp_called_name_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_3, mod_consts[13]);
        assert(!(tmp_called_name_2 == NULL));
        {
            PyObject *tmp_assign_source_5;
            PyObject *tmp_iter_arg_1;
            CHECK_OBJECT(par_fixed_aliases);
            tmp_iter_arg_1 = par_fixed_aliases;
            tmp_assign_source_5 = MAKE_ITERATOR(tmp_iter_arg_1);
            if (tmp_assign_source_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 6714;
                type_description_1 = "ooocoooo";
                goto frame_exception_exit_1;
            }
            assert(tmp_genexpr_1__$0 == NULL);
            tmp_genexpr_1__$0 = tmp_assign_source_5;
        }
        // Tried code:
        {
            struct Nuitka_CellObject *tmp_closure_1[1];

            tmp_closure_1[0] = Nuitka_Cell_New0(tmp_genexpr_1__$0);

            tmp_args_element_name_3 = MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr(tmp_closure_1);

            goto try_return_handler_2;
        }
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_2:;
        CHECK_OBJECT(tmp_genexpr_1__$0);
        Py_DECREF(tmp_genexpr_1__$0);
        tmp_genexpr_1__$0 = NULL;
        goto outline_result_1;
        // End of try:
        CHECK_OBJECT(tmp_genexpr_1__$0);
        Py_DECREF(tmp_genexpr_1__$0);
        tmp_genexpr_1__$0 = NULL;
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_result_1:;
        frame_e2f83e1870731c26347d011a360b3353->m_frame.f_lineno = 6714;
        tmp_assign_source_4 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_2, tmp_args_element_name_3);
        Py_DECREF(tmp_called_name_2);
        Py_DECREF(tmp_args_element_name_3);
        if (tmp_assign_source_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6714;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        assert(var_alias_doc == NULL);
        var_alias_doc = tmp_assign_source_4;
    }
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_called_name_3;
        PyObject *tmp_expression_name_4;
        PyObject *tmp_args_element_name_4;
        CHECK_OBJECT(var_alias_doc);
        tmp_left_name_1 = var_alias_doc;
        tmp_expression_name_4 = mod_consts[10];
        tmp_called_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_4, mod_consts[13]);
        assert(!(tmp_called_name_3 == NULL));
        {
            PyObject *tmp_assign_source_7;
            PyObject *tmp_iter_arg_2;
            tmp_iter_arg_2 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[14]);

            if (unlikely(tmp_iter_arg_2 == NULL)) {
                tmp_iter_arg_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[14]);
            }

            if (tmp_iter_arg_2 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 6716;
                type_description_1 = "ooocoooo";
                goto frame_exception_exit_1;
            }
            tmp_assign_source_7 = MAKE_ITERATOR(tmp_iter_arg_2);
            if (tmp_assign_source_7 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 6715;
                type_description_1 = "ooocoooo";
                goto frame_exception_exit_1;
            }
            assert(tmp_genexpr_2__$0 == NULL);
            tmp_genexpr_2__$0 = tmp_assign_source_7;
        }
        // Tried code:
        {
            struct Nuitka_CellObject *tmp_closure_2[2];

            tmp_closure_2[0] = Nuitka_Cell_New0(tmp_genexpr_2__$0);
            tmp_closure_2[1] = var_o;
            Py_INCREF(tmp_closure_2[1]);

            tmp_args_element_name_4 = MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr(tmp_closure_2);

            goto try_return_handler_3;
        }
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_3:;
        CHECK_OBJECT(tmp_genexpr_2__$0);
        Py_DECREF(tmp_genexpr_2__$0);
        tmp_genexpr_2__$0 = NULL;
        goto outline_result_2;
        // End of try:
        CHECK_OBJECT(tmp_genexpr_2__$0);
        Py_DECREF(tmp_genexpr_2__$0);
        tmp_genexpr_2__$0 = NULL;
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_result_2:;
        frame_e2f83e1870731c26347d011a360b3353->m_frame.f_lineno = 6715;
        tmp_right_name_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_3, tmp_args_element_name_4);
        Py_DECREF(tmp_called_name_3);
        Py_DECREF(tmp_args_element_name_4);
        if (tmp_right_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6715;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        tmp_result = BINARY_OPERATION_ADD_OBJECT_OBJECT_INPLACE(&tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6715;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_6 = tmp_left_name_1;
        var_alias_doc = tmp_assign_source_6;

    }
    {
        PyObject *tmp_assign_source_8;
        PyObject *tmp_called_name_4;
        PyObject *tmp_expression_name_5;
        PyObject *tmp_kwargs_name_1;
        PyObject *tmp_dict_key_1;
        PyObject *tmp_dict_value_1;
        PyObject *tmp_called_instance_2;
        tmp_expression_name_5 = mod_consts[15];
        tmp_called_name_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_5, mod_consts[12]);
        assert(!(tmp_called_name_4 == NULL));
        tmp_dict_key_1 = mod_consts[16];
        CHECK_OBJECT(par_doc);
        tmp_called_instance_2 = par_doc;
        frame_e2f83e1870731c26347d011a360b3353->m_frame.f_lineno = 6722;
        tmp_dict_value_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_2, mod_consts[17]);
        if (tmp_dict_value_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_4);

            exception_lineno = 6722;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        tmp_kwargs_name_1 = _PyDict_NewPresized( 4 );
        tmp_res = PyDict_SetItem(tmp_kwargs_name_1, tmp_dict_key_1, tmp_dict_value_1);
        Py_DECREF(tmp_dict_value_1);
        assert(!(tmp_res != 0));
        tmp_dict_key_1 = mod_consts[18];
        CHECK_OBJECT(var_character_code);
        tmp_dict_value_1 = var_character_code;
        tmp_res = PyDict_SetItem(tmp_kwargs_name_1, tmp_dict_key_1, tmp_dict_value_1);
        assert(!(tmp_res != 0));
        tmp_dict_key_1 = mod_consts[19];
        CHECK_OBJECT(var_canonical_name_doc);
        tmp_dict_value_1 = var_canonical_name_doc;
        tmp_res = PyDict_SetItem(tmp_kwargs_name_1, tmp_dict_key_1, tmp_dict_value_1);
        assert(!(tmp_res != 0));
        tmp_dict_key_1 = mod_consts[20];
        CHECK_OBJECT(var_alias_doc);
        tmp_dict_value_1 = var_alias_doc;
        tmp_res = PyDict_SetItem(tmp_kwargs_name_1, tmp_dict_key_1, tmp_dict_value_1);
        assert(!(tmp_res != 0));
        frame_e2f83e1870731c26347d011a360b3353->m_frame.f_lineno = 6722;
        tmp_assign_source_8 = CALL_FUNCTION_WITH_KEYARGS(tmp_called_name_4, tmp_kwargs_name_1);
        Py_DECREF(tmp_called_name_4);
        Py_DECREF(tmp_kwargs_name_1);
        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6722;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        assert(var_docstring == NULL);
        var_docstring = tmp_assign_source_8;
    }
    {
        PyObject *tmp_called_name_5;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_5;
        PyObject *tmp_args_element_name_6;
        PyObject *tmp_args_element_name_7;
        tmp_called_name_5 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_5 == NULL)) {
            tmp_called_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6725;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_5 = mod_consts[22];
        if (par_obj == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[23]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 6725;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }

        tmp_args_element_name_6 = par_obj;
        CHECK_OBJECT(var_docstring);
        tmp_args_element_name_7 = var_docstring;
        frame_e2f83e1870731c26347d011a360b3353->m_frame.f_lineno = 6725;
        {
            PyObject *call_args[] = {tmp_args_element_name_5, tmp_args_element_name_6, tmp_args_element_name_7};
            tmp_call_result_1 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_5, call_args);
        }

        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6725;
            type_description_1 = "ooocoooo";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_e2f83e1870731c26347d011a360b3353);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_e2f83e1870731c26347d011a360b3353);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_e2f83e1870731c26347d011a360b3353, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_e2f83e1870731c26347d011a360b3353->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_e2f83e1870731c26347d011a360b3353, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_e2f83e1870731c26347d011a360b3353,
        type_description_1,
        par_obj,
        par_fixed_aliases,
        par_doc,
        var_o,
        var_character_code,
        var_canonical_name_doc,
        var_alias_doc,
        var_docstring
    );


    // Release cached frame if used for exception.
    if (frame_e2f83e1870731c26347d011a360b3353 == cache_frame_e2f83e1870731c26347d011a360b3353) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_e2f83e1870731c26347d011a360b3353);
        cache_frame_e2f83e1870731c26347d011a360b3353 = NULL;
    }

    assertFrameObject(frame_e2f83e1870731c26347d011a360b3353);

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
    Py_XDECREF(par_obj);
    par_obj = NULL;
    Py_XDECREF(par_fixed_aliases);
    par_fixed_aliases = NULL;
    Py_XDECREF(par_doc);
    par_doc = NULL;
    CHECK_OBJECT(var_o);
    Py_DECREF(var_o);
    var_o = NULL;
    Py_XDECREF(var_character_code);
    var_character_code = NULL;
    Py_XDECREF(var_canonical_name_doc);
    var_canonical_name_doc = NULL;
    Py_XDECREF(var_alias_doc);
    var_alias_doc = NULL;
    CHECK_OBJECT(var_docstring);
    Py_DECREF(var_docstring);
    var_docstring = NULL;
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

    Py_XDECREF(par_obj);
    par_obj = NULL;
    Py_XDECREF(par_fixed_aliases);
    par_fixed_aliases = NULL;
    Py_XDECREF(par_doc);
    par_doc = NULL;
    CHECK_OBJECT(var_o);
    Py_DECREF(var_o);
    var_o = NULL;
    Py_XDECREF(var_character_code);
    var_character_code = NULL;
    Py_XDECREF(var_canonical_name_doc);
    var_canonical_name_doc = NULL;
    Py_XDECREF(var_alias_doc);
    var_alias_doc = NULL;
    Py_XDECREF(var_docstring);
    var_docstring = NULL;
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



struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr_locals {
    PyObject *var_alias;
    PyObject *tmp_iter_value_0;
    char const *type_description_1;
    PyObject *exception_type;
    PyObject *exception_value;
    PyTracebackObject *exception_tb;
    int exception_lineno;
    char yield_tmps[1024];
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    int exception_keeper_lineno_1;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    int exception_keeper_lineno_2;
};

static PyObject *numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr_context(struct Nuitka_GeneratorObject *generator, PyObject *yield_return_value) {
    CHECK_OBJECT(generator);
    assert(Nuitka_Generator_Check((PyObject *)generator));
    CHECK_OBJECT_X(yield_return_value);

    // Heap access if used.
    struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr_locals *generator_heap = (struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr_locals *)generator->m_heap_storage;

    // Dispatch to yield based on return label index:
    switch(generator->m_yield_return_index) {
    case 1: goto yield_return_1;
    }

    // Local variable initialization
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_m_frame = NULL;
    generator_heap->var_alias = NULL;
    generator_heap->tmp_iter_value_0 = NULL;
    generator_heap->type_description_1 = NULL;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    // Actual generator function body.
    // Tried code:
    if (isFrameUnusable(cache_m_frame)) {
        Py_XDECREF(cache_m_frame);

#if _DEBUG_REFCOUNTS
        if (cache_m_frame == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_m_frame = MAKE_FUNCTION_FRAME(codeobj_c220f471a98c826733824301ef8c9428, module_numpy$core$_add_newdocs, sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    generator->m_frame = cache_m_frame;

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    Py_INCREF(generator->m_frame);
    assert(Py_REFCNT(generator->m_frame) == 2); // Frame stack

#if PYTHON_VERSION >= 0x340
    generator->m_frame->m_frame.f_gen = (PyObject *)generator;
#endif

    assert(generator->m_frame->m_frame.f_back == NULL);
    Py_CLEAR(generator->m_frame->m_frame.f_back);

    generator->m_frame->m_frame.f_back = PyThreadState_GET()->frame;
    Py_INCREF(generator->m_frame->m_frame.f_back);

    PyThreadState_GET()->frame = &generator->m_frame->m_frame;
    Py_INCREF(generator->m_frame);

    Nuitka_Frame_MarkAsExecuting(generator->m_frame);

#if PYTHON_VERSION >= 0x300
    // Accept currently existing exception as the one to publish again when we
    // yield or yield from.
    {
        PyThreadState *thread_state = PyThreadState_GET();

        EXC_TYPE_F(generator) = EXC_TYPE(thread_state);
        if (EXC_TYPE_F(generator) == Py_None) EXC_TYPE_F(generator) = NULL;
        Py_XINCREF(EXC_TYPE_F(generator));
        EXC_VALUE_F(generator) = EXC_VALUE(thread_state);
        Py_XINCREF(EXC_VALUE_F(generator));
        EXC_TRACEBACK_F(generator) = EXC_TRACEBACK(thread_state);
        Py_XINCREF(EXC_TRACEBACK_F(generator));
    }

#endif

    // Framed code:
    // Tried code:
    loop_start_1:;
    {
        PyObject *tmp_next_source_1;
        PyObject *tmp_assign_source_1;
        CHECK_OBJECT(Nuitka_Cell_GET(generator->m_closure[0]));
        tmp_next_source_1 = Nuitka_Cell_GET(generator->m_closure[0]);
        tmp_assign_source_1 = ITERATOR_NEXT(tmp_next_source_1);
        if (tmp_assign_source_1 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_1;
            } else {

                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
                generator_heap->type_description_1 = "No";
                generator_heap->exception_lineno = 6714;
                goto try_except_handler_2;
            }
        }

        {
            PyObject *old = generator_heap->tmp_iter_value_0;
            generator_heap->tmp_iter_value_0 = tmp_assign_source_1;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_2;
        CHECK_OBJECT(generator_heap->tmp_iter_value_0);
        tmp_assign_source_2 = generator_heap->tmp_iter_value_0;
        {
            PyObject *old = generator_heap->var_alias;
            generator_heap->var_alias = tmp_assign_source_2;
            Py_INCREF(generator_heap->var_alias);
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_expression_name_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_1;
        NUITKA_MAY_BE_UNUSED PyObject *tmp_yield_result_1;
        tmp_called_instance_1 = mod_consts[24];
        CHECK_OBJECT(generator_heap->var_alias);
        tmp_args_element_name_1 = generator_heap->var_alias;
        generator->m_frame->m_frame.f_lineno = 6714;
        tmp_expression_name_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[12], tmp_args_element_name_1);
        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6714;
            generator_heap->type_description_1 = "No";
            goto try_except_handler_2;
        }
        Nuitka_PreserveHeap(generator_heap->yield_tmps, &tmp_called_instance_1, sizeof(PyObject *), &tmp_args_element_name_1, sizeof(PyObject *), NULL);
        generator->m_yield_return_index = 1;
        return tmp_expression_name_1;
        yield_return_1:
        Nuitka_RestoreHeap(generator_heap->yield_tmps, &tmp_called_instance_1, sizeof(PyObject *), &tmp_args_element_name_1, sizeof(PyObject *), NULL);
        if (yield_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6714;
            generator_heap->type_description_1 = "No";
            goto try_except_handler_2;
        }
        tmp_yield_result_1 = yield_return_value;
    }
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


        generator_heap->exception_lineno = 6714;
        generator_heap->type_description_1 = "No";
        goto try_except_handler_2;
    }
    goto loop_start_1;
    loop_end_1:;
    goto try_end_1;
    // Exception handler code:
    try_except_handler_2:;
    generator_heap->exception_keeper_type_1 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_1 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_1 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_1 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->tmp_iter_value_0);
    generator_heap->tmp_iter_value_0 = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_1;
    generator_heap->exception_value = generator_heap->exception_keeper_value_1;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_1;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_1;

    goto frame_exception_exit_1;
    // End of try:
    try_end_1:;

    Nuitka_Frame_MarkAsNotExecuting(generator->m_frame);

#if PYTHON_VERSION >= 0x300
    Py_CLEAR(EXC_TYPE_F(generator));
    Py_CLEAR(EXC_VALUE_F(generator));
    Py_CLEAR(EXC_TRACEBACK_F(generator));
#endif

    // Allow re-use of the frame again.
    Py_DECREF(generator->m_frame);
    goto frame_no_exception_1;

    frame_exception_exit_1:;

    // If it's not an exit exception, consider and create a traceback for it.
    if (!EXCEPTION_MATCH_GENERATOR(generator_heap->exception_type)) {
        if (generator_heap->exception_tb == NULL) {
            generator_heap->exception_tb = MAKE_TRACEBACK(generator->m_frame, generator_heap->exception_lineno);
        } else if (generator_heap->exception_tb->tb_frame != &generator->m_frame->m_frame) {
            generator_heap->exception_tb = ADD_TRACEBACK(generator_heap->exception_tb, generator->m_frame, generator_heap->exception_lineno);
        }

        Nuitka_Frame_AttachLocals(
            generator->m_frame,
            generator_heap->type_description_1,
            NULL,
            generator_heap->var_alias
        );


        // Release cached frame if used for exception.
        if (generator->m_frame == cache_m_frame) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_m_frame);
            cache_m_frame = NULL;
        }

        assertFrameObject(generator->m_frame);
    }

#if PYTHON_VERSION >= 0x300
    Py_CLEAR(EXC_TYPE_F(generator));
    Py_CLEAR(EXC_VALUE_F(generator));
    Py_CLEAR(EXC_TRACEBACK_F(generator));
#endif

    Py_DECREF(generator->m_frame);

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    goto try_end_2;
    // Exception handler code:
    try_except_handler_1:;
    generator_heap->exception_keeper_type_2 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_2 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_2 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_2 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->var_alias);
    generator_heap->var_alias = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_2;
    generator_heap->exception_value = generator_heap->exception_keeper_value_2;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_2;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_2;

    goto function_exception_exit;
    // End of try:
    try_end_2:;
    Py_XDECREF(generator_heap->tmp_iter_value_0);
    generator_heap->tmp_iter_value_0 = NULL;
    Py_XDECREF(generator_heap->var_alias);
    generator_heap->var_alias = NULL;


    return NULL;

    function_exception_exit:
    assert(generator_heap->exception_type);
    RESTORE_ERROR_OCCURRED(generator_heap->exception_type, generator_heap->exception_value, generator_heap->exception_tb);

    return NULL;

}

static PyObject *MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr(struct Nuitka_CellObject **closure) {
    return Nuitka_Generator_New(
        numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr_context,
        module_numpy$core$_add_newdocs,
        mod_consts[25],
#if PYTHON_VERSION >= 0x350
        mod_consts[26],
#endif
        codeobj_c220f471a98c826733824301ef8c9428,
        closure,
        1,
        sizeof(struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__1_genexpr_locals)
    );
}



struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr_locals {
    PyObject *var_alias_type;
    PyObject *var_alias;
    PyObject *var_doc;
    PyObject *tmp_iter_value_0;
    PyObject *tmp_tuple_unpack_1__element_1;
    PyObject *tmp_tuple_unpack_1__element_2;
    PyObject *tmp_tuple_unpack_1__element_3;
    PyObject *tmp_tuple_unpack_1__source_iter;
    char const *type_description_1;
    PyObject *exception_type;
    PyObject *exception_value;
    PyTracebackObject *exception_tb;
    int exception_lineno;
    PyObject *tmp_iterator_attempt;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    int exception_keeper_lineno_1;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    int exception_keeper_lineno_2;
    char yield_tmps[1024];
    PyObject *exception_keeper_type_3;
    PyObject *exception_keeper_value_3;
    PyTracebackObject *exception_keeper_tb_3;
    int exception_keeper_lineno_3;
    PyObject *exception_keeper_type_4;
    PyObject *exception_keeper_value_4;
    PyTracebackObject *exception_keeper_tb_4;
    int exception_keeper_lineno_4;
};

static PyObject *numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr_context(struct Nuitka_GeneratorObject *generator, PyObject *yield_return_value) {
    CHECK_OBJECT(generator);
    assert(Nuitka_Generator_Check((PyObject *)generator));
    CHECK_OBJECT_X(yield_return_value);

    // Heap access if used.
    struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr_locals *generator_heap = (struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr_locals *)generator->m_heap_storage;

    // Dispatch to yield based on return label index:
    switch(generator->m_yield_return_index) {
    case 1: goto yield_return_1;
    }

    // Local variable initialization
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_m_frame = NULL;
    generator_heap->var_alias_type = NULL;
    generator_heap->var_alias = NULL;
    generator_heap->var_doc = NULL;
    generator_heap->tmp_iter_value_0 = NULL;
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;
    generator_heap->tmp_tuple_unpack_1__element_3 = NULL;
    generator_heap->tmp_tuple_unpack_1__source_iter = NULL;
    generator_heap->type_description_1 = NULL;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    // Actual generator function body.
    // Tried code:
    if (isFrameUnusable(cache_m_frame)) {
        Py_XDECREF(cache_m_frame);

#if _DEBUG_REFCOUNTS
        if (cache_m_frame == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_m_frame = MAKE_FUNCTION_FRAME(codeobj_f88dc747b4ef6e9a81080b2f8f093fda, module_numpy$core$_add_newdocs, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    generator->m_frame = cache_m_frame;

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    Py_INCREF(generator->m_frame);
    assert(Py_REFCNT(generator->m_frame) == 2); // Frame stack

#if PYTHON_VERSION >= 0x340
    generator->m_frame->m_frame.f_gen = (PyObject *)generator;
#endif

    assert(generator->m_frame->m_frame.f_back == NULL);
    Py_CLEAR(generator->m_frame->m_frame.f_back);

    generator->m_frame->m_frame.f_back = PyThreadState_GET()->frame;
    Py_INCREF(generator->m_frame->m_frame.f_back);

    PyThreadState_GET()->frame = &generator->m_frame->m_frame;
    Py_INCREF(generator->m_frame);

    Nuitka_Frame_MarkAsExecuting(generator->m_frame);

#if PYTHON_VERSION >= 0x300
    // Accept currently existing exception as the one to publish again when we
    // yield or yield from.
    {
        PyThreadState *thread_state = PyThreadState_GET();

        EXC_TYPE_F(generator) = EXC_TYPE(thread_state);
        if (EXC_TYPE_F(generator) == Py_None) EXC_TYPE_F(generator) = NULL;
        Py_XINCREF(EXC_TYPE_F(generator));
        EXC_VALUE_F(generator) = EXC_VALUE(thread_state);
        Py_XINCREF(EXC_VALUE_F(generator));
        EXC_TRACEBACK_F(generator) = EXC_TRACEBACK(thread_state);
        Py_XINCREF(EXC_TRACEBACK_F(generator));
    }

#endif

    // Framed code:
    // Tried code:
    loop_start_1:;
    {
        PyObject *tmp_next_source_1;
        PyObject *tmp_assign_source_1;
        CHECK_OBJECT(Nuitka_Cell_GET(generator->m_closure[0]));
        tmp_next_source_1 = Nuitka_Cell_GET(generator->m_closure[0]);
        tmp_assign_source_1 = ITERATOR_NEXT(tmp_next_source_1);
        if (tmp_assign_source_1 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_1;
            } else {

                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
                generator_heap->type_description_1 = "Noooc";
                generator_heap->exception_lineno = 6715;
                goto try_except_handler_2;
            }
        }

        {
            PyObject *old = generator_heap->tmp_iter_value_0;
            generator_heap->tmp_iter_value_0 = tmp_assign_source_1;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_2;
        PyObject *tmp_iter_arg_1;
        CHECK_OBJECT(generator_heap->tmp_iter_value_0);
        tmp_iter_arg_1 = generator_heap->tmp_iter_value_0;
        tmp_assign_source_2 = MAKE_ITERATOR(tmp_iter_arg_1);
        if (tmp_assign_source_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6715;
            generator_heap->type_description_1 = "Noooc";
            goto try_except_handler_3;
        }
        {
            PyObject *old = generator_heap->tmp_tuple_unpack_1__source_iter;
            generator_heap->tmp_tuple_unpack_1__source_iter = tmp_assign_source_2;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_3;
        PyObject *tmp_unpack_1;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__source_iter);
        tmp_unpack_1 = generator_heap->tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_3 = UNPACK_NEXT(tmp_unpack_1, 0, 3);
        if (tmp_assign_source_3 == NULL) {
            if (!ERROR_OCCURRED()) {
                generator_heap->exception_type = PyExc_StopIteration;
                Py_INCREF(generator_heap->exception_type);
                generator_heap->exception_value = NULL;
                generator_heap->exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            }


            generator_heap->type_description_1 = "Noooc";
            generator_heap->exception_lineno = 6715;
            goto try_except_handler_4;
        }
        {
            PyObject *old = generator_heap->tmp_tuple_unpack_1__element_1;
            generator_heap->tmp_tuple_unpack_1__element_1 = tmp_assign_source_3;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_4;
        PyObject *tmp_unpack_2;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__source_iter);
        tmp_unpack_2 = generator_heap->tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_4 = UNPACK_NEXT(tmp_unpack_2, 1, 3);
        if (tmp_assign_source_4 == NULL) {
            if (!ERROR_OCCURRED()) {
                generator_heap->exception_type = PyExc_StopIteration;
                Py_INCREF(generator_heap->exception_type);
                generator_heap->exception_value = NULL;
                generator_heap->exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            }


            generator_heap->type_description_1 = "Noooc";
            generator_heap->exception_lineno = 6715;
            goto try_except_handler_4;
        }
        {
            PyObject *old = generator_heap->tmp_tuple_unpack_1__element_2;
            generator_heap->tmp_tuple_unpack_1__element_2 = tmp_assign_source_4;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_unpack_3;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__source_iter);
        tmp_unpack_3 = generator_heap->tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_5 = UNPACK_NEXT(tmp_unpack_3, 2, 3);
        if (tmp_assign_source_5 == NULL) {
            if (!ERROR_OCCURRED()) {
                generator_heap->exception_type = PyExc_StopIteration;
                Py_INCREF(generator_heap->exception_type);
                generator_heap->exception_value = NULL;
                generator_heap->exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            }


            generator_heap->type_description_1 = "Noooc";
            generator_heap->exception_lineno = 6715;
            goto try_except_handler_4;
        }
        {
            PyObject *old = generator_heap->tmp_tuple_unpack_1__element_3;
            generator_heap->tmp_tuple_unpack_1__element_3 = tmp_assign_source_5;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_iterator_name_1;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__source_iter);
        tmp_iterator_name_1 = generator_heap->tmp_tuple_unpack_1__source_iter;
        // Check if iterator has left-over elements.
        CHECK_OBJECT(tmp_iterator_name_1); assert(HAS_ITERNEXT(tmp_iterator_name_1));

        generator_heap->tmp_iterator_attempt = (*Py_TYPE(tmp_iterator_name_1)->tp_iternext)(tmp_iterator_name_1);

        if (likely(generator_heap->tmp_iterator_attempt == NULL)) {
            PyObject *error = GET_ERROR_OCCURRED();

            if (error != NULL) {
                if (EXCEPTION_MATCH_BOOL_SINGLE(error, PyExc_StopIteration)) {
                    CLEAR_ERROR_OCCURRED();
                } else {
                    FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);

                    generator_heap->type_description_1 = "Noooc";
                    generator_heap->exception_lineno = 6715;
                    goto try_except_handler_4;
                }
            }
        } else {
            Py_DECREF(generator_heap->tmp_iterator_attempt);

            generator_heap->exception_type = PyExc_ValueError;
            Py_INCREF(PyExc_ValueError);
            generator_heap->exception_value = mod_consts[27];
            Py_INCREF(generator_heap->exception_value);
            generator_heap->exception_tb = NULL;

            generator_heap->type_description_1 = "Noooc";
            generator_heap->exception_lineno = 6715;
            goto try_except_handler_4;
        }
    }
    goto try_end_1;
    // Exception handler code:
    try_except_handler_4:;
    generator_heap->exception_keeper_type_1 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_1 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_1 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_1 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__source_iter);
    generator_heap->tmp_tuple_unpack_1__source_iter = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_1;
    generator_heap->exception_value = generator_heap->exception_keeper_value_1;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_1;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_1;

    goto try_except_handler_3;
    // End of try:
    try_end_1:;
    goto try_end_2;
    // Exception handler code:
    try_except_handler_3:;
    generator_heap->exception_keeper_type_2 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_2 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_2 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_2 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_1);
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_2);
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_3);
    generator_heap->tmp_tuple_unpack_1__element_3 = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_2;
    generator_heap->exception_value = generator_heap->exception_keeper_value_2;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_2;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_2;

    goto try_except_handler_2;
    // End of try:
    try_end_2:;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__source_iter);
    generator_heap->tmp_tuple_unpack_1__source_iter = NULL;
    {
        PyObject *tmp_assign_source_6;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__element_1);
        tmp_assign_source_6 = generator_heap->tmp_tuple_unpack_1__element_1;
        {
            PyObject *old = generator_heap->var_alias_type;
            generator_heap->var_alias_type = tmp_assign_source_6;
            Py_INCREF(generator_heap->var_alias_type);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_1);
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;

    {
        PyObject *tmp_assign_source_7;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__element_2);
        tmp_assign_source_7 = generator_heap->tmp_tuple_unpack_1__element_2;
        {
            PyObject *old = generator_heap->var_alias;
            generator_heap->var_alias = tmp_assign_source_7;
            Py_INCREF(generator_heap->var_alias);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_2);
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;

    {
        PyObject *tmp_assign_source_8;
        CHECK_OBJECT(generator_heap->tmp_tuple_unpack_1__element_3);
        tmp_assign_source_8 = generator_heap->tmp_tuple_unpack_1__element_3;
        {
            PyObject *old = generator_heap->var_doc;
            generator_heap->var_doc = tmp_assign_source_8;
            Py_INCREF(generator_heap->var_doc);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_3);
    generator_heap->tmp_tuple_unpack_1__element_3 = NULL;

    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_1);
    generator_heap->tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_2);
    generator_heap->tmp_tuple_unpack_1__element_2 = NULL;
    Py_XDECREF(generator_heap->tmp_tuple_unpack_1__element_3);
    generator_heap->tmp_tuple_unpack_1__element_3 = NULL;
    {
        bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        CHECK_OBJECT(generator_heap->var_alias_type);
        tmp_compexpr_left_1 = generator_heap->var_alias_type;
        if (Nuitka_Cell_GET(generator->m_closure[1]) == NULL) {

            FORMAT_UNBOUND_CLOSURE_ERROR(&generator_heap->exception_type, &generator_heap->exception_value, mod_consts[8]);
            generator_heap->exception_tb = NULL;
            NORMALIZE_EXCEPTION(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            CHAIN_EXCEPTION(generator_heap->exception_value);

            generator_heap->exception_lineno = 6716;
            generator_heap->type_description_1 = "Noooc";
            goto try_except_handler_2;
        }

        tmp_compexpr_right_1 = Nuitka_Cell_GET(generator->m_closure[1]);
        tmp_condition_result_1 = (tmp_compexpr_left_1 == tmp_compexpr_right_1) ? true : false;
        if (tmp_condition_result_1 != false) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    {
        PyObject *tmp_expression_name_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        NUITKA_MAY_BE_UNUSED PyObject *tmp_yield_result_1;
        tmp_called_instance_1 = mod_consts[28];
        CHECK_OBJECT(generator_heap->var_alias);
        tmp_args_element_name_1 = generator_heap->var_alias;
        CHECK_OBJECT(generator_heap->var_doc);
        tmp_args_element_name_2 = generator_heap->var_doc;
        generator->m_frame->m_frame.f_lineno = 6715;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_expression_name_1 = CALL_METHOD_WITH_ARGS2(
                tmp_called_instance_1,
                mod_consts[12],
                call_args
            );
        }

        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6715;
            generator_heap->type_description_1 = "Noooc";
            goto try_except_handler_2;
        }
        Nuitka_PreserveHeap(generator_heap->yield_tmps, &tmp_called_instance_1, sizeof(PyObject *), &tmp_args_element_name_1, sizeof(PyObject *), &tmp_args_element_name_2, sizeof(PyObject *), NULL);
        generator->m_yield_return_index = 1;
        return tmp_expression_name_1;
        yield_return_1:
        Nuitka_RestoreHeap(generator_heap->yield_tmps, &tmp_called_instance_1, sizeof(PyObject *), &tmp_args_element_name_1, sizeof(PyObject *), &tmp_args_element_name_2, sizeof(PyObject *), NULL);
        if (yield_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 6715;
            generator_heap->type_description_1 = "Noooc";
            goto try_except_handler_2;
        }
        tmp_yield_result_1 = yield_return_value;
    }
    branch_no_1:;
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


        generator_heap->exception_lineno = 6715;
        generator_heap->type_description_1 = "Noooc";
        goto try_except_handler_2;
    }
    goto loop_start_1;
    loop_end_1:;
    goto try_end_3;
    // Exception handler code:
    try_except_handler_2:;
    generator_heap->exception_keeper_type_3 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_3 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_3 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_3 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->tmp_iter_value_0);
    generator_heap->tmp_iter_value_0 = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_3;
    generator_heap->exception_value = generator_heap->exception_keeper_value_3;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_3;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_3;

    goto frame_exception_exit_1;
    // End of try:
    try_end_3:;

    Nuitka_Frame_MarkAsNotExecuting(generator->m_frame);

#if PYTHON_VERSION >= 0x300
    Py_CLEAR(EXC_TYPE_F(generator));
    Py_CLEAR(EXC_VALUE_F(generator));
    Py_CLEAR(EXC_TRACEBACK_F(generator));
#endif

    // Allow re-use of the frame again.
    Py_DECREF(generator->m_frame);
    goto frame_no_exception_1;

    frame_exception_exit_1:;

    // If it's not an exit exception, consider and create a traceback for it.
    if (!EXCEPTION_MATCH_GENERATOR(generator_heap->exception_type)) {
        if (generator_heap->exception_tb == NULL) {
            generator_heap->exception_tb = MAKE_TRACEBACK(generator->m_frame, generator_heap->exception_lineno);
        } else if (generator_heap->exception_tb->tb_frame != &generator->m_frame->m_frame) {
            generator_heap->exception_tb = ADD_TRACEBACK(generator_heap->exception_tb, generator->m_frame, generator_heap->exception_lineno);
        }

        Nuitka_Frame_AttachLocals(
            generator->m_frame,
            generator_heap->type_description_1,
            NULL,
            generator_heap->var_alias_type,
            generator_heap->var_alias,
            generator_heap->var_doc,
            generator->m_closure[1]
        );


        // Release cached frame if used for exception.
        if (generator->m_frame == cache_m_frame) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_m_frame);
            cache_m_frame = NULL;
        }

        assertFrameObject(generator->m_frame);
    }

#if PYTHON_VERSION >= 0x300
    Py_CLEAR(EXC_TYPE_F(generator));
    Py_CLEAR(EXC_VALUE_F(generator));
    Py_CLEAR(EXC_TRACEBACK_F(generator));
#endif

    Py_DECREF(generator->m_frame);

    // Return the error.
    goto try_except_handler_1;

    frame_no_exception_1:;
    goto try_end_4;
    // Exception handler code:
    try_except_handler_1:;
    generator_heap->exception_keeper_type_4 = generator_heap->exception_type;
    generator_heap->exception_keeper_value_4 = generator_heap->exception_value;
    generator_heap->exception_keeper_tb_4 = generator_heap->exception_tb;
    generator_heap->exception_keeper_lineno_4 = generator_heap->exception_lineno;
    generator_heap->exception_type = NULL;
    generator_heap->exception_value = NULL;
    generator_heap->exception_tb = NULL;
    generator_heap->exception_lineno = 0;

    Py_XDECREF(generator_heap->var_alias_type);
    generator_heap->var_alias_type = NULL;
    Py_XDECREF(generator_heap->var_alias);
    generator_heap->var_alias = NULL;
    Py_XDECREF(generator_heap->var_doc);
    generator_heap->var_doc = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_4;
    generator_heap->exception_value = generator_heap->exception_keeper_value_4;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_4;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_4;

    goto function_exception_exit;
    // End of try:
    try_end_4:;
    Py_XDECREF(generator_heap->tmp_iter_value_0);
    generator_heap->tmp_iter_value_0 = NULL;
    Py_XDECREF(generator_heap->var_alias_type);
    generator_heap->var_alias_type = NULL;
    Py_XDECREF(generator_heap->var_alias);
    generator_heap->var_alias = NULL;
    Py_XDECREF(generator_heap->var_doc);
    generator_heap->var_doc = NULL;


    return NULL;

    function_exception_exit:
    assert(generator_heap->exception_type);
    RESTORE_ERROR_OCCURRED(generator_heap->exception_type, generator_heap->exception_value, generator_heap->exception_tb);

    return NULL;

}

static PyObject *MAKE_GENERATOR_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr(struct Nuitka_CellObject **closure) {
    return Nuitka_Generator_New(
        numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr_context,
        module_numpy$core$_add_newdocs,
        mod_consts[25],
#if PYTHON_VERSION >= 0x350
        mod_consts[26],
#endif
        codeobj_f88dc747b4ef6e9a81080b2f8f093fda,
        closure,
        2,
        sizeof(struct numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type$$$genexpr__2_genexpr_locals)
    );
}



static PyObject *MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases,
        mod_consts[295],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_8c22d20285be3f462dc517c5631a011a,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$core$_add_newdocs,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen(struct Nuitka_CellObject **closure) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen,
        mod_consts[0],
#if PYTHON_VERSION >= 0x300
        mod_consts[1],
#endif
        codeobj_6bb5e4f9f1973cbee91a038767b6b70f,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$core$_add_newdocs,
        NULL,
        closure,
        1
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type,
        mod_consts[297],
#if PYTHON_VERSION >= 0x300
        NULL,
#endif
        codeobj_e2f83e1870731c26347d011a360b3353,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_numpy$core$_add_newdocs,
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

function_impl_code functable_numpy$core$_add_newdocs[] = {
    impl_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases$$$function__1_type_aliases_gen,
    impl_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases,
    impl_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type,
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

    function_impl_code *current = functable_numpy$core$_add_newdocs;
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

    if (offset > sizeof(functable_numpy$core$_add_newdocs) || offset < 0) {
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
        functable_numpy$core$_add_newdocs[offset],
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
        module_numpy$core$_add_newdocs,
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
PyObject *modulecode_numpy$core$_add_newdocs(PyObject *module, struct Nuitka_MetaPathBasedLoaderEntry const *module_entry) {
    module_numpy$core$_add_newdocs = module;

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
    PRINT_STRING("numpy.core._add_newdocs: Calling setupMetaPathBasedLoader().\n");
#endif
    setupMetaPathBasedLoader();

#if PYTHON_VERSION >= 0x300
    patchInspectModule();
#endif

#endif

    /* The constants only used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("numpy.core._add_newdocs: Calling createModuleConstants().\n");
#endif
    createModuleConstants();

    /* The code objects used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("numpy.core._add_newdocs: Calling createModuleCodeObjects().\n");
#endif
    createModuleCodeObjects();

    // PRINT_STRING("in initnumpy$core$_add_newdocs\n");

    // Create the module object first. There are no methods initially, all are
    // added dynamically in actual code only.  Also no "__doc__" is initially
    // set at this time, as it could not contain NUL characters this way, they
    // are instead set in early module code.  No "self" for modules, we have no
    // use for it.

    moduledict_numpy$core$_add_newdocs = MODULE_DICT(module_numpy$core$_add_newdocs);

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
    registerDillPluginTables(module_entry->name, &_method_def_reduce_compiled_function, &_method_def_create_compiled_function);
#endif

    // Set "__compiled__" to what version information we have.
    UPDATE_STRING_DICT0(
        moduledict_numpy$core$_add_newdocs,
        (Nuitka_StringObject *)const_str_plain___compiled__,
        Nuitka_dunder_compiled_value
    );

    // Update "__package__" value to what it ought to be.
    {
#if 0
        UPDATE_STRING_DICT0(
            moduledict_numpy$core$_add_newdocs,
            (Nuitka_StringObject *)const_str_plain___package__,
            const_str_empty
        );
#elif 0
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___name__);

        UPDATE_STRING_DICT0(
            moduledict_numpy$core$_add_newdocs,
            (Nuitka_StringObject *)const_str_plain___package__,
            module_name
        );
#else

#if PYTHON_VERSION < 0x300
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___name__);
        char const *module_name_cstr = PyString_AS_STRING(module_name);

        char const *last_dot = strrchr(module_name_cstr, '.');

        if (last_dot != NULL) {
            UPDATE_STRING_DICT1(
                moduledict_numpy$core$_add_newdocs,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyString_FromStringAndSize(module_name_cstr, last_dot - module_name_cstr)
            );
        }
#else
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___name__);
        Py_ssize_t dot_index = PyUnicode_Find(module_name, const_str_dot, 0, PyUnicode_GetLength(module_name), -1);

        if (dot_index != -1) {
            UPDATE_STRING_DICT1(
                moduledict_numpy$core$_add_newdocs,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyUnicode_Substring(module_name, 0, dot_index)
            );
        }
#endif
#endif
    }

    CHECK_OBJECT(module_numpy$core$_add_newdocs);

    // For deep importing of a module we need to have "__builtins__", so we set
    // it ourselves in the same way than CPython does. Note: This must be done
    // before the frame object is allocated, or else it may fail.

    if (GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___builtins__) == NULL) {
        PyObject *value = (PyObject *)builtin_module;

        // Check if main module, not a dict then but the module itself.
#if !defined(_NUITKA_EXE) || !0
        value = PyModule_GetDict(value);
#endif

        UPDATE_STRING_DICT0(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___builtins__, value);
    }

#if PYTHON_VERSION >= 0x300
    UPDATE_STRING_DICT0(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___loader__, (PyObject *)&Nuitka_Loader_Type);
#endif

#if PYTHON_VERSION >= 0x340
// Set the "__spec__" value

#if 0
    // Main modules just get "None" as spec.
    UPDATE_STRING_DICT0(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___spec__, Py_None);
#else
    // Other modules get a "ModuleSpec" from the standard mechanism.
    {
        PyObject *bootstrap_module = getImportLibBootstrapModule();
        CHECK_OBJECT(bootstrap_module);

        PyObject *_spec_from_module = PyObject_GetAttrString(bootstrap_module, "_spec_from_module");
        CHECK_OBJECT(_spec_from_module);

        PyObject *spec_value = CALL_FUNCTION_WITH_SINGLE_ARG(_spec_from_module, module_numpy$core$_add_newdocs);
        Py_DECREF(_spec_from_module);

        // We can assume this to never fail, or else we are in trouble anyway.
        // CHECK_OBJECT(spec_value);

        if (spec_value == NULL) {
            PyErr_PrintEx(0);
            abort();
        }

// Mark the execution in the "__spec__" value.
        SET_ATTRIBUTE(spec_value, const_str_plain__initializing, Py_True);

        UPDATE_STRING_DICT1(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)const_str_plain___spec__, spec_value);
    }
#endif
#endif

    // Temp variables if any
    PyObject *tmp_for_loop_1__for_iterator = NULL;
    PyObject *tmp_for_loop_1__iter_value = NULL;
    struct Nuitka_FrameObject *frame_837e186af4984df94dc0b36608112861;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    bool tmp_result;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    int tmp_res;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Module code.
    {
        PyObject *tmp_assign_source_1;
        tmp_assign_source_1 = mod_consts[29];
        UPDATE_STRING_DICT0(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[30], tmp_assign_source_1);
    }
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = mod_consts[31];
        UPDATE_STRING_DICT0(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[32], tmp_assign_source_2);
    }
    // Frame without reuse.
    frame_837e186af4984df94dc0b36608112861 = MAKE_MODULE_FRAME(codeobj_837e186af4984df94dc0b36608112861, module_numpy$core$_add_newdocs);

    // Push the new frame as the currently active one, and we should be exclusively
    // owning it.
    pushFrameStack(frame_837e186af4984df94dc0b36608112861);
    assert(Py_REFCNT(frame_837e186af4984df94dc0b36608112861) == 2);

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_assattr_name_1 = mod_consts[31];
        tmp_assattr_target_1 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[33]);

        if (unlikely(tmp_assattr_target_1 == NULL)) {
            tmp_assattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[33]);
        }

        assert(!(tmp_assattr_target_1 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[34], tmp_assattr_name_1);
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
        tmp_assattr_target_2 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[33]);

        if (unlikely(tmp_assattr_target_2 == NULL)) {
            tmp_assattr_target_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[33]);
        }

        assert(!(tmp_assattr_target_2 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[35], tmp_assattr_name_2);
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
        UPDATE_STRING_DICT0(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[36], tmp_assign_source_3);
    }
    {
        PyObject *tmp_assign_source_4;
        PyObject *tmp_import_name_from_1;
        PyObject *tmp_name_name_1;
        PyObject *tmp_globals_arg_name_1;
        PyObject *tmp_locals_arg_name_1;
        PyObject *tmp_fromlist_name_1;
        PyObject *tmp_level_name_1;
        tmp_name_name_1 = mod_consts[37];
        tmp_globals_arg_name_1 = (PyObject *)moduledict_numpy$core$_add_newdocs;
        tmp_locals_arg_name_1 = Py_None;
        tmp_fromlist_name_1 = mod_consts[38];
        tmp_level_name_1 = mod_consts[39];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 12;
        tmp_import_name_from_1 = IMPORT_MODULE5(tmp_name_name_1, tmp_globals_arg_name_1, tmp_locals_arg_name_1, tmp_fromlist_name_1, tmp_level_name_1);
        if (tmp_import_name_from_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 12;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_1)) {
            tmp_assign_source_4 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_1,
                (PyObject *)moduledict_numpy$core$_add_newdocs,
                mod_consts[40],
                mod_consts[39]
            );
        } else {
            tmp_assign_source_4 = IMPORT_NAME(tmp_import_name_from_1, mod_consts[40]);
        }

        Py_DECREF(tmp_import_name_from_1);
        if (tmp_assign_source_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 12;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[4], tmp_assign_source_4);
    }
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_import_name_from_2;
        PyObject *tmp_name_name_2;
        PyObject *tmp_globals_arg_name_2;
        PyObject *tmp_locals_arg_name_2;
        PyObject *tmp_fromlist_name_2;
        PyObject *tmp_level_name_2;
        tmp_name_name_2 = mod_consts[37];
        tmp_globals_arg_name_2 = (PyObject *)moduledict_numpy$core$_add_newdocs;
        tmp_locals_arg_name_2 = Py_None;
        tmp_fromlist_name_2 = mod_consts[41];
        tmp_level_name_2 = mod_consts[39];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 13;
        tmp_import_name_from_2 = IMPORT_MODULE5(tmp_name_name_2, tmp_globals_arg_name_2, tmp_locals_arg_name_2, tmp_fromlist_name_2, tmp_level_name_2);
        if (tmp_import_name_from_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 13;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_2)) {
            tmp_assign_source_5 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_2,
                (PyObject *)moduledict_numpy$core$_add_newdocs,
                mod_consts[6],
                mod_consts[39]
            );
        } else {
            tmp_assign_source_5 = IMPORT_NAME(tmp_import_name_from_2, mod_consts[6]);
        }

        Py_DECREF(tmp_import_name_from_2);
        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 13;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[6], tmp_assign_source_5);
    }
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_import_name_from_3;
        PyObject *tmp_name_name_3;
        PyObject *tmp_globals_arg_name_3;
        PyObject *tmp_locals_arg_name_3;
        PyObject *tmp_fromlist_name_3;
        PyObject *tmp_level_name_3;
        tmp_name_name_3 = mod_consts[42];
        tmp_globals_arg_name_3 = (PyObject *)moduledict_numpy$core$_add_newdocs;
        tmp_locals_arg_name_3 = Py_None;
        tmp_fromlist_name_3 = mod_consts[43];
        tmp_level_name_3 = mod_consts[39];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 14;
        tmp_import_name_from_3 = IMPORT_MODULE5(tmp_name_name_3, tmp_globals_arg_name_3, tmp_locals_arg_name_3, tmp_fromlist_name_3, tmp_level_name_3);
        if (tmp_import_name_from_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_3)) {
            tmp_assign_source_6 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_3,
                (PyObject *)moduledict_numpy$core$_add_newdocs,
                mod_consts[21],
                mod_consts[39]
            );
        } else {
            tmp_assign_source_6 = IMPORT_NAME(tmp_import_name_from_3, mod_consts[21]);
        }

        Py_DECREF(tmp_import_name_from_3);
        if (tmp_assign_source_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 14;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21], tmp_assign_source_6);
    }
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_call_result_1;
        tmp_called_name_1 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_1 == NULL)) {
            tmp_called_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        assert(!(tmp_called_name_1 == NULL));
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 24;
        tmp_call_result_1 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_1, &PyTuple_GET_ITEM(mod_consts[44], 0));

        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 24;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_called_name_2;
        PyObject *tmp_call_result_2;
        tmp_called_name_2 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_2 == NULL)) {
            tmp_called_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 69;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 69;
        tmp_call_result_2 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_2, &PyTuple_GET_ITEM(mod_consts[45], 0));

        if (tmp_call_result_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 69;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_2);
    }
    {
        PyObject *tmp_called_name_3;
        PyObject *tmp_call_result_3;
        tmp_called_name_3 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_3 == NULL)) {
            tmp_called_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 84;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 84;
        tmp_call_result_3 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_3, &PyTuple_GET_ITEM(mod_consts[46], 0));

        if (tmp_call_result_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 84;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_3);
    }
    {
        PyObject *tmp_called_name_4;
        PyObject *tmp_call_result_4;
        tmp_called_name_4 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_4 == NULL)) {
            tmp_called_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 103;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 103;
        tmp_call_result_4 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_4, &PyTuple_GET_ITEM(mod_consts[47], 0));

        if (tmp_call_result_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 103;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_4);
    }
    {
        PyObject *tmp_called_name_5;
        PyObject *tmp_call_result_5;
        tmp_called_name_5 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_5 == NULL)) {
            tmp_called_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 122;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 122;
        tmp_call_result_5 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_5, &PyTuple_GET_ITEM(mod_consts[48], 0));

        if (tmp_call_result_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 122;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_5);
    }
    {
        PyObject *tmp_called_name_6;
        PyObject *tmp_call_result_6;
        tmp_called_name_6 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_6 == NULL)) {
            tmp_called_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 128;
        tmp_call_result_6 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_6, &PyTuple_GET_ITEM(mod_consts[49], 0));

        if (tmp_call_result_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 128;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_6);
    }
    {
        PyObject *tmp_called_name_7;
        PyObject *tmp_call_result_7;
        tmp_called_name_7 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_7 == NULL)) {
            tmp_called_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 153;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 153;
        tmp_call_result_7 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_7, &PyTuple_GET_ITEM(mod_consts[50], 0));

        if (tmp_call_result_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 153;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_7);
    }
    {
        PyObject *tmp_called_name_8;
        PyObject *tmp_call_result_8;
        tmp_called_name_8 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_8 == NULL)) {
            tmp_called_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 421;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 421;
        tmp_call_result_8 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_8, &PyTuple_GET_ITEM(mod_consts[51], 0));

        if (tmp_call_result_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 421;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_8);
    }
    {
        PyObject *tmp_called_name_9;
        PyObject *tmp_call_result_9;
        tmp_called_name_9 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_9 == NULL)) {
            tmp_called_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 440;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 440;
        tmp_call_result_9 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_9, &PyTuple_GET_ITEM(mod_consts[52], 0));

        if (tmp_call_result_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 440;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_9);
    }
    {
        PyObject *tmp_called_name_10;
        PyObject *tmp_call_result_10;
        tmp_called_name_10 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_10 == NULL)) {
            tmp_called_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 447;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 447;
        tmp_call_result_10 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_10, &PyTuple_GET_ITEM(mod_consts[53], 0));

        if (tmp_call_result_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 447;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_10);
    }
    {
        PyObject *tmp_called_name_11;
        PyObject *tmp_call_result_11;
        tmp_called_name_11 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_11 == NULL)) {
            tmp_called_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 455;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 455;
        tmp_call_result_11 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_11, &PyTuple_GET_ITEM(mod_consts[54], 0));

        if (tmp_call_result_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 455;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_11);
    }
    {
        PyObject *tmp_called_name_12;
        PyObject *tmp_call_result_12;
        tmp_called_name_12 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_12 == NULL)) {
            tmp_called_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 465;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 465;
        tmp_call_result_12 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_12, &PyTuple_GET_ITEM(mod_consts[55], 0));

        if (tmp_call_result_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 465;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_12);
    }
    {
        PyObject *tmp_called_name_13;
        PyObject *tmp_call_result_13;
        tmp_called_name_13 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_13 == NULL)) {
            tmp_called_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 480;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 480;
        tmp_call_result_13 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_13, &PyTuple_GET_ITEM(mod_consts[56], 0));

        if (tmp_call_result_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 480;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_13);
    }
    {
        PyObject *tmp_called_name_14;
        PyObject *tmp_call_result_14;
        tmp_called_name_14 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_14 == NULL)) {
            tmp_called_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 489;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 489;
        tmp_call_result_14 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_14, &PyTuple_GET_ITEM(mod_consts[57], 0));

        if (tmp_call_result_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 489;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_14);
    }
    {
        PyObject *tmp_called_name_15;
        PyObject *tmp_call_result_15;
        tmp_called_name_15 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_15 == NULL)) {
            tmp_called_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 498;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 498;
        tmp_call_result_15 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_15, &PyTuple_GET_ITEM(mod_consts[58], 0));

        if (tmp_call_result_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 498;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_15);
    }
    {
        PyObject *tmp_called_name_16;
        PyObject *tmp_call_result_16;
        tmp_called_name_16 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_16 == NULL)) {
            tmp_called_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 506;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 506;
        tmp_call_result_16 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_16, &PyTuple_GET_ITEM(mod_consts[59], 0));

        if (tmp_call_result_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 506;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_16);
    }
    {
        PyObject *tmp_called_name_17;
        PyObject *tmp_call_result_17;
        tmp_called_name_17 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_17 == NULL)) {
            tmp_called_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 566;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 566;
        tmp_call_result_17 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_17, &PyTuple_GET_ITEM(mod_consts[60], 0));

        if (tmp_call_result_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 566;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_17);
    }
    {
        PyObject *tmp_called_name_18;
        PyObject *tmp_call_result_18;
        tmp_called_name_18 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_18 == NULL)) {
            tmp_called_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 588;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 588;
        tmp_call_result_18 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_18, &PyTuple_GET_ITEM(mod_consts[61], 0));

        if (tmp_call_result_18 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 588;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_18);
    }
    {
        PyObject *tmp_called_name_19;
        PyObject *tmp_call_result_19;
        tmp_called_name_19 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_19 == NULL)) {
            tmp_called_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 637;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 637;
        tmp_call_result_19 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_19, &PyTuple_GET_ITEM(mod_consts[62], 0));

        if (tmp_call_result_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 637;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_19);
    }
    {
        PyObject *tmp_called_name_20;
        PyObject *tmp_call_result_20;
        tmp_called_name_20 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_20 == NULL)) {
            tmp_called_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 655;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 655;
        tmp_call_result_20 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_20, &PyTuple_GET_ITEM(mod_consts[63], 0));

        if (tmp_call_result_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 655;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_20);
    }
    {
        PyObject *tmp_called_name_21;
        PyObject *tmp_call_result_21;
        tmp_called_name_21 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_21 == NULL)) {
            tmp_called_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 677;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 677;
        tmp_call_result_21 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_21, &PyTuple_GET_ITEM(mod_consts[64], 0));

        if (tmp_call_result_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 677;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_21);
    }
    {
        PyObject *tmp_called_name_22;
        PyObject *tmp_call_result_22;
        tmp_called_name_22 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_22 == NULL)) {
            tmp_called_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 693;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 693;
        tmp_call_result_22 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_22, &PyTuple_GET_ITEM(mod_consts[65], 0));

        if (tmp_call_result_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 693;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_22);
    }
    {
        PyObject *tmp_called_name_23;
        PyObject *tmp_call_result_23;
        tmp_called_name_23 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_23 == NULL)) {
            tmp_called_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 708;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 708;
        tmp_call_result_23 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_23, &PyTuple_GET_ITEM(mod_consts[66], 0));

        if (tmp_call_result_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 708;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_23);
    }
    {
        PyObject *tmp_called_name_24;
        PyObject *tmp_call_result_24;
        tmp_called_name_24 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_24 == NULL)) {
            tmp_called_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 722;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 722;
        tmp_call_result_24 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_24, &PyTuple_GET_ITEM(mod_consts[67], 0));

        if (tmp_call_result_24 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 722;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_24);
    }
    {
        PyObject *tmp_called_name_25;
        PyObject *tmp_call_result_25;
        tmp_called_name_25 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_25 == NULL)) {
            tmp_called_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 736;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 736;
        tmp_call_result_25 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_25, &PyTuple_GET_ITEM(mod_consts[68], 0));

        if (tmp_call_result_25 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 736;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_25);
    }
    {
        PyObject *tmp_called_name_26;
        PyObject *tmp_call_result_26;
        tmp_called_name_26 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_26 == NULL)) {
            tmp_called_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 750;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 750;
        tmp_call_result_26 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_26, &PyTuple_GET_ITEM(mod_consts[69], 0));

        if (tmp_call_result_26 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 750;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_26);
    }
    {
        PyObject *tmp_called_name_27;
        PyObject *tmp_call_result_27;
        tmp_called_name_27 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_27 == NULL)) {
            tmp_called_name_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 787;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 787;
        tmp_call_result_27 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_27, &PyTuple_GET_ITEM(mod_consts[70], 0));

        if (tmp_call_result_27 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 787;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_27);
    }
    {
        PyObject *tmp_called_name_28;
        PyObject *tmp_call_result_28;
        tmp_called_name_28 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_28 == NULL)) {
            tmp_called_name_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 900;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 900;
        tmp_call_result_28 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_28, &PyTuple_GET_ITEM(mod_consts[71], 0));

        if (tmp_call_result_28 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 900;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_28);
    }
    {
        PyObject *tmp_called_name_29;
        PyObject *tmp_call_result_29;
        tmp_called_name_29 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_29 == NULL)) {
            tmp_called_name_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 951;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 951;
        tmp_call_result_29 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_29, &PyTuple_GET_ITEM(mod_consts[72], 0));

        if (tmp_call_result_29 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 951;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_29);
    }
    {
        PyObject *tmp_called_name_30;
        PyObject *tmp_call_result_30;
        tmp_called_name_30 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_30 == NULL)) {
            tmp_called_name_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 965;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 965;
        tmp_call_result_30 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_30, &PyTuple_GET_ITEM(mod_consts[73], 0));

        if (tmp_call_result_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 965;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_30);
    }
    {
        PyObject *tmp_called_name_31;
        PyObject *tmp_call_result_31;
        tmp_called_name_31 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_31 == NULL)) {
            tmp_called_name_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1018;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1018;
        tmp_call_result_31 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_31, &PyTuple_GET_ITEM(mod_consts[74], 0));

        if (tmp_call_result_31 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1018;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_31);
    }
    {
        PyObject *tmp_called_name_32;
        PyObject *tmp_call_result_32;
        tmp_called_name_32 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_32 == NULL)) {
            tmp_called_name_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1026;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1026;
        tmp_call_result_32 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_32, &PyTuple_GET_ITEM(mod_consts[75], 0));

        if (tmp_call_result_32 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1026;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_32);
    }
    {
        PyObject *tmp_called_name_33;
        PyObject *tmp_call_result_33;
        tmp_called_name_33 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_33 == NULL)) {
            tmp_called_name_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1086;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1086;
        tmp_call_result_33 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_33, &PyTuple_GET_ITEM(mod_consts[76], 0));

        if (tmp_call_result_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1086;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_33);
    }
    {
        PyObject *tmp_called_name_34;
        PyObject *tmp_call_result_34;
        tmp_called_name_34 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_34 == NULL)) {
            tmp_called_name_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1123;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1123;
        tmp_call_result_34 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_34, &PyTuple_GET_ITEM(mod_consts[77], 0));

        if (tmp_call_result_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1123;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_34);
    }
    {
        PyObject *tmp_called_name_35;
        PyObject *tmp_call_result_35;
        tmp_called_name_35 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_35 == NULL)) {
            tmp_called_name_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1157;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1157;
        tmp_call_result_35 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_35, &PyTuple_GET_ITEM(mod_consts[78], 0));

        if (tmp_call_result_35 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1157;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_35);
    }
    {
        PyObject *tmp_called_name_36;
        PyObject *tmp_call_result_36;
        tmp_called_name_36 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_36 == NULL)) {
            tmp_called_name_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1246;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1246;
        tmp_call_result_36 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_36, &PyTuple_GET_ITEM(mod_consts[79], 0));

        if (tmp_call_result_36 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1246;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_36);
    }
    {
        PyObject *tmp_called_name_37;
        PyObject *tmp_call_result_37;
        tmp_called_name_37 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_37 == NULL)) {
            tmp_called_name_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1288;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1288;
        tmp_call_result_37 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_37, &PyTuple_GET_ITEM(mod_consts[80], 0));

        if (tmp_call_result_37 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1288;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_37);
    }
    {
        PyObject *tmp_called_name_38;
        PyObject *tmp_call_result_38;
        tmp_called_name_38 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_38 == NULL)) {
            tmp_called_name_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1291;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1291;
        tmp_call_result_38 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_38, &PyTuple_GET_ITEM(mod_consts[81], 0));

        if (tmp_call_result_38 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1291;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_38);
    }
    {
        PyObject *tmp_called_name_39;
        PyObject *tmp_call_result_39;
        tmp_called_name_39 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_39 == NULL)) {
            tmp_called_name_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1294;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1294;
        tmp_call_result_39 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_39, &PyTuple_GET_ITEM(mod_consts[82], 0));

        if (tmp_call_result_39 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1294;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_39);
    }
    {
        PyObject *tmp_called_name_40;
        PyObject *tmp_call_result_40;
        tmp_called_name_40 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_40 == NULL)) {
            tmp_called_name_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1355;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1355;
        tmp_call_result_40 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_40, &PyTuple_GET_ITEM(mod_consts[83], 0));

        if (tmp_call_result_40 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1355;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_40);
    }
    {
        PyObject *tmp_called_name_41;
        PyObject *tmp_call_result_41;
        tmp_called_name_41 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_41 == NULL)) {
            tmp_called_name_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1362;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1362;
        tmp_call_result_41 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_41, &PyTuple_GET_ITEM(mod_consts[84], 0));

        if (tmp_call_result_41 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1362;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_41);
    }
    {
        PyObject *tmp_called_name_42;
        PyObject *tmp_call_result_42;
        tmp_called_name_42 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_42 == NULL)) {
            tmp_called_name_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1370;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1370;
        tmp_call_result_42 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_42, &PyTuple_GET_ITEM(mod_consts[85], 0));

        if (tmp_call_result_42 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1370;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_42);
    }
    {
        PyObject *tmp_called_name_43;
        PyObject *tmp_call_result_43;
        tmp_called_name_43 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_43 == NULL)) {
            tmp_called_name_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1378;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1378;
        tmp_call_result_43 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_43, &PyTuple_GET_ITEM(mod_consts[86], 0));

        if (tmp_call_result_43 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1378;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_43);
    }
    {
        PyObject *tmp_called_name_44;
        PyObject *tmp_call_result_44;
        tmp_called_name_44 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_44 == NULL)) {
            tmp_called_name_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1428;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1428;
        tmp_call_result_44 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_44, &PyTuple_GET_ITEM(mod_consts[87], 0));

        if (tmp_call_result_44 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1428;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_44);
    }
    {
        PyObject *tmp_called_name_45;
        PyObject *tmp_call_result_45;
        tmp_called_name_45 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_45 == NULL)) {
            tmp_called_name_45 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1488;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1488;
        tmp_call_result_45 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_45, &PyTuple_GET_ITEM(mod_consts[88], 0));

        if (tmp_call_result_45 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1488;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_45);
    }
    {
        PyObject *tmp_called_name_46;
        PyObject *tmp_call_result_46;
        tmp_called_name_46 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_46 == NULL)) {
            tmp_called_name_46 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1807;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1807;
        tmp_call_result_46 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_46, &PyTuple_GET_ITEM(mod_consts[89], 0));

        if (tmp_call_result_46 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1807;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_46);
    }
    {
        PyObject *tmp_called_name_47;
        PyObject *tmp_call_result_47;
        tmp_called_name_47 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_47 == NULL)) {
            tmp_called_name_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1936;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1936;
        tmp_call_result_47 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_47, &PyTuple_GET_ITEM(mod_consts[90], 0));

        if (tmp_call_result_47 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1936;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_47);
    }
    {
        PyObject *tmp_called_name_48;
        PyObject *tmp_call_result_48;
        tmp_called_name_48 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_48 == NULL)) {
            tmp_called_name_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1940;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1940;
        tmp_call_result_48 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_48, &PyTuple_GET_ITEM(mod_consts[91], 0));

        if (tmp_call_result_48 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1940;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_48);
    }
    {
        PyObject *tmp_called_name_49;
        PyObject *tmp_call_result_49;
        tmp_called_name_49 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_49 == NULL)) {
            tmp_called_name_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1944;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1944;
        tmp_call_result_49 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_49, &PyTuple_GET_ITEM(mod_consts[92], 0));

        if (tmp_call_result_49 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1944;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_49);
    }
    {
        PyObject *tmp_called_name_50;
        PyObject *tmp_call_result_50;
        tmp_called_name_50 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_50 == NULL)) {
            tmp_called_name_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1948;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1948;
        tmp_call_result_50 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_50, &PyTuple_GET_ITEM(mod_consts[93], 0));

        if (tmp_call_result_50 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1948;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_50);
    }
    {
        PyObject *tmp_called_name_51;
        PyObject *tmp_call_result_51;
        tmp_called_name_51 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_51 == NULL)) {
            tmp_called_name_51 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1952;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1952;
        tmp_call_result_51 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_51, &PyTuple_GET_ITEM(mod_consts[94], 0));

        if (tmp_call_result_51 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1952;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_51);
    }
    {
        PyObject *tmp_called_name_52;
        PyObject *tmp_call_result_52;
        tmp_called_name_52 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_52 == NULL)) {
            tmp_called_name_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1973;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 1973;
        tmp_call_result_52 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_52, &PyTuple_GET_ITEM(mod_consts[95], 0));

        if (tmp_call_result_52 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 1973;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_52);
    }
    {
        PyObject *tmp_called_name_53;
        PyObject *tmp_call_result_53;
        tmp_called_name_53 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_53 == NULL)) {
            tmp_called_name_53 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2050;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2050;
        tmp_call_result_53 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_53, &PyTuple_GET_ITEM(mod_consts[96], 0));

        if (tmp_call_result_53 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2050;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_53);
    }
    {
        PyObject *tmp_called_name_54;
        PyObject *tmp_call_result_54;
        tmp_called_name_54 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_54 == NULL)) {
            tmp_called_name_54 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2054;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2054;
        tmp_call_result_54 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_54, &PyTuple_GET_ITEM(mod_consts[97], 0));

        if (tmp_call_result_54 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2054;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_54);
    }
    {
        PyObject *tmp_called_name_55;
        PyObject *tmp_call_result_55;
        tmp_called_name_55 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_55 == NULL)) {
            tmp_called_name_55 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2083;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2083;
        tmp_call_result_55 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_55, &PyTuple_GET_ITEM(mod_consts[98], 0));

        if (tmp_call_result_55 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2083;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_55);
    }
    {
        PyObject *tmp_called_name_56;
        PyObject *tmp_call_result_56;
        tmp_called_name_56 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_56 == NULL)) {
            tmp_called_name_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2098;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2098;
        tmp_call_result_56 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_56, &PyTuple_GET_ITEM(mod_consts[99], 0));

        if (tmp_call_result_56 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2098;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_56);
    }
    {
        PyObject *tmp_called_name_57;
        PyObject *tmp_call_result_57;
        tmp_called_name_57 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_57 == NULL)) {
            tmp_called_name_57 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2114;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2114;
        tmp_call_result_57 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_57, &PyTuple_GET_ITEM(mod_consts[100], 0));

        if (tmp_call_result_57 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2114;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_57);
    }
    {
        PyObject *tmp_called_name_58;
        PyObject *tmp_call_result_58;
        tmp_called_name_58 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_58 == NULL)) {
            tmp_called_name_58 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2191;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2191;
        tmp_call_result_58 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_58, &PyTuple_GET_ITEM(mod_consts[101], 0));

        if (tmp_call_result_58 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2191;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_58);
    }
    {
        PyObject *tmp_called_name_59;
        PyObject *tmp_call_result_59;
        tmp_called_name_59 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_59 == NULL)) {
            tmp_called_name_59 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2233;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2233;
        tmp_call_result_59 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_59, &PyTuple_GET_ITEM(mod_consts[102], 0));

        if (tmp_call_result_59 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2233;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_59);
    }
    {
        PyObject *tmp_called_name_60;
        PyObject *tmp_call_result_60;
        tmp_called_name_60 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_60 == NULL)) {
            tmp_called_name_60 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2253;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2253;
        tmp_call_result_60 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_60, &PyTuple_GET_ITEM(mod_consts[103], 0));

        if (tmp_call_result_60 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2253;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_60);
    }
    {
        PyObject *tmp_called_name_61;
        PyObject *tmp_call_result_61;
        tmp_called_name_61 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_61 == NULL)) {
            tmp_called_name_61 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2269;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2269;
        tmp_call_result_61 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_61, &PyTuple_GET_ITEM(mod_consts[104], 0));

        if (tmp_call_result_61 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2269;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_61);
    }
    {
        PyObject *tmp_called_name_62;
        PyObject *tmp_call_result_62;
        tmp_called_name_62 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_62 == NULL)) {
            tmp_called_name_62 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2288;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2288;
        tmp_call_result_62 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_62, &PyTuple_GET_ITEM(mod_consts[105], 0));

        if (tmp_call_result_62 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2288;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_62);
    }
    {
        PyObject *tmp_called_name_63;
        PyObject *tmp_call_result_63;
        tmp_called_name_63 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_63 == NULL)) {
            tmp_called_name_63 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2330;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2330;
        tmp_call_result_63 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_63, &PyTuple_GET_ITEM(mod_consts[106], 0));

        if (tmp_call_result_63 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2330;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_63);
    }
    {
        PyObject *tmp_called_name_64;
        PyObject *tmp_call_result_64;
        tmp_called_name_64 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_64 == NULL)) {
            tmp_called_name_64 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2356;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2356;
        tmp_call_result_64 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_64, &PyTuple_GET_ITEM(mod_consts[107], 0));

        if (tmp_call_result_64 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2356;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_64);
    }
    {
        PyObject *tmp_called_name_65;
        PyObject *tmp_call_result_65;
        tmp_called_name_65 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_65 == NULL)) {
            tmp_called_name_65 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2418;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2418;
        tmp_call_result_65 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_65, &PyTuple_GET_ITEM(mod_consts[108], 0));

        if (tmp_call_result_65 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2418;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_65);
    }
    {
        PyObject *tmp_called_name_66;
        PyObject *tmp_call_result_66;
        tmp_called_name_66 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_66 == NULL)) {
            tmp_called_name_66 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2453;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2453;
        tmp_call_result_66 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_66, &PyTuple_GET_ITEM(mod_consts[109], 0));

        if (tmp_call_result_66 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2453;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_66);
    }
    {
        PyObject *tmp_called_name_67;
        PyObject *tmp_call_result_67;
        tmp_called_name_67 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_67 == NULL)) {
            tmp_called_name_67 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2463;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2463;
        tmp_call_result_67 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_67, &PyTuple_GET_ITEM(mod_consts[110], 0));

        if (tmp_call_result_67 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2463;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_67);
    }
    {
        PyObject *tmp_called_name_68;
        PyObject *tmp_call_result_68;
        tmp_called_name_68 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_68 == NULL)) {
            tmp_called_name_68 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_68 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2469;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2469;
        tmp_call_result_68 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_68, &PyTuple_GET_ITEM(mod_consts[111], 0));

        if (tmp_call_result_68 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2469;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_68);
    }
    {
        PyObject *tmp_called_name_69;
        PyObject *tmp_call_result_69;
        tmp_called_name_69 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_69 == NULL)) {
            tmp_called_name_69 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2475;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2475;
        tmp_call_result_69 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_69, &PyTuple_GET_ITEM(mod_consts[112], 0));

        if (tmp_call_result_69 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2475;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_69);
    }
    {
        PyObject *tmp_called_name_70;
        PyObject *tmp_call_result_70;
        tmp_called_name_70 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_70 == NULL)) {
            tmp_called_name_70 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2485;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2485;
        tmp_call_result_70 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_70, &PyTuple_GET_ITEM(mod_consts[113], 0));

        if (tmp_call_result_70 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2485;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_70);
    }
    {
        PyObject *tmp_called_name_71;
        PyObject *tmp_call_result_71;
        tmp_called_name_71 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_71 == NULL)) {
            tmp_called_name_71 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_71 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2493;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2493;
        tmp_call_result_71 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_71, &PyTuple_GET_ITEM(mod_consts[114], 0));

        if (tmp_call_result_71 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2493;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_71);
    }
    {
        PyObject *tmp_called_name_72;
        PyObject *tmp_call_result_72;
        tmp_called_name_72 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_72 == NULL)) {
            tmp_called_name_72 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_72 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2501;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2501;
        tmp_call_result_72 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_72, &PyTuple_GET_ITEM(mod_consts[115], 0));

        if (tmp_call_result_72 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2501;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_72);
    }
    {
        PyObject *tmp_called_name_73;
        PyObject *tmp_call_result_73;
        tmp_called_name_73 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_73 == NULL)) {
            tmp_called_name_73 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_73 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2522;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2522;
        tmp_call_result_73 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_73, &PyTuple_GET_ITEM(mod_consts[116], 0));

        if (tmp_call_result_73 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2522;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_73);
    }
    {
        PyObject *tmp_called_name_74;
        PyObject *tmp_call_result_74;
        tmp_called_name_74 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_74 == NULL)) {
            tmp_called_name_74 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_74 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2537;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2537;
        tmp_call_result_74 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_74, &PyTuple_GET_ITEM(mod_consts[117], 0));

        if (tmp_call_result_74 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2537;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_74);
    }
    {
        PyObject *tmp_called_name_75;
        PyObject *tmp_call_result_75;
        tmp_called_name_75 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_75 == NULL)) {
            tmp_called_name_75 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_75 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2552;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2552;
        tmp_call_result_75 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_75, &PyTuple_GET_ITEM(mod_consts[118], 0));

        if (tmp_call_result_75 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2552;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_75);
    }
    {
        PyObject *tmp_called_name_76;
        PyObject *tmp_call_result_76;
        tmp_called_name_76 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_76 == NULL)) {
            tmp_called_name_76 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2567;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2567;
        tmp_call_result_76 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_76, &PyTuple_GET_ITEM(mod_consts[119], 0));

        if (tmp_call_result_76 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2567;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_76);
    }
    {
        PyObject *tmp_called_name_77;
        PyObject *tmp_call_result_77;
        tmp_called_name_77 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_77 == NULL)) {
            tmp_called_name_77 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_77 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2582;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2582;
        tmp_call_result_77 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_77, &PyTuple_GET_ITEM(mod_consts[120], 0));

        if (tmp_call_result_77 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2582;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_77);
    }
    {
        PyObject *tmp_called_name_78;
        PyObject *tmp_call_result_78;
        tmp_called_name_78 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_78 == NULL)) {
            tmp_called_name_78 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_78 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2597;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2597;
        tmp_call_result_78 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_78, &PyTuple_GET_ITEM(mod_consts[121], 0));

        if (tmp_call_result_78 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2597;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_78);
    }
    {
        PyObject *tmp_called_name_79;
        PyObject *tmp_call_result_79;
        tmp_called_name_79 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_79 == NULL)) {
            tmp_called_name_79 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_79 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2614;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2614;
        tmp_call_result_79 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_79, &PyTuple_GET_ITEM(mod_consts[122], 0));

        if (tmp_call_result_79 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2614;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_79);
    }
    {
        PyObject *tmp_called_name_80;
        PyObject *tmp_call_result_80;
        tmp_called_name_80 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_80 == NULL)) {
            tmp_called_name_80 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_80 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2688;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2688;
        tmp_call_result_80 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_80, &PyTuple_GET_ITEM(mod_consts[123], 0));

        if (tmp_call_result_80 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2688;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_80);
    }
    {
        PyObject *tmp_called_name_81;
        PyObject *tmp_call_result_81;
        tmp_called_name_81 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_81 == NULL)) {
            tmp_called_name_81 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_81 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2742;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2742;
        tmp_call_result_81 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_81, &PyTuple_GET_ITEM(mod_consts[124], 0));

        if (tmp_call_result_81 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2742;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_81);
    }
    {
        PyObject *tmp_called_name_82;
        PyObject *tmp_call_result_82;
        tmp_called_name_82 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_82 == NULL)) {
            tmp_called_name_82 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_82 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2757;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2757;
        tmp_call_result_82 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_82, &PyTuple_GET_ITEM(mod_consts[125], 0));

        if (tmp_call_result_82 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2757;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_82);
    }
    {
        PyObject *tmp_called_name_83;
        PyObject *tmp_call_result_83;
        tmp_called_name_83 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_83 == NULL)) {
            tmp_called_name_83 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_83 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2773;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2773;
        tmp_call_result_83 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_83, &PyTuple_GET_ITEM(mod_consts[126], 0));

        if (tmp_call_result_83 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2773;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_83);
    }
    {
        PyObject *tmp_called_name_84;
        PyObject *tmp_call_result_84;
        tmp_called_name_84 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_84 == NULL)) {
            tmp_called_name_84 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_84 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2788;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2788;
        tmp_call_result_84 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_84, &PyTuple_GET_ITEM(mod_consts[127], 0));

        if (tmp_call_result_84 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2788;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_84);
    }
    {
        PyObject *tmp_called_name_85;
        PyObject *tmp_call_result_85;
        tmp_called_name_85 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_85 == NULL)) {
            tmp_called_name_85 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_85 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2803;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2803;
        tmp_call_result_85 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_85, &PyTuple_GET_ITEM(mod_consts[128], 0));

        if (tmp_call_result_85 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2803;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_85);
    }
    {
        PyObject *tmp_called_name_86;
        PyObject *tmp_call_result_86;
        tmp_called_name_86 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_86 == NULL)) {
            tmp_called_name_86 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_86 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2818;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2818;
        tmp_call_result_86 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_86, &PyTuple_GET_ITEM(mod_consts[129], 0));

        if (tmp_call_result_86 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2818;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_86);
    }
    {
        PyObject *tmp_called_name_87;
        PyObject *tmp_call_result_87;
        tmp_called_name_87 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_87 == NULL)) {
            tmp_called_name_87 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_87 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2861;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2861;
        tmp_call_result_87 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_87, &PyTuple_GET_ITEM(mod_consts[130], 0));

        if (tmp_call_result_87 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2861;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_87);
    }
    {
        PyObject *tmp_called_name_88;
        PyObject *tmp_call_result_88;
        tmp_called_name_88 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_88 == NULL)) {
            tmp_called_name_88 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_88 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2876;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2876;
        tmp_call_result_88 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_88, &PyTuple_GET_ITEM(mod_consts[131], 0));

        if (tmp_call_result_88 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2876;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_88);
    }
    {
        PyObject *tmp_called_name_89;
        PyObject *tmp_call_result_89;
        tmp_called_name_89 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_89 == NULL)) {
            tmp_called_name_89 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_89 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2891;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2891;
        tmp_call_result_89 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_89, &PyTuple_GET_ITEM(mod_consts[132], 0));

        if (tmp_call_result_89 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2891;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_89);
    }
    {
        PyObject *tmp_called_name_90;
        PyObject *tmp_call_result_90;
        tmp_called_name_90 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_90 == NULL)) {
            tmp_called_name_90 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_90 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2908;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2908;
        tmp_call_result_90 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_90, &PyTuple_GET_ITEM(mod_consts[133], 0));

        if (tmp_call_result_90 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2908;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_90);
    }
    {
        PyObject *tmp_called_name_91;
        PyObject *tmp_call_result_91;
        tmp_called_name_91 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_91 == NULL)) {
            tmp_called_name_91 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_91 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2937;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2937;
        tmp_call_result_91 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_91, &PyTuple_GET_ITEM(mod_consts[134], 0));

        if (tmp_call_result_91 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2937;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_91);
    }
    {
        PyObject *tmp_called_name_92;
        PyObject *tmp_call_result_92;
        tmp_called_name_92 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_92 == NULL)) {
            tmp_called_name_92 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_92 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2954;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2954;
        tmp_call_result_92 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_92, &PyTuple_GET_ITEM(mod_consts[135], 0));

        if (tmp_call_result_92 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2954;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_92);
    }
    {
        PyObject *tmp_called_name_93;
        PyObject *tmp_call_result_93;
        tmp_called_name_93 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_93 == NULL)) {
            tmp_called_name_93 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_93 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2968;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2968;
        tmp_call_result_93 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_93, &PyTuple_GET_ITEM(mod_consts[136], 0));

        if (tmp_call_result_93 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2968;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_93);
    }
    {
        PyObject *tmp_called_name_94;
        PyObject *tmp_call_result_94;
        tmp_called_name_94 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_94 == NULL)) {
            tmp_called_name_94 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_94 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2993;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 2993;
        tmp_call_result_94 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_94, &PyTuple_GET_ITEM(mod_consts[137], 0));

        if (tmp_call_result_94 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 2993;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_94);
    }
    {
        PyObject *tmp_called_name_95;
        PyObject *tmp_call_result_95;
        tmp_called_name_95 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_95 == NULL)) {
            tmp_called_name_95 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_95 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3031;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3031;
        tmp_call_result_95 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_95, &PyTuple_GET_ITEM(mod_consts[138], 0));

        if (tmp_call_result_95 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3031;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_95);
    }
    {
        PyObject *tmp_called_name_96;
        PyObject *tmp_call_result_96;
        tmp_called_name_96 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_96 == NULL)) {
            tmp_called_name_96 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_96 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3073;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3073;
        tmp_call_result_96 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_96, &PyTuple_GET_ITEM(mod_consts[139], 0));

        if (tmp_call_result_96 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3073;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_96);
    }
    {
        PyObject *tmp_called_name_97;
        PyObject *tmp_call_result_97;
        tmp_called_name_97 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_97 == NULL)) {
            tmp_called_name_97 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_97 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3132;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3132;
        tmp_call_result_97 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_97, &PyTuple_GET_ITEM(mod_consts[140], 0));

        if (tmp_call_result_97 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3132;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_97);
    }
    {
        PyObject *tmp_called_name_98;
        PyObject *tmp_call_result_98;
        tmp_called_name_98 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_98 == NULL)) {
            tmp_called_name_98 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_98 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3179;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3179;
        tmp_call_result_98 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_98, &PyTuple_GET_ITEM(mod_consts[141], 0));

        if (tmp_call_result_98 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3179;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_98);
    }
    {
        PyObject *tmp_called_name_99;
        PyObject *tmp_call_result_99;
        tmp_called_name_99 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_99 == NULL)) {
            tmp_called_name_99 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_99 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3194;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3194;
        tmp_call_result_99 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_99, &PyTuple_GET_ITEM(mod_consts[142], 0));

        if (tmp_call_result_99 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3194;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_99);
    }
    {
        PyObject *tmp_called_name_100;
        PyObject *tmp_call_result_100;
        tmp_called_name_100 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_100 == NULL)) {
            tmp_called_name_100 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_100 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3209;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3209;
        tmp_call_result_100 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_100, &PyTuple_GET_ITEM(mod_consts[143], 0));

        if (tmp_call_result_100 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3209;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_100);
    }
    {
        PyObject *tmp_called_name_101;
        PyObject *tmp_call_result_101;
        tmp_called_name_101 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_101 == NULL)) {
            tmp_called_name_101 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_101 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3224;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3224;
        tmp_call_result_101 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_101, &PyTuple_GET_ITEM(mod_consts[144], 0));

        if (tmp_call_result_101 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3224;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_101);
    }
    {
        PyObject *tmp_called_name_102;
        PyObject *tmp_call_result_102;
        tmp_called_name_102 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_102 == NULL)) {
            tmp_called_name_102 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_102 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3266;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3266;
        tmp_call_result_102 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_102, &PyTuple_GET_ITEM(mod_consts[145], 0));

        if (tmp_call_result_102 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3266;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_102);
    }
    {
        PyObject *tmp_called_name_103;
        PyObject *tmp_call_result_103;
        tmp_called_name_103 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_103 == NULL)) {
            tmp_called_name_103 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_103 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3281;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3281;
        tmp_call_result_103 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_103, &PyTuple_GET_ITEM(mod_consts[146], 0));

        if (tmp_call_result_103 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3281;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_103);
    }
    {
        PyObject *tmp_called_name_104;
        PyObject *tmp_call_result_104;
        tmp_called_name_104 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_104 == NULL)) {
            tmp_called_name_104 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_104 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3296;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3296;
        tmp_call_result_104 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_104, &PyTuple_GET_ITEM(mod_consts[147], 0));

        if (tmp_call_result_104 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3296;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_104);
    }
    {
        PyObject *tmp_called_name_105;
        PyObject *tmp_call_result_105;
        tmp_called_name_105 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_105 == NULL)) {
            tmp_called_name_105 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_105 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3311;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3311;
        tmp_call_result_105 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_105, &PyTuple_GET_ITEM(mod_consts[148], 0));

        if (tmp_call_result_105 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3311;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_105);
    }
    {
        PyObject *tmp_called_name_106;
        PyObject *tmp_call_result_106;
        tmp_called_name_106 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_106 == NULL)) {
            tmp_called_name_106 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_106 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3326;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3326;
        tmp_call_result_106 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_106, &PyTuple_GET_ITEM(mod_consts[149], 0));

        if (tmp_call_result_106 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3326;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_106);
    }
    {
        PyObject *tmp_called_name_107;
        PyObject *tmp_call_result_107;
        tmp_called_name_107 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_107 == NULL)) {
            tmp_called_name_107 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_107 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3343;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3343;
        tmp_call_result_107 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_107, &PyTuple_GET_ITEM(mod_consts[150], 0));

        if (tmp_call_result_107 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3343;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_107);
    }
    {
        PyObject *tmp_called_name_108;
        PyObject *tmp_call_result_108;
        tmp_called_name_108 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_108 == NULL)) {
            tmp_called_name_108 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_108 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3358;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3358;
        tmp_call_result_108 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_108, &PyTuple_GET_ITEM(mod_consts[151], 0));

        if (tmp_call_result_108 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3358;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_108);
    }
    {
        PyObject *tmp_called_name_109;
        PyObject *tmp_call_result_109;
        tmp_called_name_109 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_109 == NULL)) {
            tmp_called_name_109 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_109 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3380;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3380;
        tmp_call_result_109 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_109, &PyTuple_GET_ITEM(mod_consts[152], 0));

        if (tmp_call_result_109 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3380;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_109);
    }
    {
        PyObject *tmp_called_name_110;
        PyObject *tmp_call_result_110;
        tmp_called_name_110 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_110 == NULL)) {
            tmp_called_name_110 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_110 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3472;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3472;
        tmp_call_result_110 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_110, &PyTuple_GET_ITEM(mod_consts[153], 0));

        if (tmp_call_result_110 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3472;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_110);
    }
    {
        PyObject *tmp_called_name_111;
        PyObject *tmp_call_result_111;
        tmp_called_name_111 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_111 == NULL)) {
            tmp_called_name_111 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_111 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3487;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3487;
        tmp_call_result_111 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_111, &PyTuple_GET_ITEM(mod_consts[154], 0));

        if (tmp_call_result_111 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3487;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_111);
    }
    {
        PyObject *tmp_called_name_112;
        PyObject *tmp_call_result_112;
        tmp_called_name_112 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_112 == NULL)) {
            tmp_called_name_112 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_112 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3502;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3502;
        tmp_call_result_112 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_112, &PyTuple_GET_ITEM(mod_consts[155], 0));

        if (tmp_call_result_112 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3502;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_112);
    }
    {
        PyObject *tmp_called_name_113;
        PyObject *tmp_call_result_113;
        tmp_called_name_113 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_113 == NULL)) {
            tmp_called_name_113 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_113 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3553;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3553;
        tmp_call_result_113 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_113, &PyTuple_GET_ITEM(mod_consts[156], 0));

        if (tmp_call_result_113 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3553;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_113);
    }
    {
        PyObject *tmp_called_name_114;
        PyObject *tmp_call_result_114;
        tmp_called_name_114 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_114 == NULL)) {
            tmp_called_name_114 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_114 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3633;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3633;
        tmp_call_result_114 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_114, &PyTuple_GET_ITEM(mod_consts[157], 0));

        if (tmp_call_result_114 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3633;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_114);
    }
    {
        PyObject *tmp_called_name_115;
        PyObject *tmp_call_result_115;
        tmp_called_name_115 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_115 == NULL)) {
            tmp_called_name_115 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_115 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3696;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3696;
        tmp_call_result_115 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_115, &PyTuple_GET_ITEM(mod_consts[158], 0));

        if (tmp_call_result_115 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3696;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_115);
    }
    {
        PyObject *tmp_called_name_116;
        PyObject *tmp_call_result_116;
        tmp_called_name_116 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_116 == NULL)) {
            tmp_called_name_116 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_116 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3752;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3752;
        tmp_call_result_116 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_116, &PyTuple_GET_ITEM(mod_consts[159], 0));

        if (tmp_call_result_116 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3752;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_116);
    }
    {
        PyObject *tmp_called_name_117;
        PyObject *tmp_call_result_117;
        tmp_called_name_117 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_117 == NULL)) {
            tmp_called_name_117 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_117 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3767;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3767;
        tmp_call_result_117 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_117, &PyTuple_GET_ITEM(mod_consts[160], 0));

        if (tmp_call_result_117 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3767;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_117);
    }
    {
        PyObject *tmp_called_name_118;
        PyObject *tmp_call_result_118;
        tmp_called_name_118 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_118 == NULL)) {
            tmp_called_name_118 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_118 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3782;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3782;
        tmp_call_result_118 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_118, &PyTuple_GET_ITEM(mod_consts[161], 0));

        if (tmp_call_result_118 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3782;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_118);
    }
    {
        PyObject *tmp_called_name_119;
        PyObject *tmp_call_result_119;
        tmp_called_name_119 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_119 == NULL)) {
            tmp_called_name_119 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_119 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3797;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3797;
        tmp_call_result_119 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_119, &PyTuple_GET_ITEM(mod_consts[162], 0));

        if (tmp_call_result_119 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3797;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_119);
    }
    {
        PyObject *tmp_called_name_120;
        PyObject *tmp_call_result_120;
        tmp_called_name_120 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_120 == NULL)) {
            tmp_called_name_120 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_120 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3812;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3812;
        tmp_call_result_120 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_120, &PyTuple_GET_ITEM(mod_consts[163], 0));

        if (tmp_call_result_120 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3812;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_120);
    }
    {
        PyObject *tmp_called_name_121;
        PyObject *tmp_call_result_121;
        tmp_called_name_121 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_121 == NULL)) {
            tmp_called_name_121 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_121 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3827;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3827;
        tmp_call_result_121 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_121, &PyTuple_GET_ITEM(mod_consts[164], 0));

        if (tmp_call_result_121 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3827;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_121);
    }
    {
        PyObject *tmp_called_name_122;
        PyObject *tmp_call_result_122;
        tmp_called_name_122 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_122 == NULL)) {
            tmp_called_name_122 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_122 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3871;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3871;
        tmp_call_result_122 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_122, &PyTuple_GET_ITEM(mod_consts[165], 0));

        if (tmp_call_result_122 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3871;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_122);
    }
    {
        PyObject *tmp_called_name_123;
        PyObject *tmp_call_result_123;
        tmp_called_name_123 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_123 == NULL)) {
            tmp_called_name_123 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_123 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3935;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3935;
        tmp_call_result_123 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_123, &PyTuple_GET_ITEM(mod_consts[166], 0));

        if (tmp_call_result_123 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3935;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_123);
    }
    {
        PyObject *tmp_called_name_124;
        PyObject *tmp_call_result_124;
        tmp_called_name_124 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_124 == NULL)) {
            tmp_called_name_124 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_124 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3972;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3972;
        tmp_call_result_124 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_124, &PyTuple_GET_ITEM(mod_consts[167], 0));

        if (tmp_call_result_124 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3972;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_124);
    }
    {
        PyObject *tmp_called_name_125;
        PyObject *tmp_call_result_125;
        tmp_called_name_125 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_125 == NULL)) {
            tmp_called_name_125 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_125 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3983;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3983;
        tmp_call_result_125 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_125, &PyTuple_GET_ITEM(mod_consts[168], 0));

        if (tmp_call_result_125 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3983;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_125);
    }
    {
        PyObject *tmp_called_name_126;
        PyObject *tmp_call_result_126;
        tmp_called_name_126 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_126 == NULL)) {
            tmp_called_name_126 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_126 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3998;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 3998;
        tmp_call_result_126 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_126, &PyTuple_GET_ITEM(mod_consts[169], 0));

        if (tmp_call_result_126 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 3998;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_126);
    }
    {
        PyObject *tmp_called_name_127;
        PyObject *tmp_call_result_127;
        tmp_called_name_127 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_127 == NULL)) {
            tmp_called_name_127 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_127 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4055;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4055;
        tmp_call_result_127 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_127, &PyTuple_GET_ITEM(mod_consts[170], 0));

        if (tmp_call_result_127 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4055;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_127);
    }
    {
        PyObject *tmp_called_name_128;
        PyObject *tmp_call_result_128;
        tmp_called_name_128 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_128 == NULL)) {
            tmp_called_name_128 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_128 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4070;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4070;
        tmp_call_result_128 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_128, &PyTuple_GET_ITEM(mod_consts[171], 0));

        if (tmp_call_result_128 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4070;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_128);
    }
    {
        PyObject *tmp_called_name_129;
        PyObject *tmp_call_result_129;
        tmp_called_name_129 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_129 == NULL)) {
            tmp_called_name_129 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_129 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4181;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4181;
        tmp_call_result_129 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_129, &PyTuple_GET_ITEM(mod_consts[172], 0));

        if (tmp_call_result_129 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4181;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_129);
    }
    {
        PyObject *tmp_called_name_130;
        PyObject *tmp_call_result_130;
        tmp_called_name_130 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_130 == NULL)) {
            tmp_called_name_130 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_130 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4231;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4231;
        tmp_call_result_130 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_130, &PyTuple_GET_ITEM(mod_consts[173], 0));

        if (tmp_call_result_130 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4231;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_130);
    }
    {
        PyObject *tmp_called_name_131;
        PyObject *tmp_call_result_131;
        tmp_called_name_131 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_131 == NULL)) {
            tmp_called_name_131 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_131 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4295;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4295;
        tmp_call_result_131 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_131, &PyTuple_GET_ITEM(mod_consts[174], 0));

        if (tmp_call_result_131 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4295;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_131);
    }
    {
        PyObject *tmp_called_name_132;
        PyObject *tmp_call_result_132;
        tmp_called_name_132 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_132 == NULL)) {
            tmp_called_name_132 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_132 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4361;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4361;
        tmp_call_result_132 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_132, &PyTuple_GET_ITEM(mod_consts[175], 0));

        if (tmp_call_result_132 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4361;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_132);
    }
    {
        PyObject *tmp_called_name_133;
        PyObject *tmp_call_result_133;
        tmp_called_name_133 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_133 == NULL)) {
            tmp_called_name_133 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_133 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4371;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4371;
        tmp_call_result_133 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_133, &PyTuple_GET_ITEM(mod_consts[176], 0));

        if (tmp_call_result_133 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4371;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_133);
    }
    {
        PyObject *tmp_called_name_134;
        PyObject *tmp_call_result_134;
        tmp_called_name_134 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_134 == NULL)) {
            tmp_called_name_134 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_134 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4397;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4397;
        tmp_call_result_134 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_134, &PyTuple_GET_ITEM(mod_consts[177], 0));

        if (tmp_call_result_134 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4397;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_134);
    }
    {
        PyObject *tmp_called_name_135;
        PyObject *tmp_call_result_135;
        tmp_called_name_135 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_135 == NULL)) {
            tmp_called_name_135 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_135 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4406;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4406;
        tmp_call_result_135 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_135, &PyTuple_GET_ITEM(mod_consts[178], 0));

        if (tmp_call_result_135 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4406;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_135);
    }
    {
        PyObject *tmp_called_name_136;
        PyObject *tmp_call_result_136;
        tmp_called_name_136 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_136 == NULL)) {
            tmp_called_name_136 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_136 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4451;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4451;
        tmp_call_result_136 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_136, &PyTuple_GET_ITEM(mod_consts[179], 0));

        if (tmp_call_result_136 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4451;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_136);
    }
    {
        PyObject *tmp_called_name_137;
        PyObject *tmp_call_result_137;
        tmp_called_name_137 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_137 == NULL)) {
            tmp_called_name_137 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_137 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4511;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4511;
        tmp_call_result_137 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_137, &PyTuple_GET_ITEM(mod_consts[180], 0));

        if (tmp_call_result_137 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4511;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_137);
    }
    {
        PyObject *tmp_called_name_138;
        PyObject *tmp_call_result_138;
        tmp_called_name_138 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_138 == NULL)) {
            tmp_called_name_138 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_138 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4530;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4530;
        tmp_call_result_138 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_138, &PyTuple_GET_ITEM(mod_consts[181], 0));

        if (tmp_call_result_138 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4530;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_138);
    }
    {
        PyObject *tmp_called_name_139;
        PyObject *tmp_call_result_139;
        tmp_called_name_139 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_139 == NULL)) {
            tmp_called_name_139 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_139 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4554;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4554;
        tmp_call_result_139 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_139, &PyTuple_GET_ITEM(mod_consts[182], 0));

        if (tmp_call_result_139 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4554;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_139);
    }
    {
        PyObject *tmp_called_name_140;
        PyObject *tmp_call_result_140;
        tmp_called_name_140 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_140 == NULL)) {
            tmp_called_name_140 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_140 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4572;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4572;
        tmp_call_result_140 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_140, &PyTuple_GET_ITEM(mod_consts[183], 0));

        if (tmp_call_result_140 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4572;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_140);
    }
    {
        PyObject *tmp_called_name_141;
        PyObject *tmp_call_result_141;
        tmp_called_name_141 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_141 == NULL)) {
            tmp_called_name_141 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_141 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4595;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4595;
        tmp_call_result_141 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_141, &PyTuple_GET_ITEM(mod_consts[184], 0));

        if (tmp_call_result_141 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4595;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_141);
    }
    {
        PyObject *tmp_called_name_142;
        PyObject *tmp_call_result_142;
        tmp_called_name_142 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_142 == NULL)) {
            tmp_called_name_142 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_142 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4621;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4621;
        tmp_call_result_142 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_142, &PyTuple_GET_ITEM(mod_consts[185], 0));

        if (tmp_call_result_142 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4621;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_142);
    }
    {
        PyObject *tmp_called_name_143;
        PyObject *tmp_call_result_143;
        tmp_called_name_143 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_143 == NULL)) {
            tmp_called_name_143 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_143 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4658;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4658;
        tmp_call_result_143 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_143, &PyTuple_GET_ITEM(mod_consts[186], 0));

        if (tmp_call_result_143 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4658;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_143);
    }
    {
        PyObject *tmp_called_name_144;
        PyObject *tmp_call_result_144;
        tmp_called_name_144 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_144 == NULL)) {
            tmp_called_name_144 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_144 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4695;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4695;
        tmp_call_result_144 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_144, &PyTuple_GET_ITEM(mod_consts[187], 0));

        if (tmp_call_result_144 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4695;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_144);
    }
    {
        PyObject *tmp_called_name_145;
        PyObject *tmp_call_result_145;
        tmp_called_name_145 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_145 == NULL)) {
            tmp_called_name_145 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_145 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4825;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4825;
        tmp_call_result_145 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_145, &PyTuple_GET_ITEM(mod_consts[188], 0));

        if (tmp_call_result_145 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4825;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_145);
    }
    {
        PyObject *tmp_called_name_146;
        PyObject *tmp_call_result_146;
        tmp_called_name_146 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_146 == NULL)) {
            tmp_called_name_146 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_146 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4904;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 4904;
        tmp_call_result_146 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_146, &PyTuple_GET_ITEM(mod_consts[189], 0));

        if (tmp_call_result_146 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 4904;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_146);
    }
    {
        PyObject *tmp_called_name_147;
        PyObject *tmp_call_result_147;
        tmp_called_name_147 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_147 == NULL)) {
            tmp_called_name_147 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_147 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5011;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5011;
        tmp_call_result_147 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_147, &PyTuple_GET_ITEM(mod_consts[190], 0));

        if (tmp_call_result_147 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5011;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_147);
    }
    {
        PyObject *tmp_called_name_148;
        PyObject *tmp_call_result_148;
        tmp_called_name_148 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_148 == NULL)) {
            tmp_called_name_148 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_148 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5081;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5081;
        tmp_call_result_148 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_148, &PyTuple_GET_ITEM(mod_consts[191], 0));

        if (tmp_call_result_148 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5081;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_148);
    }
    {
        PyObject *tmp_called_name_149;
        PyObject *tmp_call_result_149;
        tmp_called_name_149 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_149 == NULL)) {
            tmp_called_name_149 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_149 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5145;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5145;
        tmp_call_result_149 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_149, &PyTuple_GET_ITEM(mod_consts[192], 0));

        if (tmp_call_result_149 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5145;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_149);
    }
    {
        PyObject *tmp_called_name_150;
        PyObject *tmp_call_result_150;
        tmp_called_name_150 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_150 == NULL)) {
            tmp_called_name_150 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_150 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5236;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5236;
        tmp_call_result_150 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_150, &PyTuple_GET_ITEM(mod_consts[193], 0));

        if (tmp_call_result_150 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5236;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_150);
    }
    {
        PyObject *tmp_called_name_151;
        PyObject *tmp_call_result_151;
        tmp_called_name_151 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_151 == NULL)) {
            tmp_called_name_151 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_151 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5255;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5255;
        tmp_call_result_151 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_151, &PyTuple_GET_ITEM(mod_consts[194], 0));

        if (tmp_call_result_151 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5255;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_151);
    }
    {
        PyObject *tmp_called_name_152;
        PyObject *tmp_call_result_152;
        tmp_called_name_152 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_152 == NULL)) {
            tmp_called_name_152 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_152 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5298;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5298;
        tmp_call_result_152 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_152, &PyTuple_GET_ITEM(mod_consts[195], 0));

        if (tmp_call_result_152 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5298;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_152);
    }
    {
        PyObject *tmp_called_name_153;
        PyObject *tmp_call_result_153;
        tmp_called_name_153 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_153 == NULL)) {
            tmp_called_name_153 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_153 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5310;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5310;
        tmp_call_result_153 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_153, &PyTuple_GET_ITEM(mod_consts[196], 0));

        if (tmp_call_result_153 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5310;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_153);
    }
    {
        PyObject *tmp_called_name_154;
        PyObject *tmp_call_result_154;
        tmp_called_name_154 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_154 == NULL)) {
            tmp_called_name_154 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_154 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5334;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5334;
        tmp_call_result_154 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_154, &PyTuple_GET_ITEM(mod_consts[197], 0));

        if (tmp_call_result_154 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5334;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_154);
    }
    {
        PyObject *tmp_called_name_155;
        PyObject *tmp_call_result_155;
        tmp_called_name_155 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_155 == NULL)) {
            tmp_called_name_155 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_155 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5362;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5362;
        tmp_call_result_155 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_155, &PyTuple_GET_ITEM(mod_consts[198], 0));

        if (tmp_call_result_155 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5362;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_155);
    }
    {
        PyObject *tmp_called_name_156;
        PyObject *tmp_call_result_156;
        tmp_called_name_156 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_156 == NULL)) {
            tmp_called_name_156 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_156 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5386;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5386;
        tmp_call_result_156 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_156, &PyTuple_GET_ITEM(mod_consts[199], 0));

        if (tmp_call_result_156 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5386;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_156);
    }
    {
        PyObject *tmp_called_name_157;
        PyObject *tmp_call_result_157;
        tmp_called_name_157 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_157 == NULL)) {
            tmp_called_name_157 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_157 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5399;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5399;
        tmp_call_result_157 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_157, &PyTuple_GET_ITEM(mod_consts[200], 0));

        if (tmp_call_result_157 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5399;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_157);
    }
    {
        PyObject *tmp_called_name_158;
        PyObject *tmp_call_result_158;
        tmp_called_name_158 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_158 == NULL)) {
            tmp_called_name_158 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_158 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5428;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5428;
        tmp_call_result_158 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_158, &PyTuple_GET_ITEM(mod_consts[201], 0));

        if (tmp_call_result_158 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5428;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_158);
    }
    {
        PyObject *tmp_called_name_159;
        PyObject *tmp_call_result_159;
        tmp_called_name_159 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_159 == NULL)) {
            tmp_called_name_159 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_159 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5435;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5435;
        tmp_call_result_159 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_159, &PyTuple_GET_ITEM(mod_consts[202], 0));

        if (tmp_call_result_159 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5435;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_159);
    }
    {
        PyObject *tmp_called_name_160;
        PyObject *tmp_call_result_160;
        tmp_called_name_160 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_160 == NULL)) {
            tmp_called_name_160 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_160 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5444;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5444;
        tmp_call_result_160 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_160, &PyTuple_GET_ITEM(mod_consts[203], 0));

        if (tmp_call_result_160 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5444;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_160);
    }
    {
        PyObject *tmp_called_name_161;
        PyObject *tmp_call_result_161;
        tmp_called_name_161 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_161 == NULL)) {
            tmp_called_name_161 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_161 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5466;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5466;
        tmp_call_result_161 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_161, &PyTuple_GET_ITEM(mod_consts[204], 0));

        if (tmp_call_result_161 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5466;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_161);
    }
    {
        PyObject *tmp_called_name_162;
        PyObject *tmp_call_result_162;
        tmp_called_name_162 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_162 == NULL)) {
            tmp_called_name_162 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_162 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5499;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5499;
        tmp_call_result_162 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_162, &PyTuple_GET_ITEM(mod_consts[205], 0));

        if (tmp_call_result_162 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5499;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_162);
    }
    {
        PyObject *tmp_called_name_163;
        PyObject *tmp_call_result_163;
        tmp_called_name_163 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_163 == NULL)) {
            tmp_called_name_163 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_163 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5517;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5517;
        tmp_call_result_163 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_163, &PyTuple_GET_ITEM(mod_consts[206], 0));

        if (tmp_call_result_163 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5517;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_163);
    }
    {
        PyObject *tmp_called_name_164;
        PyObject *tmp_call_result_164;
        tmp_called_name_164 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_164 == NULL)) {
            tmp_called_name_164 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_164 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5532;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5532;
        tmp_call_result_164 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_164, &PyTuple_GET_ITEM(mod_consts[207], 0));

        if (tmp_call_result_164 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5532;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_164);
    }
    {
        PyObject *tmp_called_name_165;
        PyObject *tmp_call_result_165;
        tmp_called_name_165 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_165 == NULL)) {
            tmp_called_name_165 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_165 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5551;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5551;
        tmp_call_result_165 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_165, &PyTuple_GET_ITEM(mod_consts[208], 0));

        if (tmp_call_result_165 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5551;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_165);
    }
    {
        PyObject *tmp_called_name_166;
        PyObject *tmp_call_result_166;
        tmp_called_name_166 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_166 == NULL)) {
            tmp_called_name_166 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_166 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5569;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5569;
        tmp_call_result_166 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_166, &PyTuple_GET_ITEM(mod_consts[209], 0));

        if (tmp_call_result_166 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5569;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_166);
    }
    {
        PyObject *tmp_called_name_167;
        PyObject *tmp_call_result_167;
        tmp_called_name_167 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_167 == NULL)) {
            tmp_called_name_167 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_167 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5592;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5592;
        tmp_call_result_167 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_167, &PyTuple_GET_ITEM(mod_consts[210], 0));

        if (tmp_call_result_167 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5592;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_167);
    }
    {
        PyObject *tmp_called_name_168;
        PyObject *tmp_call_result_168;
        tmp_called_name_168 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_168 == NULL)) {
            tmp_called_name_168 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_168 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5595;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5595;
        tmp_call_result_168 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_168, &PyTuple_GET_ITEM(mod_consts[211], 0));

        if (tmp_call_result_168 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5595;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_168);
    }
    {
        PyObject *tmp_called_name_169;
        PyObject *tmp_call_result_169;
        tmp_called_name_169 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_169 == NULL)) {
            tmp_called_name_169 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_169 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5623;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5623;
        tmp_call_result_169 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_169, &PyTuple_GET_ITEM(mod_consts[212], 0));

        if (tmp_call_result_169 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5623;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_169);
    }
    {
        PyObject *tmp_called_name_170;
        PyObject *tmp_call_result_170;
        tmp_called_name_170 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_170 == NULL)) {
            tmp_called_name_170 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_170 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5644;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5644;
        tmp_call_result_170 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_170, &PyTuple_GET_ITEM(mod_consts[213], 0));

        if (tmp_call_result_170 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5644;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_170);
    }
    {
        PyObject *tmp_called_name_171;
        PyObject *tmp_call_result_171;
        tmp_called_name_171 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_171 == NULL)) {
            tmp_called_name_171 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_171 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5653;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5653;
        tmp_call_result_171 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_171, &PyTuple_GET_ITEM(mod_consts[214], 0));

        if (tmp_call_result_171 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5653;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_171);
    }
    {
        PyObject *tmp_called_name_172;
        PyObject *tmp_call_result_172;
        tmp_called_name_172 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_172 == NULL)) {
            tmp_called_name_172 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_172 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5725;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5725;
        tmp_call_result_172 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_172, &PyTuple_GET_ITEM(mod_consts[215], 0));

        if (tmp_call_result_172 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5725;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_172);
    }
    {
        PyObject *tmp_called_name_173;
        PyObject *tmp_call_result_173;
        tmp_called_name_173 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_173 == NULL)) {
            tmp_called_name_173 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_173 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5789;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5789;
        tmp_call_result_173 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_173, &PyTuple_GET_ITEM(mod_consts[216], 0));

        if (tmp_call_result_173 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5789;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_173);
    }
    {
        PyObject *tmp_called_name_174;
        PyObject *tmp_call_result_174;
        tmp_called_name_174 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_174 == NULL)) {
            tmp_called_name_174 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_174 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5792;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5792;
        tmp_call_result_174 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_174, &PyTuple_GET_ITEM(mod_consts[217], 0));

        if (tmp_call_result_174 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5792;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_174);
    }
    {
        PyObject *tmp_called_name_175;
        PyObject *tmp_call_result_175;
        tmp_called_name_175 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_175 == NULL)) {
            tmp_called_name_175 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_175 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5795;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5795;
        tmp_call_result_175 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_175, &PyTuple_GET_ITEM(mod_consts[218], 0));

        if (tmp_call_result_175 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5795;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_175);
    }
    {
        PyObject *tmp_called_name_176;
        PyObject *tmp_call_result_176;
        tmp_called_name_176 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_176 == NULL)) {
            tmp_called_name_176 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_176 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5846;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5846;
        tmp_call_result_176 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_176, &PyTuple_GET_ITEM(mod_consts[219], 0));

        if (tmp_call_result_176 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5846;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_176);
    }
    {
        PyObject *tmp_called_name_177;
        PyObject *tmp_call_result_177;
        tmp_called_name_177 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_177 == NULL)) {
            tmp_called_name_177 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_177 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5890;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5890;
        tmp_call_result_177 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_177, &PyTuple_GET_ITEM(mod_consts[220], 0));

        if (tmp_call_result_177 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5890;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_177);
    }
    {
        PyObject *tmp_called_name_178;
        PyObject *tmp_call_result_178;
        tmp_called_name_178 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_178 == NULL)) {
            tmp_called_name_178 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_178 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5904;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5904;
        tmp_call_result_178 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_178, &PyTuple_GET_ITEM(mod_consts[221], 0));

        if (tmp_call_result_178 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5904;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_178);
    }
    {
        PyObject *tmp_called_name_179;
        PyObject *tmp_call_result_179;
        tmp_called_name_179 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_179 == NULL)) {
            tmp_called_name_179 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_179 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5916;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5916;
        tmp_call_result_179 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_179, &PyTuple_GET_ITEM(mod_consts[222], 0));

        if (tmp_call_result_179 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5916;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_179);
    }
    {
        PyObject *tmp_called_name_180;
        PyObject *tmp_call_result_180;
        tmp_called_name_180 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_180 == NULL)) {
            tmp_called_name_180 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_180 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5928;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5928;
        tmp_call_result_180 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_180, &PyTuple_GET_ITEM(mod_consts[223], 0));

        if (tmp_call_result_180 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5928;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_180);
    }
    {
        PyObject *tmp_called_name_181;
        PyObject *tmp_call_result_181;
        tmp_called_name_181 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_181 == NULL)) {
            tmp_called_name_181 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_181 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5931;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5931;
        tmp_call_result_181 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_181, &PyTuple_GET_ITEM(mod_consts[224], 0));

        if (tmp_call_result_181 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5931;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_181);
    }
    {
        PyObject *tmp_called_name_182;
        PyObject *tmp_call_result_182;
        tmp_called_name_182 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_182 == NULL)) {
            tmp_called_name_182 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_182 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5934;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5934;
        tmp_call_result_182 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_182, &PyTuple_GET_ITEM(mod_consts[225], 0));

        if (tmp_call_result_182 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5934;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_182);
    }
    {
        PyObject *tmp_called_name_183;
        PyObject *tmp_call_result_183;
        tmp_called_name_183 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_183 == NULL)) {
            tmp_called_name_183 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_183 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5937;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5937;
        tmp_call_result_183 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_183, &PyTuple_GET_ITEM(mod_consts[226], 0));

        if (tmp_call_result_183 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5937;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_183);
    }
    {
        PyObject *tmp_called_name_184;
        PyObject *tmp_call_result_184;
        tmp_called_name_184 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_184 == NULL)) {
            tmp_called_name_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_184 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5940;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5940;
        tmp_call_result_184 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_184, &PyTuple_GET_ITEM(mod_consts[227], 0));

        if (tmp_call_result_184 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5940;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_184);
    }
    {
        PyObject *tmp_called_name_185;
        PyObject *tmp_call_result_185;
        tmp_called_name_185 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_185 == NULL)) {
            tmp_called_name_185 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_185 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5943;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5943;
        tmp_call_result_185 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_185, &PyTuple_GET_ITEM(mod_consts[228], 0));

        if (tmp_call_result_185 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5943;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_185);
    }
    {
        PyObject *tmp_called_name_186;
        PyObject *tmp_call_result_186;
        tmp_called_name_186 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_186 == NULL)) {
            tmp_called_name_186 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_186 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5946;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5946;
        tmp_call_result_186 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_186, &PyTuple_GET_ITEM(mod_consts[229], 0));

        if (tmp_call_result_186 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5946;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_186);
    }
    {
        PyObject *tmp_called_name_187;
        PyObject *tmp_call_result_187;
        tmp_called_name_187 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_187 == NULL)) {
            tmp_called_name_187 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_187 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5949;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5949;
        tmp_call_result_187 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_187, &PyTuple_GET_ITEM(mod_consts[230], 0));

        if (tmp_call_result_187 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5949;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_187);
    }
    {
        PyObject *tmp_called_name_188;
        PyObject *tmp_call_result_188;
        tmp_called_name_188 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_188 == NULL)) {
            tmp_called_name_188 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_188 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5952;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5952;
        tmp_call_result_188 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_188, &PyTuple_GET_ITEM(mod_consts[231], 0));

        if (tmp_call_result_188 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5952;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_188);
    }
    {
        PyObject *tmp_called_name_189;
        PyObject *tmp_call_result_189;
        tmp_called_name_189 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_189 == NULL)) {
            tmp_called_name_189 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_189 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5955;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5955;
        tmp_call_result_189 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_189, &PyTuple_GET_ITEM(mod_consts[232], 0));

        if (tmp_call_result_189 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5955;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_189);
    }
    {
        PyObject *tmp_called_name_190;
        PyObject *tmp_call_result_190;
        tmp_called_name_190 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_190 == NULL)) {
            tmp_called_name_190 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_190 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5958;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5958;
        tmp_call_result_190 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_190, &PyTuple_GET_ITEM(mod_consts[233], 0));

        if (tmp_call_result_190 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5958;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_190);
    }
    {
        PyObject *tmp_called_name_191;
        PyObject *tmp_call_result_191;
        tmp_called_name_191 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_191 == NULL)) {
            tmp_called_name_191 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_191 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5961;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5961;
        tmp_call_result_191 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_191, &PyTuple_GET_ITEM(mod_consts[234], 0));

        if (tmp_call_result_191 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5961;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_191);
    }
    {
        PyObject *tmp_called_name_192;
        PyObject *tmp_call_result_192;
        tmp_called_name_192 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_192 == NULL)) {
            tmp_called_name_192 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_192 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5966;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5966;
        tmp_call_result_192 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_192, &PyTuple_GET_ITEM(mod_consts[235], 0));

        if (tmp_call_result_192 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5966;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_192);
    }
    {
        PyObject *tmp_called_name_193;
        PyObject *tmp_call_result_193;
        tmp_called_name_193 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_193 == NULL)) {
            tmp_called_name_193 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_193 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5978;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5978;
        tmp_call_result_193 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_193, &PyTuple_GET_ITEM(mod_consts[236], 0));

        if (tmp_call_result_193 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5978;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_193);
    }
    {
        PyObject *tmp_called_name_194;
        PyObject *tmp_call_result_194;
        tmp_called_name_194 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_194 == NULL)) {
            tmp_called_name_194 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_194 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5990;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 5990;
        tmp_call_result_194 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_194, &PyTuple_GET_ITEM(mod_consts[237], 0));

        if (tmp_call_result_194 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 5990;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_194);
    }
    {
        PyObject *tmp_called_name_195;
        PyObject *tmp_call_result_195;
        tmp_called_name_195 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_195 == NULL)) {
            tmp_called_name_195 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_195 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6002;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6002;
        tmp_call_result_195 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_195, &PyTuple_GET_ITEM(mod_consts[238], 0));

        if (tmp_call_result_195 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6002;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_195);
    }
    {
        PyObject *tmp_called_name_196;
        PyObject *tmp_call_result_196;
        tmp_called_name_196 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_196 == NULL)) {
            tmp_called_name_196 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_196 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6014;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6014;
        tmp_call_result_196 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_196, &PyTuple_GET_ITEM(mod_consts[239], 0));

        if (tmp_call_result_196 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6014;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_196);
    }
    {
        PyObject *tmp_called_name_197;
        PyObject *tmp_call_result_197;
        tmp_called_name_197 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_197 == NULL)) {
            tmp_called_name_197 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_197 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6026;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6026;
        tmp_call_result_197 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_197, &PyTuple_GET_ITEM(mod_consts[240], 0));

        if (tmp_call_result_197 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6026;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_197);
    }
    {
        PyObject *tmp_called_name_198;
        PyObject *tmp_call_result_198;
        tmp_called_name_198 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_198 == NULL)) {
            tmp_called_name_198 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_198 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6038;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6038;
        tmp_call_result_198 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_198, &PyTuple_GET_ITEM(mod_consts[241], 0));

        if (tmp_call_result_198 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6038;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_198);
    }
    {
        PyObject *tmp_called_name_199;
        PyObject *tmp_call_result_199;
        tmp_called_name_199 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_199 == NULL)) {
            tmp_called_name_199 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_199 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6050;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6050;
        tmp_call_result_199 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_199, &PyTuple_GET_ITEM(mod_consts[242], 0));

        if (tmp_call_result_199 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6050;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_199);
    }
    {
        PyObject *tmp_called_name_200;
        PyObject *tmp_call_result_200;
        tmp_called_name_200 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_200 == NULL)) {
            tmp_called_name_200 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_200 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6062;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6062;
        tmp_call_result_200 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_200, &PyTuple_GET_ITEM(mod_consts[243], 0));

        if (tmp_call_result_200 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6062;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_200);
    }
    {
        PyObject *tmp_called_name_201;
        PyObject *tmp_call_result_201;
        tmp_called_name_201 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_201 == NULL)) {
            tmp_called_name_201 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_201 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6074;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6074;
        tmp_call_result_201 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_201, &PyTuple_GET_ITEM(mod_consts[244], 0));

        if (tmp_call_result_201 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6074;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_201);
    }
    {
        PyObject *tmp_called_name_202;
        PyObject *tmp_call_result_202;
        tmp_called_name_202 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_202 == NULL)) {
            tmp_called_name_202 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_202 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6086;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6086;
        tmp_call_result_202 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_202, &PyTuple_GET_ITEM(mod_consts[245], 0));

        if (tmp_call_result_202 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6086;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_202);
    }
    {
        PyObject *tmp_called_name_203;
        PyObject *tmp_call_result_203;
        tmp_called_name_203 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_203 == NULL)) {
            tmp_called_name_203 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_203 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6098;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6098;
        tmp_call_result_203 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_203, &PyTuple_GET_ITEM(mod_consts[246], 0));

        if (tmp_call_result_203 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6098;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_203);
    }
    {
        PyObject *tmp_called_name_204;
        PyObject *tmp_call_result_204;
        tmp_called_name_204 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_204 == NULL)) {
            tmp_called_name_204 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_204 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6110;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6110;
        tmp_call_result_204 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_204, &PyTuple_GET_ITEM(mod_consts[247], 0));

        if (tmp_call_result_204 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6110;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_204);
    }
    {
        PyObject *tmp_called_name_205;
        PyObject *tmp_call_result_205;
        tmp_called_name_205 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_205 == NULL)) {
            tmp_called_name_205 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_205 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6122;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6122;
        tmp_call_result_205 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_205, &PyTuple_GET_ITEM(mod_consts[248], 0));

        if (tmp_call_result_205 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6122;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_205);
    }
    {
        PyObject *tmp_called_name_206;
        PyObject *tmp_call_result_206;
        tmp_called_name_206 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_206 == NULL)) {
            tmp_called_name_206 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_206 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6134;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6134;
        tmp_call_result_206 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_206, &PyTuple_GET_ITEM(mod_consts[249], 0));

        if (tmp_call_result_206 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6134;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_206);
    }
    {
        PyObject *tmp_called_name_207;
        PyObject *tmp_call_result_207;
        tmp_called_name_207 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_207 == NULL)) {
            tmp_called_name_207 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_207 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6146;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6146;
        tmp_call_result_207 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_207, &PyTuple_GET_ITEM(mod_consts[250], 0));

        if (tmp_call_result_207 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6146;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_207);
    }
    {
        PyObject *tmp_called_name_208;
        PyObject *tmp_call_result_208;
        tmp_called_name_208 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_208 == NULL)) {
            tmp_called_name_208 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_208 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6158;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6158;
        tmp_call_result_208 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_208, &PyTuple_GET_ITEM(mod_consts[251], 0));

        if (tmp_call_result_208 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6158;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_208);
    }
    {
        PyObject *tmp_called_name_209;
        PyObject *tmp_call_result_209;
        tmp_called_name_209 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_209 == NULL)) {
            tmp_called_name_209 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_209 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6170;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6170;
        tmp_call_result_209 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_209, &PyTuple_GET_ITEM(mod_consts[252], 0));

        if (tmp_call_result_209 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6170;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_209);
    }
    {
        PyObject *tmp_called_name_210;
        PyObject *tmp_call_result_210;
        tmp_called_name_210 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_210 == NULL)) {
            tmp_called_name_210 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_210 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6182;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6182;
        tmp_call_result_210 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_210, &PyTuple_GET_ITEM(mod_consts[253], 0));

        if (tmp_call_result_210 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6182;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_210);
    }
    {
        PyObject *tmp_called_name_211;
        PyObject *tmp_call_result_211;
        tmp_called_name_211 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_211 == NULL)) {
            tmp_called_name_211 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_211 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6194;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6194;
        tmp_call_result_211 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_211, &PyTuple_GET_ITEM(mod_consts[254], 0));

        if (tmp_call_result_211 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6194;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_211);
    }
    {
        PyObject *tmp_called_name_212;
        PyObject *tmp_call_result_212;
        tmp_called_name_212 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_212 == NULL)) {
            tmp_called_name_212 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_212 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6206;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6206;
        tmp_call_result_212 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_212, &PyTuple_GET_ITEM(mod_consts[255], 0));

        if (tmp_call_result_212 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6206;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_212);
    }
    {
        PyObject *tmp_called_name_213;
        PyObject *tmp_call_result_213;
        tmp_called_name_213 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_213 == NULL)) {
            tmp_called_name_213 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_213 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6218;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6218;
        tmp_call_result_213 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_213, &PyTuple_GET_ITEM(mod_consts[256], 0));

        if (tmp_call_result_213 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6218;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_213);
    }
    {
        PyObject *tmp_called_name_214;
        PyObject *tmp_call_result_214;
        tmp_called_name_214 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_214 == NULL)) {
            tmp_called_name_214 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_214 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6230;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6230;
        tmp_call_result_214 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_214, &PyTuple_GET_ITEM(mod_consts[257], 0));

        if (tmp_call_result_214 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6230;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_214);
    }
    {
        PyObject *tmp_called_name_215;
        PyObject *tmp_call_result_215;
        tmp_called_name_215 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_215 == NULL)) {
            tmp_called_name_215 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_215 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6242;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6242;
        tmp_call_result_215 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_215, &PyTuple_GET_ITEM(mod_consts[258], 0));

        if (tmp_call_result_215 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6242;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_215);
    }
    {
        PyObject *tmp_called_name_216;
        PyObject *tmp_call_result_216;
        tmp_called_name_216 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_216 == NULL)) {
            tmp_called_name_216 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_216 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6254;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6254;
        tmp_call_result_216 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_216, &PyTuple_GET_ITEM(mod_consts[259], 0));

        if (tmp_call_result_216 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6254;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_216);
    }
    {
        PyObject *tmp_called_name_217;
        PyObject *tmp_call_result_217;
        tmp_called_name_217 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_217 == NULL)) {
            tmp_called_name_217 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_217 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6266;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6266;
        tmp_call_result_217 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_217, &PyTuple_GET_ITEM(mod_consts[260], 0));

        if (tmp_call_result_217 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6266;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_217);
    }
    {
        PyObject *tmp_called_name_218;
        PyObject *tmp_call_result_218;
        tmp_called_name_218 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_218 == NULL)) {
            tmp_called_name_218 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_218 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6299;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6299;
        tmp_call_result_218 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_218, &PyTuple_GET_ITEM(mod_consts[261], 0));

        if (tmp_call_result_218 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6299;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_218);
    }
    {
        PyObject *tmp_called_name_219;
        PyObject *tmp_call_result_219;
        tmp_called_name_219 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_219 == NULL)) {
            tmp_called_name_219 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_219 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6311;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6311;
        tmp_call_result_219 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_219, &PyTuple_GET_ITEM(mod_consts[262], 0));

        if (tmp_call_result_219 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6311;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_219);
    }
    {
        PyObject *tmp_called_name_220;
        PyObject *tmp_call_result_220;
        tmp_called_name_220 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_220 == NULL)) {
            tmp_called_name_220 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_220 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6323;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6323;
        tmp_call_result_220 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_220, &PyTuple_GET_ITEM(mod_consts[263], 0));

        if (tmp_call_result_220 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6323;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_220);
    }
    {
        PyObject *tmp_called_name_221;
        PyObject *tmp_call_result_221;
        tmp_called_name_221 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_221 == NULL)) {
            tmp_called_name_221 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_221 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6335;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6335;
        tmp_call_result_221 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_221, &PyTuple_GET_ITEM(mod_consts[264], 0));

        if (tmp_call_result_221 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6335;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_221);
    }
    {
        PyObject *tmp_called_name_222;
        PyObject *tmp_call_result_222;
        tmp_called_name_222 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_222 == NULL)) {
            tmp_called_name_222 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_222 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6347;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6347;
        tmp_call_result_222 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_222, &PyTuple_GET_ITEM(mod_consts[265], 0));

        if (tmp_call_result_222 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6347;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_222);
    }
    {
        PyObject *tmp_called_name_223;
        PyObject *tmp_call_result_223;
        tmp_called_name_223 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_223 == NULL)) {
            tmp_called_name_223 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_223 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6359;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6359;
        tmp_call_result_223 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_223, &PyTuple_GET_ITEM(mod_consts[266], 0));

        if (tmp_call_result_223 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6359;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_223);
    }
    {
        PyObject *tmp_called_name_224;
        PyObject *tmp_call_result_224;
        tmp_called_name_224 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_224 == NULL)) {
            tmp_called_name_224 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_224 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6371;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6371;
        tmp_call_result_224 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_224, &PyTuple_GET_ITEM(mod_consts[267], 0));

        if (tmp_call_result_224 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6371;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_224);
    }
    {
        PyObject *tmp_called_name_225;
        PyObject *tmp_call_result_225;
        tmp_called_name_225 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_225 == NULL)) {
            tmp_called_name_225 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_225 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6383;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6383;
        tmp_call_result_225 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_225, &PyTuple_GET_ITEM(mod_consts[268], 0));

        if (tmp_call_result_225 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6383;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_225);
    }
    {
        PyObject *tmp_called_name_226;
        PyObject *tmp_call_result_226;
        tmp_called_name_226 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_226 == NULL)) {
            tmp_called_name_226 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_226 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6395;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6395;
        tmp_call_result_226 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_226, &PyTuple_GET_ITEM(mod_consts[269], 0));

        if (tmp_call_result_226 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6395;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_226);
    }
    {
        PyObject *tmp_called_name_227;
        PyObject *tmp_call_result_227;
        tmp_called_name_227 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_227 == NULL)) {
            tmp_called_name_227 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_227 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6407;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6407;
        tmp_call_result_227 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_227, &PyTuple_GET_ITEM(mod_consts[270], 0));

        if (tmp_call_result_227 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6407;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_227);
    }
    {
        PyObject *tmp_called_name_228;
        PyObject *tmp_call_result_228;
        tmp_called_name_228 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_228 == NULL)) {
            tmp_called_name_228 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_228 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6419;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6419;
        tmp_call_result_228 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_228, &PyTuple_GET_ITEM(mod_consts[271], 0));

        if (tmp_call_result_228 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6419;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_228);
    }
    {
        PyObject *tmp_called_name_229;
        PyObject *tmp_call_result_229;
        tmp_called_name_229 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_229 == NULL)) {
            tmp_called_name_229 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_229 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6431;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6431;
        tmp_call_result_229 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_229, &PyTuple_GET_ITEM(mod_consts[272], 0));

        if (tmp_call_result_229 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6431;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_229);
    }
    {
        PyObject *tmp_called_name_230;
        PyObject *tmp_call_result_230;
        tmp_called_name_230 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_230 == NULL)) {
            tmp_called_name_230 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_230 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6443;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6443;
        tmp_call_result_230 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_230, &PyTuple_GET_ITEM(mod_consts[273], 0));

        if (tmp_call_result_230 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6443;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_230);
    }
    {
        PyObject *tmp_called_name_231;
        PyObject *tmp_call_result_231;
        tmp_called_name_231 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_231 == NULL)) {
            tmp_called_name_231 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_231 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6455;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6455;
        tmp_call_result_231 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_231, &PyTuple_GET_ITEM(mod_consts[274], 0));

        if (tmp_call_result_231 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6455;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_231);
    }
    {
        PyObject *tmp_called_name_232;
        PyObject *tmp_call_result_232;
        tmp_called_name_232 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_232 == NULL)) {
            tmp_called_name_232 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_232 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6467;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6467;
        tmp_call_result_232 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_232, &PyTuple_GET_ITEM(mod_consts[275], 0));

        if (tmp_call_result_232 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6467;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_232);
    }
    {
        PyObject *tmp_called_name_233;
        PyObject *tmp_call_result_233;
        tmp_called_name_233 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_233 == NULL)) {
            tmp_called_name_233 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_233 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6479;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6479;
        tmp_call_result_233 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_233, &PyTuple_GET_ITEM(mod_consts[276], 0));

        if (tmp_call_result_233 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6479;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_233);
    }
    {
        PyObject *tmp_called_name_234;
        PyObject *tmp_call_result_234;
        tmp_called_name_234 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_234 == NULL)) {
            tmp_called_name_234 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_234 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6491;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6491;
        tmp_call_result_234 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_234, &PyTuple_GET_ITEM(mod_consts[277], 0));

        if (tmp_call_result_234 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6491;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_234);
    }
    {
        PyObject *tmp_called_name_235;
        PyObject *tmp_call_result_235;
        tmp_called_name_235 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_235 == NULL)) {
            tmp_called_name_235 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_235 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6503;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6503;
        tmp_call_result_235 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_235, &PyTuple_GET_ITEM(mod_consts[278], 0));

        if (tmp_call_result_235 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6503;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_235);
    }
    {
        PyObject *tmp_called_name_236;
        PyObject *tmp_call_result_236;
        tmp_called_name_236 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_236 == NULL)) {
            tmp_called_name_236 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_236 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6515;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6515;
        tmp_call_result_236 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_236, &PyTuple_GET_ITEM(mod_consts[279], 0));

        if (tmp_call_result_236 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6515;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_236);
    }
    {
        PyObject *tmp_called_name_237;
        PyObject *tmp_call_result_237;
        tmp_called_name_237 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_237 == NULL)) {
            tmp_called_name_237 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_237 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6527;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6527;
        tmp_call_result_237 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_237, &PyTuple_GET_ITEM(mod_consts[280], 0));

        if (tmp_call_result_237 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6527;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_237);
    }
    {
        PyObject *tmp_called_name_238;
        PyObject *tmp_call_result_238;
        tmp_called_name_238 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_238 == NULL)) {
            tmp_called_name_238 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_238 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6539;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6539;
        tmp_call_result_238 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_238, &PyTuple_GET_ITEM(mod_consts[281], 0));

        if (tmp_call_result_238 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6539;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_238);
    }
    {
        PyObject *tmp_called_name_239;
        PyObject *tmp_call_result_239;
        tmp_called_name_239 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_239 == NULL)) {
            tmp_called_name_239 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_239 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6551;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6551;
        tmp_call_result_239 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_239, &PyTuple_GET_ITEM(mod_consts[282], 0));

        if (tmp_call_result_239 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6551;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_239);
    }
    {
        PyObject *tmp_called_name_240;
        PyObject *tmp_call_result_240;
        tmp_called_name_240 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_240 == NULL)) {
            tmp_called_name_240 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_240 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6563;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6563;
        tmp_call_result_240 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_240, &PyTuple_GET_ITEM(mod_consts[283], 0));

        if (tmp_call_result_240 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6563;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_240);
    }
    {
        PyObject *tmp_called_name_241;
        PyObject *tmp_call_result_241;
        tmp_called_name_241 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_241 == NULL)) {
            tmp_called_name_241 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_241 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6575;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6575;
        tmp_call_result_241 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_241, &PyTuple_GET_ITEM(mod_consts[284], 0));

        if (tmp_call_result_241 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6575;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_241);
    }
    {
        PyObject *tmp_called_name_242;
        PyObject *tmp_call_result_242;
        tmp_called_name_242 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_242 == NULL)) {
            tmp_called_name_242 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_242 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6587;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6587;
        tmp_call_result_242 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_242, &PyTuple_GET_ITEM(mod_consts[285], 0));

        if (tmp_call_result_242 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6587;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_242);
    }
    {
        PyObject *tmp_called_name_243;
        PyObject *tmp_call_result_243;
        tmp_called_name_243 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_243 == NULL)) {
            tmp_called_name_243 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_243 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6607;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6607;
        tmp_call_result_243 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_243, &PyTuple_GET_ITEM(mod_consts[286], 0));

        if (tmp_call_result_243 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6607;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_243);
    }
    {
        PyObject *tmp_called_name_244;
        PyObject *tmp_call_result_244;
        tmp_called_name_244 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_244 == NULL)) {
            tmp_called_name_244 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_244 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6613;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6613;
        tmp_call_result_244 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_244, &PyTuple_GET_ITEM(mod_consts[287], 0));

        if (tmp_call_result_244 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6613;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_244);
    }
    {
        PyObject *tmp_called_name_245;
        PyObject *tmp_call_result_245;
        tmp_called_name_245 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_245 == NULL)) {
            tmp_called_name_245 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_245 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6619;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6619;
        tmp_call_result_245 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_245, &PyTuple_GET_ITEM(mod_consts[288], 0));

        if (tmp_call_result_245 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6619;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_245);
    }
    {
        PyObject *tmp_called_name_246;
        PyObject *tmp_call_result_246;
        tmp_called_name_246 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_246 == NULL)) {
            tmp_called_name_246 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_246 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6625;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6625;
        tmp_call_result_246 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_246, &PyTuple_GET_ITEM(mod_consts[289], 0));

        if (tmp_call_result_246 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6625;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_246);
    }
    {
        PyObject *tmp_called_name_247;
        PyObject *tmp_call_result_247;
        tmp_called_name_247 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_247 == NULL)) {
            tmp_called_name_247 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_247 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6631;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6631;
        tmp_call_result_247 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_247, &PyTuple_GET_ITEM(mod_consts[290], 0));

        if (tmp_call_result_247 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6631;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_247);
    }
    {
        PyObject *tmp_called_name_248;
        PyObject *tmp_call_result_248;
        tmp_called_name_248 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_248 == NULL)) {
            tmp_called_name_248 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_248 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6639;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6639;
        tmp_call_result_248 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_248, &PyTuple_GET_ITEM(mod_consts[291], 0));

        if (tmp_call_result_248 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6639;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_248);
    }
    {
        PyObject *tmp_called_name_249;
        PyObject *tmp_call_result_249;
        tmp_called_name_249 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_249 == NULL)) {
            tmp_called_name_249 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_249 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6645;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6645;
        tmp_call_result_249 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_249, &PyTuple_GET_ITEM(mod_consts[292], 0));

        if (tmp_call_result_249 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6645;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_249);
    }
    {
        PyObject *tmp_called_name_250;
        PyObject *tmp_call_result_250;
        tmp_called_name_250 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_250 == NULL)) {
            tmp_called_name_250 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_250 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6652;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6652;
        tmp_call_result_250 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_250, &PyTuple_GET_ITEM(mod_consts[293], 0));

        if (tmp_call_result_250 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6652;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_250);
    }
    {
        PyObject *tmp_called_name_251;
        PyObject *tmp_call_result_251;
        tmp_called_name_251 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_251 == NULL)) {
            tmp_called_name_251 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_251 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6660;

            goto frame_exception_exit_1;
        }
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6660;
        tmp_call_result_251 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_251, &PyTuple_GET_ITEM(mod_consts[294], 0));

        if (tmp_call_result_251 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6660;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_251);
    }
    {
        PyObject *tmp_assign_source_7;


        tmp_assign_source_7 = MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__1_numeric_type_aliases();

        UPDATE_STRING_DICT1(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[295], tmp_assign_source_7);
    }
    {
        PyObject *tmp_assign_source_8;
        PyObject *tmp_called_name_252;
        PyObject *tmp_call_arg_element_1;
        tmp_called_name_252 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[295]);

        if (unlikely(tmp_called_name_252 == NULL)) {
            tmp_called_name_252 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[295]);
        }

        assert(!(tmp_called_name_252 == NULL));
        tmp_call_arg_element_1 = LIST_COPY(mod_consts[296]);
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6686;
        tmp_assign_source_8 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_252, tmp_call_arg_element_1);
        Py_DECREF(tmp_call_arg_element_1);
        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6686;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[14], tmp_assign_source_8);
    }
    {
        PyObject *tmp_assign_source_9;


        tmp_assign_source_9 = MAKE_FUNCTION_numpy$core$_add_newdocs$$$function__2_add_newdoc_for_scalar_type();

        UPDATE_STRING_DICT1(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297], tmp_assign_source_9);
    }
    {
        PyObject *tmp_called_name_253;
        PyObject *tmp_call_result_252;
        PyObject *tmp_call_arg_element_2;
        PyObject *tmp_call_arg_element_3;
        PyObject *tmp_call_arg_element_4;
        tmp_called_name_253 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_253 == NULL)) {
            tmp_called_name_253 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        assert(!(tmp_called_name_253 == NULL));
        tmp_call_arg_element_2 = mod_consts[298];
        tmp_call_arg_element_3 = LIST_COPY(mod_consts[299]);
        tmp_call_arg_element_4 = mod_consts[300];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6728;
        {
            PyObject *call_args[] = {tmp_call_arg_element_2, tmp_call_arg_element_3, tmp_call_arg_element_4};
            tmp_call_result_252 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_253, call_args);
        }

        Py_DECREF(tmp_call_arg_element_3);
        if (tmp_call_result_252 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6728;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_252);
    }
    {
        PyObject *tmp_called_name_254;
        PyObject *tmp_call_result_253;
        PyObject *tmp_call_arg_element_5;
        PyObject *tmp_call_arg_element_6;
        PyObject *tmp_call_arg_element_7;
        tmp_called_name_254 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_254 == NULL)) {
            tmp_called_name_254 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_254 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6733;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_5 = mod_consts[301];
        tmp_call_arg_element_6 = PyList_New(0);
        tmp_call_arg_element_7 = mod_consts[302];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6733;
        {
            PyObject *call_args[] = {tmp_call_arg_element_5, tmp_call_arg_element_6, tmp_call_arg_element_7};
            tmp_call_result_253 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_254, call_args);
        }

        Py_DECREF(tmp_call_arg_element_6);
        if (tmp_call_result_253 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6733;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_253);
    }
    {
        PyObject *tmp_called_name_255;
        PyObject *tmp_call_result_254;
        PyObject *tmp_call_arg_element_8;
        PyObject *tmp_call_arg_element_9;
        PyObject *tmp_call_arg_element_10;
        tmp_called_name_255 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_255 == NULL)) {
            tmp_called_name_255 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_255 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6738;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_8 = mod_consts[303];
        tmp_call_arg_element_9 = PyList_New(0);
        tmp_call_arg_element_10 = mod_consts[304];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6738;
        {
            PyObject *call_args[] = {tmp_call_arg_element_8, tmp_call_arg_element_9, tmp_call_arg_element_10};
            tmp_call_result_254 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_255, call_args);
        }

        Py_DECREF(tmp_call_arg_element_9);
        if (tmp_call_result_254 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6738;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_254);
    }
    {
        PyObject *tmp_called_name_256;
        PyObject *tmp_call_result_255;
        PyObject *tmp_call_arg_element_11;
        PyObject *tmp_call_arg_element_12;
        PyObject *tmp_call_arg_element_13;
        tmp_called_name_256 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_256 == NULL)) {
            tmp_called_name_256 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_256 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6743;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_11 = mod_consts[305];
        tmp_call_arg_element_12 = PyList_New(0);
        tmp_call_arg_element_13 = mod_consts[306];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6743;
        {
            PyObject *call_args[] = {tmp_call_arg_element_11, tmp_call_arg_element_12, tmp_call_arg_element_13};
            tmp_call_result_255 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_256, call_args);
        }

        Py_DECREF(tmp_call_arg_element_12);
        if (tmp_call_result_255 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6743;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_255);
    }
    {
        PyObject *tmp_called_name_257;
        PyObject *tmp_call_result_256;
        PyObject *tmp_call_arg_element_14;
        PyObject *tmp_call_arg_element_15;
        PyObject *tmp_call_arg_element_16;
        tmp_called_name_257 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_257 == NULL)) {
            tmp_called_name_257 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_257 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6748;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_14 = mod_consts[307];
        tmp_call_arg_element_15 = PyList_New(0);
        tmp_call_arg_element_16 = mod_consts[308];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6748;
        {
            PyObject *call_args[] = {tmp_call_arg_element_14, tmp_call_arg_element_15, tmp_call_arg_element_16};
            tmp_call_result_256 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_257, call_args);
        }

        Py_DECREF(tmp_call_arg_element_15);
        if (tmp_call_result_256 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6748;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_256);
    }
    {
        PyObject *tmp_called_name_258;
        PyObject *tmp_call_result_257;
        PyObject *tmp_call_arg_element_17;
        PyObject *tmp_call_arg_element_18;
        PyObject *tmp_call_arg_element_19;
        tmp_called_name_258 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_258 == NULL)) {
            tmp_called_name_258 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_258 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6753;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_17 = mod_consts[309];
        tmp_call_arg_element_18 = PyList_New(0);
        tmp_call_arg_element_19 = mod_consts[310];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6753;
        {
            PyObject *call_args[] = {tmp_call_arg_element_17, tmp_call_arg_element_18, tmp_call_arg_element_19};
            tmp_call_result_257 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_258, call_args);
        }

        Py_DECREF(tmp_call_arg_element_18);
        if (tmp_call_result_257 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6753;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_257);
    }
    {
        PyObject *tmp_called_name_259;
        PyObject *tmp_call_result_258;
        PyObject *tmp_call_arg_element_20;
        PyObject *tmp_call_arg_element_21;
        PyObject *tmp_call_arg_element_22;
        tmp_called_name_259 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_259 == NULL)) {
            tmp_called_name_259 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_259 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6758;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_20 = mod_consts[311];
        tmp_call_arg_element_21 = PyList_New(0);
        tmp_call_arg_element_22 = mod_consts[312];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6758;
        {
            PyObject *call_args[] = {tmp_call_arg_element_20, tmp_call_arg_element_21, tmp_call_arg_element_22};
            tmp_call_result_258 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_259, call_args);
        }

        Py_DECREF(tmp_call_arg_element_21);
        if (tmp_call_result_258 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6758;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_258);
    }
    {
        PyObject *tmp_called_name_260;
        PyObject *tmp_call_result_259;
        PyObject *tmp_call_arg_element_23;
        PyObject *tmp_call_arg_element_24;
        PyObject *tmp_call_arg_element_25;
        tmp_called_name_260 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_260 == NULL)) {
            tmp_called_name_260 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_260 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6763;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_23 = mod_consts[313];
        tmp_call_arg_element_24 = PyList_New(0);
        tmp_call_arg_element_25 = mod_consts[314];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6763;
        {
            PyObject *call_args[] = {tmp_call_arg_element_23, tmp_call_arg_element_24, tmp_call_arg_element_25};
            tmp_call_result_259 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_260, call_args);
        }

        Py_DECREF(tmp_call_arg_element_24);
        if (tmp_call_result_259 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6763;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_259);
    }
    {
        PyObject *tmp_called_name_261;
        PyObject *tmp_call_result_260;
        PyObject *tmp_call_arg_element_26;
        PyObject *tmp_call_arg_element_27;
        PyObject *tmp_call_arg_element_28;
        tmp_called_name_261 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_261 == NULL)) {
            tmp_called_name_261 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_261 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6768;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_26 = mod_consts[315];
        tmp_call_arg_element_27 = PyList_New(0);
        tmp_call_arg_element_28 = mod_consts[316];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6768;
        {
            PyObject *call_args[] = {tmp_call_arg_element_26, tmp_call_arg_element_27, tmp_call_arg_element_28};
            tmp_call_result_260 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_261, call_args);
        }

        Py_DECREF(tmp_call_arg_element_27);
        if (tmp_call_result_260 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6768;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_260);
    }
    {
        PyObject *tmp_called_name_262;
        PyObject *tmp_call_result_261;
        PyObject *tmp_call_arg_element_29;
        PyObject *tmp_call_arg_element_30;
        PyObject *tmp_call_arg_element_31;
        tmp_called_name_262 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_262 == NULL)) {
            tmp_called_name_262 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_262 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6773;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_29 = mod_consts[317];
        tmp_call_arg_element_30 = PyList_New(0);
        tmp_call_arg_element_31 = mod_consts[318];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6773;
        {
            PyObject *call_args[] = {tmp_call_arg_element_29, tmp_call_arg_element_30, tmp_call_arg_element_31};
            tmp_call_result_261 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_262, call_args);
        }

        Py_DECREF(tmp_call_arg_element_30);
        if (tmp_call_result_261 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6773;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_261);
    }
    {
        PyObject *tmp_called_name_263;
        PyObject *tmp_call_result_262;
        PyObject *tmp_call_arg_element_32;
        PyObject *tmp_call_arg_element_33;
        PyObject *tmp_call_arg_element_34;
        tmp_called_name_263 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_263 == NULL)) {
            tmp_called_name_263 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_263 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6778;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_32 = mod_consts[319];
        tmp_call_arg_element_33 = PyList_New(0);
        tmp_call_arg_element_34 = mod_consts[320];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6778;
        {
            PyObject *call_args[] = {tmp_call_arg_element_32, tmp_call_arg_element_33, tmp_call_arg_element_34};
            tmp_call_result_262 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_263, call_args);
        }

        Py_DECREF(tmp_call_arg_element_33);
        if (tmp_call_result_262 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6778;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_262);
    }
    {
        PyObject *tmp_called_name_264;
        PyObject *tmp_call_result_263;
        PyObject *tmp_call_arg_element_35;
        PyObject *tmp_call_arg_element_36;
        PyObject *tmp_call_arg_element_37;
        tmp_called_name_264 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_264 == NULL)) {
            tmp_called_name_264 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_264 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6783;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_35 = mod_consts[321];
        tmp_call_arg_element_36 = PyList_New(0);
        tmp_call_arg_element_37 = mod_consts[322];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6783;
        {
            PyObject *call_args[] = {tmp_call_arg_element_35, tmp_call_arg_element_36, tmp_call_arg_element_37};
            tmp_call_result_263 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_264, call_args);
        }

        Py_DECREF(tmp_call_arg_element_36);
        if (tmp_call_result_263 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6783;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_263);
    }
    {
        PyObject *tmp_called_name_265;
        PyObject *tmp_call_result_264;
        PyObject *tmp_call_arg_element_38;
        PyObject *tmp_call_arg_element_39;
        PyObject *tmp_call_arg_element_40;
        tmp_called_name_265 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_265 == NULL)) {
            tmp_called_name_265 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_265 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6788;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_38 = mod_consts[323];
        tmp_call_arg_element_39 = PyList_New(0);
        tmp_call_arg_element_40 = mod_consts[324];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6788;
        {
            PyObject *call_args[] = {tmp_call_arg_element_38, tmp_call_arg_element_39, tmp_call_arg_element_40};
            tmp_call_result_264 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_265, call_args);
        }

        Py_DECREF(tmp_call_arg_element_39);
        if (tmp_call_result_264 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6788;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_264);
    }
    {
        PyObject *tmp_called_name_266;
        PyObject *tmp_call_result_265;
        PyObject *tmp_call_arg_element_41;
        PyObject *tmp_call_arg_element_42;
        PyObject *tmp_call_arg_element_43;
        tmp_called_name_266 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_266 == NULL)) {
            tmp_called_name_266 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_266 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6793;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_41 = mod_consts[325];
        tmp_call_arg_element_42 = LIST_COPY(mod_consts[326]);
        tmp_call_arg_element_43 = mod_consts[327];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6793;
        {
            PyObject *call_args[] = {tmp_call_arg_element_41, tmp_call_arg_element_42, tmp_call_arg_element_43};
            tmp_call_result_265 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_266, call_args);
        }

        Py_DECREF(tmp_call_arg_element_42);
        if (tmp_call_result_265 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6793;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_265);
    }
    {
        PyObject *tmp_called_name_267;
        PyObject *tmp_call_result_266;
        PyObject *tmp_call_arg_element_44;
        PyObject *tmp_call_arg_element_45;
        PyObject *tmp_call_arg_element_46;
        tmp_called_name_267 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_267 == NULL)) {
            tmp_called_name_267 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_267 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6799;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_44 = mod_consts[328];
        tmp_call_arg_element_45 = LIST_COPY(mod_consts[329]);
        tmp_call_arg_element_46 = mod_consts[330];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6799;
        {
            PyObject *call_args[] = {tmp_call_arg_element_44, tmp_call_arg_element_45, tmp_call_arg_element_46};
            tmp_call_result_266 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_267, call_args);
        }

        Py_DECREF(tmp_call_arg_element_45);
        if (tmp_call_result_266 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6799;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_266);
    }
    {
        PyObject *tmp_called_name_268;
        PyObject *tmp_call_result_267;
        PyObject *tmp_call_arg_element_47;
        PyObject *tmp_call_arg_element_48;
        PyObject *tmp_call_arg_element_49;
        tmp_called_name_268 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_268 == NULL)) {
            tmp_called_name_268 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_268 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6805;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_47 = mod_consts[331];
        tmp_call_arg_element_48 = LIST_COPY(mod_consts[332]);
        tmp_call_arg_element_49 = mod_consts[333];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6805;
        {
            PyObject *call_args[] = {tmp_call_arg_element_47, tmp_call_arg_element_48, tmp_call_arg_element_49};
            tmp_call_result_267 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_268, call_args);
        }

        Py_DECREF(tmp_call_arg_element_48);
        if (tmp_call_result_267 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6805;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_267);
    }
    {
        PyObject *tmp_called_name_269;
        PyObject *tmp_call_result_268;
        PyObject *tmp_call_arg_element_50;
        PyObject *tmp_call_arg_element_51;
        PyObject *tmp_call_arg_element_52;
        tmp_called_name_269 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_269 == NULL)) {
            tmp_called_name_269 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_269 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6811;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_50 = mod_consts[334];
        tmp_call_arg_element_51 = LIST_COPY(mod_consts[335]);
        tmp_call_arg_element_52 = mod_consts[336];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6811;
        {
            PyObject *call_args[] = {tmp_call_arg_element_50, tmp_call_arg_element_51, tmp_call_arg_element_52};
            tmp_call_result_268 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_269, call_args);
        }

        Py_DECREF(tmp_call_arg_element_51);
        if (tmp_call_result_268 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6811;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_268);
    }
    {
        PyObject *tmp_called_name_270;
        PyObject *tmp_call_result_269;
        PyObject *tmp_call_arg_element_53;
        PyObject *tmp_call_arg_element_54;
        PyObject *tmp_call_arg_element_55;
        tmp_called_name_270 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_270 == NULL)) {
            tmp_called_name_270 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_270 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6817;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_53 = mod_consts[337];
        tmp_call_arg_element_54 = LIST_COPY(mod_consts[338]);
        tmp_call_arg_element_55 = mod_consts[339];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6817;
        {
            PyObject *call_args[] = {tmp_call_arg_element_53, tmp_call_arg_element_54, tmp_call_arg_element_55};
            tmp_call_result_269 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_270, call_args);
        }

        Py_DECREF(tmp_call_arg_element_54);
        if (tmp_call_result_269 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6817;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_269);
    }
    {
        PyObject *tmp_called_name_271;
        PyObject *tmp_call_result_270;
        PyObject *tmp_call_arg_element_56;
        PyObject *tmp_call_arg_element_57;
        PyObject *tmp_call_arg_element_58;
        tmp_called_name_271 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[297]);

        if (unlikely(tmp_called_name_271 == NULL)) {
            tmp_called_name_271 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[297]);
        }

        if (tmp_called_name_271 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6823;

            goto frame_exception_exit_1;
        }
        tmp_call_arg_element_56 = mod_consts[340];
        tmp_call_arg_element_57 = PyList_New(0);
        tmp_call_arg_element_58 = mod_consts[341];
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6823;
        {
            PyObject *call_args[] = {tmp_call_arg_element_56, tmp_call_arg_element_57, tmp_call_arg_element_58};
            tmp_call_result_270 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_271, call_args);
        }

        Py_DECREF(tmp_call_arg_element_57);
        if (tmp_call_result_270 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6823;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_270);
    }
    {
        PyObject *tmp_assign_source_10;
        PyObject *tmp_iter_arg_1;
        tmp_iter_arg_1 = mod_consts[342];
        tmp_assign_source_10 = MAKE_ITERATOR_INFALLIBLE(tmp_iter_arg_1);
        assert(!(tmp_assign_source_10 == NULL));
        assert(tmp_for_loop_1__for_iterator == NULL);
        tmp_for_loop_1__for_iterator = tmp_assign_source_10;
    }
    // Tried code:
    loop_start_1:;
    {
        PyObject *tmp_next_source_1;
        PyObject *tmp_assign_source_11;
        CHECK_OBJECT(tmp_for_loop_1__for_iterator);
        tmp_next_source_1 = tmp_for_loop_1__for_iterator;
        tmp_assign_source_11 = ITERATOR_NEXT(tmp_next_source_1);
        if (tmp_assign_source_11 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_1;
            } else {

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);

                exception_lineno = 6829;
                goto try_except_handler_1;
            }
        }

        {
            PyObject *old = tmp_for_loop_1__iter_value;
            tmp_for_loop_1__iter_value = tmp_assign_source_11;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_12;
        CHECK_OBJECT(tmp_for_loop_1__iter_value);
        tmp_assign_source_12 = tmp_for_loop_1__iter_value;
        UPDATE_STRING_DICT0(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[343], tmp_assign_source_12);
    }
    {
        PyObject *tmp_called_name_272;
        PyObject *tmp_call_result_271;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        PyObject *tmp_tuple_element_1;
        tmp_called_name_272 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[21]);

        if (unlikely(tmp_called_name_272 == NULL)) {
            tmp_called_name_272 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[21]);
        }

        if (tmp_called_name_272 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6830;

            goto try_except_handler_1;
        }
        tmp_args_element_name_1 = mod_consts[22];
        tmp_args_element_name_2 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[343]);

        if (unlikely(tmp_args_element_name_2 == NULL)) {
            tmp_args_element_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[343]);
        }

        assert(!(tmp_args_element_name_2 == NULL));
        tmp_tuple_element_1 = mod_consts[344];
        tmp_args_element_name_3 = PyTuple_New(2);
        {
            PyObject *tmp_called_name_273;
            PyObject *tmp_expression_name_1;
            PyObject *tmp_kwargs_name_1;
            PyObject *tmp_dict_key_1;
            PyObject *tmp_dict_value_1;
            PyTuple_SET_ITEM0(tmp_args_element_name_3, 0, tmp_tuple_element_1);
            tmp_expression_name_1 = mod_consts[345];
            tmp_called_name_273 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[12]);
            assert(!(tmp_called_name_273 == NULL));
            tmp_dict_key_1 = mod_consts[346];
            tmp_dict_value_1 = GET_STRING_DICT_VALUE(moduledict_numpy$core$_add_newdocs, (Nuitka_StringObject *)mod_consts[343]);

            if (unlikely(tmp_dict_value_1 == NULL)) {
                tmp_dict_value_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[343]);
            }

            assert(!(tmp_dict_value_1 == NULL));
            tmp_kwargs_name_1 = _PyDict_NewPresized( 1 );
            tmp_res = PyDict_SetItem(tmp_kwargs_name_1, tmp_dict_key_1, tmp_dict_value_1);
            assert(!(tmp_res != 0));
            frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6844;
            tmp_tuple_element_1 = CALL_FUNCTION_WITH_KEYARGS(tmp_called_name_273, tmp_kwargs_name_1);
            Py_DECREF(tmp_called_name_273);
            Py_DECREF(tmp_kwargs_name_1);
            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 6844;

                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_args_element_name_3, 1, tmp_tuple_element_1);
        }
        goto tuple_build_noexception_1;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_args_element_name_3);
        goto try_except_handler_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_1:;
        frame_837e186af4984df94dc0b36608112861->m_frame.f_lineno = 6830;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2, tmp_args_element_name_3};
            tmp_call_result_271 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_272, call_args);
        }

        Py_DECREF(tmp_args_element_name_3);
        if (tmp_call_result_271 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 6830;

            goto try_except_handler_1;
        }
        Py_DECREF(tmp_call_result_271);
    }
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 6829;

        goto try_except_handler_1;
    }
    goto loop_start_1;
    loop_end_1:;
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

    Py_XDECREF(tmp_for_loop_1__iter_value);
    tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_1__for_iterator);
    tmp_for_loop_1__for_iterator = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

    goto frame_exception_exit_1;
    // End of try:
    try_end_1:;

    // Restore frame exception if necessary.
#if 0
    RESTORE_FRAME_EXCEPTION(frame_837e186af4984df94dc0b36608112861);
#endif
    popFrameStack();

    assertFrameObject(frame_837e186af4984df94dc0b36608112861);

    goto frame_no_exception_1;

    frame_exception_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_837e186af4984df94dc0b36608112861);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_837e186af4984df94dc0b36608112861, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_837e186af4984df94dc0b36608112861->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_837e186af4984df94dc0b36608112861, exception_lineno);
    }

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto module_exception_exit;

    frame_no_exception_1:;
    Py_XDECREF(tmp_for_loop_1__iter_value);
    tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_1__for_iterator);
    tmp_for_loop_1__for_iterator = NULL;

    return module_numpy$core$_add_newdocs;
    module_exception_exit:
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);
    return NULL;
}

