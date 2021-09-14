#pragma once
#include <imgui.h>
#include "imvk_gpu.h"
#include "imvk_pipeline.h"

namespace ImGui
{
class IMGUI_API Harris_vulkan
{
public:
    Harris_vulkan(int gpu = 0);
    ~Harris_vulkan();
    void filter(const ImMat& src, ImMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity);
    void filter(const ImMat& src, VkMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity);
    void filter(const VkMat& src, ImMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity);
    void filter(const VkMat& src, VkMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity);

private:
    VulkanDevice* vkdev      {nullptr};
    Option opt;
    Pipeline* pipe_harris    {nullptr};
    Pipeline* pipe_prewitt   {nullptr};
    Pipeline* pipe_nms       {nullptr};
    Pipeline * pipe_column   {nullptr};
    Pipeline * pipe_row      {nullptr};
    VkCompute * cmd          {nullptr};

private:
    ImMat kernel;
    VkMat vk_kernel;
    int blurRadius  {3};
    int xksize;
    int yksize;
    int xanchor;
    int yanchor;
    float sigma     {0};

private:
    void upload_param(const VkMat& src, VkMat& dst, int _blurRadius, float edgeStrength, float threshold, float harris, float sensitivity);
    void prepare_kernel();
};
} // namespace ImGui

