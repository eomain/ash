
#ifndef ASH_UTIL_H
#define ASH_UTIL_H

struct strbuf {
    char *buf;
    size_t index;
    size_t size;
};

void strbuf_init(struct strbuf *, size_t);
const char *strbuf_get(struct strbuf *);
void strbuf_push_char(struct strbuf *, char);
void strbuf_push_str(struct strbuf *, const char *);

#endif
