#include "kilate/native.h"

#include <dirent.h>
#include <dlfcn.h>

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/config.h"
#include "kilate/node.h"
#include "kilate/string.h"
#include "kilate/vector.h"

node_vector_t *native_functions = NULL;

void native_init()
{
        native_functions = vector_make(sizeof(native_function_node_t *));
        native_load_extern();
}

void native_load_extern()
{
        // Load ALL Native Libs found
        // IDEA: Add a way to specify which libraries are used
        // something like -lGL -EGL etc. to avoid loading unused libraries.
        for (size_t i = 0; i < libs_native_directories->size; i++) {
                char *dir = *(char **)vector_get(libs_native_directories, i);
                DIR *d = opendir(dir);
                if (!d)
                        return;

                struct dirent *entry;
                while ((entry = readdir(d))) {
                        if (strstr(entry->d_name, ".so")) {
                                char path[512];
                                snprintf(path, sizeof(path), "%s%s", dir,
                                         entry->d_name);

                                void *handle = dlopen(path, RTLD_NOW);
                                if (!handle) {
                                        fprintf(stderr,
                                                "Error loading %s: %s\n", path,
                                                dlerror());
                                        continue;
                                }

                                void (*extern_native_init)() =
                                    dlsym(handle, "KILATE_NATIVE_REGISTER");
                                if (!extern_native_init) {
                                        fprintf(
                                            stderr,
                                            "Function KILATE_NATIVE_REGISTER "
                                            "not found in %s\n",
                                            path);
                                        continue;
                                }
                                extern_native_init();
                        }
                }
                closedir(d);
        }
}

void native_end()
{
        for (size_t i = 0; i < native_functions->size; ++i) {
                function_node_t *entry =
                    *(function_node_t **)vector_get(native_functions, i);
                free(entry->function_n.name);
                if (entry->function_n.params != NULL)
                        vector_delete(entry->function_n.params);
                free(entry);
        }
        vector_delete(native_functions);
}

void native_register_function_node(native_function_node_t *entry)
{
        vector_push_back(native_functions, &entry);
}

void native_register_fn(const char *name, const char *return_type,
                        node_param_vector_t *params, native_fn_t fn)
{
        function_node_t *n = alloc_node(NODE_FUNCTION);
        n->function_n.name = strdup(name);
        n->function_n.return_type = strdup(return_type);
        n->function_n.params = params;
        n->function_n.native = true;
        n->function_n.native_fn = fn;
        native_register_function_node(n);
}

native_function_node_t *native_find_function(const char *name)
{
        for (size_t i = 0; i < native_functions->size; ++i) {
                native_function_node_t *entry = *(
                    native_function_node_t **)vector_get(native_functions, i);
                if (str_equals(entry->function_n.name, name)) {
                        return entry;
                }
        }
        return NULL;
}
