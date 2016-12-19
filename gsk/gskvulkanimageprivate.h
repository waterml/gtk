#ifndef __GSK_VULKAN_IMAGE_PRIVATE_H__
#define __GSK_VULKAN_IMAGE_PRIVATE_H__

#include <gdk/gdk.h>

#include "gsk/gskvulkancommandpoolprivate.h"

G_BEGIN_DECLS

#define GSK_TYPE_VULKAN_IMAGE (gsk_vulkan_image_get_type ())

G_DECLARE_FINAL_TYPE (GskVulkanImage, gsk_vulkan_image, GSK, VULKAN_IMAGE, GObject)

GskVulkanImage *        gsk_vulkan_image_new_for_swapchain              (GdkVulkanContext       *context,
                                                                         VkImage                 image,
                                                                         VkFormat                format,
                                                                         gsize                   width,
                                                                         gsize                   height);
GskVulkanImage *        gsk_vulkan_image_new_from_data                  (GdkVulkanContext       *context,
                                                                         GskVulkanCommandPool   *pool,
                                                                         guchar                 *data,
                                                                         gsize                   width,
                                                                         gsize                   height,
                                                                         gsize                   stride);

gsize                   gsk_vulkan_image_get_width                      (GskVulkanImage         *self);
gsize                   gsk_vulkan_image_get_height                     (GskVulkanImage         *self);
VkImage                 gsk_vulkan_image_get_image                      (GskVulkanImage         *self);
VkImageView             gsk_vulkan_image_get_image_view                 (GskVulkanImage         *self);

G_END_DECLS

#endif /* __GSK_VULKAN_IMAGE_PRIVATE_H__ */
