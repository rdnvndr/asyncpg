#pragma once

#ifdef _MSC_VER
#ifdef ASYNCPG_LIBRARY
#  define ASYNCPGLIB __declspec(dllexport)
#else
#  define ASYNCPGLIB __declspec(dllimport)
#endif
#else
#  define ASYNCPGLIB
#endif
