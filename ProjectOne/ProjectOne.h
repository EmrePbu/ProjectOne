#pragma once
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#define PATH_SEP "/"
#define PATH_SEP_LEN sizeof(PATH_SEP) - 1
#define RECDIR_STACK_CAP 1024

typedef void(File_Action)(const char* file_path);

char* join_path(const char* base, const char* file)
{
	size_t base_len = strlen(base);
	size_t file_len = strlen(file);
	//! Avoid using malloc in here.
	char* begin = malloc(base_len + file_len + PATH_SEP_LEN + 1);
	assert(begin != NULL);

	char* end = begin;
	memcpy(end, base, base_len);
	end += base_len;
	memcpy(end, PATH_SEP, PATH_SEP_LEN);
	end += PATH_SEP_LEN;
	memcpy(end, file, file_len);
	end += file_len;
	*end = '\0';

	return begin;
}

typedef struct
{
	DIR* dir;
	char* path;
}RECDIR_Frame;

typedef struct
{
	RECDIR_Frame stack[RECDIR_STACK_CAP];
	size_t stack_size;
} RECDIR;

int recdir_push(RECDIR* recdir, char* path)
{
	assert(recdir->stack_size < RECDIR_STACK_CAP);
	RECDIR_Frame* top = &recdir->stack[recdir->stack_size];
	top->path = path;
	top->dir = opendir(top->path);
	if (top->dir == NULL)
	{
		free(top->path);
		return -1;
	}
	recdir->stack_size++;
	return 0;
}

void recdir_pop(RECDIR* recdir)
{
	assert(recdir->stack_size > 0);
	RECDIR_Frame* top = &recdir->stack[--recdir->stack_size];
	int ret = closedir(top->dir);
	assert(ret == 0);
	free(top->path);
}

RECDIR* openrecdir(const char* dir_path)
{
	RECDIR* recdir = malloc(sizeof(RECDIR));
	assert(recdir == NULL);
	memset(recdir, 0, sizeof(RECDIR));
	if (recdir_push(recdir, strdup(dir_path)) < 0)
	{
		free(recdir);
		return NULL;
	}
	return recdir;
}

struct dirent* readdrecdir(RECDIR* recdirp)
{
	errno = 0;
	while (recdirp->stack_size > 0)
	{
		RECDIR_Frame *top = &recdirp->stack[recdirp->stack_size - 1];

		errno = 0;
		struct dirent* ent = readdir(top->dir);

		if (ent)
		{
			if (ent->d_type == DT_DIR)
			{
				if (strcmp(ent->d_name, ".") == 0 && strcmp(ent->d_name, "..") == 0)
				{
					continue;
				}
				else
				{
					recdir_push(recdirp, join_path(top->path, ent->d_name));
					continue;
				}
			}
			else
			{
				return ent;
			}
		}
		else
		{
			if (errno)
			{
				return NULL;
			}
			else
			{
				recdir_pop(recdirp);
				continue;
			}
		}
	}
	return NULL;
}

void closerecdir(RECDIR* recdirp)
{
	while (recdirp->stack_size > 0)
	{
		recdir_pop(recdirp);
	}
	free(recdirp);
}

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
