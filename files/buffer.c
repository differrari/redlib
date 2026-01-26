#include "buffer.h"
#include "syscalls/syscalls.h"

buffer buffer_create(size_t size, buffer_options options){
    return (buffer){
        .buffer = zalloc(size),
        .buffer_size = 0,
        .limit = size,
        .options = options,
        .cursor = 0,
    };
}

void buffer_write(buffer *buf, char* fmt, ...){
    __attribute__((aligned(16))) va_list args;
    va_start(args, fmt); 
    buffer_write_va(buf, fmt, args);
    //TODO: circular
    va_end(args);
}

void buffer_resize(buffer *buf, size_t amount){
    size_t new_size = amount ? buf->limit + amount : buf->limit * 2;
    buf->buffer = realloc_sized(buf->buffer, buf->limit, new_size);
    buf->limit = new_size;
}

void buffer_write_va(buffer *buf, char* fmt, va_list args){
    if ((buf->options & buffer_can_grow) && strlen(fmt) > buf->limit-256){
        buffer_resize(buf,strlen(fmt)*2);
    }
    size_t n = string_format_va_buf(fmt, buf->buffer+buf->cursor, buf->limit-buf->cursor, args);
    buf->cursor += n;
    buf->buffer_size += n;
    if ((buf->options & buffer_can_grow) && buf->buffer_size > buf->limit-256){
        buffer_resize(buf,0);
    }
}

void buffer_write_const(buffer *buf, char *lit){
    size_t lit_size = strlen(lit);
    buffer_write_lim(buf, lit, lit_size);
}

void buffer_write_lim(buffer *buf, char *lit, size_t lit_size){
    if ((int64_t)buf->limit - buf->buffer_size <= lit_size){
        if (buf->options & buffer_can_grow){
            buffer_resize(buf,lit_size*2);
        } else return;
        //TODO: circular
    }
    for (size_t i = 0; i < lit_size; i++){
        buf->buffer[buf->cursor++] = lit[i];
    }
    buf->buffer_size += lit_size;
}

void buffer_write_space(buffer *buf){
    buffer_write_const(buf, " ");
}

void buffer_destroy(buffer *buf){
    free_sized(buf->buffer,buf->limit);
    *buf = (buffer){};
}

#include "test.h"

bool buffer_test(){
    buffer testbuf = buffer_create(0x10, buffer_opt_none);
    buffer_write_const(&testbuf, "hey");
    assert_eq(testbuf.buffer_size, testbuf.cursor, "Size (%i) != cursor (%i)", testbuf.buffer_size,testbuf.cursor);
    assert_eq(testbuf.buffer_size, 3, "Size (%i) != written size (3)", testbuf.buffer_size);
    buffer_write_const(&testbuf, "a really long string. Should not fit");
    assert_eq(testbuf.buffer_size, 3, "Buffer grew from illegal write", testbuf.buffer_size);
    
    buffer_destroy(&testbuf);
    
    testbuf = buffer_create(0x10, buffer_can_grow);
    buffer_write_const(&testbuf, "hello");
    assert_eq(testbuf.buffer_size, testbuf.cursor, "Size (%i) != cursor (%i)", testbuf.buffer_size,testbuf.cursor);
    assert_eq(testbuf.buffer_size, 5, "Size (%i) != written size (5)", testbuf.buffer_size);
    buffer_write_const(&testbuf, "a really long string. Should expand");
    assert_eq(testbuf.buffer_size, 40, "Buffer did not reach expected size", testbuf.buffer_size);
    buffer_write_const(&testbuf, "another really long string. Should expand");
    assert_eq(testbuf.buffer_size, 81, "Buffer did not reach expected size", testbuf.buffer_size);
    
    //Loop:
        //Create buffer. Write less than capacity: stays. Write more, loops, write more than full capacity, error
    //Static
        //Create buffer. Write less than capacity: stays, Write more, error. Write again, starts at 0
        
    return true;
}