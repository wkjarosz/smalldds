//
// Copyright(c) 2019 benikabocha.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
// Modified by Wojciech Jarosz, 2025
//
// - Store and provide access to the entire raw header
// - Removed m_ prefix on some struct members that are now publicly accessible
// - Load(...) now only reads the header and copies the entire file data; it
//   does not populate the image data structures
// - Added PopulateImageDatas() which must be called after Load() if more than
//   the header is needed

#ifndef TINYDDSLOADER_H_
#define TINYDDSLOADER_H_

#include <iostream>
#include <vector>

namespace tinyddsloader {

enum Result {
    Success,
    ErrorFileOpen,
    ErrorRead,
    ErrorMagicWord,
    ErrorSize,
    ErrorVerify,
    ErrorNotSupported,
    ErrorInvalidData,
};

#define MakeFourCC(a, b, c, d) (a | b << 8 | c << 16 | d << 24)

class DDSFile {
public:
    static const char Magic[4];

    static constexpr uint32_t DXT1 = MakeFourCC('D', 'X', 'T', '1');
    static constexpr uint32_t DXT2 = MakeFourCC('D', 'X', 'T', '2');
    static constexpr uint32_t DXT3 = MakeFourCC('D', 'X', 'T', '3');
    static constexpr uint32_t DXT4 = MakeFourCC('D', 'X', 'T', '4');
    static constexpr uint32_t DXT5 = MakeFourCC('D', 'X', 'T', '5');
    static constexpr uint32_t RXGB = MakeFourCC('R', 'X', 'G', 'B');
    static constexpr uint32_t ATI1 = MakeFourCC('A', 'T', 'I', '1');
    static constexpr uint32_t ATI2 = MakeFourCC('A', 'T', 'I', '2');
    static constexpr uint32_t BC4U = MakeFourCC('B', 'C', '4', 'U');
    static constexpr uint32_t BC5U = MakeFourCC('B', 'C', '5', 'U');
    static constexpr uint32_t DX10 = MakeFourCC('D', 'X', '1', '0');

    enum PixelFormatFlagBits : uint32_t {
        AlphaPixels = 0x00000001,  ///< image has alpha channel
        AlphaOnly = 0x00000002,    ///< image has only the alpha channel
        FourCC = 0x00000004,       ///< image is compressed
        RGB = 0x00000040,          ///< image has RGB data
        Luminance = 0x00020000,    ///< image has luminance data
        Palette8 = 0x00000020,
        BumpDUDV = 0x00080000,
        Normal = 0x80000000u,  ///< image is a tangent space normal map
    };

    enum DXGIFormat : uint32_t {
        Format_Unknown = 0,
        R32G32B32A32_Typeless = 1,
        R32G32B32A32_Float = 2,
        R32G32B32A32_UInt = 3,
        R32G32B32A32_SInt = 4,
        R32G32B32_Typeless = 5,
        R32G32B32_Float = 6,
        R32G32B32_UInt = 7,
        R32G32B32_SInt = 8,
        R16G16B16A16_Typeless = 9,
        R16G16B16A16_Float = 10,
        R16G16B16A16_UNorm = 11,
        R16G16B16A16_UInt = 12,
        R16G16B16A16_SNorm = 13,
        R16G16B16A16_SInt = 14,
        R32G32_Typeless = 15,
        R32G32_Float = 16,
        R32G32_UInt = 17,
        R32G32_SInt = 18,
        R32G8X24_Typeless = 19,
        D32_Float_S8X24_UInt = 20,
        R32_Float_X8X24_Typeless = 21,
        X32_Typeless_G8X24_UInt = 22,
        R10G10B10A2_Typeless = 23,
        R10G10B10A2_UNorm = 24,
        R10G10B10A2_UInt = 25,
        R11G11B10_Float = 26,
        R8G8B8A8_Typeless = 27,
        R8G8B8A8_UNorm = 28,
        R8G8B8A8_UNorm_SRGB = 29,
        R8G8B8A8_UInt = 30,
        R8G8B8A8_SNorm = 31,
        R8G8B8A8_SInt = 32,
        R16G16_Typeless = 33,
        R16G16_Float = 34,
        R16G16_UNorm = 35,
        R16G16_UInt = 36,
        R16G16_SNorm = 37,
        R16G16_SInt = 38,
        R32_Typeless = 39,
        D32_Float = 40,
        R32_Float = 41,
        R32_UInt = 42,
        R32_SInt = 43,
        R24G8_Typeless = 44,
        D24_UNorm_S8_UInt = 45,
        R24_UNorm_X8_Typeless = 46,
        X24_Typeless_G8_UInt = 47,
        R8G8_Typeless = 48,
        R8G8_UNorm = 49,
        R8G8_UInt = 50,
        R8G8_SNorm = 51,
        R8G8_SInt = 52,
        R16_Typeless = 53,
        R16_Float = 54,
        D16_UNorm = 55,
        R16_UNorm = 56,
        R16_UInt = 57,
        R16_SNorm = 58,
        R16_SInt = 59,
        R8_Typeless = 60,
        R8_UNorm = 61,
        R8_UInt = 62,
        R8_SNorm = 63,
        R8_SInt = 64,
        A8_UNorm = 65,
        R1_UNorm = 66,
        R9G9B9E5_SHAREDEXP = 67,
        R8G8_B8G8_UNorm = 68,
        G8R8_G8B8_UNorm = 69,
        BC1_Typeless = 70,
        BC1_UNorm = 71,
        BC1_UNorm_SRGB = 72,
        BC2_Typeless = 73,
        BC2_UNorm = 74,
        BC2_UNorm_SRGB = 75,
        BC3_Typeless = 76,
        BC3_UNorm = 77,
        BC3_UNorm_SRGB = 78,
        BC4_Typeless = 79,
        BC4_UNorm = 80,
        BC4_SNorm = 81,
        BC5_Typeless = 82,
        BC5_UNorm = 83,
        BC5_SNorm = 84,
        B5G6R5_UNorm = 85,
        B5G5R5A1_UNorm = 86,
        B8G8R8A8_UNorm = 87,
        B8G8R8X8_UNorm = 88,
        R10G10B10_XR_BIAS_A2_UNorm = 89,
        B8G8R8A8_Typeless = 90,
        B8G8R8A8_UNorm_SRGB = 91,
        B8G8R8X8_Typeless = 92,
        B8G8R8X8_UNorm_SRGB = 93,
        BC6H_Typeless = 94,
        BC6H_UF16 = 95,
        BC6H_SF16 = 96,
        BC7_Typeless = 97,
        BC7_UNorm = 98,
        BC7_UNorm_SRGB = 99,
        AYUV = 100,
        Y410 = 101,
        Y416 = 102,
        NV12 = 103,
        P010 = 104,
        P016 = 105,
        YUV420_OPAQUE = 106,
        YUY2 = 107,
        Y210 = 108,
        Y216 = 109,
        NV11 = 110,
        AI44 = 111,
        IA44 = 112,
        P8 = 113,
        A8P8 = 114,
        B4G4R4A4_UNorm = 115,
        P208 = 130,
        V208 = 131,
        V408 = 132,
    };

    enum class HeaderFlagBits : uint32_t {
        Height = 0x00000002,
        Width = 0x00000004,
        Texture = 0x00001007,
        Mipmap = 0x00020000,
        Volume = 0x00800000,
        Pitch = 0x00000008,
        LinearSize = 0x00080000,
    };

    enum HeaderCaps2FlagBits : uint32_t {
        CubemapPositiveX = 0x00000600,
        CubemapNegativeX = 0x00000a00,
        CubemapPositiveY = 0x00001200,
        CubemapNegativeY = 0x00002200,
        CubemapPositiveZ = 0x00004200,
        CubemapNegativeZ = 0x00008200,
        CubemapAllFaces = CubemapPositiveX | CubemapNegativeX |
                          CubemapPositiveY | CubemapNegativeY |
                          CubemapPositiveZ | CubemapNegativeZ,
        Volume = 0x00200000,
    };

    struct PixelFormat {
        uint32_t size;
        uint32_t flags;
        uint32_t fourCC;
        uint32_t bitCount;
        uint32_t RBitMask;
        uint32_t GBitMask;
        uint32_t BBitMask;
        uint32_t ABitMask;
    };

    struct Header {
        uint32_t size;              ///< structure size, must be 124
        uint32_t flags;             ///< flags to indicate valid fields
        uint32_t height;            ///< image height
        uint32_t width;             ///< image width
        uint32_t pitchOrLinerSize;  ///< bytes per scanline (uncmp.)/total byte
                                    ///< size (cmp.)
        uint32_t depth;             ///< image depth (for 3D textures)
        uint32_t mipMapCount;       ///< number of mipmaps
        uint32_t reserved1[11];
        PixelFormat pixelFormat;
        uint32_t caps;
        uint32_t caps2;
        uint32_t caps3;
        uint32_t caps4;
        uint32_t reserved2;
    };

    enum TextureDimension : uint32_t {
        Dimension_Unknown = 0,
        Texture1D = 2,
        Texture2D = 3,
        Texture3D = 4
    };

    enum class DXT10MiscFlagBits : uint32_t { TextureCube = 0x4 };

    struct HeaderDXT10 {
        DXGIFormat format = Format_Unknown;
        TextureDimension resourceDimension = Dimension_Unknown;
        uint32_t miscFlag = 0;
        uint32_t arraySize = 1;
        uint32_t miscFlag2 = 0;
    };

    struct ImageData {
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_depth;
        void* m_mem;
        uint32_t m_memPitch;
        uint32_t m_memSlicePitch;
    };

    struct BC1Block {
        uint16_t m_color0;
        uint16_t m_color1;
        uint8_t m_row0;
        uint8_t m_row1;
        uint8_t m_row2;
        uint8_t m_row3;
    };

    struct BC2Block {
        uint16_t m_alphaRow0;
        uint16_t m_alphaRow1;
        uint16_t m_alphaRow2;
        uint16_t m_alphaRow3;
        uint16_t m_color0;
        uint16_t m_color1;
        uint8_t m_row0;
        uint8_t m_row1;
        uint8_t m_row2;
        uint8_t m_row3;
    };

    struct BC3Block {
        uint8_t m_alpha0;
        uint8_t m_alpha1;
        uint8_t m_alphaR0;
        uint8_t m_alphaR1;
        uint8_t m_alphaR2;
        uint8_t m_alphaR3;
        uint8_t m_alphaR4;
        uint8_t m_alphaR5;
        uint16_t m_color0;
        uint16_t m_color1;
        uint8_t m_row0;
        uint8_t m_row1;
        uint8_t m_row2;
        uint8_t m_row3;
    };

    struct BC4Block {
        uint8_t m_red0;
        uint8_t m_red1;
        uint8_t m_redR0;
        uint8_t m_redR1;
        uint8_t m_redR2;
        uint8_t m_redR3;
        uint8_t m_redR4;
        uint8_t m_redR5;
    };

    struct BC5Block {
        uint8_t m_red0;
        uint8_t m_red1;
        uint8_t m_redR0;
        uint8_t m_redR1;
        uint8_t m_redR2;
        uint8_t m_redR3;
        uint8_t m_redR4;
        uint8_t m_redR5;
        uint8_t m_green0;
        uint8_t m_green1;
        uint8_t m_greenR0;
        uint8_t m_greenR1;
        uint8_t m_greenR2;
        uint8_t m_greenR3;
        uint8_t m_greenR4;
        uint8_t m_greenR5;
    };

public:
    static bool IsCompressed(DXGIFormat fmt);
    static DXGIFormat GetDXGIFormat(const PixelFormat& pf);
    static uint32_t GetBitsPerPixel(DXGIFormat fmt);

    Result Load(const char* filepath);
    Result Load(std::istream& input);
    Result Load(const uint8_t* data, size_t size);
    Result Load(std::vector<uint8_t>&& dds);
    Result PopulateImageDatas();

    const ImageData* GetImageData(uint32_t mipIdx = 0,
                                  uint32_t arrayIdx = 0) const {
        if (mipIdx < m_header.mipMapCount &&
            arrayIdx < m_headerDXT10.arraySize) {
            return &m_imageDatas[m_header.mipMapCount * arrayIdx + mipIdx];
        }
        return nullptr;
    }

    bool Flip();

    const Header& GetHeader() const { return m_header; }
    const HeaderDXT10& GetHeaderDXT10() const { return m_headerDXT10; }

    // Convenient access to some header fields
    uint32_t GetWidth() const { return m_header.width; }
    uint32_t GetHeight() const { return m_header.height; }
    uint32_t GetDepth() const { return m_header.depth; }
    uint32_t GetMipCount() const { return m_header.mipMapCount; }
    uint32_t GetArraySize() const { return m_headerDXT10.arraySize; }
    DXGIFormat GetFormat() const { return m_headerDXT10.format; }
    bool IsCubemap() const { return m_isCubemap; }
    TextureDimension GetTextureDimension() const {
        return m_headerDXT10.resourceDimension;
    }

private:
    Result VerifyHeader();
    void GetImageInfo(uint32_t w, uint32_t h, DXGIFormat fmt,
                      uint32_t* outNumBytes, uint32_t* outRowBytes,
                      uint32_t* outNumRows);
    bool FlipImage(ImageData& imageData);
    bool FlipCompressedImage(ImageData& imageData);
    void FlipCompressedImageBC1(ImageData& imageData);
    void FlipCompressedImageBC2(ImageData& imageData);
    void FlipCompressedImageBC3(ImageData& imageData);
    void FlipCompressedImageBC4(ImageData& imageData);
    void FlipCompressedImageBC5(ImageData& imageData);

private:
    std::vector<uint8_t> m_dds;
    std::vector<ImageData> m_imageDatas;

    Header m_header;
    bool m_hasDXT10Header = false;
    HeaderDXT10 m_headerDXT10;
    bool m_headerVerified = false;
    bool m_isCubemap;
};

}  // namespace tinyddsloader

#ifdef TINYDDSLOADER_IMPLEMENTATION

#if _WIN32
#undef min
#undef max
#endif  // _Win32

#include <algorithm>
#include <fstream>

namespace tinyddsloader {

const char DDSFile::Magic[4] = {'D', 'D', 'S', ' '};

bool DDSFile::IsCompressed(DXGIFormat fmt) {
    switch (fmt) {
        case BC1_Typeless:
        case BC1_UNorm:
        case BC1_UNorm_SRGB:
        case BC2_Typeless:
        case BC2_UNorm:
        case BC2_UNorm_SRGB:
        case BC3_Typeless:
        case BC3_UNorm:
        case BC3_UNorm_SRGB:
        case BC4_Typeless:
        case BC4_UNorm:
        case BC4_SNorm:
        case BC5_Typeless:
        case BC5_UNorm:
        case BC5_SNorm:
        case BC6H_Typeless:
        case BC6H_UF16:
        case BC6H_SF16:
        case BC7_Typeless:
        case BC7_UNorm:
        case BC7_UNorm_SRGB:
            return true;
        default:
            return false;
    }
}

DDSFile::DXGIFormat DDSFile::GetDXGIFormat(const PixelFormat& pf) {
    if (pf.flags & uint32_t(PixelFormatFlagBits::RGB)) {
        switch (pf.bitCount) {
            case 32:
                if (pf.RBitMask == 0x000000ff && pf.GBitMask == 0x0000ff00 &&
                    pf.BBitMask == 0x00ff0000 && pf.ABitMask == 0xff000000) {
                    return R8G8B8A8_UNorm;
                }
                if (pf.RBitMask == 0x00ff0000 && pf.GBitMask == 0x0000ff00 &&
                    pf.BBitMask == 0x000000ff && pf.ABitMask == 0xff000000) {
                    return B8G8R8A8_UNorm;
                }
                if (pf.RBitMask == 0x00ff0000 && pf.GBitMask == 0x0000ff00 &&
                    pf.BBitMask == 0x000000ff && pf.ABitMask == 0x00000000) {
                    return B8G8R8X8_UNorm;
                }

                if (pf.RBitMask == 0x0000ffff && pf.GBitMask == 0xffff0000 &&
                    pf.BBitMask == 0x00000000 && pf.ABitMask == 0x00000000) {
                    return R16G16_UNorm;
                }

                if (pf.RBitMask == 0xffffffff && pf.GBitMask == 0x00000000 &&
                    pf.BBitMask == 0x00000000 && pf.ABitMask == 0x00000000) {
                    return R32_Float;
                }
                break;
            case 24:
                break;
            case 16:
                if (pf.RBitMask == 0x7c00 && pf.GBitMask == 0x03e0 &&
                    pf.BBitMask == 0x001f && pf.ABitMask == 0x8000) {
                    return B5G5R5A1_UNorm;
                }
                if (pf.RBitMask == 0xf800 && pf.GBitMask == 0x07e0 &&
                    pf.BBitMask == 0x001f && pf.ABitMask == 0x0000) {
                    return B5G6R5_UNorm;
                }

                if (pf.RBitMask == 0x0f00 && pf.GBitMask == 0x00f0 &&
                    pf.BBitMask == 0x000f && pf.ABitMask == 0xf000) {
                    return B4G4R4A4_UNorm;
                }
                if (pf.RBitMask == 0x00ff && pf.GBitMask == 0xff00 &&
                    pf.BBitMask == 0x0000 && pf.ABitMask == 0x0000) {
                    return R8G8_UNorm;
                }
                break;
            case 8:
                if (pf.RBitMask == 0x00ff && pf.GBitMask == 0x0000 &&
                    pf.BBitMask == 0x0000 && pf.ABitMask == 0x0000) {
                    return R8_UNorm;
                }
                break;
            default:
                break;
        }
    } else if (pf.flags & uint32_t(PixelFormatFlagBits::Luminance)) {
        if (8 == pf.bitCount) {
            if (pf.RBitMask == 0x000000ff && pf.GBitMask == 0x00000000 &&
                pf.BBitMask == 0x00000000 && pf.ABitMask == 0x00000000) {
                return R8_UNorm;
            }
            if (pf.RBitMask == 0x000000ff && pf.GBitMask == 0x0000ff00 &&
                pf.BBitMask == 0x00000000 && pf.ABitMask == 0x00000000) {
                return R8G8_UNorm;
            }
        }
        if (16 == pf.bitCount) {
            if (pf.RBitMask == 0x0000ffff && pf.GBitMask == 0x00000000 &&
                pf.BBitMask == 0x00000000 && pf.ABitMask == 0x00000000) {
                return R16_UNorm;
            }
            if (pf.RBitMask == 0x000000ff && pf.GBitMask == 0x0000ff00 &&
                pf.BBitMask == 0x00000000 && pf.ABitMask == 0x00000000) {
                return R8G8_UNorm;
            }
        }
    } else if (pf.flags & uint32_t(PixelFormatFlagBits::AlphaOnly)) {
        if (8 == pf.bitCount) {
            return A8_UNorm;
        }
    } else if (pf.flags & uint32_t(PixelFormatFlagBits::BumpDUDV)) {
        if (16 == pf.bitCount) {
            if (pf.RBitMask == 0x00ff && pf.GBitMask == 0xff00 &&
                pf.BBitMask == 0x0000 && pf.ABitMask == 0x0000) {
                return R8G8_SNorm;
            }
        }
        if (32 == pf.bitCount) {
            if (pf.RBitMask == 0x000000ff && pf.GBitMask == 0x0000ff00 &&
                pf.BBitMask == 0x00ff0000 && pf.ABitMask == 0xff000000) {
                return R8G8B8A8_SNorm;
            }
            if (pf.RBitMask == 0x0000ffff && pf.GBitMask == 0xffff0000 &&
                pf.BBitMask == 0x00000000 && pf.ABitMask == 0x00000000) {
                return R16G16_SNorm;
            }
        }
    } else if (pf.flags & uint32_t(PixelFormatFlagBits::FourCC)) {
        if (MakeFourCC('D', 'X', 'T', '1') == pf.fourCC) {
            return BC1_UNorm;
        }
        if (MakeFourCC('D', 'X', 'T', '3') == pf.fourCC) {
            return BC2_UNorm;
        }
        if (MakeFourCC('D', 'X', 'T', '5') == pf.fourCC) {
            return BC3_UNorm;
        }

        if (MakeFourCC('D', 'X', 'T', '4') == pf.fourCC) {
            return BC2_UNorm;
        }
        if (MakeFourCC('D', 'X', 'T', '5') == pf.fourCC) {
            return BC3_UNorm;
        }

        if (MakeFourCC('A', 'T', 'I', '1') == pf.fourCC) {
            return BC4_UNorm;
        }
        if (MakeFourCC('B', 'C', '4', 'U') == pf.fourCC) {
            return BC4_UNorm;
        }
        if (MakeFourCC('B', 'C', '4', 'S') == pf.fourCC) {
            return BC4_SNorm;
        }

        if (MakeFourCC('A', 'T', 'I', '2') == pf.fourCC) {
            return BC5_UNorm;
        }
        if (MakeFourCC('B', 'C', '5', 'U') == pf.fourCC) {
            return BC5_UNorm;
        }
        if (MakeFourCC('B', 'C', '5', 'S') == pf.fourCC) {
            return BC5_SNorm;
        }

        if (MakeFourCC('R', 'G', 'B', 'G') == pf.fourCC) {
            return R8G8_B8G8_UNorm;
        }
        if (MakeFourCC('G', 'R', 'G', 'B') == pf.fourCC) {
            return G8R8_G8B8_UNorm;
        }

        if (MakeFourCC('Y', 'U', 'Y', '2') == pf.fourCC) {
            return YUY2;
        }

        switch (pf.fourCC) {
            case 36:
                return R16G16B16A16_UNorm;
            case 110:
                return R16G16B16A16_SNorm;
            case 111:
                return R16_Float;
            case 112:
                return R16G16_Float;
            case 113:
                return R16G16B16A16_Float;
            case 114:
                return R32_Float;
            case 115:
                return R32G32_Float;
            case 116:
                return R32G32B32A32_Float;
        }
    }

    return Format_Unknown;
}

uint32_t DDSFile::GetBitsPerPixel(DXGIFormat fmt) {
    switch (fmt) {
        case R32G32B32A32_Typeless:
        case R32G32B32A32_Float:
        case R32G32B32A32_UInt:
        case R32G32B32A32_SInt:
            return 128;

        case R32G32B32_Typeless:
        case R32G32B32_Float:
        case R32G32B32_UInt:
        case R32G32B32_SInt:
            return 96;

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
        case Y216:
            return 64;

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
        case B8G8R8X8_UNorm:
        case R10G10B10_XR_BIAS_A2_UNorm:
        case B8G8R8A8_Typeless:
        case B8G8R8A8_UNorm_SRGB:
        case B8G8R8X8_Typeless:
        case B8G8R8X8_UNorm_SRGB:
        case AYUV:
        case Y410:
        case YUY2:
            return 32;

        case P010:
        case P016:
            return 24;

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
        case A8P8:
        case B4G4R4A4_UNorm:
            return 16;

        case NV12:
        case YUV420_OPAQUE:
        case NV11:
            return 12;

        case R8_Typeless:
        case R8_UNorm:
        case R8_UInt:
        case R8_SNorm:
        case R8_SInt:
        case A8_UNorm:
        case AI44:
        case IA44:
        case P8:
            return 8;

        case R1_UNorm:
            return 1;

        case BC1_Typeless:
        case BC1_UNorm:
        case BC1_UNorm_SRGB:
        case BC4_Typeless:
        case BC4_UNorm:
        case BC4_SNorm:
            return 4;

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
            return 8;
        default:
            return 0;
    }
}

Result DDSFile::Load(const char* filepath) {
    std::ifstream ifs(filepath, std::ios_base::binary);
    if (!ifs.is_open()) {
        return Result::ErrorFileOpen;
    }

    return Load(ifs);
}

Result DDSFile::Load(std::istream& input) {
    m_dds.clear();

    input.seekg(0, std::ios_base::beg);
    auto begPos = input.tellg();
    input.seekg(0, std::ios_base::end);
    auto endPos = input.tellg();
    input.seekg(0, std::ios_base::beg);

    auto fileSize = endPos - begPos;
    if (fileSize == 0) {
        return Result::ErrorRead;
    }
    std::vector<uint8_t> dds(fileSize);

    input.read(reinterpret_cast<char*>(dds.data()), fileSize);

    if (input.bad()) {
        return Result::ErrorRead;
    }

    return Load(std::move(dds));
}

Result DDSFile::Load(const uint8_t* data, size_t size) {
    std::vector<uint8_t> dds(data, data + size);
    return Load(std::move(dds));
}

Result DDSFile::Load(std::vector<uint8_t>&& dds) {
    m_dds.clear();

    if (dds.size() < 4) {
        return Result::ErrorSize;
    }

    for (int i = 0; i < 4; i++) {
        if (dds[i] != Magic[i]) {
            return Result::ErrorMagicWord;
        }
    }

    if ((sizeof(uint32_t) + sizeof(Header)) >= dds.size()) {
        return Result::ErrorSize;
    }

    std::memcpy(&m_header, dds.data() + sizeof(uint32_t), sizeof(Header));

    m_dds = std::move(dds);

    auto status = VerifyHeader();
    if (status != Result::Success) return status;

    return Result::Success;
}

Result DDSFile::VerifyHeader() {
    if (m_headerVerified) return Result::Success;

    if (m_header.size != sizeof(Header) ||
        m_header.pixelFormat.size != sizeof(PixelFormat)) {
        return Result::ErrorVerify;
    }

    m_hasDXT10Header = false;
    if ((m_header.pixelFormat.flags & uint32_t(PixelFormatFlagBits::FourCC)) &&
        (MakeFourCC('D', 'X', '1', '0') == m_header.pixelFormat.fourCC)) {
        if ((sizeof(uint32_t) + sizeof(Header) + sizeof(HeaderDXT10)) >=
            m_dds.size()) {
            return Result::ErrorSize;
        }
        m_hasDXT10Header = true;
    }

    m_isCubemap = false;
    if (m_header.mipMapCount == 0) {
        m_header.mipMapCount = 1;
    }

    if (m_hasDXT10Header) {
        // Copy the DXT10 header from m_dds
        std::memcpy(&m_headerDXT10,
                    m_dds.data() + sizeof(uint32_t) + sizeof(Header),
                    sizeof(HeaderDXT10));

        if (m_headerDXT10.arraySize == 0) {
            return Result::ErrorInvalidData;
        }

        switch (m_headerDXT10.format) {
            case AI44:
            case IA44:
            case P8:
            case A8P8:
                return Result::ErrorNotSupported;
            default:
                if (GetBitsPerPixel(m_headerDXT10.format) == 0) {
                    return Result::ErrorNotSupported;
                }
        }

        switch (m_headerDXT10.resourceDimension) {
            case Texture1D:
                if ((m_header.flags & uint32_t(HeaderFlagBits::Height) &&
                     (m_header.height != 1))) {
                    return Result::ErrorInvalidData;
                }
                m_header.height = m_header.depth = 1;
                break;
            case Texture2D:
                if (m_headerDXT10.miscFlag &
                    uint32_t(DXT10MiscFlagBits::TextureCube)) {
                    m_headerDXT10.arraySize *= 6;
                    m_isCubemap = true;
                }
                m_header.depth = 1;
                break;
            case Texture3D:
                if (!(m_header.flags & uint32_t(HeaderFlagBits::Volume))) {
                    return Result::ErrorInvalidData;
                }
                if (m_headerDXT10.arraySize > 1) {
                    return Result::ErrorNotSupported;
                }
                break;
            default:
                return Result::ErrorNotSupported;
        }

    } else {
        m_headerDXT10.format = GetDXGIFormat(m_header.pixelFormat);
        if (m_headerDXT10.format == Format_Unknown) {
            return Result::ErrorNotSupported;
        }

        if (m_header.flags & uint32_t(HeaderFlagBits::Volume)) {
            m_headerDXT10.resourceDimension = Texture3D;
        } else {
            auto caps2 = m_header.caps2 & uint32_t(CubemapAllFaces);
            if (caps2) {
                if (caps2 != uint32_t(CubemapAllFaces)) {
                    return Result::ErrorNotSupported;
                }
                m_headerDXT10.arraySize = 6;
                m_isCubemap = true;
            }

            m_header.depth = 1;
            m_headerDXT10.resourceDimension = Texture2D;
        }
    }

    m_headerVerified = true;
    return Result::Success;
}

Result DDSFile::PopulateImageDatas() {
    auto status = VerifyHeader();
    if (status != Result::Success) return status;

    ptrdiff_t offset = sizeof(uint32_t) + sizeof(Header) +
                       (m_hasDXT10Header ? sizeof(HeaderDXT10) : 0);

    std::vector<ImageData> imageDatas(m_header.mipMapCount *
                                      m_headerDXT10.arraySize);
    uint8_t* srcBits = m_dds.data() + offset;
    uint8_t* endBits = m_dds.data() + m_dds.size();
    uint32_t idx = 0;
    for (uint32_t j = 0; j < m_headerDXT10.arraySize; j++) {
        uint32_t w = m_header.width;
        uint32_t h = m_header.height;
        uint32_t d = m_header.depth;
        for (uint32_t i = 0; i < m_header.mipMapCount; i++) {
            uint32_t numBytes;
            uint32_t rowBytes;
            GetImageInfo(w, h, m_headerDXT10.format, &numBytes, &rowBytes,
                         nullptr);

            imageDatas[idx].m_width = w;
            imageDatas[idx].m_height = h;
            imageDatas[idx].m_depth = d;
            imageDatas[idx].m_mem = srcBits;
            imageDatas[idx].m_memPitch = rowBytes;
            imageDatas[idx].m_memSlicePitch = numBytes;
            idx++;

            if (srcBits + (numBytes * d) > endBits) {
                return Result::ErrorInvalidData;
            }
            srcBits += numBytes * d;
            w = std::max<uint32_t>(1, w / 2);
            h = std::max<uint32_t>(1, h / 2);
            d = std::max<uint32_t>(1, d / 2);
        }
    }

    m_imageDatas = std::move(imageDatas);

    return Result::Success;
}

void DDSFile::GetImageInfo(uint32_t w, uint32_t h, DXGIFormat fmt,
                           uint32_t* outNumBytes, uint32_t* outRowBytes,
                           uint32_t* outNumRows) {
    uint32_t numBytes = 0;
    uint32_t rowBytes = 0;
    uint32_t numRows = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    uint32_t bpe = 0;
    switch (fmt) {
        case BC1_Typeless:
        case BC1_UNorm:
        case BC1_UNorm_SRGB:
        case BC4_Typeless:
        case BC4_UNorm:
        case BC4_SNorm:
            bc = true;
            bpe = 8;
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
            bc = true;
            bpe = 16;
            break;

        case R8G8_B8G8_UNorm:
        case G8R8_G8B8_UNorm:
        case YUY2:
            packed = true;
            bpe = 4;
            break;

        case Y210:
        case Y216:
            packed = true;
            bpe = 8;
            break;

        case NV12:
        case YUV420_OPAQUE:
            planar = true;
            bpe = 2;
            break;

        case P010:
        case P016:
            planar = true;
            bpe = 4;
            break;
        default:
            break;
    }

    if (bc) {
        uint32_t numBlocksWide = 0;
        if (w > 0) {
            numBlocksWide = std::max<uint32_t>(1, (w + 3) / 4);
        }
        uint32_t numBlocksHigh = 0;
        if (h > 0) {
            numBlocksHigh = std::max<uint32_t>(1, (h + 3) / 4);
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    } else if (packed) {
        rowBytes = ((w + 1) >> 1) * bpe;
        numRows = h;
        numBytes = rowBytes * h;
    } else if (fmt == NV11) {
        rowBytes = ((w + 3) >> 2) * 4;
        numRows = h * 2;
        numBytes = rowBytes + numRows;
    } else if (planar) {
        rowBytes = ((w + 1) >> 1) * bpe;
        numBytes = (rowBytes * h) + ((rowBytes * h + 1) >> 1);
        numRows = h + ((h + 1) >> 1);
    } else {
        uint32_t bpp = GetBitsPerPixel(fmt);
        rowBytes = (w * bpp + 7) / 8;
        numRows = h;
        numBytes = rowBytes * h;
    }

    if (outNumBytes) {
        *outNumBytes = numBytes;
    }
    if (outRowBytes) {
        *outRowBytes = rowBytes;
    }
    if (outNumRows) {
        *outNumRows = numRows;
    }
}

bool DDSFile::Flip() {
    if (IsCompressed(m_headerDXT10.format)) {
        for (auto& imageData : m_imageDatas) {
            if (!FlipCompressedImage(imageData)) {
                return false;
            }
        }
    } else {
        for (auto& imageData : m_imageDatas) {
            if (!FlipImage(imageData)) {
                return false;
            }
        }
    }
    return true;
}

bool DDSFile::FlipImage(ImageData& imageData) {
    for (uint32_t y = 0; y < imageData.m_height / 2; y++) {
        auto line0 = (uint8_t*)imageData.m_mem + y * imageData.m_memPitch;
        auto line1 = (uint8_t*)imageData.m_mem +
                     (imageData.m_height - y - 1) * imageData.m_memPitch;
        for (uint32_t i = 0; i < imageData.m_memPitch; i++) {
            std::swap(*line0, *line1);
            line0++;
            line1++;
        }
    }
    return true;
}

bool DDSFile::FlipCompressedImage(ImageData& imageData) {
    if (BC1_Typeless == m_headerDXT10.format ||
        BC1_UNorm == m_headerDXT10.format ||
        BC1_UNorm_SRGB == m_headerDXT10.format) {
        FlipCompressedImageBC1(imageData);
        return true;
    } else if (BC2_Typeless == m_headerDXT10.format ||
               BC2_UNorm == m_headerDXT10.format ||
               BC2_UNorm_SRGB == m_headerDXT10.format) {
        FlipCompressedImageBC2(imageData);
        return true;
    } else if (BC3_Typeless == m_headerDXT10.format ||
               BC3_UNorm == m_headerDXT10.format ||
               BC3_UNorm_SRGB == m_headerDXT10.format) {
        FlipCompressedImageBC3(imageData);
        return true;
    } else if (BC4_Typeless == m_headerDXT10.format ||
               BC4_UNorm == m_headerDXT10.format ||
               BC4_SNorm == m_headerDXT10.format) {
        FlipCompressedImageBC4(imageData);
        return true;
    } else if (BC5_Typeless == m_headerDXT10.format ||
               BC5_UNorm == m_headerDXT10.format ||
               BC5_SNorm == m_headerDXT10.format) {
        FlipCompressedImageBC5(imageData);
        return true;
    }
    return false;
}

void DDSFile::FlipCompressedImageBC1(ImageData& imageData) {
    uint32_t numXBlocks = (imageData.m_width + 3) / 4;
    uint32_t numYBlocks = (imageData.m_height + 3) / 4;
    if (imageData.m_height == 1) {
    } else if (imageData.m_height == 2) {
        auto blocks = (BC1Block*)imageData.m_mem;
        for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            std::swap(block->m_row0, block->m_row1);
            std::swap(block->m_row2, block->m_row3);
        }
    } else {
        for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC1Block*)((uint8_t*)imageData.m_mem +
                                       imageData.m_memPitch * y);
            auto blocks1 =
                (BC1Block*)((uint8_t*)imageData.m_mem +
                            imageData.m_memPitch * (numYBlocks - y - 1));
            for (uint32_t x = 0; x < numXBlocks; x++) {
                auto block0 = blocks0 + x;
                auto block1 = blocks1 + x;
                if (blocks0 != blocks1) {
                    std::swap(block0->m_color0, block1->m_color0);
                    std::swap(block0->m_color1, block1->m_color1);
                    std::swap(block0->m_row0, block1->m_row3);
                    std::swap(block0->m_row1, block1->m_row2);
                    std::swap(block0->m_row2, block1->m_row1);
                    std::swap(block0->m_row3, block1->m_row0);
                } else {
                    std::swap(block0->m_row0, block0->m_row3);
                    std::swap(block0->m_row1, block0->m_row2);
                }
            }
        }
    }
}

void DDSFile::FlipCompressedImageBC2(ImageData& imageData) {
    uint32_t numXBlocks = (imageData.m_width + 3) / 4;
    uint32_t numYBlocks = (imageData.m_height + 3) / 4;
    if (imageData.m_height == 1) {
    } else if (imageData.m_height == 2) {
        auto blocks = (BC2Block*)imageData.m_mem;
        for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            std::swap(block->m_alphaRow0, block->m_alphaRow1);
            std::swap(block->m_alphaRow2, block->m_alphaRow3);
            std::swap(block->m_row0, block->m_row1);
            std::swap(block->m_row2, block->m_row3);
        }
    } else {
        for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC2Block*)((uint8_t*)imageData.m_mem +
                                       imageData.m_memPitch * y);
            auto blocks1 =
                (BC2Block*)((uint8_t*)imageData.m_mem +
                            imageData.m_memPitch * (numYBlocks - y - 1));
            for (uint32_t x = 0; x < numXBlocks; x++) {
                auto block0 = blocks0 + x;
                auto block1 = blocks1 + x;
                if (block0 != block1) {
                    std::swap(block0->m_alphaRow0, block1->m_alphaRow3);
                    std::swap(block0->m_alphaRow1, block1->m_alphaRow2);
                    std::swap(block0->m_alphaRow2, block1->m_alphaRow1);
                    std::swap(block0->m_alphaRow3, block1->m_alphaRow0);
                    std::swap(block0->m_color0, block1->m_color0);
                    std::swap(block0->m_color1, block1->m_color1);
                    std::swap(block0->m_row0, block1->m_row3);
                    std::swap(block0->m_row1, block1->m_row2);
                    std::swap(block0->m_row2, block1->m_row1);
                    std::swap(block0->m_row3, block1->m_row0);
                } else {
                    std::swap(block0->m_alphaRow0, block0->m_alphaRow3);
                    std::swap(block0->m_alphaRow1, block0->m_alphaRow2);
                    std::swap(block0->m_row0, block0->m_row3);
                    std::swap(block0->m_row1, block0->m_row2);
                }
            }
        }
    }
}

void DDSFile::FlipCompressedImageBC3(ImageData& imageData) {
    uint32_t numXBlocks = (imageData.m_width + 3) / 4;
    uint32_t numYBlocks = (imageData.m_height + 3) / 4;
    if (imageData.m_height == 1) {
    } else if (imageData.m_height == 2) {
        auto blocks = (BC3Block*)imageData.m_mem;
        for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            uint8_t r0 = (block->m_alphaR1 >> 4) | (block->m_alphaR2 << 4);
            uint8_t r1 = (block->m_alphaR2 >> 4) | (block->m_alphaR0 << 4);
            uint8_t r2 = (block->m_alphaR0 >> 4) | (block->m_alphaR1 << 4);
            uint8_t r3 = (block->m_alphaR4 >> 4) | (block->m_alphaR5 << 4);
            uint8_t r4 = (block->m_alphaR5 >> 4) | (block->m_alphaR3 << 4);
            uint8_t r5 = (block->m_alphaR3 >> 4) | (block->m_alphaR4 << 4);

            block->m_alphaR0 = r0;
            block->m_alphaR1 = r1;
            block->m_alphaR2 = r2;
            block->m_alphaR3 = r3;
            block->m_alphaR4 = r4;
            block->m_alphaR5 = r5;
            std::swap(block->m_row0, block->m_row1);
            std::swap(block->m_row2, block->m_row3);
        }
    } else {
        for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC3Block*)((uint8_t*)imageData.m_mem +
                                       imageData.m_memPitch * y);
            auto blocks1 =
                (BC3Block*)((uint8_t*)imageData.m_mem +
                            imageData.m_memPitch * (numYBlocks - y - 1));
            for (uint32_t x = 0; x < numXBlocks; x++) {
                auto block0 = blocks0 + x;
                auto block1 = blocks1 + x;
                if (block0 != block1) {
                    std::swap(block0->m_alpha0, block1->m_alpha0);
                    std::swap(block0->m_alpha1, block1->m_alpha1);

                    uint8_t r0[6];
                    r0[0] = (block0->m_alphaR4 >> 4) | (block0->m_alphaR5 << 4);
                    r0[1] = (block0->m_alphaR5 >> 4) | (block0->m_alphaR3 << 4);
                    r0[2] = (block0->m_alphaR3 >> 4) | (block0->m_alphaR4 << 4);
                    r0[3] = (block0->m_alphaR1 >> 4) | (block0->m_alphaR2 << 4);
                    r0[4] = (block0->m_alphaR2 >> 4) | (block0->m_alphaR0 << 4);
                    r0[5] = (block0->m_alphaR0 >> 4) | (block0->m_alphaR1 << 4);
                    uint8_t r1[6];
                    r1[0] = (block1->m_alphaR4 >> 4) | (block1->m_alphaR5 << 4);
                    r1[1] = (block1->m_alphaR5 >> 4) | (block1->m_alphaR3 << 4);
                    r1[2] = (block1->m_alphaR3 >> 4) | (block1->m_alphaR4 << 4);
                    r1[3] = (block1->m_alphaR1 >> 4) | (block1->m_alphaR2 << 4);
                    r1[4] = (block1->m_alphaR2 >> 4) | (block1->m_alphaR0 << 4);
                    r1[5] = (block1->m_alphaR0 >> 4) | (block1->m_alphaR1 << 4);

                    block0->m_alphaR0 = r1[0];
                    block0->m_alphaR1 = r1[1];
                    block0->m_alphaR2 = r1[2];
                    block0->m_alphaR3 = r1[3];
                    block0->m_alphaR4 = r1[4];
                    block0->m_alphaR5 = r1[5];

                    block1->m_alphaR0 = r0[0];
                    block1->m_alphaR1 = r0[1];
                    block1->m_alphaR2 = r0[2];
                    block1->m_alphaR3 = r0[3];
                    block1->m_alphaR4 = r0[4];
                    block1->m_alphaR5 = r0[5];

                    std::swap(block0->m_color0, block1->m_color0);
                    std::swap(block0->m_color1, block1->m_color1);
                    std::swap(block0->m_row0, block1->m_row3);
                    std::swap(block0->m_row1, block1->m_row2);
                    std::swap(block0->m_row2, block1->m_row1);
                    std::swap(block0->m_row3, block1->m_row0);
                } else {
                    uint8_t r0[6];
                    r0[0] = (block0->m_alphaR4 >> 4) | (block0->m_alphaR5 << 4);
                    r0[1] = (block0->m_alphaR5 >> 4) | (block0->m_alphaR3 << 4);
                    r0[2] = (block0->m_alphaR3 >> 4) | (block0->m_alphaR4 << 4);
                    r0[3] = (block0->m_alphaR1 >> 4) | (block0->m_alphaR2 << 4);
                    r0[4] = (block0->m_alphaR2 >> 4) | (block0->m_alphaR0 << 4);
                    r0[5] = (block0->m_alphaR0 >> 4) | (block0->m_alphaR1 << 4);

                    block0->m_alphaR0 = r0[0];
                    block0->m_alphaR1 = r0[1];
                    block0->m_alphaR2 = r0[2];
                    block0->m_alphaR3 = r0[3];
                    block0->m_alphaR4 = r0[4];
                    block0->m_alphaR5 = r0[5];

                    std::swap(block0->m_row0, block0->m_row3);
                    std::swap(block0->m_row1, block0->m_row2);
                }
            }
        }
    }
}

void DDSFile::FlipCompressedImageBC4(ImageData& imageData) {
    uint32_t numXBlocks = (imageData.m_width + 3) / 4;
    uint32_t numYBlocks = (imageData.m_height + 3) / 4;
    if (imageData.m_height == 1) {
    } else if (imageData.m_height == 2) {
        auto blocks = (BC4Block*)imageData.m_mem;
        for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            uint8_t r0 = (block->m_redR1 >> 4) | (block->m_redR2 << 4);
            uint8_t r1 = (block->m_redR2 >> 4) | (block->m_redR0 << 4);
            uint8_t r2 = (block->m_redR0 >> 4) | (block->m_redR1 << 4);
            uint8_t r3 = (block->m_redR4 >> 4) | (block->m_redR5 << 4);
            uint8_t r4 = (block->m_redR5 >> 4) | (block->m_redR3 << 4);
            uint8_t r5 = (block->m_redR3 >> 4) | (block->m_redR4 << 4);

            block->m_redR0 = r0;
            block->m_redR1 = r1;
            block->m_redR2 = r2;
            block->m_redR3 = r3;
            block->m_redR4 = r4;
            block->m_redR5 = r5;
        }
    } else {
        for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC4Block*)((uint8_t*)imageData.m_mem +
                                       imageData.m_memPitch * y);
            auto blocks1 =
                (BC4Block*)((uint8_t*)imageData.m_mem +
                            imageData.m_memPitch * (numYBlocks - y - 1));
            for (uint32_t x = 0; x < numXBlocks; x++) {
                auto block0 = blocks0 + x;
                auto block1 = blocks1 + x;
                if (block0 != block1) {
                    std::swap(block0->m_red0, block1->m_red0);
                    std::swap(block0->m_red1, block1->m_red1);

                    uint8_t r0[6];
                    r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                    r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                    r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                    r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                    r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                    r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);
                    uint8_t r1[6];
                    r1[0] = (block1->m_redR4 >> 4) | (block1->m_redR5 << 4);
                    r1[1] = (block1->m_redR5 >> 4) | (block1->m_redR3 << 4);
                    r1[2] = (block1->m_redR3 >> 4) | (block1->m_redR4 << 4);
                    r1[3] = (block1->m_redR1 >> 4) | (block1->m_redR2 << 4);
                    r1[4] = (block1->m_redR2 >> 4) | (block1->m_redR0 << 4);
                    r1[5] = (block1->m_redR0 >> 4) | (block1->m_redR1 << 4);

                    block0->m_redR0 = r1[0];
                    block0->m_redR1 = r1[1];
                    block0->m_redR2 = r1[2];
                    block0->m_redR3 = r1[3];
                    block0->m_redR4 = r1[4];
                    block0->m_redR5 = r1[5];

                    block1->m_redR0 = r0[0];
                    block1->m_redR1 = r0[1];
                    block1->m_redR2 = r0[2];
                    block1->m_redR3 = r0[3];
                    block1->m_redR4 = r0[4];
                    block1->m_redR5 = r0[5];

                } else {
                    uint8_t r0[6];
                    r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                    r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                    r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                    r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                    r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                    r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);

                    block0->m_redR0 = r0[0];
                    block0->m_redR1 = r0[1];
                    block0->m_redR2 = r0[2];
                    block0->m_redR3 = r0[3];
                    block0->m_redR4 = r0[4];
                    block0->m_redR5 = r0[5];
                }
            }
        }
    }
}

void DDSFile::FlipCompressedImageBC5(ImageData& imageData) {
    uint32_t numXBlocks = (imageData.m_width + 3) / 4;
    uint32_t numYBlocks = (imageData.m_height + 3) / 4;
    if (imageData.m_height == 1) {
    } else if (imageData.m_height == 2) {
        auto blocks = (BC5Block*)imageData.m_mem;
        for (uint32_t x = 0; x < numXBlocks; x++) {
            auto block = blocks + x;
            uint8_t r0 = (block->m_redR1 >> 4) | (block->m_redR2 << 4);
            uint8_t r1 = (block->m_redR2 >> 4) | (block->m_redR0 << 4);
            uint8_t r2 = (block->m_redR0 >> 4) | (block->m_redR1 << 4);
            uint8_t r3 = (block->m_redR4 >> 4) | (block->m_redR5 << 4);
            uint8_t r4 = (block->m_redR5 >> 4) | (block->m_redR3 << 4);
            uint8_t r5 = (block->m_redR3 >> 4) | (block->m_redR4 << 4);

            block->m_redR0 = r0;
            block->m_redR1 = r1;
            block->m_redR2 = r2;
            block->m_redR3 = r3;
            block->m_redR4 = r4;
            block->m_redR5 = r5;

            uint8_t g0 = (block->m_greenR1 >> 4) | (block->m_greenR2 << 4);
            uint8_t g1 = (block->m_greenR2 >> 4) | (block->m_greenR0 << 4);
            uint8_t g2 = (block->m_greenR0 >> 4) | (block->m_greenR1 << 4);
            uint8_t g3 = (block->m_greenR4 >> 4) | (block->m_greenR5 << 4);
            uint8_t g4 = (block->m_greenR5 >> 4) | (block->m_greenR3 << 4);
            uint8_t g5 = (block->m_greenR3 >> 4) | (block->m_greenR4 << 4);

            block->m_greenR0 = g0;
            block->m_greenR1 = g1;
            block->m_greenR2 = g2;
            block->m_greenR3 = g3;
            block->m_greenR4 = g4;
            block->m_greenR5 = g5;
        }
    } else {
        for (uint32_t y = 0; y < (numYBlocks + 1) / 2; y++) {
            auto blocks0 = (BC5Block*)((uint8_t*)imageData.m_mem +
                                       imageData.m_memPitch * y);
            auto blocks1 =
                (BC5Block*)((uint8_t*)imageData.m_mem +
                            imageData.m_memPitch * (numYBlocks - y - 1));
            for (uint32_t x = 0; x < numXBlocks; x++) {
                auto block0 = blocks0 + x;
                auto block1 = blocks1 + x;
                if (block0 != block1) {
                    std::swap(block0->m_red0, block1->m_red0);
                    std::swap(block0->m_red1, block1->m_red1);

                    uint8_t r0[6];
                    r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                    r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                    r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                    r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                    r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                    r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);
                    uint8_t r1[6];
                    r1[0] = (block1->m_redR4 >> 4) | (block1->m_redR5 << 4);
                    r1[1] = (block1->m_redR5 >> 4) | (block1->m_redR3 << 4);
                    r1[2] = (block1->m_redR3 >> 4) | (block1->m_redR4 << 4);
                    r1[3] = (block1->m_redR1 >> 4) | (block1->m_redR2 << 4);
                    r1[4] = (block1->m_redR2 >> 4) | (block1->m_redR0 << 4);
                    r1[5] = (block1->m_redR0 >> 4) | (block1->m_redR1 << 4);

                    block0->m_redR0 = r1[0];
                    block0->m_redR1 = r1[1];
                    block0->m_redR2 = r1[2];
                    block0->m_redR3 = r1[3];
                    block0->m_redR4 = r1[4];
                    block0->m_redR5 = r1[5];

                    block1->m_redR0 = r0[0];
                    block1->m_redR1 = r0[1];
                    block1->m_redR2 = r0[2];
                    block1->m_redR3 = r0[3];
                    block1->m_redR4 = r0[4];
                    block1->m_redR5 = r0[5];

                    std::swap(block0->m_green0, block1->m_green0);
                    std::swap(block0->m_green1, block1->m_green1);

                    uint8_t g0[6];
                    g0[0] = (block0->m_greenR4 >> 4) | (block0->m_greenR5 << 4);
                    g0[1] = (block0->m_greenR5 >> 4) | (block0->m_greenR3 << 4);
                    g0[2] = (block0->m_greenR3 >> 4) | (block0->m_greenR4 << 4);
                    g0[3] = (block0->m_greenR1 >> 4) | (block0->m_greenR2 << 4);
                    g0[4] = (block0->m_greenR2 >> 4) | (block0->m_greenR0 << 4);
                    g0[5] = (block0->m_greenR0 >> 4) | (block0->m_greenR1 << 4);
                    uint8_t g1[6];
                    g1[0] = (block1->m_greenR4 >> 4) | (block1->m_greenR5 << 4);
                    g1[1] = (block1->m_greenR5 >> 4) | (block1->m_greenR3 << 4);
                    g1[2] = (block1->m_greenR3 >> 4) | (block1->m_greenR4 << 4);
                    g1[3] = (block1->m_greenR1 >> 4) | (block1->m_greenR2 << 4);
                    g1[4] = (block1->m_greenR2 >> 4) | (block1->m_greenR0 << 4);
                    g1[5] = (block1->m_greenR0 >> 4) | (block1->m_greenR1 << 4);

                    block0->m_greenR0 = g1[0];
                    block0->m_greenR1 = g1[1];
                    block0->m_greenR2 = g1[2];
                    block0->m_greenR3 = g1[3];
                    block0->m_greenR4 = g1[4];
                    block0->m_greenR5 = g1[5];

                    block1->m_greenR0 = g0[0];
                    block1->m_greenR1 = g0[1];
                    block1->m_greenR2 = g0[2];
                    block1->m_greenR3 = g0[3];
                    block1->m_greenR4 = g0[4];
                    block1->m_greenR5 = g0[5];
                } else {
                    uint8_t r0[6];
                    r0[0] = (block0->m_redR4 >> 4) | (block0->m_redR5 << 4);
                    r0[1] = (block0->m_redR5 >> 4) | (block0->m_redR3 << 4);
                    r0[2] = (block0->m_redR3 >> 4) | (block0->m_redR4 << 4);
                    r0[3] = (block0->m_redR1 >> 4) | (block0->m_redR2 << 4);
                    r0[4] = (block0->m_redR2 >> 4) | (block0->m_redR0 << 4);
                    r0[5] = (block0->m_redR0 >> 4) | (block0->m_redR1 << 4);

                    block0->m_redR0 = r0[0];
                    block0->m_redR1 = r0[1];
                    block0->m_redR2 = r0[2];
                    block0->m_redR3 = r0[3];
                    block0->m_redR4 = r0[4];
                    block0->m_redR5 = r0[5];

                    uint8_t g0[6];
                    g0[0] = (block0->m_greenR4 >> 4) | (block0->m_greenR5 << 4);
                    g0[1] = (block0->m_greenR5 >> 4) | (block0->m_greenR3 << 4);
                    g0[2] = (block0->m_greenR3 >> 4) | (block0->m_greenR4 << 4);
                    g0[3] = (block0->m_greenR1 >> 4) | (block0->m_greenR2 << 4);
                    g0[4] = (block0->m_greenR2 >> 4) | (block0->m_greenR0 << 4);
                    g0[5] = (block0->m_greenR0 >> 4) | (block0->m_greenR1 << 4);

                    block0->m_greenR0 = g0[0];
                    block0->m_greenR1 = g0[1];
                    block0->m_greenR2 = g0[2];
                    block0->m_greenR3 = g0[3];
                    block0->m_greenR4 = g0[4];
                    block0->m_greenR5 = g0[5];
                }
            }
        }
    }
}

}  // namespace tinyddsloader

#endif  // !TINYDDSLOADER_IMPLEMENTATION

#endif  // !TINYDDSLOADER_H_
