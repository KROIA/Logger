#pragma once

#include "UnitTest.h"
#include "Logger.h"
#include <QApplication>
#include <QPalette>


// CR-THEME1: the console/table/tree path must be theme-aware instead of
// hard-coding a dark row background.
class TST_themeColors : public UnitTest::Test
{
	TEST_CLASS(TST_themeColors)
public:
	TST_themeColors()
		: Test("TST_themeColors")
	{
		ADD_TEST(TST_themeColors::darkBackground_isUnchanged);
		ADD_TEST(TST_themeColors::lightBackground_isNearWhite);
		ADD_TEST(TST_themeColors::explicitMode_beatsPalette);
		ADD_TEST(TST_themeColors::consumerLevelColors_surviveThemeSwitch);
	}

private:

	// Color::operator== is value-insensitive (it ANDs the channels), so every
	// comparison here goes through the channel getters.
	static bool isColor(const Log::Color& c, int r, int g, int b)
	{
		return c.getRed() == r && c.getGreen() == g && c.getBlue() == b;
	}

	// Restores the global theme state so test order cannot leak.
	class ThemeGuard
	{
	public:
		ThemeGuard()
			: m_mode(Log::Color::getThemeMode())
			, m_palette(QApplication::palette())
		{}
		~ThemeGuard()
		{
			QApplication::setPalette(m_palette);
			Log::Color::setThemeMode(m_mode);
		}
	private:
		Log::ThemeMode m_mode;
		QPalette m_palette;
	};

	// Criterion 2: no regression. Dark must stay byte-identical to the
	// hard-coded `loggerColor * 0.5f` the three call sites used to carry.
	TEST_FUNCTION(darkBackground_isUnchanged)
	{
		TEST_START;
		ThemeGuard guard;

		Log::Color::setThemeMode(Log::ThemeMode::Dark);
		const Log::Color logger(128, 128, 128);
		const Log::Color bg = Log::Color::loggerBackgroundColor(logger);
		const Log::Color expected = logger * 0.5f;
		TEST_ASSERT_M(isColor(bg, expected.getRed(), expected.getGreen(), expected.getBlue()),
			"dark background must equal loggerColor * 0.5f, got " + bg.getRGBStr());
		TEST_ASSERT_M(isColor(bg, 64, 64, 64), "0.5 * 128 must be 64, got " + bg.getRGBStr());

		// A saturated logger color too, not just the grey the consumer uses.
		const Log::Color red = Log::Color::loggerBackgroundColor(Log::Colors::red);
		TEST_ASSERT_M(isColor(red, 127, 0, 0), "dark red background must be 127,0,0, got " + red.getRGBStr());
	}

	// Criterion 1: the FB-45 case. The downstream consumer's Color(128,128,128)
	// logger must yield a near-white row tint, not the mid-grey ceiling.
	TEST_FUNCTION(lightBackground_isNearWhite)
	{
		TEST_START;
		ThemeGuard guard;

		Log::Color::setThemeMode(Log::ThemeMode::Light);
		const Log::Color bg = Log::Color::loggerBackgroundColor(Log::Color(128, 128, 128));
		TEST_ASSERT_M(bg.getRed() >= 230 && bg.getGreen() >= 230 && bg.getBlue() >= 230,
			"light background must be near white, got " + bg.getRGBStr());

		// Loggers must still be distinguishable from one another.
		const Log::Color other = Log::Color::loggerBackgroundColor(Log::Colors::red);
		TEST_ASSERT_M(!isColor(other, bg.getRed(), bg.getGreen(), bg.getBlue()),
			"different loggers must still get different tints");

		// And info must be readable on it.
		TEST_ASSERT_M(Log::Message::getLevelColor(Log::Level::info).getRed() < 128,
			"light-theme info must be dark, not white");
	}

	// Criterion 3: an explicit mode wins over the palette, both directions.
	TEST_FUNCTION(explicitMode_beatsPalette)
	{
		TEST_START;
		ThemeGuard guard;

		QPalette dark;
		dark.setColor(QPalette::Base, QColor(30, 30, 30));
		QApplication::setPalette(dark);

		Log::Color::setThemeMode(Log::ThemeMode::Auto);
		TEST_ASSERT_M(Log::Color::isDarkTheme(), "Auto must resolve a dark Base to dark");
		Log::Color::setThemeMode(Log::ThemeMode::Light);
		TEST_ASSERT_M(!Log::Color::isDarkTheme(), "explicit Light must beat a dark palette");

		QPalette light;
		light.setColor(QPalette::Base, QColor(255, 255, 255));
		QApplication::setPalette(light);

		Log::Color::setThemeMode(Log::ThemeMode::Auto);
		TEST_ASSERT_M(!Log::Color::isDarkTheme(), "Auto must resolve a light Base to light");
		Log::Color::setThemeMode(Log::ThemeMode::Dark);
		TEST_ASSERT_M(Log::Color::isDarkTheme(), "explicit Dark must beat a light palette");
	}

	// Criterion 4: a consumer's own palette outranks both built-in sets,
	// in every theme mode, across any number of switches.
	TEST_FUNCTION(consumerLevelColors_surviveThemeSwitch)
	{
		TEST_START;
		ThemeGuard guard;

		Log::LevelColors custom{
			Log::Color(1, 2, 3),		// trace
			Log::Color(4, 5, 6),		// debug
			Log::Color(7, 8, 9),		// info
			Log::Color(10, 11, 12),		// warning
			Log::Color(13, 14, 15),		// error
			Log::Color(16, 17, 18)		// custom
		};

		Log::Color::setThemeMode(Log::ThemeMode::Dark);
		Log::Message::setLevelColors(custom);

		Log::Color::setThemeMode(Log::ThemeMode::Light);
		TEST_ASSERT_M(isColor(Log::Message::getLevelColor(Log::Level::info), 7, 8, 9),
			"dark -> light must not clobber a consumer's info color");
		TEST_ASSERT_M(isColor(Log::Message::getLevelColors().warning, 10, 11, 12),
			"dark -> light must not clobber a consumer's warning color");

		Log::Color::setThemeMode(Log::ThemeMode::Dark);
		TEST_ASSERT_M(isColor(Log::Message::getLevelColor(Log::Level::info), 7, 8, 9),
			"light -> dark must not clobber a consumer's info color");

		// Hand the library's dark defaults back, so a later test in this run
		// sees the colors it would have seen without this one.
		Log::LevelColors defaults{
			Log::Colors::cyan,
			Log::Colors::magenta,
			Log::Colors::white,
			Log::Colors::yellow,
			Log::Colors::red,
			Log::Colors::green
		};
		Log::Message::setLevelColors(defaults);
	}
};
