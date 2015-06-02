#include <jlib/jlib.h>


int main(int argc, char *argv[])
{
    JQuark q0 = j_quark_try_string("nice");
    JQuark q1 = j_quark_from_string("nice");
    JQuark q2 = j_quark_from_static_string("nice");
    if (q1 != q2 || q0 != 0) {
        return -1;
    }
    j_printf("%d:%d:%d\n", q0, q1, q2);
    return 0;
}
