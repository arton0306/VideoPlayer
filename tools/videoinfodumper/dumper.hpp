#ifdef BUILD_DLL
    #define BUILD_TYPE __declspec(dllexport)
#else
    #define BUILD_TYPE
#endif

extern "C"
{
    void BUILD_TYPE libav_dumper( char const * filename ); // will output to stderr
}
