#define _DEFAULT_SOURCE

#include <stdio.h>
#include "recdir.h"


int main(int argc, char const** argv)
{
	RECDIR* recdir = recdir_open("C:\\Users\\emrep\\Desktop\\a");
	errno = 0;
	struct dirent* ent = recdir_read(recdir);

	while (ent)
	{
		printf("recdir file: %s/%s\n", recdir_path(recdir), ent->d_name);
		ent = recdir_read(recdir);
	}

	if (errno != 0)
	{
		fprintf(stderr, "ERROR: could not read the director: %s\n", recdir_path(recdir));
		exit(1);
	}

	recdir_close(recdir);

	return 0;
}
// https://youtu.be/bpCJf67e1lI?t=5567
// E:\\Belgeler\\PythonProjects\\pythoncropimage\\out bu dizin de 1 milyon gorsel var
