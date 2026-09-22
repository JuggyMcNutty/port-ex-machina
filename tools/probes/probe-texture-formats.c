/* Does this GPU support the texture formats SurrealEngine uploads with?
 *
 * TextureUploader::GetUploader hardcodes a VK_FORMAT per TextureFormat and
 * nothing in the engine queries vkGetPhysicalDeviceFormatProperties, so a GPU
 * without desktop BCn support would still get images created and sampled in
 * those formats. This prints optimalTilingFeatures for every format the
 * uploaders can pick, sampled-image and filter-linear bits included.
 *
 * Build and run: see docs/DESIGN.md "Device probes" (same recipe as
 * probe-vulkan-caps.c).
 */
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct { const char *name; VkFormat fmt; } formats[] = {
    { "R8G8B8A8_UNORM (null/dither)", VK_FORMAT_R8G8B8A8_UNORM },
    { "R5G6B5_UNORM_PACK16", VK_FORMAT_R5G6B5_UNORM_PACK16 },
    { "BC1_RGBA_UNORM_BLOCK", VK_FORMAT_BC1_RGBA_UNORM_BLOCK },
    { "R8G8B8_UNORM", VK_FORMAT_R8G8B8_UNORM },
    { "B8G8R8A8_UNORM", VK_FORMAT_B8G8R8A8_UNORM },
    { "BC2_UNORM_BLOCK", VK_FORMAT_BC2_UNORM_BLOCK },
    { "BC3_UNORM_BLOCK", VK_FORMAT_BC3_UNORM_BLOCK },
    { "BC4_UNORM_BLOCK", VK_FORMAT_BC4_UNORM_BLOCK },
    { "BC4_SNORM_BLOCK", VK_FORMAT_BC4_SNORM_BLOCK },
    { "BC5_UNORM_BLOCK", VK_FORMAT_BC5_UNORM_BLOCK },
    { "BC5_SNORM_BLOCK", VK_FORMAT_BC5_SNORM_BLOCK },
    { "BC7_UNORM_BLOCK", VK_FORMAT_BC7_UNORM_BLOCK },
    { "BC6H_SFLOAT_BLOCK", VK_FORMAT_BC6H_SFLOAT_BLOCK },
    { "BC6H_UFLOAT_BLOCK", VK_FORMAT_BC6H_UFLOAT_BLOCK },
    { "R16G16B16A16_UNORM", VK_FORMAT_R16G16B16A16_UNORM },
    { "R16G16B16A16_SNORM", VK_FORMAT_R16G16B16A16_SNORM },
    { "R8_UNORM", VK_FORMAT_R8_UNORM },
    { "R8_SNORM", VK_FORMAT_R8_SNORM },
    { "R16_UNORM", VK_FORMAT_R16_UNORM },
    { "R16_SNORM", VK_FORMAT_R16_SNORM },
    { "R8G8_UNORM", VK_FORMAT_R8G8_UNORM },
    { "R8G8_SNORM", VK_FORMAT_R8G8_SNORM },
    { "R16G16_UNORM", VK_FORMAT_R16G16_UNORM },
    { "R16G16_SNORM", VK_FORMAT_R16G16_SNORM },
    { "A2R10G10B10_UNORM_PACK32", VK_FORMAT_A2R10G10B10_UNORM_PACK32 },
    { "R32G32B32A32_SFLOAT (lightmap/fog)", VK_FORMAT_R32G32B32A32_SFLOAT },
    { "R16G16B16A16_SFLOAT", VK_FORMAT_R16G16B16A16_SFLOAT },
    { "B5G5R5A1_UNORM_PACK16", VK_FORMAT_B5G5R5A1_UNORM_PACK16 },
};

int main(void) {
    VkApplicationInfo ai = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .apiVersion = VK_MAKE_VERSION(1, 2, 0) };
    VkInstanceCreateInfo ci = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                .pApplicationInfo = &ai };
    VkInstance inst;
    if (vkCreateInstance(&ci, NULL, &inst) != VK_SUCCESS) {
        printf("vkCreateInstance failed\n");
        return 1;
    }
    uint32_t n = 0;
    vkEnumeratePhysicalDevices(inst, &n, NULL);
    if (!n) { printf("no physical devices\n"); return 2; }
    VkPhysicalDevice *devs = calloc(n, sizeof *devs);
    vkEnumeratePhysicalDevices(inst, &n, devs);

    for (uint32_t d = 0; d < n; d++) {
        VkPhysicalDeviceProperties p;
        vkGetPhysicalDeviceProperties(devs[d], &p);
        printf("== %s (api %u.%u.%u) ==\n", p.deviceName,
               VK_API_VERSION_MAJOR(p.apiVersion), VK_API_VERSION_MINOR(p.apiVersion),
               VK_API_VERSION_PATCH(p.apiVersion));
        for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
            VkFormatProperties fp;
            vkGetPhysicalDeviceFormatProperties(devs[d], formats[i].fmt, &fp);
            int sampled = (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
            int linfilter = (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
            printf("  %-38s %s linfilter=%d (optimal=0x%x)\n", formats[i].name,
                   sampled ? "sampled" : "UNSUPPORTED", linfilter, fp.optimalTilingFeatures);
        }
    }
    free(devs);
    return 0;
}
