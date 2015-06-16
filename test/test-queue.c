#include <jlib/jlib.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{
    JQueue *queue = j_queue_new();
    if (!j_queue_is_empty(queue)) {
        return 1;
    }
    j_queue_push_tail(queue, "first");
    j_queue_push_tail(queue, "second");
    if (j_queue_is_empty(queue) || j_queue_get_length(queue) != 2) {
        return 2;
    }
    if (j_strcmp0(j_queue_peek_nth(queue, 0), "first") ||
        j_strcmp0(j_queue_peek_tail(queue), "second")) {
        return 3;
    }
    j_queue_free(queue);

    JQueue q;
    j_queue_init(&q);
    if (!j_queue_is_empty(&q)) {
        return 1;
    }
    j_queue_push_nth(&q, j_strdup("last"), 3);
    j_queue_push_nth(&q, j_strdup("first"), 0);
    j_queue_push_nth(&q, j_strdup("second"), 1);
    if (j_queue_is_empty(&q) || j_queue_get_length(&q) != 3) {
        return 2;
    }
    if (j_strcmp0(j_queue_peek_nth(&q, 0), "first") ||
        j_strcmp0(j_queue_peek_tail(&q), "last")) {
        return 3;
    }
    j_queue_clear_full(&q, j_free);
    return 0;
}
