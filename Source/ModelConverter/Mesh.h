#pragma once

struct SampleChunkWriter;
struct TextureUnit;
struct VertexElement;

struct Mesh
{
    std::string materialName;
    std::vector<uint32_t> faceIndices;
    std::vector<std::vector<Vector4>> vertexStreams[14];
    std::vector<VertexElement> vertexElements;
    std::vector<uint16_t> nodeIndices;
    std::vector<TextureUnit> textureUnits;

    Mesh();
    ~Mesh();

    void write(SampleChunkWriter& writer, uint32_t dataVersion) const;
};
