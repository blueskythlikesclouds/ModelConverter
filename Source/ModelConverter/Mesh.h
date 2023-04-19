﻿#pragma once

struct SampleChunk;
struct TextureUnit;
struct VertexElement;

struct Mesh
{
    std::string materialName;
    std::vector<uint16_t> faceIndices;
    std::vector<std::vector<Vector4>> vertexStreams[14];
    std::vector<VertexElement> vertexElements;
    std::vector<uint16_t> nodeIndices;
    std::vector<TextureUnit> textureUnits;

    Mesh();
    ~Mesh();

    void write(SampleChunk& out) const;
};