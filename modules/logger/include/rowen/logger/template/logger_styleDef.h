#pragma once

enum LoggerStyle : int
{
  LOGGER_STYLE_UNSET = -1,
  LOGGER_STYLE_RESET = 0,

  // Font Style (0b00000001)
  FONT_STYLE_BOLD      = 1 << 0,  // 굵게
  FONT_STYLE_BLUR      = 1 << 1,  // 흐리게
  FONT_STYLE_ITALIC    = 1 << 2,  // 기울임
  FONT_STYLE_UNDERLINE = 1 << 3,  // 밑줄
  FONT_STYLE_REVERSE   = 1 << 4,  // 전경/배경 반전
  FONT_STYLE_HIDE      = 1 << 5,  // 숨김
  FONT_STYLE_STRIKE    = 1 << 6,  // 취소선

  // Font Color (0b00000001_00000000)
  FONT_COLOR_BLACK  = 1 << 8,   // 검정색
  FONT_COLOR_RED    = 1 << 9,   // 빨간색
  FONT_COLOR_GREEN  = 1 << 10,  // 초록색
  FONT_COLOR_YELLOW = 1 << 11,  // 노란색
  FONT_COLOR_BLUE   = 1 << 12,  // 파란색
  FONT_COLOR_PURPLE = 1 << 13,  // 자주색
  FONT_COLOR_CYAN   = 1 << 14,  // 청록색
  FONT_COLOR_WHITE  = 1 << 15,  // 흰색

  // Background Color (0b00000001_00000000_00000000)
  BACKGROUND_BLACK  = 1 << 16,  // 검정색
  BACKGROUND_RED    = 1 << 17,  // 빨간색
  BACKGROUND_GREEN  = 1 << 18,  // 초록색
  BACKGROUND_YELLOW = 1 << 19,  // 노란색
  BACKGROUND_BLUE   = 1 << 20,  // 파란색
  BACKGROUND_PURPLE = 1 << 21,  // 자주색
  BACKGROUND_CYAN   = 1 << 22,  // 청록색
  BACKGROUND_WHITE  = 1 << 23   // 흰색
};

static constexpr int RS_LOGGER_FATAL_STYLE = FONT_COLOR_RED | BACKGROUND_WHITE;
static constexpr int RS_LOGGER_ERROR_STYLE = FONT_STYLE_BOLD | FONT_COLOR_RED;
static constexpr int RS_LOGGER_WARN_STYLE  = FONT_STYLE_BOLD | FONT_COLOR_YELLOW;
static constexpr int RS_LOGGER_INFO_STYLE  = LOGGER_STYLE_UNSET;
static constexpr int RS_LOGGER_DEBUG_STYLE = FONT_COLOR_CYAN;
static constexpr int RS_LOGGER_TRACE_STYLE = FONT_STYLE_BLUR | FONT_COLOR_CYAN;
