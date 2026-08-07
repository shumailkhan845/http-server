#include "form.h"
#include <stddef.h>
struct form_data parse_form(char *request_body)
{

    struct form_data data = {0};
    if (*request_body == '\0' || request_body == NULL)
    {
        return data;
    }
    char *p = request_body;
    char *key_start = request_body;
    char *value_start = NULL;
    size_t field_index = 0;

    p = request_body;
    value_start = p;
    key_start = p;
    while (*p != '\0')
    {
        if (field_index >= MAX_FORM_FIELDS)
        {
            break;
        }
        if (*p == '=')
        {
            *p = '\0';
            data.fields[field_index].key = key_start;
            value_start = p + 1;
            p++;
            continue;
        }
        if (*p == '&')
        {
            *p = '\0';
            data.fields[field_index].value = value_start;
            key_start = p + 1;
            field_index++;
            p++;
            continue;
        }
        p++;
    }
    data.fields[field_index].key = key_start;
    data.fields[field_index].value = value_start;
    data.field_count = field_index + 1;
    return data;
}
