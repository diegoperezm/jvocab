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
  MachineApp m_app = {.current_state = STATE_A};
  MachineJText m_j_text;
  init_machine_j_text(&m_j_text);

  //const char text[] = "こんにちは、\n世界";
  const char text[] = "こんにちは、世界";
  TextData *text_data =  init_text(text);

  EventJText init_event_j_text = (text_data->font_japanese.texture.id != 0) ? 
                      evt_font_loaded : evt_font_failed;
  
  m_j_text.font_loaded = (init_event_j_text == evt_font_loaded);

  update_state_j_text(&m_j_text, init_event_j_text);

  SetTargetFPS(30);

  while (!WindowShouldClose())
  {

    BeginDrawing();
      ClearBackground(BGCOLOR);
      render_components(&m_app, &m_j_text, text_data);
    EndDrawing();
  }


  cleanup_text(text_data);
  cleanup_machine_j_text(&m_j_text);
  CloseWindow();

  return 0;
}







