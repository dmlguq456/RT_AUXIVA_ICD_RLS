#include "AUXIVA_ICD_Online.h"
#define MAKE_FILE		1			//option 1 : wav ���� (IVA��� + �Է¿���)		2: stdout ���(IVA���)		3: stdout ��� (IVA��� + �Է� ����)

class ProcBuffers
{
private:
	

public:
	ProcBuffers();
	~ProcBuffers();
	static int Process(double **input, int Nframe, double **output);
};