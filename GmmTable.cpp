#include"GmmTable.h"
#include <iostream>
#include <math.h>
#include<algorithm>

double GmmTable::normal_cdf(double index, double mean, double std) {
    return 1.0 / 2 * (1 + erf((index - mean) / std / sqrt(2)));
}


int64_t clamp(int64_t x,int64_t l,int64_t r){
    if(x<l) return l;
    else if (x>r) return r;
    else return x;
}


uint32_t softmax(uint16_t probs[3],uint16_t exp_table[],int scale=1000,int x_bound=-12){
    int mx=*max_element(probs,probs+3);
    uint32_t sum=0;
    int base=-x_bound*scale;

    for(int i=0;i<3;i++) {
        // 将idx 数值限定在[x_bound,0]
        int idx=probs[i];
        idx-=mx;
        if(probs[i]<x_bound) probs[i]=x_bound;

        //映射到exp
        idx= idx*scale +base;
        probs[i]= exp_table[idx];
        sum+=probs[i];
    }
    return sum;
}



int cnt(int x,uint32_t cdf_table[],uint16_t probs[3],uint32_t prob_sum,uint16_t means[3],uint16_t stds[3],uint32_t resolution=1e6,int scale=10000,int x_bound=-5){
    uint32_t y_bound=UINT32_MAX;
    int base=-x_bound*scale;
    __uint128_t cnt=0;

    for(int i=0;i<3;i++){
        int idx_l=((-0.5+x)-means[i])*scale/stds[i];
        idx_l =clamp(idx_l,-base,base);
        idx_l += base;

        int idx_r=((0.5+x)-means[i])*scale/stds[i];
        idx_r =clamp(idx_r,-base,base);
        idx_r +=base;
        uint64_t tr=cdf_table[idx_r],tl=cdf_table[idx_l];
        uint32_t cdf=cdf_table[idx_r]-cdf_table[idx_l];
        //分辨率*权值*概率
        cnt+= __uint128_t(1)*resolution*probs[i]*cdf;
    }

    cnt/=prob_sum;//probs[i]
    cnt/=y_bound;//cdf
    return cnt;
}



GmmTable::GmmTable (gmm_t* gmm,uint32_t _low_bound,uint32_t _high_bound,uint16_t exp_table[],uint32_t cdf_table[]){
    low_bound=_low_bound;
    high_bound=_high_bound;//左闭右闭
    uint32_t freqs_resolution = gmm->freqs_resolution; 

    symlow.resize(high_bound + 2);
    symhigh.resize(high_bound + 2);

    uint16_t m_probs[3]={gmm->prob1, gmm->prob2,  gmm->prob3};
    uint16_t m_means[3]={gmm->mean1, gmm->mean2, gmm->mean3};
    uint16_t m_stds[3]={gmm->std1, gmm->std2, gmm->std3};

    uint32_t prob_sum= softmax(m_probs,exp_table);


    int n_gauss=3;

    for(int i=low_bound;i<=high_bound+1;i++){
        int dif=cnt(i,cdf_table,m_probs,prob_sum,m_means,m_stds,freqs_resolution);
        dif=max(dif,1);
        total_freqs+=dif;
        symhigh[i]=total_freqs;
    }
}

uint32_t GmmTable::getSymbolLimit() const{
	return static_cast<uint32_t>(symlow.size());
}

