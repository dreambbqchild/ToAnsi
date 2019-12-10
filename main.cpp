#define MAGICKCORE_QUANTUM_DEPTH 16 
#define MAGICKCORE_HDRI_ENABLE 1

#include <iostream>
#include <memory>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <Magick++.h>
#include <MagickCore/MagickCore.h>

using namespace std;

const char* TwoPixels = "▀";

struct Pixel 
{
	bool IsDefault;
	uint8_t R, G, B;

	Pixel() : IsDefault(true), R(0), G(0), B(0) {}
	Pixel(uint8_t r, uint8_t g, uint8_t b) : IsDefault(false), R(r), G(g), B(b) {}
};

struct Bitmap 
{ 
	const uint32_t Width, Height; 
	vector<Pixel> EvenRowPixels;
	vector<Pixel> OddRowPixels;

	Bitmap(uint32_t width, uint32_t height) : Width(width), Height(height) { }
};

inline void Clear()
{
	cout << "\x1b[2J\x1b[H";
	cout.flush();
}

inline void SetColor(short ansiCode, Pixel& p)
{
	cout << "\x1b[" << ansiCode << ";2;" << (short)p.R << ";" << (short)p.G << ";" << (short)p.B << "m";
}

inline void SetForeground(Pixel& p)
{
	static Pixel lastValue;
	if (memcmp(&p, &lastValue, sizeof(Pixel)))
	{
		SetColor(38, p);
		lastValue = p;
	}
}

inline void SetBackground(Pixel& p)
{
	static Pixel lastValue;
	if (memcmp(&p, &lastValue, sizeof(Pixel)))
	{
		SetColor(48, p);
		lastValue = p;
	}
}

inline void Reset()
{
	cout << "\x1b[0m";
}

inline void NewLine() 
{
	cout << endl;
}

void RenderPixels(Pixel& top, Pixel& bottom)
{
	SetForeground(top);
	SetBackground(bottom);
	cout << TwoPixels;	
}

Bitmap* LoadImage(const char* path) 
{
	unique_ptr<Magick::Image> bmp(new Magick::Image());
	bmp->read(path);

	auto bounds = bmp->size();
	auto result = new Bitmap(bounds.width(), bounds.height());
	auto pixels = bmp->getConstPixels(0, 0, result->Width, result->Height);
	auto index = 0;	

	for (auto row = 0; row < result->Height; row++)
	{
		for (auto col = 0; col < result->Width; col++)
		{			
			MagickCore::Quantum r = 0, g = 0, b = 0;
			for (auto channelIndex = 0; channelIndex < bmp->channels(); channelIndex++)
			{
				auto channel = MagickCore::GetPixelChannelChannel(bmp->image(), channelIndex);
				switch (channel)
				{
				case MagickCore::PixelChannel::RedPixelChannel:
					r = pixels[index];
					break;
				case MagickCore::PixelChannel::GreenPixelChannel:
					g = pixels[index];
					break;
				case MagickCore::PixelChannel::BluePixelChannel:
					b = pixels[index];
					break;
				default:
					break;
				}
				index++;
			}

			Pixel px(
				MagickCore::ScaleQuantumToChar(r),
				MagickCore::ScaleQuantumToChar(g),
				MagickCore::ScaleQuantumToChar(b)
			);

			if(row % 2 == 0)
				result->EvenRowPixels.push_back(px);
			else
				result->OddRowPixels.push_back(px);		
		}
	}

	return result;
}

void PrintAsAnsi(Bitmap* bmp)
{
	auto index = 0;
	for (auto row = 0; row < bmp->Height; row += 2)
	{
		for (auto col = 0; col < bmp->Width; col++)
		{
			auto top = bmp->EvenRowPixels.at(index);
			auto bottom = index < bmp->OddRowPixels.size() ? bmp->OddRowPixels.at(index) : Pixel();
			index++;

			RenderPixels(top, bottom);
		}
		NewLine();
	}
}

int main(int argc, char* argv[])
{
	if (argc <= 1)
		return 0;
	
	Clear();
	unique_ptr<Bitmap> bmp(LoadImage(argv[1]));
	PrintAsAnsi(bmp.get());
	
	NewLine();
	Reset();
	 
	return 0;
}