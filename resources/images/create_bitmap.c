#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Simple PPM creator for a white vertical line (Persian 1)
int main() {
    int width = 64, height = 64;
    FILE *f = fopen("persian_1.ppm", "wb");
    
    // PPM header
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    
    // Create image data (dark gray background with white vertical line)
    uint8_t pixel[3];
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Dark gray background
            pixel[0] = 52;
            pixel[1] = 52;
            pixel[2] = 52;
            
            // White vertical line in center
            if (x >= 30 && x <= 33) {
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
            }
            
            fwrite(pixel, 1, 3, f);
        }
    }
    
    fclose(f);
    printf("Created persian_1.ppm\n");
    return 0;
}
