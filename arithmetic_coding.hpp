#pragma once

#include <algorithm>
#include <istream>
#include <ostream>
#include <vector>
#include<math.h>
#include <thread>
#include <mutex>
using namespace std;



// ****** 码流IO
class BitInputStream final {
	private: 
		istream &input;
		int currentByte;
		int numBitsRemaining; 
	public: 
		int read();
		int readNoEof();
		explicit BitInputStream(istream &in);
};

class BitOutputStream final {
	private: 
		int currentByte;
		int numBitsFilled;
	public:
		uint8_t* data_addr;
		size_t size=0;
		BitOutputStream(uint8_t* _addr);	
	    void write(int b);
		void finish();
};


// 函数 遍历txt文件(数据为每行一个)，返回对应的数组，数组长度，数据最大值，最小值
void read_txt(char file_path[],int16_t* &data, size_t &size, int &max, int &min);




class EncTable{
public:
	char exp_file_path[255] = "D:/Code/codec/data/exp.bin";
	char cdf_file_path[255] = "D:/Code/codec/data/cdf.bin";
	// 表数据
    uint16_t* exp_table = nullptr;
    size_t exp_size = 0;
    uint32_t* cdf_table = nullptr;
    size_t cdf_size = 0;	\
	// 表信息
	int exp_scale=1000,exp_x_bound=-12;
	int cdf_scale=10000,cdf_x_bound=-5;
	int scale_pred =10000;

	// 数据边界,左闭右闭
    int low_bound=0,high_bound=65536;
	long long freqs_resolution=10000000;

	// 要返回的结果
	uint32_t sym_low,sym_high,total_freqs;

	// f(low_bound-0.5)和f(high_bound+0.5)的值
	uint64_t l_bound = 0, r_bound = 0;

	// GMM参数
	int* probs=nullptr;
	int* means=nullptr;
	int* stds=nullptr;
	uint32_t prob_sum=0;

	EncTable(uint32_t freqs_resolution,int _low_bound,int _high_bound);
	void update(int m_probs[3],int m_means[3],int m_stds[3]);
	void get_bound(int x);
	// 析构函数
	~EncTable();
};




class DecTable{
public:
	// 文件路径
	char exp_file_path[255] = "D:/Code/codec/data/exp.bin";
	char cdf_file_path[255] = "D:/Code/codec/data/cdf.bin";
	// 表数据
    uint16_t* exp_table = nullptr;
    size_t exp_size = 0;
    uint32_t* cdf_table = nullptr;
    size_t cdf_size = 0;	\
	// 表信息
	int exp_scale=1000,exp_x_bound=-12;
	int cdf_scale=10000,cdf_x_bound=-5;
	int scale_pred = 10000;
	// 数据边界,左闭右闭
    int low_bound=0,high_bound=65536,freqs_resolution=1000000;

	// 要返回的结果
	int* sym_low=nullptr,total_freqs=0;
	// GMM参数
	int* probs=nullptr;
	int* means=nullptr;
	int* stds=nullptr;
	int prob_sum=0;

	DecTable(int freqs_resolution,int _low_bound,int _high_bound);
	void update(int m_probs[3],int m_means[3],int m_stds[3]);
	void get_bounds();
	// 析构函数
	~DecTable();
};





// ****** 编码器
class ArithmeticCoderBase {
	protected: int numStateBits;
		uint64_t fullRange;
		uint64_t halfRange;
		uint64_t quarterRange;
		uint64_t minimumRange;
		uint64_t maximumTotal;
		uint64_t stateMask;
		uint64_t low;
		uint64_t high;
	public: 
		explicit ArithmeticCoderBase(int numBits);
		virtual ~ArithmeticCoderBase() = 0;
		void update(uint32_t total, uint32_t symlow, uint32_t symhigh);
		virtual void shift() = 0;
		virtual void underflow() = 0;
};


class ArithmeticDecoder final : private ArithmeticCoderBase {
	private: 
		BitInputStream &input;
		uint64_t code;
	public: 
		explicit ArithmeticDecoder(int numBits, BitInputStream &in);
		int read(DecTable &freqs);
		void shift() override;
		void underflow() override;
		int readCodeBit();
};


class ArithmeticEncoder final : private ArithmeticCoderBase {
	public: 
		BitOutputStream &output;
		unsigned long numUnderflow;
		explicit ArithmeticEncoder(int numBits, BitOutputStream &out);
		void write(uint32_t total, uint32_t symLow, uint32_t symHigh);
		void finish();
		void shift() override;
		void underflow() override;
};

class EncTable_f{
public:
    int low_bound=0,high_bound=65536;
	long long freqs_resolution=1000000;
	uint32_t sym_low,sym_high,total_freqs;
	uint64_t l_bound = 0, r_bound = 0;

	// GMM参数
	float* probs=nullptr;
	float* means=nullptr;
	float* stds=nullptr;
	uint32_t prob_sum=0;

	EncTable_f(int freqs_resolution,int _low_bound,int _high_bound);
	void update(float m_probs[3],float m_means[3],float m_stds[3]);
	void get_bound(int x);
	~EncTable_f();
};




class DecTable_f{
public:
	// 数据边界,左闭右闭
    uint32_t low_bound=0,high_bound=65536,freqs_resolution=1000000;

	// 要返回的结果
	uint32_t* sym_low=nullptr,*sym_high=nullptr,total_freqs=0;
	int l_bound = 0, r_bound = 0;
	// GMM参数
	float* probs=nullptr;
	float* means=nullptr;
	float* stds=nullptr;
	uint32_t prob_sum=0;

	DecTable_f(uint32_t freqs_resolution,int _low_bound,int _high_bound);
	void update(float m_probs[3],float m_means[3],float m_stds[3]);
	void get_bounds();
	// 析构函数
	~DecTable_f();
};
