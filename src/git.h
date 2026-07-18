#ifndef GIT_H
#define GIT_H

#include "editor.h"

typedef struct {
    char* repository_path;
    char* current_branch;
    char** branches;
    int branch_count;
    char** modified_files;
    int modified_count;
    char** staged_files;
    int staged_count;
    char** untracked_files;
    int untracked_count;
    int has_changes;
} GitStatus;

GitStatus* git_init(const char* path);
void git_free(GitStatus* status);
int git_is_repository(const char* path);
int git_refresh(GitStatus* status);
void git_status(GitStatus* status);
int git_add(GitStatus* status, const char* file);
int git_commit(GitStatus* status, const char* message);
int git_branch(GitStatus* status, const char* name);
int git_checkout(GitStatus* status, const char* branch);
int git_pull(GitStatus* status);
int git_push(GitStatus* status);
void git_render_status(GitStatus* status, int x, int y);

#endif