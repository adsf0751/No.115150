/*
 * Windows platform (Win32 or WinCE) has a different syntax for defining
 * packed structs and unions, which involves pushing and poping a pragma.
 */

#ifdef CONFIG_WINDOWS
#  ifndef PRAGMA_PACKED
#    error Pragma pack not found
#  endif

#  pragma pack(pop)

#  undef PRAGMA_PACKED
#endif
