#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "filesys.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define number_of_blocks 128000/64
static int filesys_inited = 0;

/* returns 20 bytes unique hash of the buffer (buf) of length (len)
 * in input array sha1.
 */
void get_sha1_hash (const void *buf, int len, const void *sha1)
{
	SHA1 ((unsigned char*)buf, len, (unsigned char*)sha1);
}

/* Build an in-memory Merkle tree for the file.
 * Compare the integrity of file with respect to
 * root hash stored in secure.txt. If the file
 * doesn't exist, create an entry in secure.txt.
 * If an existing file is going to be truncated
 * update the hash in secure.txt.
 * returns -1 on failing the integrity check.
 */
int file_descriptior_error(int fd1,const char *pathname){
	if (fd1 == -1) {
		printf ("Unable to open %s\n",pathname);
		return 0;
	}
	return 1;
}
// long find_len(int fd1){
// 	FILE* f = fdopen(fd1, "w");
// 	lseek (fd1, 0, SEEK_END);
// 	long len=ftell(f);
// 	lseek (fd1, 0, SEEK_SET);
// 	printf("%ld\n",len);
// 	return len;
// 	fclose(f);
// }
int s_open (const char *pathname, int flags, mode_t mode)
{

	assert (filesys_inited);
	int fd1=open (pathname, flags, mode);
	return fd1;
}
size_t allocate_block(char *buf,int fd1,int index){

	lseek(fd1,index*64,SEEK_SET);
	size_t size=read(fd1,buf,64);
	if(size==-1){
		printf("error while allocating block in s_open");
	}
	return size;
}
int s_open_temp (const char *pathname, int flags, mode_t mode)
{
	merkle_tree* root=malloc(sizeof(struct merkle_tree));
	assert (filesys_inited);
	int fd1=open(pathname, flags, mode);
	int flag=file_descriptior_error(fd1,pathname);
	if (flag==0) {
		return 0;
	}
	char *container[2000];
	for(int i=0;i<number_of_blocks;i++){
		char buf[64];
		size_t temp_size=allocate_block(buf,fd1,i);
		if(temp_size==-1){
			return 0;
		}
		unsigned char* hashed_val;
		get_sha1_hash(buf,64,(unsigned char*)hashed_val);
		memcpy(container[i],hashed_val, sizeof(hashed_val));
	}
	lseek(fd1,0,SEEK_SET);
	printf("successful");
	return fd1;
}
/* SEEK_END should always return the file size 
 * updated through the secure file system APIs.
 */
int s_lseek (int fd, long offset, int whence)
{
	assert (filesys_inited);
	return lseek (fd, offset, SEEK_SET);
}

/* read the blocks that needs to be updated
 * check the integrity of the blocks
 * modify the blocks
 * update the in-memory Merkle tree and root in secure.txt
 * returns -1 on failing the integrity check.
 */

ssize_t s_write (int fd, const void *buf, size_t count)
{
	assert (filesys_inited);
	return write (fd, buf, count);
}

/* check the integrity of blocks containing the 
 * requested data.
 * returns -1 on failing the integrity check.
 */
ssize_t s_read (int fd, void *buf, size_t count)
{
	assert (filesys_inited);
	return read (fd, buf, count);
}

/* destroy the in-memory Merkle tree */
int s_close (int fd)
{
	assert (filesys_inited);
	return close (fd);
}

/* Check the integrity of all files in secure.txt
 * remove the non-existent files from secure.txt
 * returns 1, if an existing file is tampered
 * return 0 on successful initialization
 */
int filesys_init (void)
{
	filesys_inited = 1;
	int fd1;
	char filename[32];
	strcpy(filename,"secure.txt");
	fd1 = open (filename, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
	int flag=file_descriptior_error(fd1,"secure.txt");
	if (flag==0) {
		return 0;
	}

	return 0;
}
