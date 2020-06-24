#include <png.h>
#include <stdio.h>


#define ERROR                                                   \
	fprintf (stderr, "ERROR at %s:%d.\n", __FILE__, __LINE__) ;   \
	return -1 ;                                                   \

extern void filter(unsigned char * M, unsigned char * W, int width, int height);

unsigned int pole;
unsigned int eax1;
unsigned int eax2;
int timetab[50];
int parametr[50];
int i;
double srednia;
extern int func();
extern int func2(); //niepotrzebna, gdy nie ma szansy na 'przekręcenie' rejestru eax więcej niż raz na pomiar

// Plik /proc/cpuinfo podaje, że mam taki
// procesor: Intel(R) Core(TM) i3 CPU       M 350  @ 2.27GHz
// Oznacza to, że jeden krok wykonuje się 1/2270000000 sekundy.
//
// Informacje o moim systemie operacyjnym i komputerze:
// Distributor ID:	Ubuntu
// Description:	Ubuntu 18.04.2 LTS
// Release:	18.04
// Codename:	bionic
//
// Architektura:           x86_64
// Tryb(y) pracy CPU:      32-bit, 64-bit
// Kolejność bajtów:       Little Endian
// 
//
// UWAGA!* Istnieje bardzo mała szansa, że podczas mierzenia czasu
// "przekręci" się wartość w rejestrze eax. Wtedy, czyli gdy
// wartość rejestru eax jest mniejsza na końcu mierzenia
// niż na końcu, trzeba zrobić następujące wylicznie
// prawdziwej zmiany wartości zwracanej przez
// instrukcje rdtsc: 4294967296 - eax1 + eax2
// eax1 - warartosc zwracana przez rdtsc do rejestru eax na początku mierzenia
// eax2 - warartosc zwracana przez rdtsc do rejestru eax na końcu mierzenia
// 4294967296 - maksymalna wartosc rejestru eax, czyli 2^32-1, powiększona o 1
// Czemu powiększona o 1?
// Załóżmy, że eax1 to 4294967294, a eax2 to 3.
// Gdyby obliczenie wyglądało tak: 4294967295 - eax1 + eax2, to wynikiem byłoby 4,
// jednak przecież wykonuje się 5 kroków:
// 4294967294, 4294967295, 0, 1, 2, 3;
// Dlatego, aby uwzględnić 0, trzeba dodać 1 do 4294967295
//
// * W zadaniach niemożliwe na nowoczesnych komputerach jest przekręcenie się wartości
// rejestru eax przy wymaganych pomiarach więcej niż raz,
// więc nie będzie liczona rzeczywista wartość,
// która wystąpiła w takich przypadkach. Wykonanie takiego obliczenia tylko
// skomplikowałoby program.
//
// Obliczanie czasu w nanosekundach:
// czas = liczba_kroków*1/2270000000*1000000000 = liczba_kroków/2.27

int main (int argc, char ** argv)
{
	if (2 != argc)
	{
		printf ("\nUsage:\n\n%s file_name.png\n\n", argv[0]) ;

		return 0 ;
	}

	const char * file_name = argv [1] ;
	
	#define HEADER_SIZE (1)
	unsigned char header [HEADER_SIZE] ;

	FILE *fp = fopen (file_name, "rb");
	if (NULL == fp)
	{
		fprintf (stderr, "Can not open file \"%s\".\n", file_name) ;
		ERROR
	}

	if (fread (header, 1, HEADER_SIZE, fp) != HEADER_SIZE)
	{
		ERROR
	}

	if (0 != png_sig_cmp (header, 0, HEADER_SIZE))
	{
		ERROR
	}

	png_structp png_ptr = 
		png_create_read_struct
			(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if (NULL == png_ptr)
	{
		ERROR
	}

	png_infop info_ptr = png_create_info_struct (png_ptr);
	if (NULL == info_ptr)
	{
		png_destroy_read_struct (& png_ptr, NULL, NULL);

		ERROR
	}

	if (setjmp (png_jmpbuf (png_ptr))) 
	{
		png_destroy_read_struct (& png_ptr, & info_ptr, NULL);

		ERROR
	}

	png_init_io       (png_ptr, fp);
	png_set_sig_bytes (png_ptr, HEADER_SIZE);
	png_read_info     (png_ptr, info_ptr);

	png_uint_32  width, height;
	int  bit_depth, color_type;
	
	png_get_IHDR
	(
		png_ptr, info_ptr, 
		& width, & height, & bit_depth, & color_type,
		NULL, NULL, NULL
	);

	if (8 != bit_depth)
	{
		ERROR
	}
	if (0 != color_type)
	{
		ERROR
	}

	size_t size = width ;
	size *= height ;

	unsigned char * M = malloc (size) ;

	png_bytep ps [height] ;
	ps [0] = M ;
	for (unsigned i = 1 ; i < height ; i++)
	{
		ps [i] = ps [i-1] + width ;
	}
	png_set_rows (png_ptr, info_ptr, ps);
	png_read_image (png_ptr, ps) ;

	printf 
	(
		"Image %s loaded:\n"
		"\twidth      = %lu\n"
		"\theight     = %lu\n"
		"\tbit_depth  = %u\n"
		"\tcolor_type = %u\n"
		, file_name, width, height, bit_depth, color_type
	) ;

	unsigned char * W = malloc (size) ;

	
	for(i=0; i<50; i++){ //50 pomiarow
	pole=width*height;
	eax1=func();

	filter (M, W, width, height) ;

	eax2=func();
	if(eax2<eax1){
	timetab[i] = (4294967296 - eax1 + eax2)/2.27;
	parametr[i] = (4294967296 - eax1 + eax2)/pole;
	}
	else {
	timetab[i] = (eax2 - eax1)/2.27;
	parametr[i] = (eax2-eax1)/pole;
	}
	}
	srednia=0;
	printf("\nCzas [nanosekundy] wykonania funkcji filter(): ");
	for(i=0; i<50; i++){printf(" %u", timetab[i]); srednia+=timetab[i];}
	printf("\nsrednia: %f\n", srednia/(double)50);
	srednia=0;
	printf("\nParametr - Liczba cykli zegara dla wykonania funkcji filter() podzielona na pole obrazu: ");
	for(i=0; i<50; i++){printf(" %u", parametr[i]); srednia+=parametr[i];}
	printf("\nsrednia: %f\n", srednia/(double)50);


	png_structp write_png_ptr =
		png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (NULL == write_png_ptr)
	{
		ERROR
	}

	for (unsigned i = 0 ; i < height ; i++)
	{
		ps [i] += W - M ;
	}
	png_set_rows (write_png_ptr, info_ptr, ps);

	FILE *fwp = fopen ("out.png", "wb");
	if (NULL == fwp)
	{
		ERROR
	}

	png_init_io   (write_png_ptr, fwp);
	png_write_png (write_png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	fclose (fwp);

	return 0;
}
