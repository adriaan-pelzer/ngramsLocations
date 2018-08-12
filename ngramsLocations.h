#include <hashTableO1.h>

#ifndef _NGRAMS_LOCATIONS_H_
#define _NGRAMS_LOCATIONS_H_

typedef struct LatLong {
    long double lat;
    long double lon;
} LatLong_t;

typedef struct LatLongList {
    size_t size;
    LatLong_t *lat_lon;
    char *ngram;
} LatLongList_t;

typedef hashTable_t ngramsLocations_t;

ngramsLocations_t *ngramsLocations_create ( size_t size );
int ngramsLocations_add_lat_lon_list ( const char *ngram, LatLongList_t* lat_lon_list, ngramsLocations_t *ngrams_locations );
int ngramsLocations_serialise ( ngramsLocations_t *ngrams_locations, const char *filename );
ngramsLocations_t *ngramsLocations_deserialise ( const char *filename );
void ngramsLocations_free ( ngramsLocations_t *ngrams_locations );
void latLongList_free ( LatLongList_t* lat_lon_list );

#endif
