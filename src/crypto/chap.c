/*
 * Copyright (C) 2006 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <gpxe/crypto.h>
#include <gpxe/chap.h>

/** @file
 *
 * CHAP protocol
 *
 */

/**
 * Initialise CHAP challenge/response
 *
 * @v chap		CHAP challenge/response
 * @v digest		Digest algorithm to use
 * @ret rc		Return status code
 *
 * Initialises a CHAP challenge/response structure.  This routine
 * allocates memory, and so may fail.  The allocated memory must
 * eventually be freed by a call to chap_finish().
 */
int chap_init ( struct chap_challenge *chap,
		struct crypto_algorithm *digest ) {
	size_t state_len;
	void *state;

	assert ( chap->digest == NULL );
	assert ( chap->digest_context == NULL );
	assert ( chap->response == NULL );

	DBG ( "CHAP %p initialising with %s digest\n", chap, digest->name );

	state_len = ( digest->ctxsize + digest->digestsize );
	state = malloc ( state_len );
	if ( ! state ) {
		DBG ( "CHAP %p could not allocate %zd bytes for state\n",
		      chap, state_len );
		return -ENOMEM;
	}
	
	chap->digest = digest;
	chap->digest_context = state;
	chap->response = ( state + digest->ctxsize );
	chap->response_len = digest->digestsize;
	digest_init ( chap->digest, chap->digest_context );
	return 0;
}

/**
 * Add data to the CHAP challenge
 *
 * @v chap		CHAP challenge/response
 * @v data		Data to add
 * @v len		Length of data to add
 */
void chap_update ( struct chap_challenge *chap, const void *data,
		   size_t len ) {
	assert ( chap->digest != NULL );
	assert ( chap->digest_context != NULL );

	if ( ! chap->digest )
		return;

	digest_update ( chap->digest, chap->digest_context, data, len );
}

/**
 * Respond to the CHAP challenge
 *
 * @v chap		CHAP challenge/response
 *
 * Calculates the final CHAP response value, and places it in @c
 * chap->response, with a length of @c chap->response_len.
 */
void chap_respond ( struct chap_challenge *chap ) {
	assert ( chap->digest != NULL );
	assert ( chap->digest_context != NULL );
	assert ( chap->response != NULL );

	DBG ( "CHAP %p responding to challenge\n", chap );

	if ( ! chap->digest )
		return;

	digest_final ( chap->digest, chap->digest_context, chap->response );
}

/**
 * Free resources used by a CHAP challenge/response
 *
 * @v chap		CHAP challenge/response
 */
void chap_finish ( struct chap_challenge *chap ) {
	void *state = chap->digest_context;

	DBG ( "CHAP %p finished\n", chap );

	free ( state );
	memset ( chap, 0, sizeof ( *chap ) );
}
