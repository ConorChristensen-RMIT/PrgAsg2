// sprimary_block will contain information about the number of
// blocks and inodes, along with the size of the disk blocks.
#include <stdio.h>
#include <time.h>

#define BLOCK_SIZE 4096
#define NAME_LEN 12
#define MAX_DIR_NAME 256
#define MAX_INODES 32
#define MAX_DIR_LEVEL 4
#define MAX_SUBDIR 6
#define FILE_ATTR "-rw-rw-r--"
#define DIR_ATTR "drw-rw-r--"
#define OWNER_STR "conor"
#define NAME_STR "conor"

struct primary_block {
    int max_num_dirs;
    int home_dir;
    int size;
    int num_inodes;
    int num_blocks;
    int size_blocks;
};

struct directory{
    char name[NAME_LEN];
    int size;
    int parent;
    int children[MAX_SUBDIR];
    char owner[NAME_LEN];
    int inodes[MAX_INODES];
    char path[MAX_DIR_LEVEL * NAME_LEN + MAX_DIR_LEVEL];
};

struct inode {
    int size;
    int first_block;
    int parent_dir;
    // Putting a limit on the number of hard links to inode
    int linked_dirs[MAX_SUBDIR];
    time_t time_created;
    char attribute[10];
    char owner[NAME_LEN];
    char name[NAME_LEN];
    char group[NAME_LEN];
};

struct block {
    int next_block_num;
    // Block size of 4Kb
    char data[BLOCK_SIZE];

};


void create_vsfs ();
void load_vsfs(char * filename);  
void write_vsfs(char * filename);
void list(char * file_name);
void mkdir(char * FSFilename, char* name);
void rmdir(char * FSFilename, char* pathname);
void pretty_print();

// Will return a file number
int allocate_file (char name[NAME_LEN]);
void def_file_size(int file_id, int size);
void write_byte (int file_id, int pos, char *data);
void addFile(char * FSFile, char * filename_in, char * filename_ex);
void copyout(char * FSFile, char * filename, char * filename_ex);
const char* read_byte(int file_id, int pos);
void rm(char * FSFilename, char * filename_in);
void clear_inode(int inode_num);
int get_free_inode();
int get_free_block();
void delete_dir(int curr_dir_num);

