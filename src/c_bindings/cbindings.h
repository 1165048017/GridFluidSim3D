#ifndef CBINDINGS_H
#define CBINDINGS_H

#include <exception>
#include <string>

#include "../vmath.h"
#include "../aabb.h"
#include "vector3_c.h"
#include "aabb_c.h"

namespace CBindings {

extern int SUCCESS;
extern int FAIL;
extern char CBINDINGS_ERROR_MESSAGE[4096];

extern void set_error_message(std::exception &ex);
extern char* get_error_message();

template<class CLASS>
void safe_execute_method(CLASS *obj, void (CLASS::*funcptr)(void), int *err) {
    *err = SUCCESS;
    try {
        (obj->*funcptr)();
    } catch (std::exception &ex) {
        CBindings::set_error_message(ex);
        *err = FAIL;
    }
}

template<typename T, class CLASS>
T safe_execute_method(CLASS *obj, T (CLASS::*funcptr)(void), int *err) {
    *err = SUCCESS;
    T result = T();
    try {
        result = (obj->*funcptr)();
    } catch (std::exception &ex) {
        CBindings::set_error_message(ex);
        *err = FAIL;
    }

    return result;
}

template<typename T, typename RT, class CLASS>
RT safe_execute_method(CLASS *obj, RT (CLASS::*funcptr)(T), T param, int *err) {
    *err = SUCCESS;
    RT result = RT();
    try {
        result = (obj->*funcptr)(param);
    } catch (std::exception &ex) {
        CBindings::set_error_message(ex);
        *err = FAIL;
    }

    return result;
}

template<typename T, class CLASS>
void safe_execute_method(CLASS *obj, void (CLASS::*funcptr)(T), T param, int *err) {
    *err = SUCCESS;
    try {
        (obj->*funcptr)(param);
    } catch (std::exception &ex) {
        CBindings::set_error_message(ex);
        *err = FAIL;
    }
}

template<typename T, class CLASS>
void safe_execute_method(CLASS *obj,
                         void (CLASS::*funcptr)(T, T, T),
                         T param1, T param2, T param3, int *err) {
    *err = SUCCESS;
    try {
        (obj->*funcptr)(param1, param2, param3);
    } catch (std::exception &ex) {
        CBindings::set_error_message(ex);
        *err = FAIL;
    }
}

template<typename T, class RT, class CLASS>
RT safe_execute_method(CLASS *obj,
                       RT (CLASS::*funcptr)(T, T, T),
                       T param1, T param2, T param3, int *err) {
    RT result;
    *err = SUCCESS;
    try {
        result = (obj->*funcptr)(param1, param2, param3);
    } catch (std::exception &ex) {
        CBindings::set_error_message(ex);
        *err = FAIL;
    }

    return result;
}

Vector3_t to_struct(vmath::vec3 v);
vmath::vec3 to_class(Vector3_t v);
AABB_t to_struct(AABB b);
AABB to_class(AABB_t v);

}

#endif