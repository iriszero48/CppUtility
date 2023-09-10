#pragma once

#include "../Enum/Enum.hpp"

namespace CuLog
{
    CuEnum_MakeEnumDef(LogLevel, None, Error, Warn, Info, Verb, Debug);
};

CuEnum_MakeEnumSpec(CuLog, LogLevel);
