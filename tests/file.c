#include <jio/jfile.h>
#include <jlib/jlib.h>
#include <stdio.h>


int main(int argc,char *argv[])
{
    JFile *file=j_file_new_for_path("/home/wiky/Documents/CODE/Git/jlib/tests/file.c");
    char *data=j_file_readall(file);
    printf("%s",data);
    j_file_free(file);
    if(data==NULL){
        return 1;
    }
    j_free(data);
    return 0;
}
