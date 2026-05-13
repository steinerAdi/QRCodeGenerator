/**
 * The MIT License (MIT)
 *
 * This library is written and maintained by Richard Moore.
 * Major parts were derived from Project Nayuki's library.
 *
 * Copyright (c) 2025 Michael R Sweet
 * Copyright (c) 2017 Richard Moore     (https://github.com/ricmoo/QRCode)
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

#ifndef __QRCODE_H_
#define __QRCODE_H_

#include <stdbool.h>
#include <stdint.h>


// QR Code Format Encoding (public API)
#define QRCODE_MODE_NUMERIC        0
#define QRCODE_MODE_ALPHANUMERIC   1
#define QRCODE_MODE_BYTE           2

#ifdef QRCODE_ENABLE_LEGACY
/* Backwards-compatible legacy macro names (enabled by defining QRCODE_ENABLE_LEGACY)
   e.g. -DQRCODE_ENABLE_LEGACY in your build */
#define MODE_NUMERIC        QRCODE_MODE_NUMERIC
#define MODE_ALPHANUMERIC   QRCODE_MODE_ALPHANUMERIC
#define MODE_BYTE           QRCODE_MODE_BYTE
#endif /* QRCODE_ENABLE_LEGACY */


// Error Correction Code Levels (public API)
#define QRCODE_ECC_LOW            0
#define QRCODE_ECC_MEDIUM         1
#define QRCODE_ECC_QUARTILE       2
#define QRCODE_ECC_HIGH           3

#ifdef QRCODE_ENABLE_LEGACY
#define ECC_LOW            QRCODE_ECC_LOW
#define ECC_MEDIUM         QRCODE_ECC_MEDIUM
#define ECC_QUARTILE       QRCODE_ECC_QUARTILE
#define ECC_HIGH           QRCODE_ECC_HIGH
#endif /* QRCODE_ENABLE_LEGACY */


// If set to non-zero, this library can ONLY produce QR codes at that version
// This saves a lot of dynamic memory, as the codeword tables are skipped
#ifndef LOCK_VERSION
#define LOCK_VERSION       0
#endif


// Version Numbers (public API)
#if LOCK_VERSION == 0
#define QRCODE_VERSION_AUTO       0
#endif /* LOCK_VERSION == 0 */
#define QRCODE_VERSION_MIN        1
#define QRCODE_VERSION_MAX        40

#ifdef QRCODE_ENABLE_LEGACY
#if LOCK_VERSION == 0
#define VERSION_AUTO       QRCODE_VERSION_AUTO
#endif /* LOCK_VERSION == 0 */
#define VERSION_MIN        QRCODE_VERSION_MIN
#define VERSION_MAX        QRCODE_VERSION_MAX
#endif /* QRCODE_ENABLE_LEGACY */


/* Public QR code type (namespaced). To enable the legacy name 'QRCode',
   define QRCODE_ENABLE_LEGACY when building. */
typedef struct qrcode_QRCode {
    uint8_t version;
    uint8_t size;
    uint8_t ecc;
    uint8_t mode;
    uint8_t mask;
    uint8_t *modules;
} qrcode_QRCode;

#ifdef QRCODE_ENABLE_LEGACY
typedef qrcode_QRCode QRCode;
#endif /* QRCODE_ENABLE_LEGACY */


#ifdef __cplusplus
extern "C"{
#endif  /* __cplusplus */


uint16_t qrcode_getBufferSize(uint8_t version);

int8_t qrcode_initText(qrcode_QRCode *qrcode, uint8_t *modules, uint8_t version, uint8_t ecc, const char *data);
int8_t qrcode_initBytes(qrcode_QRCode *qrcode, uint8_t *modules, uint8_t version, uint8_t ecc, uint8_t *data, uint16_t length);

bool qrcode_getModule(qrcode_QRCode *qrcode, uint8_t x, uint8_t y);


#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif  /* __QRCODE_H_ */
