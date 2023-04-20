﻿#pragma once

enum Config
{
    CONFIG_FLAG_NONE                            = 0 << 0,
    CONFIG_FLAG_V1_MATERIAL                     = 1 << 0,
    CONFIG_FLAG_XBOX_VERTEX_FORMAT              = 1 << 1,
    CONFIG_FLAG_V2_SAMPLE_CHUNK                 = 1 << 2,
    CONFIG_FLAG_85_BONE_LIMIT                   = 1 << 3,
    CONFIG_FLAG_D3D11_VERTEX_FORMAT             = 1 << 4,
    CONFIG_FLAG_TRIANGLELIST_PRIMITIVE_TOPOLOGY = 1 << 5,
    CONFIG_FLAG_340_BONE_LIMIT                  = 1 << 6,

    CONFIG_UNLEASHED                            = CONFIG_FLAG_V1_MATERIAL | CONFIG_FLAG_XBOX_VERTEX_FORMAT,
    CONFIG_GENERATIONS                          = CONFIG_FLAG_NONE,
    CONFIG_LOST_WORLD                           = CONFIG_FLAG_V2_SAMPLE_CHUNK,
    CONFIG_FORCES                               = CONFIG_FLAG_V2_SAMPLE_CHUNK | CONFIG_FLAG_85_BONE_LIMIT | CONFIG_FLAG_D3D11_VERTEX_FORMAT,
    CONFIG_FRONTIERS                            = CONFIG_FLAG_V2_SAMPLE_CHUNK | CONFIG_FLAG_D3D11_VERTEX_FORMAT | CONFIG_FLAG_TRIANGLELIST_PRIMITIVE_TOPOLOGY | CONFIG_FLAG_340_BONE_LIMIT
};