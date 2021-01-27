

#include <stdlib.h>
#ifdef __linux__
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "appender.h"

/* This could be public */
struct mmap_info {
    const char*	name;
    int		fd;
    size_t	length;
    void*	addr;
    void*	ptr;
    struct stat st;
};

static int mmap_info_delete(struct mmap_info* a_minfo);
static struct mmap_info* mmap_info_new(const char* a_name);

/*******************************************************************************/
static struct mmap_info* mmap_info_new(const char* a_name)
{
    struct mmap_info* minfo = NULL;

    minfo = calloc(1, sizeof(*minfo));
    minfo->name = a_name;

    if ( (minfo->fd = open(minfo->name, O_RDWR, 0644)) == -1) {
	perror("open");
	mmap_info_delete(minfo);
	return NULL;
    }

    if (fstat(minfo->fd, &minfo->st) == -1) {
	perror("fstat");
	mmap_info_delete(minfo);
	return NULL;
    }

    minfo->length = minfo->st.st_size;

    if (!minfo->length) {
	fprintf(stderr, "file %s is empty", minfo->name);
	mmap_info_delete(minfo);
	return NULL;
    }

    return minfo;
}
/*******************************************************************************/
static int mmap_info_delete(struct mmap_info* a_minfo)
{
    if (!a_minfo)
	return -1;

    close(a_minfo->fd);
    free(a_minfo);
    return 0;
}

/*******************************************************************************/
static int mmap_open(log_appender_t* this)
{
    struct mmap_info* minfo = log_appender_get_udata(this);
    
    if (minfo)
	return 0;

    if ( (minfo = mmap_info_new(log_appender_get_name(this))) == NULL)
	return -1;

    minfo->addr = mmap(NULL, minfo->length, PROT_READ|PROT_WRITE,
		      MAP_SHARED, minfo->fd, 0);
    if (minfo->addr == NULL) {
	perror("mmap");
	mmap_info_delete(minfo);
	return -1;
    }

    minfo->ptr = minfo->addr;
    log_appender_set_udata(this, minfo);
    return 0;
}

/*******************************************************************************/
static int mmap_append(log_appender_t*	this,
               const log_logging_event_t* a_event)
{
    size_t size, available;
    struct mmap_info* minfo = log_appender_get_udata(this);

    if (!minfo && !minfo->ptr)
	return 0;

    size = strlen(a_event->evt_rendered_msg);
    available = ((char *)minfo->addr + minfo->length) - (char *)minfo->ptr;

    if (size > available) {
	memcpy(minfo->ptr, a_event->evt_rendered_msg, available);
	minfo->ptr = minfo->addr;
	size -= available;
    }

    memcpy(minfo->ptr, a_event->evt_rendered_msg, size);
    minfo->ptr = (char *)minfo->ptr + size;
    return 0;
}

/*******************************************************************************/
static int mmap_close(log_appender_t*	this)
{
    struct mmap_info* minfo = log_appender_get_udata(this);

    if (!minfo)
	return 0;

    if (munmap(minfo->addr, minfo->length) == -1)
	perror("munmap");

    mmap_info_delete(minfo);

    log_appender_set_udata(this, NULL);
    return 0;
}

/*******************************************************************************/
const log_appender_type_t log_appender_type_mmap = {
    "mmap",
    mmap_open,
    mmap_append,
    mmap_close,
};

