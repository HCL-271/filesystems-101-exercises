#pragma once

/**
   Implement this function to copy the content of a file at @path
   to a file descriptor @out. @path has no symlinks inside it.
   @img is a file descriptor of an open ntfs image.

   You may use any API provided by libntfs-3g.
*/
int dump_file(int img, const char *path, int out);
