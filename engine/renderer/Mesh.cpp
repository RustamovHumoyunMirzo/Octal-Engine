#include "Mesh.h"

#include <atomic>

namespace OctalEngine
{
    namespace
    {
        std::uint64_t nextMeshId()
        {
            static std::atomic<std::uint64_t> nextId{1};
            return nextId.fetch_add(1, std::memory_order_relaxed);
        }
    }

    VertexBuffer::VertexBuffer(std::vector<Vertex> vertices)
        : data(std::move(vertices))
    {
    }

    const std::vector<Vertex>& VertexBuffer::vertices() const
    {
        return data;
    }

    bool VertexBuffer::empty() const
    {
        return data.empty();
    }

    IndexBuffer::IndexBuffer(std::vector<std::uint16_t> indices)
        : data(std::move(indices))
    {
    }

    const std::vector<std::uint16_t>& IndexBuffer::indices() const
    {
        return data;
    }

    bool IndexBuffer::empty() const
    {
        return data.empty();
    }

    Mesh::Mesh()
        : data(std::make_shared<Data>())
    {
        data->id = nextMeshId();
    }

    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<std::uint16_t> indices)
        : Mesh()
    {
        data->vertices = std::move(vertices);
        data->indices = std::move(indices);
    }

    Mesh::Mesh(VertexBuffer vertices, IndexBuffer indices)
        : Mesh(vertices.vertices(), indices.indices())
    {
    }

    const std::vector<Vertex>& Mesh::vertices() const
    {
        return data->vertices;
    }

    const std::vector<std::uint16_t>& Mesh::indices() const
    {
        return data->indices;
    }

    bool Mesh::empty() const
    {
        return data->vertices.empty();
    }

    std::uint64_t Mesh::id() const
    {
        return data->id;
    }
}
