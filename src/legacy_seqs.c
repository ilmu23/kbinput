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

#define _SEQ_COUNT	90

#define _VEC_DEFAULT_SIZE	4

#define _VEC_OUT_OF_BOUNDS	INT64_MAX

#define get_char(vec, i)		(((seq_node *)vector_get(vec, i))->c)
#define get_name(vec, i)		(((seq_node *)vector_get(vec, i))->name)
#define get_next_vec(vec, i)	(((seq_node *)vector_get(vec, i))->next)

typedef struct __seq_node {
	vector	next;
	u16		name;
	char	c;
}	seq_node;

vector	seqs;

static inline size_t	_find_char(const char c, const vector vec);
static inline u8		_add_char(const char c, vector *vec);
static inline u8		_add_seq(const u16 name, const char *seq);

static void	_free_seq_node(void *node);

void	setup_legacy_key_seqs(void) {
	const char	*seq;
	size_t		i;
	u16			seq_names[_SEQ_COUNT] = {
				ti_kf0, ti_kf1, ti_kf2, ti_kf3, ti_kf4, ti_kf5, ti_kf6, ti_kf7, ti_kf8, ti_kf9,
				ti_kf10, ti_kf11, ti_kf12, ti_kf13, ti_kf14, ti_kf15, ti_kf16, ti_kf17, ti_kf18, ti_kf19,
				ti_kf20, ti_kf21, ti_kf22, ti_kf23, ti_kf24, ti_kf25, ti_kf26, ti_kf27, ti_kf28, ti_kf29, 
				ti_kf30, ti_kf31, ti_kf32, ti_kf33, ti_kf34, ti_kf35, ti_kf36, ti_kf37, ti_kf38, ti_kf39, 
				ti_kf40, ti_kf41, ti_kf42, ti_kf43, ti_kf44, ti_kf45, ti_kf46, ti_kf47, ti_kf48, ti_kf49, 
				ti_kf50, ti_kf51, ti_kf52, ti_kf53, ti_kf54, ti_kf55, ti_kf56, ti_kf57, ti_kf58, ti_kf59, 
				ti_kf60, ti_kf61, ti_kf62, ti_kf63, ti_kcuu1, ti_kcud1, ti_kcub1, ti_kcuf1, ti_kich1,
				ti_khome, ti_knp, ti_kdch1, ti_kend, ti_kpp, ti_kLFT, ti_kRIT, ti_kIC, ti_kHOM,
				ti_kNXT, ti_kDC, ti_kEND, ti_kPRV };

	seqs = vector(seq_node, _VEC_DEFAULT_SIZE, _free_seq_node);
	if (seqs) {
		for (i = 0; i < _SEQ_COUNT; i++) {
			seq = ti_getstr(seq_names[i]);
			if (seq != TI_ABS_STR && !_add_seq(seq_names[i], seq)) {
				vector_delete(seqs);
				seqs = NULL;
				break ;
			}
		}
	}
}

u16	parse_legacy_key_seq(const char *seq) {
	vector	vec;
	size_t	i;

	for (vec = seqs; vec && *seq; seq++) {
		i = _find_char(*seq, vec);
		if (i == _VEC_OUT_OF_BOUNDS)
			return LEGACY_SEQ_NO_MATCH;
		vec = get_next_vec(vec, i);
	}
	i = _find_char(*seq, vec);
	return (i != _VEC_OUT_OF_BOUNDS) ? get_name(vec, i) : LEGACY_SEQ_NO_MATCH;
}

static inline size_t	_find_char(const char c, const vector vec) {
	size_t	size;
	size_t	i;

	for (i = 0, size = vector_size(vec); i < size; i++)
		if (get_char(vec, i) > c)
			break ;
	return (i < size && get_char(vec, i) == c) ? i : _VEC_OUT_OF_BOUNDS;
}

static inline u8	_add_char(const char c, vector *vec) {
	seq_node	node;
	size_t		size;
	size_t		i;

	node = (seq_node){
		.next = vector(seq_node, _VEC_DEFAULT_SIZE, _free_seq_node),
		.name = UINT16_MAX,
		.c = c
	};
	if (!node.next)
		return 0;
	for (i = 0, size = vector_size(*vec); i < size; i++)
		if (get_char(*vec, i) > c)
			break ;
	return vector_insert(*vec, i, node);
}

static inline u8	_add_seq(const u16 name, const char *seq) {
	seq_node	node;
	vector		vec;
	size_t		i;

	vec = seqs;
	while (*seq) {
		i = _find_char(*seq, vec);
		if (i == _VEC_OUT_OF_BOUNDS && !_add_char(*seq, &vec))
			return 0;
		else
			vec = get_next_vec(vec, i);
		seq++;
	}
	node = (seq_node){
		.next = NULL,
		.name = name,
		.c = *seq
	};
	return vector_insert(vec, 0, node);
}

static void	_free_seq_node(void *node) {
	if (node)
		vector_delete(((seq_node *)node)->next);
}
