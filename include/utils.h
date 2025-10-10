#ifndef UTILS_H
#include "raygui.h"
#define UTILS_H
#define GRID_COLS 12.0F
#define GRID_ROWS 12.0F
#define GRID_PADDING 10.0F
#define GRID_PADDING 10.0F
#define CELL_MARGIN 12.0F
#include <stddef.h>
#define SIZE_COLS 12
#define SIZE_ROWS 12

#define STATE_TABLE                                                      \
  X(STATE_INIT)                                                          \
  X(STATE_IDLE)                                                          \
  X(STATE_HOVERING)                                                      \
  X(STATE_SELECTED)                                                      \
  X(STATE_ERROR)                                                         \
  X(INVALID_STATE)                                                       \
  X(NUM_STATES)

#define EVENT_TABLE                                                       \
  X(evt_font_loaded)                                                      \
  X(evt_font_failed)                                                      \
  X(evt_mouse_hover_char)                                                 \
  X(evt_mouse_leave_char)                                                 \
  X(evt_click_char)                                                       \
  X(evt_click_same_char)                                                  \
  X(evt_click_empty)                                                      \
  X(NUM_EVENTS)

#define X(state) state,
typedef enum
{
  STATE_TABLE
} State;
#undef X

#define X(event) event,
typedef enum
{
  EVENT_TABLE
} Event;
#undef X

extern const char *state_name[];
extern const char *event_name[];

extern State transition_table[NUM_STATES][NUM_EVENTS];
typedef struct 
{
 int select_start;
 int select_length;
 int hovered_char;
 int char_count;
 Vector2 *char_positions;
 float *char_widths;
} SelectionContext;



typedef struct
{
  State current_state;
  SelectionContext context; 
  int font_loaded;
} Machine;

#define ELEMENT_LIST                                                           \
  X(ELMNT_BLANK)                                                               \
  X(TOGGLE_GROUP)                                                              \
  X(ELMNT_NUM)

#define X(element) element,
typedef enum
{
  ELEMENT_LIST
} Element;
#undef X

extern const char *element_list[];

typedef struct
{
  char *id;
  char *content;
  size_t size_id;
  size_t size_content;
} NodeData;

typedef struct
{
  NodeData *nodes;
  int size;
} ChapterData;


typedef struct 
{
 int codepoint_count;
 int *codepoints;
 Font font_japanese;
 Vector2 position;
 char *text;
} TextData;

void init_machine(Machine *machine);
State update_state(Machine *machine, Event event);
Event process_input(Machine *machine, TextData *text_data);
void render_state(Machine *machine, TextData *text_data);
void init_text(const char text[], TextData *text_data);
void cleanup_text(TextData *text_data);
void cleanup_machine(Machine *machine);

void setup_raylib(void);
void grid_layout(Machine *machine);
int (*Return_Map_Pr(State state))[SIZE_ROWS][SIZE_COLS];
int *CodepointRemoveDuplicates(int *codepoints, int codepointCount, int *codepointResultCount);
char *chapter_data_to_string(const ChapterData *chapter);
ChapterData get_chapter_data(const char *filename);

#endif

