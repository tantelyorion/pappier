#include "git.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static char* exec_command(const char* cmd, const char* cwd) {
    char buffer[4096];
    char* result = NULL;
    size_t total = 0;
    
    char full_cmd[1024];
    if (cwd) {
        snprintf(full_cmd, sizeof(full_cmd), "cd %s && %s", cwd, cmd);
    } else {
        strcpy(full_cmd, cmd);
    }
    
    FILE* pipe = popen(full_cmd, "r");
    if (!pipe) return NULL;
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        size_t len = strlen(buffer);
        result = realloc(result, total + len + 1);
        strcpy(result + total, buffer);
        total += len;
    }
    
    pclose(pipe);
    return result;
}

GitStatus* git_init(const char* path) {
    GitStatus* status = malloc(sizeof(GitStatus));
    status->repository_path = strdup(path);
    status->current_branch = NULL;
    status->branches = NULL;
    status->branch_count = 0;
    status->modified_files = NULL;
    status->modified_count = 0;
    status->staged_files = NULL;
    status->staged_count = 0;
    status->untracked_files = NULL;
    status->untracked_count = 0;
    status->has_changes = 0;
    
    git_refresh(status);
    return status;
}

void git_free(GitStatus* status) {
    if (!status) return;
    if (status->repository_path) free(status->repository_path);
    if (status->current_branch) free(status->current_branch);
    if (status->branches) {
        for (int i = 0; i < status->branch_count; i++) free(status->branches[i]);
        free(status->branches);
    }
    if (status->modified_files) {
        for (int i = 0; i < status->modified_count; i++) free(status->modified_files[i]);
        free(status->modified_files);
    }
    if (status->staged_files) {
        for (int i = 0; i < status->staged_count; i++) free(status->staged_files[i]);
        free(status->staged_files);
    }
    if (status->untracked_files) {
        for (int i = 0; i < status->untracked_count; i++) free(status->untracked_files[i]);
        free(status->untracked_files);
    }
    free(status);
}

int git_is_repository(const char* path) {
    char git_dir[1024];
    snprintf(git_dir, sizeof(git_dir), "%s/.git", path);
    struct stat st;
    return stat(git_dir, &st) == 0;
}

int git_refresh(GitStatus* status) {
    if (!git_is_repository(status->repository_path)) return -1;
    
    // Obtenir la branche actuelle
    char* branch_output = exec_command("git branch --show-current", status->repository_path);
    if (branch_output) {
        if (status->current_branch) free(status->current_branch);
        status->current_branch = strdup(branch_output);
        branch_output[strcspn(branch_output, "\n")] = '\0';
        free(branch_output);
    }
    
    // Obtenir toutes les branches
    char* branches_output = exec_command("git branch -a", status->repository_path);
    if (branches_output) {
        if (status->branches) {
            for (int i = 0; i < status->branch_count; i++) free(status->branches[i]);
            free(status->branches);
        }
        status->branches = NULL;
        status->branch_count = 0;
        
        char* line = strtok(branches_output, "\n");
        while (line) {
            // Ignorer les branches distantes
            if (strncmp(line, "  remotes/", 10) != 0) {
                char* name = line;
                if (name[0] == '*') name += 2;
                else if (name[0] == ' ') name += 1;
                
                status->branches = realloc(status->branches, sizeof(char*) * (status->branch_count + 1));
                status->branches[status->branch_count++] = strdup(name);
            }
            line = strtok(NULL, "\n");
        }
        free(branches_output);
    }
    
    // Obtenir les fichiers modifiés
    char* modified_output = exec_command("git status --porcelain", status->repository_path);
    if (modified_output) {
        if (status->modified_files) {
            for (int i = 0; i < status->modified_count; i++) free(status->modified_files[i]);
            free(status->modified_files);
        }
        if (status->staged_files) {
            for (int i = 0; i < status->staged_count; i++) free(status->staged_files[i]);
            free(status->staged_files);
        }
        if (status->untracked_files) {
            for (int i = 0; i < status->untracked_count; i++) free(status->untracked_files[i]);
            free(status->untracked_files);
        }
        status->modified_files = NULL;
        status->modified_count = 0;
        status->staged_files = NULL;
        status->staged_count = 0;
        status->untracked_files = NULL;
        status->untracked_count = 0;
        
        char* line = strtok(modified_output, "\n");
        while (line) {
            if (strlen(line) >= 2) {
                char status_code = line[0];
                char* filename = line + 3; // Skip "?? " ou " M "
                
                if (status_code == 'M' || status_code == ' ') {
                    // Modifié
                    status->modified_files = realloc(status->modified_files, 
                        sizeof(char*) * (status->modified_count + 1));
                    status->modified_files[status->modified_count++] = strdup(filename);
                } else if (status_code == 'A') {
                    // Ajouté
                    status->staged_files = realloc(status->staged_files, 
                        sizeof(char*) * (status->staged_count + 1));
                    status->staged_files[status->staged_count++] = strdup(filename);
                } else if (status_code == '?') {
                    // Non suivi
                    status->untracked_files = realloc(status->untracked_files, 
                        sizeof(char*) * (status->untracked_count + 1));
                    status->untracked_files[status->untracked_count++] = strdup(filename);
                }
            }
            line = strtok(NULL, "\n");
        }
        free(modified_output);
    }
    
    status->has_changes = status->modified_count > 0 || status->staged_count > 0 || status->untracked_count > 0;
    return 0;
}

int git_add(GitStatus* status, const char* file) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "git add %s", file);
    char* output = exec_command(cmd, status->repository_path);
    free(output);
    return git_refresh(status);
}

int git_commit(GitStatus* status, const char* message) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "git commit -m \"%s\"", message);
    char* output = exec_command(cmd, status->repository_path);
    free(output);
    return git_refresh(status);
}

int git_branch(GitStatus* status, const char* name) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "git branch %s", name);
    char* output = exec_command(cmd, status->repository_path);
    free(output);
    return git_refresh(status);
}

int git_checkout(GitStatus* status, const char* branch) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "git checkout %s", branch);
    char* output = exec_command(cmd, status->repository_path);
    free(output);
    return git_refresh(status);
}

int git_pull(GitStatus* status) {
    char* output = exec_command("git pull", status->repository_path);
    free(output);
    return git_refresh(status);
}

int git_push(GitStatus* status) {
    char* output = exec_command("git push", status->repository_path);
    free(output);
    return git_refresh(status);
}

void git_render_status(GitStatus* status, int x, int y) {
    if (!status) return;
    
    attron(A_BOLD);
    mvprintw(y, x, "🔀 Git Status");
    attroff(A_BOLD);
    
    int line = y + 1;
    
    if (status->current_branch) {
        attron(COLOR_PAIR(2));
        mvprintw(line++, x, "  Branche: %s", status->current_branch);
        attroff(COLOR_PAIR(2));
    }
    
    if (status->modified_count > 0) {
        attron(COLOR_PAIR(3));
        mvprintw(line++, x, "  📝 Modifiés:");
        for (int i = 0; i < status->modified_count && i < 5; i++) {
            mvprintw(line++, x, "    %s", status->modified_files[i]);
        }
        attroff(COLOR_PAIR(3));
    }
    
    if (status->staged_count > 0) {
        attron(COLOR_PAIR(4));
        mvprintw(line++, x, "  ✅ En staging:");
        for (int i = 0; i < status->staged_count && i < 5; i++) {
            mvprintw(line++, x, "    %s", status->staged_files[i]);
        }
        attroff(COLOR_PAIR(4));
    }
    
    if (status->untracked_count > 0) {
        attron(COLOR_PAIR(5));
        mvprintw(line++, x, "  ❓ Non suivis:");
        for (int i = 0; i < status->untracked_count && i < 5; i++) {
            mvprintw(line++, x, "    %s", status->untracked_files[i]);
        }
        attroff(COLOR_PAIR(5));
    }
    
    if (!status->has_changes) {
        mvprintw(line++, x, "  ✅ Aucun changement");
    }
}