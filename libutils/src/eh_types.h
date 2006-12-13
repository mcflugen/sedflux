#if !defined( EH_TYPES_H )
#define EH_TYPES_H

#define API_ENTRY
/*
typedef struct
{
   char *var_name;
}
Class_Desc;
*/
typedef char* Class_Desc;

#define _CD( type ) type##_Class_Desc
#define CLASS( type ) \
   static Class_Desc _CD( type ) = G_STRINGIFY(type) ; typedef struct tag_##type

//   static Class_Desc _CD( type ) = { #type } ; typedef struct tag_##type

#define DERIVED_CLASS( base_type , type ) \
   static Class_Desc _CD( type ) = G_STRINGIFY(type) ; typedef struct tag##base_type type

//   static Class_Desc _CD( type ) = { #type } ; typedef struct tag##base_type type

#define USE_MY_VTABLE
#if defined( USE_MY_VTABLE )

#define NEW_OBJECT( type , obj ) \
   ( obj = (type)eh_malloc( sizeof(*obj),G_STRINGIFY(type),__FILE__ , __LINE__ ) )
//   ( obj = (type)eh_malloc( sizeof(*obj),&_CD(type),__FILE__ , __LINE__ ) )
#define FREE_OBJECT( obj ) ( *(void**)&obj = eh_free( obj ) )

#else

#define NEW_OBJECT( type , obj ) \
   ( obj = (type)g_malloc( sizeof(*obj) ) )
#define FREE_OBJECT( obj ) ( g_free( obj ) )

#endif

#define new_handle( Handle ) typedef struct tag_##Handle *Handle
#define derived_handle( Base_handle , Handle ) typedef struct tag_##Base_handle *Handle

#endif

