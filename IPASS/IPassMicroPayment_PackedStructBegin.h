#ifndef _PACKED_STRUCT_BEGIN_H_
#  define _PACKED_STRUCT_BEGIN_H_

#  if defined(__GNUC__)
#    define STRUCT struct __attribute__((packed))
#    define UNION union __attribute__((packed))
#  elif defined(__CC_ARM)
#    define STRUCT __packed struct
#    define UNION __packed union
#  elif defined(CONFIG_WINDOWS)
#    define STRUCT struct
#    define UNION union
#  else
#    error Keyword STRUCT and UNION unprocessed
#  endif

#endif

#ifdef CONFIG_WINDOWS
#  ifdef PRAGMA_PACKED
#    error Overlapping pragma pack detected
#  endif

#  pragma pack(push, 1)

#  define PRAGMA_PACKED
#endif
