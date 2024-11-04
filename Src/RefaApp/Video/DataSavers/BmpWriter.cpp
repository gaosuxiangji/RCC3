//#include "stdafx.h"
#include "BmpWriter.h"
#include <iostream>
#include <fstream>
struct PaletteEntry
{
	uint8_t b, g, r, a;
};

enum BmpCompression
{
	BMP_RGB = 0,
	BMP_RLE8 = 1,
	BMP_RLE4 = 2,
	BMP_BITFIELDS = 3
};

static const char* fmtSignBmp = "BM";

CBmpWriter::CBmpWriter()
{
}


CBmpWriter::~CBmpWriter()
{
}

void  FillGrayPalette(PaletteEntry* palette, int bpp, bool negative = false)
{
	int i, length = 1 << bpp;
	int xor_mask = negative ? 255 : 0;

	for (i = 0; i < length; i++)
	{
		int val = (i * 255 / (length - 1)) ^ xor_mask;
		palette[i].b = palette[i].g = palette[i].r = (uchar)val;
		palette[i].a = 0;
	}
}

bool CBmpWriter::write(const std::string & filename, const cv::Mat & img, uint16_t device_index, uint16_t device_state, const BitmapExtendInfo & extend_info)
{
	uint32_t size = sizeof(extend_info);
	std::vector<uint8_t> custom_data(size);
	memcpy(custom_data.data(), &extend_info, size);
	return write(filename, img, device_index, device_state, custom_data);
}

bool CBmpWriter::write(const std::string & filename, const cv::Mat & img, uint16_t bf_res1, uint16_t bf_res2, const std::vector<uint8_t> & custom_data)
{
	std::ofstream out(filename, std::ios::out | std::ios::binary);
	if (!out.is_open())
	{
		return false;
	}

	uint32_t width = img.cols, height = img.rows;
	uint16_t channels = img.channels();
	int fileStep = (width*channels + 3) & -4;
	uchar zeropad[] = "\0\0\0\0";

	uint32_t  bitmapHeaderSize = 40;
	uint32_t  paletteSize = channels > 1 ? 0 : 1024;
	uint32_t  bf_offbits = 14 /* fileheader */ + bitmapHeaderSize + paletteSize + custom_data.size();
	uint32_t fileSize = fileStep*height + bf_offbits;
	PaletteEntry palette[256];

	BITMAPFILEHEADER_ bitmap_file_header{ 0 };
	bitmap_file_header.bfType = 0x4D42;
	bitmap_file_header.bfSize = fileSize;
	bitmap_file_header.bfReserved1 = bf_res1;
	bitmap_file_header.bfReserved2 = bf_res2;
	bitmap_file_header.bfOffBits = bf_offbits;

	BITMAPINFOHEADER_ bitmap_info_header{ 0 };
	bitmap_info_header.biSize = bitmapHeaderSize;
	bitmap_info_header.biWidth = img.cols;
	bitmap_info_header.biHeight = img.rows;
	bitmap_info_header.biPlanes = 1;
	bitmap_info_header.biBitCount = (channels << 3);
	bitmap_info_header.biCompression = BMP_RGB;

	out.write((char*)&bitmap_file_header, sizeof(BITMAPFILEHEADER_));

	out.write((char*)&bitmap_info_header, sizeof(BITMAPINFOHEADER_));

	if (channels == 1)
	{
		FillGrayPalette(palette, 8);
		out.write((const char*)palette, sizeof(palette));
	}

	if (!custom_data.empty())
	{
		out.write((const char*)custom_data.data(), custom_data.size());
	}

	width *= channels;
	for (int y = height - 1; y >= 0; y--)
	{
		out.write((const char*)img.ptr(y), width);
		if (fileStep > width)
			out.write((const char*)zeropad, fileStep - width);
	}

	out.close();

	return true;
}

bool CBmpWriter::write(const std::string & filename, const cv::Mat & image, uint16_t device_index, uint16_t device_state, const XJ1310BitmapExtendInfo & extend_info)
{
	uint32_t size = sizeof(extend_info);
	std::vector<uint8_t> custom_data(size);
	memcpy(custom_data.data(), &extend_info, size);
	return write(filename, image, device_index, device_state, custom_data);
}
