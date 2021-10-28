#include <stdio.h>
#include "VSFS.h"


int main() {
    //printf("Starting\n");
    //create_vsfs();
    
    //printf("Write complete\n");
    //printf("About to load\n");
    load_vsfs("FS");
    


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