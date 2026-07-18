#ifndef COMPILER_H
#define COMPILER_H

#include "editor.h"

typedef struct {
    char* output;
    char* errors;
    int exit_code;
    int compile_time;
    int success;
} CompileResult;

typedef struct {
    char* compiler_path;
    char** flags;
    int flag_count;
    char* output_dir;
} CompilerConfig;

CompilerConfig* compiler_init(void);
void compiler_free(CompilerConfig* config);
CompileResult* compiler_compile(CompilerConfig* config, const char* source_file);
void compiler_run(CompilerConfig* config, const char* executable);
void compiler_render_output(CompileResult* result, int x, int y, int width, int height);
int compiler_detect_language(const char* filename);

#endif