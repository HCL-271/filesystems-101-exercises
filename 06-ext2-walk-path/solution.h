#pragma once

/**
   Implement this function to copy the content of a file at @path
   to a file descriptor @out. @path has no symlinks inside it.
   @img is a file descriptor of an open ext2 image.

   Do take care to return -ENOENT, -ENOTDIR and other errors that
   may happen during a path traversal.
*/
int dump_file(int img, const char *path, int out);
