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
  CRYING,      // 😭
  WAKE_UP      // 😲
};

extern Emotion currentEmotion;
extern int currentFrame;
extern int targetPupilX, targetPupilY;
extern int pupilX, pupilY;
extern int currentEyeHeight, targetEyeHeight;
extern bool autoPilotActive;
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
  // Micro-saccades (ultra-rare jitters for extreme realism)
  static int microX = 0, microY = 0;
  if (random(1000) < 5) {
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
    // Draw Pupil & Light Effect
    if (h > 12) {
      int px = x + w / 2 + pX;
      int py = y + top + h / 2 + pY;
      // Pupil (Black part)
      d.fillCircle(px, py, 4, SSD1306_BLACK);
      // Light Effect / Gleam (Small white highlight)
      d.fillCircle(px - 1, py - 1, 1, SSD1306_WHITE);
    }
  }
}

inline void renderEmotionFrame(Adafruit_SSD1306 &d, Emotion emo, int frame) {
  d.clearDisplay();
  int lx = LEFT_EYE_X, rx = RIGHT_EYE_X, ey = EYE_Y, cx = 64;
  d.invertDisplay(false);

  // Common Lip Sync Logic
  int mouthH =
      isTalkingNow ? ((frame % 4 == 0) ? 2 : ((frame % 4 < 2) ? 6 : 8)) : 2;
  int mouthY = 50;

  // *** UNIVERSAL BLINKING OVERRIDE ***
  if (currentEyeHeight < (EYE_HEIGHT - 4) && emo == NEUTRAL) {
    drawStandardEye(d, lx, ey, 24, currentEyeHeight, pupilX, pupilY);
    drawStandardEye(d, rx, ey, 24, currentEyeHeight, pupilX, pupilY);

    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, mouthY, 16, mouthH, 4, SSD1306_WHITE);
    } else {
      d.drawLine(cx - 10, mouthY, cx + 10, mouthY, SSD1306_WHITE);
    }
    d.display();
    return;
  }

  switch (emo) {
  case LOVE: {
    int pulse = (frame % 20 < 10) ? 0 : 2;
    d.fillCircle(lx + 6, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillCircle(lx + 18, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillTriangle(lx - pulse, ey + 15, lx + 24 + pulse, ey + 15, lx + 12,
                   ey + 27 + pulse, SSD1306_WHITE);

    d.fillCircle(rx + 6, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillCircle(rx + 18, ey + 12, 6 + pulse, SSD1306_WHITE);
    d.fillTriangle(rx - pulse, ey + 15, rx + 24 + pulse, ey + 15, rx + 12,
                   ey + 27 + pulse, SSD1306_WHITE);

    for (int i = 0; i < 2; i++) {
      int hX = (frame * 7 + i * 50) % 128;
      int hY = 64 - ((frame + i * 30) % 64);
      d.drawPixel(hX, hY, SSD1306_WHITE);
    }

    mouthY = 46;
    d.fillRoundRect(cx - 8, mouthY, 16, mouthH, 4, SSD1306_WHITE);
  } break;

  case LAUGH: {
    ey += 4;
    mouthY += 2;
    int lShake = (frame % 4 < 2) ? 0 : 2;
    ey += lShake;
    mouthY += lShake;

    d.drawCircle(lx + 12, ey + 18, 10, SSD1306_WHITE);
    d.fillRect(lx, ey + 18, 24, 12, SSD1306_BLACK);
    d.drawCircle(rx + 12, ey + 18, 10, SSD1306_WHITE);
    d.fillRect(rx, ey + 18, 24, 12, SSD1306_BLACK);

    d.drawCircle(lx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillRect(lx, ey + 14, 24, 12, SSD1306_BLACK);
    d.drawCircle(rx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillRect(rx, ey + 14, 24, 12, SSD1306_BLACK);

    d.fillCircle(lx - 4, ey + 10, 3, SSD1306_WHITE);
    d.fillTriangle(lx - 4, ey + 6, lx - 7, ey + 10, lx - 1, ey + 10,
                   SSD1306_WHITE);

    d.fillCircle(rx + 28, ey + 10, 3, SSD1306_WHITE);
    d.fillTriangle(rx + 28, ey + 6, rx + 25, ey + 10, rx + 31, ey + 10,
                   SSD1306_WHITE);

    d.fillRoundRect(cx - 14, mouthY - 4, 28,
                    isTalkingNow ? (10 + (frame % 4)) : 14, 6, SSD1306_WHITE);
    d.drawLine(cx - 12, mouthY, cx + 12, mouthY, SSD1306_BLACK);
  } break;

  case SLEEP:
    d.fillRect(lx, ey + 18, 24, 3, SSD1306_WHITE);
    d.fillRect(rx, ey + 18, 24, 3, SSD1306_WHITE);
    {
      d.setTextSize(1);
      d.setTextColor(SSD1306_WHITE);
      for (int i = 0; i < 3; i++) {
        int pos = (frame + i * 15) % 60;
        int zY = ey - pos;
        int zX = cx + 10 + (i * 12) + (pos / 4);
        if (zY > -10 && zY < 64) {
          d.setCursor(zX, zY);
          d.print(i == 0 ? "Z" : "z");
        }
      }
    }
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 52, 16, mouthH, 4, SSD1306_WHITE);
    } else {
      d.fillCircle(cx, 52, (3 + (frame % 30 < 15 ? 0 : 2)), SSD1306_WHITE);
    }
    break;

  case WINK: {
    drawStandardEye(d, lx, ey, 24, EYE_HEIGHT, pupilX, pupilY);
    int winkHeight = EYE_HEIGHT;
    if (frame == 1 || frame == 4)
      winkHeight = EYE_HEIGHT / 2;
    else if (frame == 2 || frame == 3)
      winkHeight = 2;
    else if (frame >= 5)
      winkHeight = EYE_HEIGHT;

    drawStandardEye(d, rx, ey, 24, winkHeight, 0, 0);
    d.fillRoundRect(cx - 10, 50, 20, mouthH, 2, SSD1306_WHITE);
  } break;

  case MISCHIEVOUS: {
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

  case ANGRY_RAGE: {
    int jitter = (frame % 2 == 0) ? 1 : -1;
    lx += jitter;
    rx += jitter;
    ey += jitter;
    d.invertDisplay((frame % 6) < 3);
    d.fillTriangle(lx, ey + 8, lx + 24, ey + 18, lx, ey + 28, SSD1306_WHITE);
    d.fillTriangle(rx + 24, ey + 8, rx, ey + 18, rx + 24, ey + 28,
                   SSD1306_WHITE);
    d.fillRoundRect(cx - 12, 48 + jitter, 24, isTalkingNow ? mouthH + 4 : 8, 3,
                    SSD1306_WHITE);
  } break;

  case SHOCKED: {
    int dil = (frame % 10 < 5) ? 2 : 4;
    d.fillCircle(lx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillCircle(lx + 12, ey + 14, dil, SSD1306_BLACK);
    d.fillCircle(rx + 12, ey + 14, 10, SSD1306_WHITE);
    d.fillCircle(rx + 12, ey + 14, dil, SSD1306_BLACK);
    d.drawCircle(cx, 52, isTalkingNow ? (mouthH + 4) : 8, SSD1306_WHITE);
  } break;

  case SAD: {
    drawStandardEye(d, lx, ey, 24, 14, 0, 0);
    drawStandardEye(d, rx, ey, 24, 14, 0, 0);
    int tY = (frame % 40);
    d.fillCircle(lx + 4, ey + 18 + (tY / 2), 2, SSD1306_WHITE);

    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 54, 16, mouthH, 4, SSD1306_WHITE);
    } else {
      d.drawLine(cx - 10, 56, cx, 54, SSD1306_WHITE);
      d.drawLine(cx, 54, cx + 10, 56, SSD1306_WHITE);
    }
  } break;

  case DEAD: {
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

  case DIZZY: {
    d.drawCircle(lx + 12, ey + 12, 10, SSD1306_WHITE);
    d.drawCircle(rx + 12, ey + 12, 10, SSD1306_WHITE);
    float angle = (frame % 16) * 0.39;
    int dx = (int)(6 * cos(angle));
    int dy = (int)(6 * sin(angle));
    d.fillCircle(lx + 12 + dx, ey + 12 + dy, 3, SSD1306_WHITE);
    d.fillCircle(rx + 12 - dx, ey + 12 - dy, 3, SSD1306_WHITE);
    int wob = (frame % 4 < 2) ? 1 : -1;
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 52 + wob, 16, mouthH, 2, SSD1306_WHITE);
    } else {
      d.drawLine(cx - 10, 52 + wob, cx + 10, 52 - wob, SSD1306_WHITE);
    }
  } break;

  case PARTY:
    d.fillTriangle(lx + 2, ey - 2, lx + 20, ey - 2, lx + 11, ey - 16,
                   SSD1306_WHITE);
    d.fillCircle(lx + 11, ey - 16, 2, SSD1306_WHITE);
    drawStandardEye(d, lx, ey, 24, 24, 0, 0);
    drawStandardEye(d, rx, ey, 24, 24, 0, 0);
    for (int i = 0; i < 15; i++) {
      int px = (frame * (i + 3)) % 128;
      int py = (frame * (i + 7)) % 64;
      if ((px + py) % 2 == 0)
        d.drawPixel(px, py, SSD1306_WHITE);
    }
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 48, 16, mouthH, 4, SSD1306_WHITE);
    } else {
      int ext = (frame % 8) * 2;
      d.fillCircle(cx, 48, 4, SSD1306_WHITE);
      d.drawLine(cx + 4, 48, cx + 15 + ext, 45, SSD1306_WHITE);
      d.fillTriangle(cx + 15 + ext, 42, cx + 15 + ext, 48, cx + 20 + ext, 45,
                     SSD1306_WHITE);
    }
    break;

  case SKEPTICAL: {
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

  case FRUSTRATED: {
    int jitter = (frame % 4 < 2) ? 1 : 0;
    ey += jitter;
    d.drawLine(lx, ey + 15, lx + 24, ey + 22, SSD1306_WHITE);
    d.drawLine(lx, ey + 16, lx + 24, ey + 23, SSD1306_WHITE);
    d.drawLine(rx + 24, ey + 15, rx, ey + 22, SSD1306_WHITE);
    d.drawLine(rx + 24, ey + 16, rx, ey + 23, SSD1306_WHITE);
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
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, 54 + jitter, 16, mouthH / 2 + 2, 0,
                      SSD1306_WHITE);
    } else {
      d.drawLine(cx - 8, 54 + jitter, cx + 8, 54 + jitter, SSD1306_WHITE);
    }
  } break;

  case ANGEL: {
    ey += 10;
    mouthY += 4;
    int bnc = (frame % 20 < 10) ? 0 : 2;
    int hCx = cx;
    int hCy = 8 - bnc;
    for (int i = 0; i < 360; i += 5) {
      float rad = i * 0.01745;
      float hx = hCx + 18 * cos(rad);
      float hy = hCy + 5 * sin(rad);
      d.drawPixel((int)hx, (int)hy, SSD1306_WHITE);
      if (i % 2 == 0)
        d.drawPixel((int)hx, (int)hy - 1, SSD1306_WHITE);
    }
    d.drawCircle(lx + 12, ey + 8 + bnc, 10, SSD1306_WHITE);
    d.fillRect(lx, ey + 8 + bnc, 24, 14, SSD1306_BLACK);
    d.drawCircle(rx + 12, ey + 8 + bnc, 10, SSD1306_WHITE);
    d.fillRect(rx, ey + 8 + bnc, 24, 14, SSD1306_BLACK);
    if (isTalkingNow) {
      d.fillRoundRect(cx - 8, mouthY + bnc, 16, mouthH, 6, SSD1306_WHITE);
    } else {
      d.drawCircle(cx, mouthY - 4 + bnc, 8, SSD1306_WHITE);
      d.fillRect(cx - 10, mouthY - 14 + bnc, 20, 10, SSD1306_BLACK);
    }
  } break;

  case CRYING: {
    drawStandardEye(d, lx, ey, 24, 14, 0, 0);
    drawStandardEye(d, rx, ey, 24, 14, 0, 0);
    for (int i = 0; i < 2; i++) {
      int flow = (frame + (i * 15)) % 30;
      int tY = ey + 18 + flow;
      if (tY < 64) {
        d.fillRect(lx + 4, tY, 3, 8, SSD1306_WHITE);
        d.fillRect(rx + 17, tY, 3, 8, SSD1306_WHITE);
      }
    }
    d.fillRoundRect(cx - 10, mouthY, 20, isTalkingNow ? (10 + (frame % 4)) : 14,
                    8, SSD1306_WHITE);
  } break;

  case WAKE_UP: {
    int shakeX = (frame < 10) ? random(-3, 4) : 0;
    int shakeY = (frame < 10) ? random(-3, 4) : 0;
    lx += shakeX;
    rx += shakeX;
    ey += shakeY;
    mouthY += shakeY;
    drawStandardEye(d, lx, ey, 24, 28, 0, 0);
    drawStandardEye(d, rx, ey, 24, 28, 0, 0);
    int mw = 12;
    int mh = isTalkingNow ? (12 + (frame % 4)) : 12;
    d.fillCircle(cx + shakeX, mouthY + 2, mw / 2 + 2, SSD1306_WHITE);
    d.fillCircle(cx + shakeX, mouthY + 2, mw / 2, SSD1306_BLACK);
  } break;

  default:
    drawStandardEye(d, lx, ey, 24, currentEyeHeight, pupilX, pupilY);
    drawStandardEye(d, rx, ey, 24, currentEyeHeight, pupilX, pupilY);
    if (isTalkingNow)
      d.fillRoundRect(cx - 8, 50, 16, mouthH, 3, SSD1306_WHITE);
    else
      d.drawLine(cx - 10, 52, cx + 10, 52, SSD1306_WHITE);
    break;
  }
  d.display();
}

#endif
