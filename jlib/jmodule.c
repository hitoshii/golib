/*
 * Copyright (C) 2015 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */

#include "jmodule.h"
#include "jstrfuncs.h"
#include "jmem.h"
#include "jfileutils.h"

struct _JModule {
    void *handle;
    jchar *path;
};

static inline JModule *j_module_alloc(jchar *path, void *handle) {
    JModule *mod=(JModule*)j_malloc(sizeof(JModule));
    mod->handle=handle;
    mod->path=path;
    return mod;
}

JModule *j_module_open(const jchar *filepath, JModuleFlags flags) {
    jchar *path;
    void *handle=NULL;
    if(j_file_test(filepath, J_FILE_TEST_IS_REGULAR)) {
        handle=dlopen(filepath, flags);
    }

    if(handle==NULL) {
        path=j_strconcat(filepath, "." J_MODULE_SUFFIX,NULL);
        if(j_file_test(path, J_FILE_TEST_EXISTS)) {
            handle=dlopen(path, flags);
        }
    } else {
        path=j_strdup(filepath);
    }
    if(handle==NULL) {
        j_free(path);
        return NULL;
    }

    return j_module_alloc(path, handle);
}

const jchar *j_module_name(JModule *mod) {
    return mod->path;
}

jboolean j_module_symbol(JModule *module, const jchar *symbol_name, jpointer *symbol) {
    dlerror();
    *symbol=dlsym(module->handle, symbol_name);
    if(dlerror()!=NULL) {
        return FALSE;
    }
    return TRUE;
}


void j_module_close(JModule *module) {
    dlclose(module->handle);
    j_free(module->path);
    j_free(module);
}
