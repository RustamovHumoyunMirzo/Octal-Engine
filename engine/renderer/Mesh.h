#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace OctalEngine
{
    struct Vertex
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float nx = 0.0f;
        float ny = 0.0f;
        float nz = 1.0f;
        float u = 0.0f;
        float v = 0.0f;
    };

    class VertexBuffer
    {
    public:
        VertexBuffer() = default;
        explicit VertexBuffer(std::vector<Vertex> vertices);

        const std::vector<Vertex>& vertices() const;
        bool empty() const;

    private:
        std::vector<Vertex> data;
    };

    class IndexBuffer
    {
    public:
        IndexBuffer() = default;
        explicit IndexBuffer(std::vector<std::uint16_t> indices);

        const std::vector<std::uint16_t>& indices() const;
        bool empty() const;

    private:
        std::vector<std::uint16_t> data;
    };

    class Mesh
    {
    public:
        Mesh();
        Mesh(std::vector<Vertex> vertices, std::vector<std::uint16_t> indices = {});
        Mesh(VertexBuffer vertices, IndexBuffer indices = {});

        const std::vector<Vertex>& vertices() const;
        const std::vector<std::uint16_t>& indices() const;
        bool empty() const;

        std::uint64_t id() const;

    private:
        struct Data
        {
            std::uint64_t id = 0;
            std::vector<Vertex> vertices;
            std::vector<std::uint16_t> indices;
        };

        std::shared_ptr<Data> data;
    };
}
