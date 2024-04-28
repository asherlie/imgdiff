#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>

// attribute packed?
uint8_t pngsig[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};

struct png{
    _Bool valid;
    uint32_t w, h;
    uint8_t bits_per_pixel, color_type, compression_method, filter_method, interlaced;
};

void p_buf(uint8_t* buf, int len, _Bool pchar){
    for (int i = 0; i < len; ++i) {
        if (i && i % 4 == 0) {
            printf(" ");
        }
        if (i && i % 8 == 0) {
            puts("");
        }
        if (pchar && isalpha(buf[i])) {
            printf("   %c ", buf[i]);
        } else {
            printf("0x%.2X ", buf[i]);
        }
    }
    puts("");
}

_Bool check_read(int fd, uint8_t* buf, ssize_t len, const char* failure_msg){
    ssize_t br = read(fd, buf, len);
    if (br != len) {
        puts(failure_msg);
        return 0;
    }
    return 1;
}

int parse_ihdr(uint8_t* buf, struct png* img){
    // skip past chunk type 
    uint8_t* bufptr = buf + 4;
    
	/*bread = read(fd, buf, sizeof(ihdr));*/

    memcpy(&img->w, bufptr, sizeof(uint32_t));
    img->w = ntohl(img->w);
    bufptr += sizeof(uint32_t);

    memcpy(&img->h, bufptr, sizeof(uint32_t));
    img->h = ntohl(img->h);
    bufptr += sizeof(uint32_t);

    printf("IMG: [%i x %i]\n", img->w, img->h);

    memcpy(&img->bits_per_pixel, bufptr, sizeof(uint8_t));
    bufptr += sizeof(uint8_t);

    memcpy(&img->color_type, bufptr, sizeof(uint8_t));
    bufptr += sizeof(uint8_t);

    memcpy(&img->compression_method, bufptr, sizeof(uint8_t));
    bufptr += sizeof(uint8_t);

    memcpy(&img->filter_method, bufptr, sizeof(uint8_t));
    bufptr += sizeof(uint8_t);

    memcpy(&img->interlaced, bufptr, sizeof(uint8_t));
    bufptr += sizeof(uint8_t);

    return 0;
}
int parse_idat(uint8_t* buf, struct png* img){
    return 0;
}

int read_chunk(int fd, struct png* img){
	uint16_t buflen = 21024;
	uint8_t buf[buflen];
    uint8_t chunk_type[4];
    uint32_t chunklen;

    // TODO: this is not before each chunk...
    if (!img->valid && (!check_read(fd, buf, sizeof(pngsig), "Failed to read signature") || memcmp(buf, pngsig, sizeof(pngsig)))) {
        return 1;
    }
    img->valid = 1;

    /*
     * if (bread < (ssize_t)sizeof(pngsig) || memcmp(buf, pngsig, sizeof(pngsig))) {
     *     puts("Failed to read signature");
     *     p_buf(buf, sizeof(pngsig), 1);
     *     p_buf(pngsig, sizeof(pngsig), 1);
     *     return 1;
     * }
    */

    if (!check_read(fd, (uint8_t*)&chunklen, sizeof(uint32_t), "Failed to read chunklen")) {
        return 1;
    }
    puts("chunklen:");
    p_buf((uint8_t*)&chunklen, sizeof(uint32_t), 1);
    chunklen = ntohl(chunklen);
    printf("%i bytes of content!\n", chunklen);

    if (!check_read(fd, chunk_type, sizeof(chunk_type), "Failed to read chunktype")) {
        return 1;
    }
    printf("Found chunk %s\n", chunk_type);

    if (!check_read(fd, buf, chunklen, "Failed to read chunkdata")) {
        return 1;
    }

    if (!memcmp(chunk_type, "IHDR", 4)) {
        puts("Found IHDR bytes");
        parse_ihdr(buf, img);
    }
    else if (!memcmp(chunk_type, "IDAT", 4)) {
        puts("Found IDAT bytes");
        parse_idat(buf, img);
    }
    else {
        
    }

    // skip 4 bytes to bypass CRC
    lseek(fd, 4, SEEK_CUR);

    return 0;
}

int main(int a, char** b){
	int fd;
    struct png img = {0};
    
    if (a < 2) {
        return 1;
    }

    fd = open(b[1], O_RDONLY);
    read_chunk(fd, &img);
    read_chunk(fd, &img);
}
/*TODO: ignore until we get to IDAT*/
