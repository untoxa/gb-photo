#ifndef __VECTOR_H_INCLUDE__
#define __VECTOR_H_INCLUDE__

#include <stdint.h>

#define VECTOR_DECLARE(NAME, NUM_ELEMS) uint8_t NAME[NUM_ELEMS + 1] = {0}
#define VECTOR_ITERATE(VECTOR, IDX, ELEM) for(IDX = 0, ELEM = VECTOR[1]; IDX != VECTOR[0]; IDX ++, ELEM = VECTOR[IDX + 1])

inline uint8_t vector_get(uint8_t * v, uint8_t pos) {
    return v[pos + 1];
}

inline void vector_add(uint8_t * v, uint8_t elem) {
	v[++ v[0]] = elem;
}

inline void vector_remove_at(uint8_t * v, uint8_t pos) {
	memcpy(&v[pos + 1], &v[pos + 2], v[0] - pos);
	v[0] --;
}

#endif