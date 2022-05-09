#include "so_stdio.h"

#include <sys/types.h>	/* open */
#include <sys/stat.h>	/* open */
#include <fcntl.h>	    /* open flags */
#include <string.h>

#define BUFSIZE 4096

struct _so_file {
	char buffer[BUFSIZE];   /* buffering functionality */
	HANDLE fd;              /* file descriptor */
	int cursor;          /* buffer cursor */
	int val;             /* number of bytes read/wrote before interrupt */
	int last_function;   /* 0 = read, 1 = write */
	long file_cursor;    /* file cursor */
	int eof;             /* != 0 for reaching eof*/
	size_t file_size;
};



SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	HANDLE fd;
	SO_FILE *soFile; 
	struct stat st;
	
	soFile = (SO_FILE *)malloc(sizeof(SO_FILE));

	if (!soFile)
		return NULL;

	soFile->eof = 0;
	soFile->cursor = 0;
	soFile->val = 0;
	memset(soFile->buffer, 0, BUFSIZE);
	soFile->last_function = -1;
	soFile->file_cursor = 0;

	
	stat(pathname, &st);
	soFile->file_size = st.st_size;

	if (!strcmp(mode, "r")) {
		fd = CreateFile(
			pathname,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (fd == INVALID_HANDLE_VALUE) {
			free(soFile);
			return NULL;
		}
		soFile->fd = fd;
		return soFile;

	} else if (!strcmp(mode, "r+")) {
		fd = CreateFile(
			pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (fd == INVALID_HANDLE_VALUE) {
			free(soFile);
			return NULL;
		}
		soFile->fd = fd;
		return soFile;

	} else if (!strcmp(mode, "w")) {
		fd = CreateFile(
			pathname,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (fd == INVALID_HANDLE_VALUE) {
			free(soFile);
			return NULL;
		}
		soFile->fd = fd;
		return soFile;

	} else if (!strcmp(mode, "w+")) {
		fd = CreateFile(
			pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (fd == INVALID_HANDLE_VALUE) {
			free(soFile);
			return NULL;
		}
		soFile->fd = fd;
		return soFile;

	} else if (!strcmp(mode, "a")) {
		fd = CreateFile(
			pathname,
			FILE_APPEND_DATA,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		); 

		if (fd == INVALID_HANDLE_VALUE) {
			free(soFile);
			return NULL;
		}
		soFile->fd = fd;
		return soFile;

	} else if (!strcmp(mode, "a+")) {
		fd = CreateFile(
			pathname,
			FILE_APPEND_DATA | GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		); 

		if (fd == INVALID_HANDLE_VALUE) {
			free(soFile);
			return NULL;
		}
		soFile->fd = fd;
		return soFile;

	}
	free(soFile);
	return NULL;
}

int so_fclose(SO_FILE *stream)
{
	int rc;

	so_fflush(stream);
	rc = CloseHandle(stream->fd);
	free(stream);
	return !rc;
}

int so_fgetc(SO_FILE *stream)
{
	int fileStatus = 1; /* fileStatus = {0(eof), number of bytes read, -1(err)} */
	
	/* empty, full, stopped by interrupt */
	if (stream->cursor == 0 ||
	stream->cursor == BUFSIZE ||
	stream->cursor == stream->val) {
		memset(stream->buffer, 0, BUFSIZE);
		stream->cursor = 0;
		ReadFile(
			stream->fd, 
			stream->buffer, 
			BUFSIZE,
			&fileStatus,
			NULL
		);
		stream->val = fileStatus;
		if (fileStatus == 0)
			stream->eof = 1;
	}

	if (fileStatus > 0) {
		stream->last_function = 0;
		stream->cursor += 1;
		stream->file_cursor += 1;
		return stream->buffer[stream->cursor - 1];
	} else {
		return SO_EOF;
	}
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	size_t cnt = 0; /* number of read characters */
	size_t i;
	char *word; 
	
	word = malloc(sizeof(char));
	
	while ((nmemb * size) > cnt && stream->file_size >= cnt) {
		for (i = 0; i < size; i++) {
			*word = so_fgetc(stream);
			memcpy((char *)ptr + cnt, word, 1);
			cnt++;
		}
	}

	if (stream->file_size < cnt)
		cnt--;

	free(word);
	return cnt / size;
}

int so_fputc(int c, SO_FILE *stream)
{
	int writeStatus = 0; /* {number of written characters, -1} */

	/* put character in buffer and advance cursor position */
	if (stream->cursor < BUFSIZE) {
		stream->last_function = 1;
		stream->buffer[stream->cursor] = c;
		stream->cursor++;
		stream->file_cursor++;
		return stream->buffer[stream->cursor - 1];
	}
	writeStatus = so_fflush(stream);
	stream->last_function = 1;
	stream->buffer[stream->cursor] = c;
	stream->cursor++;
	stream->file_cursor++;
	return stream->buffer[stream->cursor - 1];


	if (writeStatus == -1)
		return SO_EOF;
}


HANDLE so_fileno(SO_FILE *stream)
{
	return stream->fd;
}

int so_fflush(SO_FILE *stream)
{
	/* xwrite implementation */
	stream->val = 0;
	while (stream->val < stream->cursor) {
		int bytes_written_now;
		
		WriteFile(
			stream->fd, 
			stream->buffer + stream->val, 
			stream->cursor - stream->val,
			&bytes_written_now,
			NULL
		);

		if (bytes_written_now <= 0)
			return -1;
		stream->val += bytes_written_now;
	}

	memset(stream->buffer, 0, BUFSIZE);
	stream->cursor = 0;

	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{

	/* check if buffer was used by read and refreshes it */
	if (stream->last_function == 0) {
		memset(stream->buffer, 0, BUFSIZE);
		stream->cursor = 0;
	} else if (stream->last_function == 1) {
		so_fflush(stream);
	}

	stream->file_cursor = SetFilePointer(
		stream->fd,
		offset,
		NULL,
		whence
	);
	if (stream->file_cursor == -1)
		return -1;
	return 0;

}

long so_ftell(SO_FILE *stream)
{
	return stream->file_cursor;
}


size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	size_t i;
	char c;

	for (i = 0; i < size * nmemb; i++) {
		c = *((char *)(ptr) + i);
		so_fputc(c, stream);
	}

	return i / size;
}

int so_feof(SO_FILE *stream)
{
	return stream->eof;
}

int so_ferror(SO_FILE *stream)
{
	return -1;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	return -1;
}
