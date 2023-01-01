#include <stdlib.h>
#include "file_stream.h"
#include "err_sys.h"

/* Additional functions */
void write_bit(FileStream *stream, int bit);
void put_char(FileStream *stream, unsigned char character);

/* Function initialize and returns FileStream structure */
FileStream* open_file_stream(char *fileName, Mode mode, int char_buffer, int char_buffer_pos)
{
    const char modes[3][3] = {"rb", "wb", "ab"};  
    FileStream *stream = malloc(sizeof(FileStream));

    if(stream == NULL)
        err_sys("Allocating memory for FileStream structure");

    stream->fp = fopen(fileName, modes[mode]);
    if(stream->fp == NULL)
        err_sys("Opening file");

    stream->mode = mode;
    stream->char_buffer = char_buffer;
    stream->char_buffer_pos = char_buffer_pos;
    stream->io_buffer = malloc(IO_BUFFER_SIZE * sizeof(unsigned char));
    stream->io_buffer_pos = 0;
    stream->io_buffer_length = 0;
    
    if(stream->io_buffer == NULL)
        err_sys("Allocating memory for buffer");

    return stream;
}

/* Function closes stream and frees Filestream structure */
void close_file_stream(FileStream *stream)
{
    int return_status;
    if(stream == NULL)
        err_sys("FileStream object does not exist");

    return_status = fclose(stream->fp);
    if(return_status < 0)
        err_sys("Closing file");
    
    free(stream->io_buffer);
    free(stream);
}

/* Function clears char_buffer and io_buffer */
void clear_output_buffer(FileStream *stream)
{
    if(stream->char_buffer_pos != 0)
        put_char(stream, stream->char_buffer);
    if(stream->io_buffer_pos != 0){
        stream->io_buffer_length = fwrite(stream->io_buffer, sizeof(unsigned char), stream->io_buffer_pos, stream->fp);
        if(stream->io_buffer_length != stream->io_buffer_pos)
            err_sys("Writing to file");
    }
}

/* Function reads byte from stream and outputs its value */
int read_byte(FileStream *stream)
{
    unsigned char code = 0;
    for (int i = 0; i < 8; i++)
    {
        int bit = read_bit(stream);
        if (bit == EOF)
            return EOF;
        code = code | (bit << i);
    }
    return code;
}

/* Function reads one bit from stream and returns its value */
int read_bit(FileStream *stream)
{
    int bit;
    /* If max bit is reached */
    if (stream->char_buffer_pos == 256)
    {
        stream->char_buffer_pos = 1;
        stream->char_buffer = get_char(stream);
        if (stream->char_buffer == EOF)
            return EOF;
    }

    if(stream->char_buffer & stream->char_buffer_pos)
        bit = 1;
    else
        bit = 0;

    stream->char_buffer_pos <<= 1;
    return bit;
}

/* Function writes code of length code_length to stream */
void write_code(FileStream *stream, char *code, unsigned int code_length)
{
    int mask_pos = 0;
    while (code_length--){
        write_bit(stream, code[mask_pos]);
        mask_pos++;
    }
}

/* Function writes byte to stream */
void write_byte(FileStream *stream, unsigned char byte)
{
    int bit;
    for (int mask_pos = 0; mask_pos < 8; mask_pos++)
    {
        bit = byte & (1 << mask_pos);
        write_bit(stream, bit);
    }
}

/* Function writes bit to stream */
void write_bit(FileStream *stream, int bit)
{
    if(bit)
        stream->char_buffer |= 1 << stream->char_buffer_pos;
    else
        stream->char_buffer |= 0 << stream->char_buffer_pos;

    /* If current char is full then output */
    if (stream->char_buffer_pos++ == 7) 
    {
        put_char(stream, stream->char_buffer);
        stream->char_buffer = 0;
        stream->char_buffer_pos = 0;
    }
}

/* Function reads character from stream using custum buffering */
int get_char(FileStream *stream)
{
    if(stream->io_buffer_pos == stream->io_buffer_length){
        stream->io_buffer_length = fread(stream->io_buffer, sizeof(unsigned char), IO_BUFFER_SIZE, stream->fp);
        if(ferror(stream->fp))
            err_sys("Reading from file");
        if(feof(stream->fp) && stream->io_buffer_length == 0)
            return EOF;
        stream->io_buffer_pos = 0;
    }
    unsigned char ret_char = stream->io_buffer[stream->io_buffer_pos];
    stream->io_buffer_pos++;
    return ret_char;
}

/* Function puts character to stream using custum buffering */
void put_char(FileStream *stream, unsigned char character)
{
    if(stream->io_buffer_pos == IO_BUFFER_SIZE){
        stream->io_buffer_length = fwrite(stream->io_buffer, sizeof(unsigned char), IO_BUFFER_SIZE, stream->fp);
        if(stream->io_buffer_length != IO_BUFFER_SIZE)
            err_sys("Writing to file");
        stream->io_buffer_pos = 0;
    }
    stream->io_buffer[stream->io_buffer_pos] = character;
    stream->io_buffer_pos++;
}

