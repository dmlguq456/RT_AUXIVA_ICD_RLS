#include "AUXIVA_ICD_Online.h"
#define MAKE_FILE		1			//option 1 : wav ���� (IVA��� + �Է¿���)		2: stdout ���(IVA���)		3: stdout ��� (IVA��� + �Է� ����)

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