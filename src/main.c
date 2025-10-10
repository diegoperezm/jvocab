#include "../include/utils.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
  setup_raylib();
  const Color BGCOLOR = (Color){1, 34, 43, 255};
  Machine machine;
  init_machine(&machine);

  TextData text_data;
  const char text[] = "こんにちは、\n世界";
  init_text(text, &text_data);

  Event init_event = (text_data.font_japanese.texture.id != 0) ? 
                      evt_font_loaded : evt_font_failed;
  
  machine.font_loaded = (init_event == evt_font_loaded);

  update_state(&machine, init_event);

  SetTargetFPS(30);

  while (!WindowShouldClose())
  {
     Event event = process_input(&machine, &text_data);
     update_state(&machine, event);

    BeginDrawing();
      ClearBackground(BGCOLOR);
      render_state(&machine, &text_data);

    EndDrawing();
  }


  cleanup_text(&text_data);
  cleanup_machine(&machine);
  CloseWindow();

  return 0;
}






/*
int main(void)
{

  Machine machine  = { .currentState = STATE_ROOT };
  setup_raylib();
  const Color BGCOLOR = (Color){1, 34, 43, 255};
  char text[] = "おはようございます";
  TextData text_data = {0};

  init_text(text, &text_data);
  int charCount = text_data.codepoint_count; 


  int selectStart = -1;
  int selectLength = 0;
  int hoveredChar = -1;
  Vector2 textPositions[256] = {0};

  while (!WindowShouldClose())
  {
    Vector2 mousePos = GetMousePosition();

    if (text_data.font_japanese.texture.id != 0)
    {
      Vector2 position = {50.0f, 100.0f};
      float fontSize = 48.0f;
      float spacing = 2.0f;
      float scaleFactor =
          fontSize / (float)text_data.font_japanese.baseSize;

      hoveredChar = -1;
      int charIdx = 0;
      float offsetX = 0;
      float offsetY = 0;

      int textLen = (int)TextLength(text);
      for (int i = 0; i < textLen; charIdx++)
      {
        int codepointByteCount = 0;
        int codepoint =
            GetCodepoint(&text[i], &codepointByteCount);

        if (codepoint == '\n')
        {
          offsetY += fontSize + 10;
          offsetX = 0;
          i += codepointByteCount;
          continue;
        }

        int index = GetGlyphIndex(text_data.font_japanese, codepoint);
        float glyphWidth =
            (text_data.font_japanese.glyphs[index].advanceX == 0)
                ? text_data.font_japanese.recs[index].width *
                      scaleFactor
                : text_data.font_japanese.glyphs[index].advanceX *
                      scaleFactor;

        Rectangle charRect = {
            position.x + offsetX,
            position.y + offsetY,
            glyphWidth + spacing,
            fontSize};

        textPositions[charIdx] =
            (Vector2){charRect.x, charRect.y};

        if (CheckCollisionPointRec(mousePos, charRect))
        {
          hoveredChar = charIdx;
        }

        offsetX += glyphWidth + spacing;
        i += codepointByteCount;
      } // end::for 

      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      {
        if (hoveredChar != -1)
        {
          if (selectStart == hoveredChar)
          {
            selectStart = -1;
            selectLength = 0;
          }
          else
          {
            selectStart = hoveredChar;
            selectLength = 1;
          }
        }
        else
        {
          selectStart = -1;
          selectLength = 0;
        }
      }

      if (IsKeyPressed(KEY_A) &&
          IsKeyDown(KEY_LEFT_CONTROL))
      {
        selectStart = 0;
        selectLength = charCount;
      }

      if (IsKeyPressed(KEY_ESCAPE))
      {
        selectStart = -1;
        selectLength = 0;
      }
    }

    BeginDrawing();
      ClearBackground(BGCOLOR);
      grid_layout(&machine);

    if (text_data.font_japanese.texture.id != 0)
    {

      Vector2 position = {50.0f, 100.0f};
      float fontSize = 48.0f;
      float spacing = 2.0f;
      float scaleFactor =
          fontSize / (float)text_data.font_japanese.baseSize;

      int charIdx = 0;
      float offsetX = 0;
      float offsetY = 0;

      int textLen = (int)TextLength(text);
      for (int i = 0; i < textLen; charIdx++)
      {
        int codepointByteCount = 0;
        int codepoint =
            GetCodepoint(&text[i], &codepointByteCount);

        if (codepoint == '\n')
        {
          offsetY += fontSize + 10;
          offsetX = 0;
          i += codepointByteCount;
          continue;
        }

        int index = GetGlyphIndex(text_data.font_japanese, codepoint);
        float glyphWidth =
            (text_data.font_japanese.glyphs[index].advanceX == 0)
                ? text_data.font_japanese.recs[index].width *
                      scaleFactor
                : text_data.font_japanese.glyphs[index].advanceX *
                      scaleFactor;

        Vector2 charPos = {
            position.x + offsetX,
            position.y + offsetY};


        Color textColor = LIGHTGRAY;
        bool isSelected =
            (selectStart >= 0) &&
            (charIdx >= selectStart) &&
            (charIdx < selectStart + selectLength);
        bool isHovered = (charIdx == hoveredChar);


        if (isSelected)
        {
          DrawRectangle(
              (int)charPos.x - 2,
              (int)charPos.y,
              (int)(glyphWidth + spacing + 2),
              (int)fontSize,
              Fade(YELLOW, 0.9f));
          textColor = BLACK;
        }
        else if (isHovered)
        {
          DrawRectangle(
              (int)charPos.x - 2,
              (int)charPos.y,
              (int)(glyphWidth + spacing + 2),
              (int)fontSize,
              Fade(WHITE, 0.7f));
          textColor = BLACK;
        }


        DrawTextCodepoint(
            text_data.font_japanese,
            codepoint,
            charPos,
            fontSize,
            textColor);

        offsetX += glyphWidth + spacing;
        i += codepointByteCount;
      }

      
    }
    else
    {
      DrawText(
          "ERROR: Japanese font not loaded!",
          50,
          100,
          20,
          RED);
      DrawText(
          "Please download NotoSansCJK-Regular.ttf",
          50,
          130,
          20,
          RED);
      DrawText(
          "Place it in the same directory as the "
          "executable",
          50,
          160,
          20,
          RED);
    }

    EndDrawing();
  }

  if (text_data.font_japanese.texture.id != 0)
    UnloadFont(text_data.font_japanese);
  CloseWindow();

  return 0;
}

Draw in a box with selection support
      Rectangle textBox = {50.0f, 300.0f, 700.0f, 100.0f};
      DrawRectangleLinesEx(textBox, 2, LIGHTGRAY);
      DrawTextBoxedSelectable(
          text_data.font_japanese,
          text,
          textBox,
          36.0f,
          2.0f,
          true,
          DARKBLUE,
          selectStart >= 0 ? selectStart : 0,
          selectStart >= 0 ? selectLength : 0,
          WHITE,
          Fade(BLUE, 0.3f));
       

static void DrawTextBoxedSelectable(
    Font font,
    const char *text,
    Rectangle rec,
    float fontSize,
    float spacing,
    bool wordWrap,
    Color tint,
    int selectStart,
    int selectLength,
    Color selectTint,
    Color selectBackTint)
{
  int length = TextLength(text);
  float textOffsetY = 0.0f;
  float textOffsetX = 0.0f;
  float scaleFactor = fontSize / (float)font.baseSize;

  enum
  {
    MEASURE_STATE = 0,
    DRAW_STATE = 1
  };
  int state = wordWrap ? MEASURE_STATE : DRAW_STATE;

  int startLine = -1;
  int endLine = -1;
  int lastk = -1;

  for (int i = 0, k = 0; i < length; i++, k++)
  {
    int codepointByteCount = 0;
    int codepoint =
        GetCodepoint(&text[i], &codepointByteCount);
    int index = GetGlyphIndex(font, codepoint);

    if (codepoint == 0x3f)
      codepointByteCount = 1;
    i += (codepointByteCount - 1);

    float glyphWidth = 0;
    if (codepoint != '\n')
    {
      glyphWidth =
          (font.glyphs[index].advanceX == 0)
              ? font.recs[index].width * scaleFactor
              : font.glyphs[index].advanceX * scaleFactor;
      if (i + 1 < length)
        glyphWidth = glyphWidth + spacing;
    }

    if (state == MEASURE_STATE)
    {
      if ((codepoint == ' ') || (codepoint == '\t') ||
          (codepoint == '\n'))
        endLine = i;

      if ((textOffsetX + glyphWidth) > rec.width)
      {
        endLine = (endLine < 1) ? i : endLine;
        if (i == endLine)
          endLine -= codepointByteCount;
        if ((startLine + codepointByteCount) == endLine)
          endLine = (i - codepointByteCount);
        state = !state;
      }
      else if ((i + 1) == length)
      {
        endLine = i;
        state = !state;
      }
      else if (codepoint == '\n')
        state = !state;

      if (state == DRAW_STATE)
      {
        textOffsetX = 0;
        i = startLine;
        glyphWidth = 0;
        int tmp = lastk;
        lastk = k - 1;
        k = tmp;
      }
    }
    else
    {
      if (codepoint == '\n')
      {
        if (!wordWrap)
        {
          textOffsetY +=
              (font.baseSize + font.baseSize / 2) *
              scaleFactor;
          textOffsetX = 0;
        }
      }
      else
      {
        if (!wordWrap &&
            ((textOffsetX + glyphWidth) > rec.width))
        {
          textOffsetY +=
              (font.baseSize + font.baseSize / 2) *
              scaleFactor;
          textOffsetX = 0;
        }

        if ((textOffsetY + font.baseSize * scaleFactor) >
            rec.height)
          break;

        bool isGlyphSelected = false;
        if ((selectStart >= 0) && (k >= selectStart) &&
            (k < (selectStart + selectLength)))
        {
          DrawRectangleRec(
              (Rectangle){rec.x + textOffsetX - 1,
                          rec.y + textOffsetY,
                          glyphWidth,
                          (float)font.baseSize *
                              scaleFactor},
              selectBackTint);
          isGlyphSelected = true;
        }

        if ((codepoint != ' ') && (codepoint != '\t'))
        {
          DrawTextCodepoint(
              font,
              codepoint,
              (Vector2){rec.x + textOffsetX,
                        rec.y + textOffsetY},
              fontSize,
              isGlyphSelected ? selectTint : tint);
        }
      }

      if (wordWrap && (i == endLine))
      {
        textOffsetY += (font.baseSize + font.baseSize / 2) *
                       scaleFactor;
        textOffsetX = 0;
        startLine = endLine;
        endLine = -1;
        glyphWidth = 0;
        selectStart += lastk - k;
        k = lastk;
        state = !state;
      }
    }

    textOffsetX += glyphWidth;
  }
}

      const char *info = TextFormat(
          "Characters: %d | Selected: %s | Hovered: %d",
          charCount,
          (selectStart >= 0) ? "Yes" : "No",
          hoveredChar);

      DrawText(info, 50, 45, 15, DARKBLUE);


static int hovered  = -1;
static int selected = -1;

int main(void)
{

  setup_raylib();
  Machine machine = {.currentState = STATE_ROOT};
  const Color BGCOLOR = (Color){1, 34, 43, 255};

  TextData text_data = {0};

  char text[] =
"バンフィールド家の執事\nであるブライアンは"; Vector2 pos =
(Vector2){100,200}; text_data.codepoint_count = 0;
  text_data.codepoints = LoadCodepoints(text,
&text_data.codepoint_count); bool *selected =
(bool*)calloc(text_data.codepoint_count, sizeof(bool)); int
font_size = 32;


  Vector2 hovered_pos  = {0.0f, 0.0f};
  Vector2 selected_pos = {0.0f, 0.0f};


  Font font = load_text(&text_data);

  while (!WindowShouldClose())
  {
   if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                          (hovered  != -1)    &&
                          (hovered != selected ))
   {
     selected     = hovered;
     selected_pos = hovered_pos;
   }

   Vector2 mouse   = GetMousePosition();
   Vector2 position = {28.8f, 10.0f};
   hovered = -1;

   BeginDrawing();
      ClearBackground(BGCOLOR);
      grid_layout(&machine);

      DrawTextEx(
          font,
          text,
          pos,
          24,
          font_size,
          LIGHTGRAY);

    EndDrawing();
  }
  //free(text);
  free(selected);
  UnloadFont(font);
  CloseWindow();
  return 0;
}

{
  setup_raylib();
  Machine machine = {.currentState = STATE_ROOT};
  const Color BGCOLOR = (Color){1, 34, 43, 255};
  TextData text_data = {0};
  char text[] =
"バンフィールド家の執事\nであるブライアンは"; Vector2 pos =
(Vector2){100, 200};

  text_data.codepoint_count = 0;
  text_data.codepoints = LoadCodepoints(text,
&text_data.codepoint_count); bool *selected =
(bool*)calloc(text_data.codepoint_count, sizeof(bool));

  float font_size = 24;  // Use consistent size
  float spacing = 2;

  Font font = load_text(&text_data);

  while (!WindowShouldClose())
  {
    Vector2 mouse = GetMousePosition();

    // Click to toggle selection
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        float x = pos.x;
        for (int i = 0; i < text_data.codepoint_count; i++)
{ GlyphInfo g = GetGlyphInfo(font, text_data.codepoints[i]);

            // Use actual font_size for click detection
            float clickTop = pos.y;
            float clickBottom = pos.y + font_size;

            if (mouse.x >= x && mouse.x < x + g.advanceX &&
                mouse.y >= clickTop && mouse.y <
clickBottom) { selected[i] = !selected[i]; printf("***
SELECTED CHAR %d! ***\n", i); break;
            }
            x += g.advanceX + spacing;
        }
    }

    BeginDrawing();
    ClearBackground(BGCOLOR);
    grid_layout(&machine);

    // Draw characters individually to match click detection
    float x = pos.x;
    for (int i = 0; i < text_data.codepoint_count; i++) {
        GlyphInfo g = GetGlyphInfo(font,
text_data.codepoints[i]);

        // Draw selection highlight
        if (selected[i]) {
            DrawRectangle(x, pos.y, g.advanceX + spacing,
font_size, SKYBLUE);
        }

        // Draw character
        DrawTextCodepoint(font, text_data.codepoints[i],
                         (Vector2){x, pos.y}, font_size,
LIGHTGRAY);

        x += g.advanceX + spacing;
    }

    EndDrawing();
  }

  free(selected);
  //UnloadCodepoints(text_data.codepoints);
  UnloadFont(font);
  CloseWindow();
  return 0;
}
  // int charCount = GetCodepointCount(text);
*/
