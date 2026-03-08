#include <kilate/native.h>
#include <kilate/string.h>
#include <kilate/vector.h>

#include "io.h"

KILATE_NATIVE_REGISTER()
{
        {
                // Register native print method
                str_vector_t *requiredParams = vector_make(sizeof(char **));
                char *any = "any";
                vector_push_back(requiredParams, &any);
                native_register_fn(EXPORT_STD_PRINT_NAME, requiredParams, std_print);
        }
        {
                // Register native system method
                str_vector_t *requiredParams = vector_make(sizeof(char **));
                char *str = "string";
                vector_push_back(requiredParams, &str);
                native_register_fn(EXPORT_STD_SYSTEM_NAME, requiredParams, std_system);
        }
        {
                // Register native system method
                str_vector_t *requiredParams = vector_make(sizeof(char **));
                char *str = "long";
                vector_push_back(requiredParams, &str);
                native_register_fn(EXPORT_STD_SLEEP_NAME, requiredParams, std_sleep);
        }
}