#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ngramsLocations.h>

int main ( int argc, char **argv ) {
    int rc = EXIT_FAILURE;
    char *infile = NULL, *outfile = NULL;
    ngramsLocations_t *ngramsLocations = NULL;

    if ( argc < 3 ) {
        fprintf ( stderr, "Usage: %s infile outfile\n", argv[0] );
        goto over;
    }

    infile = argv[1];
    outfile = argv[2];

    if ( ( ngramsLocations = ngramsLocations_deserialise ( infile ) ) == NULL ) {
        fprintf ( stderr, "Cannot deserialise file %s: %s\n", infile, strerror ( errno ) );
        goto over;
    }

    if ( ngramsLocations_serialise ( ngramsLocations, outfile ) != EXIT_SUCCESS ) {
        fprintf ( stderr, "Cannot serialise to file %s: %s\n", outfile, strerror ( errno ) );
        goto over;
    }

    rc = EXIT_SUCCESS;
over:
    if ( ngramsLocations )
        ngramsLocations_free ( ngramsLocations );

    return rc;
}
