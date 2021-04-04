// Palimuvi.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <windows.h>
#include "SMX.h"
#include <memory>
#include <string>
using namespace std;

class InputSample
{
public:
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
};

int main()
{
	InputSample demo;
	int i = 0;
	// Loop forever for this sample.
	while (1)
	{
		
		Sleep(50);

		demo.SetLights(i);
		i++;
		if (i > 255) {
			i = 0;
		}
	}

	return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
