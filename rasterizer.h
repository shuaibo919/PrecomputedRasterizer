#pragma once
#include <cstdint>
#include <vector>
#include <cmath>
#include <intrin.h>
#include <immintrin.h>
#include <glm/glm.hpp>
#include "utils.h"
#include <algorithm>

class Rasterizer
{

    public:
        constexpr inline static int GridSize = 8;
        constexpr inline static float GridRange = 32; // 4 * sqrt(2)
        constexpr inline static int32_t QuantizationResolution = 64;
        constexpr inline static int32_t OffsetSample = 64;
        Rasterizer(int32_t width, int32_t height) : mWidth(width), mHeight(height)
        {
            FrameBuffer.resize(mWidth * mHeight, 0);
            PrecomputeRasterizationData();
        }
        ~Rasterizer()
        {
            if(textureData)
            {
                delete [] textureData;
                textureData = nullptr;
            }
        }

        unsigned char *textureData = nullptr;
        GLuint GetFrameBufferTexture()
        {
            textureData = new unsigned char[mWidth * mHeight * 4]; // 4 channels (RGBA)
            for (int y = 0; y < mHeight; ++y)
            {
                for (int x = 0; x < mWidth; ++x)
                {
                    int idx = (y * mWidth + x) * 4;
                    float u = static_cast<float>(x) / (mWidth - 1);
                    float v = static_cast<float>(mHeight - y - 1) / (mHeight - 1);
                    if(FrameBuffer[y * mWidth + x] > 0)
                    {
                        textureData[idx + 0] = static_cast<unsigned char>(128); // R
                        textureData[idx + 1] = static_cast<unsigned char>(u * 255); // G
                        textureData[idx + 2] = static_cast<unsigned char>(v * 255); // B
                        textureData[idx + 3] = 255; // A
                    }
                    else
                    {
                        textureData[idx + 0] = 0; // R
                        textureData[idx + 1] = 0; // G
                        textureData[idx + 2] = 0; // B
                        textureData[idx + 3] = 255; // A
                    }
                }
            }
            return LoadTextureFromArray(textureData, mWidth, mHeight, GL_REPEAT, false);
        }


        void RasterizePrototype3(std::vector<glm::vec3> &vertices)
        {
            for(int i = 0; i < vertices.size(); i+=3)
            {
                glm::vec3 v0 = vertices[i];
                glm::vec3 v1 = vertices[i+1];
                glm::vec3 v2 = vertices[i+2];
                // NDC to Screen
                v0 = (v0 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                v1 = (v1 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                v2 = (v2 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                // Bounding Box
                int minX = (int)std::floor(std::min({v0.x, v1.x, v2.x}));
                int maxX =  (int)std::ceil(std::max({v0.x, v1.x, v2.x}));
                int minY = (int)std::floor(std::min({v0.y, v1.y, v2.y}));
                int maxY =  (int)std::ceil(std::max({v0.y, v1.y, v2.y}));

                // Edge Equation
                glm::vec2 e0 = glm::vec2(v0.x - v1.x, v0.y - v1.y);
                glm::vec2 e1 = glm::vec2(v1.x - v2.x, v1.y - v2.y);
                glm::vec2 e2 = glm::vec2(v2.x - v0.x, v2.y - v0.y);
                float c0 = v0.x * v1.y - v0.y * v1.x;
                float c1 = v1.x * v2.y - v1.y * v2.x;
                float c2 = v2.x * v0.y - v2.y * v0.x;
                glm::vec2 n0 = glm::normalize(glm::vec2(e0.y, -e0.x));
                glm::vec2 n1 = glm::normalize(glm::vec2(e1.y, -e1.x));
                glm::vec2 n2 = glm::normalize(glm::vec2(e2.y, -e2.x));

                glm::vec3 line0 = glm::vec3(n0, c0 / glm::length(e0));
                glm::vec3 line1 = glm::vec3(n1, c1 / glm::length(e1));
                glm::vec3 line2 = glm::vec3(n2, c2 / glm::length(e2));


                minY = minY / GridSize;
                maxY = (maxY / GridSize)  + ((maxY % GridSize)? 1: 0);
                minX = minX / GridSize ;
                maxX = (maxX / GridSize)  + ((maxX % GridSize)? 1: 0);


                float deltax0 = line0.x * GridSize;
                float deltay0 = line0.y * GridSize;
                float deltax1 = line1.x * GridSize;
                float deltay1 = line1.y * GridSize;
                float deltax2 = line2.x * GridSize;
                float deltay2 = line2.y * GridSize;

                float Offset0 = line0.z + deltay0 * minY;
                float Offset1 = line1.z + deltay1 * minY;
                float Offset2 = line2.z + deltay2 * minY;

                int slopeIdxX0 = static_cast<int>((line0.x + 1.0f) * 0.5f * (QuantizationResolution - 1));
                int slopeIdxY0 = static_cast<int>((line0.y + 1.0f) * 0.5f * (QuantizationResolution - 1));

                int slopeIdxX1 = static_cast<int>((line1.x + 1.0f) * 0.5f * (QuantizationResolution - 1));
                int slopeIdxY1 = static_cast<int>((line1.y + 1.0f) * 0.5f * (QuantizationResolution - 1));

                int slopeIdxX2 = static_cast<int>((line2.x + 1.0f) * 0.5f * (QuantizationResolution - 1));
                int slopeIdxY2 = static_cast<int>((line2.y + 1.0f) * 0.5f * (QuantizationResolution - 1));

                uint32_t IdxPre0 = (slopeIdxY0 << 12) | (slopeIdxX0 << 6);
                uint32_t IdxPre1 = (slopeIdxY1 << 12) | (slopeIdxX1 << 6);
                uint32_t IdxPre2 = (slopeIdxY2 << 12) | (slopeIdxX2 << 6);

                for(int y = minY; y < maxY; ++y)
                {
                    float currentOffset0 = Offset0 + deltax0 * minX;
                    float currentOffset1 = Offset1 + deltax1 * minX;
                    float currentOffset2 = Offset2 + deltax2 * minX;
                    for(int x = minX; x < maxX; ++x)
                    {
                        int offsetIdx0 = static_cast<int>((currentOffset0 / GridRange - 0.5f) * OffsetSample);
                        offsetIdx0 = std::clamp(offsetIdx0, 0, OffsetSample - 1);
                        int offsetIdx1 = static_cast<int>((currentOffset1 / GridRange - 0.5f) * OffsetSample);
                        offsetIdx1 = std::clamp(offsetIdx1, 0, OffsetSample - 1);
                        int offsetIdx2 = static_cast<int>((currentOffset2 / GridRange - 0.5f) * OffsetSample);
                        offsetIdx2 = std::clamp(offsetIdx2, 0, OffsetSample - 1);

                        uint32_t tableIdx0 = IdxPre0 | offsetIdx0;
                        uint32_t tableIdx1 = IdxPre1 | offsetIdx1;
                        uint32_t tableIdx2 = IdxPre2 | offsetIdx2;

                        uint64_t d0 = BitMaskTable[tableIdx0];
                        uint64_t d1 = BitMaskTable[tableIdx1];
                        uint64_t d2 = BitMaskTable[tableIdx2];

                        // uint64_t d0 = BitMaskTable2D[slopeIdxY0][slopeIdxX0][offsetIdx0];
                        // uint64_t d1 = BitMaskTable2D[slopeIdxY1][slopeIdxX1][offsetIdx1];
                        // uint64_t d2 = BitMaskTable2D[slopeIdxY2][slopeIdxX2][offsetIdx2];

                        uint64_t finalBitmask = d0 & d1 & d2;

                        for(int gy = 0; gy < GridSize; ++gy)
                        {
                            for(int gx = 0; gx < GridSize; ++gx)
                            {
                                int pixelX = x * GridSize + gx;
                                int pixelY = y * GridSize + gy;
                               
                                int bitIdx = gy * GridSize + gx;
                                if(finalBitmask & (1ull << bitIdx))
                                {
                                    int fbIdx = pixelY * mWidth + pixelX;
                                    if(fbIdx >= 0 && fbIdx < mWidth * mHeight)
                                    {
                                        FrameBuffer[fbIdx] = 255;
                                    }
                                }
                            }
                        }
                        currentOffset0 += deltax0;
                        currentOffset1 += deltax1;
                        currentOffset2 += deltax2;
                    }
                    Offset0 += deltay0;
                    Offset1 += deltay1;
                    Offset2 += deltay2;
                }
            }
        }

        void RasterizePrototype2(std::vector<glm::vec3> &vertices)
        {
            for(int i = 0; i < vertices.size(); i+=3)
            {
                glm::vec3 v0 = vertices[i];
                glm::vec3 v1 = vertices[i+1];
                glm::vec3 v2 = vertices[i+2];
                // NDC to Screen
                v0 = (v0 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                v1 = (v1 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                v2 = (v2 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                // Bounding Box
                int minX = (int)std::floor(std::min({v0.x, v1.x, v2.x}));
                int maxX =  (int)std::ceil(std::max({v0.x, v1.x, v2.x}));
                int minY = (int)std::floor(std::min({v0.y, v1.y, v2.y}));
                int maxY =  (int)std::ceil(std::max({v0.y, v1.y, v2.y}));

                // Edge Equation
                glm::vec2 e0 = glm::vec2(v0.x - v1.x, v0.y - v1.y);
                glm::vec2 e1 = glm::vec2(v1.x - v2.x, v1.y - v2.y);
                glm::vec2 e2 = glm::vec2(v2.x - v0.x, v2.y - v0.y);
                float c0 = v0.x * v1.y - v0.y * v1.x;
                float c1 = v1.x * v2.y - v1.y * v2.x;
                float c2 = v2.x * v0.y - v2.y * v0.x;
                glm::vec2 n0 = glm::normalize(glm::vec2(e0.y, -e0.x));
                glm::vec2 n1 = glm::normalize(glm::vec2(e1.y, -e1.x));
                glm::vec2 n2 = glm::normalize(glm::vec2(e2.y, -e2.x));

                glm::vec3 line0 = glm::vec3(n0, c0 / glm::length(e0));
                glm::vec3 line1 = glm::vec3(n1, c1 / glm::length(e1));
                glm::vec3 line2 = glm::vec3(n2, c2 / glm::length(e2));


                minY = minY / GridSize;
                maxY = (maxY / GridSize)  + ((maxY % GridSize)? 1: 0);
                minX = minX / GridSize ;
                maxX = (maxX / GridSize)  + ((maxX % GridSize)? 1: 0);


                float deltax0 = line0.x * GridSize;
                float deltay0 = line0.y * GridSize;
                float deltax1 = line1.x * GridSize;
                float deltay1 = line1.y * GridSize;
                float deltax2 = line2.x * GridSize;
                float deltay2 = line2.y * GridSize;

                float Offset0 = line0.z + deltay0 * minY;
                float Offset1 = line1.z + deltay1 * minY;
                float Offset2 = line2.z + deltay2 * minY;

                for(int y = minY; y < maxY; ++y)
                {
                    float currentOffset0 = Offset0 + deltax0 * minX;
                    float currentOffset1 = Offset1 + deltax1 * minX;
                    float currentOffset2 = Offset2 + deltax2 * minX;
                    for(int x = minX; x < maxX; ++x)
                    {
                        for(int gy = 0; gy < GridSize; ++gy)
                        {
                            for(int gx = 0; gx < GridSize; ++gx)
                            {
                                int pixelX = x * GridSize + gx;
                                int pixelY = y * GridSize + gy;
                                float d0 = line0.x * (gx + 0.5f) + line0.y * (gy + 0.5f) + currentOffset0;
                                float d1 = line1.x * (gx + 0.5f) + line1.y * (gy + 0.5f) + currentOffset1;
                                float d2 = line2.x * (gx + 0.5f) + line2.y * (gy + 0.5f) + currentOffset2;
                                if(d0 >= 0 && d1 >= 0 && d2 >= 0)
                                {
                                    int fbIdx = pixelY * mWidth + pixelX;
                                    FrameBuffer[fbIdx] = 255;
                                }
                            }
                        }
                        currentOffset0 += deltax0;
                        currentOffset1 += deltax1;
                        currentOffset2 += deltax2;
                    }
                    Offset0 += deltay0;
                    Offset1 += deltay1;
                    Offset2 += deltay2;
                }
            }
        }

        void RasterizePrototype1(std::vector<glm::vec3> &vertices)
        {
            for(int i = 0; i < vertices.size(); i+=3)
            {
                glm::vec3 v0 = vertices[i];
                glm::vec3 v1 = vertices[i+1];
                glm::vec3 v2 = vertices[i+2];
                // NDC to Screen
                v0 = (v0 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                v1 = (v1 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                v2 = (v2 + 1.0f) * 0.5f * glm::vec3(mWidth, mHeight, 1.0f);
                // Bounding Box
                int minX = (int)std::floor(std::min({v0.x, v1.x, v2.x}));
                int maxX =  (int)std::ceil(std::max({v0.x, v1.x, v2.x}));
                int minY = (int)std::floor(std::min({v0.y, v1.y, v2.y}));
                int maxY =  (int)std::ceil(std::max({v0.y, v1.y, v2.y}));

                // Edge Equation
                glm::vec2 e0 = glm::vec2(v0.x - v1.x, v0.y - v1.y);
                glm::vec2 e1 = glm::vec2(v1.x - v2.x, v1.y - v2.y);
                glm::vec2 e2 = glm::vec2(v2.x - v0.x, v2.y - v0.y);
                float c0 = v0.x * v1.y - v0.y * v1.x;
                float c1 = v1.x * v2.y - v1.y * v2.x;
                float c2 = v2.x * v0.y - v2.y * v0.x;
                glm::vec2 n0 = glm::normalize(glm::vec2(e0.y, -e0.x));
                glm::vec2 n1 = glm::normalize(glm::vec2(e1.y, -e1.x));
                glm::vec2 n2 = glm::normalize(glm::vec2(e2.y, -e2.x));

                glm::vec3 line0 = glm::vec3(n0, c0 / glm::length(e0));
                glm::vec3 line1 = glm::vec3(n1, c1 / glm::length(e1));
                glm::vec3 line2 = glm::vec3(n2, c2 / glm::length(e2));


                minY = minY / GridSize;
                maxY = (maxY / GridSize)  + ((maxY % GridSize)? 1: 0);
                minX = minX / GridSize ;
                maxX = (maxX / GridSize)  + ((maxX % GridSize)? 1: 0);


                for(int y = minY; y < maxY; ++y)
                {
                    for(int x = minX; x < maxX; ++x)
                    {
                        for(int gy = 0; gy < GridSize; ++gy)
                        {
                            for(int gx = 0; gx < GridSize; ++gx)
                            {
                                int pixelX = x * GridSize + gx;
                                int pixelY = y * GridSize + gy;
                                
                                float d0 = line0.x * (pixelX + 0.5f) + line0.y * (pixelY + 0.5f) + line0.z;
                                float d1 = line1.x * (pixelX + 0.5f) + line1.y * (pixelY + 0.5f) + line1.z;
                                float d2 = line2.x * (pixelX + 0.5f) + line2.y * (pixelY + 0.5f) + line2.z;
                                if(d0 >= 0 && d1 >= 0 && d2 >= 0)
                                {
                                    int fbIdx = pixelY * mWidth + pixelX;
                                    FrameBuffer[fbIdx] = 255;
                                }
                            }
                        }
                    }
                }
            }
        }

    private:
        int32_t mWidth;
        int32_t mHeight;
        std::vector<uint64_t> BitMaskTable;
        std::vector<std::vector<std::vector<uint64_t>>> BitMaskTable2D; // [QuantizationResolution][QuantizationResolution][OffsetSample]
        std::vector<uint8_t> FrameBuffer; // R8

        void PrecomputeRasterizationData()
        {
            constexpr float SlopeScale = 1.f / 512 * 6.283185307f;
            BitMaskTable.resize(QuantizationResolution * QuantizationResolution * OffsetSample, 0);
            BitMaskTable2D.resize(QuantizationResolution, std::vector<std::vector<uint64_t>>(QuantizationResolution, std::vector<uint64_t>(OffsetSample, 0)));
            for(int i = 0; i < 512; ++i)
            {
                float angle = (float)i * SlopeScale; // 2 * PI
                float nx = std::cos(angle);
                float ny = std::sin(angle);

                // Remapping to [0, QuantizationResolution] for indexing
                int slopeIdxX = static_cast<int>((nx + 1.0f) * 0.5f * (QuantizationResolution - 1));
                int slopeIdxY = static_cast<int>((ny + 1.0f) * 0.5f * (QuantizationResolution - 1));

                for(int k = 0; k < OffsetSample; ++k)
                {
                    float nk = ((float)k / OffsetSample - 0.5f) * GridRange; // -GridRange/2 ~ GridRange/2
                    uint64_t bitmask = 0;
                    for(int x = 0; x < 8; ++x)
                    {
                        for(int y = 0; y < 8; ++y)
                        {
                            float sampleX = (float)(x) + 0.5f;
                            float sampleY = (float)(y) + 0.5f;

                            float dist = sampleX * nx + sampleY * ny + nk;

                            int Idx = y * 8 + x;

                            bitmask |= (dist >= 0) ? (1ull << Idx) : 0;
                        }
                    }

                    uint32_t tableIdx = slopeIdxY << 12 | slopeIdxX << 6 | k;
                    BitMaskTable[tableIdx] = bitmask;
                    BitMaskTable2D[slopeIdxY][slopeIdxX][k] = bitmask;
                }
            }
        }
};
