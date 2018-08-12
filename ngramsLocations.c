#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <hashTableO1.h>

#include "ngramsLocations.h"

void latLongList_free ( LatLongList_t* lat_lon_list ) {
    if ( lat_lon_list ) {
        if ( lat_lon_list->ngram )
            free ( lat_lon_list->ngram );
        if ( lat_lon_list->lat_lon )
            free ( lat_lon_list->lat_lon );
        free ( lat_lon_list );
    }
}

int ngramsLocations_add_lat_lon_list ( const char *ngram, LatLongList_t* lat_lon_list, hashTable_t *ngrams_locations ) {
    int rc = EXIT_FAILURE;
    int _rc = HT_FAILURE;

    if ( ( _rc = hashTable_add_entry ( ngrams_locations, ngram, (void *) lat_lon_list ) ) == HT_FAILURE ) {
        fprintf ( stderr, "Cannot add entry '%s'\n", ngram );
        goto over;
    }
    
    if ( _rc == HT_EXISTS ) {
        LatLongList_t *lll = NULL;
        size_t i = 0, j = 0, realSize = 0;;

        if ( ( lll = (LatLongList_t *) hashTable_find_entry_value ( ngrams_locations, ngram ) ) == NULL ) {
            goto over;
        }

        if ( ( lll->lat_lon = realloc ( lll->lat_lon, ( lll->size + lat_lon_list->size ) * sizeof ( LatLong_t ) ) ) == NULL ) {
            goto over;
        }

        realSize = lll->size;

        for ( i = 0; i < lat_lon_list->size; i++ ) {
            char exists = 0;

            for ( j = 0; j < realSize; j++ ) {
                if ( lll->lat_lon[j].lat == lat_lon_list->lat_lon[i].lat && lll->lat_lon[j].lon == lat_lon_list->lat_lon[i].lon ) {
                    exists = 1;
                    break;
                }
            }

            if ( ! exists ) {
                lll->lat_lon[realSize].lat = lat_lon_list->lat_lon[i].lat;
                lll->lat_lon[realSize].lon = lat_lon_list->lat_lon[i].lon;
                realSize++;
            }
        }

        if ( ( lll->lat_lon = realloc ( lll->lat_lon, realSize * sizeof ( LatLong_t ) ) ) == NULL ) {
            goto over;
        }

        lll->size = realSize;
        free ( lat_lon_list->lat_lon );
        free ( lat_lon_list );
    }

    rc = EXIT_SUCCESS;
over:
    if ( rc == EXIT_FAILURE && lat_lon_list ) {
        free ( lat_lon_list->lat_lon );
        free ( lat_lon_list );
    }

    return rc;
}

int ngramsLocations_serialise ( hashTable_t *ngrams_locations, const char *filename ) {
    int rc = EXIT_FAILURE;
    FILE *fp = NULL;
    size_t i = 0, j = 0;

    if ( ( fp = fopen ( filename, "w" ) ) == NULL ) {
        fprintf ( stderr, "Cannot open file %s for writing\n", filename );
        goto over;
    }

    if ( fwrite ( &ngrams_locations->size, sizeof ( size_t ), 1, fp ) != 1 ) {
        fprintf ( stderr, "Cannot write hash table size to file\n" );
        goto over;
    }

    for ( i = 0; i < ngrams_locations->size; i++ ) {
        hashTable_entry_t *entry = ngrams_locations->entries[i];

        while ( entry ) {
            LatLongList_t *lll = (LatLongList_t *) entry->value;
            size_t key_len = strlen ( entry->key );

            if ( fwrite ( &key_len, sizeof ( size_t ), 1, fp ) != 1 ) {
                fprintf ( stderr, "Cannot write entry %d key length (%d) to file\n", (int) i, (int) key_len );
                goto over;
            }

            if ( fwrite ( entry->key, strlen ( entry->key ), 1, fp ) != 1 ) {
                fprintf ( stderr, "Cannot write entry %d key (%s) to file\n", (int) i, entry->key );
                goto over;
            }

            if ( fwrite ( &lll->size, sizeof ( size_t ), 1, fp ) != 1 ) {
                fprintf ( stderr, "Cannot write entry %d size (%d) to file\n", (int) i, (int) lll->size );
                goto over;
            }

            for ( j = 0; j < lll->size; j++ ) {
                if ( fwrite ( &lll->lat_lon[j].lat, sizeof ( long double ), 1, fp ) != 1 ) {
                    fprintf ( stderr, "Cannot write entry %d point %d latitude (%lf) to file\n", (int) i, (int) j, (double) lll->lat_lon[j].lat );
                    goto over;
                }

                if ( fwrite ( &lll->lat_lon[j].lon, sizeof ( long double ), 1, fp ) != 1 ) {
                    fprintf ( stderr, "Cannot write entry %d point %d longitude (%lf) to file\n", (int) i, (int) j, (double) lll->lat_lon[j].lon );
                    goto over;
                }
            }

            entry = entry->next;
        }
    }

    rc = EXIT_SUCCESS;
over:
    if ( fp )
        fclose ( fp );

    return rc;
}

hashTable_t *ngramsLocations_deserialise ( const char *filename ) {
    hashTable_t *ngrams_locations = NULL, *_ht = NULL;
    LatLongList_t *lat_lon_list = NULL;
    FILE *fp = NULL;
    size_t i = 0, tableSize = 0;

    if ( ( fp = fopen ( filename, "r" ) ) == NULL ) {
        fprintf ( stderr, "Cannot open file %s for writing\n", filename );
        goto over;
    }

    if ( fread ( &tableSize, sizeof ( size_t ), 1, fp ) != 1 ) {
        fprintf ( stderr, "Cannot read hash table size from file\n" );
        goto over;
    }

    if ( ( _ht = hashTable_create ( tableSize ) ) == NULL ) {
        fprintf ( stderr, "Cannot create ngrams -> locations hashTable\n" );
        goto over;
    }

    while ( ! feof ( fp ) ) {
        char *key = NULL;
        size_t key_len = 0;
        hashTable_rc_t hashTable_rc = HT_FAILURE;

        if ( fread ( &key_len, sizeof ( size_t ), 1, fp ) != 1 ) {
            if ( feof ( fp ) )
                break;
            fprintf ( stderr, "Cannot read entry key length from file\n" );
            goto over;
        }

        if ( ( key = calloc ( 1, key_len + 1 ) ) == NULL ) {
            fprintf ( stderr, "Cannot allocate memory for key\n" );
            goto over;
        }

        if ( fread ( key, key_len, 1, fp ) != 1 ) {
            fprintf ( stderr, "Cannot read entry key from file\n" );
            goto over;
        }

        if ( ( lat_lon_list = calloc ( 1, sizeof ( LatLongList_t ) ) ) == NULL ) {
            fprintf ( stderr, "Cannot allocate memory for lat/lon list\n" );
            goto over;
        }

        if ( fread ( &lat_lon_list->size, sizeof ( size_t ), 1, fp ) != 1 ) {
            fprintf ( stderr, "Cannot read lat/lon list size from file\n" );
            goto over;
        }

        if ( ( lat_lon_list->lat_lon = calloc ( 1, lat_lon_list->size * sizeof ( LatLong_t ) ) ) == NULL ) {
            fprintf ( stderr, "Cannot allocate memory for lat/lon array: %s\n", strerror ( errno ) );
            goto over;
        }

        for ( i = 0; i < lat_lon_list->size; i++ ) {
            if ( fread ( &lat_lon_list->lat_lon[i].lat, sizeof ( long double ), 1, fp ) != 1 ) {
                fprintf ( stderr, "Cannot read entry point %d latitude from file\n", (int) i );
                goto over;
            }

            if ( fread ( &lat_lon_list->lat_lon[i].lon, sizeof ( long double ), 1, fp ) != 1 ) {
                fprintf ( stderr, "Cannot read entry point %d longitude from file\n", (int) i );
                goto over;
            }
        }

        if ( ( hashTable_rc = hashTable_add_entry ( _ht, (const char *) key, (void *) lat_lon_list ) ) != HT_SUCCESS ) {
            fprintf ( stderr, "Cannot add entry to hash table: %d\n", (int) hashTable_rc );
            goto over;
        }

        lat_lon_list = NULL;
        free ( key );
    }

    ngrams_locations = _ht;
over:
    if ( lat_lon_list ) {
        if ( lat_lon_list->lat_lon )
            free ( lat_lon_list->lat_lon );
        free ( lat_lon_list );
    }

    if ( _ht && _ht != ngrams_locations )
        hashTable_free ( _ht );

    if ( fp )
        fclose ( fp );

    return ngrams_locations;
}

ngramsLocations_t *ngramsLocations_create ( size_t size ) {
    return (ngramsLocations_t *) hashTable_create ( size );
}

void ngramsLocations_free ( ngramsLocations_t *ngrams_locations ) {
    hashTable_free ( (hashTable_t *) ngrams_locations );
}
