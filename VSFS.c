#include "VSFS.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

// Code won't actually run, I keep getting segmentation faults and it is almost
// like a black box. I have run out of time trying to find the errors and also cannot
// work out how to run the VSFS executable without "./VSFS". The architecture
// for the file system is all in place, however.
//
struct primary_block pr_bl;
struct directory *directories;
struct inode *inodes;
struct block *blocks;

int main(int argc, char* argv[]){
    if (strcmp(argv[1], "list")){
        list(argv[2]);
    }
    else if (strcmp(argv[1], "copyin")){
        copyin(argv[2], argv[4], argv[3]);
    }
    else if (strcmp(argv[1], "mkdir")){
        mkdir(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "copyout")){
        copyout(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "rm")){
        rm(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "rmdir")){
        rmdir(argv[2], argv[3]);
    }    
    return 0;
}

void create_vsfs () {
    // create directory array. Max number of directories is the maximum
    // number of subdirectories to the power of the maximum depth of the 
    // directory tree.
    pr_bl.max_num_dirs = pow(MAX_SUBDIR, MAX_DIR_LEVEL);
    pr_bl.home_dir = 0;
    pr_bl.size = 0;
    pr_bl.num_inodes = 1024;
    pr_bl.num_blocks = 12 * pr_bl.num_inodes;
    pr_bl.size_blocks = BLOCK_SIZE;

    int n;
    int p;
    // Create space for the inodes
    inodes = malloc (sizeof (struct inode) * pr_bl.num_inodes);
    for (n=0 ; n < pr_bl.num_inodes; n++){
        // inode not yet allocated, so set size = -1
        inodes[n].first_block = -1;
        inodes[n].size = -1;
        inodes[n].parent_dir = -1;
        strcpy(inodes[n].name, "");
        strcpy(inodes[n].owner, "");
        strcpy(inodes[n].attribute, "");
        // initialise inode hard links to -1
        for (p=0 ; n < (sizeof (inodes[n].linked_dirs) / sizeof(int)) ; p++){
            inodes[n].linked_dirs[p] = -1;
        }
    }
    
    
    // Create space for the blocks 
    blocks = malloc (sizeof (struct block) * pr_bl.num_blocks);
    for (n=0 ; n < pr_bl.num_blocks; n++){
        // initialise disk blocks, no next block yet
        blocks[n].next_block_num = -1;
    }

    directories = malloc (sizeof (struct directory) * pr_bl.max_num_dirs);
    for (n=0 ; n < pr_bl.max_num_dirs; n++){
        directories[n].size = -1;
        // initialise child dirs to -1
        for (p=0; n < (sizeof (directories[n].children) / sizeof(int)); p++){
            directories[n].children[p] = -1;
        }
        //initialise the inodes as -1
        for (p=0 ; p < MAX_INODES ; p++){
            directories[n].inodes[p] = -1;
        }
        directories[n].parent = -1;
        strcpy(directories[n].path, "");
        strcpy(directories[n].name, "");
    }

    strcpy(directories[0].name, "~");
    directories[0].size = 0;
}


void write_vsfs (char * filename) {
    // Save the state of the running program to the FS
    FILE *file;
    file = fopen(filename, "w+");
    
    // Write in the primary block
    fwrite(&(pr_bl), sizeof(struct primary_block), 1, file );
    
    fwrite(inodes, sizeof (struct inode), pr_bl.num_inodes, file);
    
    // Write blocks
    fwrite(blocks, sizeof (struct block), pr_bl.num_blocks, file);
    
    fwrite(directories, sizeof (struct directory), pr_bl.max_num_dirs, file);

    fclose(file);
}

void load_vsfs (char * filename) {
    // Load the FS state into the program
    FILE *file;
    file = fopen(filename, "r");

    // read the primary block first
    fread(&pr_bl, sizeof ( struct primary_block), 1, file);

    //allocate some memory for the inodes and blocks
    inodes = malloc ( sizeof (struct inode) * pr_bl.num_inodes);
    blocks = malloc ( sizeof (struct block) * pr_bl.num_blocks);

    // read into the allocated memory the stored data
    fread(inodes, sizeof(struct inode), pr_bl.num_inodes, file);
    fread(blocks, sizeof(struct block), pr_bl.num_blocks, file);

    fclose(file);
}

int allocate_file (char name[NAME_LEN]){
    // Find a free inode and a free first-block to allocate to an
    // incoming file.
    int free_inode = get_free_inode();
    int free_block = get_free_block();
    inodes[free_inode].first_block = free_block;
    blocks[free_block].next_block_num = -2;

    strcpy(inodes[free_inode].name, name);

    return free_inode;
}


int get_free_dir_slot(){
    // Return the index value for the directory
    int val = -1;
    int n;
    for (n = 0; n < pr_bl.max_num_dirs ; n++){
        if (directories[n].size == -1){
            val = n;
            break;
        }
    }
    return val;
}


void printInode(int inode_num){
    //
    struct inode ind = inodes[inode_num];
    int n;
    int linked = 0;
    for (n = 0 ; n < MAX_SUBDIR ; n++){
        if (ind.linked_dirs[n] >=0 ){
            linked++;
        }
    }
    char path_name[MAX_DIR_LEVEL * NAME_LEN] = "";
    strcat(path_name, directories[ind.parent_dir].path);
    strcat(path_name, "/");
    strcat(path_name, directories[ind.parent_dir].name);
    strcat(path_name, "/");
    struct tm * dtm = localtime(&ind.time_created);
    // Attribute LinkedSubdirs Owner Group Size DateTime Path/Filename
    printf("%-10s %3d %-12s %-12s %7d %02d/%02d/%04d %02d:%02d %s\n", ind.attribute, linked, ind.owner, ind.group, ind.size, dtm->tm_mday, dtm->tm_mon, dtm->tm_year, dtm->tm_hour, dtm->tm_min, path_name);
}

void listDirContents(int dir_num){
    if (directories[dir_num].size == -1) {return;}
    int n;
    for (n = 0 ; n < MAX_INODES ; n++){
        if (directories[dir_num].inodes[n] == -1){break;}
        printInode(directories[dir_num].inodes[n]);
    }
    // List contents of all child directories
    for ( n=0 ; n < MAX_SUBDIR ; n++){
        if (directories[dir_num].children[n] >= 0){
            listDirContents(directories[dir_num].children[n]);
        }
    }

}


void copyin(char * FSFile, char * filepath_in, char * filename_ex){
    load_vsfs(FSFile);
    
    // Set original parent directory
    char * parent_token = "";
    strcpy(parent_token, "~/");
    //split up the provided path to find parent directory
    char * filename_in = "";
    strcpy(filename_in, filepath_in);
    char * token = strtok(filepath_in, "/");

    //In initial case:
    if (token != NULL){
        strcpy(filename_in, token);
    }
    //Then loop:
    while (token != NULL){
        strcpy(parent_token, filename_in);
        strcpy(filename_in, token);
        token = strtok(NULL, "/");
    }
    //Find the directory int that matches the parent directory name
    int p_dir_int = -1;
    //If home directory
    if (strcmp(parent_token, "~/")) {p_dir_int = 0;}
    else{
        int i;
        for (i=0 ; i < pr_bl.max_num_dirs ; i++){
            if (strcmp(parent_token, directories[i].name)){
                p_dir_int = i;
            }
        }
    }
    //If no directory with the name exists, exit
    if (p_dir_int == -1){return;}
    
    int ind_int = allocate_file(filepath_in);
    //Open the file in binary read
    FILE *file = fopen(filename_ex, "rb");
    //Ensure pointer points to the start of the file
    fseek(file, 0, SEEK_SET);
    
    // Create a byte buffer
    char *byte;
    long file_len = ftell(file);
    directories[p_dir_int].size += file_len;
    inodes[ind_int].size = file_len + 1;
    strcpy(inodes[ind_int].attribute, FILE_ATTR);
    inodes[ind_int].time_created = time(NULL);
    strcpy(inodes[ind_int].owner, OWNER_STR);
    strcpy(inodes[ind_int].name, NAME_STR);
    inodes[ind_int].linked_dirs[0] = p_dir_int;
    //set parent directory
    inodes[ind_int].parent_dir = p_dir_int;
    // Allocate and ready the blocks to be used
    // Add an extra byte for the "\0" character at the end
    def_file_size(ind_int, file_len + 1);
    //Set byte size and space in memory
    byte = (char *)malloc(sizeof(char));

    long n;

    for (n = 0 ; n < file_len ; n++){
        //set the file pointer to the offset
        fseek(file, n, SEEK_SET);
        // write byte to the byte buffer
        fread(byte, 1, 1, file);
        //write the contents of the byte to the storage
        write_byte(ind_int, n, byte);
    }
    // Write special ending character
    write_byte(ind_int, n+1, "\0");

    write_vsfs(FSFile);

    fclose(file);
    return;
}


void copyout(char * FSFile, char * filename_in, char * filename_ex){
    load_vsfs(FSFile);
    char *contents;
    int n;
    int p;
    
    for ( n=0 ; n < pr_bl.num_inodes ; n++){
        if (strcmp(inodes[n].name, filename_in)){
            contents = (char *)malloc(inodes[n].size);
            for (p=0 ; p < inodes[n].size ; p++){
                // concatonate the read byte to the contents buffer
                strcat(contents, read_byte(n, p));     
            }
            FILE *out_file = fopen(filename_ex, "w");
            fwrite(contents, 1, sizeof(contents), out_file);
            fclose(out_file);
            break;
        }
    }
}


void list(char * file_name){
    
    load_vsfs(file_name);
    int n;
    
    //Go through all the directories and all their inodes
    for ( n=0 ; n < pr_bl.max_num_dirs ; n++){
        listDirContents(n);
    }
}


void mkdir(char * FSFilename, char* pathname){
    load_vsfs(FSFilename);
    // Set original parent directory
    char * parent_token = "";
    strcpy(parent_token, "~/");
    //split up the provided path to find parent directory
    char * dirname = "";
    strcpy(dirname, pathname);
    char * token = strtok(pathname, "/");

    //In initial case:
    if (token != NULL){
        strcpy(dirname, token);
    }
    //Then loop:
    while (token != NULL){
        strcpy(parent_token, dirname);
        strcpy(dirname, token);
        token = strtok(NULL, "/");
    }
    //Find the directory int that matches the parent directory name
    int p_dir_int = -1;
    //If home directory
    if (strcmp(parent_token, "~/")) {p_dir_int = 0;}
    else{
        int i;
        for (i=0 ; i < pr_bl.max_num_dirs ; i++){
            if (strcmp(parent_token, directories[i].name)){
                p_dir_int = i;
            }
        }
    }
    //If no directory with the name exists, exit
    if (p_dir_int == -1){return;}
    //else, set the current directory id
    int curr_dir_id = p_dir_int;

    
    int n;
    int val = -1;
    for (n=0 ; n < (sizeof (directories[curr_dir_id].children) / sizeof (int)) ; n++){
        if (directories[curr_dir_id].children[n] == -1){
            val = n;
        }
    }
    //If the maximum number of children directories is exceeded.
    if (val == -1) {
        printf("The maximum number of directories has been exceeded.\n");
        return;
    }

    int next_dir_num = get_free_dir_slot();
    directories[next_dir_num].size = 0;
    // Cope the directory name of the parent directory as the name
    strcpy(directories[next_dir_num].name, directories[curr_dir_id].name);
    // Concatonate the path '/' and the new directory name
    strcat(directories[next_dir_num].name, "/");
    strcat(directories[next_dir_num].name, dirname);
    directories[next_dir_num].parent = curr_dir_id;
    strcpy(directories[next_dir_num].owner, OWNER_STR);
    write_vsfs(FSFilename);
}


void rm(char * FSFilename, char * filename_in){
    load_vsfs(FSFilename);
    int n;
    for (n = 0 ; n < pr_bl.num_inodes ; n++){
        if (strcmp(inodes[n].name, filename_in)){
            clear_inode(n);
        }
    }
}


void rmdir(char * FSFilename, char * pathname){
    load_vsfs(FSFilename);
    // Set original parent directory
    char * parent_token = "";
    strcpy(parent_token, "~/");
    //split up the provided path to find parent directory
    char * dirname = "";
    strcpy(dirname, pathname);
    char * token = strtok(pathname, "/");

    //In initial case:
    if (token != NULL){
        strcpy(dirname, token);
    }
    //Then loop:
    while (token != NULL){
        strcpy(parent_token, dirname);
        strcpy(dirname, token);
        token = strtok(NULL, "/");
    }
    //Find the directory int that matches the parent directory name
    int p_dir_int = -1;
    //If home directory
    if (strcmp(parent_token, "~/")) {p_dir_int = 0;}
    else{
        int i;
        for (i=0 ; i < pr_bl.max_num_dirs ; i++){
            if (strcmp(parent_token, directories[i].name)){
                p_dir_int = i;
            }
        }
    }
    //If no directory with the name exists, exit
    if (p_dir_int == -1){return;}
    //else, set the current directory id
    int curr_dir_id = p_dir_int;

    delete_dir(curr_dir_id);
}


void delete_dir(int curr_dir_num){
    int n;
    for (n = 0 ; n < MAX_INODES ; n++){
        clear_inode(directories[curr_dir_num].inodes[n]);
        directories[curr_dir_num].inodes[n] = -1;
    }

    for (n=0 ; n < MAX_SUBDIR ; n++){
        if (directories[curr_dir_num].children[n] != -1){
            delete_dir(directories[curr_dir_num].children[n]);
        }
    }

    strcpy(directories[curr_dir_num].owner, "");
    directories[curr_dir_num].parent = -1;
    strcpy(directories[curr_dir_num].path, "");
    directories[curr_dir_num].size = -1;
    strcpy(directories[curr_dir_num].name, "");
}


void clear_inode(int inode_num){
    int next;
    int block_num = inodes[inode_num].first_block;
    while (blocks[block_num].next_block_num != -2){
        next = blocks[block_num].next_block_num;
        // Free up the block
        // Don't have to get rid of the data, without next_block it
        // should be unintelligable
        blocks[block_num].next_block_num = -1;
        block_num = next;
    }
    
    blocks[block_num].next_block_num = -1;
    inodes[inode_num].size = -1;
    strcpy(inodes[inode_num].attribute, "");
    inodes[inode_num].time_created = -1;
    strcpy(inodes[inode_num].owner, "");
    strcpy(inodes[inode_num].name, "");
    int n;
    for (n =0 ; n < MAX_SUBDIR ; n++){
        inodes[inode_num].linked_dirs[n] = -1;
    }   
    strcpy(inodes[inode_num].name, "");
}



int get_free_inode(){
    // If no free inode the index is set to -1
    int val = -1;
    int n;
    for (n=0 ; n < pr_bl.num_inodes ; n++){
        if (inodes[n].first_block == -1){
            // Check if the first block of each inode is empty or not
            // If so, return the inode index
            val = n;
            break;
        }
    }
    return val;
}

// Find a free block
int get_free_block(){
    int val = -1;
    int n;

    for (n=0 ; n < pr_bl.num_blocks ; n++){
        if (blocks[n].next_block_num == -1){
            val = n;
            break;
        }
    }
    return val;
}


void def_file_size(int file_id, int size){
    // How many blocks do we need?
    int num = size/BLOCK_SIZE;

    // Round up if not an integer division
    if (size % BLOCK_SIZE != 0) {num++;}
    
    int block_num = inodes[file_id].first_block;
    
    // First block set, num --
    num--;

    while (num > 0){        
        //Keep finding free blocks, and the last block will always have next
        // block being -2. -2 signifies the block is full but has no next.
        if (blocks[block_num].next_block_num == -2){
            int empty_block = get_free_block();
            blocks[block_num].next_block_num = empty_block;
            blocks[empty_block].next_block_num = -2;
        }
        block_num = blocks[block_num].next_block_num;
        num--;
    }
    
    blocks[block_num].next_block_num = -2;
}


int get_block_num(int file_id, int offset){
    int remaining = offset;
    int block_num = inodes[file_id].first_block;

    while (remaining > 0){
        block_num = blocks[block_num].next_block_num;
        remaining--;
    }

    return block_num;
}




void write_byte (int file_id, int pos, char *data){
    // Find out which block
    int relative_block = pos/BLOCK_SIZE;
    // Get actual block number
    int block_num = get_block_num(file_id, relative_block);
    // Find the byte offset within the block
    int offset = pos % BLOCK_SIZE;
    // Write (Overwrite)
    blocks[block_num].data[offset] = (*data);
}

const char* read_byte(int file_id, int pos){
    char *byte;
    // Find out which block
    int relative_block = pos/BLOCK_SIZE;
    // Get actual block number
    int block_num = get_block_num(file_id, relative_block);
    // Find the byte offset within the block
    int offset = pos % BLOCK_SIZE;
    // get the address of the byte to be returned
    byte = &blocks[block_num].data[offset];
    return byte;
}



