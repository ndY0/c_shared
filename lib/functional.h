

#define lambda(return_type, function_body) \
    ({                                     \
        return_type __fn__ function_body   \
            __fn__;                        \
    })
#define f(...) var_f((f_args){__VA_ARGS__})