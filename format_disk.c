#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "kernel/fs/fs.h"

// Here's the basic plan:
// Somehow I need to either grab/generate an img file that is 1.44MB large, just like our actual disk image
// Initialize/build superblock
// Initialize/build bootblock
// Initialize/build bitmaps (inode & data)
// Initialize/build inode block
// Initialize/build data blocks

int main() {

	return 0;
}
