/* Generated code for Python module 'pip._vendor.html5lib.filters.sanitizer'
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

/* The "module_pip$_vendor$html5lib$filters$sanitizer" is a Python object pointer of module type.
 *
 * Note: For full compatibility with CPython, every module variable access
 * needs to go through it except for cases where the module cannot possibly
 * have changed in the mean time.
 */

PyObject *module_pip$_vendor$html5lib$filters$sanitizer;
PyDictObject *moduledict_pip$_vendor$html5lib$filters$sanitizer;

/* The declarations of module constants used, if any. */
static PyObject *mod_consts[602];

static PyObject *module_filename_obj = NULL;

/* Indicator if this modules private constants were created yet. */
static bool constants_created = false;

/* Function to create module private constants. */
static void createModuleConstants(void) {
    if (constants_created == false) {
        loadConstantsBlob(&mod_consts[0], UNTRANSLATE("pip._vendor.html5lib.filters.sanitizer"));
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
void checkModuleConstants_pip$_vendor$html5lib$filters$sanitizer(void) {
    // The module may not have been used at all, then ignore this.
    if (constants_created == false) return;

    checkConstantsBlob(&mod_consts[0], "pip._vendor.html5lib.filters.sanitizer");
}
#endif

// The module code objects.
static PyCodeObject *codeobj_6bde854d712dad430c9ea5c5ac22e83b;
static PyCodeObject *codeobj_4dc3aedd5f581d86900fc22db4cf5702;
static PyCodeObject *codeobj_097fd85308f32255fc725ef557550425;
static PyCodeObject *codeobj_5851b1f734f18da6d819ad9bb56555dd;
static PyCodeObject *codeobj_fde2c08ac2458506996a6f1c4f06cd03;
static PyCodeObject *codeobj_17727f777de2e1825ac7bfbd7d4c5b42;
static PyCodeObject *codeobj_1d8a74b3054be3d7de6f45e13b01b9ed;
static PyCodeObject *codeobj_c7669c8caeb97d1c9b1825727b5cea9a;

static void createModuleCodeObjects(void) {
    module_filename_obj = mod_consts[98]; CHECK_OBJECT(module_filename_obj);
    codeobj_6bde854d712dad430c9ea5c5ac22e83b = MAKE_CODEOBJECT(module_filename_obj, 1, CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[594], NULL, NULL, 0, 0, 0);
    codeobj_4dc3aedd5f581d86900fc22db4cf5702 = MAKE_CODEOBJECT(module_filename_obj, 724, CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[0], mod_consts[595], NULL, 0, 0, 0);
    codeobj_097fd85308f32255fc725ef557550425 = MAKE_CODEOBJECT(module_filename_obj, 726, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[1], mod_consts[596], NULL, 12, 0, 0);
    codeobj_5851b1f734f18da6d819ad9bb56555dd = MAKE_CODEOBJECT(module_filename_obj, 784, CO_GENERATOR | CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[18], mod_consts[597], NULL, 1, 0, 0);
    codeobj_fde2c08ac2458506996a6f1c4f06cd03 = MAKE_CODEOBJECT(module_filename_obj, 819, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[28], mod_consts[598], NULL, 2, 0, 0);
    codeobj_17727f777de2e1825ac7bfbd7d4c5b42 = MAKE_CODEOBJECT(module_filename_obj, 869, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[29], mod_consts[599], NULL, 2, 0, 0);
    codeobj_1d8a74b3054be3d7de6f45e13b01b9ed = MAKE_CODEOBJECT(module_filename_obj, 889, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[60], mod_consts[600], NULL, 2, 0, 0);
    codeobj_c7669c8caeb97d1c9b1825727b5cea9a = MAKE_CODEOBJECT(module_filename_obj, 801, CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE | CO_FUTURE_UNICODE_LITERALS, mod_consts[20], mod_consts[601], NULL, 2, 0, 0);
}

// The module function declarations.
static PyObject *MAKE_GENERATOR_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter__(struct Nuitka_CellObject **closure);


static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__1___init__(PyObject *defaults);


static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__();


static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__3_sanitize_token();


static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__4_allowed_token();


static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__5_disallowed_token();


static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__6_sanitize_css();


// The module function definitions.
static PyObject *impl_pip$_vendor$html5lib$filters$sanitizer$$$function__1___init__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_source = python_pars[1];
    PyObject *par_allowed_elements = python_pars[2];
    PyObject *par_allowed_attributes = python_pars[3];
    PyObject *par_allowed_css_properties = python_pars[4];
    PyObject *par_allowed_css_keywords = python_pars[5];
    PyObject *par_allowed_svg_properties = python_pars[6];
    PyObject *par_allowed_protocols = python_pars[7];
    PyObject *par_allowed_content_types = python_pars[8];
    PyObject *par_attr_val_is_uri = python_pars[9];
    PyObject *par_svg_attr_val_allows_ref = python_pars[10];
    PyObject *par_svg_allow_local_href = python_pars[11];
    struct Nuitka_FrameObject *frame_097fd85308f32255fc725ef557550425;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    static struct Nuitka_FrameObject *cache_frame_097fd85308f32255fc725ef557550425 = NULL;
    PyObject *tmp_return_value = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_097fd85308f32255fc725ef557550425)) {
        Py_XDECREF(cache_frame_097fd85308f32255fc725ef557550425);

#if _DEBUG_REFCOUNTS
        if (cache_frame_097fd85308f32255fc725ef557550425 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_097fd85308f32255fc725ef557550425 = MAKE_FUNCTION_FRAME(codeobj_097fd85308f32255fc725ef557550425, module_pip$_vendor$html5lib$filters$sanitizer, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_097fd85308f32255fc725ef557550425->m_type_description == NULL);
    frame_097fd85308f32255fc725ef557550425 = cache_frame_097fd85308f32255fc725ef557550425;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_097fd85308f32255fc725ef557550425);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_097fd85308f32255fc725ef557550425) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_type_arg_name_1;
        PyObject *tmp_object_arg_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        tmp_type_arg_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[0]);

        if (unlikely(tmp_type_arg_name_1 == NULL)) {
            tmp_type_arg_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[0]);
        }

        if (tmp_type_arg_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 769;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_self);
        tmp_object_arg_name_1 = par_self;
        tmp_called_instance_1 = BUILTIN_SUPER2(tmp_type_arg_name_1, tmp_object_arg_name_1);
        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 769;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_source);
        tmp_args_element_name_1 = par_source;
        frame_097fd85308f32255fc725ef557550425->m_frame.f_lineno = 769;
        tmp_call_result_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[1], tmp_args_element_name_1);
        Py_DECREF(tmp_called_instance_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 769;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_call_result_2;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        tmp_expression_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[2]);

        if (unlikely(tmp_expression_name_1 == NULL)) {
            tmp_expression_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[2]);
        }

        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 771;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[3]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 771;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[4]);

        if (unlikely(tmp_args_element_name_2 == NULL)) {
            tmp_args_element_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[4]);
        }

        if (tmp_args_element_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 771;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[5]);

        if (unlikely(tmp_args_element_name_3 == NULL)) {
            tmp_args_element_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[5]);
        }

        if (tmp_args_element_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 771;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        frame_097fd85308f32255fc725ef557550425->m_frame.f_lineno = 771;
        {
            PyObject *call_args[] = {tmp_args_element_name_2, tmp_args_element_name_3};
            tmp_call_result_2 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_called_name_1);
        if (tmp_call_result_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 771;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_2);
    }
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        CHECK_OBJECT(par_allowed_elements);
        tmp_assattr_name_1 = par_allowed_elements;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_1 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[6], tmp_assattr_name_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 773;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_2;
        PyObject *tmp_assattr_target_2;
        CHECK_OBJECT(par_allowed_attributes);
        tmp_assattr_name_2 = par_allowed_attributes;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_2 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[7], tmp_assattr_name_2);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 774;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_3;
        PyObject *tmp_assattr_target_3;
        CHECK_OBJECT(par_allowed_css_properties);
        tmp_assattr_name_3 = par_allowed_css_properties;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_3 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_3, mod_consts[8], tmp_assattr_name_3);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 775;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_4;
        PyObject *tmp_assattr_target_4;
        CHECK_OBJECT(par_allowed_css_keywords);
        tmp_assattr_name_4 = par_allowed_css_keywords;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_4 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_4, mod_consts[9], tmp_assattr_name_4);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 776;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_5;
        PyObject *tmp_assattr_target_5;
        CHECK_OBJECT(par_allowed_svg_properties);
        tmp_assattr_name_5 = par_allowed_svg_properties;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_5 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_5, mod_consts[10], tmp_assattr_name_5);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 777;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_6;
        PyObject *tmp_assattr_target_6;
        CHECK_OBJECT(par_allowed_protocols);
        tmp_assattr_name_6 = par_allowed_protocols;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_6 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_6, mod_consts[11], tmp_assattr_name_6);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 778;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_7;
        PyObject *tmp_assattr_target_7;
        CHECK_OBJECT(par_allowed_content_types);
        tmp_assattr_name_7 = par_allowed_content_types;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_7 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_7, mod_consts[12], tmp_assattr_name_7);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 779;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_8;
        PyObject *tmp_assattr_target_8;
        CHECK_OBJECT(par_attr_val_is_uri);
        tmp_assattr_name_8 = par_attr_val_is_uri;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_8 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_8, mod_consts[13], tmp_assattr_name_8);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 780;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_9;
        PyObject *tmp_assattr_target_9;
        CHECK_OBJECT(par_svg_attr_val_allows_ref);
        tmp_assattr_name_9 = par_svg_attr_val_allows_ref;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_9 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_9, mod_consts[14], tmp_assattr_name_9);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 781;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_assattr_name_10;
        PyObject *tmp_assattr_target_10;
        CHECK_OBJECT(par_svg_allow_local_href);
        tmp_assattr_name_10 = par_svg_allow_local_href;
        CHECK_OBJECT(par_self);
        tmp_assattr_target_10 = par_self;
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_10, mod_consts[15], tmp_assattr_name_10);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 782;
            type_description_1 = "ooooooooooooN";
            goto frame_exception_exit_1;
        }
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_097fd85308f32255fc725ef557550425);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_097fd85308f32255fc725ef557550425);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_097fd85308f32255fc725ef557550425, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_097fd85308f32255fc725ef557550425->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_097fd85308f32255fc725ef557550425, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_097fd85308f32255fc725ef557550425,
        type_description_1,
        par_self,
        par_source,
        par_allowed_elements,
        par_allowed_attributes,
        par_allowed_css_properties,
        par_allowed_css_keywords,
        par_allowed_svg_properties,
        par_allowed_protocols,
        par_allowed_content_types,
        par_attr_val_is_uri,
        par_svg_attr_val_allows_ref,
        par_svg_allow_local_href,
        NULL
    );


    // Release cached frame if used for exception.
    if (frame_097fd85308f32255fc725ef557550425 == cache_frame_097fd85308f32255fc725ef557550425) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_097fd85308f32255fc725ef557550425);
        cache_frame_097fd85308f32255fc725ef557550425 = NULL;
    }

    assertFrameObject(frame_097fd85308f32255fc725ef557550425);

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
    Py_XDECREF(par_source);
    par_source = NULL;
    CHECK_OBJECT(par_allowed_elements);
    Py_DECREF(par_allowed_elements);
    par_allowed_elements = NULL;
    CHECK_OBJECT(par_allowed_attributes);
    Py_DECREF(par_allowed_attributes);
    par_allowed_attributes = NULL;
    CHECK_OBJECT(par_allowed_css_properties);
    Py_DECREF(par_allowed_css_properties);
    par_allowed_css_properties = NULL;
    CHECK_OBJECT(par_allowed_css_keywords);
    Py_DECREF(par_allowed_css_keywords);
    par_allowed_css_keywords = NULL;
    CHECK_OBJECT(par_allowed_svg_properties);
    Py_DECREF(par_allowed_svg_properties);
    par_allowed_svg_properties = NULL;
    CHECK_OBJECT(par_allowed_protocols);
    Py_DECREF(par_allowed_protocols);
    par_allowed_protocols = NULL;
    CHECK_OBJECT(par_allowed_content_types);
    Py_DECREF(par_allowed_content_types);
    par_allowed_content_types = NULL;
    CHECK_OBJECT(par_attr_val_is_uri);
    Py_DECREF(par_attr_val_is_uri);
    par_attr_val_is_uri = NULL;
    CHECK_OBJECT(par_svg_attr_val_allows_ref);
    Py_DECREF(par_svg_attr_val_allows_ref);
    par_svg_attr_val_allows_ref = NULL;
    CHECK_OBJECT(par_svg_allow_local_href);
    Py_DECREF(par_svg_allow_local_href);
    par_svg_allow_local_href = NULL;
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
    Py_XDECREF(par_source);
    par_source = NULL;
    CHECK_OBJECT(par_allowed_elements);
    Py_DECREF(par_allowed_elements);
    par_allowed_elements = NULL;
    CHECK_OBJECT(par_allowed_attributes);
    Py_DECREF(par_allowed_attributes);
    par_allowed_attributes = NULL;
    CHECK_OBJECT(par_allowed_css_properties);
    Py_DECREF(par_allowed_css_properties);
    par_allowed_css_properties = NULL;
    CHECK_OBJECT(par_allowed_css_keywords);
    Py_DECREF(par_allowed_css_keywords);
    par_allowed_css_keywords = NULL;
    CHECK_OBJECT(par_allowed_svg_properties);
    Py_DECREF(par_allowed_svg_properties);
    par_allowed_svg_properties = NULL;
    CHECK_OBJECT(par_allowed_protocols);
    Py_DECREF(par_allowed_protocols);
    par_allowed_protocols = NULL;
    CHECK_OBJECT(par_allowed_content_types);
    Py_DECREF(par_allowed_content_types);
    par_allowed_content_types = NULL;
    CHECK_OBJECT(par_attr_val_is_uri);
    Py_DECREF(par_attr_val_is_uri);
    par_attr_val_is_uri = NULL;
    CHECK_OBJECT(par_svg_attr_val_allows_ref);
    Py_DECREF(par_svg_attr_val_allows_ref);
    par_svg_attr_val_allows_ref = NULL;
    CHECK_OBJECT(par_svg_allow_local_href);
    Py_DECREF(par_svg_allow_local_href);
    par_svg_allow_local_href = NULL;
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


static PyObject *impl_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    struct Nuitka_CellObject *par_self = Nuitka_Cell_New1(python_pars[0]);
    PyObject *tmp_return_value = NULL;

    // Actual function body.
    // Tried code:
    {
        struct Nuitka_CellObject *tmp_closure_1[1];

        tmp_closure_1[0] = par_self;
        Py_INCREF(tmp_closure_1[0]);

        tmp_return_value = MAKE_GENERATOR_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter__(tmp_closure_1);

        goto try_return_handler_1;
    }
    NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
    return NULL;
    // Return handler code:
    try_return_handler_1:;
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    goto function_return_exit;
    // End of try:
    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;

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



struct pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter___locals {
    PyObject *var_token;
    PyObject *tmp_for_loop_1__for_iterator;
    PyObject *tmp_for_loop_1__iter_value;
    char const *type_description_1;
    PyObject *exception_type;
    PyObject *exception_value;
    PyTracebackObject *exception_tb;
    int exception_lineno;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    int exception_keeper_lineno_1;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    int exception_keeper_lineno_2;
};

static PyObject *pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter___context(struct Nuitka_GeneratorObject *generator, PyObject *yield_return_value) {
    CHECK_OBJECT(generator);
    assert(Nuitka_Generator_Check((PyObject *)generator));
    CHECK_OBJECT_X(yield_return_value);

    // Heap access if used.
    struct pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter___locals *generator_heap = (struct pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter___locals *)generator->m_heap_storage;

    // Dispatch to yield based on return label index:
    switch(generator->m_yield_return_index) {
    case 1: goto yield_return_1;
    }

    // Local variable initialization
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    static struct Nuitka_FrameObject *cache_m_frame = NULL;
    generator_heap->var_token = NULL;
    generator_heap->tmp_for_loop_1__for_iterator = NULL;
    generator_heap->tmp_for_loop_1__iter_value = NULL;
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
        cache_m_frame = MAKE_FUNCTION_FRAME(codeobj_5851b1f734f18da6d819ad9bb56555dd, module_pip$_vendor$html5lib$filters$sanitizer, sizeof(void *)+sizeof(void *));
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
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_args_element_name_1;
        tmp_expression_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[17]);

        if (unlikely(tmp_expression_name_2 == NULL)) {
            tmp_expression_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
        }

        if (tmp_expression_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 785;
            generator_heap->type_description_1 = "co";
            goto frame_exception_exit_1;
        }
        tmp_expression_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[0]);
        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 785;
            generator_heap->type_description_1 = "co";
            goto frame_exception_exit_1;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[18]);
        Py_DECREF(tmp_expression_name_1);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 785;
            generator_heap->type_description_1 = "co";
            goto frame_exception_exit_1;
        }
        if (Nuitka_Cell_GET(generator->m_closure[0]) == NULL) {
            Py_DECREF(tmp_called_name_1);
            FORMAT_UNBOUND_CLOSURE_ERROR(&generator_heap->exception_type, &generator_heap->exception_value, mod_consts[19]);
            generator_heap->exception_tb = NULL;
            NORMALIZE_EXCEPTION(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            CHAIN_EXCEPTION(generator_heap->exception_value);

            generator_heap->exception_lineno = 785;
            generator_heap->type_description_1 = "co";
            goto frame_exception_exit_1;
        }

        tmp_args_element_name_1 = Nuitka_Cell_GET(generator->m_closure[0]);
        generator->m_frame->m_frame.f_lineno = 785;
        tmp_iter_arg_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_1, tmp_args_element_name_1);
        Py_DECREF(tmp_called_name_1);
        if (tmp_iter_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 785;
            generator_heap->type_description_1 = "co";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_1 = MAKE_ITERATOR(tmp_iter_arg_1);
        Py_DECREF(tmp_iter_arg_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 785;
            generator_heap->type_description_1 = "co";
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
                generator_heap->type_description_1 = "co";
                generator_heap->exception_lineno = 785;
                goto try_except_handler_2;
            }
        }

        {
            PyObject *old = generator_heap->tmp_for_loop_1__iter_value;
            generator_heap->tmp_for_loop_1__iter_value = tmp_assign_source_2;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_3;
        CHECK_OBJECT(generator_heap->tmp_for_loop_1__iter_value);
        tmp_assign_source_3 = generator_heap->tmp_for_loop_1__iter_value;
        {
            PyObject *old = generator_heap->var_token;
            generator_heap->var_token = tmp_assign_source_3;
            Py_INCREF(generator_heap->var_token);
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_4;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_2;
        if (Nuitka_Cell_GET(generator->m_closure[0]) == NULL) {

            FORMAT_UNBOUND_CLOSURE_ERROR(&generator_heap->exception_type, &generator_heap->exception_value, mod_consts[19]);
            generator_heap->exception_tb = NULL;
            NORMALIZE_EXCEPTION(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);
            CHAIN_EXCEPTION(generator_heap->exception_value);

            generator_heap->exception_lineno = 786;
            generator_heap->type_description_1 = "co";
            goto try_except_handler_2;
        }

        tmp_called_instance_1 = Nuitka_Cell_GET(generator->m_closure[0]);
        CHECK_OBJECT(generator_heap->var_token);
        tmp_args_element_name_2 = generator_heap->var_token;
        generator->m_frame->m_frame.f_lineno = 786;
        tmp_assign_source_4 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[20], tmp_args_element_name_2);
        if (tmp_assign_source_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 786;
            generator_heap->type_description_1 = "co";
            goto try_except_handler_2;
        }
        {
            PyObject *old = generator_heap->var_token;
            generator_heap->var_token = tmp_assign_source_4;
            Py_XDECREF(old);
        }

    }
    {
        nuitka_bool tmp_condition_result_1;
        int tmp_truth_name_1;
        CHECK_OBJECT(generator_heap->var_token);
        tmp_truth_name_1 = CHECK_IF_TRUE(generator_heap->var_token);
        if (tmp_truth_name_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 787;
            generator_heap->type_description_1 = "co";
            goto try_except_handler_2;
        }
        tmp_condition_result_1 = tmp_truth_name_1 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    {
        PyObject *tmp_expression_name_3;
        NUITKA_MAY_BE_UNUSED PyObject *tmp_yield_result_1;
        CHECK_OBJECT(generator_heap->var_token);
        tmp_expression_name_3 = generator_heap->var_token;
        Py_INCREF(tmp_expression_name_3);
        generator->m_yield_return_index = 1;
        return tmp_expression_name_3;
        yield_return_1:
        if (yield_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


            generator_heap->exception_lineno = 788;
            generator_heap->type_description_1 = "co";
            goto try_except_handler_2;
        }
        tmp_yield_result_1 = yield_return_value;
    }
    branch_no_1:;
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&generator_heap->exception_type, &generator_heap->exception_value, &generator_heap->exception_tb);


        generator_heap->exception_lineno = 785;
        generator_heap->type_description_1 = "co";
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

    Py_XDECREF(generator_heap->tmp_for_loop_1__iter_value);
    generator_heap->tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(generator_heap->tmp_for_loop_1__for_iterator);
    generator_heap->tmp_for_loop_1__for_iterator = NULL;
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
            generator->m_closure[0],
            generator_heap->var_token
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

    Py_XDECREF(generator_heap->var_token);
    generator_heap->var_token = NULL;
    // Re-raise.
    generator_heap->exception_type = generator_heap->exception_keeper_type_2;
    generator_heap->exception_value = generator_heap->exception_keeper_value_2;
    generator_heap->exception_tb = generator_heap->exception_keeper_tb_2;
    generator_heap->exception_lineno = generator_heap->exception_keeper_lineno_2;

    goto function_exception_exit;
    // End of try:
    try_end_2:;
    Py_XDECREF(generator_heap->tmp_for_loop_1__iter_value);
    generator_heap->tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(generator_heap->tmp_for_loop_1__for_iterator);
    generator_heap->tmp_for_loop_1__for_iterator = NULL;
    Py_XDECREF(generator_heap->var_token);
    generator_heap->var_token = NULL;


    return NULL;

    function_exception_exit:
    assert(generator_heap->exception_type);
    RESTORE_ERROR_OCCURRED(generator_heap->exception_type, generator_heap->exception_value, generator_heap->exception_tb);

    return NULL;

}

static PyObject *MAKE_GENERATOR_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter__(struct Nuitka_CellObject **closure) {
    return Nuitka_Generator_New(
        pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter___context,
        module_pip$_vendor$html5lib$filters$sanitizer,
        mod_consts[18],
#if PYTHON_VERSION >= 0x350
        mod_consts[21],
#endif
        codeobj_5851b1f734f18da6d819ad9bb56555dd,
        closure,
        1,
        sizeof(struct pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__$$$genobj__1___iter___locals)
    );
}


static PyObject *impl_pip$_vendor$html5lib$filters$sanitizer$$$function__3_sanitize_token(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_token = python_pars[1];
    PyObject *var_token_type = NULL;
    PyObject *var_name = NULL;
    PyObject *var_namespace = NULL;
    struct Nuitka_FrameObject *frame_c7669c8caeb97d1c9b1825727b5cea9a;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    int tmp_res;
    PyObject *tmp_return_value = NULL;
    static struct Nuitka_FrameObject *cache_frame_c7669c8caeb97d1c9b1825727b5cea9a = NULL;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_c7669c8caeb97d1c9b1825727b5cea9a)) {
        Py_XDECREF(cache_frame_c7669c8caeb97d1c9b1825727b5cea9a);

#if _DEBUG_REFCOUNTS
        if (cache_frame_c7669c8caeb97d1c9b1825727b5cea9a == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_c7669c8caeb97d1c9b1825727b5cea9a = MAKE_FUNCTION_FRAME(codeobj_c7669c8caeb97d1c9b1825727b5cea9a, module_pip$_vendor$html5lib$filters$sanitizer, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_c7669c8caeb97d1c9b1825727b5cea9a->m_type_description == NULL);
    frame_c7669c8caeb97d1c9b1825727b5cea9a = cache_frame_c7669c8caeb97d1c9b1825727b5cea9a;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_c7669c8caeb97d1c9b1825727b5cea9a);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_c7669c8caeb97d1c9b1825727b5cea9a) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_subscript_name_1;
        CHECK_OBJECT(par_token);
        tmp_expression_name_1 = par_token;
        tmp_subscript_name_1 = mod_consts[22];
        tmp_assign_source_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_1, tmp_subscript_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 804;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        assert(var_token_type == NULL);
        var_token_type = tmp_assign_source_1;
    }
    {
        bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        CHECK_OBJECT(var_token_type);
        tmp_compexpr_left_1 = var_token_type;
        tmp_compexpr_right_1 = mod_consts[23];
        tmp_res = PySequence_Contains(tmp_compexpr_right_1, tmp_compexpr_left_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 805;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_1 = (tmp_res == 1) ? true : false;
        if (tmp_condition_result_1 != false) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    {
        PyObject *tmp_assign_source_2;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_subscript_name_2;
        CHECK_OBJECT(par_token);
        tmp_expression_name_2 = par_token;
        tmp_subscript_name_2 = mod_consts[24];
        tmp_assign_source_2 = LOOKUP_SUBSCRIPT(tmp_expression_name_2, tmp_subscript_name_2);
        if (tmp_assign_source_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 806;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        assert(var_name == NULL);
        var_name = tmp_assign_source_2;
    }
    {
        PyObject *tmp_assign_source_3;
        PyObject *tmp_expression_name_3;
        PyObject *tmp_subscript_name_3;
        CHECK_OBJECT(par_token);
        tmp_expression_name_3 = par_token;
        tmp_subscript_name_3 = mod_consts[25];
        tmp_assign_source_3 = LOOKUP_SUBSCRIPT(tmp_expression_name_3, tmp_subscript_name_3);
        if (tmp_assign_source_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 807;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        assert(var_namespace == NULL);
        var_namespace = tmp_assign_source_3;
    }
    {
        bool tmp_condition_result_2;
        int tmp_or_left_truth_1;
        bool tmp_or_left_value_1;
        bool tmp_or_right_value_1;
        PyObject *tmp_compexpr_left_2;
        PyObject *tmp_compexpr_right_2;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_expression_name_4;
        int tmp_and_left_truth_1;
        bool tmp_and_left_value_1;
        bool tmp_and_right_value_1;
        PyObject *tmp_compexpr_left_3;
        PyObject *tmp_compexpr_right_3;
        PyObject *tmp_compexpr_left_4;
        PyObject *tmp_compexpr_right_4;
        PyObject *tmp_tuple_element_2;
        PyObject *tmp_expression_name_5;
        PyObject *tmp_subscript_name_4;
        PyObject *tmp_expression_name_6;
        CHECK_OBJECT(var_namespace);
        tmp_tuple_element_1 = var_namespace;
        tmp_compexpr_left_2 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_compexpr_left_2, 0, tmp_tuple_element_1);
        CHECK_OBJECT(var_name);
        tmp_tuple_element_1 = var_name;
        PyTuple_SET_ITEM0(tmp_compexpr_left_2, 1, tmp_tuple_element_1);
        CHECK_OBJECT(par_self);
        tmp_expression_name_4 = par_self;
        tmp_compexpr_right_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_4, mod_consts[6]);
        if (tmp_compexpr_right_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_compexpr_left_2);

            exception_lineno = 808;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_2, tmp_compexpr_left_2);
        Py_DECREF(tmp_compexpr_left_2);
        Py_DECREF(tmp_compexpr_right_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 808;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_or_left_value_1 = (tmp_res == 1) ? true : false;
        tmp_or_left_truth_1 = tmp_or_left_value_1 != false ? 1 : 0;
        if (tmp_or_left_truth_1 == 1) {
            goto or_left_1;
        } else {
            goto or_right_1;
        }
        or_right_1:;
        CHECK_OBJECT(var_namespace);
        tmp_compexpr_left_3 = var_namespace;
        tmp_compexpr_right_3 = Py_None;
        tmp_and_left_value_1 = (tmp_compexpr_left_3 == tmp_compexpr_right_3) ? true : false;
        tmp_and_left_truth_1 = tmp_and_left_value_1 != false ? 1 : 0;
        if (tmp_and_left_truth_1 == 1) {
            goto and_right_1;
        } else {
            goto and_left_1;
        }
        and_right_1:;
        tmp_expression_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

        if (unlikely(tmp_expression_name_5 == NULL)) {
            tmp_expression_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
        }

        if (tmp_expression_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 810;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_subscript_name_4 = mod_consts[27];
        tmp_tuple_element_2 = LOOKUP_SUBSCRIPT(tmp_expression_name_5, tmp_subscript_name_4);
        if (tmp_tuple_element_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 810;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_compexpr_left_4 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_compexpr_left_4, 0, tmp_tuple_element_2);
        CHECK_OBJECT(var_name);
        tmp_tuple_element_2 = var_name;
        PyTuple_SET_ITEM0(tmp_compexpr_left_4, 1, tmp_tuple_element_2);
        if (par_self == NULL) {
            Py_DECREF(tmp_compexpr_left_4);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 810;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_6 = par_self;
        tmp_compexpr_right_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_6, mod_consts[6]);
        if (tmp_compexpr_right_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_compexpr_left_4);

            exception_lineno = 810;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_4, tmp_compexpr_left_4);
        Py_DECREF(tmp_compexpr_left_4);
        Py_DECREF(tmp_compexpr_right_4);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 810;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_and_right_value_1 = (tmp_res == 1) ? true : false;
        tmp_or_right_value_1 = tmp_and_right_value_1;
        goto and_end_1;
        and_left_1:;
        tmp_or_right_value_1 = tmp_and_left_value_1;
        and_end_1:;
        tmp_condition_result_2 = tmp_or_right_value_1;
        goto or_end_1;
        or_left_1:;
        tmp_condition_result_2 = tmp_or_left_value_1;
        or_end_1:;
        if (tmp_condition_result_2 != false) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_called_instance_1;
        PyObject *tmp_args_element_name_1;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 811;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }

        tmp_called_instance_1 = par_self;
        CHECK_OBJECT(par_token);
        tmp_args_element_name_1 = par_token;
        frame_c7669c8caeb97d1c9b1825727b5cea9a->m_frame.f_lineno = 811;
        tmp_return_value = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_1, mod_consts[28], tmp_args_element_name_1);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 811;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }
    goto branch_end_2;
    branch_no_2:;
    {
        PyObject *tmp_called_instance_2;
        PyObject *tmp_args_element_name_2;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 813;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }

        tmp_called_instance_2 = par_self;
        CHECK_OBJECT(par_token);
        tmp_args_element_name_2 = par_token;
        frame_c7669c8caeb97d1c9b1825727b5cea9a->m_frame.f_lineno = 813;
        tmp_return_value = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_2, mod_consts[29], tmp_args_element_name_2);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 813;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }
    branch_end_2:;
    goto branch_end_1;
    branch_no_1:;
    {
        nuitka_bool tmp_condition_result_3;
        PyObject *tmp_operand_name_1;
        PyObject *tmp_compexpr_left_5;
        PyObject *tmp_compexpr_right_5;
        CHECK_OBJECT(var_token_type);
        tmp_compexpr_left_5 = var_token_type;
        tmp_compexpr_right_5 = mod_consts[30];
        tmp_operand_name_1 = RICH_COMPARE_EQ_OBJECT_OBJECT_OBJECT(tmp_compexpr_left_5, tmp_compexpr_right_5);
        if (tmp_operand_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 814;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        Py_DECREF(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 814;
            type_description_1 = "ooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_3 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_3 == NUITKA_BOOL_TRUE) {
            goto branch_yes_3;
        } else {
            goto branch_no_3;
        }
    }
    branch_yes_3:;
    CHECK_OBJECT(par_token);
    tmp_return_value = par_token;
    Py_INCREF(tmp_return_value);
    goto frame_return_exit_1;
    branch_no_3:;
    branch_end_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c7669c8caeb97d1c9b1825727b5cea9a);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_c7669c8caeb97d1c9b1825727b5cea9a);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_c7669c8caeb97d1c9b1825727b5cea9a);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_c7669c8caeb97d1c9b1825727b5cea9a, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_c7669c8caeb97d1c9b1825727b5cea9a->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_c7669c8caeb97d1c9b1825727b5cea9a, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_c7669c8caeb97d1c9b1825727b5cea9a,
        type_description_1,
        par_self,
        par_token,
        var_token_type,
        var_name,
        var_namespace
    );


    // Release cached frame if used for exception.
    if (frame_c7669c8caeb97d1c9b1825727b5cea9a == cache_frame_c7669c8caeb97d1c9b1825727b5cea9a) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_c7669c8caeb97d1c9b1825727b5cea9a);
        cache_frame_c7669c8caeb97d1c9b1825727b5cea9a = NULL;
    }

    assertFrameObject(frame_c7669c8caeb97d1c9b1825727b5cea9a);

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
    Py_XDECREF(par_token);
    par_token = NULL;
    CHECK_OBJECT(var_token_type);
    Py_DECREF(var_token_type);
    var_token_type = NULL;
    Py_XDECREF(var_name);
    var_name = NULL;
    Py_XDECREF(var_namespace);
    var_namespace = NULL;
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
    Py_XDECREF(par_token);
    par_token = NULL;
    Py_XDECREF(var_token_type);
    var_token_type = NULL;
    Py_XDECREF(var_name);
    var_name = NULL;
    Py_XDECREF(var_namespace);
    var_namespace = NULL;
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


static PyObject *impl_pip$_vendor$html5lib$filters$sanitizer$$$function__4_allowed_token(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_token = python_pars[1];
    PyObject *var_attrs = NULL;
    PyObject *var_attr_names = NULL;
    PyObject *var_to_remove = NULL;
    PyObject *var_attr = NULL;
    PyObject *var_val_unescaped = NULL;
    PyObject *var_uri = NULL;
    PyObject *var_m = NULL;
    PyObject *tmp_for_loop_1__for_iterator = NULL;
    PyObject *tmp_for_loop_1__iter_value = NULL;
    PyObject *tmp_for_loop_2__for_iterator = NULL;
    PyObject *tmp_for_loop_2__iter_value = NULL;
    PyObject *tmp_for_loop_3__for_iterator = NULL;
    PyObject *tmp_for_loop_3__iter_value = NULL;
    struct Nuitka_FrameObject *frame_fde2c08ac2458506996a6f1c4f06cd03;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    int tmp_res;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
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
    PyObject *tmp_return_value = NULL;
    static struct Nuitka_FrameObject *cache_frame_fde2c08ac2458506996a6f1c4f06cd03 = NULL;
    PyObject *exception_keeper_type_6;
    PyObject *exception_keeper_value_6;
    PyTracebackObject *exception_keeper_tb_6;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_6;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_fde2c08ac2458506996a6f1c4f06cd03)) {
        Py_XDECREF(cache_frame_fde2c08ac2458506996a6f1c4f06cd03);

#if _DEBUG_REFCOUNTS
        if (cache_frame_fde2c08ac2458506996a6f1c4f06cd03 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_fde2c08ac2458506996a6f1c4f06cd03 = MAKE_FUNCTION_FRAME(codeobj_fde2c08ac2458506996a6f1c4f06cd03, module_pip$_vendor$html5lib$filters$sanitizer, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_fde2c08ac2458506996a6f1c4f06cd03->m_type_description == NULL);
    frame_fde2c08ac2458506996a6f1c4f06cd03 = cache_frame_fde2c08ac2458506996a6f1c4f06cd03;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_fde2c08ac2458506996a6f1c4f06cd03);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_fde2c08ac2458506996a6f1c4f06cd03) == 2); // Frame stack

    // Framed code:
    {
        bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        tmp_compexpr_left_1 = mod_consts[31];
        CHECK_OBJECT(par_token);
        tmp_compexpr_right_1 = par_token;
        tmp_res = PySequence_Contains(tmp_compexpr_right_1, tmp_compexpr_left_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 820;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_1 = (tmp_res == 1) ? true : false;
        if (tmp_condition_result_1 != false) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_subscript_name_1;
        CHECK_OBJECT(par_token);
        tmp_expression_name_1 = par_token;
        tmp_subscript_name_1 = mod_consts[31];
        tmp_assign_source_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_1, tmp_subscript_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 821;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        assert(var_attrs == NULL);
        var_attrs = tmp_assign_source_1;
    }
    {
        PyObject *tmp_assign_source_2;
        PyObject *tmp_set_arg_1;
        PyObject *tmp_called_instance_1;
        CHECK_OBJECT(var_attrs);
        tmp_called_instance_1 = var_attrs;
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 822;
        tmp_set_arg_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_1, mod_consts[32]);
        if (tmp_set_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 822;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_2 = PySet_New(tmp_set_arg_1);
        Py_DECREF(tmp_set_arg_1);
        if (tmp_assign_source_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 822;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        assert(var_attr_names == NULL);
        var_attr_names = tmp_assign_source_2;
    }
    {
        PyObject *tmp_assign_source_3;
        PyObject *tmp_iter_arg_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_expression_name_2;
        CHECK_OBJECT(var_attr_names);
        tmp_left_name_1 = var_attr_names;
        CHECK_OBJECT(par_self);
        tmp_expression_name_2 = par_self;
        tmp_right_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[7]);
        if (tmp_right_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 825;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_iter_arg_1 = BINARY_OPERATION_SUB_OBJECT_OBJECT_OBJECT(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_iter_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 825;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_3 = MAKE_ITERATOR(tmp_iter_arg_1);
        Py_DECREF(tmp_iter_arg_1);
        if (tmp_assign_source_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 825;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        assert(tmp_for_loop_1__for_iterator == NULL);
        tmp_for_loop_1__for_iterator = tmp_assign_source_3;
    }
    // Tried code:
    loop_start_1:;
    {
        PyObject *tmp_next_source_1;
        PyObject *tmp_assign_source_4;
        CHECK_OBJECT(tmp_for_loop_1__for_iterator);
        tmp_next_source_1 = tmp_for_loop_1__for_iterator;
        tmp_assign_source_4 = ITERATOR_NEXT(tmp_next_source_1);
        if (tmp_assign_source_4 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_1;
            } else {

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                type_description_1 = "ooooooooo";
                exception_lineno = 825;
                goto try_except_handler_2;
            }
        }

        {
            PyObject *old = tmp_for_loop_1__iter_value;
            tmp_for_loop_1__iter_value = tmp_assign_source_4;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_5;
        CHECK_OBJECT(tmp_for_loop_1__iter_value);
        tmp_assign_source_5 = tmp_for_loop_1__iter_value;
        {
            PyObject *old = var_to_remove;
            var_to_remove = tmp_assign_source_5;
            Py_INCREF(var_to_remove);
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_delsubscr_target_1;
        PyObject *tmp_expression_name_3;
        PyObject *tmp_subscript_name_2;
        PyObject *tmp_delsubscr_subscript_1;
        if (par_token == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 826;
            type_description_1 = "ooooooooo";
            goto try_except_handler_2;
        }

        tmp_expression_name_3 = par_token;
        tmp_subscript_name_2 = mod_consts[31];
        tmp_delsubscr_target_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_3, tmp_subscript_name_2);
        if (tmp_delsubscr_target_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 826;
            type_description_1 = "ooooooooo";
            goto try_except_handler_2;
        }
        CHECK_OBJECT(var_to_remove);
        tmp_delsubscr_subscript_1 = var_to_remove;
        tmp_result = DEL_SUBSCRIPT(tmp_delsubscr_target_1, tmp_delsubscr_subscript_1);
        Py_DECREF(tmp_delsubscr_target_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 826;
            type_description_1 = "ooooooooo";
            goto try_except_handler_2;
        }
    }
    {
        PyObject *tmp_called_instance_2;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        if (var_attr_names == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[34]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 827;
            type_description_1 = "ooooooooo";
            goto try_except_handler_2;
        }

        tmp_called_instance_2 = var_attr_names;
        CHECK_OBJECT(var_to_remove);
        tmp_args_element_name_1 = var_to_remove;
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 827;
        tmp_call_result_1 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_2, mod_consts[35], tmp_args_element_name_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 827;
            type_description_1 = "ooooooooo";
            goto try_except_handler_2;
        }
        Py_DECREF(tmp_call_result_1);
    }
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 825;
        type_description_1 = "ooooooooo";
        goto try_except_handler_2;
    }
    goto loop_start_1;
    loop_end_1:;
    goto try_end_1;
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
    Py_XDECREF(tmp_for_loop_1__iter_value);
    tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_1__for_iterator);
    tmp_for_loop_1__for_iterator = NULL;
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_iter_arg_2;
        PyObject *tmp_left_name_2;
        PyObject *tmp_right_name_2;
        PyObject *tmp_expression_name_4;
        if (var_attr_names == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[34]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 830;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_left_name_2 = var_attr_names;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 830;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_4 = par_self;
        tmp_right_name_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_4, mod_consts[13]);
        if (tmp_right_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 830;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_iter_arg_2 = BINARY_OPERATION_BITAND_OBJECT_OBJECT_OBJECT(tmp_left_name_2, tmp_right_name_2);
        Py_DECREF(tmp_right_name_2);
        if (tmp_iter_arg_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 830;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_6 = MAKE_ITERATOR(tmp_iter_arg_2);
        Py_DECREF(tmp_iter_arg_2);
        if (tmp_assign_source_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 830;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        assert(tmp_for_loop_2__for_iterator == NULL);
        tmp_for_loop_2__for_iterator = tmp_assign_source_6;
    }
    // Tried code:
    loop_start_2:;
    {
        PyObject *tmp_next_source_2;
        PyObject *tmp_assign_source_7;
        CHECK_OBJECT(tmp_for_loop_2__for_iterator);
        tmp_next_source_2 = tmp_for_loop_2__for_iterator;
        tmp_assign_source_7 = ITERATOR_NEXT(tmp_next_source_2);
        if (tmp_assign_source_7 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_2;
            } else {

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                type_description_1 = "ooooooooo";
                exception_lineno = 830;
                goto try_except_handler_3;
            }
        }

        {
            PyObject *old = tmp_for_loop_2__iter_value;
            tmp_for_loop_2__iter_value = tmp_assign_source_7;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_8;
        CHECK_OBJECT(tmp_for_loop_2__iter_value);
        tmp_assign_source_8 = tmp_for_loop_2__iter_value;
        {
            PyObject *old = var_attr;
            var_attr = tmp_assign_source_8;
            Py_INCREF(var_attr);
            Py_XDECREF(old);
        }

    }
    {
        bool tmp_condition_result_2;
        PyObject *tmp_compexpr_left_2;
        PyObject *tmp_compexpr_right_2;
        CHECK_OBJECT(var_attr);
        tmp_compexpr_left_2 = var_attr;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 831;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_compexpr_right_2 = var_attrs;
        tmp_res = PySequence_Contains(tmp_compexpr_right_2, tmp_compexpr_left_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 831;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_condition_result_2 = (tmp_res == 0) ? true : false;
        if (tmp_condition_result_2 != false) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_raise_type_1;
        tmp_raise_type_1 = PyExc_AssertionError;
        exception_type = tmp_raise_type_1;
        Py_INCREF(tmp_raise_type_1);
        exception_lineno = 831;
        RAISE_EXCEPTION_WITH_TYPE(&exception_type, &exception_value, &exception_tb);
        type_description_1 = "ooooooooo";
        goto try_except_handler_3;
    }
    branch_no_2:;
    {
        PyObject *tmp_assign_source_9;
        PyObject *tmp_called_instance_3;
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_5;
        PyObject *tmp_args_element_name_2;
        PyObject *tmp_args_element_name_3;
        PyObject *tmp_args_element_name_4;
        PyObject *tmp_called_name_2;
        PyObject *tmp_args_element_name_5;
        PyObject *tmp_expression_name_6;
        PyObject *tmp_subscript_name_3;
        tmp_expression_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_expression_name_5 == NULL)) {
            tmp_expression_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_expression_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 836;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_5, mod_consts[38]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 836;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_args_element_name_2 = mod_consts[39];
        tmp_args_element_name_3 = mod_consts[40];
        tmp_called_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[41]);

        if (unlikely(tmp_called_name_2 == NULL)) {
            tmp_called_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[41]);
        }

        if (tmp_called_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 837;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        if (var_attrs == NULL) {
            Py_DECREF(tmp_called_name_1);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 837;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_expression_name_6 = var_attrs;
        CHECK_OBJECT(var_attr);
        tmp_subscript_name_3 = var_attr;
        tmp_args_element_name_5 = LOOKUP_SUBSCRIPT(tmp_expression_name_6, tmp_subscript_name_3);
        if (tmp_args_element_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 837;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 837;
        tmp_args_element_name_4 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_2, tmp_args_element_name_5);
        Py_DECREF(tmp_args_element_name_5);
        if (tmp_args_element_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 837;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 836;
        {
            PyObject *call_args[] = {tmp_args_element_name_2, tmp_args_element_name_3, tmp_args_element_name_4};
            tmp_called_instance_3 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_called_name_1);
        Py_DECREF(tmp_args_element_name_4);
        if (tmp_called_instance_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 836;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 836;
        tmp_assign_source_9 = CALL_METHOD_NO_ARGS(tmp_called_instance_3, mod_consts[42]);
        Py_DECREF(tmp_called_instance_3);
        if (tmp_assign_source_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 836;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        {
            PyObject *old = var_val_unescaped;
            var_val_unescaped = tmp_assign_source_9;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_10;
        PyObject *tmp_called_instance_4;
        CHECK_OBJECT(var_val_unescaped);
        tmp_called_instance_4 = var_val_unescaped;
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 839;
        tmp_assign_source_10 = CALL_METHOD_WITH_ARGS2(
            tmp_called_instance_4,
            mod_consts[43],
            &PyTuple_GET_ITEM(mod_consts[44], 0)
        );

        if (tmp_assign_source_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 839;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        {
            PyObject *old = var_val_unescaped;
            var_val_unescaped = tmp_assign_source_10;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_11;
        PyObject *tmp_called_instance_5;
        PyObject *tmp_args_element_name_6;
        tmp_called_instance_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[45]);

        if (unlikely(tmp_called_instance_5 == NULL)) {
            tmp_called_instance_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[45]);
        }

        if (tmp_called_instance_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 841;
            type_description_1 = "ooooooooo";
            goto try_except_handler_4;
        }
        CHECK_OBJECT(var_val_unescaped);
        tmp_args_element_name_6 = var_val_unescaped;
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 841;
        tmp_assign_source_11 = CALL_METHOD_WITH_SINGLE_ARG(tmp_called_instance_5, mod_consts[45], tmp_args_element_name_6);
        if (tmp_assign_source_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 841;
            type_description_1 = "ooooooooo";
            goto try_except_handler_4;
        }
        {
            PyObject *old = var_uri;
            var_uri = tmp_assign_source_11;
            Py_XDECREF(old);
        }

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

    // Preserve existing published exception id 1.
    GET_CURRENT_EXCEPTION(&exception_preserved_type_1, &exception_preserved_value_1, &exception_preserved_tb_1);

    if (exception_keeper_tb_2 == NULL) {
        exception_keeper_tb_2 = MAKE_TRACEBACK(frame_fde2c08ac2458506996a6f1c4f06cd03, exception_keeper_lineno_2);
    } else if (exception_keeper_lineno_2 != 0) {
        exception_keeper_tb_2 = ADD_TRACEBACK(exception_keeper_tb_2, frame_fde2c08ac2458506996a6f1c4f06cd03, exception_keeper_lineno_2);
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
        tmp_compexpr_right_3 = PyExc_ValueError;
        tmp_res = EXCEPTION_MATCH_BOOL(tmp_compexpr_left_3, tmp_compexpr_right_3);
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
        PyObject *tmp_assign_source_12;
        tmp_assign_source_12 = Py_None;
        {
            PyObject *old = var_uri;
            var_uri = tmp_assign_source_12;
            Py_INCREF(var_uri);
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_delsubscr_target_2;
        PyObject *tmp_delsubscr_subscript_2;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 844;
            type_description_1 = "ooooooooo";
            goto try_except_handler_5;
        }

        tmp_delsubscr_target_2 = var_attrs;
        CHECK_OBJECT(var_attr);
        tmp_delsubscr_subscript_2 = var_attr;
        tmp_result = DEL_SUBSCRIPT(tmp_delsubscr_target_2, tmp_delsubscr_subscript_2);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 844;
            type_description_1 = "ooooooooo";
            goto try_except_handler_5;
        }
    }
    goto branch_end_3;
    branch_no_3:;
    tmp_result = RERAISE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
    if (unlikely(tmp_result == false)) {
        exception_lineno = 840;
    }

    if (exception_tb && exception_tb->tb_frame == &frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame) frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = exception_tb->tb_lineno;
    type_description_1 = "ooooooooo";
    goto try_except_handler_5;
    branch_end_3:;
    goto try_end_3;
    // Exception handler code:
    try_except_handler_5:;
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

    goto try_except_handler_3;
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
        nuitka_bool tmp_condition_result_4;
        int tmp_and_left_truth_1;
        nuitka_bool tmp_and_left_value_1;
        nuitka_bool tmp_and_right_value_1;
        int tmp_truth_name_1;
        PyObject *tmp_expression_name_7;
        PyObject *tmp_attribute_value_1;
        int tmp_truth_name_2;
        if (var_uri == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[46]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 845;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_truth_name_1 = CHECK_IF_TRUE(var_uri);
        if (tmp_truth_name_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 845;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_and_left_value_1 = tmp_truth_name_1 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        tmp_and_left_truth_1 = tmp_and_left_value_1 == NUITKA_BOOL_TRUE ? 1 : 0;
        if (tmp_and_left_truth_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 845;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        if (tmp_and_left_truth_1 == 1) {
            goto and_right_1;
        } else {
            goto and_left_1;
        }
        and_right_1:;
        if (var_uri == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[46]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 845;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_expression_name_7 = var_uri;
        tmp_attribute_value_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_7, mod_consts[47]);
        if (tmp_attribute_value_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 845;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_truth_name_2 = CHECK_IF_TRUE(tmp_attribute_value_1);
        if (tmp_truth_name_2 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_attribute_value_1);

            exception_lineno = 845;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_and_right_value_1 = tmp_truth_name_2 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_attribute_value_1);
        tmp_condition_result_4 = tmp_and_right_value_1;
        goto and_end_1;
        and_left_1:;
        tmp_condition_result_4 = tmp_and_left_value_1;
        and_end_1:;
        if (tmp_condition_result_4 == NUITKA_BOOL_TRUE) {
            goto branch_yes_4;
        } else {
            goto branch_no_4;
        }
    }
    branch_yes_4:;
    {
        bool tmp_condition_result_5;
        PyObject *tmp_compexpr_left_4;
        PyObject *tmp_compexpr_right_4;
        PyObject *tmp_expression_name_8;
        PyObject *tmp_expression_name_9;
        if (var_uri == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[46]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 846;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_expression_name_8 = var_uri;
        tmp_compexpr_left_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_8, mod_consts[47]);
        if (tmp_compexpr_left_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 846;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        if (par_self == NULL) {
            Py_DECREF(tmp_compexpr_left_4);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 846;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_expression_name_9 = par_self;
        tmp_compexpr_right_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_9, mod_consts[11]);
        if (tmp_compexpr_right_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_compexpr_left_4);

            exception_lineno = 846;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_4, tmp_compexpr_left_4);
        Py_DECREF(tmp_compexpr_left_4);
        Py_DECREF(tmp_compexpr_right_4);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 846;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_condition_result_5 = (tmp_res == 0) ? true : false;
        if (tmp_condition_result_5 != false) {
            goto branch_yes_5;
        } else {
            goto branch_no_5;
        }
    }
    branch_yes_5:;
    {
        PyObject *tmp_delsubscr_target_3;
        PyObject *tmp_delsubscr_subscript_3;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 847;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_delsubscr_target_3 = var_attrs;
        CHECK_OBJECT(var_attr);
        tmp_delsubscr_subscript_3 = var_attr;
        tmp_result = DEL_SUBSCRIPT(tmp_delsubscr_target_3, tmp_delsubscr_subscript_3);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 847;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
    }
    branch_no_5:;
    {
        nuitka_bool tmp_condition_result_6;
        PyObject *tmp_compexpr_left_5;
        PyObject *tmp_compexpr_right_5;
        PyObject *tmp_expression_name_10;
        PyObject *tmp_tmp_condition_result_6_object_1;
        int tmp_truth_name_3;
        if (var_uri == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[46]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 848;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_expression_name_10 = var_uri;
        tmp_compexpr_left_5 = LOOKUP_ATTRIBUTE(tmp_expression_name_10, mod_consts[47]);
        if (tmp_compexpr_left_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 848;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_compexpr_right_5 = mod_consts[31];
        tmp_tmp_condition_result_6_object_1 = RICH_COMPARE_EQ_OBJECT_OBJECT_OBJECT(tmp_compexpr_left_5, tmp_compexpr_right_5);
        Py_DECREF(tmp_compexpr_left_5);
        if (tmp_tmp_condition_result_6_object_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 848;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_truth_name_3 = CHECK_IF_TRUE(tmp_tmp_condition_result_6_object_1);
        if (tmp_truth_name_3 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_tmp_condition_result_6_object_1);

            exception_lineno = 848;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_condition_result_6 = tmp_truth_name_3 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_tmp_condition_result_6_object_1);
        if (tmp_condition_result_6 == NUITKA_BOOL_TRUE) {
            goto branch_yes_6;
        } else {
            goto branch_no_6;
        }
    }
    branch_yes_6:;
    {
        PyObject *tmp_assign_source_13;
        PyObject *tmp_called_name_3;
        PyObject *tmp_expression_name_11;
        PyObject *tmp_args_element_name_7;
        PyObject *tmp_expression_name_12;
        tmp_expression_name_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[48]);

        if (unlikely(tmp_expression_name_11 == NULL)) {
            tmp_expression_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[48]);
        }

        if (tmp_expression_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 849;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_called_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_11, mod_consts[49]);
        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 849;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        if (var_uri == NULL) {
            Py_DECREF(tmp_called_name_3);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[46]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 849;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_expression_name_12 = var_uri;
        tmp_args_element_name_7 = LOOKUP_ATTRIBUTE(tmp_expression_name_12, mod_consts[50]);
        if (tmp_args_element_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_3);

            exception_lineno = 849;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 849;
        tmp_assign_source_13 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_3, tmp_args_element_name_7);
        Py_DECREF(tmp_called_name_3);
        Py_DECREF(tmp_args_element_name_7);
        if (tmp_assign_source_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 849;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        {
            PyObject *old = var_m;
            var_m = tmp_assign_source_13;
            Py_XDECREF(old);
        }

    }
    {
        nuitka_bool tmp_condition_result_7;
        PyObject *tmp_operand_name_1;
        CHECK_OBJECT(var_m);
        tmp_operand_name_1 = var_m;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 850;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_condition_result_7 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_7 == NUITKA_BOOL_TRUE) {
            goto branch_yes_7;
        } else {
            goto branch_no_7;
        }
    }
    branch_yes_7:;
    {
        PyObject *tmp_delsubscr_target_4;
        PyObject *tmp_delsubscr_subscript_4;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 851;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_delsubscr_target_4 = var_attrs;
        CHECK_OBJECT(var_attr);
        tmp_delsubscr_subscript_4 = var_attr;
        tmp_result = DEL_SUBSCRIPT(tmp_delsubscr_target_4, tmp_delsubscr_subscript_4);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 851;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
    }
    goto branch_end_7;
    branch_no_7:;
    {
        bool tmp_condition_result_8;
        PyObject *tmp_compexpr_left_6;
        PyObject *tmp_compexpr_right_6;
        PyObject *tmp_called_instance_6;
        PyObject *tmp_expression_name_13;
        CHECK_OBJECT(var_m);
        tmp_called_instance_6 = var_m;
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 852;
        tmp_compexpr_left_6 = CALL_METHOD_WITH_ARGS1(
            tmp_called_instance_6,
            mod_consts[51],
            &PyTuple_GET_ITEM(mod_consts[52], 0)
        );

        if (tmp_compexpr_left_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 852;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        if (par_self == NULL) {
            Py_DECREF(tmp_compexpr_left_6);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 852;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_expression_name_13 = par_self;
        tmp_compexpr_right_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_13, mod_consts[12]);
        if (tmp_compexpr_right_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_compexpr_left_6);

            exception_lineno = 852;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_6, tmp_compexpr_left_6);
        Py_DECREF(tmp_compexpr_left_6);
        Py_DECREF(tmp_compexpr_right_6);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 852;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
        tmp_condition_result_8 = (tmp_res == 0) ? true : false;
        if (tmp_condition_result_8 != false) {
            goto branch_yes_8;
        } else {
            goto branch_no_8;
        }
    }
    branch_yes_8:;
    {
        PyObject *tmp_delsubscr_target_5;
        PyObject *tmp_delsubscr_subscript_5;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 853;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }

        tmp_delsubscr_target_5 = var_attrs;
        CHECK_OBJECT(var_attr);
        tmp_delsubscr_subscript_5 = var_attr;
        tmp_result = DEL_SUBSCRIPT(tmp_delsubscr_target_5, tmp_delsubscr_subscript_5);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 853;
            type_description_1 = "ooooooooo";
            goto try_except_handler_3;
        }
    }
    branch_no_8:;
    branch_end_7:;
    branch_no_6:;
    branch_no_4:;
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 830;
        type_description_1 = "ooooooooo";
        goto try_except_handler_3;
    }
    goto loop_start_2;
    loop_end_2:;
    goto try_end_4;
    // Exception handler code:
    try_except_handler_3:;
    exception_keeper_type_4 = exception_type;
    exception_keeper_value_4 = exception_value;
    exception_keeper_tb_4 = exception_tb;
    exception_keeper_lineno_4 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_for_loop_2__iter_value);
    tmp_for_loop_2__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_2__for_iterator);
    tmp_for_loop_2__for_iterator = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_4;
    exception_value = exception_keeper_value_4;
    exception_tb = exception_keeper_tb_4;
    exception_lineno = exception_keeper_lineno_4;

    goto frame_exception_exit_1;
    // End of try:
    try_end_4:;
    Py_XDECREF(tmp_for_loop_2__iter_value);
    tmp_for_loop_2__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_2__for_iterator);
    tmp_for_loop_2__for_iterator = NULL;
    {
        PyObject *tmp_assign_source_14;
        PyObject *tmp_iter_arg_3;
        PyObject *tmp_expression_name_14;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 855;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_14 = par_self;
        tmp_iter_arg_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_14, mod_consts[14]);
        if (tmp_iter_arg_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 855;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_14 = MAKE_ITERATOR(tmp_iter_arg_3);
        Py_DECREF(tmp_iter_arg_3);
        if (tmp_assign_source_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 855;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        assert(tmp_for_loop_3__for_iterator == NULL);
        tmp_for_loop_3__for_iterator = tmp_assign_source_14;
    }
    // Tried code:
    loop_start_3:;
    {
        PyObject *tmp_next_source_3;
        PyObject *tmp_assign_source_15;
        CHECK_OBJECT(tmp_for_loop_3__for_iterator);
        tmp_next_source_3 = tmp_for_loop_3__for_iterator;
        tmp_assign_source_15 = ITERATOR_NEXT(tmp_next_source_3);
        if (tmp_assign_source_15 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_3;
            } else {

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                type_description_1 = "ooooooooo";
                exception_lineno = 855;
                goto try_except_handler_6;
            }
        }

        {
            PyObject *old = tmp_for_loop_3__iter_value;
            tmp_for_loop_3__iter_value = tmp_assign_source_15;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_16;
        CHECK_OBJECT(tmp_for_loop_3__iter_value);
        tmp_assign_source_16 = tmp_for_loop_3__iter_value;
        {
            PyObject *old = var_attr;
            var_attr = tmp_assign_source_16;
            Py_INCREF(var_attr);
            Py_XDECREF(old);
        }

    }
    {
        bool tmp_condition_result_9;
        PyObject *tmp_compexpr_left_7;
        PyObject *tmp_compexpr_right_7;
        CHECK_OBJECT(var_attr);
        tmp_compexpr_left_7 = var_attr;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 856;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }

        tmp_compexpr_right_7 = var_attrs;
        tmp_res = PySequence_Contains(tmp_compexpr_right_7, tmp_compexpr_left_7);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 856;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
        tmp_condition_result_9 = (tmp_res == 1) ? true : false;
        if (tmp_condition_result_9 != false) {
            goto branch_yes_9;
        } else {
            goto branch_no_9;
        }
    }
    branch_yes_9:;
    {
        PyObject *tmp_ass_subvalue_1;
        PyObject *tmp_called_name_4;
        PyObject *tmp_expression_name_15;
        PyObject *tmp_args_element_name_8;
        PyObject *tmp_args_element_name_9;
        PyObject *tmp_args_element_name_10;
        PyObject *tmp_called_name_5;
        PyObject *tmp_args_element_name_11;
        PyObject *tmp_expression_name_16;
        PyObject *tmp_subscript_name_4;
        PyObject *tmp_ass_subscribed_1;
        PyObject *tmp_ass_subscript_1;
        tmp_expression_name_15 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_expression_name_15 == NULL)) {
            tmp_expression_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_expression_name_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 857;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
        tmp_called_name_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_15, mod_consts[38]);
        if (tmp_called_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 857;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
        tmp_args_element_name_8 = mod_consts[53];
        tmp_args_element_name_9 = mod_consts[54];
        tmp_called_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[41]);

        if (unlikely(tmp_called_name_5 == NULL)) {
            tmp_called_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[41]);
        }

        if (tmp_called_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_4);

            exception_lineno = 859;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
        if (var_attrs == NULL) {
            Py_DECREF(tmp_called_name_4);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 859;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }

        tmp_expression_name_16 = var_attrs;
        CHECK_OBJECT(var_attr);
        tmp_subscript_name_4 = var_attr;
        tmp_args_element_name_11 = LOOKUP_SUBSCRIPT(tmp_expression_name_16, tmp_subscript_name_4);
        if (tmp_args_element_name_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_4);

            exception_lineno = 859;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 859;
        tmp_args_element_name_10 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_5, tmp_args_element_name_11);
        Py_DECREF(tmp_args_element_name_11);
        if (tmp_args_element_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_4);

            exception_lineno = 859;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 857;
        {
            PyObject *call_args[] = {tmp_args_element_name_8, tmp_args_element_name_9, tmp_args_element_name_10};
            tmp_ass_subvalue_1 = CALL_FUNCTION_WITH_ARGS3(tmp_called_name_4, call_args);
        }

        Py_DECREF(tmp_called_name_4);
        Py_DECREF(tmp_args_element_name_10);
        if (tmp_ass_subvalue_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 857;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
        if (var_attrs == NULL) {
            Py_DECREF(tmp_ass_subvalue_1);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 857;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }

        tmp_ass_subscribed_1 = var_attrs;
        CHECK_OBJECT(var_attr);
        tmp_ass_subscript_1 = var_attr;
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_1, tmp_ass_subscript_1, tmp_ass_subvalue_1);
        Py_DECREF(tmp_ass_subvalue_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 857;
            type_description_1 = "ooooooooo";
            goto try_except_handler_6;
        }
    }
    branch_no_9:;
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 855;
        type_description_1 = "ooooooooo";
        goto try_except_handler_6;
    }
    goto loop_start_3;
    loop_end_3:;
    goto try_end_5;
    // Exception handler code:
    try_except_handler_6:;
    exception_keeper_type_5 = exception_type;
    exception_keeper_value_5 = exception_value;
    exception_keeper_tb_5 = exception_tb;
    exception_keeper_lineno_5 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_for_loop_3__iter_value);
    tmp_for_loop_3__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_3__for_iterator);
    tmp_for_loop_3__for_iterator = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_5;
    exception_value = exception_keeper_value_5;
    exception_tb = exception_keeper_tb_5;
    exception_lineno = exception_keeper_lineno_5;

    goto frame_exception_exit_1;
    // End of try:
    try_end_5:;
    Py_XDECREF(tmp_for_loop_3__iter_value);
    tmp_for_loop_3__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_3__for_iterator);
    tmp_for_loop_3__for_iterator = NULL;
    {
        nuitka_bool tmp_condition_result_10;
        int tmp_and_left_truth_2;
        nuitka_bool tmp_and_left_value_2;
        nuitka_bool tmp_and_right_value_2;
        PyObject *tmp_compexpr_left_8;
        PyObject *tmp_compexpr_right_8;
        PyObject *tmp_expression_name_17;
        PyObject *tmp_subscript_name_5;
        PyObject *tmp_expression_name_18;
        int tmp_and_left_truth_3;
        nuitka_bool tmp_and_left_value_3;
        nuitka_bool tmp_and_right_value_3;
        PyObject *tmp_compexpr_left_9;
        PyObject *tmp_compexpr_right_9;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_expression_name_19;
        PyObject *tmp_subscript_name_6;
        PyObject *tmp_called_name_6;
        PyObject *tmp_expression_name_20;
        PyObject *tmp_call_result_2;
        PyObject *tmp_args_element_name_12;
        PyObject *tmp_args_element_name_13;
        PyObject *tmp_expression_name_21;
        PyObject *tmp_subscript_name_7;
        PyObject *tmp_tuple_element_2;
        PyObject *tmp_expression_name_22;
        PyObject *tmp_subscript_name_8;
        int tmp_truth_name_4;
        if (par_token == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 860;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_17 = par_token;
        tmp_subscript_name_5 = mod_consts[24];
        tmp_compexpr_left_8 = LOOKUP_SUBSCRIPT(tmp_expression_name_17, tmp_subscript_name_5);
        if (tmp_compexpr_left_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 860;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        if (par_self == NULL) {
            Py_DECREF(tmp_compexpr_left_8);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 860;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_18 = par_self;
        tmp_compexpr_right_8 = LOOKUP_ATTRIBUTE(tmp_expression_name_18, mod_consts[15]);
        if (tmp_compexpr_right_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_compexpr_left_8);

            exception_lineno = 860;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_8, tmp_compexpr_left_8);
        Py_DECREF(tmp_compexpr_left_8);
        Py_DECREF(tmp_compexpr_right_8);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 860;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_and_left_value_2 = (tmp_res == 1) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        tmp_and_left_truth_2 = tmp_and_left_value_2 == NUITKA_BOOL_TRUE ? 1 : 0;
        if (tmp_and_left_truth_2 == 1) {
            goto and_right_2;
        } else {
            goto and_left_2;
        }
        and_right_2:;
        tmp_expression_name_19 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

        if (unlikely(tmp_expression_name_19 == NULL)) {
            tmp_expression_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
        }

        if (tmp_expression_name_19 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_subscript_name_6 = mod_consts[55];
        tmp_tuple_element_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_19, tmp_subscript_name_6);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_compexpr_left_9 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_compexpr_left_9, 0, tmp_tuple_element_1);
        tmp_tuple_element_1 = mod_consts[56];
        PyTuple_SET_ITEM0(tmp_compexpr_left_9, 1, tmp_tuple_element_1);
        if (var_attrs == NULL) {
            Py_DECREF(tmp_compexpr_left_9);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_compexpr_right_9 = var_attrs;
        tmp_res = PySequence_Contains(tmp_compexpr_right_9, tmp_compexpr_left_9);
        Py_DECREF(tmp_compexpr_left_9);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_and_left_value_3 = (tmp_res == 1) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        tmp_and_left_truth_3 = tmp_and_left_value_3 == NUITKA_BOOL_TRUE ? 1 : 0;
        if (tmp_and_left_truth_3 == 1) {
            goto and_right_3;
        } else {
            goto and_left_3;
        }
        and_right_3:;
        tmp_expression_name_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_expression_name_20 == NULL)) {
            tmp_expression_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_expression_name_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_called_name_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_20, mod_consts[57]);
        if (tmp_called_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_12 = mod_consts[58];
        if (var_attrs == NULL) {
            Py_DECREF(tmp_called_name_6);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 862;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_21 = var_attrs;
        tmp_expression_name_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

        if (unlikely(tmp_expression_name_22 == NULL)) {
            tmp_expression_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
        }

        if (tmp_expression_name_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_6);

            exception_lineno = 862;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_subscript_name_8 = mod_consts[55];
        tmp_tuple_element_2 = LOOKUP_SUBSCRIPT(tmp_expression_name_22, tmp_subscript_name_8);
        if (tmp_tuple_element_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_6);

            exception_lineno = 862;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_subscript_name_7 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_subscript_name_7, 0, tmp_tuple_element_2);
        tmp_tuple_element_2 = mod_consts[56];
        PyTuple_SET_ITEM0(tmp_subscript_name_7, 1, tmp_tuple_element_2);
        tmp_args_element_name_13 = LOOKUP_SUBSCRIPT(tmp_expression_name_21, tmp_subscript_name_7);
        Py_DECREF(tmp_subscript_name_7);
        if (tmp_args_element_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_6);

            exception_lineno = 862;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 861;
        {
            PyObject *call_args[] = {tmp_args_element_name_12, tmp_args_element_name_13};
            tmp_call_result_2 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_6, call_args);
        }

        Py_DECREF(tmp_called_name_6);
        Py_DECREF(tmp_args_element_name_13);
        if (tmp_call_result_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_truth_name_4 = CHECK_IF_TRUE(tmp_call_result_2);
        if (tmp_truth_name_4 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_call_result_2);

            exception_lineno = 861;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_and_right_value_3 = tmp_truth_name_4 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_call_result_2);
        tmp_and_right_value_2 = tmp_and_right_value_3;
        goto and_end_3;
        and_left_3:;
        tmp_and_right_value_2 = tmp_and_left_value_3;
        and_end_3:;
        tmp_condition_result_10 = tmp_and_right_value_2;
        goto and_end_2;
        and_left_2:;
        tmp_condition_result_10 = tmp_and_left_value_2;
        and_end_2:;
        if (tmp_condition_result_10 == NUITKA_BOOL_TRUE) {
            goto branch_yes_10;
        } else {
            goto branch_no_10;
        }
    }
    branch_yes_10:;
    {
        PyObject *tmp_delsubscr_target_6;
        PyObject *tmp_delsubscr_subscript_6;
        PyObject *tmp_tuple_element_3;
        PyObject *tmp_expression_name_23;
        PyObject *tmp_subscript_name_9;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 863;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_delsubscr_target_6 = var_attrs;
        tmp_expression_name_23 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

        if (unlikely(tmp_expression_name_23 == NULL)) {
            tmp_expression_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
        }

        if (tmp_expression_name_23 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 863;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_subscript_name_9 = mod_consts[55];
        tmp_tuple_element_3 = LOOKUP_SUBSCRIPT(tmp_expression_name_23, tmp_subscript_name_9);
        if (tmp_tuple_element_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 863;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_delsubscr_subscript_6 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_delsubscr_subscript_6, 0, tmp_tuple_element_3);
        tmp_tuple_element_3 = mod_consts[56];
        PyTuple_SET_ITEM0(tmp_delsubscr_subscript_6, 1, tmp_tuple_element_3);
        tmp_result = DEL_SUBSCRIPT(tmp_delsubscr_target_6, tmp_delsubscr_subscript_6);
        Py_DECREF(tmp_delsubscr_subscript_6);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 863;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
    }
    branch_no_10:;
    {
        bool tmp_condition_result_11;
        PyObject *tmp_compexpr_left_10;
        PyObject *tmp_compexpr_right_10;
        tmp_compexpr_left_10 = mod_consts[59];
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 864;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_compexpr_right_10 = var_attrs;
        tmp_res = PySequence_Contains(tmp_compexpr_right_10, tmp_compexpr_left_10);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 864;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_11 = (tmp_res == 1) ? true : false;
        if (tmp_condition_result_11 != false) {
            goto branch_yes_11;
        } else {
            goto branch_no_11;
        }
    }
    branch_yes_11:;
    {
        PyObject *tmp_ass_subvalue_2;
        PyObject *tmp_called_name_7;
        PyObject *tmp_expression_name_24;
        PyObject *tmp_args_element_name_14;
        PyObject *tmp_expression_name_25;
        PyObject *tmp_subscript_name_10;
        PyObject *tmp_ass_subscribed_2;
        PyObject *tmp_ass_subscript_2;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 865;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_24 = par_self;
        tmp_called_name_7 = LOOKUP_ATTRIBUTE(tmp_expression_name_24, mod_consts[60]);
        if (tmp_called_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 865;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        if (var_attrs == NULL) {
            Py_DECREF(tmp_called_name_7);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 865;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_25 = var_attrs;
        tmp_subscript_name_10 = mod_consts[59];
        tmp_args_element_name_14 = LOOKUP_SUBSCRIPT(tmp_expression_name_25, tmp_subscript_name_10);
        if (tmp_args_element_name_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_7);

            exception_lineno = 865;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame.f_lineno = 865;
        tmp_ass_subvalue_2 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_7, tmp_args_element_name_14);
        Py_DECREF(tmp_called_name_7);
        Py_DECREF(tmp_args_element_name_14);
        if (tmp_ass_subvalue_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 865;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
        if (var_attrs == NULL) {
            Py_DECREF(tmp_ass_subvalue_2);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 865;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_ass_subscribed_2 = var_attrs;
        tmp_ass_subscript_2 = mod_consts[59];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_2, tmp_ass_subscript_2, tmp_ass_subvalue_2);
        Py_DECREF(tmp_ass_subvalue_2);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 865;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
    }
    branch_no_11:;
    {
        PyObject *tmp_ass_subvalue_3;
        PyObject *tmp_ass_subscribed_3;
        PyObject *tmp_ass_subscript_3;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 866;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_ass_subvalue_3 = var_attrs;
        if (par_token == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 866;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }

        tmp_ass_subscribed_3 = par_token;
        tmp_ass_subscript_3 = mod_consts[31];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_3, tmp_ass_subscript_3, tmp_ass_subvalue_3);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 866;
            type_description_1 = "ooooooooo";
            goto frame_exception_exit_1;
        }
    }
    branch_no_1:;
    if (par_token == NULL) {

        FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
        exception_tb = NULL;
        NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
        CHAIN_EXCEPTION(exception_value);

        exception_lineno = 867;
        type_description_1 = "ooooooooo";
        goto frame_exception_exit_1;
    }

    tmp_return_value = par_token;
    Py_INCREF(tmp_return_value);
    goto frame_return_exit_1;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_fde2c08ac2458506996a6f1c4f06cd03);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_fde2c08ac2458506996a6f1c4f06cd03);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_fde2c08ac2458506996a6f1c4f06cd03);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_fde2c08ac2458506996a6f1c4f06cd03, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_fde2c08ac2458506996a6f1c4f06cd03->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_fde2c08ac2458506996a6f1c4f06cd03, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_fde2c08ac2458506996a6f1c4f06cd03,
        type_description_1,
        par_self,
        par_token,
        var_attrs,
        var_attr_names,
        var_to_remove,
        var_attr,
        var_val_unescaped,
        var_uri,
        var_m
    );


    // Release cached frame if used for exception.
    if (frame_fde2c08ac2458506996a6f1c4f06cd03 == cache_frame_fde2c08ac2458506996a6f1c4f06cd03) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_fde2c08ac2458506996a6f1c4f06cd03);
        cache_frame_fde2c08ac2458506996a6f1c4f06cd03 = NULL;
    }

    assertFrameObject(frame_fde2c08ac2458506996a6f1c4f06cd03);

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
    Py_XDECREF(par_token);
    par_token = NULL;
    Py_XDECREF(var_attrs);
    var_attrs = NULL;
    Py_XDECREF(var_attr_names);
    var_attr_names = NULL;
    Py_XDECREF(var_to_remove);
    var_to_remove = NULL;
    Py_XDECREF(var_attr);
    var_attr = NULL;
    Py_XDECREF(var_val_unescaped);
    var_val_unescaped = NULL;
    Py_XDECREF(var_uri);
    var_uri = NULL;
    Py_XDECREF(var_m);
    var_m = NULL;
    goto function_return_exit;
    // Exception handler code:
    try_except_handler_1:;
    exception_keeper_type_6 = exception_type;
    exception_keeper_value_6 = exception_value;
    exception_keeper_tb_6 = exception_tb;
    exception_keeper_lineno_6 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_token);
    par_token = NULL;
    Py_XDECREF(var_attrs);
    var_attrs = NULL;
    Py_XDECREF(var_attr_names);
    var_attr_names = NULL;
    Py_XDECREF(var_to_remove);
    var_to_remove = NULL;
    Py_XDECREF(var_attr);
    var_attr = NULL;
    Py_XDECREF(var_val_unescaped);
    var_val_unescaped = NULL;
    Py_XDECREF(var_uri);
    var_uri = NULL;
    Py_XDECREF(var_m);
    var_m = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_6;
    exception_value = exception_keeper_value_6;
    exception_tb = exception_keeper_tb_6;
    exception_lineno = exception_keeper_lineno_6;

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


static PyObject *impl_pip$_vendor$html5lib$filters$sanitizer$$$function__5_disallowed_token(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_token = python_pars[1];
    PyObject *var_token_type = NULL;
    PyObject *var_attrs = NULL;
    PyObject *var_ns = NULL;
    PyObject *var_name = NULL;
    PyObject *var_v = NULL;
    PyObject *tmp_for_loop_1__for_iterator = NULL;
    PyObject *tmp_for_loop_1__iter_value = NULL;
    PyObject *tmp_tuple_unpack_1__element_1 = NULL;
    PyObject *tmp_tuple_unpack_1__element_2 = NULL;
    PyObject *tmp_tuple_unpack_1__source_iter = NULL;
    PyObject *tmp_tuple_unpack_2__element_1 = NULL;
    PyObject *tmp_tuple_unpack_2__element_2 = NULL;
    PyObject *tmp_tuple_unpack_2__source_iter = NULL;
    struct Nuitka_FrameObject *frame_17727f777de2e1825ac7bfbd7d4c5b42;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    bool tmp_result;
    int tmp_res;
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
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    PyObject *exception_keeper_type_5;
    PyObject *exception_keeper_value_5;
    PyTracebackObject *exception_keeper_tb_5;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_5;
    PyObject *tmp_return_value = NULL;
    static struct Nuitka_FrameObject *cache_frame_17727f777de2e1825ac7bfbd7d4c5b42 = NULL;
    PyObject *exception_keeper_type_6;
    PyObject *exception_keeper_value_6;
    PyTracebackObject *exception_keeper_tb_6;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_6;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_17727f777de2e1825ac7bfbd7d4c5b42)) {
        Py_XDECREF(cache_frame_17727f777de2e1825ac7bfbd7d4c5b42);

#if _DEBUG_REFCOUNTS
        if (cache_frame_17727f777de2e1825ac7bfbd7d4c5b42 == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_17727f777de2e1825ac7bfbd7d4c5b42 = MAKE_FUNCTION_FRAME(codeobj_17727f777de2e1825ac7bfbd7d4c5b42, module_pip$_vendor$html5lib$filters$sanitizer, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_17727f777de2e1825ac7bfbd7d4c5b42->m_type_description == NULL);
    frame_17727f777de2e1825ac7bfbd7d4c5b42 = cache_frame_17727f777de2e1825ac7bfbd7d4c5b42;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_17727f777de2e1825ac7bfbd7d4c5b42);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_17727f777de2e1825ac7bfbd7d4c5b42) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_subscript_name_1;
        CHECK_OBJECT(par_token);
        tmp_expression_name_1 = par_token;
        tmp_subscript_name_1 = mod_consts[22];
        tmp_assign_source_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_1, tmp_subscript_name_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 870;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        assert(var_token_type == NULL);
        var_token_type = tmp_assign_source_1;
    }
    {
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        PyObject *tmp_tmp_condition_result_1_object_1;
        int tmp_truth_name_1;
        CHECK_OBJECT(var_token_type);
        tmp_compexpr_left_1 = var_token_type;
        tmp_compexpr_right_1 = mod_consts[61];
        tmp_tmp_condition_result_1_object_1 = RICH_COMPARE_EQ_OBJECT_OBJECT_OBJECT(tmp_compexpr_left_1, tmp_compexpr_right_1);
        if (tmp_tmp_condition_result_1_object_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 871;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_truth_name_1 = CHECK_IF_TRUE(tmp_tmp_condition_result_1_object_1);
        if (tmp_truth_name_1 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_tmp_condition_result_1_object_1);

            exception_lineno = 871;
            type_description_1 = "ooooooo";
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
        PyObject *tmp_ass_subvalue_1;
        PyObject *tmp_left_name_1;
        PyObject *tmp_right_name_1;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_subscript_name_2;
        PyObject *tmp_ass_subscribed_1;
        PyObject *tmp_ass_subscript_1;
        tmp_left_name_1 = mod_consts[62];
        CHECK_OBJECT(par_token);
        tmp_expression_name_2 = par_token;
        tmp_subscript_name_2 = mod_consts[24];
        tmp_right_name_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_2, tmp_subscript_name_2);
        if (tmp_right_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 872;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_ass_subvalue_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_OBJECT(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_ass_subvalue_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 872;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_token);
        tmp_ass_subscribed_1 = par_token;
        tmp_ass_subscript_1 = mod_consts[31];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_1, tmp_ass_subscript_1, tmp_ass_subvalue_1);
        Py_DECREF(tmp_ass_subvalue_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 872;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
    }
    goto branch_end_1;
    branch_no_1:;
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_expression_name_3;
        PyObject *tmp_subscript_name_3;
        PyObject *tmp_subscript_result_1;
        int tmp_truth_name_2;
        CHECK_OBJECT(par_token);
        tmp_expression_name_3 = par_token;
        tmp_subscript_name_3 = mod_consts[31];
        tmp_subscript_result_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_3, tmp_subscript_name_3);
        if (tmp_subscript_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 873;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_truth_name_2 = CHECK_IF_TRUE(tmp_subscript_result_1);
        if (tmp_truth_name_2 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_subscript_result_1);

            exception_lineno = 873;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_2 = tmp_truth_name_2 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_subscript_result_1);
        if (tmp_condition_result_2 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        bool tmp_condition_result_3;
        PyObject *tmp_compexpr_left_2;
        PyObject *tmp_compexpr_right_2;
        CHECK_OBJECT(var_token_type);
        tmp_compexpr_left_2 = var_token_type;
        tmp_compexpr_right_2 = mod_consts[63];
        tmp_res = PySequence_Contains(tmp_compexpr_right_2, tmp_compexpr_left_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 874;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_3 = (tmp_res == 0) ? true : false;
        if (tmp_condition_result_3 != false) {
            goto branch_yes_3;
        } else {
            goto branch_no_3;
        }
    }
    branch_yes_3:;
    {
        PyObject *tmp_raise_type_1;
        tmp_raise_type_1 = PyExc_AssertionError;
        exception_type = tmp_raise_type_1;
        Py_INCREF(tmp_raise_type_1);
        exception_lineno = 874;
        RAISE_EXCEPTION_WITH_TYPE(&exception_type, &exception_value, &exception_tb);
        type_description_1 = "ooooooo";
        goto frame_exception_exit_1;
    }
    branch_no_3:;
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = PyList_New(0);
        assert(var_attrs == NULL);
        var_attrs = tmp_assign_source_2;
    }
    {
        PyObject *tmp_assign_source_3;
        PyObject *tmp_iter_arg_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_expression_name_4;
        PyObject *tmp_subscript_name_4;
        CHECK_OBJECT(par_token);
        tmp_expression_name_4 = par_token;
        tmp_subscript_name_4 = mod_consts[31];
        tmp_called_instance_1 = LOOKUP_SUBSCRIPT(tmp_expression_name_4, tmp_subscript_name_4);
        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 876;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        frame_17727f777de2e1825ac7bfbd7d4c5b42->m_frame.f_lineno = 876;
        tmp_iter_arg_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_1, mod_consts[64]);
        Py_DECREF(tmp_called_instance_1);
        if (tmp_iter_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 876;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_3 = MAKE_ITERATOR(tmp_iter_arg_1);
        Py_DECREF(tmp_iter_arg_1);
        if (tmp_assign_source_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 876;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        assert(tmp_for_loop_1__for_iterator == NULL);
        tmp_for_loop_1__for_iterator = tmp_assign_source_3;
    }
    // Tried code:
    loop_start_1:;
    {
        PyObject *tmp_next_source_1;
        PyObject *tmp_assign_source_4;
        CHECK_OBJECT(tmp_for_loop_1__for_iterator);
        tmp_next_source_1 = tmp_for_loop_1__for_iterator;
        tmp_assign_source_4 = ITERATOR_NEXT(tmp_next_source_1);
        if (tmp_assign_source_4 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_1;
            } else {

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                type_description_1 = "ooooooo";
                exception_lineno = 876;
                goto try_except_handler_2;
            }
        }

        {
            PyObject *old = tmp_for_loop_1__iter_value;
            tmp_for_loop_1__iter_value = tmp_assign_source_4;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_iter_arg_2;
        CHECK_OBJECT(tmp_for_loop_1__iter_value);
        tmp_iter_arg_2 = tmp_for_loop_1__iter_value;
        tmp_assign_source_5 = MAKE_ITERATOR(tmp_iter_arg_2);
        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 876;
            type_description_1 = "ooooooo";
            goto try_except_handler_3;
        }
        {
            PyObject *old = tmp_tuple_unpack_1__source_iter;
            tmp_tuple_unpack_1__source_iter = tmp_assign_source_5;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_unpack_1;
        CHECK_OBJECT(tmp_tuple_unpack_1__source_iter);
        tmp_unpack_1 = tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_6 = UNPACK_NEXT(tmp_unpack_1, 0, 2);
        if (tmp_assign_source_6 == NULL) {
            if (!ERROR_OCCURRED()) {
                exception_type = PyExc_StopIteration;
                Py_INCREF(exception_type);
                exception_value = NULL;
                exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            }


            type_description_1 = "ooooooo";
            exception_lineno = 876;
            goto try_except_handler_4;
        }
        {
            PyObject *old = tmp_tuple_unpack_1__element_1;
            tmp_tuple_unpack_1__element_1 = tmp_assign_source_6;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_7;
        PyObject *tmp_unpack_2;
        CHECK_OBJECT(tmp_tuple_unpack_1__source_iter);
        tmp_unpack_2 = tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_7 = UNPACK_NEXT(tmp_unpack_2, 1, 2);
        if (tmp_assign_source_7 == NULL) {
            if (!ERROR_OCCURRED()) {
                exception_type = PyExc_StopIteration;
                Py_INCREF(exception_type);
                exception_value = NULL;
                exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            }


            type_description_1 = "ooooooo";
            exception_lineno = 876;
            goto try_except_handler_4;
        }
        {
            PyObject *old = tmp_tuple_unpack_1__element_2;
            tmp_tuple_unpack_1__element_2 = tmp_assign_source_7;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_iterator_name_1;
        CHECK_OBJECT(tmp_tuple_unpack_1__source_iter);
        tmp_iterator_name_1 = tmp_tuple_unpack_1__source_iter;
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

                    type_description_1 = "ooooooo";
                    exception_lineno = 876;
                    goto try_except_handler_4;
                }
            }
        } else {
            Py_DECREF(tmp_iterator_attempt);

            exception_type = PyExc_ValueError;
            Py_INCREF(PyExc_ValueError);
            exception_value = mod_consts[65];
            Py_INCREF(exception_value);
            exception_tb = NULL;

            type_description_1 = "ooooooo";
            exception_lineno = 876;
            goto try_except_handler_4;
        }
    }
    goto try_end_1;
    // Exception handler code:
    try_except_handler_4:;
    exception_keeper_type_1 = exception_type;
    exception_keeper_value_1 = exception_value;
    exception_keeper_tb_1 = exception_tb;
    exception_keeper_lineno_1 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_tuple_unpack_1__source_iter);
    tmp_tuple_unpack_1__source_iter = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

    goto try_except_handler_3;
    // End of try:
    try_end_1:;
    Py_XDECREF(tmp_tuple_unpack_1__source_iter);
    tmp_tuple_unpack_1__source_iter = NULL;
    // Tried code:
    {
        PyObject *tmp_assign_source_8;
        PyObject *tmp_iter_arg_3;
        CHECK_OBJECT(tmp_tuple_unpack_1__element_1);
        tmp_iter_arg_3 = tmp_tuple_unpack_1__element_1;
        tmp_assign_source_8 = MAKE_ITERATOR(tmp_iter_arg_3);
        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 876;
            type_description_1 = "ooooooo";
            goto try_except_handler_5;
        }
        {
            PyObject *old = tmp_tuple_unpack_2__source_iter;
            tmp_tuple_unpack_2__source_iter = tmp_assign_source_8;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_9;
        PyObject *tmp_unpack_3;
        CHECK_OBJECT(tmp_tuple_unpack_2__source_iter);
        tmp_unpack_3 = tmp_tuple_unpack_2__source_iter;
        tmp_assign_source_9 = UNPACK_NEXT(tmp_unpack_3, 0, 2);
        if (tmp_assign_source_9 == NULL) {
            if (!ERROR_OCCURRED()) {
                exception_type = PyExc_StopIteration;
                Py_INCREF(exception_type);
                exception_value = NULL;
                exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            }


            type_description_1 = "ooooooo";
            exception_lineno = 876;
            goto try_except_handler_6;
        }
        {
            PyObject *old = tmp_tuple_unpack_2__element_1;
            tmp_tuple_unpack_2__element_1 = tmp_assign_source_9;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_10;
        PyObject *tmp_unpack_4;
        CHECK_OBJECT(tmp_tuple_unpack_2__source_iter);
        tmp_unpack_4 = tmp_tuple_unpack_2__source_iter;
        tmp_assign_source_10 = UNPACK_NEXT(tmp_unpack_4, 1, 2);
        if (tmp_assign_source_10 == NULL) {
            if (!ERROR_OCCURRED()) {
                exception_type = PyExc_StopIteration;
                Py_INCREF(exception_type);
                exception_value = NULL;
                exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            }


            type_description_1 = "ooooooo";
            exception_lineno = 876;
            goto try_except_handler_6;
        }
        {
            PyObject *old = tmp_tuple_unpack_2__element_2;
            tmp_tuple_unpack_2__element_2 = tmp_assign_source_10;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_iterator_name_2;
        CHECK_OBJECT(tmp_tuple_unpack_2__source_iter);
        tmp_iterator_name_2 = tmp_tuple_unpack_2__source_iter;
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

                    type_description_1 = "ooooooo";
                    exception_lineno = 876;
                    goto try_except_handler_6;
                }
            }
        } else {
            Py_DECREF(tmp_iterator_attempt);

            exception_type = PyExc_ValueError;
            Py_INCREF(PyExc_ValueError);
            exception_value = mod_consts[65];
            Py_INCREF(exception_value);
            exception_tb = NULL;

            type_description_1 = "ooooooo";
            exception_lineno = 876;
            goto try_except_handler_6;
        }
    }
    goto try_end_2;
    // Exception handler code:
    try_except_handler_6:;
    exception_keeper_type_2 = exception_type;
    exception_keeper_value_2 = exception_value;
    exception_keeper_tb_2 = exception_tb;
    exception_keeper_lineno_2 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_tuple_unpack_2__source_iter);
    tmp_tuple_unpack_2__source_iter = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_2;
    exception_value = exception_keeper_value_2;
    exception_tb = exception_keeper_tb_2;
    exception_lineno = exception_keeper_lineno_2;

    goto try_except_handler_5;
    // End of try:
    try_end_2:;
    goto try_end_3;
    // Exception handler code:
    try_except_handler_5:;
    exception_keeper_type_3 = exception_type;
    exception_keeper_value_3 = exception_value;
    exception_keeper_tb_3 = exception_tb;
    exception_keeper_lineno_3 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_tuple_unpack_2__element_1);
    tmp_tuple_unpack_2__element_1 = NULL;
    Py_XDECREF(tmp_tuple_unpack_2__element_2);
    tmp_tuple_unpack_2__element_2 = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_3;
    exception_value = exception_keeper_value_3;
    exception_tb = exception_keeper_tb_3;
    exception_lineno = exception_keeper_lineno_3;

    goto try_except_handler_3;
    // End of try:
    try_end_3:;
    goto try_end_4;
    // Exception handler code:
    try_except_handler_3:;
    exception_keeper_type_4 = exception_type;
    exception_keeper_value_4 = exception_value;
    exception_keeper_tb_4 = exception_tb;
    exception_keeper_lineno_4 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_tuple_unpack_1__element_1);
    tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(tmp_tuple_unpack_1__element_2);
    tmp_tuple_unpack_1__element_2 = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_4;
    exception_value = exception_keeper_value_4;
    exception_tb = exception_keeper_tb_4;
    exception_lineno = exception_keeper_lineno_4;

    goto try_except_handler_2;
    // End of try:
    try_end_4:;
    Py_XDECREF(tmp_tuple_unpack_2__source_iter);
    tmp_tuple_unpack_2__source_iter = NULL;
    {
        PyObject *tmp_assign_source_11;
        CHECK_OBJECT(tmp_tuple_unpack_2__element_1);
        tmp_assign_source_11 = tmp_tuple_unpack_2__element_1;
        {
            PyObject *old = var_ns;
            var_ns = tmp_assign_source_11;
            Py_INCREF(var_ns);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(tmp_tuple_unpack_2__element_1);
    tmp_tuple_unpack_2__element_1 = NULL;

    {
        PyObject *tmp_assign_source_12;
        CHECK_OBJECT(tmp_tuple_unpack_2__element_2);
        tmp_assign_source_12 = tmp_tuple_unpack_2__element_2;
        {
            PyObject *old = var_name;
            var_name = tmp_assign_source_12;
            Py_INCREF(var_name);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(tmp_tuple_unpack_2__element_2);
    tmp_tuple_unpack_2__element_2 = NULL;

    Py_XDECREF(tmp_tuple_unpack_2__element_1);
    tmp_tuple_unpack_2__element_1 = NULL;
    Py_XDECREF(tmp_tuple_unpack_2__element_2);
    tmp_tuple_unpack_2__element_2 = NULL;
    Py_XDECREF(tmp_tuple_unpack_1__element_1);
    tmp_tuple_unpack_1__element_1 = NULL;

    {
        PyObject *tmp_assign_source_13;
        CHECK_OBJECT(tmp_tuple_unpack_1__element_2);
        tmp_assign_source_13 = tmp_tuple_unpack_1__element_2;
        {
            PyObject *old = var_v;
            var_v = tmp_assign_source_13;
            Py_INCREF(var_v);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(tmp_tuple_unpack_1__element_2);
    tmp_tuple_unpack_1__element_2 = NULL;

    Py_XDECREF(tmp_tuple_unpack_1__element_1);
    tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(tmp_tuple_unpack_1__element_2);
    tmp_tuple_unpack_1__element_2 = NULL;
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_5;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_left_name_2;
        PyObject *tmp_right_name_2;
        PyObject *tmp_tuple_element_1;
        bool tmp_condition_result_4;
        PyObject *tmp_compexpr_left_3;
        PyObject *tmp_compexpr_right_3;
        PyObject *tmp_left_name_3;
        PyObject *tmp_right_name_3;
        PyObject *tmp_tuple_element_2;
        PyObject *tmp_expression_name_6;
        PyObject *tmp_subscript_name_5;
        if (var_attrs == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 877;
            type_description_1 = "ooooooo";
            goto try_except_handler_2;
        }

        tmp_expression_name_5 = var_attrs;
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_5, mod_consts[66]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 877;
            type_description_1 = "ooooooo";
            goto try_except_handler_2;
        }
        tmp_left_name_2 = mod_consts[67];
        CHECK_OBJECT(var_ns);
        tmp_compexpr_left_3 = var_ns;
        tmp_compexpr_right_3 = Py_None;
        tmp_condition_result_4 = (tmp_compexpr_left_3 == tmp_compexpr_right_3) ? true : false;
        if (tmp_condition_result_4 != false) {
            goto condexpr_true_1;
        } else {
            goto condexpr_false_1;
        }
        condexpr_true_1:;
        CHECK_OBJECT(var_name);
        tmp_tuple_element_1 = var_name;
        Py_INCREF(tmp_tuple_element_1);
        goto condexpr_end_1;
        condexpr_false_1:;
        tmp_left_name_3 = mod_consts[68];
        tmp_expression_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[69]);

        if (unlikely(tmp_expression_name_6 == NULL)) {
            tmp_expression_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[69]);
        }

        if (tmp_expression_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 877;
            type_description_1 = "ooooooo";
            goto try_except_handler_2;
        }
        CHECK_OBJECT(var_ns);
        tmp_subscript_name_5 = var_ns;
        tmp_tuple_element_2 = LOOKUP_SUBSCRIPT(tmp_expression_name_6, tmp_subscript_name_5);
        if (tmp_tuple_element_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 877;
            type_description_1 = "ooooooo";
            goto try_except_handler_2;
        }
        tmp_right_name_3 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_right_name_3, 0, tmp_tuple_element_2);
        CHECK_OBJECT(var_name);
        tmp_tuple_element_2 = var_name;
        PyTuple_SET_ITEM0(tmp_right_name_3, 1, tmp_tuple_element_2);
        tmp_tuple_element_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_3, tmp_right_name_3);
        Py_DECREF(tmp_right_name_3);
        if (tmp_tuple_element_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 877;
            type_description_1 = "ooooooo";
            goto try_except_handler_2;
        }
        condexpr_end_1:;
        tmp_right_name_2 = PyTuple_New(2);
        {
            PyObject *tmp_called_name_2;
            PyObject *tmp_args_element_name_2;
            PyTuple_SET_ITEM(tmp_right_name_2, 0, tmp_tuple_element_1);
            tmp_called_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[70]);

            if (unlikely(tmp_called_name_2 == NULL)) {
                tmp_called_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[70]);
            }

            if (tmp_called_name_2 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 877;
                type_description_1 = "ooooooo";
                goto tuple_build_exception_1;
            }
            CHECK_OBJECT(var_v);
            tmp_args_element_name_2 = var_v;
            frame_17727f777de2e1825ac7bfbd7d4c5b42->m_frame.f_lineno = 877;
            tmp_tuple_element_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_2, tmp_args_element_name_2);
            if (tmp_tuple_element_1 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 877;
                type_description_1 = "ooooooo";
                goto tuple_build_exception_1;
            }
            PyTuple_SET_ITEM(tmp_right_name_2, 1, tmp_tuple_element_1);
        }
        goto tuple_build_noexception_1;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_called_name_1);
        Py_DECREF(tmp_right_name_2);
        goto try_except_handler_2;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_1:;
        tmp_args_element_name_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_2, tmp_right_name_2);
        Py_DECREF(tmp_right_name_2);
        if (tmp_args_element_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 877;
            type_description_1 = "ooooooo";
            goto try_except_handler_2;
        }
        frame_17727f777de2e1825ac7bfbd7d4c5b42->m_frame.f_lineno = 877;
        tmp_call_result_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_1, tmp_args_element_name_1);
        Py_DECREF(tmp_called_name_1);
        Py_DECREF(tmp_args_element_name_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 877;
            type_description_1 = "ooooooo";
            goto try_except_handler_2;
        }
        Py_DECREF(tmp_call_result_1);
    }
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 876;
        type_description_1 = "ooooooo";
        goto try_except_handler_2;
    }
    goto loop_start_1;
    loop_end_1:;
    goto try_end_5;
    // Exception handler code:
    try_except_handler_2:;
    exception_keeper_type_5 = exception_type;
    exception_keeper_value_5 = exception_value;
    exception_keeper_tb_5 = exception_tb;
    exception_keeper_lineno_5 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_for_loop_1__iter_value);
    tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_1__for_iterator);
    tmp_for_loop_1__for_iterator = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_5;
    exception_value = exception_keeper_value_5;
    exception_tb = exception_keeper_tb_5;
    exception_lineno = exception_keeper_lineno_5;

    goto frame_exception_exit_1;
    // End of try:
    try_end_5:;
    Py_XDECREF(tmp_for_loop_1__iter_value);
    tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_1__for_iterator);
    tmp_for_loop_1__for_iterator = NULL;
    {
        PyObject *tmp_ass_subvalue_2;
        PyObject *tmp_left_name_4;
        PyObject *tmp_right_name_4;
        PyObject *tmp_tuple_element_3;
        PyObject *tmp_expression_name_7;
        PyObject *tmp_subscript_name_6;
        PyObject *tmp_ass_subscribed_2;
        PyObject *tmp_ass_subscript_2;
        tmp_left_name_4 = mod_consts[71];
        CHECK_OBJECT(par_token);
        tmp_expression_name_7 = par_token;
        tmp_subscript_name_6 = mod_consts[24];
        tmp_tuple_element_3 = LOOKUP_SUBSCRIPT(tmp_expression_name_7, tmp_subscript_name_6);
        if (tmp_tuple_element_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 878;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_right_name_4 = PyTuple_New(2);
        {
            PyObject *tmp_called_name_3;
            PyObject *tmp_expression_name_8;
            PyObject *tmp_args_element_name_3;
            PyTuple_SET_ITEM(tmp_right_name_4, 0, tmp_tuple_element_3);
            tmp_expression_name_8 = mod_consts[40];
            tmp_called_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_8, mod_consts[72]);
            assert(!(tmp_called_name_3 == NULL));
            if (var_attrs == NULL) {
                Py_DECREF(tmp_called_name_3);
                FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[36]);
                exception_tb = NULL;
                NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
                CHAIN_EXCEPTION(exception_value);

                exception_lineno = 878;
                type_description_1 = "ooooooo";
                goto tuple_build_exception_2;
            }

            tmp_args_element_name_3 = var_attrs;
            frame_17727f777de2e1825ac7bfbd7d4c5b42->m_frame.f_lineno = 878;
            tmp_tuple_element_3 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_3, tmp_args_element_name_3);
            Py_DECREF(tmp_called_name_3);
            if (tmp_tuple_element_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 878;
                type_description_1 = "ooooooo";
                goto tuple_build_exception_2;
            }
            PyTuple_SET_ITEM(tmp_right_name_4, 1, tmp_tuple_element_3);
        }
        goto tuple_build_noexception_2;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_2:;
        Py_DECREF(tmp_right_name_4);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_2:;
        tmp_ass_subvalue_2 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_4, tmp_right_name_4);
        Py_DECREF(tmp_right_name_4);
        if (tmp_ass_subvalue_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 878;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_token);
        tmp_ass_subscribed_2 = par_token;
        tmp_ass_subscript_2 = mod_consts[31];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_2, tmp_ass_subscript_2, tmp_ass_subvalue_2);
        Py_DECREF(tmp_ass_subvalue_2);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 878;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
    }
    goto branch_end_2;
    branch_no_2:;
    {
        PyObject *tmp_ass_subvalue_3;
        PyObject *tmp_left_name_5;
        PyObject *tmp_right_name_5;
        PyObject *tmp_expression_name_9;
        PyObject *tmp_subscript_name_7;
        PyObject *tmp_ass_subscribed_3;
        PyObject *tmp_ass_subscript_3;
        tmp_left_name_5 = mod_consts[73];
        CHECK_OBJECT(par_token);
        tmp_expression_name_9 = par_token;
        tmp_subscript_name_7 = mod_consts[24];
        tmp_right_name_5 = LOOKUP_SUBSCRIPT(tmp_expression_name_9, tmp_subscript_name_7);
        if (tmp_right_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 880;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_ass_subvalue_3 = BINARY_OPERATION_MOD_OBJECT_UNICODE_OBJECT(tmp_left_name_5, tmp_right_name_5);
        Py_DECREF(tmp_right_name_5);
        if (tmp_ass_subvalue_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 880;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        CHECK_OBJECT(par_token);
        tmp_ass_subscribed_3 = par_token;
        tmp_ass_subscript_3 = mod_consts[31];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_3, tmp_ass_subscript_3, tmp_ass_subvalue_3);
        Py_DECREF(tmp_ass_subvalue_3);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 880;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
    }
    branch_end_2:;
    branch_end_1:;
    {
        nuitka_bool tmp_condition_result_5;
        PyObject *tmp_called_instance_2;
        PyObject *tmp_call_result_2;
        int tmp_truth_name_3;
        if (par_token == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 881;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }

        tmp_called_instance_2 = par_token;
        frame_17727f777de2e1825ac7bfbd7d4c5b42->m_frame.f_lineno = 881;
        tmp_call_result_2 = CALL_METHOD_WITH_ARGS1(
            tmp_called_instance_2,
            mod_consts[74],
            &PyTuple_GET_ITEM(mod_consts[75], 0)
        );

        if (tmp_call_result_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 881;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_truth_name_3 = CHECK_IF_TRUE(tmp_call_result_2);
        if (tmp_truth_name_3 == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_call_result_2);

            exception_lineno = 881;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_5 = tmp_truth_name_3 == 0 ? NUITKA_BOOL_FALSE : NUITKA_BOOL_TRUE;
        Py_DECREF(tmp_call_result_2);
        if (tmp_condition_result_5 == NUITKA_BOOL_TRUE) {
            goto branch_yes_4;
        } else {
            goto branch_no_4;
        }
    }
    branch_yes_4:;
    {
        PyObject *tmp_ass_subvalue_4;
        PyObject *tmp_left_name_6;
        PyObject *tmp_expression_name_10;
        PyObject *tmp_expression_name_11;
        PyObject *tmp_subscript_name_8;
        PyObject *tmp_subscript_name_9;
        PyObject *tmp_right_name_6;
        PyObject *tmp_ass_subscribed_4;
        PyObject *tmp_ass_subscript_4;
        if (par_token == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 882;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }

        tmp_expression_name_11 = par_token;
        tmp_subscript_name_8 = mod_consts[31];
        tmp_expression_name_10 = LOOKUP_SUBSCRIPT(tmp_expression_name_11, tmp_subscript_name_8);
        if (tmp_expression_name_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 882;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_subscript_name_9 = mod_consts[76];
        tmp_left_name_6 = LOOKUP_SUBSCRIPT(tmp_expression_name_10, tmp_subscript_name_9);
        Py_DECREF(tmp_expression_name_10);
        if (tmp_left_name_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 882;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        tmp_right_name_6 = mod_consts[77];
        tmp_ass_subvalue_4 = BINARY_OPERATION_ADD_OBJECT_OBJECT_UNICODE(tmp_left_name_6, tmp_right_name_6);
        Py_DECREF(tmp_left_name_6);
        if (tmp_ass_subvalue_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 882;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
        if (par_token == NULL) {
            Py_DECREF(tmp_ass_subvalue_4);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 882;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }

        tmp_ass_subscribed_4 = par_token;
        tmp_ass_subscript_4 = mod_consts[31];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_4, tmp_ass_subscript_4, tmp_ass_subvalue_4);
        Py_DECREF(tmp_ass_subvalue_4);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 882;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
    }
    branch_no_4:;
    {
        PyObject *tmp_ass_subvalue_5;
        PyObject *tmp_ass_subscribed_5;
        PyObject *tmp_ass_subscript_5;
        tmp_ass_subvalue_5 = mod_consts[78];
        if (par_token == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 884;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }

        tmp_ass_subscribed_5 = par_token;
        tmp_ass_subscript_5 = mod_consts[22];
        tmp_result = SET_SUBSCRIPT(tmp_ass_subscribed_5, tmp_ass_subscript_5, tmp_ass_subvalue_5);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 884;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
    }
    {
        PyObject *tmp_delsubscr_target_1;
        PyObject *tmp_delsubscr_subscript_1;
        if (par_token == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 886;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }

        tmp_delsubscr_target_1 = par_token;
        tmp_delsubscr_subscript_1 = mod_consts[24];
        tmp_result = DEL_SUBSCRIPT(tmp_delsubscr_target_1, tmp_delsubscr_subscript_1);
        if (tmp_result == false) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 886;
            type_description_1 = "ooooooo";
            goto frame_exception_exit_1;
        }
    }
    if (par_token == NULL) {

        FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[33]);
        exception_tb = NULL;
        NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
        CHAIN_EXCEPTION(exception_value);

        exception_lineno = 887;
        type_description_1 = "ooooooo";
        goto frame_exception_exit_1;
    }

    tmp_return_value = par_token;
    Py_INCREF(tmp_return_value);
    goto frame_return_exit_1;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_17727f777de2e1825ac7bfbd7d4c5b42);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_17727f777de2e1825ac7bfbd7d4c5b42);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_17727f777de2e1825ac7bfbd7d4c5b42);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_17727f777de2e1825ac7bfbd7d4c5b42, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_17727f777de2e1825ac7bfbd7d4c5b42->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_17727f777de2e1825ac7bfbd7d4c5b42, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_17727f777de2e1825ac7bfbd7d4c5b42,
        type_description_1,
        par_self,
        par_token,
        var_token_type,
        var_attrs,
        var_ns,
        var_name,
        var_v
    );


    // Release cached frame if used for exception.
    if (frame_17727f777de2e1825ac7bfbd7d4c5b42 == cache_frame_17727f777de2e1825ac7bfbd7d4c5b42) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_17727f777de2e1825ac7bfbd7d4c5b42);
        cache_frame_17727f777de2e1825ac7bfbd7d4c5b42 = NULL;
    }

    assertFrameObject(frame_17727f777de2e1825ac7bfbd7d4c5b42);

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
    Py_XDECREF(par_token);
    par_token = NULL;
    CHECK_OBJECT(var_token_type);
    Py_DECREF(var_token_type);
    var_token_type = NULL;
    Py_XDECREF(var_attrs);
    var_attrs = NULL;
    Py_XDECREF(var_ns);
    var_ns = NULL;
    Py_XDECREF(var_name);
    var_name = NULL;
    Py_XDECREF(var_v);
    var_v = NULL;
    goto function_return_exit;
    // Exception handler code:
    try_except_handler_1:;
    exception_keeper_type_6 = exception_type;
    exception_keeper_value_6 = exception_value;
    exception_keeper_tb_6 = exception_tb;
    exception_keeper_lineno_6 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    CHECK_OBJECT(par_self);
    Py_DECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_token);
    par_token = NULL;
    Py_XDECREF(var_token_type);
    var_token_type = NULL;
    Py_XDECREF(var_attrs);
    var_attrs = NULL;
    Py_XDECREF(var_ns);
    var_ns = NULL;
    Py_XDECREF(var_name);
    var_name = NULL;
    Py_XDECREF(var_v);
    var_v = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_6;
    exception_value = exception_keeper_value_6;
    exception_tb = exception_keeper_tb_6;
    exception_lineno = exception_keeper_lineno_6;

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


static PyObject *impl_pip$_vendor$html5lib$filters$sanitizer$$$function__6_sanitize_css(struct Nuitka_FunctionObject const *self, PyObject **python_pars) {
    // Preserve error status for checks
#ifndef __NUITKA_NO_ASSERT__
    NUITKA_MAY_BE_UNUSED bool had_error = ERROR_OCCURRED();
#endif

    // Local variable declarations.
    PyObject *par_self = python_pars[0];
    PyObject *par_style = python_pars[1];
    PyObject *var_clean = NULL;
    PyObject *var_prop = NULL;
    PyObject *var_value = NULL;
    PyObject *var_keyword = NULL;
    PyObject *tmp_for_loop_1__for_iterator = NULL;
    PyObject *tmp_for_loop_1__iter_value = NULL;
    nuitka_bool tmp_for_loop_2__break_indicator = NUITKA_BOOL_UNASSIGNED;
    PyObject *tmp_for_loop_2__for_iterator = NULL;
    PyObject *tmp_for_loop_2__iter_value = NULL;
    PyObject *tmp_tuple_unpack_1__element_1 = NULL;
    PyObject *tmp_tuple_unpack_1__element_2 = NULL;
    PyObject *tmp_tuple_unpack_1__source_iter = NULL;
    struct Nuitka_FrameObject *frame_1d8a74b3054be3d7de6f45e13b01b9ed;
    NUITKA_MAY_BE_UNUSED char const *type_description_1 = NULL;
    PyObject *exception_type = NULL;
    PyObject *exception_value = NULL;
    PyTracebackObject *exception_tb = NULL;
    NUITKA_MAY_BE_UNUSED int exception_lineno = 0;
    int tmp_res;
    PyObject *tmp_return_value = NULL;
    PyObject *tmp_iterator_attempt;
    PyObject *exception_keeper_type_1;
    PyObject *exception_keeper_value_1;
    PyTracebackObject *exception_keeper_tb_1;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_1;
    PyObject *exception_keeper_type_2;
    PyObject *exception_keeper_value_2;
    PyTracebackObject *exception_keeper_tb_2;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_2;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
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
    static struct Nuitka_FrameObject *cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed = NULL;
    PyObject *exception_keeper_type_6;
    PyObject *exception_keeper_value_6;
    PyTracebackObject *exception_keeper_tb_6;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_6;

    // Actual function body.
    // Tried code:
    if (isFrameUnusable(cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed)) {
        Py_XDECREF(cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed);

#if _DEBUG_REFCOUNTS
        if (cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed == NULL) {
            count_active_frame_cache_instances += 1;
        } else {
            count_released_frame_cache_instances += 1;
        }
        count_allocated_frame_cache_instances += 1;
#endif
        cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed = MAKE_FUNCTION_FRAME(codeobj_1d8a74b3054be3d7de6f45e13b01b9ed, module_pip$_vendor$html5lib$filters$sanitizer, sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *)+sizeof(void *));
#if _DEBUG_REFCOUNTS
    } else {
        count_hit_frame_cache_instances += 1;
#endif
    }
    assert(cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_type_description == NULL);
    frame_1d8a74b3054be3d7de6f45e13b01b9ed = cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed;

    // Push the new frame as the currently active one.
    pushFrameStack(frame_1d8a74b3054be3d7de6f45e13b01b9ed);

    // Mark the frame object as in use, ref count 1 will be up for reuse.
    assert(Py_REFCNT(frame_1d8a74b3054be3d7de6f45e13b01b9ed) == 2); // Frame stack

    // Framed code:
    {
        PyObject *tmp_assign_source_1;
        PyObject *tmp_called_instance_1;
        PyObject *tmp_called_instance_2;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        tmp_called_instance_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_called_instance_2 == NULL)) {
            tmp_called_instance_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_called_instance_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 891;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 891;
        tmp_called_instance_1 = CALL_METHOD_WITH_ARGS1(
            tmp_called_instance_2,
            mod_consts[79],
            &PyTuple_GET_ITEM(mod_consts[80], 0)
        );

        if (tmp_called_instance_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 891;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_1 = mod_consts[54];
        CHECK_OBJECT(par_style);
        tmp_args_element_name_2 = par_style;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 891;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_assign_source_1 = CALL_METHOD_WITH_ARGS2(
                tmp_called_instance_1,
                mod_consts[38],
                call_args
            );
        }

        Py_DECREF(tmp_called_instance_1);
        if (tmp_assign_source_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 891;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        {
            PyObject *old = par_style;
            par_style = tmp_assign_source_1;
            Py_XDECREF(old);
        }

    }
    {
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_operand_name_1;
        PyObject *tmp_called_instance_3;
        PyObject *tmp_args_element_name_3;
        PyObject *tmp_args_element_name_4;
        tmp_called_instance_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_called_instance_3 == NULL)) {
            tmp_called_instance_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_called_instance_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 894;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_3 = mod_consts[81];
        CHECK_OBJECT(par_style);
        tmp_args_element_name_4 = par_style;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 894;
        {
            PyObject *call_args[] = {tmp_args_element_name_3, tmp_args_element_name_4};
            tmp_operand_name_1 = CALL_METHOD_WITH_ARGS2(
                tmp_called_instance_3,
                mod_consts[49],
                call_args
            );
        }

        if (tmp_operand_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 894;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        Py_DECREF(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 894;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_1 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_1 == NUITKA_BOOL_TRUE) {
            goto branch_yes_1;
        } else {
            goto branch_no_1;
        }
    }
    branch_yes_1:;
    tmp_return_value = mod_consts[40];
    Py_INCREF(tmp_return_value);
    goto frame_return_exit_1;
    branch_no_1:;
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_operand_name_2;
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_args_element_name_5;
        PyObject *tmp_args_element_name_6;
        tmp_expression_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_expression_name_1 == NULL)) {
            tmp_expression_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 896;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[49]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 896;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_5 = mod_consts[82];
        if (par_style == NULL) {
            Py_DECREF(tmp_called_name_1);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[83]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 896;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }

        tmp_args_element_name_6 = par_style;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 896;
        {
            PyObject *call_args[] = {tmp_args_element_name_5, tmp_args_element_name_6};
            tmp_operand_name_2 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_called_name_1);
        if (tmp_operand_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 896;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_2);
        Py_DECREF(tmp_operand_name_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 896;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_condition_result_2 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_2 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    tmp_return_value = mod_consts[40];
    Py_INCREF(tmp_return_value);
    goto frame_return_exit_1;
    branch_no_2:;
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = PyList_New(0);
        assert(var_clean == NULL);
        var_clean = tmp_assign_source_2;
    }
    {
        PyObject *tmp_assign_source_3;
        PyObject *tmp_iter_arg_1;
        PyObject *tmp_called_name_2;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_args_element_name_7;
        PyObject *tmp_args_element_name_8;
        tmp_expression_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_expression_name_2 == NULL)) {
            tmp_expression_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_expression_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 900;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_called_name_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_2, mod_consts[84]);
        if (tmp_called_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 900;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_args_element_name_7 = mod_consts[85];
        if (par_style == NULL) {
            Py_DECREF(tmp_called_name_2);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[83]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 900;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }

        tmp_args_element_name_8 = par_style;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 900;
        {
            PyObject *call_args[] = {tmp_args_element_name_7, tmp_args_element_name_8};
            tmp_iter_arg_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_2, call_args);
        }

        Py_DECREF(tmp_called_name_2);
        if (tmp_iter_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 900;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        tmp_assign_source_3 = MAKE_ITERATOR(tmp_iter_arg_1);
        Py_DECREF(tmp_iter_arg_1);
        if (tmp_assign_source_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 900;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        assert(tmp_for_loop_1__for_iterator == NULL);
        tmp_for_loop_1__for_iterator = tmp_assign_source_3;
    }
    // Tried code:
    loop_start_1:;
    {
        PyObject *tmp_next_source_1;
        PyObject *tmp_assign_source_4;
        CHECK_OBJECT(tmp_for_loop_1__for_iterator);
        tmp_next_source_1 = tmp_for_loop_1__for_iterator;
        tmp_assign_source_4 = ITERATOR_NEXT(tmp_next_source_1);
        if (tmp_assign_source_4 == NULL) {
            if (CHECK_AND_CLEAR_STOP_ITERATION_OCCURRED()) {

                goto loop_end_1;
            } else {

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
                type_description_1 = "oooooo";
                exception_lineno = 900;
                goto try_except_handler_2;
            }
        }

        {
            PyObject *old = tmp_for_loop_1__iter_value;
            tmp_for_loop_1__iter_value = tmp_assign_source_4;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_5;
        PyObject *tmp_iter_arg_2;
        CHECK_OBJECT(tmp_for_loop_1__iter_value);
        tmp_iter_arg_2 = tmp_for_loop_1__iter_value;
        tmp_assign_source_5 = MAKE_ITERATOR(tmp_iter_arg_2);
        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 900;
            type_description_1 = "oooooo";
            goto try_except_handler_3;
        }
        {
            PyObject *old = tmp_tuple_unpack_1__source_iter;
            tmp_tuple_unpack_1__source_iter = tmp_assign_source_5;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_unpack_1;
        CHECK_OBJECT(tmp_tuple_unpack_1__source_iter);
        tmp_unpack_1 = tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_6 = UNPACK_NEXT(tmp_unpack_1, 0, 2);
        if (tmp_assign_source_6 == NULL) {
            if (!ERROR_OCCURRED()) {
                exception_type = PyExc_StopIteration;
                Py_INCREF(exception_type);
                exception_value = NULL;
                exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            }


            type_description_1 = "oooooo";
            exception_lineno = 900;
            goto try_except_handler_4;
        }
        {
            PyObject *old = tmp_tuple_unpack_1__element_1;
            tmp_tuple_unpack_1__element_1 = tmp_assign_source_6;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_assign_source_7;
        PyObject *tmp_unpack_2;
        CHECK_OBJECT(tmp_tuple_unpack_1__source_iter);
        tmp_unpack_2 = tmp_tuple_unpack_1__source_iter;
        tmp_assign_source_7 = UNPACK_NEXT(tmp_unpack_2, 1, 2);
        if (tmp_assign_source_7 == NULL) {
            if (!ERROR_OCCURRED()) {
                exception_type = PyExc_StopIteration;
                Py_INCREF(exception_type);
                exception_value = NULL;
                exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            }


            type_description_1 = "oooooo";
            exception_lineno = 900;
            goto try_except_handler_4;
        }
        {
            PyObject *old = tmp_tuple_unpack_1__element_2;
            tmp_tuple_unpack_1__element_2 = tmp_assign_source_7;
            Py_XDECREF(old);
        }

    }
    {
        PyObject *tmp_iterator_name_1;
        CHECK_OBJECT(tmp_tuple_unpack_1__source_iter);
        tmp_iterator_name_1 = tmp_tuple_unpack_1__source_iter;
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

                    type_description_1 = "oooooo";
                    exception_lineno = 900;
                    goto try_except_handler_4;
                }
            }
        } else {
            Py_DECREF(tmp_iterator_attempt);

            exception_type = PyExc_ValueError;
            Py_INCREF(PyExc_ValueError);
            exception_value = mod_consts[65];
            Py_INCREF(exception_value);
            exception_tb = NULL;

            type_description_1 = "oooooo";
            exception_lineno = 900;
            goto try_except_handler_4;
        }
    }
    goto try_end_1;
    // Exception handler code:
    try_except_handler_4:;
    exception_keeper_type_1 = exception_type;
    exception_keeper_value_1 = exception_value;
    exception_keeper_tb_1 = exception_tb;
    exception_keeper_lineno_1 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_tuple_unpack_1__source_iter);
    tmp_tuple_unpack_1__source_iter = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_1;
    exception_value = exception_keeper_value_1;
    exception_tb = exception_keeper_tb_1;
    exception_lineno = exception_keeper_lineno_1;

    goto try_except_handler_3;
    // End of try:
    try_end_1:;
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

    Py_XDECREF(tmp_tuple_unpack_1__element_1);
    tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(tmp_tuple_unpack_1__element_2);
    tmp_tuple_unpack_1__element_2 = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_2;
    exception_value = exception_keeper_value_2;
    exception_tb = exception_keeper_tb_2;
    exception_lineno = exception_keeper_lineno_2;

    goto try_except_handler_2;
    // End of try:
    try_end_2:;
    Py_XDECREF(tmp_tuple_unpack_1__source_iter);
    tmp_tuple_unpack_1__source_iter = NULL;
    {
        PyObject *tmp_assign_source_8;
        CHECK_OBJECT(tmp_tuple_unpack_1__element_1);
        tmp_assign_source_8 = tmp_tuple_unpack_1__element_1;
        {
            PyObject *old = var_prop;
            var_prop = tmp_assign_source_8;
            Py_INCREF(var_prop);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(tmp_tuple_unpack_1__element_1);
    tmp_tuple_unpack_1__element_1 = NULL;

    {
        PyObject *tmp_assign_source_9;
        CHECK_OBJECT(tmp_tuple_unpack_1__element_2);
        tmp_assign_source_9 = tmp_tuple_unpack_1__element_2;
        {
            PyObject *old = var_value;
            var_value = tmp_assign_source_9;
            Py_INCREF(var_value);
            Py_XDECREF(old);
        }

    }
    Py_XDECREF(tmp_tuple_unpack_1__element_2);
    tmp_tuple_unpack_1__element_2 = NULL;

    Py_XDECREF(tmp_tuple_unpack_1__element_1);
    tmp_tuple_unpack_1__element_1 = NULL;
    Py_XDECREF(tmp_tuple_unpack_1__element_2);
    tmp_tuple_unpack_1__element_2 = NULL;
    {
        nuitka_bool tmp_condition_result_3;
        PyObject *tmp_operand_name_3;
        CHECK_OBJECT(var_value);
        tmp_operand_name_3 = var_value;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_3);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 901;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_condition_result_3 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_3 == NUITKA_BOOL_TRUE) {
            goto branch_yes_3;
        } else {
            goto branch_no_3;
        }
    }
    branch_yes_3:;
    goto loop_start_1;
    branch_no_3:;
    {
        bool tmp_condition_result_4;
        PyObject *tmp_compexpr_left_1;
        PyObject *tmp_compexpr_right_1;
        PyObject *tmp_called_instance_4;
        PyObject *tmp_expression_name_3;
        CHECK_OBJECT(var_prop);
        tmp_called_instance_4 = var_prop;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 903;
        tmp_compexpr_left_1 = CALL_METHOD_NO_ARGS(tmp_called_instance_4, mod_consts[42]);
        if (tmp_compexpr_left_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 903;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        if (par_self == NULL) {
            Py_DECREF(tmp_compexpr_left_1);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 903;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_expression_name_3 = par_self;
        tmp_compexpr_right_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_3, mod_consts[8]);
        if (tmp_compexpr_right_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_compexpr_left_1);

            exception_lineno = 903;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_1, tmp_compexpr_left_1);
        Py_DECREF(tmp_compexpr_left_1);
        Py_DECREF(tmp_compexpr_right_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 903;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_condition_result_4 = (tmp_res == 1) ? true : false;
        if (tmp_condition_result_4 != false) {
            goto branch_yes_4;
        } else {
            goto branch_no_4;
        }
    }
    branch_yes_4:;
    {
        PyObject *tmp_called_name_3;
        PyObject *tmp_expression_name_4;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_9;
        PyObject *tmp_left_name_1;
        PyObject *tmp_left_name_2;
        PyObject *tmp_left_name_3;
        PyObject *tmp_right_name_1;
        PyObject *tmp_right_name_2;
        PyObject *tmp_right_name_3;
        if (var_clean == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[86]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 904;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_expression_name_4 = var_clean;
        tmp_called_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_4, mod_consts[66]);
        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 904;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        if (var_prop == NULL) {
            Py_DECREF(tmp_called_name_3);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[87]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 904;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_left_name_3 = var_prop;
        tmp_right_name_1 = mod_consts[88];
        tmp_left_name_2 = BINARY_OPERATION_ADD_OBJECT_OBJECT_UNICODE(tmp_left_name_3, tmp_right_name_1);
        if (tmp_left_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_3);

            exception_lineno = 904;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        CHECK_OBJECT(var_value);
        tmp_right_name_2 = var_value;
        tmp_left_name_1 = BINARY_OPERATION_ADD_OBJECT_OBJECT_OBJECT(tmp_left_name_2, tmp_right_name_2);
        Py_DECREF(tmp_left_name_2);
        if (tmp_left_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_3);

            exception_lineno = 904;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_right_name_3 = mod_consts[89];
        tmp_args_element_name_9 = BINARY_OPERATION_ADD_OBJECT_OBJECT_UNICODE(tmp_left_name_1, tmp_right_name_3);
        Py_DECREF(tmp_left_name_1);
        if (tmp_args_element_name_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_3);

            exception_lineno = 904;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 904;
        tmp_call_result_1 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_3, tmp_args_element_name_9);
        Py_DECREF(tmp_called_name_3);
        Py_DECREF(tmp_args_element_name_9);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 904;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        Py_DECREF(tmp_call_result_1);
    }
    goto branch_end_4;
    branch_no_4:;
    {
        bool tmp_condition_result_5;
        PyObject *tmp_compexpr_left_2;
        PyObject *tmp_compexpr_right_2;
        PyObject *tmp_called_instance_5;
        PyObject *tmp_expression_name_5;
        PyObject *tmp_called_instance_6;
        PyObject *tmp_subscript_name_1;
        if (var_prop == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[87]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 905;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_called_instance_6 = var_prop;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 905;
        tmp_expression_name_5 = CALL_METHOD_WITH_ARGS1(
            tmp_called_instance_6,
            mod_consts[90],
            &PyTuple_GET_ITEM(mod_consts[91], 0)
        );

        if (tmp_expression_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 905;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_subscript_name_1 = mod_consts[92];
        tmp_called_instance_5 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_5, tmp_subscript_name_1, 0);
        Py_DECREF(tmp_expression_name_5);
        if (tmp_called_instance_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 905;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 905;
        tmp_compexpr_left_2 = CALL_METHOD_NO_ARGS(tmp_called_instance_5, mod_consts[42]);
        Py_DECREF(tmp_called_instance_5);
        if (tmp_compexpr_left_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 905;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_compexpr_right_2 = LIST_COPY(mod_consts[93]);
        tmp_res = PySequence_Contains(tmp_compexpr_right_2, tmp_compexpr_left_2);
        Py_DECREF(tmp_compexpr_left_2);
        Py_DECREF(tmp_compexpr_right_2);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 905;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_condition_result_5 = (tmp_res == 1) ? true : false;
        if (tmp_condition_result_5 != false) {
            goto branch_yes_5;
        } else {
            goto branch_no_5;
        }
    }
    branch_yes_5:;
    {
        nuitka_bool tmp_assign_source_10;
        tmp_assign_source_10 = NUITKA_BOOL_FALSE;
        tmp_for_loop_2__break_indicator = tmp_assign_source_10;
    }
    {
        PyObject *tmp_assign_source_11;
        PyObject *tmp_iter_arg_3;
        PyObject *tmp_called_instance_7;
        CHECK_OBJECT(var_value);
        tmp_called_instance_7 = var_value;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 907;
        tmp_iter_arg_3 = CALL_METHOD_NO_ARGS(tmp_called_instance_7, mod_consts[90]);
        if (tmp_iter_arg_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 907;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_assign_source_11 = MAKE_ITERATOR(tmp_iter_arg_3);
        Py_DECREF(tmp_iter_arg_3);
        if (tmp_assign_source_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 907;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        {
            PyObject *old = tmp_for_loop_2__for_iterator;
            tmp_for_loop_2__for_iterator = tmp_assign_source_11;
            Py_XDECREF(old);
        }

    }
    // Tried code:
    loop_start_2:;
    // Tried code:
    {
        PyObject *tmp_assign_source_12;
        PyObject *tmp_value_name_1;
        CHECK_OBJECT(tmp_for_loop_2__for_iterator);
        tmp_value_name_1 = tmp_for_loop_2__for_iterator;
        tmp_assign_source_12 = ITERATOR_NEXT(tmp_value_name_1);
        if (tmp_assign_source_12 == NULL) {
            if (!ERROR_OCCURRED()) {
                exception_type = PyExc_StopIteration;
                Py_INCREF(exception_type);
                exception_value = NULL;
                exception_tb = NULL;
            } else {
                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            }


            type_description_1 = "oooooo";
            exception_lineno = 907;
            goto try_except_handler_6;
        }
        {
            PyObject *old = tmp_for_loop_2__iter_value;
            tmp_for_loop_2__iter_value = tmp_assign_source_12;
            Py_XDECREF(old);
        }

    }
    goto try_end_3;
    // Exception handler code:
    try_except_handler_6:;
    exception_keeper_type_3 = exception_type;
    exception_keeper_value_3 = exception_value;
    exception_keeper_tb_3 = exception_tb;
    exception_keeper_lineno_3 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    {
        bool tmp_condition_result_6;
        PyObject *tmp_compexpr_left_3;
        PyObject *tmp_compexpr_right_3;
        tmp_compexpr_left_3 = exception_keeper_type_3;
        tmp_compexpr_right_3 = PyExc_StopIteration;
        tmp_res = EXCEPTION_MATCH_BOOL(tmp_compexpr_left_3, tmp_compexpr_right_3);
        assert(!(tmp_res == -1));
        tmp_condition_result_6 = (tmp_res != 0) ? true : false;
        if (tmp_condition_result_6 != false) {
            goto branch_yes_6;
        } else {
            goto branch_no_6;
        }
    }
    branch_yes_6:;
    {
        nuitka_bool tmp_assign_source_13;
        tmp_assign_source_13 = NUITKA_BOOL_TRUE;
        tmp_for_loop_2__break_indicator = tmp_assign_source_13;
    }
    Py_DECREF(exception_keeper_type_3);
    Py_XDECREF(exception_keeper_value_3);
    Py_XDECREF(exception_keeper_tb_3);
    goto loop_end_2;
    goto branch_end_6;
    branch_no_6:;
    // Re-raise.
    exception_type = exception_keeper_type_3;
    exception_value = exception_keeper_value_3;
    exception_tb = exception_keeper_tb_3;
    exception_lineno = exception_keeper_lineno_3;

    goto try_except_handler_5;
    branch_end_6:;
    // End of try:
    try_end_3:;
    {
        PyObject *tmp_assign_source_14;
        CHECK_OBJECT(tmp_for_loop_2__iter_value);
        tmp_assign_source_14 = tmp_for_loop_2__iter_value;
        {
            PyObject *old = var_keyword;
            var_keyword = tmp_assign_source_14;
            Py_INCREF(var_keyword);
            Py_XDECREF(old);
        }

    }
    {
        nuitka_bool tmp_condition_result_7;
        int tmp_and_left_truth_1;
        nuitka_bool tmp_and_left_value_1;
        nuitka_bool tmp_and_right_value_1;
        PyObject *tmp_compexpr_left_4;
        PyObject *tmp_compexpr_right_4;
        PyObject *tmp_expression_name_6;
        PyObject *tmp_operand_name_4;
        PyObject *tmp_called_instance_8;
        PyObject *tmp_args_element_name_10;
        PyObject *tmp_args_element_name_11;
        CHECK_OBJECT(var_keyword);
        tmp_compexpr_left_4 = var_keyword;
        if (par_self == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 908;
            type_description_1 = "oooooo";
            goto try_except_handler_5;
        }

        tmp_expression_name_6 = par_self;
        tmp_compexpr_right_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_6, mod_consts[9]);
        if (tmp_compexpr_right_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 908;
            type_description_1 = "oooooo";
            goto try_except_handler_5;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_4, tmp_compexpr_left_4);
        Py_DECREF(tmp_compexpr_right_4);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 908;
            type_description_1 = "oooooo";
            goto try_except_handler_5;
        }
        tmp_and_left_value_1 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        tmp_and_left_truth_1 = tmp_and_left_value_1 == NUITKA_BOOL_TRUE ? 1 : 0;
        if (tmp_and_left_truth_1 == 1) {
            goto and_right_1;
        } else {
            goto and_left_1;
        }
        and_right_1:;
        tmp_called_instance_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_called_instance_8 == NULL)) {
            tmp_called_instance_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_called_instance_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 909;
            type_description_1 = "oooooo";
            goto try_except_handler_5;
        }
        tmp_args_element_name_10 = mod_consts[94];
        CHECK_OBJECT(var_keyword);
        tmp_args_element_name_11 = var_keyword;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 909;
        {
            PyObject *call_args[] = {tmp_args_element_name_10, tmp_args_element_name_11};
            tmp_operand_name_4 = CALL_METHOD_WITH_ARGS2(
                tmp_called_instance_8,
                mod_consts[49],
                call_args
            );
        }

        if (tmp_operand_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 909;
            type_description_1 = "oooooo";
            goto try_except_handler_5;
        }
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_4);
        Py_DECREF(tmp_operand_name_4);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 909;
            type_description_1 = "oooooo";
            goto try_except_handler_5;
        }
        tmp_and_right_value_1 = (tmp_res == 0) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        tmp_condition_result_7 = tmp_and_right_value_1;
        goto and_end_1;
        and_left_1:;
        tmp_condition_result_7 = tmp_and_left_value_1;
        and_end_1:;
        if (tmp_condition_result_7 == NUITKA_BOOL_TRUE) {
            goto branch_yes_7;
        } else {
            goto branch_no_7;
        }
    }
    branch_yes_7:;
    goto loop_end_2;
    branch_no_7:;
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 907;
        type_description_1 = "oooooo";
        goto try_except_handler_5;
    }
    goto loop_start_2;
    loop_end_2:;
    goto try_end_4;
    // Exception handler code:
    try_except_handler_5:;
    exception_keeper_type_4 = exception_type;
    exception_keeper_value_4 = exception_value;
    exception_keeper_tb_4 = exception_tb;
    exception_keeper_lineno_4 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_for_loop_2__iter_value);
    tmp_for_loop_2__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_2__for_iterator);
    tmp_for_loop_2__for_iterator = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_4;
    exception_value = exception_keeper_value_4;
    exception_tb = exception_keeper_tb_4;
    exception_lineno = exception_keeper_lineno_4;

    goto try_except_handler_2;
    // End of try:
    try_end_4:;
    Py_XDECREF(tmp_for_loop_2__iter_value);
    tmp_for_loop_2__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_2__for_iterator);
    tmp_for_loop_2__for_iterator = NULL;
    {
        bool tmp_condition_result_8;
        nuitka_bool tmp_compexpr_left_5;
        nuitka_bool tmp_compexpr_right_5;
        assert(tmp_for_loop_2__break_indicator != NUITKA_BOOL_UNASSIGNED);
        tmp_compexpr_left_5 = tmp_for_loop_2__break_indicator;
        tmp_compexpr_right_5 = NUITKA_BOOL_TRUE;
        tmp_condition_result_8 = (tmp_compexpr_left_5 == tmp_compexpr_right_5) ? true : false;
        if (tmp_condition_result_8 != false) {
            goto branch_yes_8;
        } else {
            goto branch_no_8;
        }
    }
    branch_yes_8:;
    {
        PyObject *tmp_called_name_4;
        PyObject *tmp_expression_name_7;
        PyObject *tmp_call_result_2;
        PyObject *tmp_args_element_name_12;
        PyObject *tmp_left_name_4;
        PyObject *tmp_left_name_5;
        PyObject *tmp_left_name_6;
        PyObject *tmp_right_name_4;
        PyObject *tmp_right_name_5;
        PyObject *tmp_right_name_6;
        if (var_clean == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[86]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_expression_name_7 = var_clean;
        tmp_called_name_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_7, mod_consts[66]);
        if (tmp_called_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        if (var_prop == NULL) {
            Py_DECREF(tmp_called_name_4);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[87]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_left_name_6 = var_prop;
        tmp_right_name_4 = mod_consts[88];
        tmp_left_name_5 = BINARY_OPERATION_ADD_OBJECT_OBJECT_UNICODE(tmp_left_name_6, tmp_right_name_4);
        if (tmp_left_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_4);

            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        if (var_value == NULL) {
            Py_DECREF(tmp_called_name_4);
            Py_DECREF(tmp_left_name_5);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[95]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_right_name_5 = var_value;
        tmp_left_name_4 = BINARY_OPERATION_ADD_OBJECT_OBJECT_OBJECT(tmp_left_name_5, tmp_right_name_5);
        Py_DECREF(tmp_left_name_5);
        if (tmp_left_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_4);

            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_right_name_6 = mod_consts[89];
        tmp_args_element_name_12 = BINARY_OPERATION_ADD_OBJECT_OBJECT_UNICODE(tmp_left_name_4, tmp_right_name_6);
        Py_DECREF(tmp_left_name_4);
        if (tmp_args_element_name_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_4);

            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 912;
        tmp_call_result_2 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_4, tmp_args_element_name_12);
        Py_DECREF(tmp_called_name_4);
        Py_DECREF(tmp_args_element_name_12);
        if (tmp_call_result_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 912;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        Py_DECREF(tmp_call_result_2);
    }
    branch_no_8:;
    goto branch_end_5;
    branch_no_5:;
    {
        bool tmp_condition_result_9;
        PyObject *tmp_compexpr_left_6;
        PyObject *tmp_compexpr_right_6;
        PyObject *tmp_called_instance_9;
        PyObject *tmp_expression_name_8;
        if (var_prop == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[87]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 913;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_called_instance_9 = var_prop;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 913;
        tmp_compexpr_left_6 = CALL_METHOD_NO_ARGS(tmp_called_instance_9, mod_consts[42]);
        if (tmp_compexpr_left_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 913;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        if (par_self == NULL) {
            Py_DECREF(tmp_compexpr_left_6);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[19]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 913;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_expression_name_8 = par_self;
        tmp_compexpr_right_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_8, mod_consts[10]);
        if (tmp_compexpr_right_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_compexpr_left_6);

            exception_lineno = 913;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_res = PySequence_Contains(tmp_compexpr_right_6, tmp_compexpr_left_6);
        Py_DECREF(tmp_compexpr_left_6);
        Py_DECREF(tmp_compexpr_right_6);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 913;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_condition_result_9 = (tmp_res == 1) ? true : false;
        if (tmp_condition_result_9 != false) {
            goto branch_yes_9;
        } else {
            goto branch_no_9;
        }
    }
    branch_yes_9:;
    {
        PyObject *tmp_called_name_5;
        PyObject *tmp_expression_name_9;
        PyObject *tmp_call_result_3;
        PyObject *tmp_args_element_name_13;
        PyObject *tmp_left_name_7;
        PyObject *tmp_left_name_8;
        PyObject *tmp_left_name_9;
        PyObject *tmp_right_name_7;
        PyObject *tmp_right_name_8;
        PyObject *tmp_right_name_9;
        if (var_clean == NULL) {

            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[86]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 914;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_expression_name_9 = var_clean;
        tmp_called_name_5 = LOOKUP_ATTRIBUTE(tmp_expression_name_9, mod_consts[66]);
        if (tmp_called_name_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 914;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        if (var_prop == NULL) {
            Py_DECREF(tmp_called_name_5);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[87]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 914;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }

        tmp_left_name_9 = var_prop;
        tmp_right_name_7 = mod_consts[88];
        tmp_left_name_8 = BINARY_OPERATION_ADD_OBJECT_OBJECT_UNICODE(tmp_left_name_9, tmp_right_name_7);
        if (tmp_left_name_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_5);

            exception_lineno = 914;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        CHECK_OBJECT(var_value);
        tmp_right_name_8 = var_value;
        tmp_left_name_7 = BINARY_OPERATION_ADD_OBJECT_OBJECT_OBJECT(tmp_left_name_8, tmp_right_name_8);
        Py_DECREF(tmp_left_name_8);
        if (tmp_left_name_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_5);

            exception_lineno = 914;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        tmp_right_name_9 = mod_consts[89];
        tmp_args_element_name_13 = BINARY_OPERATION_ADD_OBJECT_OBJECT_UNICODE(tmp_left_name_7, tmp_right_name_9);
        Py_DECREF(tmp_left_name_7);
        if (tmp_args_element_name_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_5);

            exception_lineno = 914;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 914;
        tmp_call_result_3 = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_5, tmp_args_element_name_13);
        Py_DECREF(tmp_called_name_5);
        Py_DECREF(tmp_args_element_name_13);
        if (tmp_call_result_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 914;
            type_description_1 = "oooooo";
            goto try_except_handler_2;
        }
        Py_DECREF(tmp_call_result_3);
    }
    branch_no_9:;
    branch_end_5:;
    branch_end_4:;
    if (CONSIDER_THREADING() == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 900;
        type_description_1 = "oooooo";
        goto try_except_handler_2;
    }
    goto loop_start_1;
    loop_end_1:;
    goto try_end_5;
    // Exception handler code:
    try_except_handler_2:;
    exception_keeper_type_5 = exception_type;
    exception_keeper_value_5 = exception_value;
    exception_keeper_tb_5 = exception_tb;
    exception_keeper_lineno_5 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(tmp_for_loop_1__iter_value);
    tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_1__for_iterator);
    tmp_for_loop_1__for_iterator = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_5;
    exception_value = exception_keeper_value_5;
    exception_tb = exception_keeper_tb_5;
    exception_lineno = exception_keeper_lineno_5;

    goto frame_exception_exit_1;
    // End of try:
    try_end_5:;
    Py_XDECREF(tmp_for_loop_1__iter_value);
    tmp_for_loop_1__iter_value = NULL;
    Py_XDECREF(tmp_for_loop_1__for_iterator);
    tmp_for_loop_1__for_iterator = NULL;
    {
        PyObject *tmp_called_name_6;
        PyObject *tmp_expression_name_10;
        PyObject *tmp_args_element_name_14;
        tmp_expression_name_10 = mod_consts[54];
        tmp_called_name_6 = LOOKUP_ATTRIBUTE(tmp_expression_name_10, mod_consts[72]);
        assert(!(tmp_called_name_6 == NULL));
        if (var_clean == NULL) {
            Py_DECREF(tmp_called_name_6);
            FORMAT_UNBOUND_LOCAL_ERROR(&exception_type, &exception_value, mod_consts[86]);
            exception_tb = NULL;
            NORMALIZE_EXCEPTION(&exception_type, &exception_value, &exception_tb);
            CHAIN_EXCEPTION(exception_value);

            exception_lineno = 916;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }

        tmp_args_element_name_14 = var_clean;
        frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame.f_lineno = 916;
        tmp_return_value = CALL_FUNCTION_WITH_SINGLE_ARG(tmp_called_name_6, tmp_args_element_name_14);
        Py_DECREF(tmp_called_name_6);
        if (tmp_return_value == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 916;
            type_description_1 = "oooooo";
            goto frame_exception_exit_1;
        }
        goto frame_return_exit_1;
    }

#if 0
    RESTORE_FRAME_EXCEPTION(frame_1d8a74b3054be3d7de6f45e13b01b9ed);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto frame_no_exception_1;

    frame_return_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_1d8a74b3054be3d7de6f45e13b01b9ed);
#endif

    // Put the previous frame back on top.
    popFrameStack();

    goto try_return_handler_1;

    frame_exception_exit_1:;

#if 0
    RESTORE_FRAME_EXCEPTION(frame_1d8a74b3054be3d7de6f45e13b01b9ed);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_1d8a74b3054be3d7de6f45e13b01b9ed, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_1d8a74b3054be3d7de6f45e13b01b9ed->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_1d8a74b3054be3d7de6f45e13b01b9ed, exception_lineno);
    }

    // Attaches locals to frame if any.
    Nuitka_Frame_AttachLocals(
        frame_1d8a74b3054be3d7de6f45e13b01b9ed,
        type_description_1,
        par_self,
        par_style,
        var_clean,
        var_prop,
        var_value,
        var_keyword
    );


    // Release cached frame if used for exception.
    if (frame_1d8a74b3054be3d7de6f45e13b01b9ed == cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed) {
#if _DEBUG_REFCOUNTS
        count_active_frame_cache_instances -= 1;
        count_released_frame_cache_instances += 1;
#endif

        Py_DECREF(cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed);
        cache_frame_1d8a74b3054be3d7de6f45e13b01b9ed = NULL;
    }

    assertFrameObject(frame_1d8a74b3054be3d7de6f45e13b01b9ed);

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
    Py_XDECREF(par_style);
    par_style = NULL;
    Py_XDECREF(var_clean);
    var_clean = NULL;
    Py_XDECREF(var_prop);
    var_prop = NULL;
    Py_XDECREF(var_value);
    var_value = NULL;
    Py_XDECREF(var_keyword);
    var_keyword = NULL;
    goto function_return_exit;
    // Exception handler code:
    try_except_handler_1:;
    exception_keeper_type_6 = exception_type;
    exception_keeper_value_6 = exception_value;
    exception_keeper_tb_6 = exception_tb;
    exception_keeper_lineno_6 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    Py_XDECREF(par_self);
    par_self = NULL;
    Py_XDECREF(par_style);
    par_style = NULL;
    Py_XDECREF(var_clean);
    var_clean = NULL;
    Py_XDECREF(var_prop);
    var_prop = NULL;
    Py_XDECREF(var_value);
    var_value = NULL;
    Py_XDECREF(var_keyword);
    var_keyword = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_6;
    exception_value = exception_keeper_value_6;
    exception_tb = exception_keeper_tb_6;
    exception_lineno = exception_keeper_lineno_6;

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



static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__1___init__(PyObject *defaults) {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$html5lib$filters$sanitizer$$$function__1___init__,
        mod_consts[1],
#if PYTHON_VERSION >= 0x300
        mod_consts[589],
#endif
        codeobj_097fd85308f32255fc725ef557550425,
        defaults,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$html5lib$filters$sanitizer,
        mod_consts[16],
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__,
        mod_consts[18],
#if PYTHON_VERSION >= 0x300
        mod_consts[21],
#endif
        codeobj_5851b1f734f18da6d819ad9bb56555dd,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$html5lib$filters$sanitizer,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__3_sanitize_token() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$html5lib$filters$sanitizer$$$function__3_sanitize_token,
        mod_consts[20],
#if PYTHON_VERSION >= 0x300
        mod_consts[590],
#endif
        codeobj_c7669c8caeb97d1c9b1825727b5cea9a,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$html5lib$filters$sanitizer,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__4_allowed_token() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$html5lib$filters$sanitizer$$$function__4_allowed_token,
        mod_consts[28],
#if PYTHON_VERSION >= 0x300
        mod_consts[591],
#endif
        codeobj_fde2c08ac2458506996a6f1c4f06cd03,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$html5lib$filters$sanitizer,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__5_disallowed_token() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$html5lib$filters$sanitizer$$$function__5_disallowed_token,
        mod_consts[29],
#if PYTHON_VERSION >= 0x300
        mod_consts[592],
#endif
        codeobj_17727f777de2e1825ac7bfbd7d4c5b42,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$html5lib$filters$sanitizer,
        NULL,
        NULL,
        0
    );


    return (PyObject *)result;
}



static PyObject *MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__6_sanitize_css() {
    struct Nuitka_FunctionObject *result = Nuitka_Function_New(
        impl_pip$_vendor$html5lib$filters$sanitizer$$$function__6_sanitize_css,
        mod_consts[60],
#if PYTHON_VERSION >= 0x300
        mod_consts[593],
#endif
        codeobj_1d8a74b3054be3d7de6f45e13b01b9ed,
        NULL,
#if PYTHON_VERSION >= 0x300
        NULL,
        NULL,
#endif
        module_pip$_vendor$html5lib$filters$sanitizer,
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

function_impl_code functable_pip$_vendor$html5lib$filters$sanitizer[] = {
    impl_pip$_vendor$html5lib$filters$sanitizer$$$function__1___init__,
    impl_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__,
    impl_pip$_vendor$html5lib$filters$sanitizer$$$function__3_sanitize_token,
    impl_pip$_vendor$html5lib$filters$sanitizer$$$function__4_allowed_token,
    impl_pip$_vendor$html5lib$filters$sanitizer$$$function__5_disallowed_token,
    impl_pip$_vendor$html5lib$filters$sanitizer$$$function__6_sanitize_css,
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

    function_impl_code *current = functable_pip$_vendor$html5lib$filters$sanitizer;
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

    if (offset > sizeof(functable_pip$_vendor$html5lib$filters$sanitizer) || offset < 0) {
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
        functable_pip$_vendor$html5lib$filters$sanitizer[offset],
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
        module_pip$_vendor$html5lib$filters$sanitizer,
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
PyObject *modulecode_pip$_vendor$html5lib$filters$sanitizer(PyObject *module, struct Nuitka_MetaPathBasedLoaderEntry const *module_entry) {
    module_pip$_vendor$html5lib$filters$sanitizer = module;

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
    PRINT_STRING("pip._vendor.html5lib.filters.sanitizer: Calling setupMetaPathBasedLoader().\n");
#endif
    setupMetaPathBasedLoader();

#if PYTHON_VERSION >= 0x300
    patchInspectModule();
#endif

#endif

    /* The constants only used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.html5lib.filters.sanitizer: Calling createModuleConstants().\n");
#endif
    createModuleConstants();

    /* The code objects used by this module are created now. */
#ifdef _NUITKA_TRACE
    PRINT_STRING("pip._vendor.html5lib.filters.sanitizer: Calling createModuleCodeObjects().\n");
#endif
    createModuleCodeObjects();

    // PRINT_STRING("in initpip$_vendor$html5lib$filters$sanitizer\n");

    // Create the module object first. There are no methods initially, all are
    // added dynamically in actual code only.  Also no "__doc__" is initially
    // set at this time, as it could not contain NUL characters this way, they
    // are instead set in early module code.  No "self" for modules, we have no
    // use for it.

    moduledict_pip$_vendor$html5lib$filters$sanitizer = MODULE_DICT(module_pip$_vendor$html5lib$filters$sanitizer);

#ifdef _NUITKA_PLUGIN_DILL_ENABLED
    registerDillPluginTables(module_entry->name, &_method_def_reduce_compiled_function, &_method_def_create_compiled_function);
#endif

    // Set "__compiled__" to what version information we have.
    UPDATE_STRING_DICT0(
        moduledict_pip$_vendor$html5lib$filters$sanitizer,
        (Nuitka_StringObject *)const_str_plain___compiled__,
        Nuitka_dunder_compiled_value
    );

    // Update "__package__" value to what it ought to be.
    {
#if 0
        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$html5lib$filters$sanitizer,
            (Nuitka_StringObject *)const_str_plain___package__,
            const_str_empty
        );
#elif 0
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___name__);

        UPDATE_STRING_DICT0(
            moduledict_pip$_vendor$html5lib$filters$sanitizer,
            (Nuitka_StringObject *)const_str_plain___package__,
            module_name
        );
#else

#if PYTHON_VERSION < 0x300
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___name__);
        char const *module_name_cstr = PyString_AS_STRING(module_name);

        char const *last_dot = strrchr(module_name_cstr, '.');

        if (last_dot != NULL) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$html5lib$filters$sanitizer,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyString_FromStringAndSize(module_name_cstr, last_dot - module_name_cstr)
            );
        }
#else
        PyObject *module_name = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___name__);
        Py_ssize_t dot_index = PyUnicode_Find(module_name, const_str_dot, 0, PyUnicode_GetLength(module_name), -1);

        if (dot_index != -1) {
            UPDATE_STRING_DICT1(
                moduledict_pip$_vendor$html5lib$filters$sanitizer,
                (Nuitka_StringObject *)const_str_plain___package__,
                PyUnicode_Substring(module_name, 0, dot_index)
            );
        }
#endif
#endif
    }

    CHECK_OBJECT(module_pip$_vendor$html5lib$filters$sanitizer);

    // For deep importing of a module we need to have "__builtins__", so we set
    // it ourselves in the same way than CPython does. Note: This must be done
    // before the frame object is allocated, or else it may fail.

    if (GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___builtins__) == NULL) {
        PyObject *value = (PyObject *)builtin_module;

        // Check if main module, not a dict then but the module itself.
#if !defined(_NUITKA_EXE) || !0
        value = PyModule_GetDict(value);
#endif

        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___builtins__, value);
    }

#if PYTHON_VERSION >= 0x300
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___loader__, (PyObject *)&Nuitka_Loader_Type);
#endif

#if PYTHON_VERSION >= 0x340
// Set the "__spec__" value

#if 0
    // Main modules just get "None" as spec.
    UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___spec__, Py_None);
#else
    // Other modules get a "ModuleSpec" from the standard mechanism.
    {
        PyObject *bootstrap_module = getImportLibBootstrapModule();
        CHECK_OBJECT(bootstrap_module);

        PyObject *_spec_from_module = PyObject_GetAttrString(bootstrap_module, "_spec_from_module");
        CHECK_OBJECT(_spec_from_module);

        PyObject *spec_value = CALL_FUNCTION_WITH_SINGLE_ARG(_spec_from_module, module_pip$_vendor$html5lib$filters$sanitizer);
        Py_DECREF(_spec_from_module);

        // We can assume this to never fail, or else we are in trouble anyway.
        // CHECK_OBJECT(spec_value);

        if (spec_value == NULL) {
            PyErr_PrintEx(0);
            abort();
        }

// Mark the execution in the "__spec__" value.
        SET_ATTRIBUTE(spec_value, const_str_plain__initializing, Py_True);

        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)const_str_plain___spec__, spec_value);
    }
#endif
#endif

    // Temp variables if any
    PyObject *outline_0_var___class__ = NULL;
    PyObject *tmp_class_creation_1__bases = NULL;
    PyObject *tmp_class_creation_1__class_decl_dict = NULL;
    PyObject *tmp_class_creation_1__metaclass = NULL;
    PyObject *tmp_class_creation_1__prepared = NULL;
    PyObject *tmp_import_from_1__module = NULL;
    PyObject *tmp_import_from_2__module = NULL;
    PyObject *tmp_import_from_3__module = NULL;
    struct Nuitka_FrameObject *frame_6bde854d712dad430c9ea5c5ac22e83b;
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
    PyObject *exception_keeper_type_3;
    PyObject *exception_keeper_value_3;
    PyTracebackObject *exception_keeper_tb_3;
    NUITKA_MAY_BE_UNUSED int exception_keeper_lineno_3;
    NUITKA_MAY_BE_UNUSED nuitka_void tmp_unused;
    int tmp_res;
    PyObject *tmp_dictdel_dict;
    PyObject *tmp_dictdel_key;
    PyObject *locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724 = NULL;
    PyObject *tmp_dictset_value;
    struct Nuitka_FrameObject *frame_4dc3aedd5f581d86900fc22db4cf5702_2;
    NUITKA_MAY_BE_UNUSED char const *type_description_2 = NULL;
    static struct Nuitka_FrameObject *cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2 = NULL;
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

    // Module code.
    {
        PyObject *tmp_assign_source_1;
        tmp_assign_source_1 = mod_consts[96];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[97], tmp_assign_source_1);
    }
    {
        PyObject *tmp_assign_source_2;
        tmp_assign_source_2 = mod_consts[98];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[99], tmp_assign_source_2);
    }
    // Frame without reuse.
    frame_6bde854d712dad430c9ea5c5ac22e83b = MAKE_MODULE_FRAME(codeobj_6bde854d712dad430c9ea5c5ac22e83b, module_pip$_vendor$html5lib$filters$sanitizer);

    // Push the new frame as the currently active one, and we should be exclusively
    // owning it.
    pushFrameStack(frame_6bde854d712dad430c9ea5c5ac22e83b);
    assert(Py_REFCNT(frame_6bde854d712dad430c9ea5c5ac22e83b) == 2);

    // Framed code:
    {
        PyObject *tmp_assattr_name_1;
        PyObject *tmp_assattr_target_1;
        tmp_assattr_name_1 = mod_consts[98];
        tmp_assattr_target_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[100]);

        if (unlikely(tmp_assattr_target_1 == NULL)) {
            tmp_assattr_target_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[100]);
        }

        assert(!(tmp_assattr_target_1 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_1, mod_consts[101], tmp_assattr_name_1);
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
        tmp_assattr_target_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[100]);

        if (unlikely(tmp_assattr_target_2 == NULL)) {
            tmp_assattr_target_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[100]);
        }

        assert(!(tmp_assattr_target_2 == NULL));
        tmp_result = SET_ATTRIBUTE(tmp_assattr_target_2, mod_consts[102], tmp_assattr_name_2);
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
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[103], tmp_assign_source_3);
    }
    {
        PyObject *tmp_assign_source_4;
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 9;
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
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[104],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_5 = IMPORT_NAME(tmp_import_name_from_1, mod_consts[104]);
        }

        if (tmp_assign_source_5 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 9;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[104], tmp_assign_source_5);
    }
    {
        PyObject *tmp_assign_source_6;
        PyObject *tmp_import_name_from_2;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_2 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_2)) {
            tmp_assign_source_6 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_2,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[105],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_6 = IMPORT_NAME(tmp_import_name_from_2, mod_consts[105]);
        }

        if (tmp_assign_source_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 9;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[105], tmp_assign_source_6);
    }
    {
        PyObject *tmp_assign_source_7;
        PyObject *tmp_import_name_from_3;
        CHECK_OBJECT(tmp_import_from_1__module);
        tmp_import_name_from_3 = tmp_import_from_1__module;
        if (PyModule_Check(tmp_import_name_from_3)) {
            tmp_assign_source_7 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_3,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[106],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_7 = IMPORT_NAME(tmp_import_name_from_3, mod_consts[106]);
        }

        if (tmp_assign_source_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 9;

            goto try_except_handler_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[106], tmp_assign_source_7);
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
        tmp_name_name_1 = mod_consts[37];
        tmp_globals_arg_name_1 = (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer;
        tmp_locals_arg_name_1 = Py_None;
        tmp_fromlist_name_1 = Py_None;
        tmp_level_name_1 = mod_consts[92];
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 11;
        tmp_assign_source_8 = IMPORT_MODULE5(tmp_name_name_1, tmp_globals_arg_name_1, tmp_locals_arg_name_1, tmp_fromlist_name_1, tmp_level_name_1);
        if (tmp_assign_source_8 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 11;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37], tmp_assign_source_8);
    }
    {
        PyObject *tmp_assign_source_9;
        PyObject *tmp_name_name_2;
        PyObject *tmp_globals_arg_name_2;
        PyObject *tmp_locals_arg_name_2;
        PyObject *tmp_fromlist_name_2;
        PyObject *tmp_level_name_2;
        tmp_name_name_2 = mod_consts[2];
        tmp_globals_arg_name_2 = (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer;
        tmp_locals_arg_name_2 = Py_None;
        tmp_fromlist_name_2 = Py_None;
        tmp_level_name_2 = mod_consts[92];
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 12;
        tmp_assign_source_9 = IMPORT_MODULE5(tmp_name_name_2, tmp_globals_arg_name_2, tmp_locals_arg_name_2, tmp_fromlist_name_2, tmp_level_name_2);
        if (tmp_assign_source_9 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 12;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[2], tmp_assign_source_9);
    }
    {
        PyObject *tmp_assign_source_10;
        PyObject *tmp_name_name_3;
        PyObject *tmp_globals_arg_name_3;
        PyObject *tmp_locals_arg_name_3;
        PyObject *tmp_fromlist_name_3;
        PyObject *tmp_level_name_3;
        tmp_name_name_3 = mod_consts[107];
        tmp_globals_arg_name_3 = (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer;
        tmp_locals_arg_name_3 = Py_None;
        tmp_fromlist_name_3 = mod_consts[108];
        tmp_level_name_3 = mod_consts[92];
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 13;
        tmp_assign_source_10 = IMPORT_MODULE5(tmp_name_name_3, tmp_globals_arg_name_3, tmp_locals_arg_name_3, tmp_fromlist_name_3, tmp_level_name_3);
        if (tmp_assign_source_10 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 13;

            goto frame_exception_exit_1;
        }
        assert(tmp_import_from_2__module == NULL);
        tmp_import_from_2__module = tmp_assign_source_10;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_11;
        PyObject *tmp_import_name_from_4;
        CHECK_OBJECT(tmp_import_from_2__module);
        tmp_import_name_from_4 = tmp_import_from_2__module;
        if (PyModule_Check(tmp_import_name_from_4)) {
            tmp_assign_source_11 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_4,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[70],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_11 = IMPORT_NAME(tmp_import_name_from_4, mod_consts[70]);
        }

        if (tmp_assign_source_11 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 13;

            goto try_except_handler_2;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[70], tmp_assign_source_11);
    }
    {
        PyObject *tmp_assign_source_12;
        PyObject *tmp_import_name_from_5;
        CHECK_OBJECT(tmp_import_from_2__module);
        tmp_import_name_from_5 = tmp_import_from_2__module;
        if (PyModule_Check(tmp_import_name_from_5)) {
            tmp_assign_source_12 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_5,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[41],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_12 = IMPORT_NAME(tmp_import_name_from_5, mod_consts[41]);
        }

        if (tmp_assign_source_12 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 13;

            goto try_except_handler_2;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[41], tmp_assign_source_12);
    }
    goto try_end_2;
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

    CHECK_OBJECT(tmp_import_from_2__module);
    Py_DECREF(tmp_import_from_2__module);
    tmp_import_from_2__module = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_2;
    exception_value = exception_keeper_value_2;
    exception_tb = exception_keeper_tb_2;
    exception_lineno = exception_keeper_lineno_2;

    goto frame_exception_exit_1;
    // End of try:
    try_end_2:;
    CHECK_OBJECT(tmp_import_from_2__module);
    Py_DECREF(tmp_import_from_2__module);
    tmp_import_from_2__module = NULL;
    {
        PyObject *tmp_assign_source_13;
        PyObject *tmp_import_name_from_6;
        PyObject *tmp_name_name_4;
        PyObject *tmp_globals_arg_name_4;
        PyObject *tmp_locals_arg_name_4;
        PyObject *tmp_fromlist_name_4;
        PyObject *tmp_level_name_4;
        tmp_name_name_4 = mod_consts[109];
        tmp_globals_arg_name_4 = (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer;
        tmp_locals_arg_name_4 = Py_None;
        tmp_fromlist_name_4 = mod_consts[110];
        tmp_level_name_4 = mod_consts[92];
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 15;
        tmp_import_name_from_6 = IMPORT_MODULE5(tmp_name_name_4, tmp_globals_arg_name_4, tmp_locals_arg_name_4, tmp_fromlist_name_4, tmp_level_name_4);
        if (tmp_import_name_from_6 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 15;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_6)) {
            tmp_assign_source_13 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_6,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[111],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_13 = IMPORT_NAME(tmp_import_name_from_6, mod_consts[111]);
        }

        Py_DECREF(tmp_import_name_from_6);
        if (tmp_assign_source_13 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 15;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[45], tmp_assign_source_13);
    }
    {
        PyObject *tmp_assign_source_14;
        PyObject *tmp_import_name_from_7;
        PyObject *tmp_name_name_5;
        PyObject *tmp_globals_arg_name_5;
        PyObject *tmp_locals_arg_name_5;
        PyObject *tmp_fromlist_name_5;
        PyObject *tmp_level_name_5;
        tmp_name_name_5 = mod_consts[40];
        tmp_globals_arg_name_5 = (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer;
        tmp_locals_arg_name_5 = Py_None;
        tmp_fromlist_name_5 = mod_consts[112];
        tmp_level_name_5 = mod_consts[113];
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 17;
        tmp_import_name_from_7 = IMPORT_MODULE5(tmp_name_name_5, tmp_globals_arg_name_5, tmp_locals_arg_name_5, tmp_fromlist_name_5, tmp_level_name_5);
        if (tmp_import_name_from_7 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 17;

            goto frame_exception_exit_1;
        }
        if (PyModule_Check(tmp_import_name_from_7)) {
            tmp_assign_source_14 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_7,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[17],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_14 = IMPORT_NAME(tmp_import_name_from_7, mod_consts[17]);
        }

        Py_DECREF(tmp_import_name_from_7);
        if (tmp_assign_source_14 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 17;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[17], tmp_assign_source_14);
    }
    {
        PyObject *tmp_assign_source_15;
        PyObject *tmp_name_name_6;
        PyObject *tmp_globals_arg_name_6;
        PyObject *tmp_locals_arg_name_6;
        PyObject *tmp_fromlist_name_6;
        PyObject *tmp_level_name_6;
        tmp_name_name_6 = mod_consts[114];
        tmp_globals_arg_name_6 = (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer;
        tmp_locals_arg_name_6 = Py_None;
        tmp_fromlist_name_6 = mod_consts[115];
        tmp_level_name_6 = mod_consts[116];
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 18;
        tmp_assign_source_15 = IMPORT_MODULE5(tmp_name_name_6, tmp_globals_arg_name_6, tmp_locals_arg_name_6, tmp_fromlist_name_6, tmp_level_name_6);
        if (tmp_assign_source_15 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 18;

            goto frame_exception_exit_1;
        }
        assert(tmp_import_from_3__module == NULL);
        tmp_import_from_3__module = tmp_assign_source_15;
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_16;
        PyObject *tmp_import_name_from_8;
        CHECK_OBJECT(tmp_import_from_3__module);
        tmp_import_name_from_8 = tmp_import_from_3__module;
        if (PyModule_Check(tmp_import_name_from_8)) {
            tmp_assign_source_16 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_8,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[26],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_16 = IMPORT_NAME(tmp_import_name_from_8, mod_consts[26]);
        }

        if (tmp_assign_source_16 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 18;

            goto try_except_handler_3;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26], tmp_assign_source_16);
    }
    {
        PyObject *tmp_assign_source_17;
        PyObject *tmp_import_name_from_9;
        CHECK_OBJECT(tmp_import_from_3__module);
        tmp_import_name_from_9 = tmp_import_from_3__module;
        if (PyModule_Check(tmp_import_name_from_9)) {
            tmp_assign_source_17 = IMPORT_NAME_OR_MODULE(
                tmp_import_name_from_9,
                (PyObject *)moduledict_pip$_vendor$html5lib$filters$sanitizer,
                mod_consts[69],
                mod_consts[92]
            );
        } else {
            tmp_assign_source_17 = IMPORT_NAME(tmp_import_name_from_9, mod_consts[69]);
        }

        if (tmp_assign_source_17 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 18;

            goto try_except_handler_3;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[69], tmp_assign_source_17);
    }
    goto try_end_3;
    // Exception handler code:
    try_except_handler_3:;
    exception_keeper_type_3 = exception_type;
    exception_keeper_value_3 = exception_value;
    exception_keeper_tb_3 = exception_tb;
    exception_keeper_lineno_3 = exception_lineno;
    exception_type = NULL;
    exception_value = NULL;
    exception_tb = NULL;
    exception_lineno = 0;

    CHECK_OBJECT(tmp_import_from_3__module);
    Py_DECREF(tmp_import_from_3__module);
    tmp_import_from_3__module = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_3;
    exception_value = exception_keeper_value_3;
    exception_tb = exception_keeper_tb_3;
    exception_lineno = exception_keeper_lineno_3;

    goto frame_exception_exit_1;
    // End of try:
    try_end_3:;
    CHECK_OBJECT(tmp_import_from_3__module);
    Py_DECREF(tmp_import_from_3__module);
    tmp_import_from_3__module = NULL;
    {
        PyObject *tmp_assign_source_18;
        tmp_assign_source_18 = LIST_COPY(mod_consts[117]);
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[118], tmp_assign_source_18);
    }
    {
        PyObject *tmp_assign_source_19;
        tmp_assign_source_19 = mod_consts[119];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[4], tmp_assign_source_19);
    }
    {
        PyObject *tmp_called_name_1;
        PyObject *tmp_expression_name_1;
        PyObject *tmp_call_result_1;
        PyObject *tmp_args_element_name_1;
        PyObject *tmp_args_element_name_2;
        tmp_expression_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[2]);

        if (unlikely(tmp_expression_name_1 == NULL)) {
            tmp_expression_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[2]);
        }

        if (tmp_expression_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;

            goto frame_exception_exit_1;
        }
        tmp_called_name_1 = LOOKUP_ATTRIBUTE(tmp_expression_name_1, mod_consts[3]);
        if (tmp_called_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_1 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[4]);

        if (unlikely(tmp_args_element_name_1 == NULL)) {
            tmp_args_element_name_1 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[4]);
        }

        if (tmp_args_element_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 29;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[5]);

        if (unlikely(tmp_args_element_name_2 == NULL)) {
            tmp_args_element_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[5]);
        }

        if (tmp_args_element_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_1);

            exception_lineno = 29;

            goto frame_exception_exit_1;
        }
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 29;
        {
            PyObject *call_args[] = {tmp_args_element_name_1, tmp_args_element_name_2};
            tmp_call_result_1 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_1, call_args);
        }

        Py_DECREF(tmp_called_name_1);
        if (tmp_call_result_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 29;

            goto frame_exception_exit_1;
        }
        Py_DECREF(tmp_call_result_1);
    }
    {
        PyObject *tmp_assign_source_20;
        PyObject *tmp_frozenset_arg_1;
        PyObject *tmp_tuple_element_1;
        PyObject *tmp_tuple_element_2;
        PyObject *tmp_expression_name_2;
        PyObject *tmp_subscript_name_1;
        tmp_expression_name_2 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

        if (unlikely(tmp_expression_name_2 == NULL)) {
            tmp_expression_name_2 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
        }

        if (tmp_expression_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto frame_exception_exit_1;
        }
        tmp_subscript_name_1 = mod_consts[27];
        tmp_tuple_element_2 = LOOKUP_SUBSCRIPT(tmp_expression_name_2, tmp_subscript_name_1);
        if (tmp_tuple_element_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 32;

            goto frame_exception_exit_1;
        }
        tmp_tuple_element_1 = PyTuple_New(2);
        PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_2);
        tmp_tuple_element_2 = mod_consts[120];
        PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_2);
        tmp_frozenset_arg_1 = PyTuple_New(161);
        {
            PyObject *tmp_tuple_element_3;
            PyObject *tmp_expression_name_3;
            PyObject *tmp_subscript_name_2;
            PyObject *tmp_tuple_element_4;
            PyObject *tmp_expression_name_4;
            PyObject *tmp_subscript_name_3;
            PyObject *tmp_tuple_element_5;
            PyObject *tmp_expression_name_5;
            PyObject *tmp_subscript_name_4;
            PyObject *tmp_tuple_element_6;
            PyObject *tmp_expression_name_6;
            PyObject *tmp_subscript_name_5;
            PyObject *tmp_tuple_element_7;
            PyObject *tmp_expression_name_7;
            PyObject *tmp_subscript_name_6;
            PyObject *tmp_tuple_element_8;
            PyObject *tmp_expression_name_8;
            PyObject *tmp_subscript_name_7;
            PyObject *tmp_tuple_element_9;
            PyObject *tmp_expression_name_9;
            PyObject *tmp_subscript_name_8;
            PyObject *tmp_tuple_element_10;
            PyObject *tmp_expression_name_10;
            PyObject *tmp_subscript_name_9;
            PyObject *tmp_tuple_element_11;
            PyObject *tmp_expression_name_11;
            PyObject *tmp_subscript_name_10;
            PyObject *tmp_tuple_element_12;
            PyObject *tmp_expression_name_12;
            PyObject *tmp_subscript_name_11;
            PyObject *tmp_tuple_element_13;
            PyObject *tmp_expression_name_13;
            PyObject *tmp_subscript_name_12;
            PyObject *tmp_tuple_element_14;
            PyObject *tmp_expression_name_14;
            PyObject *tmp_subscript_name_13;
            PyObject *tmp_tuple_element_15;
            PyObject *tmp_expression_name_15;
            PyObject *tmp_subscript_name_14;
            PyObject *tmp_tuple_element_16;
            PyObject *tmp_expression_name_16;
            PyObject *tmp_subscript_name_15;
            PyObject *tmp_tuple_element_17;
            PyObject *tmp_expression_name_17;
            PyObject *tmp_subscript_name_16;
            PyObject *tmp_tuple_element_18;
            PyObject *tmp_expression_name_18;
            PyObject *tmp_subscript_name_17;
            PyObject *tmp_tuple_element_19;
            PyObject *tmp_expression_name_19;
            PyObject *tmp_subscript_name_18;
            PyObject *tmp_tuple_element_20;
            PyObject *tmp_expression_name_20;
            PyObject *tmp_subscript_name_19;
            PyObject *tmp_tuple_element_21;
            PyObject *tmp_expression_name_21;
            PyObject *tmp_subscript_name_20;
            PyObject *tmp_tuple_element_22;
            PyObject *tmp_expression_name_22;
            PyObject *tmp_subscript_name_21;
            PyObject *tmp_tuple_element_23;
            PyObject *tmp_expression_name_23;
            PyObject *tmp_subscript_name_22;
            PyObject *tmp_tuple_element_24;
            PyObject *tmp_expression_name_24;
            PyObject *tmp_subscript_name_23;
            PyObject *tmp_tuple_element_25;
            PyObject *tmp_expression_name_25;
            PyObject *tmp_subscript_name_24;
            PyObject *tmp_tuple_element_26;
            PyObject *tmp_expression_name_26;
            PyObject *tmp_subscript_name_25;
            PyObject *tmp_tuple_element_27;
            PyObject *tmp_expression_name_27;
            PyObject *tmp_subscript_name_26;
            PyObject *tmp_tuple_element_28;
            PyObject *tmp_expression_name_28;
            PyObject *tmp_subscript_name_27;
            PyObject *tmp_tuple_element_29;
            PyObject *tmp_expression_name_29;
            PyObject *tmp_subscript_name_28;
            PyObject *tmp_tuple_element_30;
            PyObject *tmp_expression_name_30;
            PyObject *tmp_subscript_name_29;
            PyObject *tmp_tuple_element_31;
            PyObject *tmp_expression_name_31;
            PyObject *tmp_subscript_name_30;
            PyObject *tmp_tuple_element_32;
            PyObject *tmp_expression_name_32;
            PyObject *tmp_subscript_name_31;
            PyObject *tmp_tuple_element_33;
            PyObject *tmp_expression_name_33;
            PyObject *tmp_subscript_name_32;
            PyObject *tmp_tuple_element_34;
            PyObject *tmp_expression_name_34;
            PyObject *tmp_subscript_name_33;
            PyObject *tmp_tuple_element_35;
            PyObject *tmp_expression_name_35;
            PyObject *tmp_subscript_name_34;
            PyObject *tmp_tuple_element_36;
            PyObject *tmp_expression_name_36;
            PyObject *tmp_subscript_name_35;
            PyObject *tmp_tuple_element_37;
            PyObject *tmp_expression_name_37;
            PyObject *tmp_subscript_name_36;
            PyObject *tmp_tuple_element_38;
            PyObject *tmp_expression_name_38;
            PyObject *tmp_subscript_name_37;
            PyObject *tmp_tuple_element_39;
            PyObject *tmp_expression_name_39;
            PyObject *tmp_subscript_name_38;
            PyObject *tmp_tuple_element_40;
            PyObject *tmp_expression_name_40;
            PyObject *tmp_subscript_name_39;
            PyObject *tmp_tuple_element_41;
            PyObject *tmp_expression_name_41;
            PyObject *tmp_subscript_name_40;
            PyObject *tmp_tuple_element_42;
            PyObject *tmp_expression_name_42;
            PyObject *tmp_subscript_name_41;
            PyObject *tmp_tuple_element_43;
            PyObject *tmp_expression_name_43;
            PyObject *tmp_subscript_name_42;
            PyObject *tmp_tuple_element_44;
            PyObject *tmp_expression_name_44;
            PyObject *tmp_subscript_name_43;
            PyObject *tmp_tuple_element_45;
            PyObject *tmp_expression_name_45;
            PyObject *tmp_subscript_name_44;
            PyObject *tmp_tuple_element_46;
            PyObject *tmp_expression_name_46;
            PyObject *tmp_subscript_name_45;
            PyObject *tmp_tuple_element_47;
            PyObject *tmp_expression_name_47;
            PyObject *tmp_subscript_name_46;
            PyObject *tmp_tuple_element_48;
            PyObject *tmp_expression_name_48;
            PyObject *tmp_subscript_name_47;
            PyObject *tmp_tuple_element_49;
            PyObject *tmp_expression_name_49;
            PyObject *tmp_subscript_name_48;
            PyObject *tmp_tuple_element_50;
            PyObject *tmp_expression_name_50;
            PyObject *tmp_subscript_name_49;
            PyObject *tmp_tuple_element_51;
            PyObject *tmp_expression_name_51;
            PyObject *tmp_subscript_name_50;
            PyObject *tmp_tuple_element_52;
            PyObject *tmp_expression_name_52;
            PyObject *tmp_subscript_name_51;
            PyObject *tmp_tuple_element_53;
            PyObject *tmp_expression_name_53;
            PyObject *tmp_subscript_name_52;
            PyObject *tmp_tuple_element_54;
            PyObject *tmp_expression_name_54;
            PyObject *tmp_subscript_name_53;
            PyObject *tmp_tuple_element_55;
            PyObject *tmp_expression_name_55;
            PyObject *tmp_subscript_name_54;
            PyObject *tmp_tuple_element_56;
            PyObject *tmp_expression_name_56;
            PyObject *tmp_subscript_name_55;
            PyObject *tmp_tuple_element_57;
            PyObject *tmp_expression_name_57;
            PyObject *tmp_subscript_name_56;
            PyObject *tmp_tuple_element_58;
            PyObject *tmp_expression_name_58;
            PyObject *tmp_subscript_name_57;
            PyObject *tmp_tuple_element_59;
            PyObject *tmp_expression_name_59;
            PyObject *tmp_subscript_name_58;
            PyObject *tmp_tuple_element_60;
            PyObject *tmp_expression_name_60;
            PyObject *tmp_subscript_name_59;
            PyObject *tmp_tuple_element_61;
            PyObject *tmp_expression_name_61;
            PyObject *tmp_subscript_name_60;
            PyObject *tmp_tuple_element_62;
            PyObject *tmp_expression_name_62;
            PyObject *tmp_subscript_name_61;
            PyObject *tmp_tuple_element_63;
            PyObject *tmp_expression_name_63;
            PyObject *tmp_subscript_name_62;
            PyObject *tmp_tuple_element_64;
            PyObject *tmp_expression_name_64;
            PyObject *tmp_subscript_name_63;
            PyObject *tmp_tuple_element_65;
            PyObject *tmp_expression_name_65;
            PyObject *tmp_subscript_name_64;
            PyObject *tmp_tuple_element_66;
            PyObject *tmp_expression_name_66;
            PyObject *tmp_subscript_name_65;
            PyObject *tmp_tuple_element_67;
            PyObject *tmp_expression_name_67;
            PyObject *tmp_subscript_name_66;
            PyObject *tmp_tuple_element_68;
            PyObject *tmp_expression_name_68;
            PyObject *tmp_subscript_name_67;
            PyObject *tmp_tuple_element_69;
            PyObject *tmp_expression_name_69;
            PyObject *tmp_subscript_name_68;
            PyObject *tmp_tuple_element_70;
            PyObject *tmp_expression_name_70;
            PyObject *tmp_subscript_name_69;
            PyObject *tmp_tuple_element_71;
            PyObject *tmp_expression_name_71;
            PyObject *tmp_subscript_name_70;
            PyObject *tmp_tuple_element_72;
            PyObject *tmp_expression_name_72;
            PyObject *tmp_subscript_name_71;
            PyObject *tmp_tuple_element_73;
            PyObject *tmp_expression_name_73;
            PyObject *tmp_subscript_name_72;
            PyObject *tmp_tuple_element_74;
            PyObject *tmp_expression_name_74;
            PyObject *tmp_subscript_name_73;
            PyObject *tmp_tuple_element_75;
            PyObject *tmp_expression_name_75;
            PyObject *tmp_subscript_name_74;
            PyObject *tmp_tuple_element_76;
            PyObject *tmp_expression_name_76;
            PyObject *tmp_subscript_name_75;
            PyObject *tmp_tuple_element_77;
            PyObject *tmp_expression_name_77;
            PyObject *tmp_subscript_name_76;
            PyObject *tmp_tuple_element_78;
            PyObject *tmp_expression_name_78;
            PyObject *tmp_subscript_name_77;
            PyObject *tmp_tuple_element_79;
            PyObject *tmp_expression_name_79;
            PyObject *tmp_subscript_name_78;
            PyObject *tmp_tuple_element_80;
            PyObject *tmp_expression_name_80;
            PyObject *tmp_subscript_name_79;
            PyObject *tmp_tuple_element_81;
            PyObject *tmp_expression_name_81;
            PyObject *tmp_subscript_name_80;
            PyObject *tmp_tuple_element_82;
            PyObject *tmp_expression_name_82;
            PyObject *tmp_subscript_name_81;
            PyObject *tmp_tuple_element_83;
            PyObject *tmp_expression_name_83;
            PyObject *tmp_subscript_name_82;
            PyObject *tmp_tuple_element_84;
            PyObject *tmp_expression_name_84;
            PyObject *tmp_subscript_name_83;
            PyObject *tmp_tuple_element_85;
            PyObject *tmp_expression_name_85;
            PyObject *tmp_subscript_name_84;
            PyObject *tmp_tuple_element_86;
            PyObject *tmp_expression_name_86;
            PyObject *tmp_subscript_name_85;
            PyObject *tmp_tuple_element_87;
            PyObject *tmp_expression_name_87;
            PyObject *tmp_subscript_name_86;
            PyObject *tmp_tuple_element_88;
            PyObject *tmp_expression_name_88;
            PyObject *tmp_subscript_name_87;
            PyObject *tmp_tuple_element_89;
            PyObject *tmp_expression_name_89;
            PyObject *tmp_subscript_name_88;
            PyObject *tmp_tuple_element_90;
            PyObject *tmp_expression_name_90;
            PyObject *tmp_subscript_name_89;
            PyObject *tmp_tuple_element_91;
            PyObject *tmp_expression_name_91;
            PyObject *tmp_subscript_name_90;
            PyObject *tmp_tuple_element_92;
            PyObject *tmp_expression_name_92;
            PyObject *tmp_subscript_name_91;
            PyObject *tmp_tuple_element_93;
            PyObject *tmp_expression_name_93;
            PyObject *tmp_subscript_name_92;
            PyObject *tmp_tuple_element_94;
            PyObject *tmp_expression_name_94;
            PyObject *tmp_subscript_name_93;
            PyObject *tmp_tuple_element_95;
            PyObject *tmp_expression_name_95;
            PyObject *tmp_subscript_name_94;
            PyObject *tmp_tuple_element_96;
            PyObject *tmp_expression_name_96;
            PyObject *tmp_subscript_name_95;
            PyObject *tmp_tuple_element_97;
            PyObject *tmp_expression_name_97;
            PyObject *tmp_subscript_name_96;
            PyObject *tmp_tuple_element_98;
            PyObject *tmp_expression_name_98;
            PyObject *tmp_subscript_name_97;
            PyObject *tmp_tuple_element_99;
            PyObject *tmp_expression_name_99;
            PyObject *tmp_subscript_name_98;
            PyObject *tmp_tuple_element_100;
            PyObject *tmp_expression_name_100;
            PyObject *tmp_subscript_name_99;
            PyObject *tmp_tuple_element_101;
            PyObject *tmp_expression_name_101;
            PyObject *tmp_subscript_name_100;
            PyObject *tmp_tuple_element_102;
            PyObject *tmp_expression_name_102;
            PyObject *tmp_subscript_name_101;
            PyObject *tmp_tuple_element_103;
            PyObject *tmp_expression_name_103;
            PyObject *tmp_subscript_name_102;
            PyObject *tmp_tuple_element_104;
            PyObject *tmp_expression_name_104;
            PyObject *tmp_subscript_name_103;
            PyObject *tmp_tuple_element_105;
            PyObject *tmp_expression_name_105;
            PyObject *tmp_subscript_name_104;
            PyObject *tmp_tuple_element_106;
            PyObject *tmp_expression_name_106;
            PyObject *tmp_subscript_name_105;
            PyObject *tmp_tuple_element_107;
            PyObject *tmp_expression_name_107;
            PyObject *tmp_subscript_name_106;
            PyObject *tmp_tuple_element_108;
            PyObject *tmp_expression_name_108;
            PyObject *tmp_subscript_name_107;
            PyObject *tmp_tuple_element_109;
            PyObject *tmp_expression_name_109;
            PyObject *tmp_subscript_name_108;
            PyObject *tmp_tuple_element_110;
            PyObject *tmp_expression_name_110;
            PyObject *tmp_subscript_name_109;
            PyObject *tmp_tuple_element_111;
            PyObject *tmp_expression_name_111;
            PyObject *tmp_subscript_name_110;
            PyObject *tmp_tuple_element_112;
            PyObject *tmp_expression_name_112;
            PyObject *tmp_subscript_name_111;
            PyObject *tmp_tuple_element_113;
            PyObject *tmp_expression_name_113;
            PyObject *tmp_subscript_name_112;
            PyObject *tmp_tuple_element_114;
            PyObject *tmp_expression_name_114;
            PyObject *tmp_subscript_name_113;
            PyObject *tmp_tuple_element_115;
            PyObject *tmp_expression_name_115;
            PyObject *tmp_subscript_name_114;
            PyObject *tmp_tuple_element_116;
            PyObject *tmp_expression_name_116;
            PyObject *tmp_subscript_name_115;
            PyObject *tmp_tuple_element_117;
            PyObject *tmp_expression_name_117;
            PyObject *tmp_subscript_name_116;
            PyObject *tmp_tuple_element_118;
            PyObject *tmp_expression_name_118;
            PyObject *tmp_subscript_name_117;
            PyObject *tmp_tuple_element_119;
            PyObject *tmp_expression_name_119;
            PyObject *tmp_subscript_name_118;
            PyObject *tmp_tuple_element_120;
            PyObject *tmp_expression_name_120;
            PyObject *tmp_subscript_name_119;
            PyObject *tmp_tuple_element_121;
            PyObject *tmp_expression_name_121;
            PyObject *tmp_subscript_name_120;
            PyObject *tmp_tuple_element_122;
            PyObject *tmp_expression_name_122;
            PyObject *tmp_subscript_name_121;
            PyObject *tmp_tuple_element_123;
            PyObject *tmp_expression_name_123;
            PyObject *tmp_subscript_name_122;
            PyObject *tmp_tuple_element_124;
            PyObject *tmp_expression_name_124;
            PyObject *tmp_subscript_name_123;
            PyObject *tmp_tuple_element_125;
            PyObject *tmp_expression_name_125;
            PyObject *tmp_subscript_name_124;
            PyObject *tmp_tuple_element_126;
            PyObject *tmp_expression_name_126;
            PyObject *tmp_subscript_name_125;
            PyObject *tmp_tuple_element_127;
            PyObject *tmp_expression_name_127;
            PyObject *tmp_subscript_name_126;
            PyObject *tmp_tuple_element_128;
            PyObject *tmp_expression_name_128;
            PyObject *tmp_subscript_name_127;
            PyObject *tmp_tuple_element_129;
            PyObject *tmp_expression_name_129;
            PyObject *tmp_subscript_name_128;
            PyObject *tmp_tuple_element_130;
            PyObject *tmp_expression_name_130;
            PyObject *tmp_subscript_name_129;
            PyObject *tmp_tuple_element_131;
            PyObject *tmp_expression_name_131;
            PyObject *tmp_subscript_name_130;
            PyObject *tmp_tuple_element_132;
            PyObject *tmp_expression_name_132;
            PyObject *tmp_subscript_name_131;
            PyObject *tmp_tuple_element_133;
            PyObject *tmp_expression_name_133;
            PyObject *tmp_subscript_name_132;
            PyObject *tmp_tuple_element_134;
            PyObject *tmp_expression_name_134;
            PyObject *tmp_subscript_name_133;
            PyObject *tmp_tuple_element_135;
            PyObject *tmp_expression_name_135;
            PyObject *tmp_subscript_name_134;
            PyObject *tmp_tuple_element_136;
            PyObject *tmp_expression_name_136;
            PyObject *tmp_subscript_name_135;
            PyObject *tmp_tuple_element_137;
            PyObject *tmp_expression_name_137;
            PyObject *tmp_subscript_name_136;
            PyObject *tmp_tuple_element_138;
            PyObject *tmp_expression_name_138;
            PyObject *tmp_subscript_name_137;
            PyObject *tmp_tuple_element_139;
            PyObject *tmp_expression_name_139;
            PyObject *tmp_subscript_name_138;
            PyObject *tmp_tuple_element_140;
            PyObject *tmp_expression_name_140;
            PyObject *tmp_subscript_name_139;
            PyObject *tmp_tuple_element_141;
            PyObject *tmp_expression_name_141;
            PyObject *tmp_subscript_name_140;
            PyObject *tmp_tuple_element_142;
            PyObject *tmp_expression_name_142;
            PyObject *tmp_subscript_name_141;
            PyObject *tmp_tuple_element_143;
            PyObject *tmp_expression_name_143;
            PyObject *tmp_subscript_name_142;
            PyObject *tmp_tuple_element_144;
            PyObject *tmp_expression_name_144;
            PyObject *tmp_subscript_name_143;
            PyObject *tmp_tuple_element_145;
            PyObject *tmp_expression_name_145;
            PyObject *tmp_subscript_name_144;
            PyObject *tmp_tuple_element_146;
            PyObject *tmp_expression_name_146;
            PyObject *tmp_subscript_name_145;
            PyObject *tmp_tuple_element_147;
            PyObject *tmp_expression_name_147;
            PyObject *tmp_subscript_name_146;
            PyObject *tmp_tuple_element_148;
            PyObject *tmp_expression_name_148;
            PyObject *tmp_subscript_name_147;
            PyObject *tmp_tuple_element_149;
            PyObject *tmp_expression_name_149;
            PyObject *tmp_subscript_name_148;
            PyObject *tmp_tuple_element_150;
            PyObject *tmp_expression_name_150;
            PyObject *tmp_subscript_name_149;
            PyObject *tmp_tuple_element_151;
            PyObject *tmp_expression_name_151;
            PyObject *tmp_subscript_name_150;
            PyObject *tmp_tuple_element_152;
            PyObject *tmp_expression_name_152;
            PyObject *tmp_subscript_name_151;
            PyObject *tmp_tuple_element_153;
            PyObject *tmp_expression_name_153;
            PyObject *tmp_subscript_name_152;
            PyObject *tmp_tuple_element_154;
            PyObject *tmp_expression_name_154;
            PyObject *tmp_subscript_name_153;
            PyObject *tmp_tuple_element_155;
            PyObject *tmp_expression_name_155;
            PyObject *tmp_subscript_name_154;
            PyObject *tmp_tuple_element_156;
            PyObject *tmp_expression_name_156;
            PyObject *tmp_subscript_name_155;
            PyObject *tmp_tuple_element_157;
            PyObject *tmp_expression_name_157;
            PyObject *tmp_subscript_name_156;
            PyObject *tmp_tuple_element_158;
            PyObject *tmp_expression_name_158;
            PyObject *tmp_subscript_name_157;
            PyObject *tmp_tuple_element_159;
            PyObject *tmp_expression_name_159;
            PyObject *tmp_subscript_name_158;
            PyObject *tmp_tuple_element_160;
            PyObject *tmp_expression_name_160;
            PyObject *tmp_subscript_name_159;
            PyObject *tmp_tuple_element_161;
            PyObject *tmp_expression_name_161;
            PyObject *tmp_subscript_name_160;
            PyObject *tmp_tuple_element_162;
            PyObject *tmp_expression_name_162;
            PyObject *tmp_subscript_name_161;
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 0, tmp_tuple_element_1);
            tmp_expression_name_3 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_3 == NULL)) {
                tmp_expression_name_3 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 33;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_2 = mod_consts[27];
            tmp_tuple_element_3 = LOOKUP_SUBSCRIPT(tmp_expression_name_3, tmp_subscript_name_2);
            if (tmp_tuple_element_3 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 33;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_3);
            tmp_tuple_element_3 = mod_consts[121];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_3);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 1, tmp_tuple_element_1);
            tmp_expression_name_4 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_4 == NULL)) {
                tmp_expression_name_4 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_4 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 34;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_3 = mod_consts[27];
            tmp_tuple_element_4 = LOOKUP_SUBSCRIPT(tmp_expression_name_4, tmp_subscript_name_3);
            if (tmp_tuple_element_4 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 34;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_4);
            tmp_tuple_element_4 = mod_consts[122];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_4);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 2, tmp_tuple_element_1);
            tmp_expression_name_5 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_5 == NULL)) {
                tmp_expression_name_5 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 35;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_4 = mod_consts[27];
            tmp_tuple_element_5 = LOOKUP_SUBSCRIPT(tmp_expression_name_5, tmp_subscript_name_4);
            if (tmp_tuple_element_5 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 35;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_5);
            tmp_tuple_element_5 = mod_consts[123];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_5);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 3, tmp_tuple_element_1);
            tmp_expression_name_6 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_6 == NULL)) {
                tmp_expression_name_6 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 36;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_5 = mod_consts[27];
            tmp_tuple_element_6 = LOOKUP_SUBSCRIPT(tmp_expression_name_6, tmp_subscript_name_5);
            if (tmp_tuple_element_6 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 36;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_6);
            tmp_tuple_element_6 = mod_consts[124];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_6);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 4, tmp_tuple_element_1);
            tmp_expression_name_7 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_7 == NULL)) {
                tmp_expression_name_7 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_7 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 37;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_6 = mod_consts[27];
            tmp_tuple_element_7 = LOOKUP_SUBSCRIPT(tmp_expression_name_7, tmp_subscript_name_6);
            if (tmp_tuple_element_7 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 37;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_7);
            tmp_tuple_element_7 = mod_consts[125];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_7);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 5, tmp_tuple_element_1);
            tmp_expression_name_8 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_8 == NULL)) {
                tmp_expression_name_8 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_8 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 38;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_7 = mod_consts[27];
            tmp_tuple_element_8 = LOOKUP_SUBSCRIPT(tmp_expression_name_8, tmp_subscript_name_7);
            if (tmp_tuple_element_8 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 38;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_8);
            tmp_tuple_element_8 = mod_consts[126];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_8);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 6, tmp_tuple_element_1);
            tmp_expression_name_9 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_9 == NULL)) {
                tmp_expression_name_9 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_9 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 39;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_8 = mod_consts[27];
            tmp_tuple_element_9 = LOOKUP_SUBSCRIPT(tmp_expression_name_9, tmp_subscript_name_8);
            if (tmp_tuple_element_9 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 39;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_9);
            tmp_tuple_element_9 = mod_consts[127];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_9);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 7, tmp_tuple_element_1);
            tmp_expression_name_10 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_10 == NULL)) {
                tmp_expression_name_10 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_10 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 40;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_9 = mod_consts[27];
            tmp_tuple_element_10 = LOOKUP_SUBSCRIPT(tmp_expression_name_10, tmp_subscript_name_9);
            if (tmp_tuple_element_10 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 40;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_10);
            tmp_tuple_element_10 = mod_consts[128];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_10);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 8, tmp_tuple_element_1);
            tmp_expression_name_11 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_11 == NULL)) {
                tmp_expression_name_11 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_11 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 41;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_10 = mod_consts[27];
            tmp_tuple_element_11 = LOOKUP_SUBSCRIPT(tmp_expression_name_11, tmp_subscript_name_10);
            if (tmp_tuple_element_11 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 41;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_11);
            tmp_tuple_element_11 = mod_consts[129];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_11);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 9, tmp_tuple_element_1);
            tmp_expression_name_12 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_12 == NULL)) {
                tmp_expression_name_12 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_12 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 42;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_11 = mod_consts[27];
            tmp_tuple_element_12 = LOOKUP_SUBSCRIPT(tmp_expression_name_12, tmp_subscript_name_11);
            if (tmp_tuple_element_12 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 42;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_12);
            tmp_tuple_element_12 = mod_consts[130];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_12);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 10, tmp_tuple_element_1);
            tmp_expression_name_13 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_13 == NULL)) {
                tmp_expression_name_13 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_13 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 43;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_12 = mod_consts[27];
            tmp_tuple_element_13 = LOOKUP_SUBSCRIPT(tmp_expression_name_13, tmp_subscript_name_12);
            if (tmp_tuple_element_13 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 43;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_13);
            tmp_tuple_element_13 = mod_consts[131];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_13);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 11, tmp_tuple_element_1);
            tmp_expression_name_14 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_14 == NULL)) {
                tmp_expression_name_14 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_14 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 44;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_13 = mod_consts[27];
            tmp_tuple_element_14 = LOOKUP_SUBSCRIPT(tmp_expression_name_14, tmp_subscript_name_13);
            if (tmp_tuple_element_14 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 44;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_14);
            tmp_tuple_element_14 = mod_consts[132];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_14);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 12, tmp_tuple_element_1);
            tmp_expression_name_15 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_15 == NULL)) {
                tmp_expression_name_15 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_15 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 45;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_14 = mod_consts[27];
            tmp_tuple_element_15 = LOOKUP_SUBSCRIPT(tmp_expression_name_15, tmp_subscript_name_14);
            if (tmp_tuple_element_15 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 45;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_15);
            tmp_tuple_element_15 = mod_consts[133];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_15);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 13, tmp_tuple_element_1);
            tmp_expression_name_16 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_16 == NULL)) {
                tmp_expression_name_16 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 46;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_15 = mod_consts[27];
            tmp_tuple_element_16 = LOOKUP_SUBSCRIPT(tmp_expression_name_16, tmp_subscript_name_15);
            if (tmp_tuple_element_16 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 46;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_16);
            tmp_tuple_element_16 = mod_consts[134];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_16);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 14, tmp_tuple_element_1);
            tmp_expression_name_17 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_17 == NULL)) {
                tmp_expression_name_17 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 47;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_16 = mod_consts[27];
            tmp_tuple_element_17 = LOOKUP_SUBSCRIPT(tmp_expression_name_17, tmp_subscript_name_16);
            if (tmp_tuple_element_17 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 47;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_17);
            tmp_tuple_element_17 = mod_consts[135];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_17);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 15, tmp_tuple_element_1);
            tmp_expression_name_18 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_18 == NULL)) {
                tmp_expression_name_18 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_18 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 48;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_17 = mod_consts[27];
            tmp_tuple_element_18 = LOOKUP_SUBSCRIPT(tmp_expression_name_18, tmp_subscript_name_17);
            if (tmp_tuple_element_18 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 48;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_18);
            tmp_tuple_element_18 = mod_consts[136];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_18);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 16, tmp_tuple_element_1);
            tmp_expression_name_19 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_19 == NULL)) {
                tmp_expression_name_19 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 49;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_18 = mod_consts[27];
            tmp_tuple_element_19 = LOOKUP_SUBSCRIPT(tmp_expression_name_19, tmp_subscript_name_18);
            if (tmp_tuple_element_19 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 49;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_19);
            tmp_tuple_element_19 = mod_consts[137];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_19);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 17, tmp_tuple_element_1);
            tmp_expression_name_20 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_20 == NULL)) {
                tmp_expression_name_20 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_20 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 50;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_19 = mod_consts[27];
            tmp_tuple_element_20 = LOOKUP_SUBSCRIPT(tmp_expression_name_20, tmp_subscript_name_19);
            if (tmp_tuple_element_20 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 50;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_20);
            tmp_tuple_element_20 = mod_consts[138];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_20);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 18, tmp_tuple_element_1);
            tmp_expression_name_21 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_21 == NULL)) {
                tmp_expression_name_21 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 51;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_20 = mod_consts[27];
            tmp_tuple_element_21 = LOOKUP_SUBSCRIPT(tmp_expression_name_21, tmp_subscript_name_20);
            if (tmp_tuple_element_21 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 51;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_21);
            tmp_tuple_element_21 = mod_consts[139];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_21);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 19, tmp_tuple_element_1);
            tmp_expression_name_22 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_22 == NULL)) {
                tmp_expression_name_22 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_22 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 52;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_21 = mod_consts[27];
            tmp_tuple_element_22 = LOOKUP_SUBSCRIPT(tmp_expression_name_22, tmp_subscript_name_21);
            if (tmp_tuple_element_22 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 52;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_22);
            tmp_tuple_element_22 = mod_consts[140];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_22);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 20, tmp_tuple_element_1);
            tmp_expression_name_23 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_23 == NULL)) {
                tmp_expression_name_23 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_23 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 53;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_22 = mod_consts[27];
            tmp_tuple_element_23 = LOOKUP_SUBSCRIPT(tmp_expression_name_23, tmp_subscript_name_22);
            if (tmp_tuple_element_23 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 53;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_23);
            tmp_tuple_element_23 = mod_consts[141];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_23);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 21, tmp_tuple_element_1);
            tmp_expression_name_24 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_24 == NULL)) {
                tmp_expression_name_24 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_24 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 54;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_23 = mod_consts[27];
            tmp_tuple_element_24 = LOOKUP_SUBSCRIPT(tmp_expression_name_24, tmp_subscript_name_23);
            if (tmp_tuple_element_24 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 54;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_24);
            tmp_tuple_element_24 = mod_consts[142];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_24);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 22, tmp_tuple_element_1);
            tmp_expression_name_25 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_25 == NULL)) {
                tmp_expression_name_25 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_25 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 55;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_24 = mod_consts[27];
            tmp_tuple_element_25 = LOOKUP_SUBSCRIPT(tmp_expression_name_25, tmp_subscript_name_24);
            if (tmp_tuple_element_25 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 55;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_25);
            tmp_tuple_element_25 = mod_consts[143];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_25);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 23, tmp_tuple_element_1);
            tmp_expression_name_26 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_26 == NULL)) {
                tmp_expression_name_26 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_26 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 56;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_25 = mod_consts[27];
            tmp_tuple_element_26 = LOOKUP_SUBSCRIPT(tmp_expression_name_26, tmp_subscript_name_25);
            if (tmp_tuple_element_26 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 56;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_26);
            tmp_tuple_element_26 = mod_consts[144];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_26);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 24, tmp_tuple_element_1);
            tmp_expression_name_27 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_27 == NULL)) {
                tmp_expression_name_27 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_27 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 57;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_26 = mod_consts[27];
            tmp_tuple_element_27 = LOOKUP_SUBSCRIPT(tmp_expression_name_27, tmp_subscript_name_26);
            if (tmp_tuple_element_27 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 57;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_27);
            tmp_tuple_element_27 = mod_consts[145];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_27);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 25, tmp_tuple_element_1);
            tmp_expression_name_28 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_28 == NULL)) {
                tmp_expression_name_28 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_28 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 58;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_27 = mod_consts[27];
            tmp_tuple_element_28 = LOOKUP_SUBSCRIPT(tmp_expression_name_28, tmp_subscript_name_27);
            if (tmp_tuple_element_28 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 58;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_28);
            tmp_tuple_element_28 = mod_consts[146];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_28);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 26, tmp_tuple_element_1);
            tmp_expression_name_29 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_29 == NULL)) {
                tmp_expression_name_29 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_29 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 59;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_28 = mod_consts[27];
            tmp_tuple_element_29 = LOOKUP_SUBSCRIPT(tmp_expression_name_29, tmp_subscript_name_28);
            if (tmp_tuple_element_29 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 59;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_29);
            tmp_tuple_element_29 = mod_consts[147];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_29);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 27, tmp_tuple_element_1);
            tmp_expression_name_30 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_30 == NULL)) {
                tmp_expression_name_30 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_30 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 60;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_29 = mod_consts[27];
            tmp_tuple_element_30 = LOOKUP_SUBSCRIPT(tmp_expression_name_30, tmp_subscript_name_29);
            if (tmp_tuple_element_30 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 60;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_30);
            tmp_tuple_element_30 = mod_consts[148];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_30);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 28, tmp_tuple_element_1);
            tmp_expression_name_31 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_31 == NULL)) {
                tmp_expression_name_31 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_31 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 61;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_30 = mod_consts[27];
            tmp_tuple_element_31 = LOOKUP_SUBSCRIPT(tmp_expression_name_31, tmp_subscript_name_30);
            if (tmp_tuple_element_31 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 61;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_31);
            tmp_tuple_element_31 = mod_consts[149];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_31);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 29, tmp_tuple_element_1);
            tmp_expression_name_32 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_32 == NULL)) {
                tmp_expression_name_32 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_32 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 62;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_31 = mod_consts[27];
            tmp_tuple_element_32 = LOOKUP_SUBSCRIPT(tmp_expression_name_32, tmp_subscript_name_31);
            if (tmp_tuple_element_32 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 62;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_32);
            tmp_tuple_element_32 = mod_consts[150];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_32);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 30, tmp_tuple_element_1);
            tmp_expression_name_33 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_33 == NULL)) {
                tmp_expression_name_33 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 63;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_32 = mod_consts[27];
            tmp_tuple_element_33 = LOOKUP_SUBSCRIPT(tmp_expression_name_33, tmp_subscript_name_32);
            if (tmp_tuple_element_33 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 63;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_33);
            tmp_tuple_element_33 = mod_consts[151];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_33);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 31, tmp_tuple_element_1);
            tmp_expression_name_34 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_34 == NULL)) {
                tmp_expression_name_34 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_34 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 64;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_33 = mod_consts[27];
            tmp_tuple_element_34 = LOOKUP_SUBSCRIPT(tmp_expression_name_34, tmp_subscript_name_33);
            if (tmp_tuple_element_34 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 64;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_34);
            tmp_tuple_element_34 = mod_consts[152];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_34);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 32, tmp_tuple_element_1);
            tmp_expression_name_35 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_35 == NULL)) {
                tmp_expression_name_35 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_35 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 65;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_34 = mod_consts[27];
            tmp_tuple_element_35 = LOOKUP_SUBSCRIPT(tmp_expression_name_35, tmp_subscript_name_34);
            if (tmp_tuple_element_35 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 65;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_35);
            tmp_tuple_element_35 = mod_consts[153];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_35);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 33, tmp_tuple_element_1);
            tmp_expression_name_36 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_36 == NULL)) {
                tmp_expression_name_36 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_36 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 66;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_35 = mod_consts[27];
            tmp_tuple_element_36 = LOOKUP_SUBSCRIPT(tmp_expression_name_36, tmp_subscript_name_35);
            if (tmp_tuple_element_36 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 66;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_36);
            tmp_tuple_element_36 = mod_consts[154];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_36);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 34, tmp_tuple_element_1);
            tmp_expression_name_37 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_37 == NULL)) {
                tmp_expression_name_37 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 67;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_36 = mod_consts[27];
            tmp_tuple_element_37 = LOOKUP_SUBSCRIPT(tmp_expression_name_37, tmp_subscript_name_36);
            if (tmp_tuple_element_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 67;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_37);
            tmp_tuple_element_37 = mod_consts[155];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_37);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 35, tmp_tuple_element_1);
            tmp_expression_name_38 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_38 == NULL)) {
                tmp_expression_name_38 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_38 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 68;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_37 = mod_consts[27];
            tmp_tuple_element_38 = LOOKUP_SUBSCRIPT(tmp_expression_name_38, tmp_subscript_name_37);
            if (tmp_tuple_element_38 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 68;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_38);
            tmp_tuple_element_38 = mod_consts[156];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_38);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 36, tmp_tuple_element_1);
            tmp_expression_name_39 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_39 == NULL)) {
                tmp_expression_name_39 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_39 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 69;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_38 = mod_consts[27];
            tmp_tuple_element_39 = LOOKUP_SUBSCRIPT(tmp_expression_name_39, tmp_subscript_name_38);
            if (tmp_tuple_element_39 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 69;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_39);
            tmp_tuple_element_39 = mod_consts[157];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_39);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 37, tmp_tuple_element_1);
            tmp_expression_name_40 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_40 == NULL)) {
                tmp_expression_name_40 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_40 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 70;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_39 = mod_consts[27];
            tmp_tuple_element_40 = LOOKUP_SUBSCRIPT(tmp_expression_name_40, tmp_subscript_name_39);
            if (tmp_tuple_element_40 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 70;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_40);
            tmp_tuple_element_40 = mod_consts[158];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_40);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 38, tmp_tuple_element_1);
            tmp_expression_name_41 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_41 == NULL)) {
                tmp_expression_name_41 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_41 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 71;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_40 = mod_consts[27];
            tmp_tuple_element_41 = LOOKUP_SUBSCRIPT(tmp_expression_name_41, tmp_subscript_name_40);
            if (tmp_tuple_element_41 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 71;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_41);
            tmp_tuple_element_41 = mod_consts[159];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_41);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 39, tmp_tuple_element_1);
            tmp_expression_name_42 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_42 == NULL)) {
                tmp_expression_name_42 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_42 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 72;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_41 = mod_consts[27];
            tmp_tuple_element_42 = LOOKUP_SUBSCRIPT(tmp_expression_name_42, tmp_subscript_name_41);
            if (tmp_tuple_element_42 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 72;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_42);
            tmp_tuple_element_42 = mod_consts[160];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_42);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 40, tmp_tuple_element_1);
            tmp_expression_name_43 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_43 == NULL)) {
                tmp_expression_name_43 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_43 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 73;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_42 = mod_consts[27];
            tmp_tuple_element_43 = LOOKUP_SUBSCRIPT(tmp_expression_name_43, tmp_subscript_name_42);
            if (tmp_tuple_element_43 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 73;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_43);
            tmp_tuple_element_43 = mod_consts[161];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_43);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 41, tmp_tuple_element_1);
            tmp_expression_name_44 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_44 == NULL)) {
                tmp_expression_name_44 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_44 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 74;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_43 = mod_consts[27];
            tmp_tuple_element_44 = LOOKUP_SUBSCRIPT(tmp_expression_name_44, tmp_subscript_name_43);
            if (tmp_tuple_element_44 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 74;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_44);
            tmp_tuple_element_44 = mod_consts[162];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_44);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 42, tmp_tuple_element_1);
            tmp_expression_name_45 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_45 == NULL)) {
                tmp_expression_name_45 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_45 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 75;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_44 = mod_consts[27];
            tmp_tuple_element_45 = LOOKUP_SUBSCRIPT(tmp_expression_name_45, tmp_subscript_name_44);
            if (tmp_tuple_element_45 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 75;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_45);
            tmp_tuple_element_45 = mod_consts[163];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_45);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 43, tmp_tuple_element_1);
            tmp_expression_name_46 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_46 == NULL)) {
                tmp_expression_name_46 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_46 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 76;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_45 = mod_consts[27];
            tmp_tuple_element_46 = LOOKUP_SUBSCRIPT(tmp_expression_name_46, tmp_subscript_name_45);
            if (tmp_tuple_element_46 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 76;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_46);
            tmp_tuple_element_46 = mod_consts[164];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_46);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 44, tmp_tuple_element_1);
            tmp_expression_name_47 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_47 == NULL)) {
                tmp_expression_name_47 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 77;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_46 = mod_consts[27];
            tmp_tuple_element_47 = LOOKUP_SUBSCRIPT(tmp_expression_name_47, tmp_subscript_name_46);
            if (tmp_tuple_element_47 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 77;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_47);
            tmp_tuple_element_47 = mod_consts[165];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_47);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 45, tmp_tuple_element_1);
            tmp_expression_name_48 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_48 == NULL)) {
                tmp_expression_name_48 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_48 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 78;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_47 = mod_consts[27];
            tmp_tuple_element_48 = LOOKUP_SUBSCRIPT(tmp_expression_name_48, tmp_subscript_name_47);
            if (tmp_tuple_element_48 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 78;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_48);
            tmp_tuple_element_48 = mod_consts[166];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_48);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 46, tmp_tuple_element_1);
            tmp_expression_name_49 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_49 == NULL)) {
                tmp_expression_name_49 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 79;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_48 = mod_consts[27];
            tmp_tuple_element_49 = LOOKUP_SUBSCRIPT(tmp_expression_name_49, tmp_subscript_name_48);
            if (tmp_tuple_element_49 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 79;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_49);
            tmp_tuple_element_49 = mod_consts[167];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_49);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 47, tmp_tuple_element_1);
            tmp_expression_name_50 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_50 == NULL)) {
                tmp_expression_name_50 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_50 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 80;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_49 = mod_consts[27];
            tmp_tuple_element_50 = LOOKUP_SUBSCRIPT(tmp_expression_name_50, tmp_subscript_name_49);
            if (tmp_tuple_element_50 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 80;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_50);
            tmp_tuple_element_50 = mod_consts[168];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_50);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 48, tmp_tuple_element_1);
            tmp_expression_name_51 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_51 == NULL)) {
                tmp_expression_name_51 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_51 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 81;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_50 = mod_consts[27];
            tmp_tuple_element_51 = LOOKUP_SUBSCRIPT(tmp_expression_name_51, tmp_subscript_name_50);
            if (tmp_tuple_element_51 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 81;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_51);
            tmp_tuple_element_51 = mod_consts[169];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_51);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 49, tmp_tuple_element_1);
            tmp_expression_name_52 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_52 == NULL)) {
                tmp_expression_name_52 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_52 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 82;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_51 = mod_consts[27];
            tmp_tuple_element_52 = LOOKUP_SUBSCRIPT(tmp_expression_name_52, tmp_subscript_name_51);
            if (tmp_tuple_element_52 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 82;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_52);
            tmp_tuple_element_52 = mod_consts[170];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_52);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 50, tmp_tuple_element_1);
            tmp_expression_name_53 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_53 == NULL)) {
                tmp_expression_name_53 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_53 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 83;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_52 = mod_consts[27];
            tmp_tuple_element_53 = LOOKUP_SUBSCRIPT(tmp_expression_name_53, tmp_subscript_name_52);
            if (tmp_tuple_element_53 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 83;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_53);
            tmp_tuple_element_53 = mod_consts[171];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_53);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 51, tmp_tuple_element_1);
            tmp_expression_name_54 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_54 == NULL)) {
                tmp_expression_name_54 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_54 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 84;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_53 = mod_consts[27];
            tmp_tuple_element_54 = LOOKUP_SUBSCRIPT(tmp_expression_name_54, tmp_subscript_name_53);
            if (tmp_tuple_element_54 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 84;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_54);
            tmp_tuple_element_54 = mod_consts[172];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_54);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 52, tmp_tuple_element_1);
            tmp_expression_name_55 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_55 == NULL)) {
                tmp_expression_name_55 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_55 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 85;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_54 = mod_consts[27];
            tmp_tuple_element_55 = LOOKUP_SUBSCRIPT(tmp_expression_name_55, tmp_subscript_name_54);
            if (tmp_tuple_element_55 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 85;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_55);
            tmp_tuple_element_55 = mod_consts[173];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_55);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 53, tmp_tuple_element_1);
            tmp_expression_name_56 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_56 == NULL)) {
                tmp_expression_name_56 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_56 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 86;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_55 = mod_consts[27];
            tmp_tuple_element_56 = LOOKUP_SUBSCRIPT(tmp_expression_name_56, tmp_subscript_name_55);
            if (tmp_tuple_element_56 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 86;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_56);
            tmp_tuple_element_56 = mod_consts[174];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_56);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 54, tmp_tuple_element_1);
            tmp_expression_name_57 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_57 == NULL)) {
                tmp_expression_name_57 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_57 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 87;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_56 = mod_consts[27];
            tmp_tuple_element_57 = LOOKUP_SUBSCRIPT(tmp_expression_name_57, tmp_subscript_name_56);
            if (tmp_tuple_element_57 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 87;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_57);
            tmp_tuple_element_57 = mod_consts[175];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_57);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 55, tmp_tuple_element_1);
            tmp_expression_name_58 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_58 == NULL)) {
                tmp_expression_name_58 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_58 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 88;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_57 = mod_consts[27];
            tmp_tuple_element_58 = LOOKUP_SUBSCRIPT(tmp_expression_name_58, tmp_subscript_name_57);
            if (tmp_tuple_element_58 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 88;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_58);
            tmp_tuple_element_58 = mod_consts[176];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_58);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 56, tmp_tuple_element_1);
            tmp_expression_name_59 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_59 == NULL)) {
                tmp_expression_name_59 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_59 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 89;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_58 = mod_consts[27];
            tmp_tuple_element_59 = LOOKUP_SUBSCRIPT(tmp_expression_name_59, tmp_subscript_name_58);
            if (tmp_tuple_element_59 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 89;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_59);
            tmp_tuple_element_59 = mod_consts[177];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_59);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 57, tmp_tuple_element_1);
            tmp_expression_name_60 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_60 == NULL)) {
                tmp_expression_name_60 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_60 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 90;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_59 = mod_consts[27];
            tmp_tuple_element_60 = LOOKUP_SUBSCRIPT(tmp_expression_name_60, tmp_subscript_name_59);
            if (tmp_tuple_element_60 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 90;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_60);
            tmp_tuple_element_60 = mod_consts[178];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_60);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 58, tmp_tuple_element_1);
            tmp_expression_name_61 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_61 == NULL)) {
                tmp_expression_name_61 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_61 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 91;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_60 = mod_consts[27];
            tmp_tuple_element_61 = LOOKUP_SUBSCRIPT(tmp_expression_name_61, tmp_subscript_name_60);
            if (tmp_tuple_element_61 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 91;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_61);
            tmp_tuple_element_61 = mod_consts[179];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_61);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 59, tmp_tuple_element_1);
            tmp_expression_name_62 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_62 == NULL)) {
                tmp_expression_name_62 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_62 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 92;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_61 = mod_consts[27];
            tmp_tuple_element_62 = LOOKUP_SUBSCRIPT(tmp_expression_name_62, tmp_subscript_name_61);
            if (tmp_tuple_element_62 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 92;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_62);
            tmp_tuple_element_62 = mod_consts[180];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_62);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 60, tmp_tuple_element_1);
            tmp_expression_name_63 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_63 == NULL)) {
                tmp_expression_name_63 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_63 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 93;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_62 = mod_consts[27];
            tmp_tuple_element_63 = LOOKUP_SUBSCRIPT(tmp_expression_name_63, tmp_subscript_name_62);
            if (tmp_tuple_element_63 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 93;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_63);
            tmp_tuple_element_63 = mod_consts[181];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_63);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 61, tmp_tuple_element_1);
            tmp_expression_name_64 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_64 == NULL)) {
                tmp_expression_name_64 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_64 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 94;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_63 = mod_consts[27];
            tmp_tuple_element_64 = LOOKUP_SUBSCRIPT(tmp_expression_name_64, tmp_subscript_name_63);
            if (tmp_tuple_element_64 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 94;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_64);
            tmp_tuple_element_64 = mod_consts[182];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_64);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 62, tmp_tuple_element_1);
            tmp_expression_name_65 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_65 == NULL)) {
                tmp_expression_name_65 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_65 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 95;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_64 = mod_consts[27];
            tmp_tuple_element_65 = LOOKUP_SUBSCRIPT(tmp_expression_name_65, tmp_subscript_name_64);
            if (tmp_tuple_element_65 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 95;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_65);
            tmp_tuple_element_65 = mod_consts[183];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_65);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 63, tmp_tuple_element_1);
            tmp_expression_name_66 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_66 == NULL)) {
                tmp_expression_name_66 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_66 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 96;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_65 = mod_consts[27];
            tmp_tuple_element_66 = LOOKUP_SUBSCRIPT(tmp_expression_name_66, tmp_subscript_name_65);
            if (tmp_tuple_element_66 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 96;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_66);
            tmp_tuple_element_66 = mod_consts[184];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_66);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 64, tmp_tuple_element_1);
            tmp_expression_name_67 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_67 == NULL)) {
                tmp_expression_name_67 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_67 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 97;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_66 = mod_consts[27];
            tmp_tuple_element_67 = LOOKUP_SUBSCRIPT(tmp_expression_name_67, tmp_subscript_name_66);
            if (tmp_tuple_element_67 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 97;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_67);
            tmp_tuple_element_67 = mod_consts[185];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_67);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 65, tmp_tuple_element_1);
            tmp_expression_name_68 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_68 == NULL)) {
                tmp_expression_name_68 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_68 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 98;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_67 = mod_consts[27];
            tmp_tuple_element_68 = LOOKUP_SUBSCRIPT(tmp_expression_name_68, tmp_subscript_name_67);
            if (tmp_tuple_element_68 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 98;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_68);
            tmp_tuple_element_68 = mod_consts[186];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_68);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 66, tmp_tuple_element_1);
            tmp_expression_name_69 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_69 == NULL)) {
                tmp_expression_name_69 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_69 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 99;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_68 = mod_consts[27];
            tmp_tuple_element_69 = LOOKUP_SUBSCRIPT(tmp_expression_name_69, tmp_subscript_name_68);
            if (tmp_tuple_element_69 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 99;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_69);
            tmp_tuple_element_69 = mod_consts[187];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_69);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 67, tmp_tuple_element_1);
            tmp_expression_name_70 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_70 == NULL)) {
                tmp_expression_name_70 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_70 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 100;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_69 = mod_consts[27];
            tmp_tuple_element_70 = LOOKUP_SUBSCRIPT(tmp_expression_name_70, tmp_subscript_name_69);
            if (tmp_tuple_element_70 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 100;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_70);
            tmp_tuple_element_70 = mod_consts[188];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_70);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 68, tmp_tuple_element_1);
            tmp_expression_name_71 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_71 == NULL)) {
                tmp_expression_name_71 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_71 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 101;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_70 = mod_consts[27];
            tmp_tuple_element_71 = LOOKUP_SUBSCRIPT(tmp_expression_name_71, tmp_subscript_name_70);
            if (tmp_tuple_element_71 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 101;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_71);
            tmp_tuple_element_71 = mod_consts[189];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_71);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 69, tmp_tuple_element_1);
            tmp_expression_name_72 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_72 == NULL)) {
                tmp_expression_name_72 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_72 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 102;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_71 = mod_consts[27];
            tmp_tuple_element_72 = LOOKUP_SUBSCRIPT(tmp_expression_name_72, tmp_subscript_name_71);
            if (tmp_tuple_element_72 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 102;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_72);
            tmp_tuple_element_72 = mod_consts[190];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_72);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 70, tmp_tuple_element_1);
            tmp_expression_name_73 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_73 == NULL)) {
                tmp_expression_name_73 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_73 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 103;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_72 = mod_consts[27];
            tmp_tuple_element_73 = LOOKUP_SUBSCRIPT(tmp_expression_name_73, tmp_subscript_name_72);
            if (tmp_tuple_element_73 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 103;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_73);
            tmp_tuple_element_73 = mod_consts[191];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_73);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 71, tmp_tuple_element_1);
            tmp_expression_name_74 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_74 == NULL)) {
                tmp_expression_name_74 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_74 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 104;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_73 = mod_consts[27];
            tmp_tuple_element_74 = LOOKUP_SUBSCRIPT(tmp_expression_name_74, tmp_subscript_name_73);
            if (tmp_tuple_element_74 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 104;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_74);
            tmp_tuple_element_74 = mod_consts[192];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_74);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 72, tmp_tuple_element_1);
            tmp_expression_name_75 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_75 == NULL)) {
                tmp_expression_name_75 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_75 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 105;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_74 = mod_consts[27];
            tmp_tuple_element_75 = LOOKUP_SUBSCRIPT(tmp_expression_name_75, tmp_subscript_name_74);
            if (tmp_tuple_element_75 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 105;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_75);
            tmp_tuple_element_75 = mod_consts[193];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_75);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 73, tmp_tuple_element_1);
            tmp_expression_name_76 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_76 == NULL)) {
                tmp_expression_name_76 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_76 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 106;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_75 = mod_consts[27];
            tmp_tuple_element_76 = LOOKUP_SUBSCRIPT(tmp_expression_name_76, tmp_subscript_name_75);
            if (tmp_tuple_element_76 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 106;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_76);
            tmp_tuple_element_76 = mod_consts[194];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_76);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 74, tmp_tuple_element_1);
            tmp_expression_name_77 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_77 == NULL)) {
                tmp_expression_name_77 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_77 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 107;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_76 = mod_consts[27];
            tmp_tuple_element_77 = LOOKUP_SUBSCRIPT(tmp_expression_name_77, tmp_subscript_name_76);
            if (tmp_tuple_element_77 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 107;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_77);
            tmp_tuple_element_77 = mod_consts[195];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_77);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 75, tmp_tuple_element_1);
            tmp_expression_name_78 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_78 == NULL)) {
                tmp_expression_name_78 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_78 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 108;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_77 = mod_consts[27];
            tmp_tuple_element_78 = LOOKUP_SUBSCRIPT(tmp_expression_name_78, tmp_subscript_name_77);
            if (tmp_tuple_element_78 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 108;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_78);
            tmp_tuple_element_78 = mod_consts[196];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_78);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 76, tmp_tuple_element_1);
            tmp_expression_name_79 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_79 == NULL)) {
                tmp_expression_name_79 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_79 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 109;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_78 = mod_consts[27];
            tmp_tuple_element_79 = LOOKUP_SUBSCRIPT(tmp_expression_name_79, tmp_subscript_name_78);
            if (tmp_tuple_element_79 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 109;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_79);
            tmp_tuple_element_79 = mod_consts[197];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_79);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 77, tmp_tuple_element_1);
            tmp_expression_name_80 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_80 == NULL)) {
                tmp_expression_name_80 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_80 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 110;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_79 = mod_consts[27];
            tmp_tuple_element_80 = LOOKUP_SUBSCRIPT(tmp_expression_name_80, tmp_subscript_name_79);
            if (tmp_tuple_element_80 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 110;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_80);
            tmp_tuple_element_80 = mod_consts[198];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_80);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 78, tmp_tuple_element_1);
            tmp_expression_name_81 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_81 == NULL)) {
                tmp_expression_name_81 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_81 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 111;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_80 = mod_consts[27];
            tmp_tuple_element_81 = LOOKUP_SUBSCRIPT(tmp_expression_name_81, tmp_subscript_name_80);
            if (tmp_tuple_element_81 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 111;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_81);
            tmp_tuple_element_81 = mod_consts[199];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_81);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 79, tmp_tuple_element_1);
            tmp_expression_name_82 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_82 == NULL)) {
                tmp_expression_name_82 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_82 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 112;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_81 = mod_consts[27];
            tmp_tuple_element_82 = LOOKUP_SUBSCRIPT(tmp_expression_name_82, tmp_subscript_name_81);
            if (tmp_tuple_element_82 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 112;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_82);
            tmp_tuple_element_82 = mod_consts[200];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_82);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 80, tmp_tuple_element_1);
            tmp_expression_name_83 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_83 == NULL)) {
                tmp_expression_name_83 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_83 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 113;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_82 = mod_consts[27];
            tmp_tuple_element_83 = LOOKUP_SUBSCRIPT(tmp_expression_name_83, tmp_subscript_name_82);
            if (tmp_tuple_element_83 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 113;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_83);
            tmp_tuple_element_83 = mod_consts[201];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_83);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 81, tmp_tuple_element_1);
            tmp_expression_name_84 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_84 == NULL)) {
                tmp_expression_name_84 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_84 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 114;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_83 = mod_consts[27];
            tmp_tuple_element_84 = LOOKUP_SUBSCRIPT(tmp_expression_name_84, tmp_subscript_name_83);
            if (tmp_tuple_element_84 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 114;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_84);
            tmp_tuple_element_84 = mod_consts[202];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_84);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 82, tmp_tuple_element_1);
            tmp_expression_name_85 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_85 == NULL)) {
                tmp_expression_name_85 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_85 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 115;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_84 = mod_consts[27];
            tmp_tuple_element_85 = LOOKUP_SUBSCRIPT(tmp_expression_name_85, tmp_subscript_name_84);
            if (tmp_tuple_element_85 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 115;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_85);
            tmp_tuple_element_85 = mod_consts[38];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_85);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 83, tmp_tuple_element_1);
            tmp_expression_name_86 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_86 == NULL)) {
                tmp_expression_name_86 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_86 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 116;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_85 = mod_consts[27];
            tmp_tuple_element_86 = LOOKUP_SUBSCRIPT(tmp_expression_name_86, tmp_subscript_name_85);
            if (tmp_tuple_element_86 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 116;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_86);
            tmp_tuple_element_86 = mod_consts[203];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_86);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 84, tmp_tuple_element_1);
            tmp_expression_name_87 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_87 == NULL)) {
                tmp_expression_name_87 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_87 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 117;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_86 = mod_consts[27];
            tmp_tuple_element_87 = LOOKUP_SUBSCRIPT(tmp_expression_name_87, tmp_subscript_name_86);
            if (tmp_tuple_element_87 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 117;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_87);
            tmp_tuple_element_87 = mod_consts[204];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_87);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 85, tmp_tuple_element_1);
            tmp_expression_name_88 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_88 == NULL)) {
                tmp_expression_name_88 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_88 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 118;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_87 = mod_consts[27];
            tmp_tuple_element_88 = LOOKUP_SUBSCRIPT(tmp_expression_name_88, tmp_subscript_name_87);
            if (tmp_tuple_element_88 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 118;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_88);
            tmp_tuple_element_88 = mod_consts[205];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_88);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 86, tmp_tuple_element_1);
            tmp_expression_name_89 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_89 == NULL)) {
                tmp_expression_name_89 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_89 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 119;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_88 = mod_consts[27];
            tmp_tuple_element_89 = LOOKUP_SUBSCRIPT(tmp_expression_name_89, tmp_subscript_name_88);
            if (tmp_tuple_element_89 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 119;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_89);
            tmp_tuple_element_89 = mod_consts[206];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_89);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 87, tmp_tuple_element_1);
            tmp_expression_name_90 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_90 == NULL)) {
                tmp_expression_name_90 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_90 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 120;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_89 = mod_consts[27];
            tmp_tuple_element_90 = LOOKUP_SUBSCRIPT(tmp_expression_name_90, tmp_subscript_name_89);
            if (tmp_tuple_element_90 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 120;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_90);
            tmp_tuple_element_90 = mod_consts[207];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_90);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 88, tmp_tuple_element_1);
            tmp_expression_name_91 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_91 == NULL)) {
                tmp_expression_name_91 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_91 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 121;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_90 = mod_consts[27];
            tmp_tuple_element_91 = LOOKUP_SUBSCRIPT(tmp_expression_name_91, tmp_subscript_name_90);
            if (tmp_tuple_element_91 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 121;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_91);
            tmp_tuple_element_91 = mod_consts[208];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_91);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 89, tmp_tuple_element_1);
            tmp_expression_name_92 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_92 == NULL)) {
                tmp_expression_name_92 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_92 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 122;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_91 = mod_consts[27];
            tmp_tuple_element_92 = LOOKUP_SUBSCRIPT(tmp_expression_name_92, tmp_subscript_name_91);
            if (tmp_tuple_element_92 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 122;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_92);
            tmp_tuple_element_92 = mod_consts[209];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_92);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 90, tmp_tuple_element_1);
            tmp_expression_name_93 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_93 == NULL)) {
                tmp_expression_name_93 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_93 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 123;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_92 = mod_consts[27];
            tmp_tuple_element_93 = LOOKUP_SUBSCRIPT(tmp_expression_name_93, tmp_subscript_name_92);
            if (tmp_tuple_element_93 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 123;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_93);
            tmp_tuple_element_93 = mod_consts[210];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_93);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 91, tmp_tuple_element_1);
            tmp_expression_name_94 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_94 == NULL)) {
                tmp_expression_name_94 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_94 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 124;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_93 = mod_consts[27];
            tmp_tuple_element_94 = LOOKUP_SUBSCRIPT(tmp_expression_name_94, tmp_subscript_name_93);
            if (tmp_tuple_element_94 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 124;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_94);
            tmp_tuple_element_94 = mod_consts[211];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_94);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 92, tmp_tuple_element_1);
            tmp_expression_name_95 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_95 == NULL)) {
                tmp_expression_name_95 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_95 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 125;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_94 = mod_consts[27];
            tmp_tuple_element_95 = LOOKUP_SUBSCRIPT(tmp_expression_name_95, tmp_subscript_name_94);
            if (tmp_tuple_element_95 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 125;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_95);
            tmp_tuple_element_95 = mod_consts[212];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_95);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 93, tmp_tuple_element_1);
            tmp_expression_name_96 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_96 == NULL)) {
                tmp_expression_name_96 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_96 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 126;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_95 = mod_consts[27];
            tmp_tuple_element_96 = LOOKUP_SUBSCRIPT(tmp_expression_name_96, tmp_subscript_name_95);
            if (tmp_tuple_element_96 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 126;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_96);
            tmp_tuple_element_96 = mod_consts[213];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_96);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 94, tmp_tuple_element_1);
            tmp_expression_name_97 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_97 == NULL)) {
                tmp_expression_name_97 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_97 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 127;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_96 = mod_consts[27];
            tmp_tuple_element_97 = LOOKUP_SUBSCRIPT(tmp_expression_name_97, tmp_subscript_name_96);
            if (tmp_tuple_element_97 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 127;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_97);
            tmp_tuple_element_97 = mod_consts[214];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_97);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 95, tmp_tuple_element_1);
            tmp_expression_name_98 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_98 == NULL)) {
                tmp_expression_name_98 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_98 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 128;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_97 = mod_consts[27];
            tmp_tuple_element_98 = LOOKUP_SUBSCRIPT(tmp_expression_name_98, tmp_subscript_name_97);
            if (tmp_tuple_element_98 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 128;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_98);
            tmp_tuple_element_98 = mod_consts[215];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_98);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 96, tmp_tuple_element_1);
            tmp_expression_name_99 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_99 == NULL)) {
                tmp_expression_name_99 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_99 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 129;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_98 = mod_consts[27];
            tmp_tuple_element_99 = LOOKUP_SUBSCRIPT(tmp_expression_name_99, tmp_subscript_name_98);
            if (tmp_tuple_element_99 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 129;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_99);
            tmp_tuple_element_99 = mod_consts[216];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_99);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 97, tmp_tuple_element_1);
            tmp_expression_name_100 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_100 == NULL)) {
                tmp_expression_name_100 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_100 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 130;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_99 = mod_consts[27];
            tmp_tuple_element_100 = LOOKUP_SUBSCRIPT(tmp_expression_name_100, tmp_subscript_name_99);
            if (tmp_tuple_element_100 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 130;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_100);
            tmp_tuple_element_100 = mod_consts[217];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_100);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 98, tmp_tuple_element_1);
            tmp_expression_name_101 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_101 == NULL)) {
                tmp_expression_name_101 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_101 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 131;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_100 = mod_consts[218];
            tmp_tuple_element_101 = LOOKUP_SUBSCRIPT(tmp_expression_name_101, tmp_subscript_name_100);
            if (tmp_tuple_element_101 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 131;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_101);
            tmp_tuple_element_101 = mod_consts[219];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_101);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 99, tmp_tuple_element_1);
            tmp_expression_name_102 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_102 == NULL)) {
                tmp_expression_name_102 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_102 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 132;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_101 = mod_consts[218];
            tmp_tuple_element_102 = LOOKUP_SUBSCRIPT(tmp_expression_name_102, tmp_subscript_name_101);
            if (tmp_tuple_element_102 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 132;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_102);
            tmp_tuple_element_102 = mod_consts[220];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_102);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 100, tmp_tuple_element_1);
            tmp_expression_name_103 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_103 == NULL)) {
                tmp_expression_name_103 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_103 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 133;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_102 = mod_consts[218];
            tmp_tuple_element_103 = LOOKUP_SUBSCRIPT(tmp_expression_name_103, tmp_subscript_name_102);
            if (tmp_tuple_element_103 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 133;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_103);
            tmp_tuple_element_103 = mod_consts[221];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_103);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 101, tmp_tuple_element_1);
            tmp_expression_name_104 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_104 == NULL)) {
                tmp_expression_name_104 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_104 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 134;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_103 = mod_consts[218];
            tmp_tuple_element_104 = LOOKUP_SUBSCRIPT(tmp_expression_name_104, tmp_subscript_name_103);
            if (tmp_tuple_element_104 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 134;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_104);
            tmp_tuple_element_104 = mod_consts[222];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_104);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 102, tmp_tuple_element_1);
            tmp_expression_name_105 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_105 == NULL)) {
                tmp_expression_name_105 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_105 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 135;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_104 = mod_consts[218];
            tmp_tuple_element_105 = LOOKUP_SUBSCRIPT(tmp_expression_name_105, tmp_subscript_name_104);
            if (tmp_tuple_element_105 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 135;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_105);
            tmp_tuple_element_105 = mod_consts[223];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_105);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 103, tmp_tuple_element_1);
            tmp_expression_name_106 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_106 == NULL)) {
                tmp_expression_name_106 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_106 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 136;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_105 = mod_consts[218];
            tmp_tuple_element_106 = LOOKUP_SUBSCRIPT(tmp_expression_name_106, tmp_subscript_name_105);
            if (tmp_tuple_element_106 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 136;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_106);
            tmp_tuple_element_106 = mod_consts[224];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_106);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 104, tmp_tuple_element_1);
            tmp_expression_name_107 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_107 == NULL)) {
                tmp_expression_name_107 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_107 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 137;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_106 = mod_consts[218];
            tmp_tuple_element_107 = LOOKUP_SUBSCRIPT(tmp_expression_name_107, tmp_subscript_name_106);
            if (tmp_tuple_element_107 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 137;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_107);
            tmp_tuple_element_107 = mod_consts[225];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_107);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 105, tmp_tuple_element_1);
            tmp_expression_name_108 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_108 == NULL)) {
                tmp_expression_name_108 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_108 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 138;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_107 = mod_consts[218];
            tmp_tuple_element_108 = LOOKUP_SUBSCRIPT(tmp_expression_name_108, tmp_subscript_name_107);
            if (tmp_tuple_element_108 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 138;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_108);
            tmp_tuple_element_108 = mod_consts[226];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_108);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 106, tmp_tuple_element_1);
            tmp_expression_name_109 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_109 == NULL)) {
                tmp_expression_name_109 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_109 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 139;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_108 = mod_consts[218];
            tmp_tuple_element_109 = LOOKUP_SUBSCRIPT(tmp_expression_name_109, tmp_subscript_name_108);
            if (tmp_tuple_element_109 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 139;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_109);
            tmp_tuple_element_109 = mod_consts[227];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_109);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 107, tmp_tuple_element_1);
            tmp_expression_name_110 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_110 == NULL)) {
                tmp_expression_name_110 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_110 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_109 = mod_consts[218];
            tmp_tuple_element_110 = LOOKUP_SUBSCRIPT(tmp_expression_name_110, tmp_subscript_name_109);
            if (tmp_tuple_element_110 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 140;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_110);
            tmp_tuple_element_110 = mod_consts[228];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_110);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 108, tmp_tuple_element_1);
            tmp_expression_name_111 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_111 == NULL)) {
                tmp_expression_name_111 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_111 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 141;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_110 = mod_consts[218];
            tmp_tuple_element_111 = LOOKUP_SUBSCRIPT(tmp_expression_name_111, tmp_subscript_name_110);
            if (tmp_tuple_element_111 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 141;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_111);
            tmp_tuple_element_111 = mod_consts[229];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_111);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 109, tmp_tuple_element_1);
            tmp_expression_name_112 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_112 == NULL)) {
                tmp_expression_name_112 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_112 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 142;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_111 = mod_consts[218];
            tmp_tuple_element_112 = LOOKUP_SUBSCRIPT(tmp_expression_name_112, tmp_subscript_name_111);
            if (tmp_tuple_element_112 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 142;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_112);
            tmp_tuple_element_112 = mod_consts[230];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_112);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 110, tmp_tuple_element_1);
            tmp_expression_name_113 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_113 == NULL)) {
                tmp_expression_name_113 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_113 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 143;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_112 = mod_consts[218];
            tmp_tuple_element_113 = LOOKUP_SUBSCRIPT(tmp_expression_name_113, tmp_subscript_name_112);
            if (tmp_tuple_element_113 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 143;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_113);
            tmp_tuple_element_113 = mod_consts[231];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_113);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 111, tmp_tuple_element_1);
            tmp_expression_name_114 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_114 == NULL)) {
                tmp_expression_name_114 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_114 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 144;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_113 = mod_consts[218];
            tmp_tuple_element_114 = LOOKUP_SUBSCRIPT(tmp_expression_name_114, tmp_subscript_name_113);
            if (tmp_tuple_element_114 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 144;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_114);
            tmp_tuple_element_114 = mod_consts[232];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_114);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 112, tmp_tuple_element_1);
            tmp_expression_name_115 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_115 == NULL)) {
                tmp_expression_name_115 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_115 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 145;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_114 = mod_consts[218];
            tmp_tuple_element_115 = LOOKUP_SUBSCRIPT(tmp_expression_name_115, tmp_subscript_name_114);
            if (tmp_tuple_element_115 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 145;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_115);
            tmp_tuple_element_115 = mod_consts[233];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_115);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 113, tmp_tuple_element_1);
            tmp_expression_name_116 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_116 == NULL)) {
                tmp_expression_name_116 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_116 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 146;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_115 = mod_consts[218];
            tmp_tuple_element_116 = LOOKUP_SUBSCRIPT(tmp_expression_name_116, tmp_subscript_name_115);
            if (tmp_tuple_element_116 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 146;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_116);
            tmp_tuple_element_116 = mod_consts[234];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_116);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 114, tmp_tuple_element_1);
            tmp_expression_name_117 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_117 == NULL)) {
                tmp_expression_name_117 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_117 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 147;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_116 = mod_consts[218];
            tmp_tuple_element_117 = LOOKUP_SUBSCRIPT(tmp_expression_name_117, tmp_subscript_name_116);
            if (tmp_tuple_element_117 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 147;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_117);
            tmp_tuple_element_117 = mod_consts[235];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_117);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 115, tmp_tuple_element_1);
            tmp_expression_name_118 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_118 == NULL)) {
                tmp_expression_name_118 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_118 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 148;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_117 = mod_consts[218];
            tmp_tuple_element_118 = LOOKUP_SUBSCRIPT(tmp_expression_name_118, tmp_subscript_name_117);
            if (tmp_tuple_element_118 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 148;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_118);
            tmp_tuple_element_118 = mod_consts[236];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_118);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 116, tmp_tuple_element_1);
            tmp_expression_name_119 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_119 == NULL)) {
                tmp_expression_name_119 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_119 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 149;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_118 = mod_consts[218];
            tmp_tuple_element_119 = LOOKUP_SUBSCRIPT(tmp_expression_name_119, tmp_subscript_name_118);
            if (tmp_tuple_element_119 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 149;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_119);
            tmp_tuple_element_119 = mod_consts[237];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_119);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 117, tmp_tuple_element_1);
            tmp_expression_name_120 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_120 == NULL)) {
                tmp_expression_name_120 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_120 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 150;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_119 = mod_consts[218];
            tmp_tuple_element_120 = LOOKUP_SUBSCRIPT(tmp_expression_name_120, tmp_subscript_name_119);
            if (tmp_tuple_element_120 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 150;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_120);
            tmp_tuple_element_120 = mod_consts[238];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_120);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 118, tmp_tuple_element_1);
            tmp_expression_name_121 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_121 == NULL)) {
                tmp_expression_name_121 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_121 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 151;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_120 = mod_consts[218];
            tmp_tuple_element_121 = LOOKUP_SUBSCRIPT(tmp_expression_name_121, tmp_subscript_name_120);
            if (tmp_tuple_element_121 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 151;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_121);
            tmp_tuple_element_121 = mod_consts[239];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_121);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 119, tmp_tuple_element_1);
            tmp_expression_name_122 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_122 == NULL)) {
                tmp_expression_name_122 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_122 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 152;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_121 = mod_consts[218];
            tmp_tuple_element_122 = LOOKUP_SUBSCRIPT(tmp_expression_name_122, tmp_subscript_name_121);
            if (tmp_tuple_element_122 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 152;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_122);
            tmp_tuple_element_122 = mod_consts[240];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_122);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 120, tmp_tuple_element_1);
            tmp_expression_name_123 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_123 == NULL)) {
                tmp_expression_name_123 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_123 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 153;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_122 = mod_consts[218];
            tmp_tuple_element_123 = LOOKUP_SUBSCRIPT(tmp_expression_name_123, tmp_subscript_name_122);
            if (tmp_tuple_element_123 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 153;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_123);
            tmp_tuple_element_123 = mod_consts[241];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_123);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 121, tmp_tuple_element_1);
            tmp_expression_name_124 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_124 == NULL)) {
                tmp_expression_name_124 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_124 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 154;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_123 = mod_consts[218];
            tmp_tuple_element_124 = LOOKUP_SUBSCRIPT(tmp_expression_name_124, tmp_subscript_name_123);
            if (tmp_tuple_element_124 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 154;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_124);
            tmp_tuple_element_124 = mod_consts[242];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_124);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 122, tmp_tuple_element_1);
            tmp_expression_name_125 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_125 == NULL)) {
                tmp_expression_name_125 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_125 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 155;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_124 = mod_consts[218];
            tmp_tuple_element_125 = LOOKUP_SUBSCRIPT(tmp_expression_name_125, tmp_subscript_name_124);
            if (tmp_tuple_element_125 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 155;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_125);
            tmp_tuple_element_125 = mod_consts[243];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_125);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 123, tmp_tuple_element_1);
            tmp_expression_name_126 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_126 == NULL)) {
                tmp_expression_name_126 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_126 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 156;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_125 = mod_consts[218];
            tmp_tuple_element_126 = LOOKUP_SUBSCRIPT(tmp_expression_name_126, tmp_subscript_name_125);
            if (tmp_tuple_element_126 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 156;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_126);
            tmp_tuple_element_126 = mod_consts[244];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_126);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 124, tmp_tuple_element_1);
            tmp_expression_name_127 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_127 == NULL)) {
                tmp_expression_name_127 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_127 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 157;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_126 = mod_consts[218];
            tmp_tuple_element_127 = LOOKUP_SUBSCRIPT(tmp_expression_name_127, tmp_subscript_name_126);
            if (tmp_tuple_element_127 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 157;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_127);
            tmp_tuple_element_127 = mod_consts[245];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_127);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 125, tmp_tuple_element_1);
            tmp_expression_name_128 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_128 == NULL)) {
                tmp_expression_name_128 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_128 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 158;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_127 = mod_consts[246];
            tmp_tuple_element_128 = LOOKUP_SUBSCRIPT(tmp_expression_name_128, tmp_subscript_name_127);
            if (tmp_tuple_element_128 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 158;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_128);
            tmp_tuple_element_128 = mod_consts[120];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_128);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 126, tmp_tuple_element_1);
            tmp_expression_name_129 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_129 == NULL)) {
                tmp_expression_name_129 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_129 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 159;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_128 = mod_consts[246];
            tmp_tuple_element_129 = LOOKUP_SUBSCRIPT(tmp_expression_name_129, tmp_subscript_name_128);
            if (tmp_tuple_element_129 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 159;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_129);
            tmp_tuple_element_129 = mod_consts[247];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_129);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 127, tmp_tuple_element_1);
            tmp_expression_name_130 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_130 == NULL)) {
                tmp_expression_name_130 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_130 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 160;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_129 = mod_consts[246];
            tmp_tuple_element_130 = LOOKUP_SUBSCRIPT(tmp_expression_name_130, tmp_subscript_name_129);
            if (tmp_tuple_element_130 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 160;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_130);
            tmp_tuple_element_130 = mod_consts[248];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_130);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 128, tmp_tuple_element_1);
            tmp_expression_name_131 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_131 == NULL)) {
                tmp_expression_name_131 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_131 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 161;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_130 = mod_consts[246];
            tmp_tuple_element_131 = LOOKUP_SUBSCRIPT(tmp_expression_name_131, tmp_subscript_name_130);
            if (tmp_tuple_element_131 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 161;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_131);
            tmp_tuple_element_131 = mod_consts[249];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_131);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 129, tmp_tuple_element_1);
            tmp_expression_name_132 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_132 == NULL)) {
                tmp_expression_name_132 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_132 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 162;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_131 = mod_consts[246];
            tmp_tuple_element_132 = LOOKUP_SUBSCRIPT(tmp_expression_name_132, tmp_subscript_name_131);
            if (tmp_tuple_element_132 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 162;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_132);
            tmp_tuple_element_132 = mod_consts[250];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_132);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 130, tmp_tuple_element_1);
            tmp_expression_name_133 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_133 == NULL)) {
                tmp_expression_name_133 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_133 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 163;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_132 = mod_consts[246];
            tmp_tuple_element_133 = LOOKUP_SUBSCRIPT(tmp_expression_name_133, tmp_subscript_name_132);
            if (tmp_tuple_element_133 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 163;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_133);
            tmp_tuple_element_133 = mod_consts[251];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_133);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 131, tmp_tuple_element_1);
            tmp_expression_name_134 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_134 == NULL)) {
                tmp_expression_name_134 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_134 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 164;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_133 = mod_consts[246];
            tmp_tuple_element_134 = LOOKUP_SUBSCRIPT(tmp_expression_name_134, tmp_subscript_name_133);
            if (tmp_tuple_element_134 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 164;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_134);
            tmp_tuple_element_134 = mod_consts[252];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_134);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 132, tmp_tuple_element_1);
            tmp_expression_name_135 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_135 == NULL)) {
                tmp_expression_name_135 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_135 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 165;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_134 = mod_consts[246];
            tmp_tuple_element_135 = LOOKUP_SUBSCRIPT(tmp_expression_name_135, tmp_subscript_name_134);
            if (tmp_tuple_element_135 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 165;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_135);
            tmp_tuple_element_135 = mod_consts[253];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_135);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 133, tmp_tuple_element_1);
            tmp_expression_name_136 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_136 == NULL)) {
                tmp_expression_name_136 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_136 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 166;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_135 = mod_consts[246];
            tmp_tuple_element_136 = LOOKUP_SUBSCRIPT(tmp_expression_name_136, tmp_subscript_name_135);
            if (tmp_tuple_element_136 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 166;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_136);
            tmp_tuple_element_136 = mod_consts[254];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_136);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 134, tmp_tuple_element_1);
            tmp_expression_name_137 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_137 == NULL)) {
                tmp_expression_name_137 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_137 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 167;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_136 = mod_consts[246];
            tmp_tuple_element_137 = LOOKUP_SUBSCRIPT(tmp_expression_name_137, tmp_subscript_name_136);
            if (tmp_tuple_element_137 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 167;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_137);
            tmp_tuple_element_137 = mod_consts[255];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_137);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 135, tmp_tuple_element_1);
            tmp_expression_name_138 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_138 == NULL)) {
                tmp_expression_name_138 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_138 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 168;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_137 = mod_consts[246];
            tmp_tuple_element_138 = LOOKUP_SUBSCRIPT(tmp_expression_name_138, tmp_subscript_name_137);
            if (tmp_tuple_element_138 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 168;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_138);
            tmp_tuple_element_138 = mod_consts[256];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_138);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 136, tmp_tuple_element_1);
            tmp_expression_name_139 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_139 == NULL)) {
                tmp_expression_name_139 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_139 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 169;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_138 = mod_consts[246];
            tmp_tuple_element_139 = LOOKUP_SUBSCRIPT(tmp_expression_name_139, tmp_subscript_name_138);
            if (tmp_tuple_element_139 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 169;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_139);
            tmp_tuple_element_139 = mod_consts[257];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_139);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 137, tmp_tuple_element_1);
            tmp_expression_name_140 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_140 == NULL)) {
                tmp_expression_name_140 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_140 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 170;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_139 = mod_consts[246];
            tmp_tuple_element_140 = LOOKUP_SUBSCRIPT(tmp_expression_name_140, tmp_subscript_name_139);
            if (tmp_tuple_element_140 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 170;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_140);
            tmp_tuple_element_140 = mod_consts[258];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_140);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 138, tmp_tuple_element_1);
            tmp_expression_name_141 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_141 == NULL)) {
                tmp_expression_name_141 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_141 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 171;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_140 = mod_consts[246];
            tmp_tuple_element_141 = LOOKUP_SUBSCRIPT(tmp_expression_name_141, tmp_subscript_name_140);
            if (tmp_tuple_element_141 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 171;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_141);
            tmp_tuple_element_141 = mod_consts[259];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_141);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 139, tmp_tuple_element_1);
            tmp_expression_name_142 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_142 == NULL)) {
                tmp_expression_name_142 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_142 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 172;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_141 = mod_consts[246];
            tmp_tuple_element_142 = LOOKUP_SUBSCRIPT(tmp_expression_name_142, tmp_subscript_name_141);
            if (tmp_tuple_element_142 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 172;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_142);
            tmp_tuple_element_142 = mod_consts[260];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_142);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 140, tmp_tuple_element_1);
            tmp_expression_name_143 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_143 == NULL)) {
                tmp_expression_name_143 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_143 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 173;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_142 = mod_consts[246];
            tmp_tuple_element_143 = LOOKUP_SUBSCRIPT(tmp_expression_name_143, tmp_subscript_name_142);
            if (tmp_tuple_element_143 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 173;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_143);
            tmp_tuple_element_143 = mod_consts[261];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_143);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 141, tmp_tuple_element_1);
            tmp_expression_name_144 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_144 == NULL)) {
                tmp_expression_name_144 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_144 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 174;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_143 = mod_consts[246];
            tmp_tuple_element_144 = LOOKUP_SUBSCRIPT(tmp_expression_name_144, tmp_subscript_name_143);
            if (tmp_tuple_element_144 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 174;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_144);
            tmp_tuple_element_144 = mod_consts[262];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_144);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 142, tmp_tuple_element_1);
            tmp_expression_name_145 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_145 == NULL)) {
                tmp_expression_name_145 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_145 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 175;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_144 = mod_consts[246];
            tmp_tuple_element_145 = LOOKUP_SUBSCRIPT(tmp_expression_name_145, tmp_subscript_name_144);
            if (tmp_tuple_element_145 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 175;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_145);
            tmp_tuple_element_145 = mod_consts[263];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_145);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 143, tmp_tuple_element_1);
            tmp_expression_name_146 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_146 == NULL)) {
                tmp_expression_name_146 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_146 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 176;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_145 = mod_consts[246];
            tmp_tuple_element_146 = LOOKUP_SUBSCRIPT(tmp_expression_name_146, tmp_subscript_name_145);
            if (tmp_tuple_element_146 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 176;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_146);
            tmp_tuple_element_146 = mod_consts[264];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_146);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 144, tmp_tuple_element_1);
            tmp_expression_name_147 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_147 == NULL)) {
                tmp_expression_name_147 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_147 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 177;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_146 = mod_consts[246];
            tmp_tuple_element_147 = LOOKUP_SUBSCRIPT(tmp_expression_name_147, tmp_subscript_name_146);
            if (tmp_tuple_element_147 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 177;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_147);
            tmp_tuple_element_147 = mod_consts[265];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_147);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 145, tmp_tuple_element_1);
            tmp_expression_name_148 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_148 == NULL)) {
                tmp_expression_name_148 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_148 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 178;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_147 = mod_consts[246];
            tmp_tuple_element_148 = LOOKUP_SUBSCRIPT(tmp_expression_name_148, tmp_subscript_name_147);
            if (tmp_tuple_element_148 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 178;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_148);
            tmp_tuple_element_148 = mod_consts[266];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_148);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 146, tmp_tuple_element_1);
            tmp_expression_name_149 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_149 == NULL)) {
                tmp_expression_name_149 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_149 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 179;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_148 = mod_consts[246];
            tmp_tuple_element_149 = LOOKUP_SUBSCRIPT(tmp_expression_name_149, tmp_subscript_name_148);
            if (tmp_tuple_element_149 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 179;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_149);
            tmp_tuple_element_149 = mod_consts[267];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_149);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 147, tmp_tuple_element_1);
            tmp_expression_name_150 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_150 == NULL)) {
                tmp_expression_name_150 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_150 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 180;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_149 = mod_consts[246];
            tmp_tuple_element_150 = LOOKUP_SUBSCRIPT(tmp_expression_name_150, tmp_subscript_name_149);
            if (tmp_tuple_element_150 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 180;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_150);
            tmp_tuple_element_150 = mod_consts[50];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_150);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 148, tmp_tuple_element_1);
            tmp_expression_name_151 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_151 == NULL)) {
                tmp_expression_name_151 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_151 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 181;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_150 = mod_consts[246];
            tmp_tuple_element_151 = LOOKUP_SUBSCRIPT(tmp_expression_name_151, tmp_subscript_name_150);
            if (tmp_tuple_element_151 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 181;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_151);
            tmp_tuple_element_151 = mod_consts[268];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_151);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 149, tmp_tuple_element_1);
            tmp_expression_name_152 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_152 == NULL)) {
                tmp_expression_name_152 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_152 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 182;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_151 = mod_consts[246];
            tmp_tuple_element_152 = LOOKUP_SUBSCRIPT(tmp_expression_name_152, tmp_subscript_name_151);
            if (tmp_tuple_element_152 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 182;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_152);
            tmp_tuple_element_152 = mod_consts[269];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_152);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 150, tmp_tuple_element_1);
            tmp_expression_name_153 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_153 == NULL)) {
                tmp_expression_name_153 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_153 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 183;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_152 = mod_consts[246];
            tmp_tuple_element_153 = LOOKUP_SUBSCRIPT(tmp_expression_name_153, tmp_subscript_name_152);
            if (tmp_tuple_element_153 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 183;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_153);
            tmp_tuple_element_153 = mod_consts[270];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_153);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 151, tmp_tuple_element_1);
            tmp_expression_name_154 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_154 == NULL)) {
                tmp_expression_name_154 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_154 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 184;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_153 = mod_consts[246];
            tmp_tuple_element_154 = LOOKUP_SUBSCRIPT(tmp_expression_name_154, tmp_subscript_name_153);
            if (tmp_tuple_element_154 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 184;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_154);
            tmp_tuple_element_154 = mod_consts[271];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_154);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 152, tmp_tuple_element_1);
            tmp_expression_name_155 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_155 == NULL)) {
                tmp_expression_name_155 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_155 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 185;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_154 = mod_consts[246];
            tmp_tuple_element_155 = LOOKUP_SUBSCRIPT(tmp_expression_name_155, tmp_subscript_name_154);
            if (tmp_tuple_element_155 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 185;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_155);
            tmp_tuple_element_155 = mod_consts[272];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_155);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 153, tmp_tuple_element_1);
            tmp_expression_name_156 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_156 == NULL)) {
                tmp_expression_name_156 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_156 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 186;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_155 = mod_consts[246];
            tmp_tuple_element_156 = LOOKUP_SUBSCRIPT(tmp_expression_name_156, tmp_subscript_name_155);
            if (tmp_tuple_element_156 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 186;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_156);
            tmp_tuple_element_156 = mod_consts[273];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_156);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 154, tmp_tuple_element_1);
            tmp_expression_name_157 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_157 == NULL)) {
                tmp_expression_name_157 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_157 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 187;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_156 = mod_consts[246];
            tmp_tuple_element_157 = LOOKUP_SUBSCRIPT(tmp_expression_name_157, tmp_subscript_name_156);
            if (tmp_tuple_element_157 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 187;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_157);
            tmp_tuple_element_157 = mod_consts[246];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_157);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 155, tmp_tuple_element_1);
            tmp_expression_name_158 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_158 == NULL)) {
                tmp_expression_name_158 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_158 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 188;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_157 = mod_consts[246];
            tmp_tuple_element_158 = LOOKUP_SUBSCRIPT(tmp_expression_name_158, tmp_subscript_name_157);
            if (tmp_tuple_element_158 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 188;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_158);
            tmp_tuple_element_158 = mod_consts[274];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_158);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 156, tmp_tuple_element_1);
            tmp_expression_name_159 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_159 == NULL)) {
                tmp_expression_name_159 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_159 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 189;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_158 = mod_consts[246];
            tmp_tuple_element_159 = LOOKUP_SUBSCRIPT(tmp_expression_name_159, tmp_subscript_name_158);
            if (tmp_tuple_element_159 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 189;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_159);
            tmp_tuple_element_159 = mod_consts[275];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_159);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 157, tmp_tuple_element_1);
            tmp_expression_name_160 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_160 == NULL)) {
                tmp_expression_name_160 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_160 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 190;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_159 = mod_consts[246];
            tmp_tuple_element_160 = LOOKUP_SUBSCRIPT(tmp_expression_name_160, tmp_subscript_name_159);
            if (tmp_tuple_element_160 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 190;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_160);
            tmp_tuple_element_160 = mod_consts[276];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_160);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 158, tmp_tuple_element_1);
            tmp_expression_name_161 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_161 == NULL)) {
                tmp_expression_name_161 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_161 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 191;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_160 = mod_consts[246];
            tmp_tuple_element_161 = LOOKUP_SUBSCRIPT(tmp_expression_name_161, tmp_subscript_name_160);
            if (tmp_tuple_element_161 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 191;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_161);
            tmp_tuple_element_161 = mod_consts[277];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_161);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 159, tmp_tuple_element_1);
            tmp_expression_name_162 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_162 == NULL)) {
                tmp_expression_name_162 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_162 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 192;

                goto tuple_build_exception_1;
            }
            tmp_subscript_name_161 = mod_consts[246];
            tmp_tuple_element_162 = LOOKUP_SUBSCRIPT(tmp_expression_name_162, tmp_subscript_name_161);
            if (tmp_tuple_element_162 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 192;

                goto tuple_build_exception_1;
            }
            tmp_tuple_element_1 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_1, 0, tmp_tuple_element_162);
            tmp_tuple_element_162 = mod_consts[278];
            PyTuple_SET_ITEM0(tmp_tuple_element_1, 1, tmp_tuple_element_162);
            PyTuple_SET_ITEM(tmp_frozenset_arg_1, 160, tmp_tuple_element_1);
        }
        goto tuple_build_noexception_1;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_1:;
        Py_DECREF(tmp_frozenset_arg_1);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_1:;
        tmp_assign_source_20 = PyFrozenSet_New(tmp_frozenset_arg_1);
        Py_DECREF(tmp_frozenset_arg_1);
        if (tmp_assign_source_20 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 31;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[6], tmp_assign_source_20);
    }
    {
        PyObject *tmp_assign_source_21;
        PyObject *tmp_frozenset_arg_2;
        PyObject *tmp_tuple_element_163;
        tmp_tuple_element_163 = mod_consts[279];
        tmp_frozenset_arg_2 = PyTuple_New(323);
        {
            PyObject *tmp_tuple_element_164;
            PyObject *tmp_expression_name_163;
            PyObject *tmp_subscript_name_162;
            PyObject *tmp_tuple_element_165;
            PyObject *tmp_expression_name_164;
            PyObject *tmp_subscript_name_163;
            PyObject *tmp_tuple_element_166;
            PyObject *tmp_expression_name_165;
            PyObject *tmp_subscript_name_164;
            PyObject *tmp_tuple_element_167;
            PyObject *tmp_expression_name_166;
            PyObject *tmp_subscript_name_165;
            PyObject *tmp_tuple_element_168;
            PyObject *tmp_expression_name_167;
            PyObject *tmp_subscript_name_166;
            PyObject *tmp_tuple_element_169;
            PyObject *tmp_expression_name_168;
            PyObject *tmp_subscript_name_167;
            PyObject *tmp_tuple_element_170;
            PyObject *tmp_expression_name_169;
            PyObject *tmp_subscript_name_168;
            PyObject *tmp_tuple_element_171;
            PyObject *tmp_expression_name_170;
            PyObject *tmp_subscript_name_169;
            PyObject *tmp_tuple_element_172;
            PyObject *tmp_expression_name_171;
            PyObject *tmp_subscript_name_170;
            PyObject *tmp_tuple_element_173;
            PyObject *tmp_expression_name_172;
            PyObject *tmp_subscript_name_171;
            PyObject *tmp_tuple_element_174;
            PyObject *tmp_expression_name_173;
            PyObject *tmp_subscript_name_172;
            PyObject *tmp_tuple_element_175;
            PyObject *tmp_expression_name_174;
            PyObject *tmp_subscript_name_173;
            PyObject *tmp_tuple_element_176;
            PyObject *tmp_expression_name_175;
            PyObject *tmp_subscript_name_174;
            PyObject *tmp_tuple_element_177;
            PyObject *tmp_expression_name_176;
            PyObject *tmp_subscript_name_175;
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 0, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[280];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 1, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[281];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 2, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[282];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 3, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[283];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 4, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[284];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 5, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[285];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 6, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[286];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 7, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[287];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 8, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[288];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 9, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[289];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 10, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[290];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 11, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[291];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 12, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[292];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 13, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[293];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 14, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[294];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 15, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[295];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 16, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[296];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 17, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[297];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 18, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[298];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 19, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[299];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 20, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[300];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 21, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[301];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 22, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[302];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 23, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[303];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 24, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[304];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 25, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[305];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 26, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[306];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 27, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[307];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 28, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[308];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 29, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[309];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 30, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[310];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 31, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[311];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 32, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[312];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 33, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[313];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 34, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[314];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 35, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[315];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 36, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[316];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 37, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[317];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 38, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[318];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 39, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[319];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 40, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[320];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 41, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[321];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 42, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[322];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 43, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[323];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 44, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[324];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 45, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[325];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 46, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[326];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 47, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[327];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 48, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[328];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 49, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[329];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 50, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[330];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 51, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[331];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 52, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[332];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 53, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[333];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 54, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[334];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 55, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[335];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 56, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[336];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 57, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[337];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 58, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[338];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 59, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[339];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 60, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[340];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 61, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[341];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 62, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[342];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 63, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[343];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 64, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[344];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 65, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[345];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 66, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[346];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 67, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[347];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 68, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[348];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 69, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[349];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 70, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[350];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 71, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[351];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 72, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[352];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 73, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[353];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 74, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[354];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 75, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[355];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 76, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[356];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 77, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[357];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 78, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[358];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 79, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[359];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 80, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[360];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 81, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[361];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 82, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[362];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 83, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[363];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 84, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[364];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 85, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[365];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 86, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[366];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 87, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[367];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 88, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[368];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 89, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[369];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 90, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[370];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 91, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[371];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 92, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[372];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 93, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[373];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 94, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[374];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 95, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[375];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 96, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[376];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 97, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[377];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 98, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[378];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 99, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[379];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 100, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[380];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 101, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[381];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 102, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[382];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 103, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[383];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 104, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[384];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 105, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[385];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 106, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[386];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 107, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[387];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 108, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[388];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 109, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[389];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 110, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[390];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 111, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[391];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 112, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[392];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 113, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[393];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 114, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[394];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 115, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[395];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 116, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[396];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 117, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[397];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 118, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[398];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 119, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[59];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 120, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[399];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 121, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[400];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 122, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[401];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 123, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[402];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 124, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[403];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 125, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[404];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 126, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[405];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 127, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[406];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 128, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[407];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 129, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[408];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 130, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[409];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 131, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[410];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 132, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[411];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 133, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[412];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 134, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[413];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 135, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[414];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 136, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[415];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 137, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[416];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 138, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[417];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 139, tmp_tuple_element_163);
            tmp_expression_name_163 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_163 == NULL)) {
                tmp_expression_name_163 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_163 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 337;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_162 = mod_consts[418];
            tmp_tuple_element_164 = LOOKUP_SUBSCRIPT(tmp_expression_name_163, tmp_subscript_name_162);
            if (tmp_tuple_element_164 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 337;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_164);
            tmp_tuple_element_164 = mod_consts[419];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_164);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 140, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[420];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 141, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[284];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 142, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[421];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 143, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[421];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 144, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[421];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 145, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[422];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 146, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[423];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 147, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[424];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 148, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[425];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 149, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[426];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 150, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[427];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 151, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[428];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 152, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[429];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 153, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[430];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 154, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[431];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 155, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[432];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 156, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[333];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 157, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[337];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 158, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[433];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 159, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[434];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 160, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[435];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 161, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[436];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 162, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[437];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 163, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[437];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 164, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[438];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 165, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[439];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 166, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[440];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 167, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[441];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 168, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[441];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 169, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[441];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 170, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[442];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 171, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[443];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 172, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[389];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 173, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[444];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 174, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[445];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 175, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[446];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 176, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[447];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 177, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[448];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 178, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[416];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 179, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[416];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 180, tmp_tuple_element_163);
            tmp_expression_name_164 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_164 == NULL)) {
                tmp_expression_name_164 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_164 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 379;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_163 = mod_consts[55];
            tmp_tuple_element_165 = LOOKUP_SUBSCRIPT(tmp_expression_name_164, tmp_subscript_name_163);
            if (tmp_tuple_element_165 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 379;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_165);
            tmp_tuple_element_165 = mod_consts[56];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_165);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 181, tmp_tuple_element_163);
            tmp_expression_name_165 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_165 == NULL)) {
                tmp_expression_name_165 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_165 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 380;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_164 = mod_consts[55];
            tmp_tuple_element_166 = LOOKUP_SUBSCRIPT(tmp_expression_name_165, tmp_subscript_name_164);
            if (tmp_tuple_element_166 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 380;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_166);
            tmp_tuple_element_166 = mod_consts[449];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_166);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 182, tmp_tuple_element_163);
            tmp_expression_name_166 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_166 == NULL)) {
                tmp_expression_name_166 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_166 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 381;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_165 = mod_consts[55];
            tmp_tuple_element_167 = LOOKUP_SUBSCRIPT(tmp_expression_name_166, tmp_subscript_name_165);
            if (tmp_tuple_element_167 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 381;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_167);
            tmp_tuple_element_167 = mod_consts[22];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_167);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 183, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[450];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 184, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[451];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 185, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[452];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 186, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[453];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 187, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[454];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 188, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[455];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 189, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[456];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 190, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[457];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 191, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[458];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 192, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[459];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 193, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[460];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 194, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[461];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 195, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[462];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 196, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[463];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 197, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[308];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 198, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[464];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 199, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[310];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 200, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[465];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 201, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[466];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 202, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[467];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 203, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[468];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 204, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[469];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 205, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[470];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 206, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[471];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 207, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[472];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 208, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[426];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 209, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[473];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 210, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[329];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 211, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[474];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 212, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[475];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 213, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[476];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 214, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[477];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 215, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[478];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 216, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[479];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 217, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[480];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 218, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[481];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 219, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[482];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 220, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[483];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 221, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[484];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 222, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[485];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 223, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[486];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 224, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[487];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 225, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[488];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 226, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[489];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 227, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[490];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 228, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[337];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 229, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[491];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 230, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[492];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 231, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[345];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 232, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[493];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 233, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[494];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 234, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[495];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 235, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[496];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 236, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[497];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 237, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[351];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 238, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[498];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 239, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[499];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 240, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[500];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 241, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[501];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 242, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[502];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 243, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[503];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 244, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[504];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 245, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[360];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 246, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[364];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 247, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[366];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 248, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[505];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 249, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[506];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 250, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[507];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 251, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[508];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 252, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[509];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 253, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[510];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 254, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[511];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 255, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[512];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 256, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[513];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 257, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[514];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 258, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[515];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 259, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[516];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 260, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[517];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 261, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[518];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 262, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[519];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 263, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[520];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 264, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[521];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 265, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[522];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 266, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[523];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 267, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[524];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 268, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[525];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 269, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[526];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 270, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[527];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 271, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[528];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 272, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[529];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 273, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[530];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 274, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[531];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 275, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[532];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 276, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[533];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 277, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[534];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 278, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[535];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 279, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[536];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 280, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[537];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 281, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[538];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 282, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[539];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 283, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[540];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 284, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[541];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 285, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[542];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 286, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[402];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 287, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[543];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 288, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[544];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 289, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[545];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 290, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[406];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 291, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[546];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 292, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[547];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 293, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[548];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 294, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[549];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 295, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[550];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 296, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[551];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 297, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[552];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 298, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[553];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 299, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[554];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 300, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[555];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 301, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[556];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 302, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[416];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 303, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[557];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 304, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[558];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 305, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[559];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 306, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[560];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 307, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[561];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 308, tmp_tuple_element_163);
            tmp_expression_name_167 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_167 == NULL)) {
                tmp_expression_name_167 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_167 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 508;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_166 = mod_consts[55];
            tmp_tuple_element_168 = LOOKUP_SUBSCRIPT(tmp_expression_name_167, tmp_subscript_name_166);
            if (tmp_tuple_element_168 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 508;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_168);
            tmp_tuple_element_168 = mod_consts[562];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_168);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 309, tmp_tuple_element_163);
            tmp_expression_name_168 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_168 == NULL)) {
                tmp_expression_name_168 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_168 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 509;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_167 = mod_consts[55];
            tmp_tuple_element_169 = LOOKUP_SUBSCRIPT(tmp_expression_name_168, tmp_subscript_name_167);
            if (tmp_tuple_element_169 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 509;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_169);
            tmp_tuple_element_169 = mod_consts[563];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_169);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 310, tmp_tuple_element_163);
            tmp_expression_name_169 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_169 == NULL)) {
                tmp_expression_name_169 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_169 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 510;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_168 = mod_consts[55];
            tmp_tuple_element_170 = LOOKUP_SUBSCRIPT(tmp_expression_name_169, tmp_subscript_name_168);
            if (tmp_tuple_element_170 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 510;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_170);
            tmp_tuple_element_170 = mod_consts[56];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_170);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 311, tmp_tuple_element_163);
            tmp_expression_name_170 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_170 == NULL)) {
                tmp_expression_name_170 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_170 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 511;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_169 = mod_consts[55];
            tmp_tuple_element_171 = LOOKUP_SUBSCRIPT(tmp_expression_name_170, tmp_subscript_name_169);
            if (tmp_tuple_element_171 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 511;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_171);
            tmp_tuple_element_171 = mod_consts[564];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_171);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 312, tmp_tuple_element_163);
            tmp_expression_name_171 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_171 == NULL)) {
                tmp_expression_name_171 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_171 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 512;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_170 = mod_consts[55];
            tmp_tuple_element_172 = LOOKUP_SUBSCRIPT(tmp_expression_name_171, tmp_subscript_name_170);
            if (tmp_tuple_element_172 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 512;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_172);
            tmp_tuple_element_172 = mod_consts[449];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_172);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 313, tmp_tuple_element_163);
            tmp_expression_name_172 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_172 == NULL)) {
                tmp_expression_name_172 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_172 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 513;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_171 = mod_consts[55];
            tmp_tuple_element_173 = LOOKUP_SUBSCRIPT(tmp_expression_name_172, tmp_subscript_name_171);
            if (tmp_tuple_element_173 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 513;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_173);
            tmp_tuple_element_173 = mod_consts[276];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_173);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 314, tmp_tuple_element_163);
            tmp_expression_name_173 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_173 == NULL)) {
                tmp_expression_name_173 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_173 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 514;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_172 = mod_consts[55];
            tmp_tuple_element_174 = LOOKUP_SUBSCRIPT(tmp_expression_name_173, tmp_subscript_name_172);
            if (tmp_tuple_element_174 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 514;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_174);
            tmp_tuple_element_174 = mod_consts[22];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_174);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 315, tmp_tuple_element_163);
            tmp_expression_name_174 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_174 == NULL)) {
                tmp_expression_name_174 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_174 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 515;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_173 = mod_consts[418];
            tmp_tuple_element_175 = LOOKUP_SUBSCRIPT(tmp_expression_name_174, tmp_subscript_name_173);
            if (tmp_tuple_element_175 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 515;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_175);
            tmp_tuple_element_175 = mod_consts[17];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_175);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 316, tmp_tuple_element_163);
            tmp_expression_name_175 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_175 == NULL)) {
                tmp_expression_name_175 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_175 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 516;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_174 = mod_consts[418];
            tmp_tuple_element_176 = LOOKUP_SUBSCRIPT(tmp_expression_name_175, tmp_subscript_name_174);
            if (tmp_tuple_element_176 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 516;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_176);
            tmp_tuple_element_176 = mod_consts[419];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_176);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 317, tmp_tuple_element_163);
            tmp_expression_name_176 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_176 == NULL)) {
                tmp_expression_name_176 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_176 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 517;

                goto tuple_build_exception_2;
            }
            tmp_subscript_name_175 = mod_consts[418];
            tmp_tuple_element_177 = LOOKUP_SUBSCRIPT(tmp_expression_name_176, tmp_subscript_name_175);
            if (tmp_tuple_element_177 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 517;

                goto tuple_build_exception_2;
            }
            tmp_tuple_element_163 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_163, 0, tmp_tuple_element_177);
            tmp_tuple_element_177 = mod_consts[565];
            PyTuple_SET_ITEM0(tmp_tuple_element_163, 1, tmp_tuple_element_177);
            PyTuple_SET_ITEM(tmp_frozenset_arg_2, 318, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[566];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 319, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[567];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 320, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[568];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 321, tmp_tuple_element_163);
            tmp_tuple_element_163 = mod_consts[569];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_2, 322, tmp_tuple_element_163);
        }
        goto tuple_build_noexception_2;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_2:;
        Py_DECREF(tmp_frozenset_arg_2);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_2:;
        tmp_assign_source_21 = PyFrozenSet_New(tmp_frozenset_arg_2);
        Py_DECREF(tmp_frozenset_arg_2);
        if (tmp_assign_source_21 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 195;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[7], tmp_assign_source_21);
    }
    {
        PyObject *tmp_assign_source_22;
        PyObject *tmp_frozenset_arg_3;
        PyObject *tmp_tuple_element_178;
        tmp_tuple_element_178 = mod_consts[341];
        tmp_frozenset_arg_3 = PyTuple_New(13);
        {
            PyObject *tmp_tuple_element_179;
            PyObject *tmp_expression_name_177;
            PyObject *tmp_subscript_name_176;
            PyObject *tmp_tuple_element_180;
            PyObject *tmp_expression_name_178;
            PyObject *tmp_subscript_name_177;
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 0, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[396];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 1, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[307];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 2, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[283];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 3, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[353];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 4, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[375];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 5, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[289];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 6, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[320];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 7, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[327];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 8, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[359];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 9, tmp_tuple_element_178);
            tmp_tuple_element_178 = mod_consts[373];
            PyTuple_SET_ITEM0(tmp_frozenset_arg_3, 10, tmp_tuple_element_178);
            tmp_expression_name_177 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_177 == NULL)) {
                tmp_expression_name_177 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_177 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 536;

                goto tuple_build_exception_3;
            }
            tmp_subscript_name_176 = mod_consts[55];
            tmp_tuple_element_179 = LOOKUP_SUBSCRIPT(tmp_expression_name_177, tmp_subscript_name_176);
            if (tmp_tuple_element_179 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 536;

                goto tuple_build_exception_3;
            }
            tmp_tuple_element_178 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_178, 0, tmp_tuple_element_179);
            tmp_tuple_element_179 = mod_consts[56];
            PyTuple_SET_ITEM0(tmp_tuple_element_178, 1, tmp_tuple_element_179);
            PyTuple_SET_ITEM(tmp_frozenset_arg_3, 11, tmp_tuple_element_178);
            tmp_expression_name_178 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[26]);

            if (unlikely(tmp_expression_name_178 == NULL)) {
                tmp_expression_name_178 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[26]);
            }

            if (tmp_expression_name_178 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 537;

                goto tuple_build_exception_3;
            }
            tmp_subscript_name_177 = mod_consts[418];
            tmp_tuple_element_180 = LOOKUP_SUBSCRIPT(tmp_expression_name_178, tmp_subscript_name_177);
            if (tmp_tuple_element_180 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 537;

                goto tuple_build_exception_3;
            }
            tmp_tuple_element_178 = PyTuple_New(2);
            PyTuple_SET_ITEM(tmp_tuple_element_178, 0, tmp_tuple_element_180);
            tmp_tuple_element_180 = mod_consts[17];
            PyTuple_SET_ITEM0(tmp_tuple_element_178, 1, tmp_tuple_element_180);
            PyTuple_SET_ITEM(tmp_frozenset_arg_3, 12, tmp_tuple_element_178);
        }
        goto tuple_build_noexception_3;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_3:;
        Py_DECREF(tmp_frozenset_arg_3);
        goto frame_exception_exit_1;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_3:;
        tmp_assign_source_22 = PyFrozenSet_New(tmp_frozenset_arg_3);
        Py_DECREF(tmp_frozenset_arg_3);
        if (tmp_assign_source_22 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 524;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[13], tmp_assign_source_22);
    }
    {
        PyObject *tmp_assign_source_23;
        tmp_assign_source_23 = mod_consts[570];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[14], tmp_assign_source_23);
    }
    {
        PyObject *tmp_assign_source_24;
        tmp_assign_source_24 = mod_consts[571];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[15], tmp_assign_source_24);
    }
    {
        PyObject *tmp_assign_source_25;
        tmp_assign_source_25 = mod_consts[572];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[8], tmp_assign_source_25);
    }
    {
        PyObject *tmp_assign_source_26;
        tmp_assign_source_26 = mod_consts[573];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[9], tmp_assign_source_26);
    }
    {
        PyObject *tmp_assign_source_27;
        tmp_assign_source_27 = mod_consts[574];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[10], tmp_assign_source_27);
    }
    {
        PyObject *tmp_assign_source_28;
        tmp_assign_source_28 = mod_consts[575];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[11], tmp_assign_source_28);
    }
    {
        PyObject *tmp_assign_source_29;
        tmp_assign_source_29 = mod_consts[576];
        UPDATE_STRING_DICT0(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[12], tmp_assign_source_29);
    }
    {
        PyObject *tmp_assign_source_30;
        PyObject *tmp_called_name_2;
        PyObject *tmp_expression_name_179;
        PyObject *tmp_args_element_name_3;
        PyObject *tmp_args_element_name_4;
        PyObject *tmp_expression_name_180;
        tmp_expression_name_179 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_expression_name_179 == NULL)) {
            tmp_expression_name_179 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_expression_name_179 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 710;

            goto frame_exception_exit_1;
        }
        tmp_called_name_2 = LOOKUP_ATTRIBUTE(tmp_expression_name_179, mod_consts[79]);
        if (tmp_called_name_2 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 710;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_3 = mod_consts[577];
        tmp_expression_name_180 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[37]);

        if (unlikely(tmp_expression_name_180 == NULL)) {
            tmp_expression_name_180 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[37]);
        }

        if (tmp_expression_name_180 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_2);

            exception_lineno = 721;

            goto frame_exception_exit_1;
        }
        tmp_args_element_name_4 = LOOKUP_ATTRIBUTE(tmp_expression_name_180, mod_consts[578]);
        if (tmp_args_element_name_4 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);
            Py_DECREF(tmp_called_name_2);

            exception_lineno = 721;

            goto frame_exception_exit_1;
        }
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 710;
        {
            PyObject *call_args[] = {tmp_args_element_name_3, tmp_args_element_name_4};
            tmp_assign_source_30 = CALL_FUNCTION_WITH_ARGS2(tmp_called_name_2, call_args);
        }

        Py_DECREF(tmp_called_name_2);
        Py_DECREF(tmp_args_element_name_4);
        if (tmp_assign_source_30 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 710;

            goto frame_exception_exit_1;
        }
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[48], tmp_assign_source_30);
    }
    // Tried code:
    {
        PyObject *tmp_assign_source_31;
        PyObject *tmp_tuple_element_181;
        PyObject *tmp_expression_name_181;
        tmp_expression_name_181 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[17]);

        if (unlikely(tmp_expression_name_181 == NULL)) {
            tmp_expression_name_181 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[17]);
        }

        if (tmp_expression_name_181 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        tmp_tuple_element_181 = LOOKUP_ATTRIBUTE(tmp_expression_name_181, mod_consts[0]);
        if (tmp_tuple_element_181 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        tmp_assign_source_31 = PyTuple_New(1);
        PyTuple_SET_ITEM(tmp_assign_source_31, 0, tmp_tuple_element_181);
        assert(tmp_class_creation_1__bases == NULL);
        tmp_class_creation_1__bases = tmp_assign_source_31;
    }
    {
        PyObject *tmp_assign_source_32;
        tmp_assign_source_32 = PyDict_New();
        assert(tmp_class_creation_1__class_decl_dict == NULL);
        tmp_class_creation_1__class_decl_dict = tmp_assign_source_32;
    }
    {
        PyObject *tmp_assign_source_33;
        PyObject *tmp_metaclass_name_1;
        nuitka_bool tmp_condition_result_1;
        PyObject *tmp_key_name_1;
        PyObject *tmp_dict_arg_name_1;
        PyObject *tmp_dict_arg_name_2;
        PyObject *tmp_key_name_2;
        PyObject *tmp_type_arg_1;
        PyObject *tmp_expression_name_182;
        PyObject *tmp_subscript_name_178;
        PyObject *tmp_bases_name_1;
        tmp_key_name_1 = mod_consts[579];
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
        tmp_key_name_2 = mod_consts[579];
        tmp_metaclass_name_1 = DICT_GET_ITEM_WITH_ERROR(tmp_dict_arg_name_2, tmp_key_name_2);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        goto condexpr_end_1;
        condexpr_false_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_expression_name_182 = tmp_class_creation_1__bases;
        tmp_subscript_name_178 = mod_consts[92];
        tmp_type_arg_1 = LOOKUP_SUBSCRIPT_CONST(tmp_expression_name_182, tmp_subscript_name_178, 0);
        if (tmp_type_arg_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        tmp_metaclass_name_1 = BUILTIN_TYPE1(tmp_type_arg_1);
        Py_DECREF(tmp_type_arg_1);
        if (tmp_metaclass_name_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        condexpr_end_1:;
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_bases_name_1 = tmp_class_creation_1__bases;
        tmp_assign_source_33 = SELECT_METACLASS(tmp_metaclass_name_1, tmp_bases_name_1);
        Py_DECREF(tmp_metaclass_name_1);
        if (tmp_assign_source_33 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        assert(tmp_class_creation_1__metaclass == NULL);
        tmp_class_creation_1__metaclass = tmp_assign_source_33;
    }
    {
        nuitka_bool tmp_condition_result_2;
        PyObject *tmp_key_name_3;
        PyObject *tmp_dict_arg_name_3;
        tmp_key_name_3 = mod_consts[579];
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
    tmp_dictdel_key = mod_consts[579];
    tmp_result = DICT_REMOVE_ITEM(tmp_dictdel_dict, tmp_dictdel_key);
    if (tmp_result == false) {
        assert(ERROR_OCCURRED());

        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


        exception_lineno = 724;

        goto try_except_handler_4;
    }
    branch_no_1:;
    {
        nuitka_bool tmp_condition_result_3;
        PyObject *tmp_expression_name_183;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_183 = tmp_class_creation_1__metaclass;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_183, mod_consts[580]);
        tmp_condition_result_3 = (tmp_result) ? NUITKA_BOOL_TRUE : NUITKA_BOOL_FALSE;
        if (tmp_condition_result_3 == NUITKA_BOOL_TRUE) {
            goto branch_yes_2;
        } else {
            goto branch_no_2;
        }
    }
    branch_yes_2:;
    {
        PyObject *tmp_assign_source_34;
        PyObject *tmp_called_name_3;
        PyObject *tmp_expression_name_184;
        PyObject *tmp_args_name_1;
        PyObject *tmp_tuple_element_182;
        PyObject *tmp_kwargs_name_1;
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_expression_name_184 = tmp_class_creation_1__metaclass;
        tmp_called_name_3 = LOOKUP_ATTRIBUTE(tmp_expression_name_184, mod_consts[580]);
        if (tmp_called_name_3 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        tmp_tuple_element_182 = mod_consts[0];
        tmp_args_name_1 = PyTuple_New(2);
        PyTuple_SET_ITEM0(tmp_args_name_1, 0, tmp_tuple_element_182);
        CHECK_OBJECT(tmp_class_creation_1__bases);
        tmp_tuple_element_182 = tmp_class_creation_1__bases;
        PyTuple_SET_ITEM0(tmp_args_name_1, 1, tmp_tuple_element_182);
        CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
        tmp_kwargs_name_1 = tmp_class_creation_1__class_decl_dict;
        frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 724;
        tmp_assign_source_34 = CALL_FUNCTION(tmp_called_name_3, tmp_args_name_1, tmp_kwargs_name_1);
        Py_DECREF(tmp_called_name_3);
        Py_DECREF(tmp_args_name_1);
        if (tmp_assign_source_34 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_34;
    }
    {
        nuitka_bool tmp_condition_result_4;
        PyObject *tmp_operand_name_1;
        PyObject *tmp_expression_name_185;
        CHECK_OBJECT(tmp_class_creation_1__prepared);
        tmp_expression_name_185 = tmp_class_creation_1__prepared;
        tmp_result = HAS_ATTR_BOOL(tmp_expression_name_185, mod_consts[581]);
        tmp_operand_name_1 = (tmp_result) ? Py_True : Py_False;
        tmp_res = CHECK_IF_TRUE(tmp_operand_name_1);
        if (tmp_res == -1) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
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
        PyObject *tmp_tuple_element_183;
        PyObject *tmp_getattr_target_1;
        PyObject *tmp_getattr_attr_1;
        PyObject *tmp_getattr_default_1;
        tmp_raise_type_1 = PyExc_TypeError;
        tmp_left_name_1 = mod_consts[582];
        CHECK_OBJECT(tmp_class_creation_1__metaclass);
        tmp_getattr_target_1 = tmp_class_creation_1__metaclass;
        tmp_getattr_attr_1 = mod_consts[583];
        tmp_getattr_default_1 = mod_consts[584];
        tmp_tuple_element_183 = BUILTIN_GETATTR(tmp_getattr_target_1, tmp_getattr_attr_1, tmp_getattr_default_1);
        if (tmp_tuple_element_183 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        tmp_right_name_1 = PyTuple_New(2);
        {
            PyObject *tmp_expression_name_186;
            PyObject *tmp_type_arg_2;
            PyTuple_SET_ITEM(tmp_right_name_1, 0, tmp_tuple_element_183);
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_type_arg_2 = tmp_class_creation_1__prepared;
            tmp_expression_name_186 = BUILTIN_TYPE1(tmp_type_arg_2);
            assert(!(tmp_expression_name_186 == NULL));
            tmp_tuple_element_183 = LOOKUP_ATTRIBUTE(tmp_expression_name_186, mod_consts[583]);
            Py_DECREF(tmp_expression_name_186);
            if (tmp_tuple_element_183 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 724;

                goto tuple_build_exception_4;
            }
            PyTuple_SET_ITEM(tmp_right_name_1, 1, tmp_tuple_element_183);
        }
        goto tuple_build_noexception_4;
        // Exception handling pass through code for tuple_build:
        tuple_build_exception_4:;
        Py_DECREF(tmp_right_name_1);
        goto try_except_handler_4;
        // Finished with no exception for tuple_build:
        tuple_build_noexception_4:;
        tmp_raise_value_1 = BINARY_OPERATION_MOD_OBJECT_UNICODE_TUPLE(tmp_left_name_1, tmp_right_name_1);
        Py_DECREF(tmp_right_name_1);
        if (tmp_raise_value_1 == NULL) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_4;
        }
        exception_type = tmp_raise_type_1;
        Py_INCREF(tmp_raise_type_1);
        exception_value = tmp_raise_value_1;
        exception_lineno = 724;
        RAISE_EXCEPTION_IMPLICIT(&exception_type, &exception_value, &exception_tb);

        goto try_except_handler_4;
    }
    branch_no_3:;
    goto branch_end_2;
    branch_no_2:;
    {
        PyObject *tmp_assign_source_35;
        tmp_assign_source_35 = PyDict_New();
        assert(tmp_class_creation_1__prepared == NULL);
        tmp_class_creation_1__prepared = tmp_assign_source_35;
    }
    branch_end_2:;
    {
        PyObject *tmp_assign_source_36;
        {
            PyObject *tmp_set_locals_1;
            CHECK_OBJECT(tmp_class_creation_1__prepared);
            tmp_set_locals_1 = tmp_class_creation_1__prepared;
            locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724 = tmp_set_locals_1;
            Py_INCREF(tmp_set_locals_1);
        }
        // Tried code:
        // Tried code:
        tmp_dictset_value = mod_consts[585];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[586], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_6;
        }
        tmp_dictset_value = mod_consts[587];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[97], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_6;
        }
        tmp_dictset_value = mod_consts[0];
        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[588], tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 724;

            goto try_except_handler_6;
        }
        if (isFrameUnusable(cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2)) {
            Py_XDECREF(cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2);

#if _DEBUG_REFCOUNTS
            if (cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2 == NULL) {
                count_active_frame_cache_instances += 1;
            } else {
                count_released_frame_cache_instances += 1;
            }
            count_allocated_frame_cache_instances += 1;
#endif
            cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2 = MAKE_FUNCTION_FRAME(codeobj_4dc3aedd5f581d86900fc22db4cf5702, module_pip$_vendor$html5lib$filters$sanitizer, sizeof(void *));
#if _DEBUG_REFCOUNTS
        } else {
            count_hit_frame_cache_instances += 1;
#endif
        }
        assert(cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2->m_type_description == NULL);
        frame_4dc3aedd5f581d86900fc22db4cf5702_2 = cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2;

        // Push the new frame as the currently active one.
        pushFrameStack(frame_4dc3aedd5f581d86900fc22db4cf5702_2);

        // Mark the frame object as in use, ref count 1 will be up for reuse.
        assert(Py_REFCNT(frame_4dc3aedd5f581d86900fc22db4cf5702_2) == 2); // Frame stack

        // Framed code:
        {
            PyObject *tmp_defaults_1;
            PyObject *tmp_tuple_element_184;
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[6]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[6]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[6]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 728;
                        type_description_2 = "o";
                        goto frame_exception_exit_2;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            tmp_defaults_1 = PyTuple_New(10);
            PyTuple_SET_ITEM(tmp_defaults_1, 0, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[7]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[7]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[7]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 729;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 1, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[8]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[8]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[8]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 730;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 2, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[9]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[9]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[9]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 731;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 3, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[10]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[10]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[10]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 732;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 4, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[11]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[11]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[11]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 733;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 5, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[12]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[12]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[12]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 734;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 6, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[13]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[13]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[13]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 735;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 7, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[14]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[14]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[14]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 736;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 8, tmp_tuple_element_184);
            tmp_tuple_element_184 = PyObject_GetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[15]);

            if (tmp_tuple_element_184 == NULL) {
                if (CHECK_AND_CLEAR_KEY_ERROR_OCCURRED()) {
                    tmp_tuple_element_184 = GET_STRING_DICT_VALUE(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[15]);

                    if (unlikely(tmp_tuple_element_184 == NULL)) {
                        tmp_tuple_element_184 = GET_MODULE_VARIABLE_VALUE_FALLBACK(mod_consts[15]);
                    }

                    if (tmp_tuple_element_184 == NULL) {
                        assert(ERROR_OCCURRED());

                        FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                        exception_lineno = 737;
                        type_description_2 = "o";
                        goto tuple_build_exception_5;
                    }
                    Py_INCREF(tmp_tuple_element_184);
                }
            }

            PyTuple_SET_ITEM(tmp_defaults_1, 9, tmp_tuple_element_184);
            goto tuple_build_noexception_5;
            // Exception handling pass through code for tuple_build:
            tuple_build_exception_5:;
            Py_DECREF(tmp_defaults_1);
            goto frame_exception_exit_2;
            // Finished with no exception for tuple_build:
            tuple_build_noexception_5:;


            tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__1___init__(tmp_defaults_1);

            tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[1], tmp_dictset_value);
            Py_DECREF(tmp_dictset_value);
            if (tmp_res != 0) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 726;
                type_description_2 = "o";
                goto frame_exception_exit_2;
            }
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__2___iter__();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[18], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 784;
            type_description_2 = "o";
            goto frame_exception_exit_2;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__3_sanitize_token();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[20], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 801;
            type_description_2 = "o";
            goto frame_exception_exit_2;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__4_allowed_token();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[28], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 819;
            type_description_2 = "o";
            goto frame_exception_exit_2;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__5_disallowed_token();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[29], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 869;
            type_description_2 = "o";
            goto frame_exception_exit_2;
        }


        tmp_dictset_value = MAKE_FUNCTION_pip$_vendor$html5lib$filters$sanitizer$$$function__6_sanitize_css();

        tmp_res = PyObject_SetItem(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724, mod_consts[60], tmp_dictset_value);
        Py_DECREF(tmp_dictset_value);
        if (tmp_res != 0) {
            assert(ERROR_OCCURRED());

            FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


            exception_lineno = 889;
            type_description_2 = "o";
            goto frame_exception_exit_2;
        }

#if 0
        RESTORE_FRAME_EXCEPTION(frame_4dc3aedd5f581d86900fc22db4cf5702_2);
#endif

        // Put the previous frame back on top.
        popFrameStack();

        goto frame_no_exception_1;

        frame_exception_exit_2:;

#if 0
        RESTORE_FRAME_EXCEPTION(frame_4dc3aedd5f581d86900fc22db4cf5702_2);
#endif

        if (exception_tb == NULL) {
            exception_tb = MAKE_TRACEBACK(frame_4dc3aedd5f581d86900fc22db4cf5702_2, exception_lineno);
        } else if (exception_tb->tb_frame != &frame_4dc3aedd5f581d86900fc22db4cf5702_2->m_frame) {
            exception_tb = ADD_TRACEBACK(exception_tb, frame_4dc3aedd5f581d86900fc22db4cf5702_2, exception_lineno);
        }

        // Attaches locals to frame if any.
        Nuitka_Frame_AttachLocals(
            frame_4dc3aedd5f581d86900fc22db4cf5702_2,
            type_description_2,
            outline_0_var___class__
        );


        // Release cached frame if used for exception.
        if (frame_4dc3aedd5f581d86900fc22db4cf5702_2 == cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2) {
#if _DEBUG_REFCOUNTS
            count_active_frame_cache_instances -= 1;
            count_released_frame_cache_instances += 1;
#endif

            Py_DECREF(cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2);
            cache_frame_4dc3aedd5f581d86900fc22db4cf5702_2 = NULL;
        }

        assertFrameObject(frame_4dc3aedd5f581d86900fc22db4cf5702_2);

        // Put the previous frame back on top.
        popFrameStack();

        // Return the error.
        goto nested_frame_exit_1;

        frame_no_exception_1:;
        goto skip_nested_handling_1;
        nested_frame_exit_1:;

        goto try_except_handler_6;
        skip_nested_handling_1:;
        {
            PyObject *tmp_assign_source_37;
            PyObject *tmp_called_name_4;
            PyObject *tmp_args_name_2;
            PyObject *tmp_tuple_element_185;
            PyObject *tmp_kwargs_name_2;
            CHECK_OBJECT(tmp_class_creation_1__metaclass);
            tmp_called_name_4 = tmp_class_creation_1__metaclass;
            tmp_tuple_element_185 = mod_consts[0];
            tmp_args_name_2 = PyTuple_New(3);
            PyTuple_SET_ITEM0(tmp_args_name_2, 0, tmp_tuple_element_185);
            CHECK_OBJECT(tmp_class_creation_1__bases);
            tmp_tuple_element_185 = tmp_class_creation_1__bases;
            PyTuple_SET_ITEM0(tmp_args_name_2, 1, tmp_tuple_element_185);
            tmp_tuple_element_185 = locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724;
            PyTuple_SET_ITEM0(tmp_args_name_2, 2, tmp_tuple_element_185);
            CHECK_OBJECT(tmp_class_creation_1__class_decl_dict);
            tmp_kwargs_name_2 = tmp_class_creation_1__class_decl_dict;
            frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame.f_lineno = 724;
            tmp_assign_source_37 = CALL_FUNCTION(tmp_called_name_4, tmp_args_name_2, tmp_kwargs_name_2);
            Py_DECREF(tmp_args_name_2);
            if (tmp_assign_source_37 == NULL) {
                assert(ERROR_OCCURRED());

                FETCH_ERROR_OCCURRED(&exception_type, &exception_value, &exception_tb);


                exception_lineno = 724;

                goto try_except_handler_6;
            }
            assert(outline_0_var___class__ == NULL);
            outline_0_var___class__ = tmp_assign_source_37;
        }
        CHECK_OBJECT(outline_0_var___class__);
        tmp_assign_source_36 = outline_0_var___class__;
        Py_INCREF(tmp_assign_source_36);
        goto try_return_handler_6;
        NUITKA_CANNOT_GET_HERE("tried codes exits in all cases");
        return NULL;
        // Return handler code:
        try_return_handler_6:;
        Py_DECREF(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724);
        locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724 = NULL;
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

        Py_DECREF(locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724);
        locals_pip$_vendor$html5lib$filters$sanitizer$$$class__1_Filter_724 = NULL;
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
        CHECK_OBJECT(outline_0_var___class__);
        Py_DECREF(outline_0_var___class__);
        outline_0_var___class__ = NULL;
        goto outline_result_1;
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

        goto outline_exception_1;
        // End of try:
        NUITKA_CANNOT_GET_HERE("Return statement must have exited already.");
        return NULL;
        outline_exception_1:;
        exception_lineno = 724;
        goto try_except_handler_4;
        outline_result_1:;
        UPDATE_STRING_DICT1(moduledict_pip$_vendor$html5lib$filters$sanitizer, (Nuitka_StringObject *)mod_consts[0], tmp_assign_source_36);
    }
    goto try_end_4;
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

    Py_XDECREF(tmp_class_creation_1__bases);
    tmp_class_creation_1__bases = NULL;
    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    Py_XDECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;
    // Re-raise.
    exception_type = exception_keeper_type_6;
    exception_value = exception_keeper_value_6;
    exception_tb = exception_keeper_tb_6;
    exception_lineno = exception_keeper_lineno_6;

    goto frame_exception_exit_1;
    // End of try:
    try_end_4:;

    // Restore frame exception if necessary.
#if 0
    RESTORE_FRAME_EXCEPTION(frame_6bde854d712dad430c9ea5c5ac22e83b);
#endif
    popFrameStack();

    assertFrameObject(frame_6bde854d712dad430c9ea5c5ac22e83b);

    goto frame_no_exception_2;

    frame_exception_exit_1:;
#if 0
    RESTORE_FRAME_EXCEPTION(frame_6bde854d712dad430c9ea5c5ac22e83b);
#endif

    if (exception_tb == NULL) {
        exception_tb = MAKE_TRACEBACK(frame_6bde854d712dad430c9ea5c5ac22e83b, exception_lineno);
    } else if (exception_tb->tb_frame != &frame_6bde854d712dad430c9ea5c5ac22e83b->m_frame) {
        exception_tb = ADD_TRACEBACK(exception_tb, frame_6bde854d712dad430c9ea5c5ac22e83b, exception_lineno);
    }

    // Put the previous frame back on top.
    popFrameStack();

    // Return the error.
    goto module_exception_exit;

    frame_no_exception_2:;
    Py_XDECREF(tmp_class_creation_1__bases);
    tmp_class_creation_1__bases = NULL;
    Py_XDECREF(tmp_class_creation_1__class_decl_dict);
    tmp_class_creation_1__class_decl_dict = NULL;
    Py_XDECREF(tmp_class_creation_1__metaclass);
    tmp_class_creation_1__metaclass = NULL;
    CHECK_OBJECT(tmp_class_creation_1__prepared);
    Py_DECREF(tmp_class_creation_1__prepared);
    tmp_class_creation_1__prepared = NULL;

    return module_pip$_vendor$html5lib$filters$sanitizer;
    module_exception_exit:
    RESTORE_ERROR_OCCURRED(exception_type, exception_value, exception_tb);
    return NULL;
}

