#ifndef VZT_VULKAN_TYPE_HPP
#define VZT_VULKAN_TYPE_HPP

#include <volk.h>

#include "setup.hpp"
#include "vzt/core/meta.hpp"

namespace vzt
{
    enum class Format : uint32_t
    {
        Undefined                               = VK_FORMAT_UNDEFINED,
        R4G4                                    = VK_FORMAT_R4G4_UNORM_PACK8,
        R4G4B4A4UnormPack16                     = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
        B4G4R4A4UnormPack16                     = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
        R5G6B5UnormPack16                       = VK_FORMAT_R5G6B5_UNORM_PACK16,
        B5G6R5UnormPack16                       = VK_FORMAT_B5G6R5_UNORM_PACK16,
        R5G5B5A1UnormPack16                     = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
        B5G5R5A1UnormPack16                     = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
        A1R5G5B5UnormPack16                     = VK_FORMAT_A1R5G5B5_UNORM_PACK16,
        R8UNorm                                 = VK_FORMAT_R8_UNORM,
        R8SNorm                                 = VK_FORMAT_R8_SNORM,
        R8UScaled                               = VK_FORMAT_R8_USCALED,
        R8SScaled                               = VK_FORMAT_R8_SSCALED,
        R8UInt                                  = VK_FORMAT_R8_UINT,
        R8SInt                                  = VK_FORMAT_R8_SINT,
        R8SRGB                                  = VK_FORMAT_R8_SRGB,
        R8G8UNorm                               = VK_FORMAT_R8G8_UNORM,
        R8G8SNorm                               = VK_FORMAT_R8G8_SNORM,
        R8G8UScaled                             = VK_FORMAT_R8G8_USCALED,
        R8G8SScaled                             = VK_FORMAT_R8G8_SSCALED,
        R8G8UInt                                = VK_FORMAT_R8G8_UINT,
        R8G8SInt                                = VK_FORMAT_R8G8_SINT,
        R8G8SRGB                                = VK_FORMAT_R8G8_SRGB,
        R8G8B8UNorm                             = VK_FORMAT_R8G8B8_UNORM,
        R8G8B8SNorm                             = VK_FORMAT_R8G8B8_SNORM,
        R8G8B8UScaled                           = VK_FORMAT_R8G8B8_USCALED,
        R8G8B8SScaled                           = VK_FORMAT_R8G8B8_SSCALED,
        R8G8B8UInt                              = VK_FORMAT_R8G8B8_UINT,
        R8G8B8SInt                              = VK_FORMAT_R8G8B8_SINT,
        R8G8B8SRGB                              = VK_FORMAT_R8G8B8_SRGB,
        B8G8R8UNorm                             = VK_FORMAT_B8G8R8_UNORM,
        B8G8R8SNorm                             = VK_FORMAT_B8G8R8_SNORM,
        B8G8R8UScaled                           = VK_FORMAT_B8G8R8_USCALED,
        B8G8R8SScaled                           = VK_FORMAT_B8G8R8_SSCALED,
        B8G8R8UInt                              = VK_FORMAT_B8G8R8_UINT,
        B8G8R8SInt                              = VK_FORMAT_B8G8R8_SINT,
        B8G8R8SRGB                              = VK_FORMAT_B8G8R8_SRGB,
        R8G8B8A8UNorm                           = VK_FORMAT_R8G8B8A8_UNORM,
        R8G8B8A8SNorm                           = VK_FORMAT_R8G8B8A8_SNORM,
        R8G8B8A8UScaled                         = VK_FORMAT_R8G8B8A8_USCALED,
        R8G8B8A8SScaled                         = VK_FORMAT_R8G8B8A8_SSCALED,
        R8G8B8A8UInt                            = VK_FORMAT_R8G8B8A8_UINT,
        R8G8B8A8SInt                            = VK_FORMAT_R8G8B8A8_SINT,
        R8G8B8A8SRGB                            = VK_FORMAT_R8G8B8A8_SRGB,
        B8G8R8A8UNorm                           = VK_FORMAT_B8G8R8A8_UNORM,
        B8G8R8A8SNorm                           = VK_FORMAT_B8G8R8A8_SNORM,
        B8G8R8A8UScaled                         = VK_FORMAT_B8G8R8A8_USCALED,
        B8G8R8A8SScaled                         = VK_FORMAT_B8G8R8A8_SSCALED,
        B8G8R8A8UInt                            = VK_FORMAT_B8G8R8A8_UINT,
        B8G8R8A8SInt                            = VK_FORMAT_B8G8R8A8_SINT,
        B8G8R8A8SRGB                            = VK_FORMAT_B8G8R8A8_SRGB,
        A8B8G8R8UNormPack32                     = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        A8B8G8R8SNormPack32                     = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
        A8B8G8R8UScaledPack32                   = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
        A8B8G8R8SScaledPack32                   = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
        A8B8G8R8UIntPack32                      = VK_FORMAT_A8B8G8R8_UINT_PACK32,
        A8B8G8R8SIntPack32                      = VK_FORMAT_A8B8G8R8_SINT_PACK32,
        A8B8G8R8SRGBPack32                      = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
        A2R10G10B10UNormPack32                  = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        A2R10G10B10SNormPack32                  = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
        A2R10G10B10UScaledPack32                = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
        A2R10G10B10SScaledPack32                = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
        A2R10G10B10UIntPack32                   = VK_FORMAT_A2R10G10B10_UINT_PACK32,
        A2R10G10B10SIntPack32                   = VK_FORMAT_A2R10G10B10_SINT_PACK32,
        A2B10G10R10UNormPack32                  = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        A2B10G10R10SNormPack32                  = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
        A2B10G10R10UScaledPack32                = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
        A2B10G10R10SScaledPack32                = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
        A2B10G10R10UIntPack32                   = VK_FORMAT_A2B10G10R10_UINT_PACK32,
        A2B10G10R10SIntPack32                   = VK_FORMAT_A2B10G10R10_SINT_PACK32,
        R16UNorm                                = VK_FORMAT_R16_UNORM,
        R16SNorm                                = VK_FORMAT_R16_SNORM,
        R16UScaled                              = VK_FORMAT_R16_USCALED,
        R16SScaled                              = VK_FORMAT_R16_SSCALED,
        R16UInt                                 = VK_FORMAT_R16_UINT,
        R16SInt                                 = VK_FORMAT_R16_SINT,
        R16SFloat                               = VK_FORMAT_R16_SFLOAT,
        R16G16UNorm                             = VK_FORMAT_R16G16_UNORM,
        R16G16SNorm                             = VK_FORMAT_R16G16_SNORM,
        R16G16UScaled                           = VK_FORMAT_R16G16_USCALED,
        R16G16SScaled                           = VK_FORMAT_R16G16_SSCALED,
        R16G16UInt                              = VK_FORMAT_R16G16_UINT,
        R16G16SInt                              = VK_FORMAT_R16G16_SINT,
        R16G16SFloat                            = VK_FORMAT_R16G16_SFLOAT,
        R16G16B16UNorm                          = VK_FORMAT_R16G16B16_UNORM,
        R16G16B16SNorm                          = VK_FORMAT_R16G16B16_SNORM,
        R16G16B16UScaled                        = VK_FORMAT_R16G16B16_USCALED,
        R16G16B16SScaled                        = VK_FORMAT_R16G16B16_SSCALED,
        R16G16B16UInt                           = VK_FORMAT_R16G16B16_UINT,
        R16G16B16SInt                           = VK_FORMAT_R16G16B16_SINT,
        R16G16B16SFloat                         = VK_FORMAT_R16G16B16_SFLOAT,
        R16G16B16A16UNorm                       = VK_FORMAT_R16G16B16A16_UNORM,
        R16G16B16A16SNorm                       = VK_FORMAT_R16G16B16A16_SNORM,
        R16G16B16A16UScaled                     = VK_FORMAT_R16G16B16A16_USCALED,
        R16G16B16A16SScaled                     = VK_FORMAT_R16G16B16A16_SSCALED,
        R16G16B16A16UInt                        = VK_FORMAT_R16G16B16A16_UINT,
        R16G16B16A16SInt                        = VK_FORMAT_R16G16B16A16_SINT,
        R16G16B16A16SFloat                      = VK_FORMAT_R16G16B16A16_SFLOAT,
        R32UInt                                 = VK_FORMAT_R32_UINT,
        R32SInt                                 = VK_FORMAT_R32_SINT,
        R32SFloat                               = VK_FORMAT_R32_SFLOAT,
        R32G32UInt                              = VK_FORMAT_R32G32_UINT,
        R32G32SInt                              = VK_FORMAT_R32G32_SINT,
        R32G32SFloat                            = VK_FORMAT_R32G32_SFLOAT,
        R32G32B32UInt                           = VK_FORMAT_R32G32B32_UINT,
        R32G32B32SInt                           = VK_FORMAT_R32G32B32_SINT,
        R32G32B32SFloat                         = VK_FORMAT_R32G32B32_SFLOAT,
        R32G32B32A32UInt                        = VK_FORMAT_R32G32B32A32_UINT,
        R32G32B32A32SInt                        = VK_FORMAT_R32G32B32A32_SINT,
        R32G32B32A32SFloat                      = VK_FORMAT_R32G32B32A32_SFLOAT,
        R64UInt                                 = VK_FORMAT_R64_UINT,
        R64SInt                                 = VK_FORMAT_R64_SINT,
        R64SFloat                               = VK_FORMAT_R64_SFLOAT,
        R64G64UInt                              = VK_FORMAT_R64G64_UINT,
        R64G64SInt                              = VK_FORMAT_R64G64_SINT,
        R64G64SFloat                            = VK_FORMAT_R64G64_SFLOAT,
        R64G64B64UInt                           = VK_FORMAT_R64G64B64_UINT,
        R64G64B64SInt                           = VK_FORMAT_R64G64B64_SINT,
        R64G64B64SFloat                         = VK_FORMAT_R64G64B64_SFLOAT,
        R64G64B64A64UInt                        = VK_FORMAT_R64G64B64A64_UINT,
        R64G64B64A64SInt                        = VK_FORMAT_R64G64B64A64_SINT,
        R64G64B64A64SFloat                      = VK_FORMAT_R64G64B64A64_SFLOAT,
        B10G11R11UFloatPack32                   = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
        E5B9G9R9UFloatPack32                    = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
        D16UNorm                                = VK_FORMAT_D16_UNORM,
        X8D24UNormPack32                        = VK_FORMAT_X8_D24_UNORM_PACK32,
        D32SFloat                               = VK_FORMAT_D32_SFLOAT,
        S8UInt                                  = VK_FORMAT_S8_UINT,
        D16UNormS8UInt                          = VK_FORMAT_D16_UNORM_S8_UINT,
        D24UNormS8UInt                          = VK_FORMAT_D24_UNORM_S8_UINT,
        D32SFloatS8UInt                         = VK_FORMAT_D32_SFLOAT_S8_UINT,
        BC1RGBUNormBlock                        = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
        BC1RGBSRGBBlock                         = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
        BC1RGBAUNormBlock                       = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
        BC1RGBASRGBBlock                        = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
        BC2UNormBlock                           = VK_FORMAT_BC2_UNORM_BLOCK,
        BC2SRGBBlock                            = VK_FORMAT_BC2_SRGB_BLOCK,
        BC3UNormBlock                           = VK_FORMAT_BC3_UNORM_BLOCK,
        BC3SRGBBlock                            = VK_FORMAT_BC3_SRGB_BLOCK,
        BC4UNormBlock                           = VK_FORMAT_BC4_UNORM_BLOCK,
        BC4SNormBlock                           = VK_FORMAT_BC4_SNORM_BLOCK,
        BC5UNormBlock                           = VK_FORMAT_BC5_UNORM_BLOCK,
        BC5SNormBlock                           = VK_FORMAT_BC5_SNORM_BLOCK,
        BC6HUFloatBlock                         = VK_FORMAT_BC6H_UFLOAT_BLOCK,
        BC6HSFloatBlock                         = VK_FORMAT_BC6H_SFLOAT_BLOCK,
        BC7UNormBlock                           = VK_FORMAT_BC7_UNORM_BLOCK,
        BC7SRGBBlock                            = VK_FORMAT_BC7_SRGB_BLOCK,
        ETC2R8G8B8UNormBlock                    = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
        ETC2R8G8B8SRGBBlock                     = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
        ETC2R8G8B8A1UNormBlock                  = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
        ETC2R8G8B8A1SRGBBlock                   = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
        ETC2R8G8B8A8UNormBlock                  = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
        ETC2R8G8B8A8SRGBBlock                   = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
        EACR11UNormBlock                        = VK_FORMAT_EAC_R11_UNORM_BLOCK,
        EACR11SNormBlock                        = VK_FORMAT_EAC_R11_SNORM_BLOCK,
        EACR11G11UNormBlock                     = VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
        EACR11G11SNormBlock                     = VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
        ASTC4x4UNormBlock                       = VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
        ASTC4x4SRGBBlock                        = VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
        ASTC5x4UNormBlock                       = VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
        ASTC5x4SRGBBlock                        = VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
        ASTC5x5UNormBlock                       = VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
        ASTC5x5SRGBBlock                        = VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
        ASTC6x5UNormBlock                       = VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
        ASTC6x5SRGBBlock                        = VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
        ASTC6x6UNormBlock                       = VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
        ASTC6x6SRGBBlock                        = VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
        ASTC8x5UNormBlock                       = VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
        ASTC8x5SRGBBlock                        = VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
        ASTC8x6UNormBlock                       = VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
        ASTC8x6SRGBBlock                        = VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
        ASTC8x8UNormBlock                       = VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
        ASTC8x8SRGBBlock                        = VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
        ASTC10x5UNormBlock                      = VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
        ASTC10x5SRGBBlock                       = VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
        ASTC10x6UNormBlock                      = VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
        ASTC10x6SRGBBlock                       = VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
        ASTC10x8UNormBlock                      = VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
        ASTC10x8SRGBBlock                       = VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
        ASTC10x10UNormBlock                     = VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
        ASTC10x10SRGBBlock                      = VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
        ASTC12x10UNormBlock                     = VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
        ASTC12x10SRGBBlock                      = VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
        ASTC12x12UNormBlock                     = VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
        ASTC12x12SRGBBlock                      = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
        G8B8G8R8422UNorm                        = VK_FORMAT_G8B8G8R8_422_UNORM,
        B8G8R8G8422UNorm                        = VK_FORMAT_B8G8R8G8_422_UNORM,
        G8B8R83Plane420UNorm                    = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
        G8B8R82PLANE420UNorm                    = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
        G8B8R83PLANE422UNorm                    = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
        G8B8R82PLANE422UNorm                    = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
        G8B8R83PLANE444UNorm                    = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
        R10X6UNormPack16                        = VK_FORMAT_R10X6_UNORM_PACK16,
        R10X6G10X6UNorm2Pack16                  = VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
        R10X6G10X6B10X6A10X6UNorm4Pack16        = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        G10X6B10X6G10X6R10X6422UNorm4Pack16     = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        B10X6G10X6R10X6G10X6422UNorm4Pack16     = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        G10X6B10X6R10X63Plane420UNorm3Pack16    = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        G10X6B10X6R10X62Plane420UNorm3Pack16    = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        G10X6B10X6R10X63Plane422UNorm3Pack16    = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        G10X6B10X6R10X62Plane422UNorm3Pack16    = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        G10X6B10X6R10X63Plane444UNorm3Pack16    = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        R12X4UNormPack16                        = VK_FORMAT_R12X4_UNORM_PACK16,
        R12X4G12X4UNorm2Pack16                  = VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
        R12X4G12X4B12X4A12X4UNorm4Pack16        = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        G12X4B12X4G12X4R12X4422UNorm4Pack16     = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        B12X4G12X4R12X4G12X4422UNorm4Pack16     = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        G12X4B12X4R12X43Plane420UNorm3Pack16    = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        G12X4B12X4R12X42Plane420UNorm3Pack16    = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        G12X4B12X4R12X43Plane422UNorm3Pack16    = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        G12X4B12X4R12X42Plane422UNorm3Pack16    = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        G12X4B12X4R12X43Plane444UNorm3Pack16    = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        G16B16G16R16422UNorm                    = VK_FORMAT_G16B16G16R16_422_UNORM,
        B16G16R16G16422UNorm                    = VK_FORMAT_B16G16R16G16_422_UNORM,
        G16B16R163Plane420UNorm                 = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
        G16B16R162Plane420UNorm                 = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
        G16B16R163Plane422UNorm                 = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
        G16B16R162Plane422UNorm                 = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
        G16B16R163Plane444UNorm                 = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
        PVRTC12BppUNormBlockImg                 = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
        PVRTC14BppUNormBlockImg                 = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
        PVRTC22BppUNormBlockImg                 = VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
        PVRTC24BppUNormBlockImg                 = VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
        PVRTC12BppSRGBBlockImg                  = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
        PVRTC14BppSRGBBlockImg                  = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
        PVRTC22BppSRGBBlockImg                  = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
        PVRTC24BppSRGBBlockImg                  = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
        ASTC4x4SFloatBlockExt                   = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,
        ASTC5x4SFloatBlockExt                   = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,
        ASTC5x5SFloatBlockExt                   = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,
        ASTC6x5SFloatBlockExt                   = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,
        ASTC6x6SFloatBlockExt                   = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,
        ASTC8x5SFloatBlockExt                   = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,
        ASTC8x6SFloatBlockExt                   = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,
        ASTC8x8SFloatBlockExt                   = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,
        ASTC10x5SFloatBlockExt                  = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,
        ASTC10x6SFloatBlockExt                  = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,
        ASTC10x8SFloatBlockExt                  = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,
        ASTC10x10SFloatBlockExt                 = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT,
        ASTC12x10SFloatBlockExt                 = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT,
        ASTC12x12SFloatBlockExt                 = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT,
        G8B8R82Plane444UNormExt                 = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT,
        G10X6B10X6R10X62Plane444UNorm3Pack16Ext = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT,
        G12X4B12X4R12X42Plane444UNorm3Pack16Ext = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT,
        G16B16R162Plane444UNormExt              = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT,
        A4R4G4B4UNormPack16Ext                  = VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT,
        A4B4G4R4UNormPack16Ext                  = VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT,
        G8B8G8R8422UNormKHR                     = VK_FORMAT_G8B8G8R8_422_UNORM_KHR,
        B8G8R8G8422UNormKHR                     = VK_FORMAT_B8G8R8G8_422_UNORM_KHR,
        G8B8R83Plane420UNormKHR                 = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR,
        G8B8R82Plane420UNormKHR                 = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
        G8B8R83Plane422UNormKHR                 = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR,
        G8B8R82Plane422UNormKHR                 = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR,
        G8B8R83Plane444UNormKHR                 = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR,
        R10X6UNormPack16KHR                     = VK_FORMAT_R10X6_UNORM_PACK16_KHR,
        R10X6G10X6UNorm2Pack16KHR               = VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR,
        R10X6G10X6B10X6A10X6UNorm4Pack16KHR     = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR,
        G10X6B10X6G10X6R10X6422UNorm4Pack16KHR  = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR,
        B10X6G10X6R10X6G10X6422UNorm4Pack16KHR  = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR,
        G10X6B10X6R10X63Plane420UNorm3Pack16KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR,
        G10X6B10X6R10X62Plane420UNorm3Pack16KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR,
        G10X6B10X6R10X63Plane422UNorm3Pack16KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR,
        G10X6B10X6R10X62Plane422UNorm3Pack16KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR,
        G10X6B10X6R10X63Plane444UNorm3Pack16KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR,
        R12X4UNormPack16KHR                     = VK_FORMAT_R12X4_UNORM_PACK16_KHR,
        R12X4G12X4UNorm2Pack16KHR               = VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR,
        R12X4G12X4B12X4A12X4UNorm4Pack16KHR     = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR,
        G12X4B12X4G12X4R12X4422UNorm4Pack16KHR  = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR,
        B12X4G12X4R12X4G12X4422UNorm4Pack16KHR  = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR,
        G12X4B12X4R12X43Plane420UNorm3Pack16KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR,
        G12X4B12X4R12X42Plane420UNorm3Pack16KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR,
        G12X4B12X4R12X43Plane422UNorm3Pack16KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR,
        G12X4B12X4R12X42Plane422UNorm3Pack16KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR,
        G12X4B12X4R12X43Plane444UNorm3Pack16KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR,
        G16B16G16R16422UNormKHR                 = VK_FORMAT_G16B16G16R16_422_UNORM_KHR,
        B16G16R16G16422UNormKHR                 = VK_FORMAT_B16G16R16G16_422_UNORM_KHR,
        G16B16R163Plane420UNormKHR              = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR,
        G16B16R162Plane420UNormKHR              = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR,
        G16B16R163Plane422UNormKHR              = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR,
        G16B16R162Plane422UNormKHR              = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR,
        G16B16R163Plane444UNormKHR              = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(Format, VkFormat)

    enum class FormatFeature
    {
        SampledImage             = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
        StorageImage             = VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
        StorageImageAtomic       = VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT,
        UniformTexelBuffer       = VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT,
        StorageTexelBuffer       = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT,
        StorageTexelBufferAtomic = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT,
        VertexBuffer             = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,
        ColorAttachment          = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
        ColorAttachmentBlend     = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,
        DepthStencilAttachment   = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        BlitSrc                  = VK_FORMAT_FEATURE_BLIT_SRC_BIT,
        BlitDst                  = VK_FORMAT_FEATURE_BLIT_DST_BIT,
        SampledImageFilterLinear = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
        // Provided by VK_VERSION_1_1
        TransferSrc = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
        // Provided by VK_VERSION_1_1
        TransferDst = VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
        // Provided by VK_VERSION_1_1
        MidpointChromaSamples = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(FormatFeature, VkFormatFeatureFlagBits)
    VZT_DEFINE_BITWISE_FUNCTIONS(FormatFeature)

    enum class BufferUsage : uint32_t
    {
        None               = 0,
        TransferSrc        = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        TransferDst        = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        UniformTexelBuffer = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
        StorageTexelBuffer = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
        UniformBuffer      = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        StorageBuffer      = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        IndexBuffer        = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VertexBuffer       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        IndirectBuffer     = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,

        // Provided by VK_VERSION_1_2
        ShaderDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,

        // Provided by VK_EXT_transform_feedback
        TransformFeedback = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
        // Provided by VK_EXT_transform_feedback
        TransformFeedbackCounter = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
        // Provided by VK_EXT_conditional_rendering
        ConditionalRendering = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureBuildInputReadOnly = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureStorage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
        // Provided by VK_KHR_ray_tracing_pipeline
        ShaderBindingTable = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(BufferUsage)
    VZT_DEFINE_TO_VULKAN_FUNCTION(BufferUsage, VkBufferUsageFlagBits)

    enum class DescriptorType
    {
        Sampler               = VK_DESCRIPTOR_TYPE_SAMPLER,
        CombinedSampler       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        SampledImage          = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        StorageImage          = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        UniformTexelBuffer    = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        StorageTexelBuffer    = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        UniformBuffer         = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        StorageBuffer         = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        UniformBufferDynamic  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        StorageBufferDynamic  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        InputAttachment       = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        InlineUniformBlock    = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
        AccelerationStructure = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        None                  = VK_DESCRIPTOR_TYPE_MAX_ENUM
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(DescriptorType, VkDescriptorType)

    enum class GeometryType : uint8_t
    {
        Triangles = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        AABBs     = VK_GEOMETRY_TYPE_AABBS_KHR,
        Instances = VK_GEOMETRY_TYPE_INSTANCES_KHR
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(GeometryType, VkGeometryTypeKHR)

    enum class GeometryFlag : uint8_t
    {
        Opaque                      = VK_GEOMETRY_OPAQUE_BIT_KHR,
        NoDuplicateAnyHitInvocation = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(GeometryFlag, VkGeometryFlagsKHR)
    VZT_DEFINE_BITWISE_FUNCTIONS(GeometryFlag)

    enum class BuildAccelerationStructureFlag : uint32_t
    {
        AllowUpdate     = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
        AllowCompaction = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR,
        PreferFastTrace = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        PreferFastBuild = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR,
        LowMemory       = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(BuildAccelerationStructureFlag, VkBuildAccelerationStructureFlagsKHR)
    VZT_DEFINE_BITWISE_FUNCTIONS(BuildAccelerationStructureFlag)

    enum class GeometryInstanceFlag : uint8_t
    {
        TriangleFacingCullDisable = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
        TriangleFlibFacing        = VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR,
        ForceOpaque               = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR,
        ForceNoOpaque             = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR,

        // Provided by VK_EXT_opacity_micromap
        ForceOpacityMicroMap2State = VK_GEOMETRY_INSTANCE_FORCE_OPACITY_MICROMAP_2_STATE_EXT,

        // Provided by VK_EXT_opacity_micromap
        DisableOpacityMicromaps       = VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_EXT,
        TriangleFrontCounterClockwise = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(GeometryInstanceFlag, VkGeometryInstanceFlagsKHR)
    VZT_DEFINE_BITWISE_FUNCTIONS(GeometryInstanceFlag)

    enum class ImageAspect : uint32_t
    {
        Color           = VK_IMAGE_ASPECT_COLOR_BIT,
        Depth           = VK_IMAGE_ASPECT_DEPTH_BIT,
        Stencil         = VK_IMAGE_ASPECT_STENCIL_BIT,
        MetaData        = VK_IMAGE_ASPECT_METADATA_BIT,
        Plane0          = VK_IMAGE_ASPECT_PLANE_0_BIT,
        Plane1          = VK_IMAGE_ASPECT_PLANE_1_BIT,
        Plane2          = VK_IMAGE_ASPECT_PLANE_2_BIT,
        MemoryPlane0Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
        MemoryPlane1Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
        MemoryPlane2Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
        MemoryPlane3Ext = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
        Plane0KHR       = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR,
        Plane1KHR       = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR,
        Plane2KHR       = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageAspect, VkImageAspectFlags)
    VZT_DEFINE_BITWISE_FUNCTIONS(ImageAspect)

    enum class ImageLayout : uint32_t
    {
        Undefined                                = VK_IMAGE_LAYOUT_UNDEFINED,
        General                                  = VK_IMAGE_LAYOUT_GENERAL,
        ColorAttachmentOptimal                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        DepthStencilAttachmentOptimal            = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        DepthStencilReadOnlyOptimal              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        ShaderReadOnlyOptimal                    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        TransferSrcOptimal                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        TransferDstOptimal                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        Preinitialized                           = VK_IMAGE_LAYOUT_PREINITIALIZED,
        DepthReadOnlyStencilAttachmentOptimal    = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        DepthAttachmentStencilReadOnlyOptimal    = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        DepthAttachmentOptimal                   = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        DepthReadOnlyOptimal                     = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        StencilAttachmentOptimal                 = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        StencilReadOnlyOptimal                   = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        PresentSrcKHR                            = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        SharedPresentKHR                         = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
        FragmentDensityMapOptimalExt             = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
        FragmentShadingRateAttachmentOptimalKHR  = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
        ReadOnlyOptimal                          = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        AttachmentOptimal                        = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        DepthReadOnlyStencilAttachmentOptimalKHR = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        DepthAttachmentStencilReadOnlyOptimalKHR = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        ShadingRateOptimal                       = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
        DepthAttachmentOptimalKHR                = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        DepthReadOnlyOptimalKHR                  = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        StencilAttachmentOptimalKHR              = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        StencilReadOnlyOptimalKHR                = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageLayout, VkImageLayout)

    enum class ImageUsage : uint32_t
    {
        TransferSrc                   = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        TransferDst                   = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        Sampled                       = VK_IMAGE_USAGE_SAMPLED_BIT,
        Storage                       = VK_IMAGE_USAGE_STORAGE_BIT,
        ColorAttachment               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        DepthStencilAttachment        = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        TransientAttachment           = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        InputAttachment               = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        FragmentDensityMap            = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
        FragmentShadingRateAttachment = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
        InvocationMask                = VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI,
        ShadingRateImage              = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageUsage, VkImageUsageFlags)
    VZT_DEFINE_BITWISE_FUNCTIONS(ImageUsage)

    enum class ImageTiling
    {
        Optimal = VK_IMAGE_TILING_OPTIMAL,
        Linear  = VK_IMAGE_TILING_LINEAR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageTiling, VkImageTiling)

    enum class ImageType
    {
        T1D = VK_IMAGE_TYPE_1D,
        T2D = VK_IMAGE_TYPE_2D,
        T3D = VK_IMAGE_TYPE_3D
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageType, VkImageType)

    enum class SampleCount : uint32_t
    {
        Sample1  = VK_SAMPLE_COUNT_1_BIT,
        Sample2  = VK_SAMPLE_COUNT_2_BIT,
        Sample4  = VK_SAMPLE_COUNT_4_BIT,
        Sample8  = VK_SAMPLE_COUNT_8_BIT,
        Sample16 = VK_SAMPLE_COUNT_16_BIT,
        Sample32 = VK_SAMPLE_COUNT_32_BIT,
        Sample64 = VK_SAMPLE_COUNT_64_BIT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(SampleCount, VkSampleCountFlagBits)

    enum class SharingMode
    {
        Exclusive  = VK_SHARING_MODE_EXCLUSIVE,
        Concurrent = VK_SHARING_MODE_CONCURRENT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(SharingMode, VkSharingMode)

    enum class ShaderStage : uint32_t
    {
        All                    = VK_SHADER_STAGE_ALL,
        AllGraphics            = VK_SHADER_STAGE_ALL_GRAPHICS,
        Vertex                 = VK_SHADER_STAGE_VERTEX_BIT,
        TessellationControl    = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        TessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        Geometry               = VK_SHADER_STAGE_GEOMETRY_BIT,
        Fragment               = VK_SHADER_STAGE_FRAGMENT_BIT,
        Compute                = VK_SHADER_STAGE_COMPUTE_BIT,
        RayGen                 = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        AnyHit                 = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        ClosestHit             = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        Miss                   = VK_SHADER_STAGE_MISS_BIT_KHR,
        Intersection           = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        Callable               = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        Task                   = VK_SHADER_STAGE_TASK_BIT_NV,
        Mesh                   = VK_SHADER_STAGE_MESH_BIT_NV
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ShaderStage, VkShaderStageFlagBits);

    enum class ShaderGroupType : uint8_t
    {
        General            = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
        TrianglesHitGroup  = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
        ProceduralHitGroup = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ShaderGroupType, VkRayTracingShaderGroupTypeKHR);

    enum class QueryType : uint32_t
    {
        Occlusion          = VK_QUERY_TYPE_OCCLUSION,
        PipelineStatistics = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        Timestamp          = VK_QUERY_TYPE_TIMESTAMP,
        // Provided by VK_KHR_video_queue
        ResultStatusOnlyKHR = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR,
        // Provided by VK_EXT_transform_feedback
        TransformFeedbackStreamEXT = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT,
        // Provided by VK_KHR_performance_query
        PerformanceQueryKHR = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureCompactedSizeKHR = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
        // Provided by VK_KHR_acceleration_structure
        AccelerationStructureSerializationSizeKHR = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR,
        // Provided by VK_INTEL_performance_query
        PerformanceQueryIntel = VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL,
        // Provided by VK_EXT_mesh_shader
        MeshPrimitivesGeneratedEXT = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT,
        // Provided by VK_EXT_primitives_generated_query
        PrimitivesGeneratedEXT = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT,
        // Provided by VK_KHR_ray_tracing_maintenance1
        AccelerationStructureSerializationBottomLevelPointers =
            VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR,
        // Provided by VK_KHR_ray_tracing_maintenance1
        AccelerationStructureSizeKHR = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,
        // Provided by VK_EXT_opacity_micromap
        MicromapSerializationSizeEXT = VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT,
        // Provided by VK_EXT_opacity_micromap
        MicromapCompactedSizeEXT = VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(QueryType, VkQueryType)

    enum class QueryResultFlag : uint32_t
    {
        N64              = VK_QUERY_RESULT_64_BIT,
        Wait             = VK_QUERY_RESULT_WAIT_BIT,
        WithAvailability = VK_QUERY_RESULT_WITH_AVAILABILITY_BIT,
        Partial          = VK_QUERY_RESULT_PARTIAL_BIT,
        // Provided by VK_KHR_video_queue
        WithStatusKHR = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(QueryResultFlag, VkQueryResultFlagBits)
    VZT_DEFINE_BITWISE_FUNCTIONS(QueryResultFlag)

    enum class AttachmentAccess : uint32_t
    {
        IndirectCommandRead               = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        IndexRead                         = VK_ACCESS_INDEX_READ_BIT,
        VertexAttributeRead               = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        UniformRead                       = VK_ACCESS_UNIFORM_READ_BIT,
        InputAttachmentRead               = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        ShaderRead                        = VK_ACCESS_SHADER_READ_BIT,
        ShaderWrite                       = VK_ACCESS_SHADER_WRITE_BIT,
        ColorAttachmentRead               = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        ColorAttachmentWrite              = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        DepthStencilAttachmentRead        = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        DepthStencilAttachmentWrite       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        TransferRead                      = VK_ACCESS_TRANSFER_READ_BIT,
        TransferWrite                     = VK_ACCESS_TRANSFER_WRITE_BIT,
        HostRead                          = VK_ACCESS_HOST_READ_BIT,
        HostWrite                         = VK_ACCESS_HOST_WRITE_BIT,
        MemoryRead                        = VK_ACCESS_MEMORY_READ_BIT,
        MemoryWrite                       = VK_ACCESS_MEMORY_WRITE_BIT,
        None                              = VK_ACCESS_NONE,
        TransformFeedbackWrite            = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
        TransformFeedbackCounterRead      = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
        TransformFeedbackCounterWrite     = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
        ConditionalRenderingRead          = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
        ColorAttachmentReadNonCoherent    = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
        AccelerationStructureRead         = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        AccelerationStructureWrite        = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        FragmentDensityMapRead            = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
        FragmentShadingRateAttachmentRead = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(AttachmentAccess)
    VZT_DEFINE_TO_VULKAN_FUNCTION(AttachmentAccess, VkAccessFlags)

    enum class PipelineBindPoint : uint32_t
    {
        Graphics      = VK_PIPELINE_BIND_POINT_GRAPHICS,
        Compute       = VK_PIPELINE_BIND_POINT_COMPUTE,
        RaytracingKHR = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(PipelineBindPoint, VkPipelineBindPoint)

    enum class DependencyFlag : uint32_t
    {
        None        = 0,
        ByRegion    = VK_DEPENDENCY_BY_REGION_BIT,
        DeviceGroup = VK_DEPENDENCY_DEVICE_GROUP_BIT,
        ViewLocal   = VK_DEPENDENCY_VIEW_LOCAL_BIT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(DependencyFlag, VkDependencyFlags)

    enum class LoadOp : uint32_t
    {
        Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
        Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
        DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(LoadOp, VkAttachmentLoadOp)

    enum class StoreOp : uint32_t
    {
        Store    = VK_ATTACHMENT_STORE_OP_STORE,
        DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        NoneQCOM = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
        NoneExt  = VK_ATTACHMENT_STORE_OP_NONE_EXT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(StoreOp, VkAttachmentStoreOp)

    enum class PipelineStage : uint32_t
    {
        TopOfPipe                    = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        DrawIndirect                 = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        VertexInput                  = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VertexShader                 = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        TessellationControlShader    = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
        TessellationEvaluationShader = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
        GeometryShader               = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
        FragmentShader               = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        EarlyFragmentTests           = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        LateFragmentTests            = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        ColorAttachmentOutput        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        ComputeShader                = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Transfer                     = VK_PIPELINE_STAGE_TRANSFER_BIT,
        BottomOfPipe                 = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        Host                         = VK_PIPELINE_STAGE_HOST_BIT,
        AllGraphic                   = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
        AllCommands                  = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        TransformFeedback            = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
        ConditionRendering           = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
        AccelerationStructureBuild   = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        RaytracingShader             = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
        TaskShaderNV                 = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
        MeshShaderNV                 = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
        None                         = VK_PIPELINE_STAGE_NONE_KHR,
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(PipelineStage)
    VZT_DEFINE_TO_VULKAN_FUNCTION(PipelineStage, VkPipelineStageFlagBits)

    enum class Dependency : uint8_t
    {
        None        = 0,
        ByRegion    = VK_DEPENDENCY_BY_REGION_BIT,
        DeviceGroup = VK_DEPENDENCY_DEVICE_GROUP_BIT,
        ViewLocal   = VK_DEPENDENCY_VIEW_LOCAL_BIT,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(Dependency, VkDependencyFlags)

    enum class Access : uint32_t
    {
        IndirectCommandRead               = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        IndexRead                         = VK_ACCESS_INDEX_READ_BIT,
        VertexAttributeRead               = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        UniformRead                       = VK_ACCESS_UNIFORM_READ_BIT,
        InputAttachmentRead               = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        ShaderRead                        = VK_ACCESS_SHADER_READ_BIT,
        ShaderWrite                       = VK_ACCESS_SHADER_WRITE_BIT,
        ColorAttachmentRead               = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        ColorAttachmentWrite              = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        DepthStencilAttachmentRead        = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        DepthStencilAttachmentWrite       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        TransferRead                      = VK_ACCESS_TRANSFER_READ_BIT,
        TransferWrite                     = VK_ACCESS_TRANSFER_WRITE_BIT,
        HostRead                          = VK_ACCESS_HOST_READ_BIT,
        HostWrite                         = VK_ACCESS_HOST_WRITE_BIT,
        MemoryRead                        = VK_ACCESS_MEMORY_READ_BIT,
        MemoryWrite                       = VK_ACCESS_MEMORY_WRITE_BIT,
        None                              = VK_ACCESS_NONE,
        TransformFeedbackWrite            = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
        TransformFeedbackCounterRead      = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
        TransformFeedbackCounterWrite     = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
        ConditionalRenderingRead          = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
        ColorAttachmentReadNonCoherent    = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
        AccelerationStructureRead         = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        AccelerationStructureWrite        = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        FragmentDensityMapRead            = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
        FragmentShadingRateAttachmentRead = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(Access)
    VZT_DEFINE_TO_VULKAN_FUNCTION(Access, VkAccessFlags)

    enum class QueueType : uint8_t
    {
        None     = 0,
        Graphics = VK_QUEUE_GRAPHICS_BIT,
        Compute  = VK_QUEUE_COMPUTE_BIT,
        Transfer = VK_QUEUE_TRANSFER_BIT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(QueueType, VkQueueFlagBits)
    VZT_DEFINE_BITWISE_FUNCTIONS(QueueType)

    enum class Filter : uint32_t
    {
        Nearest  = VK_FILTER_NEAREST,
        Linear   = VK_FILTER_LINEAR,
        CubicImg = VK_FILTER_CUBIC_IMG,
        CubicExt = VK_FILTER_CUBIC_EXT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(Filter, VkFilter)

    enum class AddressMode : uint32_t
    {
        Repeat               = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        MirroredRepeat       = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        ClampToEdge          = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        ClampToBorder        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        MirrorClampToEdge    = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
        MirrorClampToEdgeKHR = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(AddressMode, VkSamplerAddressMode)

    enum class MipmapMode : uint32_t
    {
        Nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        Linear  = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(MipmapMode, VkSamplerMipmapMode)

    enum class BorderColor : uint32_t
    {
        FloatTransparentBlack = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        IntTransparentBlack   = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
        FloatOpaqueBlack      = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        IntOpaqueBlack        = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        FloatOpaqueWhite      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        IntOpaqueWhite        = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
        FloatCustomExt        = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT,
        IntCustomExt          = VK_BORDER_COLOR_INT_CUSTOM_EXT
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(BorderColor, VkBorderColor)

    enum class Rendering
    {
        None                            = 0,
        ContextSecondaryCommandBuffers  = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT,
        Suspending                      = VK_RENDERING_SUSPENDING_BIT,
        Resuming                        = VK_RENDERING_RESUMING_BIT,
        EnableLegacyDithering           = VK_RENDERING_ENABLE_LEGACY_DITHERING_BIT_EXT,
        ContentsInline                  = VK_RENDERING_CONTENTS_INLINE_BIT_KHR,
        ContentsSecondaryCommandBuffers = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR,
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(Rendering)
    VZT_DEFINE_TO_VULKAN_FUNCTION(Rendering, VkRenderingFlags)

    enum class DescriptorPoolCreateFlag
    {
        None                    = 0,
        FreeDescriptorSet       = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        UpdateAfterBind         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        HostOnly                = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT,
        AllowOverAllocationSets = VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV,
    };
    VZT_DEFINE_BITWISE_FUNCTIONS(DescriptorPoolCreateFlag)
    VZT_DEFINE_TO_VULKAN_FUNCTION(DescriptorPoolCreateFlag, VkDescriptorPoolCreateFlagBits)
} // namespace vzt

#endif // VZT_VULKAN_TYPE_HPP
