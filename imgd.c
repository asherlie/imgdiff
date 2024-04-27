#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>

// attribute packed?
uint8_t pngsig[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
uint8_t ihdr[4] = {'I', 'H', 'D', 'R'};
struct png_hdr{
    uint8_t magic[8];
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

int main(int a, char** b){
	uint16_t buflen = 21024;
	uint8_t buf[buflen];
    uint32_t ihdrlen, img_w, img_h;
    uint8_t bits_per_pixel, color_type, compression_method, filter_method;
	int fd;
    
    if (a < 2) {
        return 1;
    }

    fd = open(b[1], O_RDONLY);

	/*bread = read(fd, buf, sizeof(pngsig));*/
    if (!check_read(fd, buf, sizeof(pngsig), "Failed to read signature") || memcmp(buf, pngsig, sizeof(pngsig))) {
        return 1;
    }

    /*
     * if (bread < (ssize_t)sizeof(pngsig) || memcmp(buf, pngsig, sizeof(pngsig))) {
     *     puts("Failed to read signature");
     *     p_buf(buf, sizeof(pngsig), 1);
     *     p_buf(pngsig, sizeof(pngsig), 1);
     *     return 1;
     * }
    */

    if (!check_read(fd, (uint8_t*)&ihdrlen, sizeof(uint32_t), "Failed to read ihdrlen")) {
        return 1;
    }
    ihdrlen = ntohl(ihdrlen);
    printf("%i bytes of content!\n", ihdrlen);
    // TODO: actually read the above number of bytes before parsing
    //
    // UGH. should just do this now. this is important because i think it's the only way to 
    // know the length of the CRC bytes. it's the last n bytes of the header
    // i can replace these reads with just a check_read(ihdrlen)


	/*bread = read(fd, buf, sizeof(ihdr));*/
    if (!check_read(fd, buf, sizeof(ihdr), "Failed to read IHDR") || memcmp(buf, ihdr, sizeof(ihdr))) {
        return 1;
    }

    if (!check_read(fd, (uint8_t*)&img_w, sizeof(uint32_t), "Failed to read img width")) {
        return 1;
    }
    img_w = ntohl(img_w);

    if (!check_read(fd, (uint8_t*)&img_h, sizeof(uint32_t), "Failed to read img height")) {
        return 1;
    }
    img_h = ntohl(img_h);

    printf("IMG: %s - [%i x %i]\n", b[1], img_w, img_h);

}
