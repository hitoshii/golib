#include <jlib/jlib.h>

J_PRIVATE_DEFINE_STATIC(private, NULL);

int main(int argc, char *argv[])
{
    J_MUTEX_DEFINE(lock);

    j_mutex_lock(&lock);
    j_mutex_unlock(&lock);

    j_mutex_clear(&lock);

    jpointer data = j_private_get(&private);
    if (data != NULL) {
        return 1;
    }
    j_private_set(&private, (jpointer) 1);
    data = j_private_get(&private);
    if (data != (jpointer) 1) {
        return 1;
    }
    return 0;
}
