#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "recdir.h"

typedef void(File_Action)(const char* file_path);

void visit_files(const char* dir_path)
{
	DIR* dir = opendir(dir_path);
	if (dir == NULL)
	{
		fprintf(stderr, "ERROR: Could not open directory. %s: %s\n", dir_path, strerror(errno));
		exit(1);
	}
	errno = 0;
	struct dirent* ent = readdir(dir);
	while (ent != NULL)
	{
		char* child_path = join_path(dir_path, ent->d_name);
		if (ent->d_type == DT_DIR)
		{
			if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
			{
				visit_files(child_path);
			}
		}
		else
		{
			printf("file: %s\n", child_path);
		}

		free(child_path);
		ent = readdir(dir);
	}
	if (errno != 0)
	{
		fprintf(stderr, "ERROR: Could not read directory. %s: %s\n", dir_path, strerror(errno));
		exit(1);
	}
	closedir(dir);
}


int main(int argc, char const** argv)
{
	RECDIR* recdir = openrecdir("C:\\Users\\emrep\\Desktop\\a");
	errno = 0;
	struct dirent* ent = readrecdir(recdir);

	while (ent)
	{
		printf("recdir file: %s/%s\n", recdir_path(recdir), ent->d_name);
		ent = readrecdir(recdir);
	}

	if (errno != 0)
	{
		fprintf(stderr, "ERROR: could not read the director: %s\n", recdir_path(recdir));
		exit(1);
	}

	closerecdir(recdir);

	return 0;
}
// https://youtu.be/bpCJf67e1lI?t=4665
// E:\\Belgeler\\PythonProjects\\pythoncropimage\\out bu dizin de 1 milyon gorsel var
