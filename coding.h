#define CODING_H 1
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdint>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <vector>
#include <stddef.h> 
#include <ctime>
#include <iostream>
#include <dirent.h>
#include <random>
#include "arithmetic_coding.hpp"

typedef int prob_t;
typedef int mean_t;
typedef mean_t std_t;

typedef struct {
  prob_t prob1, prob2, prob3;
  mean_t mean1, mean2, mean3;
  std_t std1, std2, std3;
} gmm_t;

typedef struct {
  float prob1, prob2, prob3;
  float mean1, mean2, mean3;
  float std1, std2, std3;
} gmm_f;


typedef struct {
    uint8_t* data;  // 编码后的数据指针
    size_t length;  // 数据长度
} CodingResult;

typedef struct {
    int16_t* data[13];  // 编码后的数据指针
    size_t length;  // 数据长度
} DecodingResult;


bool write_bin(uint8_t* data_addr,size_t size,char file[]);
gmm_t* read_gmmt(const char* file_path, size_t len, int gmm_scale);
gmm_f* read_gmmf(const char* file_path, size_t len);

// void mk_gmmt(gmm_t* &gmm, size_t len, int16_t xmin, int16_t xmax,int gmm_scale=10000);
CodingResult codings(gmm_t* gmms[13], int16_t* datas[13], size_t* lens,int gmm_scale=10000);
DecodingResult decoings(gmm_t* gmms[13], const char* binfile, size_t* lens,int gmm_scale=10000);
bool checks(gmm_t* gmms[13], int16_t* datas[13], size_t* lens,int gmm_scale=10000);

CodingResult codings_f(gmm_f* gmms[13], int16_t* datas[13], size_t* lens);