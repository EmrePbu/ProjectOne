#define _DEFAULT_SOURCE

#include <stdio.h>
#include <assert.h>
#include "recdir.h"
#include "sha256.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct
{
	BYTE bytes[32];
}Hash;

char hex_digit(unsigned int digit)
{
	digit = digit % 0x10;
	if (0 <= digit && digit <= 9)
	{
		return digit + '0';
	}
	if (10 <= digit && digit <= 15)
	{
		return digit - 10 + 'a';
	}
	assert(0 && "unreachable");
}

void hash_as_cstr(Hash hash, char output[32 * 2 + 1])
{
	for (size_t i = 0; i < 32; i++)
	{
		output[i * 2 + 0] = hex_digit(hash.bytes[i] / 0x10);
		output[i * 2 + 1] = hex_digit(hash.bytes[i] % 0x10);
	}
	output[32 * 2] = '\0';
}

void hash_of_file(const char* file_path, Hash* hash)
{
	SHA256_CTX ctx;
	memset(&ctx, 0, sizeof(ctx));
	sha256_init(&ctx);

	FILE* f = fopen(file_path, "rb");
	if (f == NULL)
	{
		fprintf(stderr, "Could not open file %s: %s\n", file_path, strerror(errno));
		exit(1);
	}

	BYTE buffer[1024];
	size_t buffer_size = fread(buffer, 1, sizeof(buffer), f);
	while (buffer_size > 0)
	{
		sha256_update(&ctx, buffer, buffer_size);
		buffer_size = fread(buffer, sizeof(buffer), 1, f);
	}

	if (ferror(f))
	{
		fprintf(stderr, "Could not read from file %s: %s\n", file_path, strerror(errno));
		exit(1);
	}
	fclose(f);

	sha256_final(&ctx, hash->bytes);
}

typedef struct
{
	Hash key;
	char** paths;
}Record;

Record* db = NULL;

int main(int argc, char const** argv)
{
	RECDIR* recdir = recdir_open("C:\\Users\\emrep\\Desktop\\a");
	errno = 0;
	struct dirent* ent = recdir_read(recdir);

	while (ent)
	{
		Hash hash;
		char* path = join_path(recdir_top(recdir)->path, ent->d_name);
		hash_of_file(path, &hash);

		ptrdiff_t index = hmgeti(db, hash);
		if (index < 0)
		{
			Record record;
			record.key = hash;
			record.paths = NULL;
			arrput(record.paths, path);
			hmputs(db, record);
		}
		else
		{
			arrput(db[index].paths, path);
		}

		ent = recdir_read(recdir);
		//free(path);
	}

	if (errno != 0)
	{
		fprintf(stderr, "ERROR: could not read the director: %s\n", recdir_top(recdir)->path);
		exit(1);
	}
	recdir_close(recdir);

	for (ptrdiff_t i = 0; i < hmlen(db); ++i)
	{
		if (arrlen(db[i].paths) > 1) 
		{
			char output[32 * 2 + 1];
			hash_as_cstr(db[i].key, output);
			printf("%s\n", output);

			for (ptrdiff_t j = 0; j < arrlen(db[i].paths); ++j)
			{
				printf("    %s\n", db[i].paths[j]);
			}
		}
	}

	return 0;
}
// https://youtu.be/bpCJf67e1lI?t=7795
// E:\\Belgeler\\PythonProjects\\pythoncropimage\\out bu dizin de 1 milyon gorsel var
/*
int main(int argc, char const** argv)
{
	printf("%s", argv[1]);
}

cmd > *.exe emre
out > emre

cmd > *.exe 123
out > 123
*/