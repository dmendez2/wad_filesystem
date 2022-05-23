/*	The Following Code Was Based Off of the Following Resources:

	Author: Mohammed Q. Hussain
	Date Published: May 21, 2016
	Title: Writing a Simple Filesystem Using FUSE in C
	Type: Source Code
	Web Address: https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/

	Author: Lorenzo Fontana
	Date Published: January 11, 2016
	Title: Write a Filesystem with FUSE
	Type: Source Code
	Web Address: https://engineering.facile.it/blog/eng/write-filesystem-fuse/
 */


#define FUSE_USE_VERSION 26

#include <fuse.h>
#include "../libWad/Wad.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <cstring>

using namespace std;
static Wad* myWad;

static int do_getattr(const char* path, struct stat* st){
	memset(st, 0, sizeof(struct stat));

	st->st_uid = 0;
	st->st_gid = 0;
	st->st_atime = 0;
	st->st_mtime = 0;
	st->st_nlink = 2;

	if(myWad->isDirectory(path)){
		st -> st_mode = S_IFDIR | 0555;
		return 0;
	}

	if(myWad->isContent(path)){
		st->st_mode = S_IFREG | 0444;
		st->st_size = myWad->getSize(path);
		return 0;
	}

	return -ENOENT;
}

static int do_open(const char* path, struct fuse_file_info* fi){
	return 0;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	(void) fi;
	return myWad->getContents(path, buffer, size, offset);
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	(void) offset;
	(void) fi;

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	vector<string> entries;
	int ret = myWad->getDirectory(path,&entries);
	for(int ii = 0; ii < entries.size(); ii += 1){
		filler(buffer, entries.at(ii).c_str(), NULL, 0);
	}

	return 0;
}

static void do_destroy(void* private_data){
	myWad->Clear(myWad->getRoot());
	delete myWad;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .open 	= do_open,
    .read       = do_read,
    .readdir	= do_readdir,
    .destroy 	= do_destroy,
};

int main(int argc, char* argv[]){
	myWad = Wad::loadWad(argv[1]);
	char* targv[2];
	targv[0] = argv[0];
	targv[1] = argv[2];
	return fuse_main(2,targv, &operations, NULL );
}
