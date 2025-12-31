#ifndef BITMAPS_H
#define BITMAPS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum Emotion {
  NEUTRAL,     // 😐
  LOVE,        // 😍
  LAUGH,       // 😂
  SLEEP,       // 😴
  WINK,        // 😉
  MISCHIEVOUS, // 😈
  ANGRY_RAGE,  // 🤬
  SHOCKED,     // 😮
  SAD,         // 😢
  DEAD,        // 💀
  DIZZY,       // 😵
  PARTY,       // 🥳
  SKEPTICAL,   // 🤨
  FRUSTRATED,  // 😤
  ANGEL,       // 😇
  CRYING       // 😭
};

extern Emotion currentEmotion;
extern int currentFrame;
extern int targetPupilX, targetPupilY;
extern int pupilX, pupilY;
extern int currentEyeHeight, targetEyeHeight;
extern bool
    autoPilotActive; // Note: In Main.ino consistent name verification needed
extern bool isTalkingNow;
extern unsigned long lastEmotionUpdate;
extern void setEmotion(Emotion e);

#define LEFT_EYE_X 28
#define RIGHT_EYE_X 76
#define EYE_Y 18
#define EYE_W 24
#define EYE_H 28
#define EYE_HEIGHT EYE_H
#define EYE_WIDTH EYE_W

void drawStandardEye(Adafruit_SSD1306 &d, int x, int y, int w, int h, int pX,
                     int pY) {
  // Micro-saccades (tiny jitters for realism)
  static int microX = 0, microY = 0;
  if (random(100) < 15) {
    microX = random(-1, 2);
    microY = random(-1, 2);
  }
  pX += microX;
  pY += microY;

  if (h <= 4)
    d.fillRect(x, 42, w, 2, SSD1306_WHITE);
  else {
    int top = 28 - h;
    d.fillRoundRect(x, y + top, w, h, 8, SSD1306_WHITE);
    // Only draw pupil if eye is sufficiently open
    if (h > 12) {
      d.fillCircle(x + w / 2 + pX, y + top + h / 2 + pY, 4, SSD1306_BLACK);
    }
  }
}

void renderEmotionFrame(Adafruit_SSD1306 &d, Emotion emo, int frame) {
  d.clearDisplay();
  int lx = LEFT_EYE_X, rx = RIGHT_EYE_X, ey = EYE_Y, cx = 64;
  d.invertDisplay(false);

  // Common Lip Sync Logic
  int mouthH =
      isTalkingNow ? ((frame % 4 == 0) ? 2 : ((frame % 4 < 2) ? 6 : 8)) : 2;
  int mouthY = 50;

  // Seamless "Breathing" Animation (Bounce)
  // DISABLE global bounce to prevent jitter as requested ("olpo olpo upor
  // nichu").
  int bounce = 0;

  // *** UNIVERSAL BLINKING OVERRIDE ***
  // STRICT RULE: Blinking ONLY happens in NEUTRAL state.
  if (currentEyeHeight < (EYE_HEIGHT - 4) && emo == NEUTRAL) {
    drawStandardEye(d, lx, ey, 24, currentEyeHeight, pupilX, pupilY);
    drawStandardEye(d, rx, ey, 24, currentEyeHeight, pupilX, pupilY);

    // Draw Mouth (Maintain current emotion's mouth style during blink)
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, mouthY, 16, mouthH, 4, SSD1306_WHITE);
    } else {
      // Neutral Mouth during blink (Straight line)
      d.drawLine(cx - 10, mouthY, cx + 10, mouthY, SSD1306_WHITE);
    }
    d.display();
    return;
  }

  // During Emotions, pupilX/Y are forced to 0 in Main.ino for "Steady" (shuja)
  // look. We use the variables here anyway just in case.

  switch (emo) {
  case LOVE: { // Heart Eyes Pulse + Floating Hearts
    int pulse = (frame % 20 < 10) ? 0 : 2;
    d.fillCircle(lx + 6, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillCircle(lx + 18, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillTriangle(lx - pulse, ey + 15, lx + 24 + pulse, ey + 15, lx + 12,
                   ey + 27 + pulse, SSD1306_WHITE);

    d.fillCircle(rx + 6, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillCircle(rx + 18, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillTriangle(rx - pulse, ey + 15, rx + 24 + pulse, ey + 15, rx + 12,
                   ey + 27 + pulse, SSD1306_WHITE);

    // Floating heart bits
    for (int i = 0; i < 2; i++) {
      int hX = (frame * 7 + i * 50) % 128;
      int hY = 64 - ((frame + i * 30) % 64);
      d.drawPixel(hX, hY, SSD1306_WHITE);
    }

    mouthY = 46;
    d.fillRoundRect(cx - 8, mouthY, 16, mouthH, 4, SSD1306_WHITE);
  } break;

  case LAUGH: { // Joy - Big Smile + Arched Eyes + Joy Drops + Shake
    // Base offset for centering
    ey += 4;
    mouthY += 2;
    int lShake = (frame % 4 < 2) ? 0 : 2;
    ey += lShake;
    mouthY += lShake;

    // Eyes: Joyful Arches
    d.drawCircle(lx + 12, ey + 18, 10, SSD1306_WHITE);
    d.fillRect(lx, ey + 18, 24, 12, SSD1306_BLACK); // Crop bottom

    d.drawCircle(rx + 12, ey + 18, 10, SSD1306_WHITE);
    d.fillRect(rx, ey + 18, 24, 12, SSD1306_BLACK); // Crop bottom

    // High Arches (brows)
    d.drawCircle(lx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillRect(lx, ey + 14, 24, 12, SSD1306_BLACK);

    d.drawCircle(rx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillRect(rx, ey + 14, 24, 12, SSD1306_BLACK);

    // Recognizable Joy Drops (😂 style)
    d.fillCircle(lx - 4, ey + 10, 3, SSD1306_WHITE);
    d.fillTriangle(lx - 4, ey + 6, lx - 7, ey + 10, lx - 1, ey + 10,
                   SSD1306_WHITE);

    d.fillCircle(rx + 28, ey + 10, 3, SSD1306_WHITE);
    d.fillTriangle(rx + 28, ey + 6, rx + 25, ey + 10, rx + 31, ey + 10,
                   SSD1306_WHITE);

    // Mouth: Balanced Wide Open D shape
    d.fillRoundRect(cx - 14, mouthY - 4, 28,
                    isTalkingNow ? (10 + (frame % 4)) : 14, 6, SSD1306_WHITE);
    d.drawLine(cx - 12, mouthY, cx + 12, mouthY, SSD1306_BLACK);
  } break;

  case SLEEP: // Zzz
    d.fillRect(lx, ey + 18, 24, 3, SSD1306_WHITE);
    d.fillRect(rx, ey + 18, 24, 3, SSD1306_WHITE);
    {
      for (int i = 0; i < 3; i++) {
        int pos = (frame + i * 14) % 50;
        int zY = ey - 5 - pos;
        int zX = cx + (i * 10) + (pos / 2);
        if (zY < ey + 5 && zY > -5) {
          d.setCursor(zX, zY);
          d.setTextSize(1);
          d.write(i == 0 ? 'Z' : 'z');
        }
      }
    }
    d.fillCircle(cx, 52, isTalkingNow ? 3 : (3 + (frame % 30 < 15 ? 0 : 2)),
                 SSD1306_WHITE);
    break;

  case WINK: { // Subtle open eye pulse
    int blink = (frame % 30 < 2) ? 2 : 24;
    drawStandardEye(d, lx, ey, 24, blink, 0, 0);
    d.fillRect(rx, ey + 14, 24, 4, SSD1306_WHITE); // Wink Right
    d.fillRoundRect(cx - 10, 50, 20, mouthH, 2, SSD1306_WHITE);
  } break;

  case MISCHIEVOUS: { // Smirk twitch
    int twitch = (frame % 10 < 5) ? 0 : 1;
    d.fillTriangle(lx, ey + 10, lx + 24, ey + 15, lx + 5, ey + 25,
                   SSD1306_WHITE);
    d.fillCircle(lx + 8 + pupilX, ey + 18 + pupilY, 3, SSD1306_BLACK);
    d.fillTriangle(rx + 24, ey + 10, rx, ey + 15, rx + 19, ey + 25,
                   SSD1306_WHITE);
    d.fillCircle(rx + 16 + pupilX, ey + 18 + pupilY, 3, SSD1306_BLACK);
    if (isTalkingNow) {
      d.fillRoundRect(cx - 10, 50, 20, mouthH, 2, SSD1306_WHITE);
    } else {
      d.fillTriangle(cx - 12, 52 + twitch, cx + 12, 48, cx, 58, SSD1306_WHITE);
    }
  } break;

  case ANGRY_RAGE: { // Flashing + Jitter
    int jitter = (frame % 2 == 0) ? 1 : -1;
    lx += jitter;
    rx += jitter;
    ey += jitter;
    d.invertDisplay((frame % 6) < 3);
    d.fillTriangle(lx, ey + 8, lx + 24, ey + 18, lx, ey + 28, SSD1306_WHITE);
    d.fillTriangle(rx + 24, ey + 8, rx, ey + 18, rx + 24, ey + 28,
                   SSD1306_WHITE);
    d.fillRoundRect(cx - 12, 48 + jitter, 24, isTalkingNow ? 10 : 8, 3,
                    SSD1306_WHITE);
  } break;

  case SHOCKED: { // Pupil Dilation Pulse
    int dil = (frame % 10 < 5) ? 2 : 4;
    d.fillCircle(lx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillCircle(lx + 12, ey + 14, dil, SSD1306_BLACK);
    d.fillCircle(rx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillCircle(rx + 12, ey + 14, dil, SSD1306_BLACK);
    d.drawCircle(cx, 52, 8 + (isTalkingNow ? 2 : 0), SSD1306_WHITE);
  } break;

  case SAD: { // 😢 - Sad Heavy Eyes + Single Tear + Lip Sync
    drawStandardEye(d, lx, ey, 24, 14, 0, 0);
    drawStandardEye(d, rx, ey, 24, 14, 0, 0);

    // Single tear drop (Stops at frame 30)
    int tY = frame;
    if (tY > 30)
      tY = 30;
    d.fillCircle(lx + 4, ey + 18 + (tY / 2), 2, SSD1306_WHITE);

    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 54, 16, mouthH, 4, SSD1306_WHITE);
    } else {
      d.drawLine(cx - 10, 56, cx, 54, SSD1306_WHITE);
      d.drawLine(cx, 54, cx + 10, 56, SSD1306_WHITE);
    }
  } break;

  case DEAD: { // Static Flicker + Jittery Stitches
    int flicker = (frame % 4 == 0) ? 2 : 0;
    d.drawLine(lx + flicker, ey + 5, lx + 24 - flicker, ey + 25, SSD1306_WHITE);
    d.drawLine(lx + 24 - flicker, ey + 5, lx + flicker, ey + 25, SSD1306_WHITE);
    d.drawLine(rx + flicker, ey + 5, rx + 24 - flicker, ey + 25, SSD1306_WHITE);
    d.drawLine(rx + 24 - flicker, ey + 5, rx + flicker, ey + 25, SSD1306_WHITE);

    int mOffset = isTalkingNow ? (frame % 2) : 0;
    d.fillRect(cx - 10, 50 + mOffset, 20, 6, SSD1306_WHITE);
    d.drawLine(cx, 50 + mOffset, cx, 56 + mOffset, SSD1306_BLACK);
    d.drawLine(cx - 5, 50 + mOffset, cx - 5, 56 + mOffset, SSD1306_BLACK);
    d.drawLine(cx + 5, 50 + mOffset, cx + 5, 56 + mOffset, SSD1306_BLACK);
  } break;

  case DIZZY: { // Spirals
                // Draw Circles
    d.drawCircle(lx + 12, ey + 12, 10, SSD1306_WHITE);
    d.drawCircle(rx + 12, ey + 12, 10, SSD1306_WHITE);

    // Simulate spiral with orbiting dot
    float angle = (frame % 16) * 0.39; // ~2PI/16
    int dx = (int)(6 * cos(angle));
    int dy = (int)(6 * sin(angle));

    d.fillCircle(lx + 12 + dx, ey + 12 + dy, 3, SSD1306_WHITE);
    d.fillCircle(rx + 12 - dx, ey + 12 - dy, 3, SSD1306_WHITE); // Opposite spin

    // Wobbly mouth line
    int wob = (frame % 4 < 2) ? 1 : -1;
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 52 + wob, 16, mouthH, 2, SSD1306_WHITE);
    } else {
      d.drawLine(cx - 10, 52 + wob, cx + 10, 52 - wob, SSD1306_WHITE);
    }

  } break;

  case PARTY: // Confetti + Happy + Cap
    // Party Cap (Tilted Triangle)
    d.fillTriangle(lx + 2, ey - 2, lx + 20, ey - 2, lx + 11, ey - 16,
                   SSD1306_WHITE);
    d.fillCircle(lx + 11, ey - 16, 2, SSD1306_WHITE); // Pom-pom

    // Happy Eyes
    drawStandardEye(d, lx, ey, 24, 24, 0, 0);
    drawStandardEye(d, rx, ey, 24, 24, 0, 0);

    // Confetti Pixels (Randomized by frame)
    for (int i = 0; i < 15; i++) {
      int px = (frame * (i + 3)) % 128;
      int py = (frame * (i + 7)) % 64;
      if ((px + py) % 2 == 0)
        d.drawPixel(px, py, SSD1306_WHITE);
    }

    // Party Blower (Horn) logic
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 48, 16, mouthH, 4, SSD1306_WHITE);
    } else {
      // Blowing horn animation
      int ext = (frame % 8) * 2;
      d.fillCircle(cx, 48, 4, SSD1306_WHITE);                   // Mouth
      d.drawLine(cx + 4, 48, cx + 15 + ext, 45, SSD1306_WHITE); // Horn shaft
      d.fillTriangle(cx + 15 + ext, 42, cx + 15 + ext, 48, cx + 20 + ext, 45,
                     SSD1306_WHITE); // Bell
    }
    break;

  case SKEPTICAL: { // Eyebrow move
    int eb = (frame % 20 < 10) ? 0 : 3;
    d.fillRoundRect(lx, ey + 6 + eb, 24, 18, 6, SSD1306_WHITE);
    d.fillCircle(lx + 12 + pupilX, ey + 15 + pupilY + eb, 4, SSD1306_BLACK);
    d.fillRoundRect(rx, ey - 6 + eb, 24, 24, 6, SSD1306_WHITE);
    d.fillCircle(rx + 12 + pupilX, ey + 6 + pupilY + eb, 4, SSD1306_BLACK);
    if (isTalkingNow) {
      d.fillRoundRect(cx - 6, 50, 12, mouthH, 2, SSD1306_WHITE);
    } else {
      d.drawLine(cx - 6, 52, cx + 6, 50, SSD1306_WHITE);
    }
  } break;

  case FRUSTRATED: { // 😤 Dynamic Steam Puffs + Jitter
    // Subtle frustration jitter
    int jitter = (frame % 4 < 2) ? 1 : 0;
    ey += jitter;

    // Eyes: Closed angry slants \ /
    d.drawLine(lx, ey + 15, lx + 24, ey + 22, SSD1306_WHITE);
    d.drawLine(lx, ey + 16, lx + 24, ey + 23, SSD1306_WHITE);
    d.drawLine(rx + 24, ey + 15, rx, ey + 22, SSD1306_WHITE);
    d.drawLine(rx + 24, ey + 16, rx, ey + 23, SSD1306_WHITE);

    // Steam Puffs Animation (3 clouds per side, staggered)
    for (int i = 0; i < 3; i++) {
      int stage = (frame + i * 6) % 18;
      if (stage < 14) {
        int sX = 8 + stage;
        int sY = 40 + stage;
        int size = 2 + (stage / 4);
        d.fillCircle(cx - sX, sY, size, SSD1306_WHITE);
        d.fillCircle(cx + sX, sY, size, SSD1306_WHITE);
      }
    }

    // Tense mouth line
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 54 + jitter, 16, mouthH / 2 + 2, 0,
                      SSD1306_WHITE);
    } else {
      d.drawLine(cx - 8, 54 + jitter, cx + 8, 54 + jitter, SSD1306_WHITE);
    }
  } break;

  case ANGEL: { // 😇 Halo Bounce + Arched Eyes
    // Compact face to fit Halo
    ey += 10;    // Move eyes down
    mouthY += 4; // Move mouth down
    int bounce = (frame % 20 < 10) ? 0 : 2;

    // Draw Halo (Flattened Oval to fit screen)
    // Center (64, 8), RadiusX 18, RadiusY 5
    int hCx = cx;
    int hCy = 8 - bounce;
    for (int i = 0; i < 360; i += 5) {
      float rad = i * 0.01745;
      int hx = hCx + 18 * cos(rad);
      int hy = hCy + 5 * sin(rad);
      d.drawPixel(hx, hy, SSD1306_WHITE);
      // Thickness
      if (i % 2 == 0)
        d.drawPixel(hx, hy - 1, SSD1306_WHITE);
    }

    // Eyes: Happy Arches (Closed Happy)
    // Draw Arc by drawing circle and masking bottom
    d.drawCircle(lx + 12, ey + 8 + bounce, 10, SSD1306_WHITE);
    d.fillRect(lx, ey + 8 + bounce, 24, 14,
               SSD1306_BLACK); // Crop bottom to leave 'n'

    d.drawCircle(rx + 12, ey + 8 + bounce, 10, SSD1306_WHITE);
    d.fillRect(rx, ey + 8 + bounce, 24, 14, SSD1306_BLACK); // Crop bottom

    // Mouth: Small smile (u shape)
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, mouthY + bounce, 16, mouthH, 6, SSD1306_WHITE);
    } else {
      // Draw Circle and masking TOP to leave 'u'
      d.drawCircle(cx, mouthY - 4 + bounce, 8, SSD1306_WHITE);
      d.fillRect(cx - 10, mouthY - 14 + bounce, 20, 10, SSD1306_BLACK);
    }
  } break;

  case CRYING: { // 😭 - Sad Heavy Eyes + Stable Waterfall (One-shot)
    drawStandardEye(d, lx, ey, 24, 14, 0, 0);
    drawStandardEye(d, rx, ey, 24, 14, 0, 0);

    // Tears waterfall animation (Stops at frame 32)
    int flow = frame;
    if (flow > 32)
      flow = 32;
    d.fillRect(lx + 4, ey + 20 + (flow / 2), 3, 6, SSD1306_WHITE);
    d.fillRect(rx + 17, ey + 20 + (flow / 2), 3, 6, SSD1306_WHITE);

    // Mouth: Sad Oval
    d.fillRoundRect(cx - 10, mouthY, 20, isTalkingNow ? (10 + (frame % 4)) : 14,
                    8, SSD1306_WHITE);
  } break;

  default: // NEUTRAL
    // Fix: Remove vertical offset. Eye anchors to bottom, lid lowers.
    drawStandardEye(d, lx, ey, 24, currentEyeHeight, pupilX, pupilY);
    drawStandardEye(d, rx, ey, 24, currentEyeHeight, pupilX, pupilY);

    if (isTalkingNow)
      d.fillRoundRect(cx - 8, 50, 16, mouthH, 3, SSD1306_WHITE);
    else
      // Steady flat mouth (😐 style)
      d.drawLine(cx - 10, 52, cx + 10, 52, SSD1306_WHITE);
    break;
  }
  d.display();
}

#endif
