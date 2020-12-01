#include "AUXIVA_ICD_Online.h"
#define MAKE_FILE		1			//option 1 : wav 저장 (IVA출력 + 입력원본)		2: stdout 출력(IVA출력)		3: stdout 출력 (IVA출력 + 입력 원본)

class ProcBuffers
{
private:
	double** InitCond, * XX_LP, * XX, ** xx_lp, ** x;

	double** out_buff;
	short** IVA_out;
	double** input_temp;
	double** output;

	double** in_buff;
	short** origin_out;
	double** input;

	int BuffCnt, isNew16k;


public:
	ProcBuffers();
	~ProcBuffers();
	int Process(double **input, int Nframe, double **output);
};