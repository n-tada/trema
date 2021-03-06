/*
 * Author: Shin-ya Zenke
 *
 * Copyright (C) 2008-2011 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "checks.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#ifdef pthread_mutex_init
#undef pthread_mutex_init
#endif
#define pthread_mutex_init mock_pthread_mutex_init
int mock_pthread_mutex_init( pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr );

#ifdef pthread_mutexattr_init
#undef pthread_mutexattr_init
#endif
#define pthread_mutexattr_init mock_pthread_mutexattr_init
int mock_pthread_mutexattr_init( pthread_mutexattr_t *attr );

#ifdef pthread_mutexattr_settype
#undef pthread_mutexattr_settype
#endif
#define pthread_mutexattr_settype mock_pthread_mutexattr_settype
int mock_pthread_mutexattr_settype( pthread_mutexattr_t *attr, int kind );

#ifdef pthread_mutex_lock
#undef pthread_mutex_lock
#endif
#define pthread_mutex_lock mock_pthread_mutex_lock
int mock_pthread_mutex_lock( pthread_mutex_t *mutex );

#ifdef pthread_mutex_unlock
#undef pthread_mutex_unlock
#endif
#define pthread_mutex_unlock mock_pthread_mutex_unlock
int mock_pthread_mutex_unlock( pthread_mutex_t *mutex );

#ifdef pthread_mutex_destroy
#undef pthread_mutex_destroy
#endif
#define pthread_mutex_destroy mock_pthread_mutex_destroy
int mock_pthread_mutex_destroy( pthread_mutex_t *mutex );

#endif // UNIT_TESTING


typedef struct private_buffer {
  buffer public;
  size_t real_length;
  void *top; // pointer to the head of user data area. only valid if public.data is allocated.
  pthread_mutex_t *mutex;
} private_buffer;


static size_t
front_length_of( const private_buffer *pbuf ) {
  assert( pbuf != NULL );

  return ( size_t ) ( ( char * ) pbuf->public.data - ( char * ) pbuf->top );
}


static bool
already_allocated( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  size_t required_length = ( size_t ) front_length_of( pbuf ) + pbuf->public.length + length;

  return ( pbuf->real_length >= required_length );
}


static private_buffer *
alloc_new_data( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  pbuf->public.data = xmalloc( length );
  pbuf->public.length = length;
  pbuf->top = pbuf->public.data;
  pbuf->real_length = length;

  return pbuf;
}


static private_buffer *
alloc_private_buffer() {
  private_buffer *new_buf = xcalloc( 1, sizeof( private_buffer ) );

  new_buf->public.data = NULL;
  new_buf->public.length = 0;
  new_buf->public.user_data = NULL;
  new_buf->top = NULL;
  new_buf->real_length = 0;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  new_buf->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( new_buf->mutex, &attr );

  return new_buf;
}


static private_buffer *
append_front( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  void *new_data = xmalloc( front_length_of( pbuf ) + pbuf->public.length + length );
  memcpy( ( char * ) new_data + front_length_of( pbuf ) + length, pbuf->public.data, pbuf->public.length );
  xfree( pbuf->top );

  pbuf->public.data = ( char * ) new_data + front_length_of( pbuf );
  pbuf->real_length = sizeof( new_data );
  pbuf->top = new_data;

  return pbuf;
}


static private_buffer *
append_back( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  void *new_data = xmalloc( front_length_of( pbuf ) + pbuf->public.length + length );
  memcpy( ( char * ) new_data + front_length_of( pbuf ), pbuf->public.data, pbuf->public.length );
  xfree( pbuf->top );

  pbuf->public.data = ( char * ) new_data + front_length_of( pbuf );
  pbuf->real_length = sizeof( new_data );
  pbuf->top = new_data;

  return pbuf;
}


buffer *
alloc_buffer() {
  return ( buffer * ) alloc_private_buffer();
}


buffer *
alloc_buffer_with_length( size_t length ) {
  assert( length != 0 );

  private_buffer *new_buf = xcalloc( 1, sizeof( private_buffer ) );
  new_buf->public.data = xmalloc( length );
  new_buf->public.length = 0;
  new_buf->public.user_data = NULL;
  new_buf->top = new_buf->public.data;
  new_buf->real_length = length;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  new_buf->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( new_buf->mutex, &attr );

  return ( buffer * ) new_buf;
}


void
free_buffer( buffer *buf ) {
  assert( buf != NULL );

  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );
  private_buffer *delete_me = ( private_buffer * ) buf;
  if ( delete_me->top != NULL ) {
    xfree( delete_me->top );
  }
  pthread_mutex_unlock( delete_me->mutex );
  pthread_mutex_destroy( delete_me->mutex );
  xfree( delete_me->mutex );
  xfree( delete_me );
}


void *
append_front_buffer( buffer *buf, size_t length ) {
  assert( buf != NULL );
  assert( length != 0 );

  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );

  private_buffer *pbuf = ( private_buffer * ) buf;

  if ( pbuf->top == NULL ) {
    alloc_new_data( pbuf, length );
    pthread_mutex_unlock( pbuf->mutex );
    return pbuf->public.data;
  }

  buffer *b = &( pbuf->public );
  if ( already_allocated( pbuf, length ) ) {
    memmove( ( char * ) b->data + length, b->data, b->length );
    memset( b->data, 0, length );
  } else {
    append_front( pbuf, length );
  }
  b->length += length;

  pthread_mutex_unlock( pbuf->mutex );

  return b->data;
}


void *
remove_front_buffer( buffer *buf, size_t length ) {
  assert( buf != NULL );
  assert( length != 0 );

  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );

  private_buffer *pbuf = ( private_buffer * ) buf;
  assert( pbuf->public.length >= length );

  pbuf->public.data = ( char * ) pbuf->public.data + length;
  pbuf->public.length -= length;

  pthread_mutex_unlock( pbuf->mutex );

  return pbuf->public.data;
}


void *
append_back_buffer( buffer *buf, size_t length ) {
  assert( buf != NULL );
  assert( length != 0 );

  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );

  private_buffer *pbuf = ( private_buffer * ) buf;

  if ( pbuf->real_length == 0 ) {
    alloc_new_data( pbuf, length );
    pthread_mutex_unlock( pbuf->mutex );
    return ( char * ) pbuf->public.data;
  }
 
  if ( !already_allocated( pbuf, length ) ) {
    append_back( pbuf, length );
  }

  void *appended = ( char * ) pbuf->public.data + pbuf->public.length;
  pbuf->public.length += length;

  pthread_mutex_unlock( pbuf->mutex );

  return appended;
}


buffer *
duplicate_buffer( const buffer *buf ) {
  assert( buf != NULL );

  pthread_mutex_lock( ( ( const private_buffer * ) buf )->mutex );

  private_buffer *new_buffer = alloc_private_buffer();
  const private_buffer *old_buffer = ( const private_buffer * ) buf;

  if ( old_buffer->real_length == 0 ) {
    pthread_mutex_unlock( old_buffer->mutex );
    return ( buffer * ) new_buffer;
  }

  alloc_new_data( new_buffer, old_buffer->real_length );
  memcpy( new_buffer->top, old_buffer->top, old_buffer->real_length );

  new_buffer->public.length = old_buffer->public.length;
  new_buffer->public.user_data = old_buffer->public.user_data;
  new_buffer->public.data = ( char * ) ( new_buffer->public.data ) + front_length_of( old_buffer );

  pthread_mutex_unlock( old_buffer->mutex );

  return ( buffer * ) new_buffer;
}


void
dump_buffer( const buffer *buf, void dump_function( const char *format, ... ) ) {
  assert( dump_function != NULL );

  pthread_mutex_lock( ( ( const private_buffer * ) buf )->mutex );

  char *hex = xmalloc( sizeof( char ) * ( buf->length * 2 + 1 ) );
  char *datap = buf->data;
  char *hexp = hex;
  for ( unsigned int i = 0; i < buf->length; i++, datap++, hexp += 2 ) {
    snprintf( hexp, 3, "%02x", *datap );
  }
  ( *dump_function )( "%s", hex );

  xfree( hex );

  pthread_mutex_unlock( ( ( const private_buffer * ) buf )->mutex );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
