
#if !defined( EH_TYPES_H )
#define EH_TYPES_H

#define API_ENTRY

typedef struct
{
   char *var_name;
}
Class_Desc;

#define _CD( type ) type##_Class_Desc
#define CLASS( type ) \
   static Class_Desc _CD( type ) = { #type } ; typedef struct tag_##type
#define DERIVED_CLASS( base_type , type ) \
   static Class_Desc _CD( type ) = { #type } ; typedef struct tag##base_type type

#define NEW_OBJECT( type , obj ) \
   ( obj = (type)eh_malloc( sizeof(*obj),&_CD(type),__FILE__ , __LINE__ ) )
#define FREE_OBJECT( obj ) ( *(void**)&obj = eh_free( obj ) )

#define new_handle( Handle ) typedef struct tag_##Handle *Handle
#define derived_handle( Base_handle , Handle ) typedef struct tag_##Base_handle *Handle

#endif

