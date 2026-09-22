/* Which of Surreal Engine's required Vulkan features does this GPU have?
 *
 * SurrealGPU's VulkanDeviceBuilder::FindDevices rejects any device lacking
 * VK_KHR_swapchain or any of four core features. This reports each one so we
 * know whether a fork can relax the requirement or whether it is fundamental.
 */
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YN(x) ((x) ? "yes" : "NO")

int main(void) {
    VkApplicationInfo ai = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .apiVersion = VK_API_VERSION_1_0 };
    VkInstanceCreateInfo ci = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                .pApplicationInfo = &ai };
    VkInstance inst;
    if (vkCreateInstance(&ci, NULL, &inst) != VK_SUCCESS) {
        printf("vkCreateInstance failed\n");
        return 1;
    }
    uint32_t n = 0;
    vkEnumeratePhysicalDevices(inst, &n, NULL);
    VkPhysicalDevice *devs = calloc(n ? n : 1, sizeof *devs);
    vkEnumeratePhysicalDevices(inst, &n, devs);

    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceProperties p;
        vkGetPhysicalDeviceProperties(devs[i], &p);
        printf("device: %s (API %u.%u.%u)\n", p.deviceName,
               VK_VERSION_MAJOR(p.apiVersion), VK_VERSION_MINOR(p.apiVersion),
               VK_VERSION_PATCH(p.apiVersion));

        VkPhysicalDeviceFeatures f;
        vkGetPhysicalDeviceFeatures(devs[i], &f);
        printf("  REQUIRED by SurrealGPU:\n");
        printf("    samplerAnisotropy        %s\n", YN(f.samplerAnisotropy));
        printf("    fragmentStoresAndAtomics %s\n", YN(f.fragmentStoresAndAtomics));
        printf("    multiDrawIndirect        %s\n", YN(f.multiDrawIndirect));
        printf("    independentBlend         %s\n", YN(f.independentBlend));
        printf("  also used if present:\n");
        printf("    depthClamp               %s\n", YN(f.depthClamp));
        printf("    shaderClipDistance       %s\n", YN(f.shaderClipDistance));
        printf("    imageCubeArray           %s\n", YN(f.imageCubeArray));

        uint32_t en = 0;
        vkEnumerateDeviceExtensionProperties(devs[i], NULL, &en, NULL);
        VkExtensionProperties *ext = calloc(en ? en : 1, sizeof *ext);
        vkEnumerateDeviceExtensionProperties(devs[i], NULL, &en, ext);
        int swapchain = 0;
        for (uint32_t k = 0; k < en; k++)
            if (!strcmp(ext[k].extensionName, "VK_KHR_swapchain")) swapchain = 1;
        printf("    VK_KHR_swapchain         %s   (%u device extensions total)\n",
               YN(swapchain), en);
        free(ext);
    }
    free(devs);
    vkDestroyInstance(inst, NULL);
    return 0;
}
