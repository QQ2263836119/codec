#pragma once

#include <cstdint>
#include <vector>
#include<math.h>
using namespace std;

// 定义结构体
typedef struct {
    double* probs;
    double* means;
    double* stds;
    int len;
    uint32_t freqs_resolution;
} Gmm;

typedef double prob_t;
typedef double mean_t;
typedef mean_t std_t;

typedef struct {
  prob_t prob1, prob2, prob3;
  mean_t mean1, mean2, mean3;
  std_t std1, std2, std3;
  uint32_t freqs_resolution;
} gmm_t;

Gmm initGmm(double* _probs, double* _means, double* _stds,int _len, uint32_t _freqs_resolution);

class GmmTable{
public:
    vector<uint32_t> symlow,symhigh;
    uint64_t total_freqs=0;
    uint32_t low_bound=0,high_bound=65536;
public:
    double normal_cdf(double index, double mean, double std);
    GmmTable (gmm_t* gmm,uint32_t _low_bound=0,uint32_t _high_bound=65536);
    std::uint32_t getSymbolLimit() const;
};

