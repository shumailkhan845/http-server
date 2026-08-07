#ifndef FORM_H
#define FORM_H

#define MAX_FORM_FIELDS 32
struct form_field
{
    char *key;
    char *value;

};

struct form_data
{
    struct form_field fields[MAX_FORM_FIELDS];
    int field_count;
};

struct form_data parse_form(char *request_body);

#endif