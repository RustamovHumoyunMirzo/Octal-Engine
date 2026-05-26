#pragma once

#include "../RendererMath.h"
#include "../Mesh.h"
#include "../Renderer.h"

#include <variant>
#include <vector>

namespace OctalEngine::RendererInternal
{
    enum class RenderCommandType
    {
        DrawMesh,
        DrawVertices,
        SetTransform,
        SetCamera,
        Resize
    };

    struct DrawMeshCommand
    {
        Mesh mesh;
        Mat4 transform;
        RenderColor color;
    };

    struct DrawVerticesCommand
    {
        std::vector<Vertex> vertices;
        Mat4 transform;
    };

    struct SetTransformCommand
    {
        Mat4 transform;
    };

    struct SetCameraCommand
    {
        RenderCamera camera;
    };

    struct ResizeCommand
    {
        int width = 1;
        int height = 1;
    };

    using RenderCommand = std::variant<DrawMeshCommand, DrawVerticesCommand, SetTransformCommand, SetCameraCommand, ResizeCommand>;

    struct CommandBuffer
    {
        std::vector<RenderCommand> commands;

        void clear()
        {
            commands.clear();
        }
    };
}
