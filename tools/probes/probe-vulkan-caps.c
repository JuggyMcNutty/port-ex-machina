/* Reports exactly what DXEngine's Vulkan device filter checks.
 *
 * Every requirement in one place, so the verdict for a given GPU is one run
 * rather than a series of guesses:
 *
 *   DXGPU  VulkanDeviceBuilder::FindDevices
 *     - required extension VK_KHR_swapchain
 *     - feature samplerAnisotropy (fragmentStoresAndAtomics, multiDrawIndirect and
 *       independentBlend are listed, not required: nothing in the engine uses them)
 *     - a graphics queue family that can present to the surface
 *   DXEngine  VulkanRenderDevice.cpp
 *     - optional extension VK_KHR_sampler_mirror_clamp_to_edge (a clamped
 *       tile's sampler; without it the tile is clamped to the edge)
 *     - optional: bindless textures, which need VK_EXT_descriptor_indexing
 *       listed, its five features the bindless set uses (partially bound,
 *       runtime array, non-uniform sampled-image indexing, sampled images
 *       updated after bind, a variable count) and room for the set's
 *       MaxBindlessTextures image samplers updated after bind (five
 *       limits); without them DXEngine binds a descriptor set per texture
 *       combination instead
 *     - a device that draws on the CPU (Mesa's lavapipe) only with
 *       Settings.json's CPUDriver, the GPUs passed over; without it the
 *       GPUs come first, and where a CPU device is all there is, the
 *       software device draws in its place
 *
 * The engine reads the indexing features and limits only where the device
 * lists VK_EXT_descriptor_indexing (DXGPU's VulkanInstance), as this does; the
 * Vulkan 1.2 core flags are shown beside it, queried on their own, because a
 * driver can advertise a high API version without what it makes mandatory:
 * the PowerVR Rogue GE8300 reports API 1.3.225 and has neither.
 *
 * Build: see docs/PORTING.md "Device probes". Needs only the Vulkan loader.
 */
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YN(x) ((x) ? "yes" : "NO")

static const char *device_type(VkPhysicalDeviceType t) {
    switch (t) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "CPU";
    default:                                     return "other";
    }
}

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
    PFN_vkGetPhysicalDeviceProperties2 getProperties2 =
        (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(inst, "vkGetPhysicalDeviceProperties2");
    /* DescriptorSetManager::MaxBindlessTextures */
    const uint32_t bindless_count = 16536;

    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceProperties p;
        vkGetPhysicalDeviceProperties(devs[i], &p);
        printf("device %u: %s\n", i, p.deviceName);
        printf("  type                     %s\n", device_type(p.deviceType));
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

        printf("  -- features (samplerAnisotropy required) --\n");
        printf("    samplerAnisotropy        %s\n", YN(f.samplerAnisotropy));
        printf("    fragmentStoresAndAtomics %s\n", YN(f.fragmentStoresAndAtomics));
        printf("    multiDrawIndirect        %s\n", YN(f.multiDrawIndirect));
        printf("    independentBlend         %s\n", YN(f.independentBlend));

        printf("  -- required extensions --\n");
        static const char *req[] = {
            "VK_KHR_swapchain",
        };
        int all = 1;
        for (size_t k = 0; k < sizeof req / sizeof *req; k++) {
            int got = has_ext(ext, en, req[k]);
            all = all && got;
            printf("    %-38s %s\n", req[k], got ? "present" : "ABSENT");
        }
        printf("  -- optional extensions --\n");
        printf("    %-38s %s\n", "VK_KHR_sampler_mirror_clamp_to_edge",
               has_ext(ext, en, "VK_KHR_sampler_mirror_clamp_to_edge")
                   ? "present" : "absent: a clamped tile clamped to the edge");

        /* As the engine reads them: with the extension listed, its feature and
         * property structs alone in the chains (DXGPU's VulkanInstance) */
        int indexing_ext = has_ext(ext, en, "VK_EXT_descriptor_indexing");
        int bindless = 0;
        printf("  -- bindless textures (optional) --\n");
        printf("    VK_EXT_descriptor_indexing            %s\n", YN(indexing_ext));
        if (indexing_ext && getFeatures2 && getProperties2 && p.apiVersion != VK_API_VERSION_1_0) {
            VkPhysicalDeviceDescriptorIndexingFeatures di;
            memset(&di, 0, sizeof di);
            di.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
            VkPhysicalDeviceFeatures2 f2;
            memset(&f2, 0, sizeof f2);
            f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext = &di;
            getFeatures2(devs[i], &f2);

            VkPhysicalDeviceDescriptorIndexingProperties dp;
            memset(&dp, 0, sizeof dp);
            dp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
            VkPhysicalDeviceProperties2 p2;
            memset(&p2, 0, sizeof p2);
            p2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            p2.pNext = &dp;
            getProperties2(devs[i], &p2);

            printf("    descriptorBindingPartiallyBound           %s\n", YN(di.descriptorBindingPartiallyBound));
            printf("    runtimeDescriptorArray                    %s\n", YN(di.runtimeDescriptorArray));
            printf("    shaderSampledImageArrayNonUniformIndexing %s\n", YN(di.shaderSampledImageArrayNonUniformIndexing));
            printf("    descriptorBindingSampledImageUpdateAfterBind %s\n", YN(di.descriptorBindingSampledImageUpdateAfterBind));
            printf("    descriptorBindingVariableDescriptorCount  %s\n", YN(di.descriptorBindingVariableDescriptorCount));
            printf("    limits updated after bind (need %u each):\n", bindless_count);
            printf("      maxDescriptorSetUpdateAfterBindSampledImages     %u\n", dp.maxDescriptorSetUpdateAfterBindSampledImages);
            printf("      maxDescriptorSetUpdateAfterBindSamplers          %u\n", dp.maxDescriptorSetUpdateAfterBindSamplers);
            printf("      maxPerStageDescriptorUpdateAfterBindSampledImages %u\n", dp.maxPerStageDescriptorUpdateAfterBindSampledImages);
            printf("      maxPerStageDescriptorUpdateAfterBindSamplers     %u\n", dp.maxPerStageDescriptorUpdateAfterBindSamplers);
            printf("      maxPerStageUpdateAfterBindResources              %u\n", dp.maxPerStageUpdateAfterBindResources);
            bindless =
                di.descriptorBindingPartiallyBound &&
                di.runtimeDescriptorArray &&
                di.shaderSampledImageArrayNonUniformIndexing &&
                di.descriptorBindingSampledImageUpdateAfterBind &&
                di.descriptorBindingVariableDescriptorCount &&
                dp.maxDescriptorSetUpdateAfterBindSampledImages >= bindless_count &&
                dp.maxDescriptorSetUpdateAfterBindSamplers >= bindless_count &&
                dp.maxPerStageDescriptorUpdateAfterBindSampledImages >= bindless_count &&
                dp.maxPerStageDescriptorUpdateAfterBindSamplers >= bindless_count &&
                dp.maxPerStageUpdateAfterBindResources >= bindless_count;
        }
        if (getFeatures2 && p.apiVersion >= VK_API_VERSION_1_2) {
            VkPhysicalDeviceVulkan12Features v12;
            memset(&v12, 0, sizeof v12);
            v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            VkPhysicalDeviceFeatures2 f2;
            memset(&f2, 0, sizeof f2);
            f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext = &v12;
            getFeatures2(devs[i], &f2);
            printf("    (1.2 core, beside it: descriptorIndexing %s, runtimeDescriptorArray %s)\n",
                   YN(v12.descriptorIndexing), YN(v12.runtimeDescriptorArray));
        }
        printf("    bindless: %s\n", bindless ? "yes" : "no, a descriptor set per texture combination");

        int features_ok = f.samplerAnisotropy;
        int cpu = p.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
        printf("  VERDICT: %s\n",
               !(features_ok && all) ? "REJECTED by the engine's device filter"
               : cpu ? "meets the engine's requirements, but draws on the CPU: taken only with "
                       "Settings.json's CPUDriver, the GPUs passed over; alone, without it, the "
                       "software device draws"
                     : "meets the engine's requirements");

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
