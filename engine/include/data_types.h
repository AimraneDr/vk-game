#ifndef DATA_TYPES_H
#define DATA_TYPES_H
//Basic types

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef u8 bool;

#define true 1
#define false 0 

#define null 0


#define RESULT_CODE_SUCCESS 0U
#define RESULT_CODE_FAILED_DISPLAY_OPENING 1U
#define RESULT_CODE_FAILED_SYS_INIT 2U
#define RESULT_CODE_FAILED_DEBUG_MESSANGER_CREATION 3U
#define RESULT_CODE_NO_GPU_PRESENT 4U
#define RESULT_CODE_GPU_NOT_SUITABLE 5U
#define RESULT_CODE_FAILED_DEVICE_CREATION 6U
#define RESULT_CODE_FAILED_SURFACE_CREATION 7U
#define RESULT_CODE_FAILED_SWAPCHAIN_CREATION 8U
#define RESULT_CODE_UNDEFINED_ERROR (u16)-1

typedef u16 Result;

#endif //DATA_TYPES_H