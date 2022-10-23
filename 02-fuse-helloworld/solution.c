#include "solution.h"

#include <fuse.h>

static void *init_hello(struct fuse_conn_info *fuse_conn_info, struct fuse_config *fuse_config) {
  (void)fuse_conn_info;
  (void)fuse_config;
  return NULL;
}

static const struct fuse_operations hellofs_ops = {
	    .init = init_hello
};

int helloworld(const char *mntp)
{
	char *argv[] = {"exercise", "-f", (char *)mntp, NULL};
	return fuse_main(3, argv, &hellofs_ops, NULL);
}

