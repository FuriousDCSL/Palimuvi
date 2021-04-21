// Palimuvi.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "SMX.h"
#include <string>
#include "bass.h"
#include "basswasapi.h"
using namespace std;


BASS_WASAPI_INFO info;



class InputSample
{
public:
	char lights[1350];

	InputSample()
	{
		// Set a logging callback.  This can be called before SMX_Start.
		// SMX_SetLogCallback( SMXLogCallback );

		// Start scanning.  The update callback will be called when devices connect or
		// disconnect or panels are pressed or released.  This callback will be called
		// from a thread.
		SMX_Start(SMXStateChangedCallback, this);
	}

	static void SMXStateChangedCallback(int pad, SMXUpdateCallbackReason reason, void* pUser)
	{
		InputSample* pSelf = (InputSample*)pUser;
		pSelf->SMXStateChanged(pad, reason);
	}

	static void SMXLogCallback(const char* log)
	{
		printf("-> %s\n", log);
	}

	void SMXStateChanged(int pad, SMXUpdateCallbackReason reason)
	{
		printf("Device %i state changed: %04x\n", pad, SMX_GetInputState(pad));

	}

	uint16_t SetLight(uint8_t player, uint8_t panel, uint8_t light, uint8_t red, uint8_t blue, uint8_t green) {
		uint16_t loc = player * 675;
		loc += panel * 75;
		loc += light * 3;

		lights[loc] = red;
		lights[loc + 1] = blue;
		lights[loc + 2] = green;

		return loc;
	}

	int iPanelToLight = 0;
	void SetLights(int color)
	{
		string sLightsData;
		auto addColor = [&sLightsData](uint8_t r, uint8_t g, uint8_t b)
		{
			sLightsData.append(1, r);
			sLightsData.append(1, g);
			sLightsData.append(1, b);
		};
		for (int iPad = 0; iPad < 2; ++iPad)
		{
			for (int iPanel = 0; iPanel < 9; ++iPanel)
			{
				bool bLight = iPanel == iPanelToLight && iPad == 0;
				if (!bLight)
				{
					// We're not lighting this panel, so append black for the 4x4 and 3x3 lights.
					for (int iLED = 0; iLED < 25; ++iLED)
						addColor(color, color, color);
					continue;
				}

				// Append light data for the outer 4x4 grid of lights.
				addColor(0xFF, 0, 0);
				addColor(0xFF, 0, 0);
				addColor(0xFF, 0, 0);
				addColor(0xFF, 0, 0);
				addColor(0, 0xFF, 0);
				addColor(0, 0xFF, 0);
				addColor(0, 0xFF, 0);
				addColor(0, 0xFF, 0);
				addColor(0, 0, 0xFF);
				addColor(0, 0, 0xFF);
				addColor(0, 0, 0xFF);
				addColor(0, 0, 0xFF);
				addColor(0xFF, 0xFF, 0);
				addColor(0xFF, 0xFF, 0);
				addColor(0xFF, 0xFF, 0);
				addColor(0xFF, 0xFF, 0);

				// Append light data for the inner 3x3 grid of lights, if present.  These
				// are ignored if the platform doesn't have them.
				addColor(0xFF, 0, 0);
				addColor(0xFF, 0, 0);
				addColor(0xFF, 0, 0);
				addColor(0, 0xFF, 0);
				addColor(0, 0xFF, 0);
				addColor(0, 0xFF, 0);
				addColor(0, 0, 0xFF);
				addColor(0, 0, 0xFF);
				addColor(0, 0, 0xFF);
			}
		}

		SMX_SetLights2(sLightsData.data(), sLightsData.size());
	}
	void updateLights() {
		SMX_SetLights2(lights, 1350);

	}
	void clearLights() {
		for (int i = 0; i < 1350; i++) {
			lights[i] = 0;
		}
		updateLights();

	}
};



DWORD CALLBACK DuffRecording(void* buffer, DWORD length, void* user)
{
	return TRUE; // continue recording
}

void Error(const char* text)
{
	printf("Error(%d): %s\n", BASS_ErrorGetCode(), text);
	BASS_Free();
	exit(0);
}

void ListDevices()
{
	BASS_DEVICEINFO di;
	int a;
	for (a = 0; BASS_GetDeviceInfo(a, &di); a++) {
		if (di.flags & BASS_DEVICE_ENABLED) // enabled output device
			printf("dev %d: %s\n", a, di.name);
	}
}

float sumSamples(float* samples, int low, int high) {
	float sum = 0;
	for (int i = low; i < high; i++) {
		sum += samples[i];
	}
	if (sum > 1) {
		sum = 1;
	}
	return sum;
}

float* getSample(void) {

	float fft[1024];
	static float fsamSum[9];

	BASS_WASAPI_GetData(fft, BASS_DATA_FFT2048); // get the FFT data
	fsamSum[0] = sumSamples(fft, 0, 4);
	fsamSum[1] = sumSamples(fft, 4, 8);
	fsamSum[2] = sumSamples(fft, 8, 16);
	fsamSum[3] = sumSamples(fft, 16, 32);
	fsamSum[4] = sumSamples(fft, 32, 64);
	fsamSum[5] = sumSamples(fft, 64, 128);
	fsamSum[6] = sumSamples(fft, 128, 256);
	fsamSum[7] = sumSamples(fft, 256, 512);
	fsamSum[8] = sumSamples(fft, 512, 1024);

	return fsamSum;

}

int burst[5][25] = {

	//	00	01	02	03	04	05	06	07	08	09	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24
	{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
	{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0},
	{	0,	0,	0,	0,	0,	1,	1,	0,	0,	1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0},
	{	0,	0,	0,	0,	0,	1,	1,	0,	0,	1,	1,	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1},
	{	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1},
};



InputSample demo;


void burstLight(int panel, int level) {

	for (int i = 0; i < 25; i++) {
		int player = 1;
		demo.SetLight(1, panel, i, burst[level][i] * 255, 0, 0);
		//		printf("%d\t%d\t%d\t%d\n",player,panel,i, burst[level][i]);
	}
	return;
}


void myBassInit(void) {
	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		Error("An incorrect version of BASS was loaded");
		return;
	}


	BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1); // enable playlist processing
	BASS_SetConfig(BASS_CONFIG_NET_PREBUF_WAIT, 0); // disable BASS_StreamCreateURL pre-buffering

	// initialize output device
	if (!BASS_Init(0, 44100, 0, 0, NULL))
		printf("Can't initialize device");
	if (!BASS_WASAPI_Init(-3, 0, 0, BASS_WASAPI_BUFFER, 1, 0.1, &DuffRecording, NULL)
		&& !BASS_WASAPI_Init(-2, 0, 0, BASS_WASAPI_BUFFER, 1, 0.1, &DuffRecording, NULL)) {
		Error("Can't initialize WASAPI device");
	}


	BASS_WASAPI_GetInfo(&info);
	BASS_WASAPI_Start();

}


void main(int argc, char** argv) {

	int filep, device = -1;

	printf("Palimuvi\n"
		"--------------------------\n");
	for (filep = 1; filep < argc; filep++) {
		if (!strcmp(argv[filep], "-l")) {
			ListDevices();
			return;
		}
		else if (!strcmp(argv[filep], "-d") && filep + 1 < argc) device = atoi(argv[++filep]);
		else break;
	}

	myBassInit();

	demo.clearLights();

	while (1)
	{

		float* fsamSum;
		fsamSum = getSample();
		for (int i = 0; i < 9; i++) {
			//				printf("%d\t", int(fsamSum[i] * 255));
			if (fsamSum[i] < 0.01) {
				//				printf("0\t");
				burstLight(i, 0);
			}
			else if (fsamSum[i] < 0.25) {
				//				printf("1\t");
				burstLight(i, 1);
			}
			else if (fsamSum[i] < 0.5) {
				//				printf("2\t");
				burstLight(i, 2);
			}
			else if (fsamSum[i] < 0.75) {
				//				printf("3\t");
				burstLight(i, 3);
			}
			else if (fsamSum[i] <= 1) {
				//				printf("4\t");
				burstLight(i, 4);
			}
		}
		//		printf("\n");
		demo.updateLights();
		Sleep(30);
	}
}

