#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

static uint32_t last_key = 0;

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t key = inl(KBD_ADDR);
  if (key & KEYDOWN_MASK) {
    // press a button
    kbd->keydown = true;
    kbd->keycode = key & (~KEYDOWN_MASK);
  }
  else if (key != last_key) {
    // release a button
    kbd->keydown = false;
    kbd->keycode = key & (~KEYDOWN_MASK);
  } 
  else {
    // stateless event
    kbd->keydown = false;
    kbd->keycode = AM_KEY_NONE;
  }
  last_key = key;
}
