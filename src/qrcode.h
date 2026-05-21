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
#define QRCODE_MODE_NUMERIC 0
#define QRCODE_MODE_ALPHANUMERIC 1
#define QRCODE_MODE_BYTE 2

#ifdef QRCODE_ENABLE_LEGACY
/* Backwards-compatible legacy macro names (enabled by defining QRCODE_ENABLE_LEGACY)
   e.g. -DQRCODE_ENABLE_LEGACY in your build */
#define MODE_NUMERIC QRCODE_MODE_NUMERIC
#define MODE_ALPHANUMERIC QRCODE_MODE_ALPHANUMERIC
#define MODE_BYTE QRCODE_MODE_BYTE
#endif /* QRCODE_ENABLE_LEGACY */

/**
 * @brief QR Code Error Correction Code (ECC) Levels.
 *
 * @note The numeric values correspond to the specific internal array indexing
 *       of this library (Medium, Low, High, Quartile) and differ from the
 *       standard ISO/IEC 18004 sorting sequence.
 */
typedef enum
{
    /** @brief Medium level: Recovers up to ~15% of data loss. (Default choice for balance) */
    QRCODE_ECC_MEDIUM = 0,
    /** @brief Low level: Recovers up to ~7% of data loss. (Maximizes data capacity) */
    QRCODE_ECC_LOW = 1,
    /** @brief High level: Recovers up to ~30% of data loss. (Highest safety against damage) */
    QRCODE_ECC_HIGH = 2,
    /** @brief Quartile level: Recovers up to ~25% of data loss. (Good for branded/logo codes) */
    QRCODE_ECC_QUARTILE = 3
} qrcode_EccLevel;

#ifdef QRCODE_ENABLE_LEGACY
#define ECC_LOW QRCODE_ECC_LOW
#define ECC_MEDIUM QRCODE_ECC_MEDIUM
#define ECC_QUARTILE QRCODE_ECC_QUARTILE
#define ECC_HIGH QRCODE_ECC_HIGH
#endif /* QRCODE_ENABLE_LEGACY */

// If set to non-zero, this library can ONLY produce QR codes at that version
// This saves a lot of dynamic memory, as the codeword tables are skipped
#ifndef LOCK_VERSION
#define LOCK_VERSION 0
#endif

// Version Numbers (public API)
#if LOCK_VERSION == 0
#define QRCODE_VERSION_AUTO 0
#endif /* LOCK_VERSION == 0 */
#define QRCODE_VERSION_MIN 1
#define QRCODE_VERSION_MAX 40

#ifdef QRCODE_ENABLE_LEGACY
#if LOCK_VERSION == 0
#define VERSION_AUTO QRCODE_VERSION_AUTO
#endif /* LOCK_VERSION == 0 */
#define VERSION_MIN QRCODE_VERSION_MIN
#define VERSION_MAX QRCODE_VERSION_MAX
#endif /* QRCODE_ENABLE_LEGACY */

/**
 * @brief Public QR Code structure (namespaced).
 *
 * This structure holds the configuration metadata and a pointer to the actual
 * bit-packed pixel grid representing a generated QR Code.
 *
 * @note To enable the legacy name 'QRCode' instead of 'qrcode_QRCode',
 *       define @c QRCODE_ENABLE_LEGACY in your build configuration.
 */
typedef struct qrcode_QRCode
{
    /**
     * @brief The QR Code version used for generation.
     * @details Valid range is @c QRCODE_VERSION_MIN to @c QRCODE_VERSION_MAX.
     *          Determines the density and theoretical data capacity of the symbol.
     */
    uint8_t version;

    /**
     * @brief The grid width and height of the QR Code in modules (pixels).
     * @details Calculated based on the version using the formula:
     *          @f$ \text{size} = 17 + 4 \times \text{version} @f$.
     *          For example, Version 1 is 21x21 modules, and Version 40 is 177x177 modules.
     */
    uint8_t size;

    /**
     * @brief The Error Correction Code (ECC) level applied to the code.
     * @details Uses the library-specific indexing:
     *          - 0 = Medium (M)
     *          - 1 = Low (L)
     *          - 2 = High (H)
     *          - 3 = Quartile (Q)
     */
    qrcode_EccLevel ecc;

    /**
     * @brief The data encoding mode utilized for the payload.
     * @details Defines how data bits are formatted (e.g., Numeric, Alphanumeric, Byte, or Kanji).
     */
    uint8_t mode;

    /**
     * @brief The index of the grid mask pattern applied to the modules.
     * @details Valid range is 0 to 7. Masking is automatically evaluated and applied
     *          to balance the ratio of dark and light modules and prevent scanning artifacts.
     */
    uint8_t mask;

    /**
     * @brief Pointer to the raw, bit-packed module data buffer.
     * @details Each byte in this buffer holds 8 sequential modules (pixels).
     *          A bit value of @c 1 represents a dark/black module, while @c 0
     *          represents a light/white module. Use qrcode_getModule() to safely
     *          read from this buffer.
     */
    uint8_t *modules;
} qrcode_QRCode;

#ifdef QRCODE_ENABLE_LEGACY
typedef qrcode_QRCode QRCode;
#endif /* QRCODE_ENABLE_LEGACY */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Calculates the required buffer size (in bytes) for a QR Code modules array.
     *
     * Since each module (pixel) in the QR Code is stored internally as a single bit,
     * the buffer size depends directly on the grid size of the selected QR Code version.
     * Formula: @f$ \lceil \frac{(17 + 4 \times \text{version})^2}{8} \rceil @f$
     *
     * @param[in] version The QR Code version (Valid range: 1 to 40).
     * @return The minimum buffer size in bytes required for the @p modules array.
     */
    uint16_t qrcode_getBufferSize(uint8_t version);

    /**
     * @brief Initializes a QR Code and encodes a plain text string.
     *
     * This function analyzes the provided ASCII/UTF-8 text, selects the optimal
     * encoding mode (usually Alphanumeric or Byte), and generates the QR Code pattern
     * inside the provided memory buffer.
     *
     * @param[out] qrcode  Pointer to the qrcode_QRCode structure to be initialized.
     * @param[in]  modules Pointer to a pre-allocated memory buffer for the pixel data.
     *                     The size must be at least qrcode_getBufferSize(version).
     * @param[in]  version The desired QR Code version (1 to 40). If LOCK_VERSION > 0,
     *                     this must match the fixed LOCK_VERSION value.
     * @param[in]  ecc     The error correction level (0 = Medium, 1 = Low, 2 = High, 3 = Quartile).
     * @param[in]  data    Null-terminated C-string (ASCII/UTF-8) to be encoded into the QR Code.
     *
     * @retval 0  Successfully initialized and encoded.
     * @retval -1 Error: The data is too long for the chosen version and ECC level combination.
     */
    int8_t qrcode_initText(qrcode_QRCode *qrcode, uint8_t *modules, uint8_t version, qrcode_EccLevel ecc, const char *data);

    /**
     * @brief Initializes a QR Code and encodes raw binary data (byte array).
     *
     * Unlike qrcode_initText, this function allows encoding of arbitrary binary data,
     * including null bytes (`0x00`). Ideal for URLs, encrypted payloads, or structured
     * binary protocols.
     *
     * @param[out] qrcode  Pointer to the qrcode_QRCode structure to be initialized.
     * @param[in]  modules Pointer to a pre-allocated memory buffer for the pixel data.
     *                     The size must be at least qrcode_getBufferSize(version).
     * @param[in]  version The desired QR Code version (1 to 40). If LOCK_VERSION > 0,
     *                     this must match the fixed LOCK_VERSION value.
     * @param[in]  ecc     The error correction level (0 = Medium, 1 = Low, 2 = High, 3 = Quartile).
     * @param[in]  data    Pointer to the byte array containing the raw data to encode.
     * @param[in]  length  The number of bytes to read from the @p data array.
     *
     * @retval 0  Successfully initialized and encoded.
     * @retval -1 Error: The data length exceeds the capacity of the selected version and ECC level.
     */
    int8_t qrcode_initBytes(qrcode_QRCode *qrcode, uint8_t *modules, uint8_t version, qrcode_EccLevel ecc, uint8_t *data, uint16_t length);

    /**
     * @brief Queries the state (black or white) of a specific module (pixel).
     *
     * Internally calculates the bit offset within the compressed modules buffer
     * and returns the state for the given 2D coordinates.
     *
     * @param[in] qrcode Pointer to the initialized qrcode_QRCode structure.
     * @param[in] x      The X-coordinate (column) of the module, starting at 0 from the left.
     * @param[in] y      The Y-coordinate (row) of the module, starting at 0 from the top.
     *
     * @pre The QR Code must have been successfully initialized with qrcode_initText()
     *      or qrcode_initBytes(). Coordinates must be within the grid bounds
     *      (0 to grid_size - 1).
     *
     * @retval true  The module is "black" (set bit / data module).
     * @retval false The module is "white" (cleared bit / background).
     */
    bool qrcode_getModule(qrcode_QRCode *qrcode, uint8_t x, uint8_t y);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __QRCODE_H_ */
