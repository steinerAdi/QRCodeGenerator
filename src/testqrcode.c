/**
 * Test program that generates a SVG QR code using the API.
 *
 * Usage:
 *
 *   ./testqrcode [-e {low,medium,quartile,high}] [-v VERSION] TEXT >FILENAME.svg
 *
 * The MIT License (MIT)
 *
 * This library is written and maintained by Richard Moore.
 * Major parts were derived from Project Nayuki's library.
 *
 * Copyright (c) 2025 by Michael R Sweet
 * Copyright (c) 2017 Richard Moore     (https://github.com/ricmoo/qrcode_QRCode)
 * Copyright (c) 2017 Project Nayuki    (https://www.nayuki.io/page/qr-code-generator-library)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qrcode.h"
#include <zlib.h>


// Image export constants...
#define QR_SCALE    5                  // Nominal size of modules
#define QR_PADDING  4                  // White padding around QR code


// Local function for PNG output...
static unsigned char *png_add_crc(unsigned char *pngdata, unsigned char *pngptr, unsigned char *pngend);
static unsigned char *png_add_unsigned(unsigned val, unsigned char *pngptr, unsigned char *pngend);


// Main entry
int main(int argc, char *argv[]) {
    const char *progname;               // Program name
    int        i;                       // Looping var
    uint8_t    ecc = QRCODE_ECC_LOW;           // Error correction level
    uint8_t    version = QRCODE_VERSION_AUTO;  // Version/size
    const char *text = NULL;            // Text to encode
    qrcode_QRCode     qrcode;                  // QR code data
    uint8_t    qrcodeBytes[qrcode_getBufferSize(QRCODE_VERSION_MAX)];
                                        // QR code buffer
    bool       makeSVG = false;         // Output SVG?


    // Parse command-line...
    if ((progname = strrchr(argv[0], '/')) != NULL)
      progname ++;
    else
      progname = argv[0];

    for (i = 1; i < argc; i ++) {
        if (argv[i][0] == '-') {
            for (const char *opt = argv[i] + 1; *opt; opt ++) {
                switch (*opt) {
                    case 'e' : /* -e ECC */
                        i ++;
                        if (i >= argc) {
                            fprintf(stderr, "%s: Missing error correction level after '-e'.\n", progname);
                            return 1;
                        } else if (!strcmp(argv[i], "low")) {
                            ecc = QRCODE_ECC_LOW;
                        } else if (!strcmp(argv[i], "medium")) {
                            ecc = QRCODE_ECC_MEDIUM;
                        } else if (!strcmp(argv[i], "quartile")) {
                            ecc = QRCODE_ECC_QUARTILE;
                        } else if (!strcmp(argv[i], "high")) {
                            ecc = QRCODE_ECC_HIGH;
                        } else {
                            fprintf(stderr, "%s: Bad error correction level '-e %s'.\n", progname, argv[i]);
                            return 1;
                        }
                        break;

                    case 'f' : /* -f FORMAT */
                        i ++;
                        if (i >= argc) {
                            fprintf(stderr, "%s: Missing format after '-f'.\n", progname);
                            return 1;
                        } else {
                            if (!strcmp(argv[i], "svg")) {
                                makeSVG = true;
                            } else if (strcmp(argv[i], "png")) {
                                fprintf(stderr, "%s: Unsupported format '%s'.\n", progname, argv[i]);
                                return 1;
                            }
                        }
                        break;

                    case 'v' : /* -v VERSION */
                        i ++;
                        if (i >= argc) {
                            fprintf(stderr, "%s: Missing version number after '-v'.\n", progname);
                            return 1;
                        } else {
                            long tempval = strtol(argv[i], NULL, 10);
                            if (tempval < QRCODE_VERSION_MIN || tempval > QRCODE_VERSION_MAX) {
                                fprintf(stderr, "%s: Bad version '-v %s'.\n", progname, argv[i]);
                                return 1;
                            }
                            version = (uint8_t)tempval;
                        }
                        break;

                    default :
                        fprintf(stderr, "%s: Unknown option '-%c'.\n", progname, *opt);
                        return 1;
                }
            }
        } else if (text != NULL) {
            fprintf(stderr, "%s: Unknown option '%s'.\n", progname, argv[i]);
            return 1;
        } else {
            text = argv[i];
        }
    }

    // Verify we have something to generate...
    if (text == NULL) {
        fprintf(stderr, "Usage: %s [-e ECC] [-v VERSION] TEXT >FILENAME.svg\n", progname);
        fputs("Options:\n", stderr);
        fputs("-e ECC      Specify error correction (low,medium,quartile,high)\n", stderr);
        fputs("-e VERSION  Specify version/size (1 to 40, default is auto)\n", stderr);
        return 1;
    }

    // Generate QR code...
    if (qrcode_initText(&qrcode, qrcodeBytes, version, ecc, text) < 0) {
        fprintf(stderr, "%s: Unable to generate QR code.\n", progname);
        return 1;
    }

    if (makeSVG) {
	// Write SVG to stdout...
	printf("<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n", (qrcode.size + 2 * QR_PADDING) * QR_SCALE, (qrcode.size + 2 * QR_PADDING) * QR_SCALE);
	printf("  <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"white\" />\n", (qrcode.size + 2 * QR_PADDING) * QR_SCALE, (qrcode.size + 2 * QR_PADDING) * QR_SCALE);

	for (uint8_t y = 0; y < qrcode.size; y++) {
	    uint8_t xstart = 0, xcount = 0;

	    for (uint8_t x = 0; x < qrcode.size; x++) {
		if (qrcode_getModule(&qrcode, x, y)) {
		    if (xcount == 0) { xstart = x; }
		    xcount ++;
		} else if (xcount > 0) {
		    printf("  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\" />\n", (xstart + QR_PADDING) * QR_SCALE, (y + QR_PADDING) * QR_SCALE, xcount * QR_SCALE, QR_SCALE);
		    xcount = 0;
		}
	    }

	    if (xcount > 0) {
		printf("  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\" />\n", (xstart + QR_PADDING) * QR_SCALE, (y + QR_PADDING) * QR_SCALE, xcount * QR_SCALE, QR_SCALE);
	    }
	}

	puts("</svg>");
    } else {
        // Write PNG to stdout...
        unsigned char	pngbuf[65536],	// PNG output buffer
			*pngptr = pngbuf,
					// Pointer into PNG buffer
			*pngend = pngbuf + sizeof(pngbuf),
					// Pointer to end of PNG buffer
			*pngdata,	// Start of PNG chunk data
			line[1 + (QR_SCALE * (255 + 2 * QR_PADDING) + 7) / 8],
					// PNG bitmap line starting with filter byte
			*lineptr,	// Pointer into line
			bit;		// Current bit
        unsigned	size = QR_SCALE * (qrcode.size + 2 * QR_PADDING),
					// Size of image
			linelen = (size + 7) / 8,
					// Length of a line
			x, x0, y, y0,	// Looping vars
			xoff = (QR_SCALE * QR_PADDING) / 8,
			xmod = (QR_SCALE * QR_PADDING) & 7;
        int		zerr;		// ZLIB error code
	z_stream	zstream;	// ZLIB compression stream


        // Add the PNG file header...
        *pngptr++ = 137;
        *pngptr++ = 80;
        *pngptr++ = 78;
        *pngptr++ = 71;
        *pngptr++ = 13;
        *pngptr++ = 10;
        *pngptr++ = 26;
        *pngptr++ = 10;

        // Add the IHDR chunk...
        pngptr    = png_add_unsigned(13, pngptr, pngend);
        pngdata   = pngptr;

        *pngptr++ = 'I';
        *pngptr++ = 'H';
        *pngptr++ = 'D';
        *pngptr++ = 'R';

        pngptr    = png_add_unsigned(size, pngptr, pngend);
					// Width
        pngptr    = png_add_unsigned(size, pngptr, pngend);
					// Height
        *pngptr++ = 1;			// Bit depth
        *pngptr++ = 0;			// Color type grayscale
        *pngptr++ = 0;			// Compression method 0 (deflate)
        *pngptr++ = 0;			// Filter method 0 (adaptive)
        *pngptr++ = 0;			// Interlace method 0 (no interlace)
        pngptr    = png_add_crc(pngdata, pngptr, pngend);

        // Add the IDAT chunk...
        pngptr    += 4;			// Leave room for length
        pngdata   = pngptr;

        *pngptr++ = 'I';
        *pngptr++ = 'D';
        *pngptr++ = 'A';
        *pngptr++ = 'T';

        // Initialize zlib compressor...
        if ((zerr = deflateInit2(&zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, /*windowBits*/11, /*memLevel*/7, Z_DEFAULT_STRATEGY)) < Z_OK) {
            fprintf(stderr, "%s: Unable to create deflate stream (%d).\n", progname, zerr);
            return 1;
        }

        zstream.next_in   = (Bytef *)line;
        zstream.next_out  = (Bytef *)pngptr;
        zstream.avail_out = (uInt)(sizeof(pngbuf) - (pngptr - pngbuf));

        // All lines start with the "None" (0) filter...
        line[0] = 0;

        // Add padding at the top...
        memset(line + 1, 0xff, linelen);
        for (y = 0; y < (QR_SCALE * QR_PADDING); y ++) {
	    zstream.next_in  = (Bytef *)line;
            zstream.avail_in = linelen + 1;
            if ((zerr = deflate(&zstream, Z_NO_FLUSH)) < Z_OK) {
                fprintf(stderr, "%s: Unable to deflate image (%d).\n", progname, zerr);
                return 1;
            }
        }

        // Add lines from the QR code...
        for (y = 0; y < qrcode.size; y ++) {
	    memset(line + 1, 0xff, linelen);

	    for (x = 0, lineptr = line + 1 + xoff, bit = 128 >> xmod; x < qrcode.size; x ++) {
		bool qrset = qrcode_getModule(&qrcode, x, y);

		for (x0 = 0; x0 < QR_SCALE; x0 ++) {
		    if (qrset) {
			*lineptr ^= bit;
		    }

		    if (bit == 1) {
			lineptr ++;
			bit = 128;
		    } else {
			bit = bit / 2;
		    }
		}
	    }

	    for (y0 = 0; y0 < QR_SCALE; y0 ++) {
		zstream.next_in  = (Bytef *)line;
		zstream.avail_in = linelen + 1;
		if ((zerr = deflate(&zstream, Z_NO_FLUSH)) < Z_OK) {
		    fprintf(stderr, "%s: Unable to deflate image (%d).\n", progname, zerr);
		    return 1;
		}
	    }
        }

        // Add padding at the bottom...
        memset(line + 1, 0xff, linelen);
        for (y = 0; y < (QR_SCALE * QR_PADDING); y ++) {
	    zstream.next_in  = (Bytef *)line;
            zstream.avail_in = linelen + 1;
            if ((zerr = deflate(&zstream, Z_NO_FLUSH)) < Z_OK) {
                fprintf(stderr, "%s: Unable to deflate image (%d).\n", progname, zerr);
                return 1;
            }
        }

        // Finish compression...
	zstream.next_in  = (Bytef *)line;
	zstream.avail_in = 0;
	if ((zerr = deflate(&zstream, Z_FINISH)) != Z_STREAM_END) {
	    fprintf(stderr, "%s: Unable to end image (%d).\n", progname, zerr);
	    return 1;
	}

        pngptr = (unsigned char *)zstream.next_out;

        png_add_unsigned((unsigned)(pngptr - pngdata - 4), pngdata - 4, pngend);
        pngptr = png_add_crc(pngdata, pngptr, pngend);

        // Add the IEND chunk...
        pngptr  = png_add_unsigned(0, pngptr, pngend);
        pngdata = pngptr;

        *pngptr++ = 'I';
        *pngptr++ = 'E';
        *pngptr++ = 'N';
        *pngptr++ = 'D';

        pngptr    = png_add_crc(pngdata, pngptr, pngend);

        // Write the PNG file to stdout...
        fwrite(pngbuf, (size_t)(pngptr - pngbuf), 1, stdout);
        fflush(stdout);
    }

    return 0;
}


//
// 'png_add_crc()' - Compute and append the chunk data CRC.
//

static unsigned char *			// O - Next byte in output buffer
png_add_crc(unsigned char *pngdata,	// I - Pointer to start of chunk data
            unsigned char *pngptr,	// I - Pointer to end of chunk data (where CRC goes)
            unsigned char *pngend)	// I - Pointer to end of PNG output buffer
{
  unsigned		c;		// CRC value


  c = crc32(0, Z_NULL, 0);
  c = crc32(c, pngdata, (uInt)(pngptr - pngdata));

  // Append the CRC to the buffer...
  return (png_add_unsigned(c, pngptr, pngend));
}


//
// 'png_add_unsigned()' - Add a 32-bit unsigned integer to the PNG buffer.
//

static unsigned char *			// O - Next byte in output buffer
png_add_unsigned(unsigned      val,	// I - Value to append
                 unsigned char *pngptr,	// I - Pointer into output buffer
                 unsigned char *pngend)	// I - Pointer to end of output buffer
{
  // Append the value to the buffer...
  if (pngptr < pngend)
    *pngptr++ = (val >> 24) & 0xff;
  if (pngptr < pngend)
    *pngptr++ = (val >> 16) & 0xff;
  if (pngptr < pngend)
    *pngptr++ = (val >> 8) & 0xff;
  if (pngptr < pngend)
    *pngptr++ = val & 0xff;

  return (pngptr);
}
