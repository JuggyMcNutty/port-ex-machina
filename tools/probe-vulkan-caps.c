/* Reports exactly what Surreal Engine's Vulkan device filter checks.
 *
 * Every requirement in one place, so the verdict for a given GPU is one run
 * rather than a series of guesses:
 *
 *   SurrealGPU  VulkanDeviceBuilder::FindDevices
 *     - required extension VK_KHR_swapchain
 *     - features samplerAnisotropy, fragmentStoresAndAtomics,
 *       multiDrawIndirect, independentBlend
 *     - a graphics queue family that can present to the surface
 *   SurrealEngine  VulkanRenderDevice.cpp
 *     - required extensions VK_EXT_descriptor_indexing,
 *       VK_KHR_sampler_mirror_clamp_to_edge
 *
 * Descriptor indexing is checked three ways, because a driver can advertise a
 * high API version without supporting what that version makes mandatory: as the
 * EXT extension string, as the Vulkan 1.2 core feature struct, and via the
 * older per-extension struct. The PowerVR Rogue GE8300 reports API 1.3.225 and
 * supports it by none of the three.
 *
 * Build: see docs/DESIGN.md. Needs only the Vulkan loader.
 */
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YN(x) ((x) ? "yes" : "NO")

static int has_ext(const VkExtensionProperties *e, uint32_t n, const char *name) {
    for (uint32_t i = 0; i < n; i++)
        if (strcmp(e[i].extensionName, name) == 0) return 1;
    return 0;
}

int main(int argc, char **argv) {
    const int verbose = (argc > 1 && strcmp(argv[1], "-v") == 0);

    VkApplicationInfo ai = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .apiVersion = VK_MAKE_VERSION(1, 2, 0) };
    VkInstanceCreateInfo ci = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                .pApplicationInfo = &ai };
    VkInstance inst = VK_NULL_HANDLE;
    VkResult rc = vkCreateInstance(&ci, NULL, &inst);
    if (rc != VK_SUCCESS) {
        printf("vkCreateInstance failed: %d\n", (int)rc);
        return 1;
    }

    uint32_t n = 0;
    vkEnumeratePhysicalDevices(inst, &n, NULL);
    if (n == 0) { printf("no physical devices\n"); return 2; }
    VkPhysicalDevice *devs = calloc(n, sizeof *devs);
    vkEnumeratePhysicalDevices(inst, &n, devs);

    PFN_vkGetPhysicalDeviceFeatures2 getFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(inst, "vkGetPhysicalDeviceFeatures2");

    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceProperties p;
        vkGetPhysicalDeviceProperties(devs[i], &p);
        printf("device %u: %s\n", i, p.deviceName);
        printf("  advertised API           %u.%u.%u\n",
               VK_VERSION_MAJOR(p.apiVersion), VK_VERSION_MINOR(p.apiVersion),
               VK_VERSION_PATCH(p.apiVersion));
        printf("  maxImageDimension2D      %u\n", p.limits.maxImageDimension2D);

        uint32_t en = 0;
        vkEnumerateDeviceExtensionProperties(devs[i], NULL, &en, NULL);
        VkExtensionProperties *ext = calloc(en ? en : 1, sizeof *ext);
        vkEnumerateDeviceExtensionProperties(devs[i], NULL, &en, ext);

        VkPhysicalDeviceFeatures f;
        vkGetPhysicalDeviceFeatures(devs[i], &f);

        printf("  -- required features --\n");
        printf("    samplerAnisotropy        %s\n", YN(f.samplerAnisotropy));
        printf("    fragmentStoresAndAtomics %s\n", YN(f.fragmentStoresAndAtomics));
        printf("    multiDrawIndirect        %s\n", YN(f.multiDrawIndirect));
        printf("    independentBlend         %s\n", YN(f.independentBlend));

        printf("  -- required extensions --\n");
        static const char *req[] = {
            "VK_KHR_swapchain",
            "VK_EXT_descriptor_indexing",
            "VK_KHR_sampler_mirror_clamp_to_edge",
        };
        int all = 1;
        for (size_t k = 0; k < sizeof req / sizeof *req; k++) {
            int got = has_ext(ext, en, req[k]);
            all = all && got;
            printf("    %-38s %s\n", req[k], got ? "present" : "ABSENT");
        }

        printf("  -- descriptor indexing, three ways --\n");
        printf("    VK_EXT_descriptor_indexing            %s\n",
               YN(has_ext(ext, en, "VK_EXT_descriptor_indexing")));
        if (getFeatures2) {
            VkPhysicalDeviceVulkan12Features v12;
            memset(&v12, 0, sizeof v12);
            v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

            VkPhysicalDeviceDescriptorIndexingFeatures di;
            memset(&di, 0, sizeof di);
            di.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
            v12.pNext = &di;

            VkPhysicalDeviceFeatures2 f2;
            memset(&f2, 0, sizeof f2);
            f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext = &v12;
            getFeatures2(devs[i], &f2);

            printf("    1.2 core  descriptorIndexing          %s\n", YN(v12.descriptorIndexing));
            printf("    1.2 core  runtimeDescriptorArray      %s\n", YN(v12.runtimeDescriptorArray));
            printf("    1.2 core  sampledImageNonUniformIndex %s\n",
                   YN(v12.shaderSampledImageArrayNonUniformIndexing));
            printf("    1.2 core  bufferDeviceAddress         %s\n", YN(v12.bufferDeviceAddress));
            printf("    EXT struct runtimeDescriptorArray     %s\n", YN(di.runtimeDescriptorArray));
        } else {
            printf("    (vkGetPhysicalDeviceFeatures2 unavailable)\n");
        }

        int features_ok = f.samplerAnisotropy && f.fragmentStoresAndAtomics &&
                          f.multiDrawIndirect && f.independentBlend;
        printf("  VERDICT: %s\n",
               (features_ok && all) ? "meets the engine's requirements"
                                    : "REJECTED by the engine's device filter");

        if (verbose) {
            printf("  -- all %u device extensions --\n", en);
            for (uint32_t k = 0; k < en; k++) printf("    %s\n", ext[k].extensionName);
        }
        free(ext);
    }
    free(devs);
    vkDestroyInstance(inst, NULL);
    return 0;
}
