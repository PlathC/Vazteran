#ifndef VZT_VULKAN_IMAGE_HPP
#define VZT_VULKAN_IMAGE_HPP

#include <vk_mem_alloc.h>

#include "vzt/Core/Math.hpp"
#include "vzt/Core/Meta.hpp"
#include "vzt/Core/Type.hpp"
#include "vzt/Core/Vulkan.hpp"

namespace vzt
{
    class Device;

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

    struct ImageBuilder
    {
        Extent3D    size;
        ImageUsage  usage;
        Format      format;
        uint32_t    mipLevels   = 1;
        ImageLayout layout      = ImageLayout::Undefined;
        SampleCount sampleCount = SampleCount::Sample1;
        ImageType   type        = ImageType::T2D;
        SharingMode sharingMode = SharingMode::Concurrent;
    };

    class Image
    {
      public:
        Image(View<Device> device, Extent3D size, ImageUsage usage, Format format, uint32_t mipLevels = 1,
              ImageLayout layout = ImageLayout::Undefined, SampleCount sampleCount = SampleCount::Sample1,
              ImageType type = ImageType::T2D, SharingMode sharingMode = SharingMode::Concurrent);
        Image(View<Device> device, ImageBuilder builder);
        Image(View<Device> device, VkImage image, Extent3D size, Format format);

        Image(const Image&)            = delete;
        Image& operator=(const Image&) = delete;

        Image(Image&&) noexcept;
        Image& operator=(Image&&) noexcept;

        ~Image();

        inline Extent3D    getSize() const;
        inline ImageUsage  getUsage() const;
        inline Format      getFormat() const;
        inline uint32_t    getMipLevels() const;
        inline ImageLayout getLayout() const;
        inline SampleCount getSampleCount() const;
        inline ImageType   getImageType() const;
        inline SharingMode getSharingMode() const;
        inline VkImage     getHandle() const;

      private:
        View<Device>  m_device     = {};
        VkImage       m_handle     = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;

        Extent3D    m_size;
        ImageUsage  m_usage;
        Format      m_format;
        uint32_t    m_mipLevels;
        ImageLayout m_layout;
        SampleCount m_sampleCount;
        ImageType   m_type;
        SharingMode m_sharingMode;
    };

    enum class ImageViewType
    {
        T1D       = VK_IMAGE_VIEW_TYPE_1D,
        T2D       = VK_IMAGE_VIEW_TYPE_2D,
        T3D       = VK_IMAGE_VIEW_TYPE_3D,
        Cube      = VK_IMAGE_VIEW_TYPE_CUBE,
        T1DArray  = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
        T2DArray  = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        CubeArray = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
    };
    VZT_DEFINE_TO_VULKAN_FUNCTION(ImageViewType, VkImageViewType)

    class ImageView
    {
      public:
        ImageView(View<Device> device, View<Image> image, ImageViewType type, ImageAspect aspect, Format format,
                  uint32_t baseMipLevel, uint32_t levelCount);
        ImageView(View<Device> device, View<Image> image, ImageAspect aspect, ImageViewType type);
        ImageView(View<Device> device, View<Image> image, ImageAspect aspect);

        ImageView(const ImageView&)            = delete;
        ImageView& operator=(const ImageView&) = delete;

        ImageView(ImageView&&) noexcept;
        ImageView& operator=(ImageView&&) noexcept;

        ~ImageView();

        inline ImageAspect getAspect() const;
        inline Format      getFormat() const;
        inline View<Image> getImage() const;
        inline VkImageView getHandle() const;

      private:
        View<Device> m_device;
        View<Image>  m_image;
        VkImageView  m_handle = VK_NULL_HANDLE;

        ImageAspect m_aspect;
        Format      m_format;
    };
} // namespace vzt

#include "vzt/Vulkan/Image.inl"

#endif // VZT_VULKAN_IMAGE_HPP
