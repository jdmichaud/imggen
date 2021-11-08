// echo imggen.c | entr bash -c 'reset && cc imggen.c -o imggen && echo "done."'
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_NUMBER_SIZE 32
#define WHITE           0xffffffL
#define FONT_SIZE       4

static int font(int c, int x, int y) {
  /* 8x8 font data for "0123456789." */
  static const uint64_t font[] = {
    0x3c7e666666667e3c, 0x1838781818181818, 0x183c66460c183e7e,
    0x3c7e063c3c067e3c, 0x0c1c34647e0c0c0c, 0x7e7e607c7e067e3c,
    0x3c7e607c7e667e3c, 0x7e7e06060c0c1818, 0x3c7e663c3c667e3c,
    0x3c7e667e3e067e3c, 0x0000000000383838
  };
  uint64_t f = font[c == '.' ? 10 : c - '0'];
  return (f >> ((7 - y) * 8 + (7 - x))) & 1;
}

void print_number(unsigned char *image, uint16_t width, char *buf,
                  uint16_t posx, uint16_t posy, uint32_t color, uint8_t scale) {
  for (uint32_t i = 0; buf[i]; i++) {
    for (int y = 0; y < 8 * scale; y++) {
      for (int x = 0; x < 8 * scale; x++) {
        if (font(buf[i], x / scale, y / scale)) {
          int px = posx + x + i * scale * 8;
          int py = posy + y;
          image[py * width * 3 + px * 3] = color >> 16;
          image[py * width * 3 + px * 3 + 1] = color >> 8;
          image[py * width * 3 + px * 3 + 2] = color >> 0;
        }
      }
    }
  }
}

void generate(unsigned char *image, uint16_t width, uint16_t height, uint64_t frameid) {
  memset(image, 0, height * width * 3);
  char frameidstr[MAX_NUMBER_SIZE];
  snprintf(frameidstr, MAX_NUMBER_SIZE, "%lu", frameid);
  print_number(image, width, frameidstr, 10, 10, WHITE, FONT_SIZE);
}

size_t write(unsigned char *image, uint16_t width, uint16_t height) {
  printf("P6\n%d %d\n255\n", width, height);
  return fwrite(image, width * height * 3, 1, stdout);
}

int main(int argc, char **argv) {
  uint16_t width = 512;
  uint16_t height = 512;

  uint64_t count = 0;
  uint64_t lastCount = 0;;
  struct timespec last;
  clock_gettime(CLOCK_MONOTONIC_RAW, &last);
  while (count < 100000) {
    unsigned char image[width * height * 3];
    generate((unsigned char *) &image, width, height, count++);
    // fprintf(stderr, "writing image %lu\n", count);
    write((unsigned char *) &image, width, height);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    if (now.tv_sec - last.tv_sec > 0) {
      fprintf(stderr, "%f fps\n",
        ((double) (count - lastCount)) / (now.tv_sec - last.tv_sec));
      last.tv_sec = now.tv_sec;
      lastCount = count;
    }
  }
  return 0;
}
