#include "ndstool.h"
#include "raster.h"

using namespace std;

#pragma pack(1)

struct Banner
{
	unsigned_short version;
	unsigned_short crc;
	unsigned char reserved[28];
	unsigned char tile_data[4][4][8][4];
	unsigned_short palette[16];
	unsigned_short title[6][128];
};

#pragma pack()


#define RGB16(r,g,b)			((r) | (g<<5) | (b<<10))

/*
 * RGBQuadToRGB16
 */
unsigned short RGBQuadToRGB16(RGBQUAD quad)
{
	unsigned short r = quad.rgbRed;
	unsigned short g = quad.rgbGreen;
	unsigned short b = quad.rgbBlue;

	r >>= 3; g >>= 3; b >>= 3;

	return RGB16(r, g, b);
}

/*
 * IconFromBMP
 */
void IconFromBMP()
{
	CRaster bmp;
	int rval = bmp.LoadBMP(bannerfilename);
	if (rval < 0) exit(1);

	if (bmp.width != 32 || bmp.height != 32) { fprintf(stderr, "Image should be 32 x 32.\n"); exit(1); }

	Banner banner;
	memset(&banner, 0, sizeof(banner));
	banner.version = 1;

	// tile data (4 bit / tile, 4x4 total tiles)
	// 32 bytes per tile (in 4 bit mode)
	for (int row=0; row<4; row++)
	{
		for (int col=0; col<4; col++)
		{
			for (int y=0; y<8; y++)
			{
				for (int x=0; x<8; x+=2)
				{
					unsigned char b0 = bmp[row*8 + y][col*8 + x + 0];
					unsigned char b1 = bmp[row*8 + y][col*8 + x + 1];
					banner.tile_data[row][col][y][x/2] = (b1 << 4) | b0;
				}
			}
		}
	}

	// palette
	for (int i = 0; i < 16; i++)
	{
		banner.palette[i] = RGBQuadToRGB16(bmp.palette[i]);
	}

	// put title
	for (int i=0; bannertext[i]; i++)
	{
		char c = bannertext[i];
		if (c == ';') c = 0x0A;
		for (int l=0; l<6; l++)
		{
			banner.title[l][i] = c;
		}
	}
	
	// calculate CRC
	banner.crc = CalcCRC((unsigned char *)&banner + 32, 0x840 - 32);

	fwrite(&banner, 1, sizeof(banner), fNDS);
}
