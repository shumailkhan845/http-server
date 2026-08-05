#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct file_data read_file(const char *filepath)
{
    printf("I am reading the file...1\n\n");
    struct file_data file = {0};
    printf("I am reading the file...2\n\n");
    FILE *f_ptr = fopen(filepath, "rb");
    printf("I am reading the file...3\n\n");

    if (f_ptr == NULL)
    {
        return file;
    }
    printf("I am reading the file...3\n\n");

    fseek(f_ptr, 0, SEEK_END);
    long file_size = ftell(f_ptr);
    if (file_size < 0)
    {
        fclose(f_ptr);
        return file;
    }
    rewind(f_ptr);

    char *content = malloc(file_size + 1);
    if (content == NULL)
    {
        perror("Memory allocation failed");
        fclose(f_ptr);
        return file;
    }
    size_t bytesRead = fread(content, 1, file_size, f_ptr);
    if (bytesRead != (size_t)file_size)
    {
        free(content);
        fclose(f_ptr);
        return file;
    }
    content[bytesRead] = '\0';
    fclose(f_ptr);
    file.data = content;
    file.size = file_size;
    printf("File size = %ld\n", file_size);
    printf("Bytes read = %zu\n", bytesRead);
    return file;
}