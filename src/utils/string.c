// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<string.c>>

#include <stdlib.h>
#include <string.h>

#include "internal/utils/string.h"

size_t	__strlcpy(char *dst, const char *src, const size_t n) {
	size_t	len;

	len = strnlen(src, n - 1);
	memcpy(dst, src, len);
	dst[len] = '\0';
	return strlen(src);
}

char	*__substr(const char *s, const size_t start, const size_t len) {
	size_t	_len;
	char	*out;

	_len = strnlen(&s[start], len) + 1;
	out = malloc(_len * sizeof(*out));
	if (out)
		__strlcpy(out, &s[start], _len);
	return out;
}
