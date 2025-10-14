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

  const char text[] = "こんにちは、\n世界";
  TextData *text_data =  init_text(text);

  Event init_event = (text_data->font_japanese.texture.id != 0) ? 
                      evt_font_loaded : evt_font_failed;
  
  machine.font_loaded = (init_event == evt_font_loaded);

  update_state(&machine, init_event);

  SetTargetFPS(30);

  while (!WindowShouldClose())
  {

    BeginDrawing();
      ClearBackground(BGCOLOR);
      render_state(&machine, text_data);
    EndDrawing();
  }


  cleanup_text(text_data);
  cleanup_machine(&machine);
  CloseWindow();

  return 0;
}







