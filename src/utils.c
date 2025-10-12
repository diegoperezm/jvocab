#include <raylib.h>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "../include/utils.h"
#include "../include/style_cyber.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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
    [STATE_INIT]  = {
       [evt_font_loaded]  = STATE_IDLE, 
       [evt_font_failed]  = STATE_ERROR, 
    },
   [STATE_IDLE] = {
     [evt_mouse_hover_char]  = STATE_HOVERING, 
     [evt_click_char]        = STATE_SELECTED, 
   },
   [STATE_HOVERING] = {
     [evt_mouse_hover_char]  = INVALID_STATE, 
     [evt_mouse_leave_char]  = STATE_IDLE, 
     [evt_click_char]        = STATE_SELECTED, 
   },
   [STATE_SELECTED] = {
     [evt_mouse_hover_char]  = INVALID_STATE,
     [evt_click_char]        = INVALID_STATE, 
     [evt_click_same_char]   = STATE_IDLE, 
     [evt_click_empty]   = STATE_IDLE, 

   },
   [STATE_ERROR] = {0},
   [INVALID_STATE] = {0},
};


void init_machine(Machine *machine)
{
 machine->current_state           = STATE_INIT;
 machine->context.hovered_char    = -1;
 machine->context.select_start    = -1;
 machine->context.select_length   = 0; 
 machine->font_loaded             = 0;
}


State update_state(Machine *machine, Event event)
{
  State current = machine->current_state;
  State next = transition_table[current][event];
 
  if (next != INVALID_STATE && next != 0)
  {
    TraceLog(LOG_INFO, "State transition: %s + %s -> %s", 
             state_name[current], event_name[event], state_name[next]);
    machine->current_state = next;

   // not sure   
    switch (next)
    {
      case STATE_IDLE:
        machine->context.select_start = -1;
        machine->context.select_length = 0;
        machine->context.hovered_char = -1;
        break;
        
      default:
        break;
    }
    // not sure   

    return next;
  }
  
  return current;
}


Event process_input(Machine *machine, TextData *text_data)
{
  State current_state = machine->current_state;
  Vector2 mouse_pos = GetMousePosition();
  

  int old_hovered = machine->context.hovered_char;
  machine->context.hovered_char = -1;
  
  // Check for hovered character
  for(int i = 0; i < text_data->char_count; i++) 
  {
    if(CheckCollisionPointRec(mouse_pos, text_data->char_pos[i]))
    {
      machine->context.hovered_char = i;
      break;
    }
  }
  

  if(current_state == STATE_INIT)
  {
    return NUM_EVENTS;
  }
  
  if(current_state == STATE_ERROR)
  {
    return NUM_EVENTS;
  }
  
  if(current_state == STATE_IDLE)
  {
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
      if(machine->context.hovered_char != -1)
      {
        machine->context.select_start = machine->context.hovered_char;
        machine->context.select_length = 1;
        return evt_click_char;
      }
      else
      {
        return evt_click_empty;
      }
    }
    
    // Handle hover transitions
    if(machine->context.hovered_char != -1 && old_hovered == -1)
    {
      return evt_mouse_hover_char;
    }
  }
  
  if(current_state == STATE_HOVERING)
  {
    // Handle mouse button press while hovering
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
      if(machine->context.hovered_char != -1)
      {
        machine->context.select_start = machine->context.hovered_char;
        machine->context.select_length = 1;
        return evt_click_char;
      }
      else
      {
        return evt_click_empty;
      }
    }
    
    // Handle leaving hover
    if(machine->context.hovered_char == -1 && old_hovered != -1)
    {
      return evt_mouse_leave_char;
    }
  }
  
  if(current_state == STATE_SELECTED)
  {
    // Handle clicking on selected character
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
      if(machine->context.hovered_char != -1)
      {
        if(machine->context.hovered_char == machine->context.select_start)
        {
          return evt_click_same_char;
        }
        else
        {
          machine->context.select_start = machine->context.hovered_char;
          machine->context.select_length = 1;
          return evt_click_char;
        }
      }
      else
      {
        return evt_click_empty;
      }
    }
    
    // Handle hover transitions
    if(machine->context.hovered_char != -1 && old_hovered == -1)
    {
      return evt_mouse_hover_char;
    }
    if(machine->context.hovered_char == -1 && old_hovered != -1)
    {
      return evt_mouse_leave_char;
    }
  }
  
  return NUM_EVENTS;
}

void render_state(Machine *machine, TextData *text_data)
{
  State current_state = machine->current_state;
  
  if (!machine->font_loaded || text_data->font_japanese.texture.id == 0)
  {
    DrawText("ERROR: Japanese font not loaded", 50, 100, 20, RED);
    DrawText("Please download NotoSansJP-Regular.ttf", 50, 130, 20, RED);
    return;
  }
  
  if(current_state == STATE_INIT)
  {
    DrawText("Initializing...", 50, 50, 20, GRAY);
    return;
  }
  
  if(current_state == STATE_ERROR)
  {
    DrawText("ERROR STATE", 50, 50, 20, RED);
    return;
  }
  
  for (int i = 0; i < text_data->char_count; i++)
  {
    Vector2 char_pos = { text_data->char_pos[i].x, text_data->char_pos[i].y }; 
    Rectangle char_rect = text_data->char_pos[i];
    
    Color text_color = LIGHTGRAY;
    int is_selected = (machine->context.select_length > 0) &&
                      (i >= machine->context.select_start) && 
                      (i < machine->context.select_start + machine->context.select_length);
    int is_hovered = (i == machine->context.hovered_char);
    
    // Priority: hover over selection
    if(is_hovered && (current_state == STATE_HOVERING || current_state == STATE_SELECTED))
    {
      DrawRectangle((int)char_rect.x - 2, (int)char_rect.y, 
                    (int)char_rect.width + 2, (int)char_rect.height, 
                    Fade(SKYBLUE, 0.6f));
      text_color = BLACK;
    }
    else if(is_selected && current_state == STATE_SELECTED)
    {
      DrawRectangle((int)char_rect.x - 2, (int)char_rect.y, 
                    (int)char_rect.width + 2, (int)char_rect.height, 
                    Fade(YELLOW, 0.8f));
      text_color = BLACK;
    }
    
    DrawTextCodepoint(text_data->font_japanese, 
                      text_data->text_codepoints[i],
                      char_pos,
                      text_data->font_size,
                      text_color);
  }
  
  // Debug: Show current state
  const char* state_n = state_name[current_state];
  DrawText(TextFormat("State: %s", state_n), 10, 10, 20, GREEN);
}


TextData *init_text(const char text[])
{
  int text_len = (int)TextLength(text);
  TextData *text_data = malloc(sizeof(TextData) + text_len * sizeof(Rectangle));

  text_data->font_size = 48.0f;
  text_data->spacing = 2.0f;
  text_data->text = (char *)malloc(strlen(text) + 1);
  text_data->position = (Vector2){ 50.0f, 100.0f };
  strcpy(text_data->text, text); 

  text_data->text_codepoints = (int *)malloc(text_len + sizeof(int));

  float offset_x = 0;
  float offset_y = 0;



  int ranges[][2] = {
    {0x0020, 0x007E},  // Basic Latin
    {0x3000, 0x303F},  // Symbols and Punctuation
    {0x3040, 0x309F},  // Hiragana
    {0x30A0, 0x30FF},  // Katakana
    {0x4E00, 0x9FAF}   // Unified Ideographs
  };
  
  int num_ranges = sizeof(ranges) / sizeof(ranges[0]);
  int codepoint_count = 0;
  
  for (int i = 0; i < num_ranges; i++) {
    codepoint_count += (ranges[i][1] - ranges[i][0] + 1);
  }
  
  int *codepoints = (int *)malloc(codepoint_count * sizeof(int));

  int idx = 0;
  for (int i = 0; i < num_ranges; i++) {
    for (int cp = ranges[i][0]; cp <= ranges[i][1]; cp++) {
      codepoints[idx++] = cp;
    }
  }
  
  text_data->font_japanese = LoadFontEx(
                                "../fonts/NotoSansJP-Regular.ttf",
                                64,
                                codepoints,
                                codepoint_count);
  free(codepoints);

  float scale_factor = text_data->font_size / (float)text_data->font_japanese.baseSize;
  int j = 0;
  for (int i = 0; i < text_len;) 
  {
        int byte_count = 0;
        int cp = GetCodepoint(&text[i], &byte_count);
        if (cp == '\n') {
            offset_y += text_data->font_size + 10;
            offset_x = 0;
            i += byte_count;
            continue;
        }
        text_data->text_codepoints[j] = cp;

        int index = GetGlyphIndex(text_data->font_japanese, cp);
        float glyph_width = (text_data->font_japanese.glyphs[index].advanceX == 0)
            ? text_data->font_japanese.recs[index].width * scale_factor
            : text_data->font_japanese.glyphs[index].advanceX * scale_factor;

        text_data->char_pos[j] = (Rectangle){
            .x = text_data->position.x + offset_x,
            .y = text_data->position.y + offset_y,
            .width = glyph_width + text_data->spacing,
            .height = text_data->font_size
        };

        offset_x += glyph_width + text_data->spacing;
        i += byte_count;
        j++;
    } // end::for  

  text_data->char_count = j; 
  return text_data;
}

void cleanup_text(TextData *text_data)
{
  if (text_data->text) free(text_data->text);
  if (text_data->text_codepoints) free(text_data->text_codepoints);
  if (text_data->font_japanese.texture.id != 0) UnloadFont(text_data->font_japanese);
}


void cleanup_machine(Machine *machine) { 
  (void)printf("%d", machine->current_state);
}

int (*Return_Map_Pr(const State state))[SIZE_ROWS][SIZE_COLS] {
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

  int(*map)[SIZE_ROWS][SIZE_COLS] = Return_Map_Pr(machine->current_state);
  int temp = machine->current_state;

  for (int row = 0; row < SIZE_ROWS; row++)
  {
    for (int col = 0; col < SIZE_COLS; col++)
    {
      const float cell_x = (float)col * cell_width;
      const float cell_y = (float)row * cell_height;
      const Rectangle cell = {cell_x, cell_y, cell_width, cell_height};

      switch ((*map)[row][col])
      {
      case TOGGLE_GROUP:
        GuiToggleGroup((Rectangle){cell.x, cell.y, cell.width, cell.height},
                       "A;B;C;D", &temp);

        if (temp != (int)machine->current_state)
        {
          /*
          Event(temp):
          order of EVENT_TABLE (utils.h)  and
          GuiToggleGroup(..., "TODAY;MONTH;YEAR;GRAPH", ...);
          should be the same.
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
  SetTargetFPS(3);
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

 
