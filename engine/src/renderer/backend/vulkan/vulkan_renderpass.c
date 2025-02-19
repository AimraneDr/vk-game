#include "renderer/backend/vulkan/vulkan_renderpass.h"

#include "core/debugger.h"
#include "renderer/backend/vulkan/details/resources/depth.h"

void createVulkanRenderPass(VkPhysicalDevice gpu, VkDevice device, VkFormat swapchain_image_format, const RenderpassInitConfig* config, VkRenderPass* out_render_pass) {
    // Check if depth attachment is needed
    bool hasDepth = false;
    for (u8 i = 0; i < config->pipelinesCount; i++) {
        if (config->pipelines[i].depthTestingEnabled) {
            hasDepth = true;
            break;
        }
    }

    VkSampleCountFlagBits msaaSamples = (VkSampleCountFlagBits)config->msaaSamples;
    bool resolveNeeded = (msaaSamples > VK_SAMPLE_COUNT_1_BIT);

    // Create attachments array
    u32 attachmentsCount = 1 + (hasDepth ? 1 : 0) + (resolveNeeded ? 1 : 0);
    VkAttachmentDescription* attachments = malloc(attachmentsCount * sizeof(VkAttachmentDescription));
    if (!attachments) {
        LOG_FATAL("Failed to allocate attachments");
        return;
    }

    u32 index = 0;
    // Color attachment
    attachments[index++] = (VkAttachmentDescription){
        .format = swapchain_image_format,
        .samples = msaaSamples,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = resolveNeeded ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    // Depth attachment
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    if (hasDepth) {
        depthFormat = findDepthFormat(gpu);
        if (depthFormat == VK_FORMAT_UNDEFINED) {
            LOG_FATAL("Failed to find depth format");
            free(attachments);
            return;
        }
        attachments[index++] = (VkAttachmentDescription){
            .format = depthFormat,
            .samples = msaaSamples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
    }

    // Resolve attachment
    if (resolveNeeded) {
        attachments[index++] = (VkAttachmentDescription){
            .format = swapchain_image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
    }

    // Create subpasses
    VkSubpassDescription* subpasses = malloc(config->pipelinesCount * sizeof(VkSubpassDescription));
    if (!subpasses) {
        LOG_FATAL("Failed to allocate subpasses");
        free(attachments);
        return;
    }

    for (u8 i = 0; i < config->pipelinesCount; i++) {
        const PipelineConfig* pipeline = &config->pipelines[i];
        VkAttachmentReference colorRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        VkAttachmentReference depthRef;
        if (pipeline->depthTestingEnabled && hasDepth) {
            depthRef.attachment = 1;
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else {
            depthRef.attachment = VK_ATTACHMENT_UNUSED;
            depthRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        VkAttachmentReference resolveRef;
        if (i == config->pipelinesCount - 1 && resolveNeeded) {
            resolveRef.attachment = hasDepth ? 2 : 1;
            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            resolveRef.attachment = VK_ATTACHMENT_UNUSED;
            resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        subpasses[i] = (VkSubpassDescription){
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorRef,
            .pDepthStencilAttachment = (pipeline->depthTestingEnabled && hasDepth) ? &depthRef : NULL,
            .pResolveAttachments = (i == config->pipelinesCount - 1 && resolveNeeded) ? &resolveRef : NULL
        };
    }

    // Create dependencies
    u32 dependencyCount = config->pipelinesCount + 1;
    VkSubpassDependency* dependencies = malloc(dependencyCount * sizeof(VkSubpassDependency));
    if (!dependencies) {
        LOG_FATAL("Failed to allocate dependencies");
        free(attachments);
        free(subpasses);
        return;
    }

    dependencies[0] = (VkSubpassDependency){
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    for (u8 i = 0; i < config->pipelinesCount - 1; i++) {
        dependencies[i + 1] = (VkSubpassDependency){
            .srcSubpass = i,
            .dstSubpass = i + 1,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        };
    }

    dependencies[dependencyCount - 1] = (VkSubpassDependency){
        .srcSubpass = config->pipelinesCount - 1,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstAccessMask = 0,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    };

    // Create render pass
    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachmentsCount,
        .pAttachments = attachments,
        .subpassCount = config->pipelinesCount,
        .pSubpasses = subpasses,
        .dependencyCount = dependencyCount,
        .pDependencies = dependencies
    };

    VkResult res = vkCreateRenderPass(device, &renderPassInfo, NULL, out_render_pass);
    free(attachments);
    free(subpasses);
    free(dependencies);

    if (res != VK_SUCCESS) {
        LOG_FATAL("Failed to create render pass: %d", res);
    }
}

void destroyVulkanRenderPass(VulkanContext* c, RenderPass* render_pass){
    vkDestroyRenderPass(c->device, render_pass->ref, 0);
}