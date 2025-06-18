/*
 * miniz.h
 *
 * Erstellt von Ruben Hockemeyer und Ruben Grest auf Grundlage von
 * http://code.google.com/p/miniz/source/browse/trunk/example1.c
 * (Public domain, May 15 2011, Rich Geldreich, richgel99@gmail.com).
 *
 * 22.10.2014
 */

#ifndef MINIZ_H_
#define MINIZ_H_

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

typedef struct result
{
	const char *data;
	unsigned long length;
} Result;

/*
 * Es wird eine "Instanz" der struct Result auf dem Heap erzeugt.
 * Dann wird der String koprimiert und das Ergebnis im Result im Feld "data"
 * gespeichert (dafür wird neuer Heap-Speicher alloziert). Die Länge des komprimierten
 * Inhalts wird im Feld "length" gespeichert.
 *
 * Es wird ein Pointer auf o.g. Result zurückgeliefert oder NULL, wenn kein
 * Speicher mehr reserviert werden konnte oder das Komprimieren fehlschlug.
 *
 * Wird das Result nicht mehr gebraucht, so muss zuerst das Feld "data" und danach
 * das Result selbst durch free() freigegeben werden.
 */
const Result *compress_string(const char *string)
{
	unsigned long length = (unsigned long) strlen(string);
	unsigned long compression_bounds = compressBound(length);
	unsigned char *buffer = (unsigned char *) malloc((size_t) compression_bounds);

	if (!buffer)
	{
		perror("Es konnte kein Speicher reserviert werden.");
		return NULL;
	}

	int status = compress(buffer, &compression_bounds, (unsigned char *) string, length);
	if (status != Z_OK)
	{
	  perror("Fehler beim Komprimieren!");
	  free(buffer);
	  return NULL;
	}

	Result *result = (Result *) malloc(sizeof(Result));

	if (!result)
	{
		perror("Es konnte kein Speicher reserviert werden.");
		free(buffer);
		return NULL;
	}

	result->data = (const char *) buffer;
	result->length = compression_bounds;

	return result;
}

#endif /* MINIZ_H_ */
