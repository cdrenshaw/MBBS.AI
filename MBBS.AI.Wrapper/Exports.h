#pragma once

#ifdef MBBSAIWRAPPER_EXPORTS
#   define MBBSAI_API __declspec(dllexport)
#else
#   define MBBSAI_API __declspec(dllimport)
#endif
