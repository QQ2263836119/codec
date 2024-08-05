#include <algorithm> 
#include <cstdlib>   
#include <fstream>   
#include <iostream>  
#include <istream>   
#include <ostream>  
#include <sstream>
#include <string> 
#include "coding.h"  



bool write_bin(uint8_t* data_addr,size_t size,char file[]){
	ofstream outputFile(file, ios::binary);

    if (!outputFile.is_open()) {
        cerr << "Error opening file." << endl;
        return false;
    }

    outputFile.write(reinterpret_cast<char*>(data_addr), size);
    outputFile.close();
    return true;
}

gmm_t *read_gmmt(const char *file_path, size_t len, int gmm_scale)
{
    gmm_t *gmmt = new gmm_t[len];
    FILE *file = fopen(file_path, "r");
    if (file == NULL) fprintf(stderr, "无法打开文件\n");
    float mean1, mean2, mean3, std1, std2, std3, prob1, prob2, prob3;

    for(int i=0;i<len;i++){
        if (fscanf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f", 
            &mean1, &mean2, &mean3, &std1, &std2, &std3, &prob1, &prob2, &prob3) != 9) {
            fprintf(stderr, "文件读取错误\n");
            fclose(file);
            return NULL;
        }
        gmmt[i].prob1 = gmm_scale*prob1;
        gmmt[i].prob2 = gmm_scale*prob2;
        gmmt[i].prob3 = gmm_scale*prob3;
        gmmt[i].mean1 = gmm_scale*mean1;
        gmmt[i].mean2 = gmm_scale*mean2;
        gmmt[i].mean3 = gmm_scale*mean3;
        gmmt[i].std1 = max(1,int(abs(gmm_scale*std1)));
        gmmt[i].std2 = max(1,int(abs(gmm_scale*std2)));
        gmmt[i].std3 = max(1,int(abs(gmm_scale*std3)));
    }
    fclose(file);
    return gmmt;
}

gmm_f *read_gmmf(const char *file_path, size_t len)
{
    gmm_f *gmmf = new gmm_f[len];
    FILE *file = fopen(file_path, "r");
    if (file == NULL) fprintf(stderr, "无法打开文件\n");
    float mean1, mean2, mean3, std1, std2, std3, prob1, prob2, prob3;

    for(int i=0;i<len;i++){
        if (fscanf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f", 
            &mean1, &mean2, &mean3, &std1, &std2, &std3, &prob1, &prob2, &prob3) != 9) {
            fprintf(stderr, "文件读取错误\n");
            fclose(file);
            return NULL;
        }
        gmmf[i].prob1 = prob1;
        gmmf[i].prob2 = prob2;
        gmmf[i].prob3 = prob3;
        gmmf[i].mean1 = mean1;
        gmmf[i].mean2 = mean2;
        gmmf[i].mean3 = mean3;
        gmmf[i].std1 = abs(std1);
        gmmf[i].std2 = abs(std2);
        gmmf[i].std3 = abs(std3);
    }
    fclose(file);
    return gmmf;
}

CodingResult codings(gmm_t* gmms[13], int16_t* datas[13], size_t* lens, int gmm_scale)
{
    size_t sum_len = accumulate(lens, lens + 13, 0);
    uint8_t* bins = (uint8_t*)malloc((sum_len*8)*sizeof(uint8_t));

    BitOutputStream bout(bins + 26*sizeof(int16_t));  // 从第52=13*2*2B开始
    ArithmeticEncoder enc(32, bout);

    for (int i = 0; i < 13; i++) {
        int len = lens[i];
        gmm_t* gmm = gmms[i];
        int16_t* data = datas[i];
        int16_t xmax = *max_element(data, data + len);
        int16_t xmin = *min_element(data, data + len);
        *reinterpret_cast<int16_t*>(bins + i*2*sizeof(int16_t)) = xmin;
        *reinterpret_cast<int16_t*>(bins + (i*2+1)*sizeof(int16_t)) = xmax;

        if (xmin >= xmax) continue;
        EncTable freqs_table(1000000, xmin, xmax);
        freqs_table.scale_pred = gmm_scale;
        for (int p = 0; p < len; p++, gmm++) {
            int m_probs[3] = {gmm->prob1, gmm->prob2, gmm->prob3};
            int m_means[3] = {gmm->mean1, gmm->mean2, gmm->mean3};
            int m_stds[3] = {gmm->std1, gmm->std2, gmm->std3};
            freqs_table.update(m_probs, m_means, m_stds);

            freqs_table.get_bound(data[p]);
            int total_freqs = freqs_table.total_freqs, symlow = freqs_table.sym_low, symhigh = freqs_table.sym_high;
            enc.write(total_freqs, symlow, symhigh);
        }
    }

    enc.finish();
    bout.finish();

    CodingResult result = {bins, bout.size + 26 * sizeof(int16_t)};
    return result;
}

DecodingResult decoings(gmm_t* gmms[13], const char *binfile, size_t* lens, int gmm_scale) {
    ifstream in(binfile, ios::binary);
    DecodingResult result = {nullptr, 0};
    if (!in) {
        return result;
    }

    int16_t xmins[13], xmaxs[13];

    for (int i = 0; i < 13; i++) {
        in.read(reinterpret_cast<char*>(xmins + i), sizeof(int16_t));
        in.read(reinterpret_cast<char*>(xmaxs + i), sizeof(int16_t));
    }

    BitInputStream bin(in);
    ArithmeticDecoder dec(32, bin);
    
    for (int i = 0, dec_ptr = 0; i < 13; i++) {
        int16_t xmax = xmaxs[i], xmin = xmins[i];
        if (xmin >= xmax) continue;
        gmm_t* gmm = gmms[i];
        // 网络给
        int len = lens[i];
        result.data[i] = (int16_t*)malloc(len* sizeof(int16_t));

        DecTable freqs_table(1000000, xmin, xmax);
        freqs_table.scale_pred = gmm_scale;
        for (int p = 0; p < len; p++, gmm++) {
            int m_probs[3] = {gmm->prob1, gmm->prob2, gmm->prob3};
            int m_means[3] = {gmm->mean1, gmm->mean2, gmm->mean3};
            int m_stds[3] = {gmm->std1, gmm->std2, gmm->std3};
            freqs_table.update(m_probs, m_means, m_stds);
            freqs_table.get_bounds();
            int16_t symbol = dec.read(freqs_table);
            result.data[i][p] = symbol;
        }
    }
    return result;
}

bool checks(gmm_t *gmms[13], int16_t *datas[13], size_t *lens, int gmm_scale)
{
    for (int i = 0; i < 13; i++) {
        int len = lens[i];
        gmm_t* gmm = gmms[i];
        int16_t* data = datas[i];
        int16_t xmax = *max_element(data, data + len);
        int16_t xmin = *min_element(data, data + len);
        if (xmin >= xmax) continue;
        EncTable enc_table(1000000, xmin, xmax);
        DecTable dec_table(1000000, xmin, xmax);

        for (int p = 0; p < len; p++, gmm++) {
            int m_probs[3] = {gmm->prob1, gmm->prob2, gmm->prob3};
            int m_means[3] = {gmm->mean1, gmm->mean2, gmm->mean3};
            int m_stds[3] = {gmm->std1, gmm->std2, gmm->std3};

            int m_probs2[3] = {gmm->prob1, gmm->prob2, gmm->prob3};
            int m_means2[3] = {gmm->mean1, gmm->mean2, gmm->mean3};
            int m_stds2[3] = {gmm->std1, gmm->std2, gmm->std3};            
            enc_table.update(m_probs, m_means, m_stds);
            dec_table.update(m_probs2, m_means2, m_stds2);

            enc_table.get_bound(data[p]);
            int total_freqs = enc_table.total_freqs, symlow = enc_table.sym_low, symhigh = enc_table.sym_high;

            dec_table.get_bounds();
            int dec_sym_low = dec_table.sym_low[data[p]-xmin], dec_sym_high = dec_table.sym_low[data[p] + 1-xmin],dec_total = dec_table.total_freqs;

            if(total_freqs!=dec_total || symlow!=dec_sym_low || symhigh!=dec_sym_high){
                enc_table.get_bound(data[p]);
                dec_table.get_bounds();
                printf("Enc %d: %d %d %d\n",data[p],total_freqs,symlow,symhigh);
                printf("Dec %d: %d %d %d\n",data[p],dec_total,dec_sym_low,dec_sym_high);
                printf("Error\n");
                return false;
            }
        }
    }
    return true;
}

CodingResult codings_f(gmm_f *gmms[13], int16_t *datas[13], size_t *lens)
{
    size_t sum_len = accumulate(lens, lens + 13, 0);
    uint8_t* bins = (uint8_t*)malloc((sum_len*8)*sizeof(uint8_t));

    BitOutputStream bout(bins + 26*sizeof(int16_t));  // 从第52=13*2*2B开始
    ArithmeticEncoder enc(32, bout);

    for (int i = 0; i < 13; i++) {
        int len = lens[i];
        gmm_f* gmm = gmms[i];
        int16_t* data = datas[i];
        int16_t xmax = *max_element(data, data + len);
        int16_t xmin = *min_element(data, data + len);
        *reinterpret_cast<int16_t*>(bins + i*2*sizeof(int16_t)) = xmin;
        *reinterpret_cast<int16_t*>(bins + (i*2+1)*sizeof(int16_t)) = xmax;

        if (xmin >= xmax) continue;
        EncTable_f freqs_table(1000000, xmin, xmax);
   
        for (int p = 0; p < len; p++, gmm++) {
            float m_probs[3] = {gmm->prob1, gmm->prob2, gmm->prob3};
            float m_means[3] = {gmm->mean1, gmm->mean2, gmm->mean3};
            float m_stds[3] = {gmm->std1, gmm->std2, gmm->std3};
            freqs_table.update(m_probs, m_means, m_stds);

            freqs_table.get_bound(data[p]);
            int total_freqs = freqs_table.total_freqs, symlow = freqs_table.sym_low, symhigh = freqs_table.sym_high;
            uint32_t xx= total_freqs;
            if(symlow>=symhigh||symhigh>total_freqs||xx==544004){
                freqs_table.get_bound(data[p]);
                printf("Error\n");
            }
            enc.write(total_freqs, symlow, symhigh);
        }
    }

    enc.finish();
    bout.finish();

    CodingResult result = {bins, bout.size + 26 * sizeof(int16_t)};
    return result;
}

// void mk_gmmt(gmm_t* &gmm, size_t len, int16_t xmin, int16_t xmax,int gmm_scale)
// {
//     gmm = new gmm_t[len];
//     for (int i = 0; i < len; i++)
//     {
//         gmm[i].prob1 = gmm_scale*1;
//         gmm[i].prob2 = gmm_scale*2;
//         gmm[i].prob3 = gmm_scale*1;
//         gmm[i].mean1 =gmm_scale*(xmin+ (xmax - xmin) / 4);
//         gmm[i].mean2 = gmm_scale*(xmin+ (xmax - xmin) / 2);
//         gmm[i].mean3 = gmm_scale*(xmin+ 3 * (xmax - xmin) / 4);
//         gmm[i].std1 = gmm_scale*(10);
//         gmm[i].std2 = gmm_scale*(10);
//         gmm[i].std3 = gmm_scale*(10);
//     }
// }