
#pragma once 

namespace vkutil {

    void transiton_image(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
};