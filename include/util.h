/**
 * util.h
 *
 * Toke Høiland-Jørgensen
 * 2013-06-24
 */

#ifndef UTIL_H
#define UTIL_H

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#endif
