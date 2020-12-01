/*
duplex.cpp
by Gary P. Scavone, 2006-2007.

This program opens a duplex stream and passes
input directly through to the output.
*/
/******************************************/

#include "RtAudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <io.h>
#include <iostream>
#include "header.h"
#include "ProcBuffers.h"
#include <thread>
#include <atomic>

/*
typedef char MY_TYPE;
#define FORMAT RTAUDIO_SINT8
*/


typedef signed short MY_TYPE;
#define FORMAT RTAUDIO_SINT16

std::atomic<int> record_num;
std::atomic<int> copyend;

/*
typedef S24 MY_TYPE;
#define FORMAT RTAUDIO_SINT24

typedef signed long MY_TYPE;
#define FORMAT RTAUDIO_SINT32

typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32

typedef double MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64
*/

// Platform-dependent sleep routines.
#if defined( __WINDOWS_ASIO__ ) || defined( __WINDOWS_DS__ ) || defined( __WINDOWS_WASAPI__ )
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

void usage(void) {
	// Error function in case of incorrect command-line
	// argument specifications
	std::cout << "\nuseage: duplex N fs <iDevice> <oDevice> <iChannelOffset> <oChannelOffset>\n";
	std::cout << "    where N = number of channels,\n";
	std::cout << "    fs = the sample rate,\n";
	std::cout << "    iDevice = optional input device to use (default = 0),\n";
	std::cout << "    oDevice = optional output device to use (default = 0),\n";
	std::cout << "    iChannelOffset = an optional input channel offset (default = 0),\n";
	std::cout << "    and oChannelOffset = optional output channel offset (default = 0).\n\n";
	exit(0);
}

struct InputData {
	MY_TYPE* in_buffer;
	MY_TYPE* out_buffer;
	MY_TYPE* z_buffer;
	unsigned long bufferBytes;
	unsigned long totalFrames;
	unsigned long iframeCounter;
	unsigned long oframeCounter;
	unsigned int channels;
};


// �ǽð����� MIC�� ���� �Է��� ���ۿ� �����ϰ� ������ �����͸� processing���� ó���� �� �ǽð����� ����Ѵ�.
int inout(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
	InputData *iData = (InputData *)data;

	////�ǽð����� �Է��� �޴� �κ�
	unsigned int frames = nBufferFrames;
	//�ִ� ���ۻ���� ���� ��� ũ�⸦ �Ѵ� �����ʹ� �������� �ʴ´�.
	if (iData->iframeCounter + nBufferFrames > iData->totalFrames)
	{
		frames = iData->totalFrames - iData->iframeCounter;
		iData->bufferBytes = frames * iData->channels * sizeof(MY_TYPE);
	}
	unsigned long in_offset = iData->iframeCounter * iData->channels;
	//����� ���۰� �����ϸ� �̸� process�ϱ� ���� �����ϸ� ó���� �����Ͱ� �����Ѵٰ� record_num���� Ȯ���Ѵ�.
	memcpy(iData->in_buffer + in_offset, inputBuffer, iData->bufferBytes);
	iData->iframeCounter += frames;
	record_num.fetch_add(1);

	//����� �����Ͱ� �ִ� ���ۻ���� �ѱ�� �ٽ� offset�� 0���� �Ͽ� overwriting�Ѵ�.
	//�̹� ���� �����ʹ� process�� ���� ����Ǿ���.
	if (iData->iframeCounter >= iData->totalFrames)
	{
		return 2; //���� �ð�(32��)�� �� �Ǹ� ���α׷��� ���ߵ��� �����Ͽ���.
		iData->iframeCounter = 0;
	}

	//process���� ó���� �����Ͱ� �ǽð� ����� ���� ó�� �� �߻��ϴ� copyend��� ��ȣ�� �߸� callback�Լ��� output���� �����Ѵ�.
	if (copyend.load())
	{
		if (iData->oframeCounter + nBufferFrames > iData->totalFrames)
		{
			frames = iData->totalFrames - iData->oframeCounter;
			iData->bufferBytes = frames * iData->channels * sizeof(MY_TYPE);
		}
		unsigned long out_offset = iData->oframeCounter * iData->channels;
		memcpy(outputBuffer, iData->out_buffer + out_offset, iData->bufferBytes);
		iData->oframeCounter += frames;
		//ó���� �����Ͱ� ��µǸ� copyend���� ���δ�.
		copyend.fetch_sub(1);
		if (iData->oframeCounter >= iData->totalFrames)
		{
			iData->oframeCounter = 0;
		}
	}
	//ó���� �����Ͱ� ������ (��ȭ������ ���°��) 0�� ����Ѵ�.
	else if (copyend.load() == 0)
	{
		memcpy(outputBuffer, iData->z_buffer, iData->bufferBytes);
	}
	return 0;
}
bool fF4 = false;
bool fF5 = false;
bool fF6 = false;

void Keyboard_interrupt()
{
	while (true)
	{
		SLEEP(50);
		if (GetKeyState('S') < 0 && !fF4) // Stop
		{
			fF4 = true;
		}
		if (GetKeyState('A') < 0 && !fF5) // Begin
		{
			fF5 = true;
		}
		if (GetKeyState('D') < 0 && !fF6) // Quit
		{
			fF6 = true;
			return;
		}
	}
}

int main(void)
{
	double time = 2048;
	std::thread t1(Keyboard_interrupt);
	while (true)
	{
		while (true)
		{
			SLEEP(50);
			if (fF5 == true)
			{
				SLEEP(200);
				fF5 = false;
				fF4 = false;
				if (MAKE_FILE == 1) std::cout << "... Recording and Processing has begun" << std::endl;
				break;
			}
			if (fF4 == true)
			{
				fF4 = false;
			}
			if (fF6 == true)
			{
				SLEEP(200);
				fF6 = false;
				t1.join();
				return 0;
			}
		}

		record_num.store(0);
		copyend.store(0);
		int in_buffer_cnt = 0;
		int out_buffer_cnt = 0;
		int i, j, ch;
		int proc_end = 0;
		int proc_count = 0;
		unsigned int channels, fs, bufferBytes, oDevice = 0, iDevice = 0, iOffset = 0, oOffset = 0;

		double **input, **proc_output;

		input = new double* [Nch];
		proc_output = new double* [Nch];
		for (i = 0; i < Nch; i++)
		{
			input[i] = new double[BufferSize];
			proc_output[i] = new double[3 * BufferSize];
			for (j = 0; j < BufferSize; j++)
			{
				input[i][j] = 0.0;
			}
		}
		ProcBuffers* proc;
		proc = new ProcBuffers();

		RtAudio adac;
		if (adac.getDeviceCount() < 1) {
			std::cout << "\nNo audio devices found!\n";
			exit(1);
		}
		channels = Nch;
		fs = 48000;

		adac.showWarnings(true);

		// Set the same number of channels for both input and output.
		unsigned int bufferFrames = BufferSize;
		RtAudio::StreamParameters iParams, oParams;
		iParams.deviceId = iDevice;
		iParams.nChannels = channels;
		iParams.firstChannel = iOffset;
		oParams.deviceId = oDevice;
		oParams.nChannels = channels;
		oParams.firstChannel = oOffset;

		if (iDevice == 0)
			iParams.deviceId = adac.getDefaultInputDevice();
		if (oDevice == 0)
			oParams.deviceId = adac.getDefaultOutputDevice();

		RtAudio::StreamOptions options;
		//options.flags |= RTAUDIO_NONINTERLEAVED;

		InputData data;
		data.in_buffer = 0;
		data.out_buffer = 0;
		data.z_buffer = 0;

		//���� inout�̶�� callback�Լ��� ��Ʈ���� �ϱ� ���� ���� argument�� �Է��ϰ� open�Ѵ�.
		try {
			adac.openStream(&oParams, &iParams, FORMAT, fs, &bufferFrames, &inout, (void*)&data, &options);
		}
		catch (RtAudioError & e) {
			std::cout << '\n' << e.getMessage() << '\n' << std::endl;
			exit(1);
		}

		data.bufferBytes = bufferFrames * channels * sizeof(MY_TYPE);
		data.totalFrames = (unsigned long)(fs * time);
		data.iframeCounter = 0;
		data.oframeCounter = 0;
		data.channels = channels;
		unsigned long totalBytes;
		totalBytes = data.totalFrames * channels * sizeof(MY_TYPE);

		// Allocate the entire data buffer before starting stream.
		data.in_buffer = (MY_TYPE*)malloc(totalBytes);
		data.out_buffer = (MY_TYPE*)malloc(totalBytes);
		data.z_buffer = new MY_TYPE[BufferSize * channels];
		for (i = 0; i < BufferSize * channels; i++)
		{
			data.z_buffer[i] = 0.0;
		}

		if (data.in_buffer == 0 || data.out_buffer == 0) {
			std::cout << "Memory allocation error ... quitting!\n";
			goto cleanup;
		}

		//�غ�� callback�Լ��� streaming �����Ѵ�.
		try {
			adac.startStream();

		}
		catch (RtAudioError & e) {
			std::cout << '\n' << e.getMessage() << '\n' << std::endl;
			goto cleanup;
		}

		//streaming�� ���鼭 ���� callback(inout)�Լ����� 
		while (adac.isStreamRunning())
		{
			if (record_num.load())
			{
				for (ch = 0; ch < channels; ch++)
				{
					for (i = 0; i < bufferFrames; i++)
					{
						input[ch][i] = (data.in_buffer[channels * i + ch + in_buffer_cnt]) / 32768.0; //input�ڷ����� �°� ��ȯ�Ͽ� ����
					}
				}
				in_buffer_cnt += bufferFrames * channels;
				proc_count++;
				//Process���� ��ȭ���������� proc_end�� 1�� return�Ѵ�. ��ȭ������ �ƴϸ� 0�� return�Ѵ�.
				proc_end = proc->Process(input, proc_count, proc_output);

				//��ȭ�������� ó���� �����Ϳ� ���� ����� ���� callback�Լ��� outbuffer�� �־��ش�.
				if (proc_end == 1)
				{
					proc_end = 0;
					for (ch = 0; ch < channels; ch++)
					{
						for (i = 0; i < bufferFrames; i++)
						{
							data.out_buffer[channels * (3 * i) + ch + out_buffer_cnt] = (MY_TYPE)(proc_output[ch][i] * 32768.0); //input�ڷ����� �°� ��ȯ�Ͽ� ����
							data.out_buffer[channels * (3 * i + 1) + ch + out_buffer_cnt] = (MY_TYPE)(proc_output[ch][i] * 32768.0); //input�ڷ����� �°� ��ȯ�Ͽ� ����
							data.out_buffer[channels * (3 * i + 2) + ch + out_buffer_cnt] = (MY_TYPE)(proc_output[ch][i] * 32768.0); //input�ڷ����� �°� ��ȯ�Ͽ� ����
						}
					}
					out_buffer_cnt += 3 * bufferFrames * channels;
					copyend.fetch_add(3);
				}
				record_num.fetch_sub(1);
			}
			else
			{
				SLEEP(4);
			}
			if (fF4 == true || fF6 == true)
			{
				SLEEP(100);
				fF4 = false;
				fF5 = false;
				if (MAKE_FILE == 1) std::cout << "... Recording and Processing have stopped" << std::endl;
				break;
			}
		}
		delete proc;

		for (i = 0; i < Nch; i++)
		{
			delete[] input[i];
			delete[] proc_output[i];
		}
		delete[] input;
		delete[] proc_output;

	cleanup:
		if (adac.isStreamOpen()) adac.closeStream();
	

		delete data.in_buffer;
		delete data.out_buffer;
		if (fF6 == true)
		{
			SLEEP(200);
			t1.join();
			if (MAKE_FILE == 1) std::cout << "... Program will be closed" << std::endl;
			return 0;
		}
	}
}
