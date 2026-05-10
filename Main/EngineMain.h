#pragma once

namespace ho
{
struct EngineMainParam
{
    int Argc;
    char** Argv;
};

int EngineMain(const EngineMainParam& param);
} // namespace ho
