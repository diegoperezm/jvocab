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
const char *state_name[] = {STATE_TABLE};
#undef X

#define X(event) #event,
const char *event_name[] = {EVENT_TABLE};
#undef X

#define X(element) #element,
const char *element_list[] = {ELEMENT_LIST};
#undef X

State transition_table[NUM_STATES][NUM_EVENTS] = {
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
            [evt_mouse_leave_char] = STATE_IDLE,
            [evt_click_char] = STATE_SELECTED,
        },
    [STATE_SELECTED] =
        {
            [evt_mouse_hover_char] = STATE_HOVERING,
            [evt_mouse_leave_char] = STATE_IDLE,
            [evt_click_char] = INVALID_STATE,
            [evt_click_same_char] = INVALID_STATE,
            [evt_click_empty] = STATE_IDLE,
        },
    [STATE_ERROR] = {0},
    [INVALID_STATE] = {0},
};

void init_machine(Machine *machine)
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
      (int *)malloc(text_len + sizeof(int));

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

void cleanup_machine(Machine *machine)
{
  (void)printf("%d", machine->current_state);
}

int (*Return_Map_Pr(const State state))
    [SIZE_ROWS][SIZE_COLS]
{
  static int map[SIZE_ROWS][SIZE_COLS] = {0};

  static int map_state_root[SIZE_ROWS][SIZE_COLS] = {
      {TOGGLE_GROUP},
  };

  switch (state)
  {
  case STATE_INIT:
    return &map_state_root;
  case INVALID_STATE:
  case NUM_STATES:
  default:
    return &map;
  }
}

void grid_layout(Machine *machine)
{
  const float width = (float)GetScreenWidth();
  const float height = (float)GetScreenHeight();
  const float cell_width = width / GRID_COLS;
  const float cell_height = height / GRID_ROWS;
  // const Color font_color = GetColor(GuiGetStyle(0, 2));
  // const int font_size = (int)(cell_width * 0.5F);

  int (*map)[SIZE_ROWS][SIZE_COLS] =
      Return_Map_Pr(machine->current_state);
  int temp = machine->current_state;

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
            "A;B;C;D",
            &temp);

        if (temp != (int)machine->current_state)
        {
          /*
          Event(temp):
          order of EVENT_TABLE (utils.h)  and
          GuiToggleGroup(..., "TODAY;MONTH;YEAR;GRAPH",
          ...); should be the same.
          */
          update_state(machine, temp);
        }

        break;
      case J_TEXT:

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
  SetTargetFPS(1);
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

void render_state(Machine *machine, TextData *text_data)
{
  State current_state = machine->current_state;

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

// STATE_IDLE
    if (current_state == STATE_IDLE)
    {
      int button_state = GuiButtonCodepoint(
          char_rect,
          text_data->font_japanese,
          text_data->text_codepoints[button_index],
          text_data->font_size,
          LIGHTGRAY);

      if (button_state == is_selected)
      {
        machine->context.clicked_char = button_index;
        update_state(machine, evt_click_char);
      }

      if (button_state == is_hovered)
      {
        machine->context.hovered_char = button_index;
        update_state(machine, evt_mouse_hover_char);
      }
    } // end::idle

// STATE_HOVERING
    if (current_state == STATE_HOVERING)
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
        update_state(machine, evt_click_char);
      }
    }

// STATE_SELECTED
    if (current_state == STATE_SELECTED)
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
    }

  } // end::for

  // Debug: Show current state
  const char *state_n = state_name[current_state];
  DrawText(
      TextFormat("State: %s", state_n),
      10,
      10,
      20,
      GREEN);
}

State update_state(Machine *machine, Event event)
{
  State current = machine->current_state;
  State next = transition_table[current][event];

  if (next != INVALID_STATE && next != 0)
  {
    TraceLog(
        LOG_INFO,
        "State transition: %s + %s -> %s",
        state_name[current],
        event_name[event],
        state_name[next]);
    machine->current_state = next;

    /*   case STATE_IDLE:
          machine->context.select_start = -1;
          machine->context.select_length = 0;
          machine->context.hovered_char = -1;
          machine->context.clicked_char = -1;
    */
    return next;
  }

  return current;
}
