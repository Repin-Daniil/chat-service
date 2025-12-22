#include "message_text.hpp"

#include <gtest/gtest.h>

using namespace NChat::NCore::NDomain;

// ===== БАЗОВЫЕ ПОЗИТИВНЫЕ ТЕСТЫ =====

TEST(TMessageTextTest, ValidSimpleMessage) {
  EXPECT_NO_THROW({
    TMessageText text("Hello, World!");
    EXPECT_EQ(text.Value(), "Hello, World!");
    EXPECT_FALSE(text.IsEmpty());
  });
}

TEST(TMessageTextTest, ValidMessageWithNewlines) {
  EXPECT_NO_THROW({
    TMessageText text("Line 1\nLine 2\r\nLine 3");
    EXPECT_EQ(text.Value(), "Line 1\nLine 2\r\nLine 3");
  });
}

TEST(TMessageTextTest, ValidMessageWithTabs) {
  EXPECT_NO_THROW({
    TMessageText text("Column1\tColumn2\tColumn3");
    EXPECT_EQ(text.Value(), "Column1\tColumn2\tColumn3");
  });
}

TEST(TMessageTextTest, SingleCharacter) {
  EXPECT_NO_THROW({
    TMessageText text("A");
    EXPECT_EQ(text.Value(), "A");
  });
}

// ===== ТЕСТЫ TRIM (пробелы по краям) =====

TEST(TMessageTextTest, TrimLeadingWhitespace) {
  TMessageText text("   Hello");
  EXPECT_EQ(text.Value(), "Hello");
}

TEST(TMessageTextTest, TrimTrailingWhitespace) {
  TMessageText text("Hello   ");
  EXPECT_EQ(text.Value(), "Hello");
}

TEST(TMessageTextTest, TrimBothSides) {
  TMessageText text("   Hello World   ");
  EXPECT_EQ(text.Value(), "Hello World");
}

TEST(TMessageTextTest, TrimWithNewlines) {
  TMessageText text("\n\n  Message  \r\n");
  EXPECT_EQ(text.Value(), "Message");
}

TEST(TMessageTextTest, InternalWhitespacePreserved) {
  TMessageText text("  Hello   World  ");
  EXPECT_EQ(text.Value(), "Hello   World");
}

// ===== НЕГАТИВНЫЕ ТЕСТЫ: ПУСТЫЕ СООБЩЕНИЯ =====

TEST(TMessageTextTest, EmptyStringThrows) {
  EXPECT_THROW({ TMessageText text(""); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, OnlyWhitespaceThrows) {
  EXPECT_THROW({ TMessageText text("     "); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, OnlyNewlinesThrows) {
  EXPECT_THROW({ TMessageText text("\n\n\n"); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, OnlyTabsThrows) {
  EXPECT_THROW({ TMessageText text("\t\t\t"); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, MixedWhitespaceThrows) {
  EXPECT_THROW({ TMessageText text("  \t\n\r\n  "); }, TMessageTextInvalidException);
}

// ===== UTF-8 ТЕСТЫ =====

TEST(TMessageTextTest, CyrillicText) {
  EXPECT_NO_THROW({
    TMessageText text("Привет, мир!");
    EXPECT_EQ(text.Value(), "Привет, мир!");
  });
}

TEST(TMessageTextTest, ChineseText) {
  EXPECT_NO_THROW({
    TMessageText text("你好世界");
    EXPECT_EQ(text.Value(), "你好世界");
  });
}

TEST(TMessageTextTest, ArabicText) {
  EXPECT_NO_THROW({
    TMessageText text("مرحبا بالعالم");
    EXPECT_EQ(text.Value(), "مرحبا بالعالم");
  });
}

TEST(TMessageTextTest, JapaneseText) {
  EXPECT_NO_THROW({
    TMessageText text("こんにちは世界");
    EXPECT_EQ(text.Value(), "こんにちは世界");
  });
}

TEST(TMessageTextTest, EmojiSimple) {
  EXPECT_NO_THROW({
    TMessageText text("Hello 😀");
    EXPECT_EQ(text.Value(), "Hello 😀");
  });
}

TEST(TMessageTextTest, MultipleEmojis) {
  EXPECT_NO_THROW({
    TMessageText text("🔥💯👍🎉🚀");
    EXPECT_EQ(text.Value(), "🔥💯👍🎉🚀");
  });
}

TEST(TMessageTextTest, ComplexEmoji) {
  EXPECT_NO_THROW({
    TMessageText text("👨‍👩‍👧‍👦 family");
    EXPECT_EQ(text.Value(), "👨‍👩‍👧‍👦 family");
  });
}

TEST(TMessageTextTest, MixedLanguages) {
  EXPECT_NO_THROW({
    TMessageText text("Hello мир 世界 🌍");
    EXPECT_EQ(text.Value(), "Hello мир 世界 🌍");
  });
}

TEST(TMessageTextTest, SpecialUnicodeCharacters) {
  EXPECT_NO_THROW({
    TMessageText text("©®™€£¥");
    EXPECT_EQ(text.Value(), "©®™€£¥");
  });
}

TEST(TMessageTextTest, MathematicalSymbols) {
  EXPECT_NO_THROW({
    TMessageText text("∑∏∫√∞≠≈");
    EXPECT_EQ(text.Value(), "∑∏∫√∞≠≈");
  });
}

// ===== ТЕСТЫ НЕДОПУСТИМЫХ УПРАВЛЯЮЩИХ СИМВОЛОВ =====

TEST(TMessageTextTest, NullCharacterThrows) {
  std::string msg = "Hello\0World";
  msg.resize(11);
  EXPECT_THROW({ TMessageText text(msg); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, BellCharacterThrows) {
  EXPECT_THROW({ TMessageText text("Hello\x07"); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, BackspaceThrows) {
  EXPECT_THROW({ TMessageText text("Hello\x08"); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, EscapeCharacterThrows) {
  EXPECT_THROW({ TMessageText text("Hello\x1B"); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, MultipleControlCharactersThrow) {
  EXPECT_THROW({ TMessageText text("Hello\x01\x02\x03"); }, TMessageTextInvalidException);
}

// ===== ТЕСТЫ ДЛИНЫ (СИМВОЛЫ vs БАЙТЫ) =====

TEST(TMessageTextTest, MaxLengthASCII) {
  std::string msg(MAX_TEXT_CHARS, 'A');
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, ExceedMaxLengthASCII) {
  std::string msg(MAX_TEXT_CHARS + 1, 'A');
  EXPECT_THROW({ TMessageText text(msg); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, MaxLengthCyrillic) {
  // Кириллица: 2 байта на символ
  std::string msg;
  for (size_t i = 0; i < MAX_TEXT_CHARS; ++i) {
    msg += "а";  // U+0430
  }
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, ExceedMaxLengthCyrillic) {
  std::string msg;
  for (size_t i = 0; i < MAX_TEXT_CHARS + 1; ++i) {
    msg += "а";
  }
  EXPECT_THROW({ TMessageText text(msg); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, MaxLengthChinese) {
  // Китайские символы: обычно 3 байта
  std::string msg;
  for (size_t i = 0; i < MAX_TEXT_CHARS; ++i) {
    msg += "中";  // U+4E2D
  }
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, MaxLengthEmojis) {
  // Эмодзи: обычно 4 байта
  std::string msg;
  for (size_t i = 0; i < MAX_TEXT_CHARS; ++i) {
    msg += "😀";  // U+1F600
  }
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, ExceedMaxBytesButValidChars) {
  // 4097 символов по 4 байта = превышение лимита байт
  std::string msg;
  for (size_t i = 0; i < 4097; ++i) {
    msg += "😀";
  }
  EXPECT_THROW({ TMessageText text(msg); }, TMessageTextInvalidException);
}

TEST(TMessageTextTest, ExactlyMaxBytes) {
  std::string four_byte_char = "😀";  // 4 байта UTF-8

  std::string msg;
  msg.reserve(MAX_TEXT_BYTES);

  for (size_t i = 0; i < MAX_TEXT_CHARS; ++i) {
    msg += four_byte_char;
  }

  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, ExceedMaxBytes) {
  std::string msg(MAX_TEXT_BYTES + 1, 'A');
  EXPECT_THROW({ TMessageText text(msg); }, TMessageTextInvalidException);
}

// ===== ГРАНИЧНЫЕ СЛУЧАИ =====

TEST(TMessageTextTest, MixedCharacterSizes) {
  // 1000 ASCII + 1000 кириллицы + 1000 китайских + 1000 эмодзи = 4000 символов
  std::string msg;
  msg += std::string(1000, 'A');
  for (int i = 0; i < 1000; ++i) msg += "а";
  for (int i = 0; i < 1000; ++i) msg += "中";
  for (int i = 0; i < 1000; ++i) msg += "😀";

  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, AllowedControlCharactersOnly) {
  std::string msg = "Line1\nLine2\rLine3\tTab";
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, EdgeCase4096Characters) {
  std::string msg(4096, 'X');
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, EdgeCase4097Characters) {
  std::string msg(4097, 'X');
  EXPECT_THROW({ TMessageText text(msg); }, TMessageTextInvalidException);
}

// ===== ТЕСТЫ ИСКЛЮЧЕНИЙ =====

TEST(TMessageTextTest, ExceptionFieldName) {
  try {
    TMessageText text("");
    FAIL() << "Expected TMessageTextInvalidException";
  } catch (const TMessageTextInvalidException& e) {
    EXPECT_EQ(e.GetField(), "text");
  }
}

TEST(TMessageTextTest, ExceptionMessageForEmpty) {
  try {
    TMessageText text("   ");
    FAIL() << "Expected TMessageTextInvalidException";
  } catch (const TMessageTextInvalidException& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("empty"), std::string::npos);
  }
}

TEST(TMessageTextTest, ExceptionMessageForTooLong) {
  try {
    std::string long_msg(5000, 'A');
    TMessageText text(long_msg);
    FAIL() << "Expected TMessageTextInvalidException";
  } catch (const TMessageTextInvalidException& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("too long"), std::string::npos);
    EXPECT_NE(msg.find("4096"), std::string::npos);
  }
}

TEST(TMessageTextTest, ExceptionMessageForControlChars) {
  try {
    TMessageText text("Hello\x01World");
    FAIL() << "Expected TMessageTextInvalidException";
  } catch (const TMessageTextInvalidException& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("control"), std::string::npos);
  }
}

// ===== СТРЕСС-ТЕСТЫ И EDGE CASES =====

TEST(TMessageTextTest, OnlySpacesInMiddle) {
  EXPECT_NO_THROW({
    TMessageText text("A     B");
    EXPECT_EQ(text.Value(), "A     B");
  });
}

TEST(TMessageTextTest, RepeatedEmojis) {
  std::string msg;
  for (int i = 0; i < 100; ++i) {
    msg += "🎉";
  }
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

TEST(TMessageTextTest, ZeroWidthCharacters) {
  // Zero-width joiner, zero-width space
  EXPECT_NO_THROW({ TMessageText text("Test\u200B\u200C\u200DText"); });
}

TEST(TMessageTextTest, RightToLeftMarks) {
  EXPECT_NO_THROW({ TMessageText text("Test\u200E\u200FText"); });
}

TEST(TMessageTextTest, CombiningDiacritics) {
  EXPECT_NO_THROW({
    TMessageText text("e\u0301");  // é with combining acute
  });
}

TEST(TMessageTextTest, SurrogatePairsInEmojis) {
  // Эмодзи с surrogate pairs (уже в UTF-8)
  EXPECT_NO_THROW({
    TMessageText text("𝕳𝖊𝖑𝖑𝖔");  // Mathematical bold
  });
}

// ===== ПРОВЕРКА КОРРЕКТНОГО ПОДСЧЁТА UTF-8 =====

TEST(TMessageTextTest, UTF8CountingAscii) {
  std::string msg(100, 'A');
  TMessageText text(msg);
  // 100 ASCII символов
  EXPECT_EQ(text.Value().size(), 100);
}

TEST(TMessageTextTest, UTF8CountingMultibyte) {
  // "😀" = 4 байта, 1 символ
  std::string msg = "😀😀😀";
  TMessageText text(msg);
  // 12 байт, 3 символа
  EXPECT_EQ(text.Value().size(), 12);
}

TEST(TMessageTextTest, ContinuationBytesNotCountedAsChars) {
  // Проверяем что continuation bytes (10xxxxxx) не считаются
  std::string msg = "Привет";  // каждая буква по 2 байта
  EXPECT_NO_THROW({ TMessageText text(msg); });
}

// ===== СПЕЦИФИЧЕСКИЕ ПРОБЛЕМЫ HIGHLOAD =====

TEST(TMessageTextTest, NoMemoryLeakOnException) {
  // Проверка что при выбросе исключения нет утечек
  for (int i = 0; i < 1000; ++i) {
    EXPECT_THROW({ TMessageText text(""); }, TMessageTextInvalidException);
  }
}

TEST(TMessageTextTest, MoveSemantics) {
  std::string original = "Test Message";
  TMessageText text(std::move(original));
  // original должен быть перемещён
  EXPECT_EQ(text.Value(), "Test Message");
}

TEST(TMessageTextTest, LargeValidMessage) {
  std::string msg(4000, 'X');
  EXPECT_NO_THROW({
    TMessageText text(msg);
    EXPECT_FALSE(text.IsEmpty());
  });
}
