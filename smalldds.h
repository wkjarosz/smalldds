//
// smalldds - A small, cross-platform, single-header DDS loader with minimal dependencies.
//
// Copyright (c) 2025 Wojciech Jarosz. Distributed under the
// Apache 2.0 License (https://opensource.org/license/apache-2-0)
//
// This work was originally based on the DDS loaders in smalldds, OIIO, and nvpro_core2, though largely rewritten with
// different design goals in mind.
//
// Copyright and license info of those projects below:
//
// tinyddsloader - https://github.com/benikabocha/tinyddsloader
// Copyright (c) 2019 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
// Open image IO - https://github.com/AcademySoftwareFoundation/OpenImageIO
// Copyright (c) Contributors to the OpenImageIO project.
// Distributed under the Apache 2.0 License
// (https://opensource.org/license/apache-2-0)
//
// nvpro_core2 - https://github.com/nvpro-samples/nvpro_core2
// Copyright (c) 2016-2025, NVIDIA CORPORATION.  All rights reserved.
// Distributed under the Apache 2.0 License
// (https://opensource.org/license/apache-2-0)
//

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace smalldds
{

struct Result
{
    enum Type
    {
        Success = 0,
        Info    = 1,
        Warning = 2,
        Error   = 3
    };

    void add_message(Type t, const std::string &m)
    {
        type = std::max(type, t);
        message += message.empty() ? m : "\n" + m;
    }

    Type        type;
    std::string message;
};

constexpr uint32_t MakeFourCC(char ch0, char ch1, char ch2, char ch3)
{
    return (uint32_t(uint8_t(ch0)) | (uint32_t(uint8_t(ch1)) << 8) | (uint32_t(uint8_t(ch2)) << 16) |
            (uint32_t(uint8_t(ch3)) << 24));
}

std::string fourCC_to_string(const std::array<char, 4> &fourCC);
std::string fourCC_to_string(uint32_t fourCC);

/** Represents and loads a DirectDraw Surface (DDS) file, providing access to its header, pixel format, and image data.

    This class encapsulates the logic for parsing, validating, and extracting image data from DDS files, including
    support for legacy and modern DDS formats (with or without the DXT10 header). It provides convenient accessors for
    header fields, image data, and format information, as well as utilities for working with compressed and uncompressed
    DDS images.

    Supported features include:
    - Parsing of standard and extended (DXT10) DDS headers
    - Extraction of mipmaps and array slices
    - Bitmask and channel information for uncompressed formats
    - Alpha mode and color transform metadata
    - Detection and handling of various DDS compression formats (BCn, ASTC, etc.)
      - Note that this loader does not perform any decompression or unpacking of the pixel data; it only provides access
        to the raw data.

    Usage example:
    @code
    #define SMALLDDS_IMPLEMENTATION
    #include "smalldds.h"

    using namespace smalldds;

    DDSFile dds;
    DDSFile::Result result = dds.load("texture.dds");
    if (result.type != DDSFile::Result::Success)
        // check result.message for info, warning or error messages

    // continue if result.type is not Result::Error

    result = dds.populate_image_data();
    if (result.type != DDSFile::Result::Success)
        // check result.message for info, warning or error messages

    // continue if result.type is not Result::Error
    auto data = dds.get_image_data(0, 0);
    if (data) {
        // Access data->bytes for pixel data and dds for various header information
    }
    @endcode

    @note This class is not thread-safe.
    @note For more information on the DDS format, see:
            https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide
 */
class DDSFile
{
public:
    static const char Magic[4];

    static constexpr uint32_t FOURCC_DXT1 = MakeFourCC('D', 'X', 'T', '1');
    static constexpr uint32_t FOURCC_DXT2 = MakeFourCC('D', 'X', 'T', '2');
    static constexpr uint32_t FOURCC_DXT3 = MakeFourCC('D', 'X', 'T', '3');
    static constexpr uint32_t FOURCC_DXT4 = MakeFourCC('D', 'X', 'T', '4');
    static constexpr uint32_t FOURCC_DXT5 = MakeFourCC('D', 'X', 'T', '5');
    static constexpr uint32_t FOURCC_RXGB = MakeFourCC('R', 'X', 'G', 'B');
    static constexpr uint32_t FOURCC_ATI1 = MakeFourCC('A', 'T', 'I', '1');
    static constexpr uint32_t FOURCC_ATI2 = MakeFourCC('A', 'T', 'I', '2');
    static constexpr uint32_t FOURCC_BC4U = MakeFourCC('B', 'C', '4', 'U');
    static constexpr uint32_t FOURCC_BC4S = MakeFourCC('B', 'C', '4', 'S');
    static constexpr uint32_t FOURCC_BC5U = MakeFourCC('B', 'C', '5', 'U');
    static constexpr uint32_t FOURCC_BC5S = MakeFourCC('B', 'C', '5', 'S');
    static constexpr uint32_t FOURCC_BC6H = MakeFourCC('B', 'C', '6', 'H');
    static constexpr uint32_t FOURCC_DX10 = MakeFourCC('D', 'X', '1', '0');
    static constexpr uint32_t FOURCC_RGBG = MakeFourCC('R', 'G', 'B', 'G');
    static constexpr uint32_t FOURCC_GRGB = MakeFourCC('G', 'R', 'G', 'B');
    static constexpr uint32_t FOURCC_YUY2 = MakeFourCC('Y', 'U', 'Y', '2');
    static constexpr uint32_t FOURCC_UYVY = MakeFourCC('U', 'Y', 'V', 'Y');
    static constexpr uint32_t FOURCC_BC7L = MakeFourCC('B', 'C', '7', 'L');
    static constexpr uint32_t FOURCC_BC70 = MakeFourCC('B', 'C', '7', '\0');
    // Written by NVTT
    static constexpr uint32_t FOURCC_A2XY = MakeFourCC('A', '2', 'X', 'Y');
    static constexpr uint32_t FOURCC_A2D5 = MakeFourCC('A', '2', 'D', '5');
    static constexpr uint32_t FOURCC_ZOLA = MakeFourCC('Z', 'O', 'L', 'A');
    static constexpr uint32_t FOURCC_CTX1 = MakeFourCC('C', 'T', 'X', '1');
    // ASTC formats
    static constexpr uint32_t FOURCC_ASTC4x4 = MakeFourCC('A', 'S', '4', '4');
    static constexpr uint32_t FOURCC_ASTC5x4 = MakeFourCC('A', 'S', '5', '4');
    static constexpr uint32_t FOURCC_ASTC5x5 = MakeFourCC('A', 'S', '5', '5');
    static constexpr uint32_t FOURCC_ASTC6x5 = MakeFourCC('A', 'S', '6', '5');
    static constexpr uint32_t FOURCC_ASTC6x6 = MakeFourCC('A', 'S', '6', '6');
    static constexpr uint32_t FOURCC_ASTC8x5 = MakeFourCC('A', 'S', '8', '5');
    static constexpr uint32_t FOURCC_ASTC8x6 = MakeFourCC('A', 'S', '8', '6');
    static constexpr uint32_t FOURCC_ASTC8x8 = MakeFourCC('A', 'S', '8', '8');

    static constexpr uint32_t FOURCC_ASTC10x5  = MakeFourCC('A', 'S', 'A', '5');
    static constexpr uint32_t FOURCC_ASTC10x6  = MakeFourCC('A', 'S', 'A', '6');
    static constexpr uint32_t FOURCC_ASTC10x8  = MakeFourCC('A', 'S', 'A', '8');
    static constexpr uint32_t FOURCC_ASTC10x10 = MakeFourCC('A', 'S', 'A', 'A');
    static constexpr uint32_t FOURCC_ASTC12x10 = MakeFourCC('A', 'S', 'C', 'A');
    static constexpr uint32_t FOURCC_ASTC12x12 = MakeFourCC('A', 'S', 'C', 'C');
    // additional ASTC FourCCs used by some DDS writers
    static constexpr uint32_t FOURCC_ASTC10x5_ALT  = MakeFourCC('A', 'S', ':', '5');
    static constexpr uint32_t FOURCC_ASTC10x6_ALT  = MakeFourCC('A', 'S', ':', '6');
    static constexpr uint32_t FOURCC_ASTC10x8_ALT  = MakeFourCC('A', 'S', ':', '8');
    static constexpr uint32_t FOURCC_ASTC10x10_ALT = MakeFourCC('A', 'S', ':', ':');
    static constexpr uint32_t FOURCC_ASTC12x10_ALT = MakeFourCC('A', 'S', '<', ':');
    static constexpr uint32_t FOURCC_ASTC12x12_ALT = MakeFourCC('A', 'S', '<', '<');

    // Some DDS writers (e.g. GLI, some modes of DirectXTex, and floating-point
    // formats in old versions of NVTT) write out formats by storing their D3D9
    // D3DFMT in the FourCC field.
    // See https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dformat
    // and https://github.com/g-truc/gli/blob/master/gli/dx.hpp
    static constexpr uint32_t D3DFMT_UNKNOWN             = 0;
    static constexpr uint32_t D3DFMT_R8G8B8              = 20;
    static constexpr uint32_t D3DFMT_A8R8G8B8            = 21;
    static constexpr uint32_t D3DFMT_X8R8G8B8            = 22;
    static constexpr uint32_t D3DFMT_R5G6B5              = 23;
    static constexpr uint32_t D3DFMT_X1R5G5B5            = 24;
    static constexpr uint32_t D3DFMT_A1R5G5B5            = 25;
    static constexpr uint32_t D3DFMT_A4R4G4B4            = 26;
    static constexpr uint32_t D3DFMT_R3G3B2              = 27;
    static constexpr uint32_t D3DFMT_A8                  = 28;
    static constexpr uint32_t D3DFMT_A8R3G3B2            = 29;
    static constexpr uint32_t D3DFMT_X4R4G4B4            = 30;
    static constexpr uint32_t D3DFMT_A2B10G10R10         = 31;
    static constexpr uint32_t D3DFMT_A8B8G8R8            = 32;
    static constexpr uint32_t D3DFMT_X8B8G8R8            = 33;
    static constexpr uint32_t D3DFMT_G16R16              = 34;
    static constexpr uint32_t D3DFMT_A2R10G10B10         = 35;
    static constexpr uint32_t D3DFMT_A16B16G16R16        = 36;
    static constexpr uint32_t D3DFMT_A8P8                = 40;
    static constexpr uint32_t D3DFMT_P8                  = 41;
    static constexpr uint32_t D3DFMT_L8                  = 50;
    static constexpr uint32_t D3DFMT_A8L8                = 51;
    static constexpr uint32_t D3DFMT_A4L4                = 52;
    static constexpr uint32_t D3DFMT_V8U8                = 60;
    static constexpr uint32_t D3DFMT_L6V5U5              = 61;
    static constexpr uint32_t D3DFMT_X8L8V8U8            = 62;
    static constexpr uint32_t D3DFMT_Q8W8V8U8            = 63;
    static constexpr uint32_t D3DFMT_V16U16              = 64;
    static constexpr uint32_t D3DFMT_A2W10V10U10         = 67;
    static constexpr uint32_t D3DFMT_D16_LOCKABLE        = 70;
    static constexpr uint32_t D3DFMT_D32                 = 71;
    static constexpr uint32_t D3DFMT_D15S1               = 73;
    static constexpr uint32_t D3DFMT_D24S8               = 75;
    static constexpr uint32_t D3DFMT_D24X8               = 77;
    static constexpr uint32_t D3DFMT_D24X4S4             = 79;
    static constexpr uint32_t D3DFMT_D16                 = 80;
    static constexpr uint32_t D3DFMT_L16                 = 81;
    static constexpr uint32_t D3DFMT_D32F_LOCKABLE       = 82;
    static constexpr uint32_t D3DFMT_D24FS8              = 83;
    static constexpr uint32_t D3DFMT_D32_LOCKABLE        = 84;
    static constexpr uint32_t D3DFMT_S8_LOCKABLE         = 85;
    static constexpr uint32_t D3DFMT_VERTEXDATA          = 100;
    static constexpr uint32_t D3DFMT_INDEX16             = 101;
    static constexpr uint32_t D3DFMT_INDEX32             = 102;
    static constexpr uint32_t D3DFMT_Q16W16V16U16        = 110;
    static constexpr uint32_t D3DFMT_R16F                = 111;
    static constexpr uint32_t D3DFMT_G16R16F             = 112;
    static constexpr uint32_t D3DFMT_A16B16G16R16F       = 113;
    static constexpr uint32_t D3DFMT_R32F                = 114;
    static constexpr uint32_t D3DFMT_G32R32F             = 115;
    static constexpr uint32_t D3DFMT_A32B32G32R32F       = 116;
    static constexpr uint32_t D3DFMT_CxV8U8              = 117;
    static constexpr uint32_t D3DFMT_A1                  = 118;
    static constexpr uint32_t D3DFMT_A2B10G10R10_XR_BIAS = 119;
    static constexpr uint32_t D3DFMT_BINARYBUFFER        = 199;

    // miscFlags2 enumeration: Alpha modes
    static constexpr uint32_t ALPHA_MODE_UNKNOWN       = 0x0;
    static constexpr uint32_t ALPHA_MODE_STRAIGHT      = 0x1;
    static constexpr uint32_t ALPHA_MODE_PREMULTIPLIED = 0x2;
    static constexpr uint32_t ALPHA_MODE_OPAQUE        = 0x3;
    static constexpr uint32_t ALPHA_MODE_CUSTOM        = 0x4;

    enum class ColorTransform
    {
        eNone,
        eLuminance, ///< Stores luminance values (e.g. L8 instead of R8)
        eAGBR,      ///< Red and alpha channel are swapped (aka RXGB)
        eYUV,
        eYCoCg,             ///< Data is in the YCoCg color model.
        eYCoCgScaled,       ///< Data is in a scaled YCoCg format.
        eAEXP,              ///< The alpha channel acts as a scaling factor.
        eSwapRG,            ///< Swap the red and green channels.
        eSwapRB,            ///< Swap the red and blue channels.
        eOrthographicNormal ///< Reconstruct b = sqrt(1-r^2-g^2).
    };

    enum class Compression
    {
        None,
        BC1_DXT1,
        BC2_DXT2,
        BC2_DXT3,
        BC3_DXT4,
        BC3_DXT5,
        BC4, // aka ATI1
        BC5, // aka ATI2
        BC6HU,
        BC6HS,
        BC7,
        ASTC
    };

    enum class PixelFormatFlagBits : uint32_t
    {
        AlphaPixels       = 0x00000001U, ///< image has alpha channel
        AlphaOnly         = 0x00000002U, ///< image has only the alpha channel
        FourCC            = 0x00000004U, ///< image is compressed
        PaletteIndexed4   = 0x00000008U,
        PaletteIndexedTo8 = 0x00000010U,
        PaletteIndexed8   = 0x00000020U,
        RGB               = 0x00000040U, ///< image has RGB data
        Compressed        = 0x00000080U,
        RGBToYUV          = 0x00000100U,
        YUV               = 0x00000200U,
        ZBuffer           = 0x00000400U,
        PaletteIndexed1   = 0x00000800U,
        PaletteIndexed2   = 0x00001000U,
        ZPixels           = 0x00002000U,
        StencilBuffer     = 0x00004000U,
        AlphaPreMult      = 0x00008000U,
        Luminance         = 0x00020000U, ///< image has luminance data
        BumpLuminance     = 0x00040000U,
        BumpDuDv          = 0x00080000U,
        RGBA              = RGB | AlphaPixels,
        // Custom NVTT flags.
        SRGB   = 0x40000000U,
        Normal = 0x80000000U, ///< image is a tangent space normal map
    };

    struct PixelFormat
    {
        uint32_t size;      ///< structure size, must be 32
        uint32_t flags;     ///< flags to indicate valid fields
        uint32_t fourCC;    ///< compression four-character code
        uint32_t bit_count; ///< bits per pixel
        uint32_t masks[4];  ///< bitmasks for the r,g,b,a channels
    };

    enum class HeaderFlagBits : uint32_t
    {
        Caps        = 0x00000001,
        Height      = 0x00000002,
        Width       = 0x00000004,
        Pitch       = 0x00000008,
        PixelFormat = 0x00001000,
        Texture     = 0x00001007,
        Mipmap      = 0x00020000,
        Depth       = 0x00800000,
        LinearSize  = 0x00080000
    };

    enum HeaderCaps2FlagBits : uint32_t
    {
        CubemapPositiveX = 0x00000600,
        CubemapNegativeX = 0x00000a00,
        CubemapPositiveY = 0x00001200,
        CubemapNegativeY = 0x00002200,
        CubemapPositiveZ = 0x00004200,
        CubemapNegativeZ = 0x00008200,
        CubemapAllFaces = CubemapPositiveX | CubemapNegativeX | CubemapPositiveY | CubemapNegativeY | CubemapPositiveZ |
                          CubemapNegativeZ,
        Volume = 0x00200000,
    };

    struct Header
    {
        uint32_t size;                 ///< structure size, must be 124
        uint32_t flags;                ///< flags to indicate valid fields
        uint32_t height;               ///< image height
        uint32_t width;                ///< image width
        uint32_t pitch_or_linear_size; ///< bytes per scanline (uncmp.)/total
                                       ///< byte size (cmp.)
        uint32_t    depth;             ///< image depth (for 3D textures)
        uint32_t    mipmap_count;      ///< number of mipmaps
        uint32_t    reserved1[11];
        PixelFormat pixel_format;
        uint32_t    caps1;
        uint32_t    caps2;
        uint32_t    caps3;
        uint32_t    caps4;
        uint32_t    reserved2;
    };

    enum TextureDimension : uint32_t
    {
        Texture0D = 0, ///< Unknown
        Texture1D = 2,
        Texture2D = 3,
        Texture3D = 4
    };

    enum class DXT10MiscFlagBits : uint32_t
    {
        TextureCube = 0x4
    };

    enum DXGIFormat : uint32_t
    {
        Format_Unknown             = 0,
        R32G32B32A32_Typeless      = 1,
        R32G32B32A32_Float         = 2,
        R32G32B32A32_UInt          = 3,
        R32G32B32A32_SInt          = 4,
        R32G32B32_Typeless         = 5,
        R32G32B32_Float            = 6,
        R32G32B32_UInt             = 7,
        R32G32B32_SInt             = 8,
        R16G16B16A16_Typeless      = 9,
        R16G16B16A16_Float         = 10,
        R16G16B16A16_UNorm         = 11,
        R16G16B16A16_UInt          = 12,
        R16G16B16A16_SNorm         = 13,
        R16G16B16A16_SInt          = 14,
        R32G32_Typeless            = 15,
        R32G32_Float               = 16,
        R32G32_UInt                = 17,
        R32G32_SInt                = 18,
        R32G8X24_Typeless          = 19,
        D32_Float_S8X24_UInt       = 20,
        R32_Float_X8X24_Typeless   = 21,
        X32_Typeless_G8X24_UInt    = 22,
        R10G10B10A2_Typeless       = 23,
        R10G10B10A2_UNorm          = 24,
        R10G10B10A2_UInt           = 25,
        R11G11B10_Float            = 26,
        R8G8B8A8_Typeless          = 27,
        R8G8B8A8_UNorm             = 28,
        R8G8B8A8_UNorm_SRGB        = 29,
        R8G8B8A8_UInt              = 30,
        R8G8B8A8_SNorm             = 31,
        R8G8B8A8_SInt              = 32,
        R16G16_Typeless            = 33,
        R16G16_Float               = 34,
        R16G16_UNorm               = 35,
        R16G16_UInt                = 36,
        R16G16_SNorm               = 37,
        R16G16_SInt                = 38,
        R32_Typeless               = 39,
        D32_Float                  = 40,
        R32_Float                  = 41,
        R32_UInt                   = 42,
        R32_SInt                   = 43,
        R24G8_Typeless             = 44,
        D24_UNorm_S8_UInt          = 45,
        R24_UNorm_X8_Typeless      = 46,
        X24_Typeless_G8_UInt       = 47,
        R8G8_Typeless              = 48,
        R8G8_UNorm                 = 49,
        R8G8_UInt                  = 50,
        R8G8_SNorm                 = 51,
        R8G8_SInt                  = 52,
        R16_Typeless               = 53,
        R16_Float                  = 54,
        D16_UNorm                  = 55,
        R16_UNorm                  = 56,
        R16_UInt                   = 57,
        R16_SNorm                  = 58,
        R16_SInt                   = 59,
        R8_Typeless                = 60,
        R8_UNorm                   = 61,
        R8_UInt                    = 62,
        R8_SNorm                   = 63,
        R8_SInt                    = 64,
        A8_UNorm                   = 65,
        R1_UNorm                   = 66,
        R9G9B9E5_SHAREDEXP         = 67,
        R8G8_B8G8_UNorm            = 68,
        G8R8_G8B8_UNorm            = 69,
        BC1_Typeless               = 70,
        BC1_UNorm                  = 71,
        BC1_UNorm_SRGB             = 72,
        BC2_Typeless               = 73,
        BC2_UNorm                  = 74,
        BC2_UNorm_SRGB             = 75,
        BC3_Typeless               = 76,
        BC3_UNorm                  = 77,
        BC3_UNorm_SRGB             = 78,
        BC4_Typeless               = 79,
        BC4_UNorm                  = 80,
        BC4_SNorm                  = 81,
        BC5_Typeless               = 82,
        BC5_UNorm                  = 83,
        BC5_SNorm                  = 84,
        B5G6R5_UNorm               = 85,
        B5G5R5A1_UNorm             = 86,
        B8G8R8A8_UNorm             = 87,
        B8G8R8X8_UNorm             = 88,
        R10G10B10_XR_BIAS_A2_UNorm = 89,
        B8G8R8A8_Typeless          = 90,
        B8G8R8A8_UNorm_SRGB        = 91,
        B8G8R8X8_Typeless          = 92,
        B8G8R8X8_UNorm_SRGB        = 93,
        BC6H_Typeless              = 94,
        BC6H_UF16                  = 95,
        BC6H_SF16                  = 96,
        BC7_Typeless               = 97,
        BC7_UNorm                  = 98,
        BC7_UNorm_SRGB             = 99,
        AYUV                       = 100,
        Y410                       = 101,
        Y416                       = 102,
        NV12                       = 103,
        P010                       = 104,
        P016                       = 105,
        YUV420_OPAQUE              = 106,
        YUY2                       = 107,
        Y210                       = 108,
        Y216                       = 109,
        NV11                       = 110,
        AI44                       = 111,
        IA44                       = 112,
        P8                         = 113,
        A8P8                       = 114,
        B4G4R4A4_UNorm             = 115,
        P208                       = 130,
        V208                       = 131,
        V408                       = 132,
        // DXGI ASTC extension
        ASTC_4X4_Typeless     = 133,
        ASTC_4X4_UNorm        = 134,
        ASTC_4X4_UNorm_SRGB   = 135,
        ASTC_5X4_Typeless     = 137,
        ASTC_5X4_UNorm        = 138,
        ASTC_5X4_UNorm_SRGB   = 139,
        ASTC_5X5_Typeless     = 141,
        ASTC_5X5_UNorm        = 142,
        ASTC_5X5_UNorm_SRGB   = 143,
        ASTC_6X5_Typeless     = 145,
        ASTC_6X5_UNorm        = 146,
        ASTC_6X5_UNorm_SRGB   = 147,
        ASTC_6X6_Typeless     = 149,
        ASTC_6X6_UNorm        = 150,
        ASTC_6X6_UNorm_SRGB   = 151,
        ASTC_8X5_Typeless     = 153,
        ASTC_8X5_UNorm        = 154,
        ASTC_8X5_UNorm_SRGB   = 155,
        ASTC_8X6_Typeless     = 157,
        ASTC_8X6_UNorm        = 158,
        ASTC_8X6_UNorm_SRGB   = 159,
        ASTC_8X8_Typeless     = 161,
        ASTC_8X8_UNorm        = 162,
        ASTC_8X8_UNorm_SRGB   = 163,
        ASTC_10X5_Typeless    = 165,
        ASTC_10X5_UNorm       = 166,
        ASTC_10X5_UNorm_SRGB  = 167,
        ASTC_10X6_Typeless    = 169,
        ASTC_10X6_UNorm       = 170,
        ASTC_10X6_UNorm_SRGB  = 171,
        ASTC_10X8_Typeless    = 173,
        ASTC_10X8_UNorm       = 174,
        ASTC_10X8_UNorm_SRGB  = 175,
        ASTC_10X10_Typeless   = 177,
        ASTC_10X10_UNorm      = 178,
        ASTC_10X10_UNorm_SRGB = 179,
        ASTC_12X10_Typeless   = 181,
        ASTC_12X10_UNorm      = 182,
        ASTC_12X10_UNorm_SRGB = 183,
        ASTC_12X12_Typeless   = 185,
        ASTC_12X12_UNorm      = 186,
        ASTC_12X12_UNorm_SRGB = 187,
        A4B4G4R4_UNorm        = 191,
    };

    enum class DataType : uint32_t
    {
        Unknown = 0,
        Typeless8,
        Typeless16,
        Typeless32,
        Packed, //!< <8 bits or an unequal number of bits per channel, e.g. B5G5R5A1_UNorm
        SInt8,
        SInt16,
        SInt32,
        UInt8,
        UInt16,
        UInt32,
        SNorm8,
        SNorm16,
        UNorm8,
        UNorm16,
        Float16,
        Float32,
    };

    struct HeaderDXT10
    {
        DXGIFormat       format             = Format_Unknown;
        TextureDimension resource_dimension = Texture0D;
        uint32_t         misc_flag          = 0;
        uint32_t         array_size         = 1;
        uint32_t         misc_flag2         = 0;
    };

    struct ImageData
    {
        uint32_t                        width  = 0;
        uint32_t                        height = 0;
        uint32_t                        depth  = 0;
        std::basic_string_view<uint8_t> bytes  = {};
    };

public:
    static bool     is_compressed(DXGIFormat fmt);
    static DataType data_type(DXGIFormat fmt);
    static size_t   data_type_size(DataType type);
    static void     calc_shifts(uint32_t mask, uint32_t &count, uint32_t &right);

    Result load(const char *filepath);
    Result load(std::istream &input);
    Result load(const uint8_t *data, size_t size);
    Result load(std::vector<uint8_t> &&dds);
    Result populate_image_data();

    const ImageData *get_image_data(uint32_t mipIdx = 0, uint32_t arrayIdx = 0) const
    {
        if (mipIdx < header.mipmap_count && arrayIdx < header_DXT10.array_size)
            return &image_data[header.mipmap_count * arrayIdx + mipIdx];
        return nullptr;
    }

    // Convenient access to some header fields
    uint32_t         width() const { return header.width; }
    uint32_t         height() const { return header.height; }
    uint32_t         depth() const { return header.depth; }
    uint32_t         mip_count() const { return header.mipmap_count; }
    uint32_t         array_size() const { return header_DXT10.array_size; }
    DXGIFormat       format() const { return header_DXT10.format; }
    TextureDimension texture_dimension() const { return header_DXT10.resource_dimension; }
    uint32_t         block_width() const;
    uint32_t         block_height() const;
    bool             is_sRGB() const;

    std::vector<uint8_t>   dds;
    std::vector<ImageData> image_data;

    Header      header;
    bool        has_DXT10_header = false;
    HeaderDXT10 header_DXT10;
    bool        is_cubemap;
    Compression compression = Compression::None;

    int bpp          = 0; ///< Bits per pixel, 0 if unknown
    int num_channels = 0;

    /// The alpha mode; straight, premultiplied, opaque, or custom.
    uint32_t alpha_mode = ALPHA_MODE_UNKNOWN;

    /// See ColorTransform.
    ColorTransform color_transform = ColorTransform::eNone;

    /// Whether the DDS file should be decompressed using bitmasks
    bool bitmasked = false;
    /// If bitmasked, whether there is an alpha component.
    bool bitmask_has_alpha = false;
    /// If bitmasked, whether there are RGB components.
    bool bitmask_has_rgb = false;
    /// If bitmasked, whether it uses the "bump du dv" encoding for normal maps.
    bool bitmask_was_bump_du_dv = false;

    uint32_t bit_counts[4]   = {0, 0, 0, 0}; ///< Bit counts for r,g,b,a channels
    uint32_t right_shifts[4] = {0, 0, 0, 0}; ///< Shifts to extract r,g,b,a channels

private:
    void       calc_channel_info(Result &res);
    DXGIFormat deduce_format_from_fourCC(Result &res);
    void       deduce_bitmasks_from_pixel_format();
    Result     verify_header();
    size_t     image_data_size(uint32_t w, uint32_t h, uint32_t d, Result &res) const;

    bool m_header_verified = false;
};

/// Convert 11-bit float (5 exp + 6 mantissa) to 32-bit float
inline float decode_float11(uint32_t bits)
{
    if (bits == 0)
        return 0.0f;

    uint32_t exponent = (bits >> 6) & 0x1F; // 5 bits
    uint32_t mantissa = bits & 0x3F;        // 6 bits

    if (exponent == 0)
    {
        // Denormalized number
        return std::ldexp(float(mantissa) / 64.0f, -14);
    }
    else if (exponent == 31)
    {
        // Infinity or NaN
        return mantissa ? NAN : INFINITY;
    }
    else
    {
        // Normalized number
        float m = 1.0f + float(mantissa) / 64.0f;
        return std::ldexp(m, int(exponent) - 15);
    }
}

/// Convert 10-bit float (5 exp + 5 mantissa) to 32-bit float
inline float decode_float10(uint32_t bits)
{
    if (bits == 0)
        return 0.0f;

    uint32_t exponent = (bits >> 5) & 0x1F; // 5 bits
    uint32_t mantissa = bits & 0x1F;        // 5 bits

    if (exponent == 0)
    {
        // Denormalized number
        return std::ldexp(float(mantissa) / 32.0f, -14);
    }
    else if (exponent == 31)
    {
        // Infinity or NaN
        return mantissa ? NAN : INFINITY;
    }
    else
    {
        // Normalized number
        float m = 1.0f + float(mantissa) / 32.0f;
        return std::ldexp(m, int(exponent) - 15);
    }
}

/// Useful for sign-extended right shifts for signed types
template <typename T>
inline T arithmetic_right_shift(T value, unsigned int n)
{
    if constexpr (std::is_unsigned<T>::value)
        return value >> n; // Logical right shift for unsigned types
    else
    {
        // Arithmetic right shift for signed types
        if (value >= 0)
            return value >> n; // Logical right shift for non-negative values
        else
            return (value + (T(1) << n) - 1) >> n; // Adjust for negative values
    }
}

inline float decode_float9_exp_5(uint32_t mantissa9, uint32_t shared_exp_bits)
{
    constexpr int bias = 15;

    if (shared_exp_bits == 0)
        // Exponent = 0 means zero or subnormal → result is 0.0f in this format
        return 0.0f;

    int exponent = shared_exp_bits - bias;

    // mantissa9 is 9-bit fraction representing fraction/512 (2^9)
    float mantissa = mantissa9 / 512.0f;

    // value = mantissa * 2^exponent
    return std::ldexp(mantissa, exponent);
}

// see https://learn.microsoft.com/en-us/windows-hardware/drivers/display/xr-bias-to-float-conversion-rules
inline float xr_bias_to_float(int bits) { return (bits - 384) / 510.f; }

} // namespace smalldds

#ifdef SMALLDDS_IMPLEMENTATION

#if _WIN32
#undef min
#undef max
#endif // _Win32

#include <fstream>

namespace smalldds
{

const char DDSFile::Magic[4] = {'D', 'D', 'S', ' '};

std::string fourCC_to_string(const std::array<char, 4> &fourCC)
{
    bool all_printable = true;
    all_printable &= (fourCC[0] >= '!' && fourCC[0] <= '~');
    all_printable &= (fourCC[1] >= '!' && fourCC[1] <= '~');
    all_printable &= (fourCC[2] >= '!' && fourCC[2] <= '~');
    all_printable &= (fourCC[3] >= '!' && fourCC[3] <= '~');

    if (all_printable)
        return std::string(fourCC.data(), fourCC.size());

    std::string s;
    for (int b = 0; b < 4; b++)
    {
        if (fourCC[b] >= '!' && fourCC[b] <= '~')
            s += "'" + std::string(1, fourCC[b]) + "'";
        else
            s += std::to_string(static_cast<uint8_t>(fourCC[b]));

        if (b != 3)
            s += ", ";
    }
    return s;
}

std::string fourCC_to_string(uint32_t fourCC)
{
    return fourCC_to_string({static_cast<char>((fourCC >> 0) & 0xFF), static_cast<char>((fourCC >> 8) & 0xFF),
                             static_cast<char>((fourCC >> 16) & 0xFF), static_cast<char>((fourCC >> 24) & 0xFF)});
}

const char *color_transform_name(DDSFile::ColorTransform t)
{
    switch (t)
    {
    case DDSFile::ColorTransform::eNone: return "None";
    case DDSFile::ColorTransform::eLuminance: return "Luminance";
    case DDSFile::ColorTransform::eAGBR: return "AGBR (RXGB)";
    case DDSFile::ColorTransform::eYUV: return "YUV";
    case DDSFile::ColorTransform::eYCoCg: return "YCoCg";
    case DDSFile::ColorTransform::eYCoCgScaled: return "YCoCg Scaled";
    case DDSFile::ColorTransform::eAEXP: return "AEXP";
    case DDSFile::ColorTransform::eSwapRG: return "Swap RG";
    case DDSFile::ColorTransform::eSwapRB: return "Swap RB";
    case DDSFile::ColorTransform::eOrthographicNormal: return "Orthographic Normal";
    default: return "Unknown";
    }
}

const char *format_name(DDSFile::DXGIFormat fmt)
{
#define CASE_RETURN_STRING(PAR)                                                                                        \
    case DDSFile::PAR: return #PAR

    switch (fmt)
    {
        CASE_RETURN_STRING(Format_Unknown);
        CASE_RETURN_STRING(R32G32B32A32_Typeless);
        CASE_RETURN_STRING(R32G32B32A32_Float);
        CASE_RETURN_STRING(R32G32B32A32_UInt);
        CASE_RETURN_STRING(R32G32B32A32_SInt);
        CASE_RETURN_STRING(R32G32B32_Typeless);
        CASE_RETURN_STRING(R32G32B32_Float);
        CASE_RETURN_STRING(R32G32B32_UInt);
        CASE_RETURN_STRING(R32G32B32_SInt);
        CASE_RETURN_STRING(R16G16B16A16_Typeless);
        CASE_RETURN_STRING(R16G16B16A16_Float);
        CASE_RETURN_STRING(R16G16B16A16_UNorm);
        CASE_RETURN_STRING(R16G16B16A16_UInt);
        CASE_RETURN_STRING(R16G16B16A16_SNorm);
        CASE_RETURN_STRING(R16G16B16A16_SInt);
        CASE_RETURN_STRING(R32G32_Typeless);
        CASE_RETURN_STRING(R32G32_Float);
        CASE_RETURN_STRING(R32G32_UInt);
        CASE_RETURN_STRING(R32G32_SInt);
        CASE_RETURN_STRING(R32G8X24_Typeless);
        CASE_RETURN_STRING(D32_Float_S8X24_UInt);
        CASE_RETURN_STRING(R32_Float_X8X24_Typeless);
        CASE_RETURN_STRING(X32_Typeless_G8X24_UInt);
        CASE_RETURN_STRING(R10G10B10A2_Typeless);
        CASE_RETURN_STRING(R10G10B10A2_UNorm);
        CASE_RETURN_STRING(R10G10B10A2_UInt);
        CASE_RETURN_STRING(R11G11B10_Float);
        CASE_RETURN_STRING(R8G8B8A8_Typeless);
        CASE_RETURN_STRING(R8G8B8A8_UNorm);
        CASE_RETURN_STRING(R8G8B8A8_UNorm_SRGB);
        CASE_RETURN_STRING(R8G8B8A8_UInt);
        CASE_RETURN_STRING(R8G8B8A8_SNorm);
        CASE_RETURN_STRING(R8G8B8A8_SInt);
        CASE_RETURN_STRING(R16G16_Typeless);
        CASE_RETURN_STRING(R16G16_Float);
        CASE_RETURN_STRING(R16G16_UNorm);
        CASE_RETURN_STRING(R16G16_UInt);
        CASE_RETURN_STRING(R16G16_SNorm);
        CASE_RETURN_STRING(R16G16_SInt);
        CASE_RETURN_STRING(R32_Typeless);
        CASE_RETURN_STRING(D32_Float);
        CASE_RETURN_STRING(R32_Float);
        CASE_RETURN_STRING(R32_UInt);
        CASE_RETURN_STRING(R32_SInt);
        CASE_RETURN_STRING(R24G8_Typeless);
        CASE_RETURN_STRING(D24_UNorm_S8_UInt);
        CASE_RETURN_STRING(R24_UNorm_X8_Typeless);
        CASE_RETURN_STRING(X24_Typeless_G8_UInt);
        CASE_RETURN_STRING(R8G8_Typeless);
        CASE_RETURN_STRING(R8G8_UNorm);
        CASE_RETURN_STRING(R8G8_UInt);
        CASE_RETURN_STRING(R8G8_SNorm);
        CASE_RETURN_STRING(R8G8_SInt);
        CASE_RETURN_STRING(R16_Typeless);
        CASE_RETURN_STRING(R16_Float);
        CASE_RETURN_STRING(D16_UNorm);
        CASE_RETURN_STRING(R16_UNorm);
        CASE_RETURN_STRING(R16_UInt);
        CASE_RETURN_STRING(R16_SNorm);
        CASE_RETURN_STRING(R16_SInt);
        CASE_RETURN_STRING(R8_Typeless);
        CASE_RETURN_STRING(R8_UNorm);
        CASE_RETURN_STRING(R8_UInt);
        CASE_RETURN_STRING(R8_SNorm);
        CASE_RETURN_STRING(R8_SInt);
        CASE_RETURN_STRING(A8_UNorm);
        CASE_RETURN_STRING(R1_UNorm);
        CASE_RETURN_STRING(R9G9B9E5_SHAREDEXP);
        CASE_RETURN_STRING(R8G8_B8G8_UNorm);
        CASE_RETURN_STRING(G8R8_G8B8_UNorm);
        CASE_RETURN_STRING(BC1_Typeless);
        CASE_RETURN_STRING(BC1_UNorm);
        CASE_RETURN_STRING(BC1_UNorm_SRGB);
        CASE_RETURN_STRING(BC2_Typeless);
        CASE_RETURN_STRING(BC2_UNorm);
        CASE_RETURN_STRING(BC2_UNorm_SRGB);
        CASE_RETURN_STRING(BC3_Typeless);
        CASE_RETURN_STRING(BC3_UNorm);
        CASE_RETURN_STRING(BC3_UNorm_SRGB);
        CASE_RETURN_STRING(BC4_Typeless);
        CASE_RETURN_STRING(BC4_UNorm);
        CASE_RETURN_STRING(BC4_SNorm);
        CASE_RETURN_STRING(BC5_Typeless);
        CASE_RETURN_STRING(BC5_UNorm);
        CASE_RETURN_STRING(BC5_SNorm);
        CASE_RETURN_STRING(B5G6R5_UNorm);
        CASE_RETURN_STRING(B5G5R5A1_UNorm);
        CASE_RETURN_STRING(B8G8R8A8_UNorm);
        CASE_RETURN_STRING(B8G8R8X8_UNorm);
        CASE_RETURN_STRING(R10G10B10_XR_BIAS_A2_UNorm);
        CASE_RETURN_STRING(B8G8R8A8_Typeless);
        CASE_RETURN_STRING(B8G8R8A8_UNorm_SRGB);
        CASE_RETURN_STRING(B8G8R8X8_Typeless);
        CASE_RETURN_STRING(B8G8R8X8_UNorm_SRGB);
        CASE_RETURN_STRING(BC6H_Typeless);
        CASE_RETURN_STRING(BC6H_UF16);
        CASE_RETURN_STRING(BC6H_SF16);
        CASE_RETURN_STRING(BC7_Typeless);
        CASE_RETURN_STRING(BC7_UNorm);
        CASE_RETURN_STRING(BC7_UNorm_SRGB);
        CASE_RETURN_STRING(AYUV);
        CASE_RETURN_STRING(Y410);
        CASE_RETURN_STRING(Y416);
        CASE_RETURN_STRING(NV12);
        CASE_RETURN_STRING(P010);
        CASE_RETURN_STRING(P016);
        CASE_RETURN_STRING(YUV420_OPAQUE);
        CASE_RETURN_STRING(YUY2);
        CASE_RETURN_STRING(Y210);
        CASE_RETURN_STRING(Y216);
        CASE_RETURN_STRING(NV11);
        CASE_RETURN_STRING(AI44);
        CASE_RETURN_STRING(IA44);
        CASE_RETURN_STRING(P8);
        CASE_RETURN_STRING(A8P8);
        CASE_RETURN_STRING(B4G4R4A4_UNorm);
        CASE_RETURN_STRING(P208);
        CASE_RETURN_STRING(V208);
        CASE_RETURN_STRING(V408);
        CASE_RETURN_STRING(ASTC_4X4_Typeless);
        CASE_RETURN_STRING(ASTC_4X4_UNorm);
        CASE_RETURN_STRING(ASTC_4X4_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_5X4_Typeless);
        CASE_RETURN_STRING(ASTC_5X4_UNorm);
        CASE_RETURN_STRING(ASTC_5X4_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_5X5_Typeless);
        CASE_RETURN_STRING(ASTC_5X5_UNorm);
        CASE_RETURN_STRING(ASTC_5X5_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_6X5_Typeless);
        CASE_RETURN_STRING(ASTC_6X5_UNorm);
        CASE_RETURN_STRING(ASTC_6X5_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_6X6_Typeless);
        CASE_RETURN_STRING(ASTC_6X6_UNorm);
        CASE_RETURN_STRING(ASTC_6X6_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_8X5_Typeless);
        CASE_RETURN_STRING(ASTC_8X5_UNorm);
        CASE_RETURN_STRING(ASTC_8X5_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_8X6_Typeless);
        CASE_RETURN_STRING(ASTC_8X6_UNorm);
        CASE_RETURN_STRING(ASTC_8X6_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_8X8_Typeless);
        CASE_RETURN_STRING(ASTC_8X8_UNorm);
        CASE_RETURN_STRING(ASTC_8X8_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_10X5_Typeless);
        CASE_RETURN_STRING(ASTC_10X5_UNorm);
        CASE_RETURN_STRING(ASTC_10X5_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_10X6_Typeless);
        CASE_RETURN_STRING(ASTC_10X6_UNorm);
        CASE_RETURN_STRING(ASTC_10X6_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_10X8_Typeless);
        CASE_RETURN_STRING(ASTC_10X8_UNorm);
        CASE_RETURN_STRING(ASTC_10X8_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_10X10_Typeless);
        CASE_RETURN_STRING(ASTC_10X10_UNorm);
        CASE_RETURN_STRING(ASTC_10X10_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_12X10_Typeless);
        CASE_RETURN_STRING(ASTC_12X10_UNorm);
        CASE_RETURN_STRING(ASTC_12X10_UNorm_SRGB);
        CASE_RETURN_STRING(ASTC_12X12_Typeless);
        CASE_RETURN_STRING(ASTC_12X12_UNorm);
        CASE_RETURN_STRING(ASTC_12X12_UNorm_SRGB);
        CASE_RETURN_STRING(A4B4G4R4_UNorm);
    default: return "Unknown";
    }
#undef CASE_RETURN_STRING
}

const char *compression_name(DDSFile::Compression cmp)
{
    switch (cmp)
    {
    default:
    case DDSFile::Compression::None: return "None";
    case DDSFile::Compression::BC1_DXT1: return "BC1/DXT1";
    case DDSFile::Compression::BC2_DXT2: return "BC2/DXT2";
    case DDSFile::Compression::BC2_DXT3: return "BC2/DXT3";
    case DDSFile::Compression::BC3_DXT4: return "BC3/DXT4";
    case DDSFile::Compression::BC3_DXT5: return "BC3/DXT5";
    case DDSFile::Compression::BC4: return "BC4";
    case DDSFile::Compression::BC5: return "BC5";
    case DDSFile::Compression::BC6HU: return "BC6HU";
    case DDSFile::Compression::BC6HS: return "BC6HS";
    case DDSFile::Compression::BC7: return "BC7";
    case DDSFile::Compression::ASTC: return "ASTC";
    }
}

const char *alpha_mode_name(uint32_t a)
{
    switch (a)
    {
    default:
    case DDSFile::ALPHA_MODE_UNKNOWN: return "Unknown";
    case DDSFile::ALPHA_MODE_STRAIGHT: return "Straight";
    case DDSFile::ALPHA_MODE_PREMULTIPLIED: return "Premultiplied";
    case DDSFile::ALPHA_MODE_OPAQUE: return "Opaque";
    case DDSFile::ALPHA_MODE_CUSTOM: return "Custom";
    }
}

/// Compute the number of bits set in a bitmask and the number of bits to shift
/// to the right to extract the first bit.
void DDSFile::calc_shifts(uint32_t mask, uint32_t &count, uint32_t &right)
{
    if (mask == 0)
    {
        count = right = 0;
        return;
    }

    int i;
    for (i = 0; i < 32; i++, mask >>= 1)
        if (mask & 1)
            break;
    right = i;

    for (i = 0; i < 32; i++, mask >>= 1)
        if (!(mask & 1))
            break;

    count = i;
}

bool DDSFile::is_compressed(DXGIFormat fmt)
{
    return (fmt >= BC1_Typeless && fmt <= BC7_UNorm_SRGB) || (fmt >= ASTC_4X4_Typeless && fmt <= ASTC_12X12_UNorm_SRGB);
}

DDSFile::DataType DDSFile::data_type(DDSFile::DXGIFormat fmt)
{
    using DXGI = DXGIFormat;

    // num_file_channels = 0;
    // First, determine type
    switch (fmt)
    {
    // 8-bit typeless formats
    case DXGI::BC1_Typeless:
    case DXGI::BC2_Typeless:
    case DXGI::BC3_Typeless:
    case DXGI::BC4_Typeless:
    case DXGI::BC5_Typeless:
    case DXGI::BC7_Typeless:
    case DXGI::R8_Typeless:
    case DXGI::R8G8_Typeless:
    case DXGI::R8G8B8A8_Typeless:
    case DXGI::B8G8R8A8_Typeless:
    case DXGI::B8G8R8X8_Typeless: return DataType::Typeless8;

    // UNorm8 formats (compressed and uncompressed)
    case DXGI::BC1_UNorm:
    case DXGI::BC1_UNorm_SRGB:
    case DXGI::BC2_UNorm:
    case DXGI::BC2_UNorm_SRGB:
    case DXGI::BC3_UNorm:
    case DXGI::BC3_UNorm_SRGB:
    case DXGI::BC4_UNorm:
    case DXGI::BC5_UNorm:
    case DXGI::BC7_UNorm:
    case DXGI::BC7_UNorm_SRGB:
    case DXGI::A8_UNorm:
    case DXGI::R8_UNorm:
    case DXGI::R8G8_UNorm:
    case DXGI::R8G8B8A8_UNorm:
    case DXGI::R8G8B8A8_UNorm_SRGB:
    case DXGI::B8G8R8A8_UNorm:
    case DXGI::B8G8R8A8_UNorm_SRGB:
    case DXGI::B8G8R8X8_UNorm:
    case DXGI::B8G8R8X8_UNorm_SRGB: return DataType::UNorm8;

    // SNorm8 formats (compressed and uncompressed)
    case DXGI::BC4_SNorm:
    case DXGI::BC5_SNorm:
    case DXGI::R8G8B8A8_SNorm:
    case DXGI::R8G8_SNorm:
    case DXGI::R8_SNorm: return DataType::SNorm8;

    // Float16 formats (compressed and uncompressed)
    case DXGI::BC6H_UF16:
    case DXGI::BC6H_SF16:
    case DXGI::R16G16B16A16_Float:
    case DXGI::R16G16_Float:
    case DXGI::R16_Float:
    // best guess for typeless
    case DXGI::BC6H_Typeless: return DataType::Float16;

    // Float32 formats (compressed and uncompressed)
    case DXGI::R32G32B32A32_Float:
    case DXGI::R32G32B32_Float:
    case DXGI::R32G32_Float:
    case DXGI::R32_Float:
    case DXGI::D32_Float: return DataType::Float32;

    // UInt32 formats
    case DXGI::R32G32B32A32_UInt:
    case DXGI::R32G32B32_UInt:
    case DXGI::R32G32_UInt:
    case DXGI::R32_UInt:
    // best guess for typeless formats
    case DXGI::R32G32B32A32_Typeless:
    case DXGI::R32G32B32_Typeless:
    case DXGI::R32G32_Typeless: return DataType::UInt32;

    // UInt16 formats
    case DXGI::R16G16B16A16_UInt:
    case DXGI::R16G16_UInt:
    case DXGI::R16_UInt: return DataType::UInt16;

    // UInt8 formats
    case DXGI::R8G8B8A8_UInt:
    case DXGI::R8G8_UInt:
    case DXGI::R8_UInt: return DataType::UInt8;

    // SInt32 formats
    case DXGI::R32G32B32A32_SInt:
    case DXGI::R32G32B32_SInt:
    case DXGI::R32G32_SInt:
    case DXGI::R32_SInt: return DataType::SInt32;

    // SInt16 formats
    case DXGI::R16G16B16A16_SInt:
    case DXGI::R16G16_SInt:
    case DXGI::R16_SInt: return DataType::SInt16;

    // SInt8 formats
    case DXGI::R8G8B8A8_SInt:
    case DXGI::R8G8_SInt:
    case DXGI::R8_SInt: return DataType::SInt8;

    // SNorm16 formats
    case DXGI::R16G16B16A16_SNorm:
    case DXGI::R16G16_SNorm:
    case DXGI::R16_SNorm: return DataType::SNorm16;

    // UNorm16 formats
    case DXGI::R16G16B16A16_UNorm:
    case DXGI::R16G16_UNorm:
    case DXGI::R16_UNorm:
    case DXGI::D16_UNorm: return DataType::UNorm16;

    // Packed formats that require special handling
    case DXGI::R11G11B10_Float:
    case DXGI::B5G5R5A1_UNorm:
    case DXGI::R32G8X24_Typeless:
    case DXGI::D32_Float_S8X24_UInt:
    case DXGI::R32_Float_X8X24_Typeless:
    case DXGI::X32_Typeless_G8X24_UInt:
    case DXGI::R24G8_Typeless:
    case DXGI::D24_UNorm_S8_UInt:
    case DXGI::R24_UNorm_X8_Typeless:
    case DXGI::B4G4R4A4_UNorm:
    case DXGI::A4B4G4R4_UNorm:
    case DXGI::X24_Typeless_G8_UInt:
    case DXGI::B5G6R5_UNorm:
    case DXGI::R10G10B10A2_Typeless:
    case DXGI::R10G10B10A2_UNorm:
    case DXGI::R10G10B10A2_UInt:
    case DXGI::R9G9B9E5_SHAREDEXP:
    case DXGI::R10G10B10_XR_BIAS_A2_UNorm:
    case DXGI::R1_UNorm: return DataType::Packed;

    // We don't currently handle the remaining cases, which are planar and YUV formats
    // case DXGI::AYUV:
    // case DXGI::Y410:
    // case DXGI::Y416:
    // case DXGI::NV12:
    // case DXGI::P010:
    // case DXGI::P016:
    // case DXGI::YUV420_OPAQUE:
    // case DXGI::YUY2:
    // case DXGI::Y210:
    // case DXGI::Y216:
    // case DXGI::NV11:
    // case DXGI::AI44:
    // case DXGI::IA44:
    // case DXGI::P8:
    // case DXGI::A8P8:
    // case DXGI::P208:
    // case DXGI::V208:
    // case DXGI::V408:
    case DXGI::Format_Unknown:
    default: return DataType::Unknown;
    }
}

size_t DDSFile::data_type_size(DataType type)
{
    switch (type)
    {
    case DataType::Typeless8:
    case DataType::SNorm8:
    case DataType::UNorm8:
    case DataType::UInt8:
    case DataType::SInt8: return 1;
    case DataType::Typeless16:
    case DataType::SNorm16:
    case DataType::UNorm16:
    case DataType::UInt16:
    case DataType::SInt16:
    case DataType::Float16: return 2;
    case DataType::Typeless32:
    case DataType::Float32:
    case DataType::UInt32:
    case DataType::SInt32: return 4;
    case DataType::Packed: return 0; // Packed formats have variable sizes
    default: return 0;               // Unknown or unsupported types
    }
}

uint32_t DDSFile::block_width() const
{
    switch (header_DXT10.format)
    {
    case BC1_Typeless:
    case BC1_UNorm:
    case BC1_UNorm_SRGB:
    case BC4_Typeless:
    case BC4_UNorm:
    case BC4_SNorm:
    case BC2_Typeless:
    case BC2_UNorm:
    case BC2_UNorm_SRGB:
    case BC3_Typeless:
    case BC3_UNorm:
    case BC3_UNorm_SRGB:
    case BC5_Typeless:
    case BC5_UNorm:
    case BC5_SNorm:
    case BC6H_Typeless:
    case BC6H_UF16:
    case BC6H_SF16:
    case BC7_Typeless:
    case BC7_UNorm:
    case BC7_UNorm_SRGB: return 4;
    case ASTC_4X4_Typeless:
    case ASTC_4X4_UNorm:
    case ASTC_4X4_UNorm_SRGB: return 4;
    case ASTC_5X4_Typeless:
    case ASTC_5X4_UNorm:
    case ASTC_5X4_UNorm_SRGB:
    case ASTC_5X5_Typeless:
    case ASTC_5X5_UNorm:
    case ASTC_5X5_UNorm_SRGB: return 5;
    case ASTC_6X5_Typeless:
    case ASTC_6X5_UNorm:
    case ASTC_6X5_UNorm_SRGB:
    case ASTC_6X6_Typeless:
    case ASTC_6X6_UNorm:
    case ASTC_6X6_UNorm_SRGB: return 6;
    case ASTC_8X5_Typeless:
    case ASTC_8X5_UNorm:
    case ASTC_8X5_UNorm_SRGB:
    case ASTC_8X6_Typeless:
    case ASTC_8X6_UNorm:
    case ASTC_8X6_UNorm_SRGB:
    case ASTC_8X8_Typeless:
    case ASTC_8X8_UNorm:
    case ASTC_8X8_UNorm_SRGB: return 8;
    case ASTC_10X5_Typeless:
    case ASTC_10X5_UNorm:
    case ASTC_10X5_UNorm_SRGB:
    case ASTC_10X6_Typeless:
    case ASTC_10X6_UNorm:
    case ASTC_10X6_UNorm_SRGB:
    case ASTC_10X8_Typeless:
    case ASTC_10X8_UNorm:
    case ASTC_10X8_UNorm_SRGB:
    case ASTC_10X10_Typeless:
    case ASTC_10X10_UNorm:
    case ASTC_10X10_UNorm_SRGB: return 10;
    case ASTC_12X10_Typeless:
    case ASTC_12X10_UNorm:
    case ASTC_12X10_UNorm_SRGB:
    case ASTC_12X12_Typeless:
    case ASTC_12X12_UNorm:
    case ASTC_12X12_UNorm_SRGB: return 12;
    default: return 1;
    }
}

uint32_t DDSFile::block_height() const
{
    switch (header_DXT10.format)
    {
    case BC1_Typeless:
    case BC1_UNorm:
    case BC1_UNorm_SRGB:
    case BC4_Typeless:
    case BC4_UNorm:
    case BC4_SNorm:
    case BC2_Typeless:
    case BC2_UNorm:
    case BC2_UNorm_SRGB:
    case BC3_Typeless:
    case BC3_UNorm:
    case BC3_UNorm_SRGB:
    case BC5_Typeless:
    case BC5_UNorm:
    case BC5_SNorm:
    case BC6H_Typeless:
    case BC6H_UF16:
    case BC6H_SF16:
    case BC7_Typeless:
    case BC7_UNorm:
    case BC7_UNorm_SRGB: return 4; // BCn formats are always 4x4 blocks
    case ASTC_4X4_Typeless:
    case ASTC_4X4_UNorm:
    case ASTC_4X4_UNorm_SRGB:
    case ASTC_5X4_Typeless:
    case ASTC_5X4_UNorm:
    case ASTC_5X4_UNorm_SRGB: return 4;
    case ASTC_5X5_Typeless:
    case ASTC_5X5_UNorm:
    case ASTC_5X5_UNorm_SRGB:
    case ASTC_6X5_Typeless:
    case ASTC_6X5_UNorm:
    case ASTC_6X5_UNorm_SRGB:
    case ASTC_8X5_Typeless:
    case ASTC_8X5_UNorm:
    case ASTC_8X5_UNorm_SRGB:
    case ASTC_10X5_Typeless:
    case ASTC_10X5_UNorm:
    case ASTC_10X5_UNorm_SRGB: return 5;
    case ASTC_6X6_Typeless:
    case ASTC_6X6_UNorm:
    case ASTC_6X6_UNorm_SRGB:
    case ASTC_8X6_Typeless:
    case ASTC_8X6_UNorm:
    case ASTC_8X6_UNorm_SRGB:
    case ASTC_10X6_Typeless:
    case ASTC_10X6_UNorm:
    case ASTC_10X6_UNorm_SRGB: return 6;
    case ASTC_8X8_Typeless:
    case ASTC_8X8_UNorm:
    case ASTC_8X8_UNorm_SRGB:
    case ASTC_10X8_Typeless:
    case ASTC_10X8_UNorm:
    case ASTC_10X8_UNorm_SRGB: return 8;
    case ASTC_10X10_Typeless:
    case ASTC_10X10_UNorm:
    case ASTC_10X10_UNorm_SRGB:
    case ASTC_12X10_Typeless:
    case ASTC_12X10_UNorm:
    case ASTC_12X10_UNorm_SRGB: return 10;
    case ASTC_12X12_Typeless:
    case ASTC_12X12_UNorm:
    case ASTC_12X12_UNorm_SRGB: return 12;
    default: return 1;
    }
}

bool DDSFile::is_sRGB() const
{
    if (header.pixel_format.flags & uint32_t(PixelFormatFlagBits::SRGB))
    {
        // The sRGB flag is set, so we assume the format is sRGB.
        return true;
    }

    // Check the DXGI format for sRGB variants.
    switch (header_DXT10.format)
    {
    case R8G8B8A8_UNorm_SRGB:
    case B8G8R8A8_UNorm_SRGB:
    case B8G8R8X8_UNorm_SRGB:
    case BC1_UNorm_SRGB:
    case BC2_UNorm_SRGB:
    case BC3_UNorm_SRGB:
    case BC7_UNorm_SRGB:
    case ASTC_4X4_UNorm_SRGB:
    case ASTC_5X4_UNorm_SRGB:
    case ASTC_5X5_UNorm_SRGB:
    case ASTC_6X5_UNorm_SRGB:
    case ASTC_6X6_UNorm_SRGB:
    case ASTC_8X5_UNorm_SRGB:
    case ASTC_8X6_UNorm_SRGB:
    case ASTC_8X8_UNorm_SRGB:
    case ASTC_10X5_UNorm_SRGB:
    case ASTC_10X6_UNorm_SRGB:
    case ASTC_10X8_UNorm_SRGB:
    case ASTC_10X10_UNorm_SRGB:
    case ASTC_12X10_UNorm_SRGB:
    case ASTC_12X12_UNorm_SRGB:
        // These formats are explicitly defined as sRGB.
        return true;
    default:
        // If the format is not explicitly defined as sRGB, we assume it is
        // not.
        return false;
    }
}

void DDSFile::deduce_bitmasks_from_pixel_format()
{
    const auto &pf = header.pixel_format;
    if ((pf.flags & uint32_t(PixelFormatFlagBits::BumpDuDv)) != 0)
    {
        bitmask_was_bump_du_dv = true;
        bitmask_has_rgb        = true;
    }
    bitmask_has_alpha =
        ((pf.flags & (uint32_t(PixelFormatFlagBits::AlphaPixels) | uint32_t(PixelFormatFlagBits::AlphaOnly))) != 0);
    bitmask_has_rgb |= ((pf.flags & (uint32_t(PixelFormatFlagBits::YUV) | uint32_t(PixelFormatFlagBits::Luminance) |
                                     uint32_t(PixelFormatFlagBits::RGB))) != 0);
    bitmasked = true;
}

DDSFile::DXGIFormat DDSFile::deduce_format_from_fourCC(Result &res)
{
    const auto &pf = header.pixel_format;

    bool has_fourCC = header.pixel_format.flags & uint32_t(PixelFormatFlagBits::FourCC);
    if (!has_fourCC && pf.fourCC != 0)
    {
        res.add_message(Result::Warning, std::string("DDSFile: pixel format has non-zero fourCC (") +
                                             fourCC_to_string(pf.fourCC) +
                                             "), but the FourCC flag is not set. Assuming FourCC is valid.");
        has_fourCC = true;
        header.pixel_format.flags |= uint32_t(PixelFormatFlagBits::FourCC);
    }

    // NOTE: We overwrite the header itself below to propagate information to the bitmasking code!
    if (has_fourCC)
    {
        // Detect the format based on the format code:
        switch (pf.fourCC)
        {
        case FOURCC_DXT1: compression = Compression::BC1_DXT1; return BC1_UNorm;
        case FOURCC_DXT2: compression = Compression::BC2_DXT2; return BC2_UNorm;
        case FOURCC_DXT3: compression = Compression::BC2_DXT3; return BC2_UNorm;
        case FOURCC_DXT4: compression = Compression::BC3_DXT4; return BC3_UNorm;
        case FOURCC_DXT5: compression = Compression::BC3_DXT5; return BC3_UNorm;
        case FOURCC_RXGB:
            compression     = Compression::BC3_DXT5;
            color_transform = ColorTransform::eAGBR;
            header.pixel_format.flags &= ~uint32_t(PixelFormatFlagBits::Normal);
            return BC3_UNorm;
        case FOURCC_BC4U:
        case FOURCC_ATI1: compression = Compression::BC4; return BC4_UNorm;
        case FOURCC_BC4S: compression = Compression::BC4; return BC4_SNorm;
        case FOURCC_BC5U: compression = Compression::BC5; return BC5_UNorm;
        case FOURCC_ATI2:
            // ATI2 is BC5 but with the red and green channels swapped.
            if (color_transform == ColorTransform::eNone)
                color_transform = ColorTransform::eSwapRG;
            else if (color_transform == ColorTransform::eSwapRG)
                color_transform = ColorTransform::eNone;
            // else // shouldn't happen, but we'll let it slide
            compression = Compression::BC5;
            return BC5_UNorm;
        case FOURCC_BC5S: compression = Compression::BC5; return BC5_SNorm;
        case FOURCC_BC6H: compression = Compression::BC6HU; return BC6H_UF16;
        case FOURCC_BC7L:
        case FOURCC_BC70:
        case FOURCC_ZOLA: compression = Compression::BC7; return BC7_UNorm;
        case FOURCC_RGBG: return R8G8_B8G8_UNorm;
        case FOURCC_GRGB: return G8R8_G8B8_UNorm;
        case FOURCC_YUY2: return YUY2;
        case FOURCC_UYVY: return R8G8_B8G8_UNorm;

        // ASTC formats
        case FOURCC_ASTC4x4: compression = Compression::ASTC; return ASTC_4X4_UNorm;
        case FOURCC_ASTC5x4: compression = Compression::ASTC; return ASTC_5X4_UNorm;
        case FOURCC_ASTC5x5: compression = Compression::ASTC; return ASTC_5X5_UNorm;
        case FOURCC_ASTC6x5: compression = Compression::ASTC; return ASTC_6X5_UNorm;
        case FOURCC_ASTC6x6: compression = Compression::ASTC; return ASTC_6X6_UNorm;
        case FOURCC_ASTC8x5: compression = Compression::ASTC; return ASTC_8X5_UNorm;
        case FOURCC_ASTC8x6: compression = Compression::ASTC; return ASTC_8X6_UNorm;
        case FOURCC_ASTC8x8: compression = Compression::ASTC; return ASTC_8X8_UNorm;
        case FOURCC_ASTC10x5:
        case FOURCC_ASTC10x5_ALT: compression = Compression::ASTC; return ASTC_10X5_UNorm;
        case FOURCC_ASTC10x6:
        case FOURCC_ASTC10x6_ALT: compression = Compression::ASTC; return ASTC_10X6_UNorm;
        case FOURCC_ASTC10x8:
        case FOURCC_ASTC10x8_ALT: compression = Compression::ASTC; return ASTC_10X8_UNorm;
        case FOURCC_ASTC10x10:
        case FOURCC_ASTC10x10_ALT: compression = Compression::ASTC; return ASTC_10X10_UNorm;
        case FOURCC_ASTC12x10:
        case FOURCC_ASTC12x10_ALT: compression = Compression::ASTC; return ASTC_12X10_UNorm;
        case FOURCC_ASTC12x12:
        case FOURCC_ASTC12x12_ALT: compression = Compression::ASTC; return ASTC_12X12_UNorm;

        case FOURCC_DX10:
        {
            switch (header_DXT10.format)
            {
            // compressed formats
            default:
                if (header_DXT10.format >= ASTC_4X4_Typeless && header_DXT10.format <= ASTC_12X12_UNorm_SRGB)
                    compression = Compression::ASTC;
                break;
            case BC1_UNorm:
            case BC1_UNorm_SRGB: compression = Compression::BC1_DXT1; break;
            case BC2_UNorm:
            case BC2_UNorm_SRGB: compression = Compression::BC2_DXT3; break;
            case BC3_UNorm:
            case BC3_UNorm_SRGB: compression = Compression::BC3_DXT5; break;
            case BC4_UNorm:
            case BC4_SNorm: compression = Compression::BC4; break;
            case BC5_UNorm:
            case BC5_SNorm: compression = Compression::BC5; break;
            case BC6H_UF16: compression = Compression::BC6HU; break;
            case BC6H_SF16: compression = Compression::BC6HS; break;
            case BC7_UNorm:
            case BC7_UNorm_SRGB: compression = Compression::BC7; break;

            // uncompressed but packed formats that we need to handle
            // with bitmasks
            case R10G10B10A2_Typeless:
            case R10G10B10A2_UNorm:
            case R10G10B10A2_UInt:
                header.pixel_format.bit_count = 32;
                header.pixel_format.masks[0]  = 0b00000000000000000000001111111111;
                header.pixel_format.masks[1]  = 0b00000000000011111111110000000000;
                header.pixel_format.masks[2]  = 0b00111111111100000000000000000000;
                header.pixel_format.masks[3]  = 0b11000000000000000000000000000000;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = true;
                break;
            case A4B4G4R4_UNorm:
                header.pixel_format.bit_count = 16;
                header.pixel_format.masks[0]  = 0xf000;
                header.pixel_format.masks[1]  = 0x0f00;
                header.pixel_format.masks[2]  = 0x00f0;
                header.pixel_format.masks[3]  = 0x000f;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = true;
                break;
            case B4G4R4A4_UNorm:
                header.pixel_format.bit_count = 16;
                header.pixel_format.masks[0]  = 0x0f00;
                header.pixel_format.masks[1]  = 0x00f0;
                header.pixel_format.masks[2]  = 0x000f;
                header.pixel_format.masks[3]  = 0xff00;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = true;
                break;
            case B5G5R5A1_UNorm:
                header.pixel_format.bit_count = 16;
                header.pixel_format.masks[0]  = 0x7C00;
                header.pixel_format.masks[1]  = 0x03E0;
                header.pixel_format.masks[2]  = 0x001F;
                header.pixel_format.masks[3]  = 0x8000;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = true;
                break;
            case B5G6R5_UNorm:
                header.pixel_format.bit_count = 16;
                header.pixel_format.masks[2]  = 0x001F;
                header.pixel_format.masks[1]  = 0x07E0;
                header.pixel_format.masks[0]  = 0xF800;
                header.pixel_format.masks[3]  = 0x0000;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = false;
                break;
            case R11G11B10_Float:
                header.pixel_format.bit_count = 32;
                header.pixel_format.masks[0]  = 0x000003FF;
                header.pixel_format.masks[1]  = 0x007FF000;
                header.pixel_format.masks[2]  = 0xFFC00000;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = false;
                break;
            case R10G10B10_XR_BIAS_A2_UNorm:
                header.pixel_format.bit_count = 32;
                header.pixel_format.masks[0]  = 0x000003FF;
                header.pixel_format.masks[1]  = 0x000FFC00;
                header.pixel_format.masks[2]  = 0x3FF00000;
                header.pixel_format.masks[3]  = 0xC0000000;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = true;
                break;
            case R9G9B9E5_SHAREDEXP:
                header.pixel_format.bit_count = 32;
                header.pixel_format.masks[0]  = 0x000001FF;
                header.pixel_format.masks[1]  = 0x0003FE00;
                header.pixel_format.masks[2]  = 0x07FC0000;
                header.pixel_format.masks[3]  = 0xF8000000;
                bitmasked                     = true;
                bitmask_has_rgb               = true;
                bitmask_has_alpha             = false;
                break;
            }

            return header_DXT10.format;
        }

        // GLI and DirectXTex will write some DXGI formats without a
        // DX10 header and using Direct3D format numbers by default, so
        // we have to account for that here. Note that most of these are
        // untested - R and B swaps are likely!
        case D3DFMT_R8G8B8:
            header.pixel_format.bit_count = 24;
            header.pixel_format.masks[0]  = 0xff0000;
            header.pixel_format.masks[1]  = 0x00ff00;
            header.pixel_format.masks[2]  = 0x0000ff;
            header.pixel_format.masks[3]  = 0x000000;
            bitmask_has_alpha             = false;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return R8G8B8A8_UNorm;
        case D3DFMT_A8R8G8B8:
            header.pixel_format.bit_count = 32;
            header.pixel_format.masks[0]  = 0x00ff0000;
            header.pixel_format.masks[1]  = 0x0000ff00;
            header.pixel_format.masks[2]  = 0x000000ff;
            header.pixel_format.masks[3]  = 0xff000000;
            bitmask_has_alpha             = false;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return B8G8R8A8_UNorm;
        case D3DFMT_X8R8G8B8:
            header.pixel_format.bit_count = 32;
            header.pixel_format.masks[0]  = 0x00ff0000;
            header.pixel_format.masks[1]  = 0x0000ff00;
            header.pixel_format.masks[2]  = 0x000000ff;
            header.pixel_format.masks[3]  = 0x00000000;
            bitmask_has_alpha             = false;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return B8G8R8X8_UNorm;
        case D3DFMT_R5G6B5:
            header.pixel_format.bit_count = 16;
            header.pixel_format.masks[3]  = 0b0000000000000000;
            header.pixel_format.masks[0]  = 0b1111100000000000;
            header.pixel_format.masks[1]  = 0b0000011111100000;
            header.pixel_format.masks[2]  = 0b0000000000011111;
            bitmask_has_alpha             = false;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return B5G6R5_UNorm;
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A1R5G5B5:
            header.pixel_format.bit_count = 16;
            header.pixel_format.masks[3]  = pf.fourCC == D3DFMT_X1R5G5B5 ? 0 : 0b0000000000000001;
            header.pixel_format.masks[0]  = 0b1111100000000000;
            header.pixel_format.masks[1]  = 0b0000011111000000;
            header.pixel_format.masks[2]  = 0b0000000000111110;
            bitmask_has_alpha             = pf.fourCC == D3DFMT_A1R5G5B5;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return B5G5R5A1_UNorm;
        case D3DFMT_A4R4G4B4:
            header.pixel_format.bit_count = 16;
            header.pixel_format.masks[0]  = 0xf000;
            header.pixel_format.masks[1]  = 0x0f00;
            header.pixel_format.masks[2]  = 0x00f0;
            header.pixel_format.masks[3]  = 0x000f;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return A4B4G4R4_UNorm;
        case D3DFMT_R3G3B2:
            header.pixel_format.bit_count = 8;
            header.pixel_format.masks[3]  = 0;
            header.pixel_format.masks[0]  = 0b11100000;
            header.pixel_format.masks[1]  = 0b00011100;
            header.pixel_format.masks[2]  = 0b00000011;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return Format_Unknown;
        case D3DFMT_A8: return A8_UNorm;
        case D3DFMT_A8R3G3B2:
            header.pixel_format.bit_count = 16;
            header.pixel_format.masks[3]  = 0xff00;
            header.pixel_format.masks[0]  = 0x00e0;
            header.pixel_format.masks[1]  = 0x001c;
            header.pixel_format.masks[2]  = 0x0003;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return Format_Unknown;
        case D3DFMT_X4R4G4B4:
            header.pixel_format.bit_count = 16;
            header.pixel_format.masks[3]  = 0x0000;
            header.pixel_format.masks[0]  = 0x0f00;
            header.pixel_format.masks[1]  = 0x00f0;
            header.pixel_format.masks[2]  = 0x000f;
            bitmask_has_alpha             = false;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return Format_Unknown;
        case D3DFMT_A2B10G10R10:
            header.pixel_format.bit_count = 32;
            header.pixel_format.masks[0]  = 0b00000000000000000000001111111111;
            header.pixel_format.masks[1]  = 0b00000000000011111111110000000000;
            header.pixel_format.masks[2]  = 0b00111111111100000000000000000000;
            header.pixel_format.masks[3]  = 0b11000000000000000000000000000000;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return R10G10B10A2_UNorm;
        case D3DFMT_A8B8G8R8:
            header.pixel_format.bit_count = 32;
            header.pixel_format.masks[3]  = 0xff000000;
            header.pixel_format.masks[2]  = 0x00ff0000;
            header.pixel_format.masks[1]  = 0x0000ff00;
            header.pixel_format.masks[0]  = 0x000000ff;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return R8G8B8A8_UNorm;
        case D3DFMT_X8B8G8R8:
            header.pixel_format.bit_count = 32;
            header.pixel_format.masks[3]  = 0xff000000;
            header.pixel_format.masks[2]  = 0x00ff0000;
            header.pixel_format.masks[1]  = 0x0000ff00;
            header.pixel_format.masks[0]  = 0x00000000;
            bitmask_has_alpha             = false;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return R8G8B8A8_UNorm;
        case D3DFMT_G16R16: return R16G16_UNorm;
        case D3DFMT_A2R10G10B10:
            header.pixel_format.bit_count = 32;
            header.pixel_format.masks[0]  = 0b00000000000000000000001111111111;
            header.pixel_format.masks[1]  = 0b00000000000011111111110000000000;
            header.pixel_format.masks[2]  = 0b00111111111100000000000000000000;
            header.pixel_format.masks[3]  = 0b11000000000000000000000000000000;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            return R10G10B10A2_UNorm;
        case D3DFMT_A16B16G16R16: return R16G16B16A16_UNorm;
        case D3DFMT_L8:
            header.pixel_format.bit_count = 8;
            header.pixel_format.masks[0]  = 0xff;
            header.pixel_format.masks[1]  = 0x00;
            header.pixel_format.masks[2]  = 0x00;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            color_transform               = ColorTransform::eLuminance;
            return R8_UNorm;
        case D3DFMT_A8L8:
            header.pixel_format.bit_count = 16;
            header.pixel_format.masks[3]  = 0xff00;
            header.pixel_format.masks[0]  = 0x00ff;
            header.pixel_format.masks[1]  = 0x0000;
            header.pixel_format.masks[2]  = 0x0000;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            color_transform               = ColorTransform::eLuminance;
            return R32G32B32_Float;
        case D3DFMT_A4L4:
            header.pixel_format.bit_count = 8;
            header.pixel_format.masks[3]  = 0xf0;
            header.pixel_format.masks[0]  = 0x0f;
            header.pixel_format.masks[1]  = 0x00;
            header.pixel_format.masks[2]  = 0x00;
            bitmask_has_alpha             = true;
            bitmask_has_rgb               = true;
            bitmasked                     = true;
            color_transform               = ColorTransform::eLuminance;
            return R32G32B32_Float;
        case D3DFMT_V8U8: return R8G8_SNorm;
        case D3DFMT_Q8W8V8U8: return R8G8B8A8_SNorm;
        case D3DFMT_V16U16: return R16G16_SNorm;
        case D3DFMT_A2W10V10U10: return R10G10B10A2_UInt;
        case D3DFMT_D16:
        case D3DFMT_D16_LOCKABLE: return D16_UNorm;
        case D3DFMT_D32:
        case D3DFMT_D32F_LOCKABLE: return D32_Float;
        case D3DFMT_D24S8:
        case D3DFMT_D24X8:
        case D3DFMT_D24X4S4: return D24_UNorm_S8_UInt;
        case D3DFMT_S8_LOCKABLE: return R8_UInt;
        case D3DFMT_L16: color_transform = ColorTransform::eLuminance; return R16_UNorm;
        case D3DFMT_Q16W16V16U16: return R16G16B16A16_SNorm;
        case D3DFMT_R16F: return R16_Float;
        case D3DFMT_G16R16F: return R16G16_Float;
        case D3DFMT_A16B16G16R16F: return R16G16B16A16_Float;
        case D3DFMT_R32F: return R32_Float;
        case D3DFMT_G32R32F: return R32G32_Float;
        case D3DFMT_A32B32G32R32F: return R32G32B32A32_Float;
        case D3DFMT_CxV8U8: color_transform = ColorTransform::eOrthographicNormal; return R8G8_SNorm;
        case D3DFMT_A2B10G10R10_XR_BIAS:
            header.pixel_format.bit_count = 32;
            header.pixel_format.masks[0]  = 0x000003FF;
            header.pixel_format.masks[1]  = 0x000FFC00;
            header.pixel_format.masks[2]  = 0x3FF00000;
            header.pixel_format.masks[3]  = 0xC0000000;
            bitmasked                     = true;
            bitmask_has_rgb               = true;
            bitmask_has_alpha             = true;
            return R10G10B10_XR_BIAS_A2_UNorm;
        default: break;
        }
    }

    return header_DXT10.format;
}

void DDSFile::calc_channel_info(Result &res)
{
    auto fmt = format();

    if (!bitmasked && fmt != 0)
    {
        switch (fmt)
        {
        case R32G32B32A32_Typeless:
        case R32G32B32A32_Float:
        case R32G32B32A32_UInt:
        case R32G32B32A32_SInt: bpp = 128; break;

        case R32G32B32_Typeless:
        case R32G32B32_Float:
        case R32G32B32_UInt:
        case R32G32B32_SInt: bpp = 96; break;

        case R16G16B16A16_Typeless:
        case R16G16B16A16_Float:
        case R16G16B16A16_UNorm:
        case R16G16B16A16_UInt:
        case R16G16B16A16_SNorm:
        case R16G16B16A16_SInt:
        case R32G32_Typeless:
        case R32G32_Float:
        case R32G32_UInt:
        case R32G32_SInt:
        case R32G8X24_Typeless:
        case D32_Float_S8X24_UInt:
        case R32_Float_X8X24_Typeless:
        case X32_Typeless_G8X24_UInt:
        case Y416:
        case Y210:
        case Y216: bpp = 64; break;

        case R10G10B10A2_Typeless:
        case R10G10B10A2_UNorm:
        case R10G10B10A2_UInt:
        case R11G11B10_Float:
        case R8G8B8A8_Typeless:
        case R8G8B8A8_UNorm:
        case R8G8B8A8_UNorm_SRGB:
        case R8G8B8A8_UInt:
        case R8G8B8A8_SNorm:
        case R8G8B8A8_SInt:
        case R16G16_Typeless:
        case R16G16_Float:
        case R16G16_UNorm:
        case R16G16_UInt:
        case R16G16_SNorm:
        case R16G16_SInt:
        case R32_Typeless:
        case D32_Float:
        case R32_Float:
        case R32_UInt:
        case R32_SInt:
        case R24G8_Typeless:
        case D24_UNorm_S8_UInt:
        case R24_UNorm_X8_Typeless:
        case X24_Typeless_G8_UInt:
        case R9G9B9E5_SHAREDEXP:
        case R8G8_B8G8_UNorm:
        case G8R8_G8B8_UNorm:
        case B8G8R8A8_UNorm:
        case R10G10B10_XR_BIAS_A2_UNorm:
        case B8G8R8A8_Typeless:
        case B8G8R8A8_UNorm_SRGB:
        case B8G8R8X8_Typeless:
        case B8G8R8X8_UNorm:
        case B8G8R8X8_UNorm_SRGB:
        case AYUV:
        case Y410:
        case YUY2: bpp = 32; break;

        case P010:
        case P016: bpp = 24; break;

        case R8G8_Typeless:
        case R8G8_UNorm:
        case R8G8_UInt:
        case R8G8_SNorm:
        case R8G8_SInt:
        case R16_Typeless:
        case R16_Float:
        case D16_UNorm:
        case R16_UNorm:
        case R16_UInt:
        case R16_SNorm:
        case R16_SInt:
        case B5G6R5_UNorm:
        case B5G5R5A1_UNorm:
        case B4G4R4A4_UNorm:
        case A4B4G4R4_UNorm:
        case A8P8: bpp = 16; break;

        case NV12:
        case YUV420_OPAQUE:
        case NV11: bpp = 12; break;

        case R8_Typeless:
        case R8_UNorm:
        case R8_UInt:
        case R8_SNorm:
        case R8_SInt:
        case A8_UNorm:
        case AI44:
        case IA44:
        case P8: bpp = 8; break;

        case BC2_Typeless:
        case BC2_UNorm:
        case BC2_UNorm_SRGB:
        case BC3_Typeless:
        case BC3_UNorm:
        case BC3_UNorm_SRGB:
        case BC5_Typeless:
        case BC5_UNorm:
        case BC5_SNorm:
        case BC6H_Typeless:
        case BC6H_UF16:
        case BC6H_SF16:
        case BC7_Typeless:
        case BC7_UNorm:
        case BC7_UNorm_SRGB:
            // actually 16 bytes per block (= 16 bytes per block * 8 bits per byte / 16 pixels per block)
            bpp = 8;
            break;

        case BC1_Typeless:
        case BC1_UNorm:
        case BC1_UNorm_SRGB:
        case BC4_Typeless:
        case BC4_UNorm:
        case BC4_SNorm:
            // actually 8 bytes per block (= 8 bytes per block * 8 bits per byte / 16 pixels per block)
            bpp = 4;
            break;

        case R1_UNorm: bpp = 1; break;

        case ASTC_4X4_Typeless:
        case ASTC_4X4_UNorm:
        case ASTC_4X4_UNorm_SRGB:
        case ASTC_5X4_Typeless:
        case ASTC_5X4_UNorm:
        case ASTC_5X4_UNorm_SRGB:
        case ASTC_5X5_Typeless:
        case ASTC_5X5_UNorm:
        case ASTC_5X5_UNorm_SRGB:
        case ASTC_6X5_Typeless:
        case ASTC_6X5_UNorm:
        case ASTC_6X5_UNorm_SRGB:
        case ASTC_6X6_Typeless:
        case ASTC_6X6_UNorm:
        case ASTC_6X6_UNorm_SRGB:
        case ASTC_8X5_Typeless:
        case ASTC_8X5_UNorm:
        case ASTC_8X5_UNorm_SRGB:
        case ASTC_8X6_Typeless:
        case ASTC_8X6_UNorm:
        case ASTC_8X6_UNorm_SRGB:
        case ASTC_8X8_Typeless:
        case ASTC_8X8_UNorm:
        case ASTC_8X8_UNorm_SRGB:
        case ASTC_10X5_Typeless:
        case ASTC_10X5_UNorm:
        case ASTC_10X5_UNorm_SRGB:
        case ASTC_10X6_Typeless:
        case ASTC_10X6_UNorm:
        case ASTC_10X6_UNorm_SRGB:
        case ASTC_10X8_Typeless:
        case ASTC_10X8_UNorm:
        case ASTC_10X8_UNorm_SRGB:
        case ASTC_10X10_Typeless:
        case ASTC_10X10_UNorm:
        case ASTC_10X10_UNorm_SRGB:
        case ASTC_12X10_Typeless:
        case ASTC_12X10_UNorm:
        case ASTC_12X10_UNorm_SRGB:
        case ASTC_12X12_Typeless:
        case ASTC_12X12_UNorm:
        case ASTC_12X12_UNorm_SRGB:
            bpp = 128; // this is bits per block, not per pixel
            break;

        // we don't support these at all
        case P208:
        case V208:
        case V408:
        default:
        {
            res.add_message(Result::Warning, std::string("Unsupported format in bits_per_pixel: ") + format_name(fmt) +
                                                 " (" + std::to_string((uint32_t)fmt) + ")");
            bpp = 0;
        }
        }
    }
    else if (header.pixel_format.bit_count != 0)
    {
        if (header.pixel_format.bit_count > std::numeric_limits<size_t>::max() - 7)
        {
            res.add_message(Result::Warning, "DDS file has a pixel format with a bit count that is too large: " +
                                                 std::to_string(header.pixel_format.bit_count));
            bpp = 0;
        }
        bpp = header.pixel_format.bit_count;
    }
    else
    {
        // Since this branch is uncompressed, pitch_or_linear_size is
        // the number of bytes per scanline in the base mip.
        // Try to get the number of bits per pixel, assuming images are
        // densely packed.
        if ((header.pitch_or_linear_size % header.width) != 0)
        {
            res.add_message(Result::Warning,
                            "This DDS file is probably not valid: it didn't seem to contain DXGI format information, "
                            "and its bit_count was 0. In this situation, pitch_or_linear_size should be the number of "
                            "bits in each scanline of mip 0 - but it wasn't evenly divisible by mip 0's width.");
            bpp = 0;
        }
        bpp = header.pitch_or_linear_size / header.width;
    }

    bool is_normal = (header.pixel_format.flags & uint32_t(PixelFormatFlagBits::Normal));

    // determine the number of channels we have
    if (fmt != Format_Unknown)
    {
        switch (fmt)
        {
        // 4-channel formats
        case BC1_UNorm:
        case BC1_UNorm_SRGB:
        case BC2_UNorm:
        case BC2_UNorm_SRGB:
        case BC7_UNorm:
        case BC7_UNorm_SRGB:
        case R32G32B32A32_Float:
        case R16G16B16A16_Float:
        case R32G32B32A32_UInt:
        case R16G16B16A16_UInt:
        case R8G8B8A8_UInt:
        case R32G32B32A32_SInt:
        case R16G16B16A16_SInt:
        case R8G8B8A8_SInt:
        case R16G16B16A16_SNorm:
        case R8G8B8A8_SNorm:
        case B5G5R5A1_UNorm:
        case R16G16B16A16_UNorm:
        case R8G8B8A8_UNorm:
        case R8G8B8A8_UNorm_SRGB:
        case B8G8R8A8_UNorm:
        case B8G8R8A8_UNorm_SRGB:
        case R10G10B10A2_Typeless:
        case R10G10B10A2_UNorm:
        case R10G10B10A2_UInt:
        case B4G4R4A4_UNorm:
        case A4B4G4R4_UNorm:
        case R10G10B10_XR_BIAS_A2_UNorm:
        case ASTC_4X4_Typeless:
        case ASTC_4X4_UNorm:
        case ASTC_4X4_UNorm_SRGB:
        case ASTC_5X4_Typeless:
        case ASTC_5X4_UNorm:
        case ASTC_5X4_UNorm_SRGB:
        case ASTC_5X5_Typeless:
        case ASTC_5X5_UNorm:
        case ASTC_5X5_UNorm_SRGB:
        case ASTC_6X5_Typeless:
        case ASTC_6X5_UNorm:
        case ASTC_6X5_UNorm_SRGB:
        case ASTC_6X6_Typeless:
        case ASTC_6X6_UNorm:
        case ASTC_6X6_UNorm_SRGB:
        case ASTC_8X5_Typeless:
        case ASTC_8X5_UNorm:
        case ASTC_8X5_UNorm_SRGB:
        case ASTC_8X6_Typeless:
        case ASTC_8X6_UNorm:
        case ASTC_8X6_UNorm_SRGB:
        case ASTC_8X8_Typeless:
        case ASTC_8X8_UNorm:
        case ASTC_8X8_UNorm_SRGB:
        case ASTC_10X5_Typeless:
        case ASTC_10X5_UNorm:
        case ASTC_10X5_UNorm_SRGB:
        case ASTC_10X6_Typeless:
        case ASTC_10X6_UNorm:
        case ASTC_10X6_UNorm_SRGB:
        case ASTC_10X8_Typeless:
        case ASTC_10X8_UNorm:
        case ASTC_10X8_UNorm_SRGB:
        case ASTC_10X10_Typeless:
        case ASTC_10X10_UNorm:
        case ASTC_10X10_UNorm_SRGB:
        case ASTC_12X10_Typeless:
        case ASTC_12X10_UNorm:
        case ASTC_12X10_UNorm_SRGB:
        case ASTC_12X12_Typeless:
        case ASTC_12X12_UNorm:
        case ASTC_12X12_UNorm_SRGB: num_channels = 4; break;

        case BC3_UNorm:
        case BC3_UNorm_SRGB: num_channels = is_normal || color_transform == ColorTransform::eAGBR ? 3 : 4; break;

        // 3-channel formats
        case R32G32B32_Float:
        case R32G32B32_UInt:
        case R32G32B32_SInt:
        case BC6H_UF16:
        case BC6H_SF16:
        case R11G11B10_Float:
        case Format_Unknown:
        case B5G6R5_UNorm:
        case B8G8R8X8_Typeless:
        case B8G8R8X8_UNorm:
        case B8G8R8X8_UNorm_SRGB:
        case R9G9B9E5_SHAREDEXP: num_channels = 3; break;

        // 2-channel formats
        case R32G32_Float:
        case R32G32_UInt:
        case R32G32_SInt:
        case R16G16_Float:
        case R16G16_UInt:
        case R8G8_UInt:
        case R16G16_SInt:
        case R8G8_SInt:
        case R16G16_SNorm:
        case R8G8_SNorm:
        case R16G16_UNorm:
        case R8G8_UNorm: num_channels = 2; break;

        case BC5_UNorm:
        case BC5_SNorm:
            num_channels = is_normal ? 3 : 2;
            num_channels = is_normal ? 3 : 2;
            break;

        // 1-channel formats
        case R32_Float:
        case D32_Float:
        case R32_UInt:
        case R16_Float:
        case R16_UInt:
        case R8_UInt:
        case R32_SInt:
        case R16_SInt:
        case R8_SInt:
        case R16_SNorm:
        case R8_SNorm:
        case R16_UNorm:
        case D16_UNorm:
        case A8_UNorm:
        case R8_UNorm:
        case BC4_UNorm:
        case BC4_SNorm:
        case R1_UNorm: num_channels = 1; break;

        default: num_channels = 0; break;
        }
    }
    else
    {
        num_channels = 0;
        for (int i = 0; i < 4; ++i)
            if (header.pixel_format.masks[i] != 0)
                num_channels++;
    }

    for (int i = 0; i < 4; ++i) calc_shifts(header.pixel_format.masks[i], bit_counts[i], right_shifts[i]);
}

Result DDSFile::load(const char *filepath)
{
    std::ifstream ifs(filepath, std::ios_base::binary);
    if (!ifs.is_open())
    {
        return Result{Result::Error, "Cannot open file"};
    }

    return load(ifs);
}

Result DDSFile::load(std::istream &input)
{
    dds.clear();

    input.seekg(0, std::ios_base::beg);
    auto begPos = input.tellg();
    input.seekg(0, std::ios_base::end);
    auto endPos = input.tellg();
    input.seekg(0, std::ios_base::beg);

    auto fileSize = endPos - begPos;
    if (fileSize == 0)
        return Result{Result::Error, "Cannot read file: file is empty"};

    std::vector<uint8_t> _dds(fileSize);

    input.read(reinterpret_cast<char *>(_dds.data()), fileSize);

    if (input.bad())
        return Result{Result::Error, "Cannot read file: I/O error"};

    return load(std::move(_dds));
}

Result DDSFile::load(const uint8_t *data, size_t size)
{
    std::vector<uint8_t> _dds(data, data + size);
    return load(std::move(_dds));
}

Result DDSFile::load(std::vector<uint8_t> &&_dds)
{
    dds.clear();

    if (_dds.size() < 4)
        return Result{Result::Error, "File too small for magic number"};

    for (int i = 0; i < 4; i++)
        if (_dds[i] != Magic[i])
            return Result{Result::Error, "Magic number not found"};

    if ((sizeof(uint32_t) + sizeof(Header)) >= _dds.size())
        return Result{Result::Error, "File too small for DDS header"};

    std::memcpy(&header, _dds.data() + sizeof(uint32_t), sizeof(Header));

    dds = std::move(_dds);

    return verify_header();
}

Result DDSFile::verify_header()
{
    Result res{Result::Success};
    if (m_header_verified)
        return res;

    if (header.size != sizeof(Header))
        res.add_message(Result::Warning, "DDS header size is incorrect. "
                                         "Expected " +
                                             std::to_string(sizeof(Header)) + " but got " +
                                             std::to_string(header.size) + ". Attempting to continue...");

    if (header.pixel_format.size != sizeof(PixelFormat))
        res.add_message(Result::Warning, "Pixel format size is incorrect. Expected " +
                                             std::to_string(sizeof(PixelFormat)) + " but got " +
                                             std::to_string(header.pixel_format.size) + ". Attempting to continue...");

    // Validate number of mips in the image.
    header.mipmap_count = std::max(1U, header.mipmap_count);
    if (header.mipmap_count >= 32)
    {
        res.add_message(Result::Warning, "The number of mips in the DDS file must be less than 32. "
                                         "Otherwise, the base mip would need to have a dimension of "
                                         "2^32 or larger, which isn't possible. Setting to 1 and trying to "
                                         "continue.");
        header.mipmap_count = 1;
    }

    is_cubemap = false;

    const bool hasFourCC = header.pixel_format.flags & uint32_t(PixelFormatFlagBits::FourCC);

    // Handle DPPF_ALPHAPREMULT, in case it's there for compatibility.
    if ((header.pixel_format.flags & uint32_t(PixelFormatFlagBits::AlphaPreMult)) != 0)
        alpha_mode = ALPHA_MODE_PREMULTIPLIED;

    has_DXT10_header = false;
    if (hasFourCC && header.pixel_format.fourCC == FOURCC_DX10)
    {
        res.add_message(Result::Info, "DDS: DXT10 header found.");

        // check header exists
        if ((sizeof(uint32_t) + sizeof(Header) + sizeof(HeaderDXT10)) >= dds.size())
        {
            res.add_message(Result::Error, "DDS: DXT10 header found, but file is too small for it. "
                                           "Expected at least " +
                                               std::to_string(sizeof(uint32_t) + sizeof(Header) + sizeof(HeaderDXT10)) +
                                               " bytes, but got only " + std::to_string(dds.size()));
            return res;
        }

        has_DXT10_header = true;

        // Copy the DXT10 header from dds
        std::memcpy(&header_DXT10, dds.data() + sizeof(uint32_t) + sizeof(Header), sizeof(HeaderDXT10));

        if (header_DXT10.array_size == 0)
        {
            res.add_message(Result::Warning, "DDS: DXT10 header array_size is 0. Assuming this should be "
                                             "1 and trying to continue.");
            header_DXT10.array_size = 1;
        }

        switch (header_DXT10.resource_dimension)
        {
        case Texture1D:
            if ((header.flags & uint32_t(HeaderFlagBits::Height) && (header.height != 1)))
                res.add_message(Result::Warning, "DDS: Texture1D with height != 1 is not supported. "
                                                 "Will assume height == 1.");

            header.height = header.depth = 1;
            break;
        case Texture2D:
            if (header_DXT10.misc_flag & uint32_t(DXT10MiscFlagBits::TextureCube))
            {
                header_DXT10.array_size *= 6;
                is_cubemap = true;
            }
            header.depth = 1;
            break;
        case Texture3D:
            if (!(header.flags & uint32_t(HeaderFlagBits::Depth)))
                res.add_message(Result::Warning, "DDS: Texture3D without depth doesn't make sense. "
                                                 "Assuming depth == 1 and trying to continue.");

            if (header_DXT10.array_size > 1)
            {
                res.add_message(Result::Warning, "DDS: Texture3D with array_size > 1 is not "
                                                 "supported. Will assume array_size == 1.");
                header_DXT10.array_size = 1;
            }
            break;
        default:
            res.add_message(Result::Warning, "DDS: Unknown resource dimension " +
                                                 std::to_string(header_DXT10.resource_dimension) +
                                                 ". Attempting to continue.");
        }

        // Look at lower 3 bits of miscFlags2 to determine alpha mode
        alpha_mode = header_DXT10.misc_flag2 & 0x7;
    }
    else
    {
        // No DX10 header.
        res.add_message(Result::Info, "DDS: No DXT10 header found. Assuming this is a DX9 file.");

        if (header.flags & uint32_t(HeaderFlagBits::Depth))
            header_DXT10.resource_dimension = Texture3D;
        else
        {
            auto caps2 = header.caps2 & uint32_t(CubemapAllFaces);
            if (caps2)
            {
                if (caps2 != uint32_t(CubemapAllFaces))
                    res.add_message(Result::Warning, "DDS: Cubemap with non-cubemap caps2 bits set. "
                                                     "Assuming this is a cubemap and trying to continue.");
                header_DXT10.array_size = 6;
                is_cubemap              = true;
            }

            header.depth                    = 1;
            header_DXT10.resource_dimension = Texture2D;
        }
    }

    header_DXT10.format = deduce_format_from_fourCC(res);

    // if we get here and the format is still unknown, we need to set
    // bitmasks.
    // Either we didn't have the DXT10 header and no recognized fourCC code, or
    // we had a DXT10 header with no format written. In either case we must
    // resort to bitmasks
    if (header_DXT10.format == Format_Unknown && !bitmasked)
        deduce_bitmasks_from_pixel_format();

    calc_channel_info(res);

    // Detect the swizzle code, which is stored in bit_count. There's no
    // standard for this, so we just check for what old versions of NVTT
    // could output.
    switch (header.pixel_format.bit_count)
    {
    case FOURCC_A2XY: color_transform = ColorTransform::eSwapRG; break;
    case FOURCC_A2D5: color_transform = ColorTransform::eAGBR; break;
    }
    // Read additional color transform info from pf.flags whether we're in
    // DX9 or DX10 mode.
    if ((header.pixel_format.flags & uint32_t(PixelFormatFlagBits::YUV)))
        color_transform = ColorTransform::eYUV;
    if ((header.pixel_format.flags & uint32_t(PixelFormatFlagBits::Luminance)))
        color_transform = ColorTransform::eLuminance;

    if (!bitmasked)
        switch (header_DXT10.format)
        {
        case B5G5R5A1_UNorm:
        case B8G8R8A8_UNorm:
        case B8G8R8A8_Typeless:
        case B8G8R8A8_UNorm_SRGB:
        case B8G8R8X8_UNorm:
        case B8G8R8X8_Typeless:
        case B8G8R8X8_UNorm_SRGB: color_transform = ColorTransform::eSwapRB;
        default: break;
        }

    switch (header_DXT10.format)
    {
    case AI44:
    case IA44:
    case P8:
    case A8P8:
        res.add_message(Result::Warning, "DDS: AI44, IA44, P8 and A8P8 formats are not supported. "
                                         "Assuming they are R8G8B8A8_UNorm and trying to continue.");

        header_DXT10.format = R8G8B8A8_UNorm;
        bpp                 = 32;
        num_channels        = 4;
        bitmasked           = true;
        bitmask_has_rgb     = true;
        bitmask_has_alpha   = true;
        right_shifts[0]     = 0;
        right_shifts[1]     = 8;
        right_shifts[2]     = 16;
        right_shifts[3]     = 24;
        break;
    default:
        if (bpp == 0)
        {
            res.add_message(Result::Error, std::string("DDS: Couldn't deduce bits per pixel for format ") +
                                               format_name(header_DXT10.format) +
                                               ". This is a fatal error, cannot continue.");
            return res;
        }
    }

    m_header_verified = true;
    return res;
}

size_t DDSFile::image_data_size(uint32_t w, uint32_t h, uint32_t d, Result &res) const
{
    auto fmt = format();

    uint32_t num_pixels = w * h;

    if (!bitmasked && fmt != 0)
    {
        // Computes the size of a `w` x `h` x `d` image encoded
        // using ASTC blocks of size `block_width` x `block_height` x `1`.
        auto astc_size = [w, h, d](uint32_t block_width, uint32_t block_height)
        {
            return (((w + block_width - 1) / block_width) *   // # of ASTC blocks along the x axis
                    ((h + block_height - 1) / block_height) * // # of ASTC blocks along the y axis
                    ((d + 1 - 1) / 1) *                       // # of ASTC blocks along the z axis
                    16);                                      // Each ASTC block size is 128 bits = 16 bytes
        };

        uint32_t num_bytes     = 0;
        uint32_t num_bc_blocks = ((w + 3) / 4) * ((h + 3) / 4);
        switch (fmt)
        {
        default:
            // the easy base case
            num_bytes = (bpp * num_pixels + 7) / 8;
            break;

        // packed/compressed formats
        case BC1_Typeless:
        case BC1_UNorm:
        case BC1_UNorm_SRGB:
        case BC4_Typeless:
        case BC4_UNorm:
        case BC4_SNorm:
            // 8 bytes per block
            num_bytes = num_bc_blocks * 8;
            break;

        case BC2_Typeless:
        case BC2_UNorm:
        case BC2_UNorm_SRGB:
        case BC3_Typeless:
        case BC3_UNorm:
        case BC3_UNorm_SRGB:
        case BC5_Typeless:
        case BC5_UNorm:
        case BC5_SNorm:
        case BC6H_Typeless:
        case BC6H_UF16:
        case BC6H_SF16:
        case BC7_Typeless:
        case BC7_UNorm:
        case BC7_UNorm_SRGB:
            // 16 bytes per block
            num_bytes = num_bc_blocks * 16;
            break;
        case ASTC_4X4_Typeless:
        case ASTC_4X4_UNorm:
        case ASTC_4X4_UNorm_SRGB: num_bytes = astc_size(4, 4); break;
        case ASTC_5X4_Typeless:
        case ASTC_5X4_UNorm:
        case ASTC_5X4_UNorm_SRGB: num_bytes = astc_size(5, 4); break;
        case ASTC_5X5_Typeless:
        case ASTC_5X5_UNorm:
        case ASTC_5X5_UNorm_SRGB: num_bytes = astc_size(5, 5); break;
        case ASTC_6X5_Typeless:
        case ASTC_6X5_UNorm:
        case ASTC_6X5_UNorm_SRGB: num_bytes = astc_size(6, 5); break;
        case ASTC_6X6_Typeless:
        case ASTC_6X6_UNorm:
        case ASTC_6X6_UNorm_SRGB: num_bytes = astc_size(6, 6); break;
        case ASTC_8X5_Typeless:
        case ASTC_8X5_UNorm:
        case ASTC_8X5_UNorm_SRGB: num_bytes = astc_size(8, 5); break;
        case ASTC_8X6_Typeless:
        case ASTC_8X6_UNorm:
        case ASTC_8X6_UNorm_SRGB: num_bytes = astc_size(8, 6); break;
        case ASTC_8X8_Typeless:
        case ASTC_8X8_UNorm:
        case ASTC_8X8_UNorm_SRGB: num_bytes = astc_size(8, 8); break;
        case ASTC_10X5_Typeless:
        case ASTC_10X5_UNorm:
        case ASTC_10X5_UNorm_SRGB: num_bytes = astc_size(10, 5); break;
        case ASTC_10X6_Typeless:
        case ASTC_10X6_UNorm:
        case ASTC_10X6_UNorm_SRGB: num_bytes = astc_size(10, 6); break;
        case ASTC_10X8_Typeless:
        case ASTC_10X8_UNorm:
        case ASTC_10X8_UNorm_SRGB: num_bytes = astc_size(10, 8); break;
        case ASTC_10X10_Typeless:
        case ASTC_10X10_UNorm:
        case ASTC_10X10_UNorm_SRGB: num_bytes = astc_size(10, 10); break;
        case ASTC_12X10_Typeless:
        case ASTC_12X10_UNorm:
        case ASTC_12X10_UNorm_SRGB: num_bytes = astc_size(12, 10); break;
        case ASTC_12X12_Typeless:
        case ASTC_12X12_UNorm:
        case ASTC_12X12_UNorm_SRGB: num_bytes = astc_size(12, 12); break;

        case R8G8_B8G8_UNorm:
        case G8R8_G8B8_UNorm:
        case YUY2: num_bytes = (((w + 1) >> 1) * 4) * h; break;

        case Y210:
        case Y216: num_bytes = (((w + 1) >> 1) * 8) * h; break;

        case NV11: num_bytes = (((w + 3) >> 2) * 4) + h * 2; break;

        case NV12:
        case YUV420_OPAQUE: num_bytes = ((((w + 1) >> 1) * 2) * h) + (((((w + 1) >> 1) * 2) * h + 1) >> 1); break;

        case P010:
        case P016: num_bytes = ((((w + 1) >> 1) * 4) * h) + (((((w + 1) >> 1) * 4) * h + 1) >> 1); break;
        }

        if (!is_compressed(fmt))
        {
            auto Bpp = num_bytes / num_pixels;
            if (header.pixel_format.bit_count != 0 && header.pixel_format.bit_count <= 128 &&
                Bpp != header.pixel_format.bit_count / 8)
            {
                res.add_message(Result::Warning, "Image data size mismatch: bit_count field says " +
                                                     std::to_string(header.pixel_format.bit_count) +
                                                     " bits, but format calculation suggests " + std::to_string(Bpp) +
                                                     " bits: " + std::to_string(num_bytes) + "/" +
                                                     std::to_string(num_pixels) +
                                                     " * 8. Using the latter and trying to continue.");
                num_bytes = header.pixel_format.bit_count / 8;
            }
        }

        return num_bytes * d;
    }
    else if (header.pixel_format.bit_count != 0)
    {
        size_t file_size_bits = header.pixel_format.bit_count * w * h * d;
        if (file_size_bits > std::numeric_limits<size_t>::max() - 7)
        {
            res.add_message(Result::Warning, "DDS: File size of " + std::to_string(file_size_bits) +
                                                 " bits is too large to calculate image data size.");
            return 0;
        }
        return (file_size_bits + 7) / 8;
    }
    else
    {
        // Since this branch is uncompressed, pitch_or_linear_size is
        // the number of bytes per scanline in the base mip.
        // Try to get the number of bits per pixel, assuming images are
        // densely packed.
        if ((header.pitch_or_linear_size % header.width) != 0)
        {
            res.add_message(Result::Warning, "This file is probably not valid: it didn't seem to "
                                             "contain DXGI format information, and its "
                                             "bit_count was 0. In this situation, "
                                             "pitch_or_linear_size should be the number of bits "
                                             "in each scanline of mip 0 - but it wasn't evenly "
                                             "divisible by mip 0's width.");
            return 0;
        }
        auto           bitmasked_bits_per_pixel = header.pitch_or_linear_size / header.width;
        const uint32_t pitch                    = bitmasked_bits_per_pixel * w;
        return pitch * h * d;
    }
}

Result DDSFile::populate_image_data()
{
    auto res = verify_header();
    if (res.type != Result::Success)
        return res;

    ptrdiff_t offset = sizeof(uint32_t) + sizeof(Header) + (has_DXT10_header ? sizeof(HeaderDXT10) : 0);

    image_data.resize(0);
    image_data.reserve(header_DXT10.array_size * header.mipmap_count);

    uint8_t *src_bytes = dds.data() + offset;
    uint8_t *end       = dds.data() + dds.size();
    for (uint32_t j = 0; j < header_DXT10.array_size; j++)
    {
        uint32_t w = header.width;
        uint32_t h = header.height;
        uint32_t d = header.depth;
        for (uint32_t i = 0; i < header.mipmap_count; i++)
        {
            auto data_size = image_data_size(w, h, d, res);
            if (data_size == 0)
            {
                res.add_message(Result::Warning, "DDS: Image data size for image " + std::to_string(j + 1) + " (of " +
                                                     std::to_string(header_DXT10.array_size) + ") and mip " +
                                                     std::to_string(i + 1) + " (of " +
                                                     std::to_string(header.mipmap_count) +
                                                     ") is 0. Will try to continue with the image data we "
                                                     "already read.");
                header.mipmap_count     = i;
                header_DXT10.array_size = j + (i > 0 ? 1 : 0);

                break;
            }

            if (data_size > static_cast<size_t>(end - src_bytes))
            {
                res.add_message(Result::Warning,
                                "DDS: Image data for image " + std::to_string(j + 1) + " (of " +
                                    std::to_string(header_DXT10.array_size) + ") and mip " + std::to_string(i + 1) +
                                    " (of " + std::to_string(header.mipmap_count) + ") is too large (" +
                                    std::to_string(data_size) + " bytes) and goes past the end of the file (" +
                                    std::to_string(end - src_bytes) +
                                    " bytes to go). "
                                    "Will try to continue with the data we have.");
                header.mipmap_count     = i;
                header_DXT10.array_size = j + (i > 0 ? 1 : 0);
                break;
                // src_bytes = end - data_size;
                // data_size = end - src_bytes;
            }

            // Also, make sure this isn't impossibly large.
            if (((data_size / w) / h) / d > 16)
            {
                res.add_message(Result::Warning,
                                "DDS: Image data for image " + std::to_string(j + 1) + " (of " +
                                    std::to_string(header_DXT10.array_size) + ") and mip " + std::to_string(i + 1) +
                                    " (of " + std::to_string(header.mipmap_count) + ") is larger than a mip of size " +
                                    std::to_string(w) + " x " + std::to_string(h) + " x " + std::to_string(d) +
                                    " would be in the largest DDS format, RGBA32F. "
                                    "This is probably not valid data. Will try to continue "
                                    "with the data we have.");
                header_DXT10.array_size = j + (i > 0 ? 1 : 0);
                break;
            }

            image_data.emplace_back(ImageData{w, h, d, {src_bytes, data_size}});
            src_bytes += data_size;

            w = std::max<uint32_t>(1, w / 2);
            h = std::max<uint32_t>(1, h / 2);
            d = std::max<uint32_t>(1, d / 2);
        }
    }

    if (image_data.empty())
        res.add_message(Result::Error, "DDS: Could not read any image data from the file.");

    return res;
}

} // namespace smalldds

#endif // !SMALLDDS_IMPLEMENTATION
