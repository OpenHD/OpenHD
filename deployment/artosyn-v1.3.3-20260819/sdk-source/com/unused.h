#ifndef __UNUSED_H__
#define __UNUSED_H__

#define UNUSED(expr)                                                                                                   \
    do {                                                                                                               \
        (void)(expr);                                                                                                  \
    } while (0)

#endif
