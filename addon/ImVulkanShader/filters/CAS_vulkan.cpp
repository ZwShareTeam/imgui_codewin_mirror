#include "CAS_vulkan.h"
#include "CAS_shader.h"
#include "ImVulkanShader.h"

static inline float lerpf(float v0, float v1, float f)
{
    return v0 + (v1 - v0) * f;
}

namespace ImGui 
{
CAS_vulkan::CAS_vulkan(int gpu)
{
    vkdev = ImGui::get_gpu_device(gpu);
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    opt.use_fp16_storage = true;
    cmd = new ImGui::VkCompute(vkdev);
    std::vector<ImGui::vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;

    ImGui::compile_spirv_module(CAS_data, opt, spirv_data);
    pipe = new ImGui::Pipeline(vkdev);
    pipe->set_optimal_local_size_xyz(16, 16, 1);
    pipe->create(spirv_data.data(), spirv_data.size() * 4, specializations);

    cmd->reset();
}

CAS_vulkan::~CAS_vulkan()
{
    if (vkdev)
    {
        if (pipe) { delete pipe; pipe = nullptr; }
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
}

void CAS_vulkan::upload_param(const ImGui::VkMat& src, ImGui::VkMat& dst, float strength)
{
    std::vector<ImGui::VkMat> bindings(8);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (src.type == IM_DT_INT8)     bindings[4] = src;
    else if (src.type == IM_DT_INT16)    bindings[5] = src;
    else if (src.type == IM_DT_FLOAT16)  bindings[6] = src;
    else if (src.type == IM_DT_FLOAT32)  bindings[7] = src;

    std::vector<ImGui::vk_constant_type> constants(11);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = src.type;
    constants[5].i = dst.w;
    constants[6].i = dst.h;
    constants[7].i = dst.c;
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    constants[10].f = -lerpf(16.f, 4.01f, strength);
    cmd->record_pipeline(pipe, bindings, constants, dst);
}

void CAS_vulkan::filter(const ImGui::ImMat& src, ImGui::ImMat& dst, float strength)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

    ImGui::VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    ImGui::VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu, strength);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void CAS_vulkan::filter(const ImGui::ImMat& src, ImGui::VkMat& dst, float strength)
{
    if (!vkdev || !pipe  || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);

    ImGui::VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst, strength);

    cmd->submit_and_wait();
    cmd->reset();
}

void CAS_vulkan::filter(const ImGui::VkMat& src, ImGui::ImMat& dst, float strength)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

    ImGui::VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu, strength);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void CAS_vulkan::filter(const ImGui::VkMat& src, ImGui::VkMat& dst, float strength)
{
    if (!vkdev || !pipe || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);

    upload_param(src, dst, strength);

    cmd->submit_and_wait();
    cmd->reset();
}
} // namespace ImGui