#include "AUXIVA_ICD_Online.h"
#define MAKE_FILE		1			//option 1 : wav 저장 (IVA출력 + 입력원본)		2: stdout 출력(IVA출력)		3: stdout 출력 (IVA출력 + 입력 원본)

class ProcBuffers
{
private:
	

public:
	ProcBuffers();
	~ProcBuffers();
	static int Process(double **input, int Nframe, double **output);
};