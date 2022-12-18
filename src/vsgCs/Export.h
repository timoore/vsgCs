#pragma once

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( VSGCS_LIBRARY_STATIC )
    #    define VSGCS_EXPORT
    #  elif defined( VSGCS_LIBRARY )
    #    define VSGCS_EXPORT   __declspec(dllexport)
    #  else
    #    define VSGCS_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define VSGCS_EXPORT
#endif  
