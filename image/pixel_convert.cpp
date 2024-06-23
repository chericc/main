#include <cstdio>

void rgba2argb(FILE* fp_in, FILE* fp_out) {
    char pixel[4];

    while (1) {
        int ret = fread(pixel, 1, 4, fp_in);
        if (ret == 4) {
            // RGBA
            // (ARGB)
            // BGRA (ARGB reverse)
            char pixel_a[4];
            // pixel_a[0] = pixel[3];
            // pixel_a[1] = pixel[0];
            // pixel_a[2] = pixel[1];
            // pixel_a[3] = pixel[2];
            pixel_a[0] = pixel[2];
            pixel_a[1] = pixel[1];
            pixel_a[2] = pixel[0];
            pixel_a[3] = pixel[3];
            fwrite(pixel_a, 1, sizeof(pixel_a), fp_out);
        } else if (ret == 0) {
            break;
        } else {
            printf("error, ret=%d\n", ret);
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        return -1;
    }

    const char* file_in = argv[1];
    const char* file_out = argv[2];
    printf("%s --> %s\n", file_in, file_out);

    FILE* fp_in = nullptr;
    FILE* fp_out = nullptr;

    do {
        fp_in = fopen(file_in, "r");
        fp_out = fopen(file_out, "w");

        if (!fp_in || !fp_out) {
            printf("open failed\n");
            break;
        }

        rgba2argb(fp_in, fp_out);
    } while (0);

    if (fp_in) {
        fclose(fp_in);
        fp_in = nullptr;
    }

    if (fp_out) {
        fclose(fp_out);
        fp_out = nullptr;
    }

    return 0;
}