#include "Box.h"
#include "ImVulkanShader.h"

BoxBlur::BoxBlur(int gpu)
    : ImGui::Filter2DS_vulkan(gpu)
{
    prepare_kernel();
}

BoxBlur::~BoxBlur()
{
}

void BoxBlur::prepare_kernel()
{
    xksize = xSize;
    yksize = ySize;
    xanchor = xSize / 2;
    yanchor = ySize / 2;
    float kvulve = 2.0f / (float)(xSize + ySize);
    kernel.create(xSize, ySize, size_t(4u), 1);
    for (int x = 0; x < xSize; x++)
    {
        for (int y = 0; y < ySize; y++)
        {
            kernel.at<float>(x, y) = kvulve;
        }

    }
    ImGui::VkTransfer tran(vkdev);
    tran.record_upload(kernel, vk_kernel, opt, false);
    tran.submit_and_wait();
}

void BoxBlur::SetParam(int _xSize, int _ySize)
{
    if (xSize != _xSize || ySize != _ySize)
    {
        xSize = _xSize;
        ySize = _ySize;
        prepare_kernel();
    }
}
