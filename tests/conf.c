#include <jconf/node.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    JConfNode *node =j_conf_node_new(J_CONF_NODE_DIRECTIVE,"Hello");
    j_conf_node_set_arguments(node," False \"Hello Every one\" 1d23 True dhjaeid nice     ");
    JList *ptr=j_conf_node_get_arguments(node);
    while(ptr){
        // printf("%p,%p\n",ptr,ptr->data);
        JConfData *data=(JConfData*)j_list_data(ptr);
        if(j_conf_data_is_true(data)){
            printf("TRUE ");
        }else if(j_conf_data_is_string(data)||j_conf_data_is_raw(data)){
            printf("%s ",j_conf_data_get_string(data));
        }else if(j_conf_data_is_int(data)){
            printf("%ld ",j_conf_data_get_int(data));
        }else if(j_conf_data_is_false(data)){
            printf("FALES ");
        }
        ptr=j_list_next(ptr);
    }
    printf("!\n");
    j_conf_node_free(node);
    return 0;
}
