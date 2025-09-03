// ███████╗████████╗     ██████╗ ██╗   ██╗████████╗ ██████╗██╗  ██╗ █████╗ ██████╗
// ██╔════╝╚══██╔══╝     ██╔══██╗██║   ██║╚══██╔══╝██╔════╝██║  ██║██╔══██╗██╔══██╗
// █████╗     ██║        ██████╔╝██║   ██║   ██║   ██║     ███████║███████║██████╔╝
// ██╔══╝     ██║        ██╔═══╝ ██║   ██║   ██║   ██║     ██╔══██║██╔══██║██╔══██╗
// ██║        ██║███████╗██║     ╚██████╔╝   ██║   ╚██████╗██║  ██║██║  ██║██║  ██║
// ╚═╝        ╚═╝╚══════╝╚═╝      ╚═════╝    ╚═╝    ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
//
// <<legacy_seqs.c>>

#include <string.h>

#include "internal/_kbinput.h"

#define _SEQ_COUNT	84

#define _VEC_OUT_OF_BOUNDS	INT64_MAX

typedef struct {
	vector	text;
	u16		name;
}	seq;

vector	seqs;

static inline u8	_add_seq(const u16 name, const char *text);

static void	_free_seq(void *seq);

void	setup_legacy_key_seqs(void) {
	const char	*seq_str;
	size_t		i;
	u16			seq_names[_SEQ_COUNT] = {
				ti_kf0, ti_kf1, ti_kf2, ti_kf3, ti_kf4, ti_kf5, ti_kf6, ti_kf7, ti_kf8, ti_kf9,
				ti_kf10, ti_kf11, ti_kf12, ti_kf13, ti_kf14, ti_kf15, ti_kf16, ti_kf17, ti_kf18,
				ti_kf19, ti_kf20, ti_kf21, ti_kf22, ti_kf23, ti_kf24, ti_kf25, ti_kf26, ti_kf27,
				ti_kf28, ti_kf29, ti_kf30, ti_kf31, ti_kf32, ti_kf33, ti_kf34, ti_kf35, ti_kf36,
				ti_kf37, ti_kf38, ti_kf39, ti_kf40, ti_kf41, ti_kf42, ti_kf43, ti_kf44, ti_kf45,
				ti_kf46, ti_kf47, ti_kf48, ti_kf49, ti_kf50, ti_kf51, ti_kf52, ti_kf53, ti_kf54,
				ti_kf55, ti_kf56, ti_kf57, ti_kf58, ti_kf59, ti_kf60, ti_kf61, ti_kf62, ti_kf63,
				ti_kcuu1, ti_kcud1, ti_kcub1, ti_kcuf1, ti_kich1, ti_khome, ti_knp, ti_kdch1,
				ti_kend, ti_kpp, ti_kri, ti_kind,  ti_kLFT, ti_kRIT, ti_kIC, ti_kHOM,
				ti_kNXT, ti_kDC, ti_kEND, ti_kPRV };

	seqs = vector(seq, _SEQ_COUNT, _free_seq);
	if (seqs) {
		for (i = 0; i < _SEQ_COUNT; i++) {
			seq_str = ti_getstr(seq_names[i]);
			if (seq_str != TI_ABS_STR && !_add_seq(seq_names[i], seq_str)) {
				vector_delete(seqs);
				seqs = NULL;
				break ;
			}
		}
		vector_shrink_to_fit(seqs);
	}
}

u16	parse_legacy_key_seq(const char *text) {
	const seq	*_seq;
	size_t		size;
	size_t		i;

	if (!seqs)
		return LEGACY_SEQ_NO_MATCH;
	for (i = 0, size = vector_size(seqs); i < size; i++) {
		_seq = vector_get(seqs, i);
		if (strncmp(text, vector_get(_seq->text, 0), vector_size(_seq->text)) == 0)
			break ;
	}
	return (i < size) ? _seq->name : LEGACY_SEQ_NO_MATCH;
}

static inline u8	_add_seq(const u16 name, const char *text) {
	size_t	i;
	size_t	len;
	seq		_seq;

	len = strlen(text) + 1;
	_seq.text = vector(char, len, NULL);
	if (!_seq.text)
		return 0;
	for (_seq.name = name, i = 0; i < len; i++)
		if (!vector_push(_seq.text, text[i]))
			return 0;
	return vector_push(seqs, _seq);
}

static void	_free_seq(void *seq) {
	if (seq)
		vector_delete(seq);
}
