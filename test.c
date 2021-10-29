#include <stdio.h>
#include "VSFS.h"


int main() {
    create_vsfs();
    printf("FS Created.");
    load_vsfs("FS");
    //write more
    write_vsfs("FS");

    list("FS");

    //allocate_file("Test");

    
    def_file_size(0, 10000);
    
    char data = 'a';

    int i;
    for (i=0 ; i<49 ; i++){
        write_byte(0, i*100, &data);
    }

    pretty_print();

    //printf("About to write\n");
    write_vsfs("FS");
    printf(" Write Complete\n");
    
    return 0;
}