#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
using namespace std;
#include <stddef.h> // 如果你需要使用标准库的 size_t，包含此头文件

size_t bin_to_data(const char *bin_file, uint8_t *addr) {
    FILE *file = fopen(bin_file, "rb");
    
    if (!file) {
        fprintf(stderr, "Error opening file.\n");
        return 0; // 返回0表示出错
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fread(addr, 1, size, file) != size) {
        fprintf(stderr, "Error reading file.\n");
        fclose(file);
        return 0; // 返回0表示出错
    }

    fclose(file);
    return size;
}


int main() {
    // 打开二进制文件并读取数据
    char bin_file[8]="enc.bin";
    FILE *file = fopen("enc.bin", "rb");
    
    if (!file) {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    // 创建字符数组以存储数据
    uint8_t *binary_data = (uint8_t*)malloc(size + 1); // +1 用于存储 null 终止符

    size_t t=bin_to_data(bin_file,binary_data);

    free(binary_data);

    return 0;
}
