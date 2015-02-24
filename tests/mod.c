#include <jmod/jmod.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{
    JModule *mod = j_mod_load(".", "module");
    if (mod == NULL) {
        return 1;
    }
    return 0;
}
