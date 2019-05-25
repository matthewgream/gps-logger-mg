
/* ----------------------------------------------------------------------------------------------------*/
/*
	mod_packlzari.c: gps logger module for LZARI based packer.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.
 */
/* ----------------------------------------------------------------------------------------------------*/

#ifndef PACKLZARI_STANDALONE

#include "common.h"
#include "mod_util.h"
#include "mod_packlzari.h"

#else

#define VERSION "@@VERSION@@"
#define COPYRIGHT "@@COPYRIGHT@@"

#define NULL (0)

typedef unsigned char boolean;

#define TRUE ((boolean) 1)
#define FALSE ((boolean) 0)

/*@-exportlocal@*/
/*@-exportheader@*/
#ifdef DEBUG
/*@-namechecks@*/ /*@noreturnwhenfalse@*/ static void __assert (/*@notnull@*/ const char* const filename, /*@notnull@*/ const unsigned int fileline, /*@notnull@*/ const char* const expression) __attribute__ ((noreturn)); /*@=namechecks@*/
#define assert(x) do { if (!(x)) __assert (__FILE__, (unsigned int) __LINE__, #x); } while (FALSE)
#else
/*@i@*/ #define assert(x)
#endif

typedef boolean (*packlzari_output_handler_t) (const unsigned char* const, const unsigned int);

#define UNUSED(arg) /*@-noeffect@*/ (void) arg /*@=noeffect@*/

#endif

/* ----------------------------------------------------------------------------------------------------*/

/* Derived from LZARI - 4/7/1989 Haruhiko Okumura - EXTENSIVELY modified */

#ifndef PACKLZARI_ENCODE
#ifndef PACKLZARI_DECODE
#error "must define PACKLZARI_ENCODE or PACKLZARI_DECODE, or both!"
#endif
#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef PACKLZARI_N
#define N				PACKLZARI_N
#else
#define N				256
#endif

#ifdef PACKLZARI_F
#define F				PACKLZARI_F
#else
#define F				96
#endif

#ifdef PACKLZARI_T
#define T				PACKLZARI_T
#else
#define T				2
#endif

#define NIL				N

#define M				15

#define Q1				(1UL << M)
#define Q2				(2 * Q1)
#define Q3				(3 * Q1)
#define Q4				(4 * Q1)
#define MAX_CUM			(Q1 - 1)

#define N_CHAR			(256 - T + F)

/* ----------------------------------------------------------------------------------------------------*/

#ifdef PACKLZARI_ENCODE
typedef enum {
		state_encode_initial = 0,
		state_encode_main
} state_encode_t;
#endif

#ifdef PACKLZARI_DECODE
typedef enum {
		state_decode_initial = 0,
		state_decode_char,
		state_decode_posn,
		state_decode_over
} state_decode_t;
#endif

typedef struct {
#ifdef PACKLZARI_ENCODE
		state_encode_t encode_S;
		unsigned short encode_i, encode_r, encode_s, encode_l;
		unsigned short match_pos, match_len, match_len_last;
		unsigned short lson [N + 1], rson [N + 1 + 256], dad [N + 1];
		unsigned short shifts;
#endif
#ifdef PACKLZARI_DECODE
		state_decode_t decode_S;
		unsigned short decode_i, decode_r, decode_s, decode_c, decode_p;
		unsigned long value;
#endif
		unsigned char bitbuff, bitmask;
		/*@shared@*/ packlzari_output_handler_t output_handler;
		unsigned char buffer [N + F - 1];
		unsigned long lo, hi;
		unsigned short chr_to_sym [N_CHAR], sym_to_chr [N_CHAR + 1];
		unsigned short sym_cnt [N_CHAR + 1], sym_cum [N_CHAR + 1], pos_cum [N + 1];
} state_t;

typedef struct {
		/*@shared@*/ const unsigned char * buffer;
		unsigned short length, offset;
} input_t;

/* ----------------------------------------------------------------------------------------------------*/

static void model_init (/*@out@*/ state_t* const state)
{
		unsigned short i, sym;

		assert (state != NULL);

		state->sym_cum [N_CHAR] = 0;
		for (sym = (unsigned short) N_CHAR; sym >= (unsigned short) 1; sym--) {
				unsigned short ch = (unsigned short) (sym - 1);
				state->chr_to_sym [ch] = sym;
				state->sym_to_chr [sym] = ch;
				state->sym_cnt [sym] = (unsigned short) 1;
				state->sym_cum [sym - 1] = (unsigned short) (state->sym_cum [sym] + state->sym_cnt [sym]);
		}
		state->sym_cnt [0] = 0;
		state->pos_cum [N] = 0;
		for (i = (unsigned short) N; i >= (unsigned short) 1; i--)
				state->pos_cum [i - 1] = (unsigned short) (state->pos_cum [i] + 10000 / (i + 200));
		state->lo = 0;
		state->hi = Q4;
}

static void model_update (state_t* const state, const unsigned short sym)
{
		unsigned short i;

		assert (state != NULL);
		assert (sym < (unsigned short) (N_CHAR + 1));

		if (state->sym_cum [0] >= (unsigned short) MAX_CUM) {
				unsigned short c = 0;
				for (i = (unsigned short) N_CHAR; i > 0; i--) {
						state->sym_cum [i] = c;
						state->sym_cnt [i] = (unsigned short) ((unsigned short) (state->sym_cnt [i] + 1) >> 1);
						c += state->sym_cnt [i];
				}
				state->sym_cum [0] = c;
		}
		for (i = sym; state->sym_cnt [i] == state->sym_cnt [i - 1]; i--)
				/*@i@*/ ;
		if (i < sym) {
				unsigned short ch_i = state->sym_to_chr [i], ch_sym = state->sym_to_chr [sym];
				state->sym_to_chr [i] = ch_sym;
				state->sym_to_chr [sym] = ch_i;
				state->chr_to_sym [ch_i] = sym;
				state->chr_to_sym [ch_sym] = i;
		}
		state->sym_cnt [i]++;
		while (--i > 0)
				state->sym_cum [i]++;
		if (i == 0)
				state->sym_cum [i]++;
}

/* ----------------------------------------------------------------------------------------------------*/

static void arith_update (state_t* const state, const unsigned short* const cum, const unsigned short off)
{
		unsigned long range;

		assert (state != NULL);
		assert (cum != NULL);
		assert (state->hi >= state->lo);

		range = state->hi - state->lo;
		state->hi = state->lo + (range * cum [off]) / cum [0];
		state->lo = state->lo + (range * cum [off + 1]) / cum [0];
}

/* ----------------------------------------------------------------------------------------------------*/

#ifdef PACKLZARI_ENCODE

static void node_init (/*@out@*/ state_t* const state)
{
		unsigned short i;

		assert (state != NULL);

		for (i = (unsigned short) (N + 1); i <= (unsigned short) (N + 256); i++)
				state->rson [i] = (unsigned short) NIL;
		for (i = 0; i < (unsigned short) N; i++)
				state->dad [i] = (unsigned short) NIL;
}

static void node_insert (state_t* const state, const unsigned short r, const unsigned char* const b, unsigned short* const match_pos, unsigned short* const match_len)
{
		int cmp = 1;
		unsigned short i, p = (unsigned short) (N + 1 + (int) b [r]);

		assert (state != NULL);
		assert (r < (unsigned short) (N + 1));
		assert (b != NULL);
		assert (match_pos != NULL && match_len != NULL);

		state->rson [r] = state->lson [r] = (unsigned short) NIL;
		*match_len = 0;
		for ( ; ; ) {
				if (cmp >= 0) {
						if (state->rson [p] != (unsigned short) NIL) {
								p = state->rson [p];
						} else {
								state->rson [p] = r;
								state->dad [r] = p;
								return;
						}
				} else {
						if (state->lson [p] != (unsigned short) NIL) {
								p = state->lson [p];
						} else {
								state->lson [p] = r;
								state->dad [r] = p;
								return;
						}
				}
				for (i = (unsigned short) 1; i < (unsigned short) (F - 1); i++) {
						if ((cmp = (int) (b [r + i] - b [p + i])) != 0)
								/*@innerbreak@*/ break;
				}
				if (i > (unsigned short) T) {
						if (i > *match_len) {
								*match_pos = (unsigned short) ((r - p) & (N - 1));
								*match_len = i;
								if (i >= (unsigned short) (F - 1))
										break;
						} else if (i == *match_len) {
								unsigned short t = (unsigned short) ((r - p) & (N - 1));
								if (t < *match_pos)
										*match_pos = t;
						}
				}
		}
		state->dad [r] = state->dad [p];
		state->lson [r] = state->lson [p];
		state->rson [r] = state->rson [p];
		state->dad [state->lson [p]] = r;
		state->dad [state->rson [p]] = r;
		if (state->rson [state->dad [p]] == p)
				state->rson [state->dad [p]] = r;
		else
				state->lson [state->dad [p]] = r;
		state->dad [p] = (unsigned short) NIL;
}

static void node_remove (state_t* const state, const unsigned short p)
{
		unsigned short q;

		assert (state != NULL);
		assert (p < (unsigned short) (N + 1));

		if (state->dad [p] == (unsigned short) NIL)
				return;
		if (state->rson [p] == (unsigned short) NIL) {
				q = state->lson [p];
		} else if (state->lson [p] == (unsigned short) NIL) {
				q = state->rson [p];
		} else {
				q = state->lson [p];
				if (state->rson [q] != (unsigned short) NIL) {
						do {
								q = state->rson [q];
						} while (state->rson [q] != (unsigned short) NIL);
						state->rson [state->dad [q]] = state->lson [q];
						state->dad [state->lson [q]] = state->dad [q];
						state->lson [q] = state->lson [p];
						state->dad [state->lson [p]] = q;
				}
				state->rson [q] = state->rson [p];
				state->dad [state->rson [p]] = q;
		}
		state->dad [q] = state->dad [p];
		if (state->rson [state->dad [p]] == p)
				state->rson [state->dad [p]] = q;
		else
				state->lson [state->dad [p]] = q;
		state->dad [p] = (unsigned short) NIL;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean encode_b_put (state_t* const state, const unsigned char b)
{
		assert (state != NULL);
		assert (b == (unsigned char) 0 || b == (unsigned char) 1);

		if (b > (unsigned char) 0)
				state->bitbuff |= state->bitmask;
		if ((state->bitmask >>= 1) == (unsigned char) 0) {
				if (state->output_handler (&(state->bitbuff), (unsigned int) 1) == FALSE)
						return FALSE;
				state->bitbuff = (unsigned char) 0; state->bitmask = (unsigned char) 0x80;
		}
		return TRUE;
}

static boolean encode_b_flush (state_t* const state)
{
		assert (state != NULL);

		/*@i@*/ while (state->bitmask != (unsigned char) 0x80) {
				if (encode_b_put (state, (unsigned char) 0) == FALSE)
						return FALSE;
		}
		return TRUE;
}

/* ----------------------------------------------------------------------------------------------------*/

static boolean encode_p_output (state_t* const state, const unsigned char bit)
{
		unsigned char nbit;

		assert (state != NULL);
		assert (bit == (unsigned char) 0 || bit == (unsigned char) 1);

		nbit = (unsigned char) (bit > (unsigned char) 0 ? 0 : 1);
		if (encode_b_put (state, bit) == FALSE)
				return FALSE;
		for ( ; state->shifts > 0; state->shifts--) {
				if (encode_b_put (state, nbit) == FALSE)
						return FALSE;
		}
		return TRUE;
}

static boolean encode_p_init (/*@out@*/ state_t* const state)
{
		assert (state != NULL);

		state->shifts = 0;
		state->bitbuff = (unsigned char) 0;
		state->bitmask = (unsigned char) 0x80;
		model_init (state);
		return TRUE;
}

static boolean encode_p_update (state_t* const state)
{
		assert (state != NULL);

		for ( ; ; ) {
				if (state->hi <= Q2) {
						if (encode_p_output (state, (unsigned char) 0) == FALSE)
								return FALSE;
				} else if (state->lo >= Q2) {
						if (encode_p_output (state, (unsigned char) 1) == FALSE)
								return FALSE;
						state->lo -= Q2; state->hi -= Q2;
				} else if (state->lo >= Q1 && state->hi <= Q3) {
						state->shifts++;
						state->lo -= Q1; state->hi -= Q1;
				} else {
						break;
				}
				state->lo += state->lo; state->hi += state->hi;
		}
		return TRUE;
}

static boolean encode_p_chr (state_t* const state, const unsigned short chr)
{
		unsigned short sym;
		assert (state != NULL);
		assert (chr < (unsigned short) N_CHAR);

		sym = state->chr_to_sym [chr];
		assert ((unsigned short) (sym - 1) < (unsigned short) sizeof (state->sym_cum));
		arith_update (state, state->sym_cum, (unsigned short) (sym - 1));
		if (encode_p_update (state) == FALSE)
				return FALSE;
		model_update (state, sym);
		return TRUE;
}

static boolean encode_p_pos (state_t* const state, const unsigned short pos)
{
		assert (state != NULL);
		assert (pos < (unsigned short) sizeof (state->pos_cum));

		arith_update (state, state->pos_cum, pos);
		if (encode_p_update (state) == FALSE)
				return FALSE;	
		return TRUE;
}

static boolean encode_p_term (state_t* const state)
{
		assert (state != NULL);

		if (encode_p_chr (state, (unsigned short) (255 - T + F)) == FALSE) /* end token */
				return FALSE;
		state->shifts++;
		if (encode_p_output (state, (unsigned char) (state->lo < Q1 ? 0 : 1)) == FALSE)
				return FALSE;
		if (encode_b_flush (state) == FALSE)
				return FALSE;
		return TRUE;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef PACKLZARI_DECODE

static unsigned char decode_b_get (state_t* const state, input_t* const input)
{
		assert (state != NULL);
		assert (input != NULL);
		assert (!(input->offset == input->length && (state->bitmask >> 1) == (unsigned char) 0));

		if ((state->bitmask >>= 1) == (unsigned char) 0) {
				state->bitbuff = input->buffer [input->offset++]; state->bitmask = (unsigned char) 0x80;
		}
		return (unsigned char) ((state->bitbuff & state->bitmask) != (unsigned char) 0	? 1 : 0);
}

static boolean decode_b_empty (state_t* const state, input_t* const input)
{
		assert (state != NULL);
		assert (input != NULL);

		return (input->offset == input->length && (state->bitmask >> 1) == (unsigned char) 0) ? TRUE : FALSE;
}

/* ----------------------------------------------------------------------------------------------------*/

static unsigned short decode_p_search (unsigned short* const t, const unsigned short n, const unsigned short x)
{
		unsigned short i = (unsigned short) 1, j = n, k;

		assert (t != NULL);

		while (i < j) {
				k = (unsigned short) ((i + j) >> 1);
				if (t[k] > x) {
						i = (unsigned short) (k + 1); 
				} else { 
						j = k;
				}
		}
		return i;
}

static boolean decode_p_init (/*@out@*/ state_t* const state)
{
		assert (state != NULL);

		state->bitbuff = (unsigned char) 0;
		state->bitmask = (unsigned char) 0;
		state->value = 0;
		model_init (state);
		return TRUE;
}

static boolean decode_p_pre (state_t* const state, input_t* const input, unsigned short* const offset)
{
		assert (state != NULL);
		assert (input != NULL);
		assert (offset != NULL);

		for (; *offset < (unsigned short) (M + 2); (*offset)++) {
				if (decode_b_empty (state, input) == TRUE)
						return TRUE;
				state->value = (state->value << 1) + (unsigned long) decode_b_get (state, input);
		}
		return FALSE;
}

static boolean decode_p_update (state_t* const state, input_t* const input)
{
		assert (state != NULL);
		assert (input != NULL);

		for ( ; ; ) {
				if (state->lo >= Q2) {
						if (decode_b_empty (state, input) == TRUE)
								return TRUE;
						state->value -= Q2;	 state->lo -= Q2;  state->hi -= Q2;
				} else if (state->lo >= Q1 && state->hi <= Q3) {
						if (decode_b_empty (state, input) == TRUE)
								return TRUE;
						state->value -= Q1;	 state->lo -= Q1;  state->hi -= Q1;
				} else if (state->hi > Q2) {
						break;
				} else {
						if (decode_b_empty (state, input) == TRUE)
								return TRUE;
				}
				state->lo += state->lo;	 state->hi += state->hi;
				state->value = (state->value << 1) + (unsigned long) decode_b_get (state, input);
		}
		return FALSE;
}

static void decode_p_chr_initial (state_t* const state)
{
		assert (state != NULL);

		state->decode_s = decode_p_search (state->sym_cum, (unsigned short) N_CHAR, 
				(unsigned short) (((state->value - state->lo + 1) * state->sym_cum [0] - 1) / (state->hi - state->lo)));
		assert (state->decode_s < (unsigned short) sizeof (state->sym_to_chr));
		state->decode_c = state->sym_to_chr [state->decode_s];
		assert ((unsigned short) (state->decode_s - 1) < (unsigned short) sizeof (state->sym_cum));
		arith_update (state, state->sym_cum, (unsigned short) (state->decode_s - 1));
}

static boolean decode_p_chr_process (state_t* const state, input_t* const input)
{
		assert (state != NULL);
		assert (input != NULL);

		if (decode_p_update (state, input) == TRUE)
				return TRUE;
		model_update (state, state->decode_s);
		return FALSE;
}

static void decode_p_pos_initial (state_t* const state)
{
		assert (state != NULL);

		state->decode_p = decode_p_search (state->pos_cum, (unsigned short) N,
				(unsigned short) (((state->value - state->lo + 1) * state->pos_cum [0] - 1) / (state->hi - state->lo))) - (unsigned short) 1;
		assert (state->decode_p < (unsigned short) sizeof (state->pos_cum));
		arith_update (state, state->pos_cum, state->decode_p);
}

static boolean decode_p_pos_process (state_t* const state, input_t* const input)
{
		assert (state != NULL);
		assert (input != NULL);

		if (decode_p_update (state, input) == TRUE)
				return TRUE;
		return FALSE;
}

static boolean decode_p_term (state_t* const state)
{
		assert (state != NULL);

		UNUSED (state);
		return TRUE;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef PACKLZARI_ENCODE

static boolean encode_init (/*@out@*/ state_t* const state, packlzari_output_handler_t output_handler)
{
		state->output_handler = output_handler;
		if (encode_p_init (state) == FALSE)
				return FALSE;
		node_init (state);
		for (state->encode_r = 0; state->encode_r < (unsigned short) (N - F); state->encode_r++)
				state->buffer [state->encode_r] = ' ';
		state->encode_l = 0;
		state->encode_S = state_encode_initial;
		return TRUE;
}

static boolean encode_process_middlefill_leader (state_t* const state)
{
		if (state->match_len > state->encode_l)
				state->match_len = state->encode_l;
		if (state->match_len <= (unsigned short) T) {
				state->match_len = (unsigned short) 1;
				if (encode_p_chr (state, (unsigned short) state->buffer [state->encode_r]) == FALSE)
						return FALSE;
		} else {
				if (encode_p_chr (state, (unsigned short) (255 - T + state->match_len)) == FALSE)
						return FALSE;
				if (encode_p_pos (state, (unsigned short) (state->match_pos - 1)) == FALSE)
						return FALSE;
		}
		state->match_len_last = state->match_len;
		return TRUE;
}

static boolean encode_process_middlefill_trailer (state_t* const state)
{
		while (state->encode_i++ < state->match_len_last) {
				node_remove (state, state->encode_s);
				if (state->encode_s++ == (unsigned short) (N - 1))
						state->encode_s = 0;
				if (state->encode_r++ == (unsigned short) (N - 1))
						state->encode_r = 0;
				if (--state->encode_l > 0)
						node_insert (state, state->encode_r, state->buffer, &state->match_pos, &state->match_len);
		}
		return TRUE;
}

static boolean encode_process_initialfill_final (state_t* const state)
{
		for (state->encode_i = (unsigned short) 1; state->encode_i <= (unsigned short) F; state->encode_i++)
				node_insert (state, (unsigned short) (state->encode_r - state->encode_i), state->buffer, &state->match_pos, &state->match_len);
		node_insert (state, state->encode_r, state->buffer, &state->match_pos, &state->match_len);
		if (encode_process_middlefill_leader (state) == FALSE)
				return FALSE;
		state->encode_i = 0;
		return TRUE;
}

static boolean encode_process_middlefill_final (state_t* const state)
{
		if (encode_process_middlefill_trailer (state) == FALSE)
				return FALSE;
		if (state->encode_l > 0) {
				if (encode_process_middlefill_leader (state) == FALSE)
						return FALSE;
				state->encode_i = 0;
		}
		return TRUE;
}

static boolean encode_process (state_t* const state, const unsigned char* const buffer, const unsigned short length)
{
		unsigned short offset = 0;

		{
				if (state->encode_S == state_encode_initial) {
						for (; state->encode_l < (unsigned short) F && offset < length; state->encode_l++, offset++)
								state->buffer [state->encode_r + state->encode_l] = buffer [offset];
						if (state->encode_l < (unsigned short) F) /* assert (offset == length); */
								return TRUE;
						if (encode_process_initialfill_final (state) == FALSE)
								return FALSE;
						state->encode_S = state_encode_main;
				}
				if (state->encode_S == state_encode_main) {
						do {
								for (; state->encode_i < state->match_len_last && offset < length; state->encode_i++, offset++) {
										node_remove (state, state->encode_s);
										state->buffer [state->encode_s] = buffer [offset];
										if (state->encode_s < (unsigned short) (F - 1))
												state->buffer [state->encode_s + N] = buffer [offset];
										if (state->encode_s++ == (unsigned short) (N - 1))
												state->encode_s = 0;
										if (state->encode_r++ == (unsigned short) (N - 1))
												state->encode_r = 0;
										node_insert (state, state->encode_r, state->buffer, &state->match_pos, &state->match_len);
								}
								if (state->encode_i < state->match_len_last) /* assert (offset == length); */
										return TRUE;
								if (encode_process_middlefill_final (state) == FALSE)
										return FALSE;
						} while (state->encode_l > 0);
				}
		}
		return TRUE;
}

static boolean encode_term (state_t* const state)
{
		if (state->encode_S == state_encode_initial) {
				if (state->encode_l == (unsigned short) 0) /* didn't process anything! */
						return TRUE;
				if (encode_process_initialfill_final (state) == FALSE)
						return FALSE;
				state->encode_S = state_encode_main;
		}
		if (state->encode_S == state_encode_main) {
				do {
						if (encode_process_middlefill_final (state) == FALSE)
								return FALSE;
				} while (state->encode_l > 0);
		}
		if (encode_p_term (state) == FALSE)
				return FALSE;	
		return TRUE;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef PACKLZARI_DECODE

static boolean decode_init (/*@out@*/ state_t* const state, packlzari_output_handler_t output_handler)
{
		state->output_handler = output_handler;
		if (decode_p_init (state) == FALSE)
				return FALSE;
		for (state->decode_r = 0; state->decode_r < (unsigned short) (N - F); state->decode_r++)
				state->buffer [state->decode_r] = ' ';
		state->decode_i = 0;
		state->decode_S = state_decode_initial;
		return TRUE;
}

static boolean decode_output_chr (state_t* const state, const unsigned char chr)
{
		if (state->output_handler (&chr, (unsigned int) 1) == FALSE)
				return FALSE;
		state->buffer [state->decode_r] = chr;
		if (state->decode_r++ == (unsigned short) (N - 1))
				state->decode_r = 0;
		return TRUE;
}

static boolean decode_output_pos (state_t* const state, const unsigned short chr, const unsigned short pos)
{
		unsigned short i, p = (unsigned short) ((unsigned short) (state->decode_r - pos - 1) & (N - 1)),
					   n = (unsigned short) (chr - 255 + T);
		for (i = 0; i < n; i++) {
				if (decode_output_chr (state, state->buffer [(p + i) & (N - 1)]) == FALSE)
						return FALSE;
		}
		return TRUE;
}

static boolean decode_process (state_t* const state, const unsigned char* const buffer, const unsigned short length)
{
		input_t input;
		input.buffer = buffer;
		input.length = length;
		input.offset = 0;

		while (state->decode_S != state_decode_over)
		{
				if (state->decode_S == state_decode_initial) {
						if (decode_p_pre (state, &input, &state->decode_i) == TRUE)
								break;
						decode_p_chr_initial (state);
						state->decode_S = state_decode_char;
				}
				if (state->decode_S == state_decode_char) {
						if (decode_p_chr_process (state, &input) == TRUE)
								break;
						if (state->decode_c < (unsigned short) 256) {
								if (decode_output_chr (state, (unsigned char) state->decode_c) == FALSE)
										return FALSE;
								decode_p_chr_initial (state);
								/* state->decode_S = state_decode_char; */
						} else if ((unsigned short) (state->decode_c - 255 + T) < (unsigned short) F) {
								decode_p_pos_initial (state);
								state->decode_S = state_decode_posn;
						} else {
								state->decode_S = state_decode_over;
						}
				}
				if (state->decode_S == state_decode_posn) {
						if (decode_p_pos_process (state, &input) == TRUE)
								break;
						if (decode_output_pos (state, state->decode_c, state->decode_p) == FALSE)
								return FALSE;
						decode_p_chr_initial (state);
						state->decode_S = state_decode_char;
				}
		}
		return TRUE;
}

static boolean decode_term (state_t* const state)
{
		const unsigned char deof = (unsigned char) 0xFF;
		if (state->decode_S != state_decode_initial) {
				/*@i@*/ while (state->decode_S != state_decode_over) {
						if (decode_process (state, &deof, (unsigned short) 1) == FALSE)
								return FALSE;
				}
		}
		if (decode_p_term (state) == FALSE)
				return FALSE;	
		return TRUE;
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifndef PACKLZARI_STANDALONE

/*@-evalorderuncon@*/

/*@null@*/ static state_t* packlzari_state = NULL;

#ifdef PACKLZARI_ENCODE
boolean packlzari_encode_init (packlzari_output_handler_t handler)
		{ DPRINTF (("packlzari_encode_init\n"));
		  if (packlzari_state == NULL) packlzari_state = (state_t*) util_buffer_alloc ((unsigned int) sizeof (*packlzari_state)); 
		  if (packlzari_state == NULL) return FALSE;
		  (void) util_memset ((void *) packlzari_state, (unsigned char) 0, (unsigned int) sizeof (*packlzari_state));
		  return encode_init (packlzari_state, handler); }
boolean packlzari_encode_term (void) 
		{ assert (packlzari_state != NULL);
		  return encode_term (packlzari_state); }
boolean packlzari_encode_write (const unsigned char* const buffer, const unsigned int length)
		{ assert (packlzari_state != NULL);
		  return encode_process (packlzari_state, buffer, (unsigned short) length); }
#endif

#ifdef PACKLZARI_DECODE
static boolean packlzari_decode_init (packlzari_output_handler_t handler)
		{ DPRINTF (("packlzari_decode_init\n"));
		  if (packlzari_state == NULL) packlzari_state = (state_t*) util_buffer_alloc ((unsigned int) sizeof (*packlzari_state)); 
		  if (packlzari_state == NULL) return FALSE;
		  (void) util_memset ((void *) packlzari_state, (unsigned char) 0, (unsigned int) sizeof (*packlzari_state));
		  return decode_init (packlzari_state, handler); }
static boolean packlzari_decode_term (void) 
		{ assert (packlzari_state != NULL);
		  return decode_term (packlzari_state); }
static boolean packlzari_decode_write (const unsigned char* const buffer, const unsigned int length)
		{ assert (packlzari_state != NULL);
		  return decode_process (packlzari_state, buffer, (unsigned short) length); }
#endif

/*@=evalorderuncon@*/

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifndef PACKLZARI_STANDALONE

#ifdef TEST_ENABLED

/*@observer@*/ static const char * const packlzari_test_cdata = "$GPGGA,,,,,,0,00,,,,,,,*66\r\n";
static const unsigned int packlzari_test_csize = (unsigned int) sizeof ("$GPGGA,,,,,,0,00,,,,,,,*66\r\n") - 1;
/*@shared@*/ /*@null@*/ static unsigned char * packlzari_test_buffer;
static unsigned int packlzari_test_length;
static volatile unsigned int packlzari_test_offset;
static unsigned int packlzari_test_count;

#define PACKLZARI_TEST_BLENGTH 4096
#define PACKLZARI_TEST_BGUARD  512

static boolean packlzari_test_handler_encode (const unsigned char* const data, const unsigned int size)
{
		unsigned int wrsz = UTIL_MIN (size, packlzari_test_length - packlzari_test_offset);
		if (wrsz > 0) {
				if (packlzari_test_buffer != NULL)
						(void) util_memcpy (&packlzari_test_buffer [packlzari_test_offset], data, wrsz);
				packlzari_test_offset += wrsz;
		}
		return (packlzari_test_offset <= packlzari_test_length) ? TRUE : FALSE;
}
static boolean packlzari_test_handler_decode_data (const unsigned char* const data, const unsigned int size)
{
		unsigned int wrct;
		for (wrct = 0; wrct < size; ++wrct) {
				if (data [wrct] != (unsigned char ) packlzari_test_cdata [packlzari_test_count++ % packlzari_test_csize])
						return FALSE;
		}
		return TRUE;
}
static boolean packlzari_test_handler_decode_size (const unsigned char* const data, const unsigned int size)
{
		UNUSED (data);
		packlzari_test_count += size;
		return TRUE;
}

test_result_t packlzari_test (void)
{
		unsigned int count;

/*@-evalorderuncon@*/
		packlzari_test_buffer = util_buffer_alloc ((unsigned int) PACKLZARI_TEST_BLENGTH);
/*@=evalorderuncon@*/
		packlzari_test_length = (unsigned int) PACKLZARI_TEST_BLENGTH;

		test_assert (packlzari_test_buffer != NULL);

		/* encode/decode empty */
		packlzari_test_offset = 0; count = 0;
		test_assert (packlzari_encode_init (packlzari_test_handler_encode) == TRUE);
		test_assert (packlzari_encode_term () == TRUE);
		DPRINTF (("pack-enc: in=%u, out=%u\n", count, packlzari_test_offset));
		test_assert (count == 0 && packlzari_test_offset == 0);
		packlzari_test_count = 0;
		test_assert (packlzari_decode_init (packlzari_test_handler_decode_size) == TRUE);
		test_assert (packlzari_decode_term () == TRUE);
		DPRINTF (("pack-dec: in=%u, out=%u\n", packlzari_test_offset, packlzari_test_count));
		test_assert (packlzari_test_count == count);

		/* encode/decode single */
		packlzari_test_offset = 0; count = 0;
		test_assert (packlzari_encode_init (packlzari_test_handler_encode) == TRUE);
		test_assert (packlzari_encode_write ((const unsigned char *) packlzari_test_cdata, (count = packlzari_test_csize)) == TRUE);
		test_assert (packlzari_encode_term () == TRUE);
		DPRINTF (("pack-enc: in=%u, out=%u\n", count, packlzari_test_offset));
		packlzari_test_count = 0;
		test_assert (packlzari_decode_init (packlzari_test_handler_decode_size) == TRUE);
		test_assert (packlzari_decode_write (packlzari_test_buffer, packlzari_test_offset) == TRUE);
		test_assert (packlzari_decode_term () == TRUE);
		DPRINTF (("pack-dec: in=%u, out=%u\n", packlzari_test_offset, packlzari_test_count));
		test_assert (packlzari_test_count == count);

		/* encode/decode multiple */	
		packlzari_test_offset = 0; count = 0;
		test_assert (packlzari_encode_init (packlzari_test_handler_encode) == TRUE);
		/*@i@*/ while (packlzari_test_offset < (packlzari_test_length - PACKLZARI_TEST_BGUARD)) {
				test_assert (packlzari_encode_write ((const unsigned char *) packlzari_test_cdata, packlzari_test_csize) == TRUE);
				count += packlzari_test_csize;
		}
		test_assert (packlzari_encode_term () == TRUE);
		DPRINTF (("pack-enc: in=%u, out=%u\n", count, packlzari_test_offset));
		packlzari_test_count = 0;
		test_assert (packlzari_decode_init (packlzari_test_handler_decode_data) == TRUE);
		test_assert (packlzari_decode_write (packlzari_test_buffer, packlzari_test_offset) == TRUE);
		test_assert (packlzari_decode_term () == TRUE);
		DPRINTF (("pack-dec: in=%u, out=%u\n", packlzari_test_offset, packlzari_test_count));

		/* encode/decode random */
		util_rand_seed ((unsigned int) 676144);
		packlzari_test_offset = 0; count = 0;
		test_assert (packlzari_encode_init (packlzari_test_handler_encode) == TRUE);
		/*@i@*/ while (packlzari_test_offset < (packlzari_test_length - PACKLZARI_TEST_BGUARD)) {
				unsigned char cbuf [64];
				unsigned int csiz = (util_rand () % sizeof (cbuf)), coff = 0;
				cbuf [0] = '\0';
				while (coff < csiz) cbuf [coff++] = (unsigned char) (util_rand () & 0xFF);
				test_assert (packlzari_encode_write (cbuf, csiz) == TRUE);
				count += csiz;
		}
		test_assert (packlzari_encode_term () == TRUE);
		DPRINTF (("pack-enc: in=%u, out=%u\n", count, packlzari_test_offset));
		packlzari_test_count = 0;
		test_assert (packlzari_decode_init (packlzari_test_handler_decode_size) == TRUE);
		test_assert (packlzari_decode_write (packlzari_test_buffer, packlzari_test_offset) == TRUE);
		test_assert (packlzari_decode_term () == TRUE);
		DPRINTF (("pack-dec: in=%u, out=%u\n", packlzari_test_offset, packlzari_test_count));
		test_assert (packlzari_test_count == count);

		util_buffer_reset ();

		return TEST_RESULT_OKAY;
}

#endif

#endif

/* ----------------------------------------------------------------------------------------------------*/

#ifdef PACKLZARI_STANDALONE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static unsigned long exec_count_read, exec_count_write;

static boolean exec_putx (const unsigned char* const buffer, const unsigned int length)
{
		unsigned int l = (unsigned int) write (fileno (stdout), buffer, (size_t) length);
		exec_count_write += (unsigned long) length;
		return (l == length) ? TRUE : FALSE;
}

static boolean exec (boolean (*init) (/*@out@*/ state_t* const, packlzari_output_handler_t), 
				boolean (*proc) (state_t* const, const unsigned char* const, const unsigned short), boolean (*term) (state_t* const))
{
		unsigned char buffer [4096]; /* arbitrary */
		long length;

		state_t state;

		exec_count_read = exec_count_write = 0;
		if ((*init) (&state, exec_putx) == FALSE)
				return FALSE;
		while ((length = (long) read (fileno (stdin), buffer, sizeof (buffer))) > 0) {
				exec_count_read += (unsigned long) length;
				if ((*proc) (&state, buffer, (unsigned short) length) == FALSE)
						return FALSE;
		}
		if ((*term) (&state) == FALSE)
				return FALSE;
		(void) fprintf (stderr, "(%d/%d/%d; %d) :: ", N, F, T, (int) sizeof (state_t));
		(void) fprintf (stderr, "read = %lu bytes, write = %lu bytes :: ", exec_count_read, exec_count_write);
		(void) fprintf (stderr, "ratio = %f\n", (double) exec_count_write / exec_count_read);
		return TRUE;
}

#ifdef DEBUG
static void __assert (const char* const filename, const unsigned int fileline, const char* const expression)
{
		(void) fprintf (stderr, "assert(%s:%u): %s\n", filename, fileline, expression);
		exit (EXIT_FAILURE);
}
#endif

int main (int argc, char* argv [])
{
		UNUSED (argc);
		UNUSED (argv);

		fprintf (stderr, "gps_logger_mg v" VERSION ": utility lzari-pack\n");
		fprintf (stderr, COPYRIGHT "\n");

#ifdef PACKLZARI_ENCODE
#ifdef PACKLZARI_DECODE
		if (argc == 2 && (*argv[1] == 'E' || *argv[1] == 'e'))
#endif
		return exec (encode_init, encode_process, encode_term) == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
#endif
#ifdef PACKLZARI_DECODE
#ifdef PACKLZARI_ENCODE
		if (argc == 2 && (*argv[1] == 'D' || *argv[1] == 'd'))
#endif
		return exec (decode_init, decode_process, decode_term) == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
#endif

#ifdef PACKLZARI_ENCODE
#ifdef PACKLZARI_DECODE
		(void) fprintf (stderr, 
					"'%s e' encodes stdin to stdout.\n"
					"'%s d' decodes stdin to stdout.\n",
					argv [0], argv [0]
		);
		return EXIT_FAILURE;
#endif
#endif
}

#endif

/* ----------------------------------------------------------------------------------------------------*/

