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

#define STATE_TABLE_APP                                                  \
  X(STATE_CHAPTERS)                                                      \
  X(STATE_VOCABULARY)                                                    \
  X(STATE_DICTIONARY)                                                    \
  X(STATE_PRACTICE)                                                      \
  X(STATE_PROGRESS)                                                      \
  X(INVALID_STATE_APP)                                                   \
  X(NUM_STATES_APP)

#define EVENT_TABLE_APP                                                  \
  X(evt_click_chapters)                                                  \
  X(evt_click_vocabulary)                                                \
  X(evt_click_dictionary)                                                \
  X(evt_click_practice)                                                  \
  X(evt_click_progress)                                                  \
  X(evt_click_j_text)                                                    \
  X(NUM_EVENTS_APP)

#define STATE_TABLE_J_TEXT                                               \
  X(STATE_INIT)                                                          \
  X(STATE_IDLE)                                                          \
  X(STATE_HOVERING)                                                      \
  X(STATE_SELECTED)                                                      \
  X(STATE_SELECTED_HOVERING)                                             \
  X(STATE_ERROR)                                                         \
  X(INVALID_STATE_J_TEXT)                                                \
  X(NUM_STATES_J_TEXT)

#define EVENT_TABLE_J_TEXT                                                \
  X(evt_font_loaded)                                                      \
  X(evt_font_failed)                                                      \
  X(evt_mouse_hover_char)                                                 \
  X(evt_mouse_leave_char)                                                 \
  X(evt_click_char)                                                       \
  X(evt_click_same_char)                                                  \
  X(evt_click_empty)                                                      \
  X(NUM_EVENTS_J_TEXT)


#define ELEMENT_LIST                                                      \
  X(ELMNT_BLANK)                                                          \
  X(TOGGLE_GROUP)                                                         \
  X(ELMNT_CHAPTERS)                                                       \
  X(J_TEXT)                                                               \
  X(ELMNT_NUM)


#define X(state) state,
typedef enum
{
  STATE_TABLE_APP
} StateApp;
#undef X

#define X(event) event,
typedef enum
{
  EVENT_TABLE_APP
} EventApp;
#undef X


#define X(state) state,
typedef enum
{
  STATE_TABLE_J_TEXT
} StateJText;
#undef X

#define X(event) event,
typedef enum
{
  EVENT_TABLE_J_TEXT
} EventJText;
#undef X

#define X(element) element,
typedef enum
{
  ELEMENT_LIST
} Element;
#undef X


extern const char *state_name_app[];
extern const char *event_name_app[];

extern const char *state_name_j_text[];
extern const char *event_name_j_text[];

extern const char *element_list[];

extern StateApp   transition_table_app[NUM_STATES_APP][NUM_EVENTS_APP];
extern StateJText transition_table_j_text[NUM_STATES_J_TEXT][NUM_EVENTS_J_TEXT];

typedef struct 
{
 int hovered_char;
 int select_start;
 int select_length;
 int clicked_char;
} SelectionContext;

typedef struct
{
  StateJText current_state;
  SelectionContext context; 
  int font_loaded;
} MachineJText;


typedef struct
{
  StateApp current_state;
} MachineApp;

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
 Font font_japanese;
 char *text;
 Vector2 position;
 float font_size;
 float spacing;
 int char_count;
 int *text_codepoints;
 Rectangle char_pos[];
} TextData;


void init_machine_j_text(MachineJText *machine);
void update_state_j_text(MachineJText *machine, EventJText event);
void render_j_text(MachineJText *machine, TextData *text_data);
void cleanup_machine_j_text(MachineJText *machine);

void init_machine_app(MachineApp *machine);
void update_state_app(MachineApp *machine, EventApp event);

TextData *init_text(const char text[]);
void cleanup_text(TextData *text_data);

int GuiButtonCodepoint(Rectangle bounds, Font font, int codepoint,
    float fontSize, Color textColor);

void setup_raylib(void);

void render_components(MachineApp *m_app, MachineJText *m_j_text, TextData *text_data);

int (*Return_Map(StateApp state))[SIZE_ROWS][SIZE_COLS];

int *CodepointRemoveDuplicates(int *codepoints, int codepointCount, 
                               int *codepointResultCount);

char *chapter_data_to_string(const ChapterData *chapter);
ChapterData get_chapter_data(const char *filename);

#endif

