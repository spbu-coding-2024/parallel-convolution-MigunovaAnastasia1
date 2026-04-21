#include <stdio.h>
#include <stdbool.h>
#ifndef OPTIONS_H
#define OPTIONS_H

typedef enum
{
    FILTER_BLUR,     // размытие
    FILTER_SHARPEN,  // повышение резкости
    FILTER_EDGE,     // выделение границ
    FILTER_EMBOSS,   // тиснение
    FILTER_GAUSSIAN, // гауссово размытие
    FILTER_MOTION,   // размытие в движении
} FilterType;

typedef enum
{
    MODE_SEQ,    // последовательный
    MODE_PIXEL,  // попиксельный
    MODE_ROW,    // построчный
    MODE_COLUMN, // постолбцовый
    MODE_BLOCK,  // произвольная сетка
} ModeType;

typedef enum
{
    VAL_TYPE_INT, 
    VAL_TYPE_STRING, 
    VAL_TYPE_BOOL,   
    VAL_TYPE_FILTER, 
    VAL_TYPE_MODE   
} ValueType;

typedef struct
{
    ValueType type; 
    union
    { 
        const char *as_string;
        FilterType as_filter;
        int as_int;
        ModeType as_mode;
        bool as_bool;
    };

} OptionValue;

// Макрос для создания объектов OptionValue
#define VAL_INT(x) {.type = VAL_TYPE_INT, .as_int = x}
#define VAL_STRING(x) {.type = VAL_TYPE_STRING, .as_string = x}
#define VAL_BOOL(x) {.type = VAL_TYPE_BOOL, .as_bool = x}
#define VAL_FILTER(x) {.type = VAL_TYPE_FILTER, .as_filter = x}
#define VAL_MODE(x) {.type = VAL_TYPE_MODE, .as_mode = x}

typedef struct
{
    const char *cmd_name;
    OptionValue value;
    const char *description;
} Option;

// Макрос для создания и инициализации опции
#define OPTION(name_literal, default_value, desc_literal) \
    {.cmd_name = name_literal, .value = default_value, .description = desc_literal}

// ============ Структура для хранения аргументов ============
typedef struct
{
    Option input;
    Option filter;
    Option size;
    Option mode;
    Option clean;
    Option help;
} Options;

#endif
