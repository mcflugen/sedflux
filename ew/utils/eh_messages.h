#ifndef __EH_MESSAGES_H__
#define __EH_MESSAGES_H__

#ifdef __cplusplus
extern "C" {
#endif
#if defined( DISABLE_WATCH_POINTS )

#define eh_watch_int( val ) {}
#define eh_watch_lng( val ) {}
#define eh_watch_ptr( val ) {}
#define eh_watch_dbl( val ) {}
#define eh_watch_exp( val ) {}
#define eh_watch_str( val ) {}
#define eh_watch_chr( val ) {}
#define eh_watch_dbl_vec( val , l , h ) {}

#else

#define eh_watch_int( val )  G_STMT_START { \
   fprintf(stderr,                          \
           "%s = %d (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_lng( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %ld (%s, line %d)\n",      \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);          } G_STMT_END

#define eh_watch_ptr( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %p (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);          } G_STMT_END

#define eh_watch_dbl( val ) G_STMT_START {  \
   if ( fabs(val) < 1e3 && fabs(val)>1e-3 ) \
      fprintf(stderr,                       \
              "%s = %f (%s, line %d)\n",    \
              #val,                         \
              val,                          \
              g_basename(__FILE__),         \
              __LINE__),                    \
      fflush(stderr);                       \
   else                                     \
      eh_watch_exp( val );       } G_STMT_END

#define eh_watch_exp( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %g (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_str( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %s (%s, line %d)\n",       \
           #val,                            \
           (val)?(val):"(null)" ,           \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_chr( val ) G_STMT_START {  \
   fprintf(stderr,                          \
           "%s = %f (%s, line %d)\n",       \
           #val,                            \
           val,                             \
           g_basename(__FILE__),            \
           __LINE__),                       \
   fflush(stderr);               } G_STMT_END

#define eh_watch_dbl_vec( val , l , h ) G_STMT_START { \
   int i;                                              \
   for ( i=l ; i<=h ; i++ ) {                          \
      fprintf(stderr,                                  \
              "%s[%d] = %f (%s, line %d)\n",           \
              #val,                                    \
              i,                                       \
              val[i],                                  \
              g_basename(__FILE__),                    \
              __LINE__),                               \
      fflush(stderr);       }               } G_STMT_END

#endif

#if defined( DISABLE_CHECKS )

#define eh_require( expr ) { }
#define eh_require_msg( expr ) { }
#define eh_require_critical( expr ) { }
#define eh_require_msg_critical( expr ) { }
#define eh_require_not_reached( ) { }
#define eh_make_note( expr ) { (expr); }
#define eh_return_if_fail( expr ) { }
#define eh_return_val_if_fail( expr , val ) { }

#else

#define eh_require( expr )                                            \
   if ( !(expr) ) {                                                   \
      fprintf( stderr ,                                               \
               "%s : line %d : requirement failed : (%s)\n" ,         \
               g_basename(__FILE__),                                  \
               __LINE__,                                              \
               #expr ),                                               \
      fflush( stderr ); } else

#define eh_require_msg( expr , msg )                                  \
   if ( !(expr) ) {                                                   \
      fprintf( stderr ,                                               \
               "%s : line %d : requirement (%s) failed : (%s)\n" ,    \
               g_basename(__FILE__),                                  \
               __LINE__,                                              \
               (msg)?(msg):"NULL" ,                                   \
               #expr ),                                               \
      fflush( stderr ); } else
/*
#define eh_require( expr ) {                                          \
   if ( expr ) { } else {                                             \
      fprintf( stderr ,                                               \
               "%s : line %d : check failed : (%s)\n" ,               \
               g_basename(__FILE__),                                  \
               __LINE__,                                              \
               #expr ),                                               \
      fflush( stderr ); }  }
*/

#define eh_require_msg_critical( expr , msg )                         \
   if ( !(expr) ) { eh_require_msg( expr , msg ); eh_exit(-1); } else

#define eh_require_critical( expr )                                   \
   if ( !(expr) ) { eh_require( expr ); eh_exit(-1); } else

#define eh_require_not_reached( ) {                                   \
   fprintf( stderr ,                                                  \
            "%s : line %d : should not be reached\n" ,                \
            g_basename(__FILE__) ,                                    \
            __LINE__ ),                                               \
   fflush( stderr ); }

#define eh_make_note( expr ) {                                        \
   (expr),                                                            \
   fprintf( stderr ,                                                  \
            "%s : line %d : %s\n" ,                                   \
            g_basename(__FILE__) ,                                    \
            __LINE__ ,                                                \
            #expr ),                                                  \
   fflush( stderr ); }

#define eh_note_block( msg , expr )                                   \
   if ( (expr) )                                                      \
   {                                                                  \
      gchar* base_name = g_path_get_basename(__FILE__);               \
      eh_debug(                                                       \
               "%s : line %d : %s\n" ,                                \
               base_name ,                                            \
               __LINE__,                                              \
               (msg)?(msg):"NULL" ) ,                                 \
      eh_free( base_name );                                           \
      fflush( stderr );                                               \
   }                                                                  \
   if ( !(expr) ) {} else

#define eh_return_val_if_fail( expr , val ) if ( !(expr) ) { return (val); } else
#define eh_return_if_fail( expr ) if ( !(expr) ) { return; } else

#endif /* disable checks */

#ifdef __cplusplus
}
#endif

#endif /* eh_messages.h */
