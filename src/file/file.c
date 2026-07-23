#include "file.h"
#include <stdio.h>
#include <stdlib.h>
char *read_file(const char *filepath)
{
    FILE *f_ptr = fopen(filepath, "r");
    if (f_ptr == NULL)
    {
        return NULL;
    }
    fseek(f_ptr, 0, SEEK_END);
    long file_size = ftell(f_ptr);
    if (file_size < 0)
    {
        fclose(f_ptr);
        return NULL;
    }
    rewind(f_ptr);

    char *content = malloc(file_size + 1);
    if (content == NULL)
    {
        perror("Memory allocation failed");
        fclose(f_ptr);
        return NULL;
    }
    int bytesRead = fread(content, 1, file_size, f_ptr);
    if (bytesRead != file_size)
    {
        free(content);
        fclose(f_ptr);
        return NULL;
    }
    content[bytesRead] = '\0';
    fclose(f_ptr);
    return content;
}