#ifndef ICAL_H_
#define ICAL_H_

// Internal Type Definitions
#if 1
typedef unsigned char	UCHAR;
typedef signed char			INT8;
typedef INT8			int8_t;
typedef unsigned char	UINT8;
typedef UINT8			uint8_t;
typedef short			INT16;
typedef INT16			int16_t;
typedef unsigned short	UINT16;
typedef UINT16			uint16_t;
typedef int			INT32;
//typedef INT32			int32_t;
typedef unsigned int	UINT32;
//typedef UINT32			uint32_t;
#endif
#if 0
typedef int8_t			INT8;
typedef uint8_t			UINT8;
typedef int16_t			INT16;
typedef uint16_t		UINT16;
typedef int32_t			INT32;
typedef uint32_t		UINT32;
#endif

typedef float			REAL;
typedef long long		int64_t;
typedef int64_t			INT64;
typedef unsigned long long		uint64_t;
typedef uint64_t		UINT64;

typedef struct
{
	int16_t	Offset[3];		// Magnetic Hard Iron x, y, z offsets
	int16_t rr;
} TRANSFORM_T;

//#define QST_ICAL_CORE_DEBUG

#ifdef QST_ICAL_CORE_DEBUG
#define QST_PRINT(...) do{printf(__VA_ARGS__);}while(0)
#else
#define QST_PRINT(...) do{}while(0)
#endif

void qst_ical_init(REAL *calipara);

int convert_magnetic(REAL *raw, REAL *result,REAL *offset,REAL *RR,int8_t *accuracy);

int QST_ORI_progress(float *Acc,float *Mag,float *Ori,int8_t *accuracy);

#endif /* ICAL_H_ */
