#include "../include/utils.h"
#include "../include/style_cyber.h"
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define X(state) #state,
const char *state_name_app[] = {STATE_TABLE_APP};
#undef X

#define X(event) #event,
const char *event_name_app[] = {EVENT_TABLE_APP};
#undef X

#define X(state) #state,
const char *state_name_j_text[] = {STATE_TABLE_J_TEXT};
#undef X

#define X(event) #event,
const char *event_name_j_text[] = {EVENT_TABLE_J_TEXT};
#undef X

#define X(element) #element,
const char *element_list[] = {ELEMENT_LIST};
#undef X

StateApp
    transition_table_app[NUM_STATES_APP][NUM_EVENTS_APP] = {
        [STATE_CHAPTERS] =
            {
                [evt_click_chapters]   = INVALID_STATE_APP,
                [evt_click_vocabulary] = STATE_VOCABULARY,
                [evt_click_dictionary] = STATE_DICTIONARY,
                [evt_click_practice]   = STATE_PRACTICE, 
                [evt_click_progress]   = STATE_PROGRESS,
            },

        [STATE_VOCABULARY] =
            {
                [evt_click_chapters]   = STATE_CHAPTERS,
                [evt_click_vocabulary] = INVALID_STATE_APP,
                [evt_click_dictionary] = STATE_DICTIONARY,
                [evt_click_practice]   = STATE_PRACTICE, 
                [evt_click_progress]   = STATE_PROGRESS,
 
            },

        [STATE_DICTIONARY] = 
        {
                [evt_click_chapters]   = STATE_CHAPTERS,
                [evt_click_vocabulary] = STATE_VOCABULARY,
                [evt_click_dictionary] = INVALID_STATE_APP, 
                [evt_click_practice]   = STATE_PRACTICE, 
                [evt_click_progress]   = STATE_PROGRESS,
 
        },
        [STATE_PRACTICE] = 
        {
                [evt_click_chapters]   = STATE_CHAPTERS,
                [evt_click_vocabulary] = STATE_VOCABULARY,
                [evt_click_dictionary] = STATE_DICTIONARY,
                [evt_click_practice]   = INVALID_STATE_APP,  
                [evt_click_progress]   = STATE_PROGRESS,
 
         },

        [STATE_PROGRESS] = {
                [evt_click_chapters]   = STATE_CHAPTERS,
                [evt_click_vocabulary] = STATE_VOCABULARY,
                [evt_click_dictionary] = STATE_DICTIONARY,
                [evt_click_practice]   = STATE_PRACTICE,
                [evt_click_progress]   = INVALID_STATE_APP,   
        },

        };

StateJText transition_table_j_text
    [NUM_STATES_J_TEXT][NUM_EVENTS_J_TEXT] = {
        [STATE_INIT] =
            {
                [evt_font_loaded] = STATE_IDLE,
                [evt_font_failed] = STATE_ERROR,
            },
        [STATE_IDLE] =
            {
                [evt_mouse_hover_char] = STATE_HOVERING,
                [evt_click_char] = STATE_SELECTED,
            },

        [STATE_HOVERING] =
            {
                [evt_mouse_hover_char] = STATE_HOVERING,
                [evt_click_char] = STATE_SELECTED,
                //  [evt_mouse_leave_char] = STATE_IDLE,
            },
        [STATE_SELECTED] =
            {
                [evt_click_char] = INVALID_STATE_J_TEXT,
                //[evt_mouse_hover_char] = STATE_HOVERING,
                //[evt_mouse_leave_char] = STATE_IDLE,
                //[evt_click_empty] = STATE_IDLE,
                //[evt_click_same_char] = INVALID_STATE,
            },
        [STATE_ERROR] = {0},
        [INVALID_STATE_J_TEXT] = {0},
};

void init_machine_app(MachineApp *machine)
{
  machine->current_state = STATE_CHAPTERS;
}

void init_machine_j_text(MachineJText *machine)
{
  machine->current_state = STATE_INIT;
  machine->context.hovered_char = -1;
  machine->context.select_start = -1;
  machine->context.select_length = 0;
  machine->font_loaded = 0;
}

TextData *init_text(const char text[])
{
  int text_len = (int)TextLength(text);
  TextData *text_data = malloc(
      sizeof(TextData) + text_len * sizeof(Rectangle));

  text_data->font_size = 48.0f;
  text_data->spacing = 2.0f;
  text_data->text = (char *)malloc(strlen(text) + 1);
  text_data->position = (Vector2){50.0f, 100.0f};
  strcpy(text_data->text, text);

  text_data->text_codepoints =
      (int *)malloc(text_len * sizeof(int));

  float offset_x = 0;
  float offset_y = 0;

  int ranges[][2] = {
      {0x0020, 0x007E}, // Basic Latin
      {0x3000, 0x303F}, // Symbols and Punctuation
      {0x3040, 0x309F}, // Hiragana
      {0x30A0, 0x30FF}, // Katakana
      {0x4E00, 0x9FAF}  // Unified Ideographs
  };

  int num_ranges = sizeof(ranges) / sizeof(ranges[0]);
  int codepoint_count = 0;

  for (int i = 0; i < num_ranges; i++)
  {
    codepoint_count += (ranges[i][1] - ranges[i][0] + 1);
  }

  int *codepoints =
      (int *)malloc(codepoint_count * sizeof(int));

  int idx = 0;
  for (int i = 0; i < num_ranges; i++)
  {
    for (int cp = ranges[i][0]; cp <= ranges[i][1]; cp++)
    {
      codepoints[idx++] = cp;
    }
  }

  text_data->font_japanese = LoadFontEx(
      "../fonts/NotoSansJP-Regular.ttf",
      64,
      codepoints,
      codepoint_count);
  free(codepoints);

  float scale_factor =
      text_data->font_size /
      (float)text_data->font_japanese.baseSize;
  int j = 0;
  for (int i = 0; i < text_len;)
  {
    int byte_count = 0;
    int cp = GetCodepoint(&text[i], &byte_count);
    if (cp == '\n')
    {
      offset_y += text_data->font_size + 10;
      offset_x = 0;
      i += byte_count;
      continue;
    }
    text_data->text_codepoints[j] = cp;

    int index = GetGlyphIndex(text_data->font_japanese, cp);
    float glyph_width =
        (text_data->font_japanese.glyphs[index].advanceX ==
         0)
            ? text_data->font_japanese.recs[index].width *
                  scale_factor
            : text_data->font_japanese.glyphs[index]
                      .advanceX *
                  scale_factor;

    text_data->char_pos[j] = (Rectangle){
        .x = text_data->position.x + offset_x,
        .y = text_data->position.y + offset_y,
        .width = glyph_width + text_data->spacing,
        .height = text_data->font_size};

    offset_x += glyph_width + text_data->spacing;
    i += byte_count;
    j++;
  } // end::for

  text_data->char_count = j;
  return text_data;
}

void cleanup_text(TextData *text_data)
{
  if (text_data->text)
    free(text_data->text);
  if (text_data->text_codepoints)
    free(text_data->text_codepoints);
  if (text_data->font_japanese.texture.id != 0)
    UnloadFont(text_data->font_japanese);
}

void cleanup_machine_j_text(MachineJText *machine)
{
  (void)printf("%d", machine->current_state);
}

int (*Return_Map(StateApp state)) [SIZE_ROWS][SIZE_COLS]
{
  static int map[SIZE_ROWS][SIZE_COLS] = {0};

  static int map_state_chapters[SIZE_ROWS][SIZE_COLS] = {
      {TOGGLE_GROUP},
  };

  static int map_state_vocabulary[SIZE_ROWS][SIZE_COLS] = {
      {TOGGLE_GROUP},
  };

  static int map_state_dictionary[SIZE_ROWS][SIZE_COLS] = {
      {TOGGLE_GROUP},
      {ELMNT_BLANK},
      {J_TEXT},
  };

  static int map_state_practice[SIZE_ROWS][SIZE_COLS] = {
      {TOGGLE_GROUP},
  };

  static int map_state_progress[SIZE_ROWS][SIZE_COLS] = {
      {TOGGLE_GROUP},
  };

  switch (state)
  {
  case STATE_CHAPTERS:
    return &map_state_chapters;
  case STATE_VOCABULARY:
    return &map_state_vocabulary;
  case STATE_DICTIONARY:
    return &map_state_dictionary;
  case STATE_PRACTICE:
    return &map_state_practice;
  case STATE_PROGRESS:
    return &map_state_progress;
  case INVALID_STATE_APP:
  case NUM_STATES_APP:
  default:
    return &map;
  }
  return &map;
}

void render_components(
    MachineApp *m_app,
    MachineJText *m_j_text,
    TextData *text_data)
{
  const float width = (float)GetScreenWidth();
  const float height = (float)GetScreenHeight();
  const float cell_width = width / GRID_COLS;
  const float cell_height = height / GRID_ROWS;
  // const Color font_color = GetColor(GuiGetStyle(0, 2));
  // const int font_size = (int)(cell_width * 0.5F);

  int (*map)[SIZE_ROWS][SIZE_COLS] =
      Return_Map(m_app->current_state);

  int temp = m_app->current_state;

  for (int row = 0; row < SIZE_ROWS; row++)
  {
    for (int col = 0; col < SIZE_COLS; col++)
    {
      const float cell_x = (float)col * cell_width;
      const float cell_y = (float)row * cell_height;
      const Rectangle cell =
          {cell_x, cell_y, cell_width, cell_height};

      switch ((*map)[row][col])
      {
      case TOGGLE_GROUP:
        GuiToggleGroup(
            (Rectangle){cell.x,
                        cell.y,
                        cell.width,
                        cell.height},
            "CHAPTERS;VOCABULARY;DICTIONARY;PRACTICE;PROGRESS",
            &temp);




        if (temp != (int)m_app->current_state)
        {
          /*
          Event(temp):
          order of EVENT_TABLE (utils.h)  and
          GuiToggleGroup(..., "TODAY;MONTH;YEAR;GRAPH",
          ...); should be the same.
          */
          update_state_app(m_app, (EventApp)temp);
        }

        break;
      case J_TEXT:
        render_j_text(m_j_text, text_data);
      default:
        break;
      }
    }
  }

} // grid layout

void setup_raylib(void)
{
  const int screenW = 1200;
  const int screenH = 600;
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenW, screenH, "Journal");
  SetTargetFPS(30);
  GuiLoadStyleCyber();
}

char *chapter_data_to_string(const ChapterData *chapter)
{
  if (!chapter || !chapter->nodes || chapter->size <= 0)
    return NULL;

  size_t total_size = 0;
  for (int i = 0; i < chapter->size; i++)
  {
    total_size += chapter->nodes[i].size_id;
    total_size += chapter->nodes[i].size_content;
    total_size += 2;
  }

  total_size += 1; // null  terminator

  char *text = malloc(total_size);
  if (!text)
    return NULL;

  char *pos = text;

  for (int i = 0; i < chapter->size; ++i)
  {
    if (!chapter->nodes[i].id || !chapter->nodes[i].content)
      continue;
    if (chapter->nodes[i].size_id > 0)
    {
      memcpy(
          pos,
          chapter->nodes[i].id,
          chapter->nodes[i].size_id);

      pos += chapter->nodes[i].size_id;
    }
    *pos++ = ' '; // space after id

    if (chapter->nodes[i].size_content > 0)
    {
      memcpy(
          pos,
          chapter->nodes[i].content,
          chapter->nodes[i].size_content);
      pos += chapter->nodes[i].size_content;
    }
    *pos++ = '\n'; // newline after content
  }
  *pos = '\0';
  return text;
}

ChapterData get_chapter_data(const char *filename)
{
  LIBXML_TEST_VERSION

  htmlDocPtr doc = htmlReadFile(
      filename,
      NULL,
      HTML_PARSE_RECOVER | HTML_PARSE_NOERROR |
          HTML_PARSE_NOWARNING);

  if (NULL == doc)
  {
    fprintf(stderr, "Failed to parse %s\n", filename);
  }
  xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
  if (NULL == ctx)
  {
    fprintf(stderr, "Failed to create Xpath context\n");
    xmlFree(doc);
  }
  xmlXPathObjectPtr result = xmlXPathEvalExpression(
      (const xmlChar *)"//p[starts-with(@id,'L')]",
      ctx);
  NodeData *nodes;
  ChapterData chapter = {.nodes = 0, .size = 0};
  if (result && result->nodesetval->nodeNr > 0)
  {
    int idx = 0;
    int total_nodes = result->nodesetval->nodeNr;
    int odd_nodes = (int)(total_nodes + 1) / 2;
    nodes = malloc(sizeof(NodeData) * odd_nodes);
    /*
     * This only works if (no other data):
     * L1 content
     * L2 empty
     *
     */
    for (int i = 0; i < total_nodes; i += 2)
    {
      xmlNodePtr node = result->nodesetval->nodeTab[i];
      xmlChar *id = xmlGetProp(node, (const xmlChar *)"id");
      xmlChar *content = xmlNodeGetContent(node);
      nodes[idx].id = strdup((char *)id);
      nodes[idx].content = strdup((char *)content);
      nodes[idx].size_id = strlen(nodes[idx].id);
      nodes[idx].size_content = strlen(nodes[idx].content);
      // printf("%s %s\n",nodes[idx].id,nodes[idx].content);
      idx++;
      if (id)
        xmlFree(id);
      if (content)
        xmlFree(content);
    }

    chapter.nodes = nodes;
    chapter.size = odd_nodes;
  }
  else
  {
    nodes = NULL;
  }

  xmlXPathFreeObject(result);
  xmlXPathFreeContext(ctx);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  return chapter;
}

// ChapterData chapter = get_chapter_data("../data/3");
// char *text = chapter_data_to_string(&chapter);

// Button idle: result    = 0
// Button pressed: result = 1
// Button hovered: result = 2
int GuiButtonCodepoint(
    Rectangle bounds,
    Font font,
    int codepoint,
    float fontSize,
    Color textColor)
{
  int result = 0;
  GuiState state = (GuiState)GuiGetState();

  // Update control
  //--------------------------------------------------------------------
  if ((state != STATE_DISABLED) && !GuiIsLocked())
  {
    Vector2 mousePoint = GetMousePosition();

    // Check button state
    if (CheckCollisionPointRec(mousePoint, bounds))
    {
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
      {
        state = STATE_PRESSED;
      }
      else
      {
        state = STATE_FOCUSED;
        result = 2;
      }
      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        result = 1;
    }
  }

  // Calculate center position for the codepoint
  int utf8Size = 0;
  const char *utf8Text =
      CodepointToUTF8(codepoint, &utf8Size);
  Vector2 glyphSize =
      MeasureTextEx(font, utf8Text, fontSize, 0);

  Vector2 charPos = {
      bounds.x + (bounds.width - glyphSize.x) / 2.0f,
      bounds.y + (bounds.height - glyphSize.y) / 2.0f};

  //  Apply text color based on state if textColor has alpha
  //  0 Color finalTextColor =
  //     (textColor.a == 0)
  //        ? GetColor(GuiGetStyle(
  //             BUTTON,
  //             TEXT_COLOR_NORMAL + (state * 3)))
  //      : textColor;

  DrawTextCodepoint(
      font,
      codepoint,
      charPos,
      fontSize,
      textColor); // finalTextColor);
  //------------------------------------------------------------------

  return result;
}

void render_j_text(
    MachineJText *machine,
    TextData *text_data)
{
  StateJText current_state = machine->current_state;

  if (!machine->font_loaded ||
      text_data->font_japanese.texture.id == 0)
  {
    DrawText(
        "ERROR: Japanese font not loaded",
        50,
        100,
        20,
        RED);
    DrawText(
        "Please download NotoSansJP-Regular.ttf",
        50,
        130,
        20,
        RED);
    return;
  }

  if (current_state == STATE_INIT)
  {
    DrawText("Initializing...", 50, 50, 20, GRAY);
    return;
  }

  if (current_state == STATE_ERROR)
  {
    DrawText("ERROR STATE", 50, 50, 20, RED);
    return;
  }
  int is_idle = 0;
  int is_selected = 1;
  int is_hovered = 2;

  for (int button_index = 0;
       button_index < text_data->char_count;
       button_index++)
  {
    Rectangle char_rect = text_data->char_pos[button_index];

    switch (current_state)
    {
    case STATE_IDLE:
    {
      machine->context.select_start = -1;
      machine->context.select_length = 0;
      machine->context.hovered_char = -1;
      machine->context.clicked_char = -1;

      int button_state = GuiButtonCodepoint(
          char_rect,
          text_data->font_japanese,
          text_data->text_codepoints[button_index],
          text_data->font_size,
          LIGHTGRAY);
      if (button_state == is_selected)
      {
        machine->context.clicked_char = button_index;
        update_state_j_text(machine, evt_click_char);
      }
      if (button_state == is_hovered)
      {
        machine->context.hovered_char = button_index;
        update_state_j_text(machine, evt_mouse_hover_char);
      }
      break;
    }
    case STATE_HOVERING:
    {
      int button_state = GuiButtonCodepoint(
          char_rect,
          text_data->font_japanese,
          text_data->text_codepoints[button_index],
          text_data->font_size,
          LIGHTGRAY);
      if (button_state == is_idle)
      {
      }
      if (button_state == is_hovered)
      {
        machine->context.hovered_char = button_index;
        DrawRectangle(
            (int)char_rect.x - 2,
            (int)char_rect.y,
            (int)char_rect.width + 2,
            (int)char_rect.height,
            Fade(WHITE, 0.6f));
      }
      if (button_state == is_selected)
      {
        machine->context.clicked_char = button_index;
        update_state_j_text(machine, evt_click_char);
      }
      break;
    }
    case STATE_SELECTED:
    {
      int button_state = GuiButtonCodepoint(
          char_rect,
          text_data->font_japanese,
          text_data->text_codepoints[button_index],
          text_data->font_size,
          LIGHTGRAY);
      if (machine->context.clicked_char == button_index)
      {
        machine->context.clicked_char = button_index;
        DrawRectangle(
            (int)char_rect.x - 2,
            (int)char_rect.y,
            (int)char_rect.width + 2,
            (int)char_rect.height,
            Fade(YELLOW, 0.8f));
        GuiButtonCodepoint(
            char_rect,
            text_data->font_japanese,
            text_data->text_codepoints[button_index],
            text_data->font_size,
            BLACK);
      }
      else if (
          machine->context.clicked_char != button_index &&
          button_state == is_selected)
      {
        machine->context.clicked_char = button_index;
        DrawRectangle(
            (int)char_rect.x - 2,
            (int)char_rect.y,
            (int)char_rect.width + 2,
            (int)char_rect.height,
            Fade(YELLOW, 0.8f));
        GuiButtonCodepoint(
            char_rect,
            text_data->font_japanese,
            text_data->text_codepoints[button_index],
            text_data->font_size,
            BLACK);
      }
      if (button_state == is_hovered &&
          machine->context.clicked_char != button_index)
      {
        machine->context.hovered_char = button_index;
        DrawRectangle(
            (int)char_rect.x - 2,
            (int)char_rect.y,
            (int)char_rect.width + 2,
            (int)char_rect.height,
            Fade(WHITE, 0.6f));
      }
      break;
    }
    case STATE_INIT:
    case STATE_SELECTED_HOVERING:
    case STATE_ERROR:
    case INVALID_STATE_J_TEXT:
    case NUM_STATES_J_TEXT:
      break;
    }
  } // end::for
  /*
    const char *state_n = state_name[current_state];
    DrawText(
        TextFormat("State: %s", state_n),
        10,
        10,
        20,
        GREEN);
  */
}

void update_state_app(MachineApp *machine, EventApp event)
{
  StateApp current = machine->current_state;
  StateApp next = transition_table_app[current][event];

  if (next != INVALID_STATE_APP)
  {
    TraceLog(
        LOG_INFO,
        "State transition: %s + %s -> %s",
        state_name_app[current],
        event_name_app[event],
        state_name_app[next]);
    machine->current_state = next;
  }
}

void update_state_j_text(
    MachineJText *machine,
    EventJText event)
{
  StateJText current = machine->current_state;
  StateJText next = transition_table_j_text[current][event];

  if (next != INVALID_STATE_J_TEXT && next != 0)
  {
    TraceLog(
        LOG_INFO,
        "State transition: %s + %s -> %s",
        state_name_j_text[current],
        event_name_j_text[event],
        state_name_j_text[next]);
    machine->current_state = next;
  }
}
