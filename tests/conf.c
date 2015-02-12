#include <jconf/jconf.h>
#include <stdio.h>


static inline void print_arguments(JList *args)
{
    while(args){
        JConfData *d=(JConfData*)j_list_data(args);
        if(j_conf_data_is_true(d)){
            printf("[TRUE]");
        }else if(j_conf_data_is_false(d)){
            printf("[FALSE]");
        }else if(j_conf_data_is_raw(d)){
            printf("%s",j_conf_data_get_string(d));
        }else if(j_conf_data_is_string(d)){
            printf("\"%s\"",j_conf_data_get_string(d));
        }else if(j_conf_data_is_int(d)){
            printf("%ld",j_conf_data_get_int(d));
        }
        args=j_list_next(args);
        if(args){
            printf(" ");
        }
    }
}


static inline void print_node(JConfNode* node,int tab)
{
    int i;
    for(i=0;i<tab;i++){
        printf("\t");
    }
    if(j_conf_node_is_directive(node)){
        printf("%s: ",j_conf_node_get_name(node));
        print_arguments(j_conf_node_get_arguments(node));
    }else{
        printf("<%s",j_conf_node_get_name(node));
        print_arguments(j_conf_node_get_arguments(node));
        printf(">\n");
        JList *children= j_conf_node_get_children(node);
        while(children){
            JConfNode *n=(JConfNode*)j_list_data(children);
            print_node(n,tab+1);
            children=j_list_next(children);
        }
        for(i=0;i<tab;i++){
            printf("\t");
        }
        printf("</%s>",j_conf_node_get_name(node));
    }
    printf("\n");
}


int main(int argc, char *argv[])
{
    JConfParser *p=j_conf_parser_new();
    char *error=NULL;
    if(!j_conf_parser_parse(p,"./test.conf",&error)){
        printf("%s\n",error);
        return 1;
    }
    print_node(j_conf_parser_get_root(p),0);
    j_conf_parser_free(p);
    return 0;
}
