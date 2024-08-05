#include"coding.h"
using namespace std;



char inputs[13][255] = {
  "D:/Code/codec/input/encode_data_LL.txt",
  "D:/Code/codec/input/encode_data_HL_3.txt",
  "D:/Code/codec/input/encode_data_LH_3.txt",
  "D:/Code/codec/input/encode_data_HH_3.txt",
  "D:/Code/codec/input/encode_data_HL_2.txt",
  "D:/Code/codec/input/encode_data_LH_2.txt",
  "D:/Code/codec/input/encode_data_HH_2.txt",
  "D:/Code/codec/input/encode_data_HL_1.txt",
  "D:/Code/codec/input/encode_data_LH_1.txt",
  "D:/Code/codec/input/encode_data_HH_1.txt",
  "D:/Code/codec/input/encode_data_HL_0.txt",
  "D:/Code/codec/input/encode_data_LH_0.txt",
  "D:/Code/codec/input/encode_data_HH_0.txt"
};



char gmmfiles[13][255] ={
  "D:/Code/codec/GMM/LL.txt",
  "D:/Code/codec/GMM/HL_3.txt",
  "D:/Code/codec/GMM/LH_3.txt",
  "D:/Code/codec/GMM/HH_3.txt",
  "D:/Code/codec/GMM/HL_2.txt",
  "D:/Code/codec/GMM/LH_2.txt",
  "D:/Code/codec/GMM/HH_2.txt",
  "D:/Code/codec/GMM/HL_1.txt",
  "D:/Code/codec/GMM/LH_1.txt",
  "D:/Code/codec/GMM/HH_1.txt",
  "D:/Code/codec/GMM/HL_0.txt",
  "D:/Code/codec/GMM/LH_0.txt",
  "D:/Code/codec/GMM/HH_0.txt"
};
void test(int gmm_scale){
    int16_t* datas[13];
    size_t lens[13];
    gmm_t* gmms[13];
    int maxs[13], mins[13];
    for(int i=0;i<13;i++){
        read_txt(inputs[i],datas[i],lens[i],maxs[i],mins[i]);
        gmms[i]=read_gmmt(gmmfiles[i],lens[i],gmm_scale);
    }

    clock_t start_time, end_time;
    float elapsed_time;

    start_time = clock();
    CodingResult enc_res = codings(gmms, datas, lens, gmm_scale);
    end_time = clock();

    elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("    coding time: %.3f s\n", elapsed_time);

    char binfile[255];
    sprintf(binfile, "D:/Code/codec/bin/gmm_sacle_%d.bin", gmm_scale);
    write_bin(enc_res.data, enc_res.length, binfile);
	float bpp = 8.0*enc_res.length/(3840*2160);
	printf("	bin_len: %ld\n", enc_res.length);
	printf("	BPP: %f\n", bpp);

    start_time = clock();
    DecodingResult dec_res = decoings(gmms, binfile, lens,gmm_scale);
    end_time = clock();
    elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("    decoding time: %.3f s\n", elapsed_time);

	int success_cnt=0;
    for(int i=0;i<13;i++){
        int16_t* enc_xs= datas[i];
        int16_t* dec_xs= dec_res.data[i];
        size_t len = lens[i];
        if(memcmp(enc_xs,dec_xs,len*sizeof(int16_t))==0) success_cnt++;
    }
	printf("    success_cnt: %d/%d\n", success_cnt, 13);


    for(int i=0;i<13;i++){
        free(datas[i]);
        free(gmms[i]);
		free(dec_res.data[i]);
    }
}

void test_f(){
    int16_t* datas[13];
    size_t lens[13];
    gmm_f* gmms[13];
    int maxs[13], mins[13];
    for(int i=0;i<13;i++){
        read_txt(inputs[i],datas[i],lens[i],maxs[i],mins[i]);
        gmms[i]=read_gmmf(gmmfiles[i],lens[i]);
    }

    clock_t start_time, end_time;
    float elapsed_time;

    start_time = clock();
    CodingResult enc_res = codings_f(gmms, datas, lens);
    end_time = clock();

    elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("    coding time: %.3f s\n", elapsed_time);

    char binfile[255];
    sprintf(binfile, "D:/Code/codec/bin/gmm_sacle_float.bin");
    write_bin(enc_res.data, enc_res.length, binfile);
	float bpp = 8.0*enc_res.length/(3840*2160);
	printf("    bin_len: %ld\n", enc_res.length);
	printf("    bpp: %f\n", bpp);

    for(int i=0;i<13;i++){
        free(datas[i]);
        free(gmms[i]);
		// free(dec_res.data[i]);
    }  
}

int main(int argc, char* argv[]){
	printf("float\n");
    test_f();
	for(int scale=10;scale<=10000;scale*=10){
		printf("scale: %d\n", scale);
		test(scale);
	}

    return 0;
}

