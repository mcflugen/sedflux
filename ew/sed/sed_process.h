//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#ifndef PROCESS_H
# define PROCESS_H

#include <glib.h>
#include "utils/utils.h"

#include "sed_cube.h"

G_BEGIN_DECLS

#define PROCESS_ALWAYS    ( 1 << 3 )
#define PROCESS_REGULAR   ( 1 << 2 )
#define PROCESS_IRREGULAR ( 1 << 1 )
#define PROCESS_SPECIFIED ( 1 << 0 )

#define SED_MAX_LOG_FILES (2)

#define SED_ERROR_MULTIPLE_PROCS      (1<<0)
#define SED_ERROR_PROC_ABSENT         (1<<1)
#define SED_ERROR_INACTIVE            (1<<2)
#define SED_ERROR_NOT_ALWAYS          (1<<3)
#define SED_ERROR_ABSENT_PARENT       (1<<4)
#define SED_ERROR_INACTIVE_PARENT     (1<<5)
#define SED_ERROR_MULTIPLE_PARENTS    (1<<6)
#define SED_ERROR_INACTIVE_CHILDREN   (1<<7)
#define SED_ERROR_DT_MISMATCH         (1<<8)

#define SED_PROC_DEFAULT        (0)
#define SED_PROC_UNIQUE         (1<<0)
#define SED_PROC_ACTIVE         (1<<1)
#define SED_PROC_ALWAYS         (1<<2)
#define SED_PROC_ACTIVE_PARENT  (1<<3)
#define SED_PROC_ACTIVE_CHILD   (1<<4)
#define SED_PROC_UNIQUE_PARENT  (1<<5)
#define SED_PROC_UNIQUE_CHILD   (1<<6)
#define SED_PROC_SAME_INTERVAL  (1<<7)

new_handle(Sed_process);
new_handle(Sed_process_queue);

typedef struct {
    // Public
    double   mass_added;
    double   mass_lost;
    gboolean error;
}
Sed_process_info;

typedef struct {
    const gchar* parent;
    const gchar* child;
}
Sed_process_family;

typedef struct {
    const gchar* parent;
    const gchar* child;
    gint         flags;
}
Sed_process_check;

const static Sed_process_info __empty_info = { 0, 0, FALSE };

#define SED_EMPTY_INFO __empty_info

typedef gboolean(*init_func)(Sed_process, Eh_symbol_table, GError**);
typedef Sed_process_info(*run_func)(Sed_process, Sed_cube);
typedef gboolean(*destroy_func)(Sed_process);

typedef gboolean(Sed_proc_init)(Sed_process, Eh_symbol_table, GError**);
typedef Sed_process_info(Sed_proc_run)(Sed_process, Sed_cube);
typedef gboolean(Sed_proc_destroy)(Sed_process);


//typedef gboolean         (*init_func)  (Eh_symbol_table,gpointer);
//typedef Sed_process_info (*run_func)   (gpointer,Sed_cube);

typedef gboolean(*load_func)(gpointer, FILE*);
typedef gboolean(*dump_func)(gpointer, FILE*);

typedef struct {
    const gchar* name;      //< The name of the process
    init_func    init_f;    //< Function that initialize the process
    run_func     run_f;     //< Function that runs the process
    destroy_func destroy_f; //< Function that destroys the process
}
Sed_process_init_t;

typedef enum {
    SED_PROC_ERROR_BAD_INIT_FILE,
    SED_PROC_ERROR_NOT_FOUND,
    SED_PROC_ERROR_MISSING_PARENT
}
Sed_process_error;

#define SED_PROC_ERROR sed_process_error_quark()

Sed_process
sed_process_create(const char*  name,
    init_func    f_init,
    run_func     f_run,
    destroy_func f_destroy);
Sed_process
sed_process_copy(Sed_process d, Sed_process s);
Sed_process
sed_process_dup(Sed_process s);
Sed_process
sed_process_destroy(Sed_process p);
void
sed_process_clean(Sed_process p);
double
sed_process_next_event(Sed_process p);
Sed_process
sed_process_set_next_event(Sed_process p, double new_next_event);
gboolean
sed_process_is_expired(Sed_process, double);
gboolean
sed_process_is_on(Sed_process p, double time);
gboolean
sed_process_array_run(GPtrArray* a, Sed_cube);
gboolean
sed_process_run(Sed_process, Sed_cube);
gboolean
sed_process_run_at_end(Sed_process, Sed_cube);
gboolean
sed_process_run_now(Sed_process, Sed_cube);
void
sed_process_init(Sed_process a, Eh_symbol_table symbol_table, GError** error);
GList*
sed_process_scan(Eh_key_file k, Sed_process p, GError** error);
gssize
sed_process_fprint(FILE* fp, Sed_process p);
gssize
sed_process_queue_fprint(FILE* fp, Sed_process_queue q);
gssize
sed_process_queue_summary(FILE* fp, Sed_process_queue q);
gssize
sed_process_queue_size(Sed_process_queue q);
gssize
sed_process_queue_n_active(Sed_process_queue q);
gssize
sed_process_queue_n_absent(Sed_process_queue q);
gssize
sed_process_queue_n_inactive(Sed_process_queue q);

gpointer
sed_process_data(Sed_process p);
void
sed_process_provide(Sed_process p, GQuark key, gpointer data);
void
sed_process_withhold(Sed_process p, GQuark key);
gpointer
sed_process_use(Sed_process p, GQuark key);

#define sed_process_new_user_data( p , t ) ( (t*)sed_process_malloc_user_data(p,sizeof(t)) )

Sed_process
sed_process_child(Sed_process p, const gchar* child_s);
Sed_process
sed_process_append_child(Sed_process p, Sed_process c);
gboolean
sed_process_is_parent(Sed_process p);
double
sed_process_interval(Sed_process p);
gboolean
sed_process_interval_is_always(Sed_process p);
gboolean
sed_process_interval_is_at_end(Sed_process p);
gchar*
sed_process_name(Sed_process p);
gchar*
sed_process_prefix(Sed_process p);
gint
sed_process_run_count(Sed_process p);
gboolean
sed_process_is_set(Sed_process p);
gpointer
sed_process_user_data(Sed_process p);
gpointer
sed_process_malloc_user_data(Sed_process p, gssize n_bytes);
gboolean
sed_process_name_is_same(Sed_process a, Sed_process b);
gboolean
sed_process_is_active(Sed_process p);
void
sed_process_set_inactive(Sed_process p);

gssize
sed_process_fprint_info(FILE* fp, Sed_process p);
gssize
sed_process_summary(FILE* fp, Sed_process p);
gboolean
sed_process_error(Sed_process p);

int
sed_process_queue_check_item(Sed_process_queue, const gchar*);
int
sed_process_queue_check_family(Sed_process_queue,
    const gchar* parent,
    const gchar* child);
double
sed_process_queue_item_interval(Sed_process_queue q, const gchar* name);

Sed_process_queue
sed_process_queue_new(void);
Sed_process_queue
sed_process_queue_dup(Sed_process_queue);
Sed_process_queue
sed_process_queue_copy(Sed_process_queue, Sed_process_queue);
Sed_process_queue
sed_process_queue_destroy(Sed_process_queue);
char*
sed_process_queue_names(Sed_process_init_t p_list[]);
Sed_process_queue
sed_process_queue_init(const gchar* file,
    const gchar* prefix,
    Sed_process_init_t* p_list,
    Sed_process_family p_family[],
    Sed_process_check p_check[],
    GError** error);
Sed_process_queue
sed_process_queue_set_families(Sed_process_queue q, Sed_process_family f[],
    GError** error);
Sed_process_queue
sed_process_queue_scan(Sed_process_queue, Eh_key_file, GError**);
Sed_process_queue
sed_process_queue_remove(Sed_process_queue, gchar*);
Sed_process_queue
sed_process_queue_delete(Sed_process_queue, const gchar*);
Sed_process_queue
sed_process_queue_run(Sed_process_queue, Sed_cube);
Sed_process_queue
sed_process_queue_run_until(Sed_process_queue q, Sed_cube p, double t_total);
Sed_process_queue
sed_process_queue_run_at_end(Sed_process_queue q, Sed_cube p);
Sed_process_queue
sed_process_queue_run_process_now(Sed_process_queue q,
    const gchar*      name,
    Sed_cube          cube);

Sed_process
sed_process_queue_find_nth_obj(Sed_process_queue q,
    const gchar* name,
    gssize n);
Sed_process_queue
sed_process_queue_push(Sed_process_queue q,
    Sed_process_init_t init);
gpointer*
sed_process_queue_obj_data(Sed_process_queue q, const char* name);
Sed_process_queue
sed_process_queue_activate(Sed_process_queue q,
    const gchar* name);
Sed_process_queue
sed_process_queue_deactivate(Sed_process_queue q,
    const gchar* name);
Sed_process_queue
sed_process_queue_set_active(Sed_process_queue q,
    const gchar* name,
    gboolean val);
gboolean
sed_process_queue_validate(Sed_process_queue q, Sed_process_check check[],
    GError** error);

#define sed_process_new(name,type,f_init,f_run) ( \
    sed_process_create( name , sizeof(type) , f_init , f_run ) )
#define sed_process_data_val(p,member,type) ( ((type*)(sed_process_data(p)))->member )

//#define sed_new_process(name,type,f_init,f_run,f_load,f_dump) ( sed_process_create(name,sizeof(type),f_init,f_run,f_load,f_dump) )

//#define sed_process_data_val(p,member,type) ( ((type*)((p)->data))->member )

G_END_DECLS

#endif /* process.h is included */
