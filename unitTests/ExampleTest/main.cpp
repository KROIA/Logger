#ifdef QT_ENABLED
#include <QApplication>
#endif
#include <iostream>
#include "Logger.h"
#include <iostream>
#include "tests.h"

#ifdef QT_WIDGETS_ENABLED
#include <QWidget>
#endif

// Instantiate Tests here:
// TEST_INSTANTIATE(Test_simple); // Where Test_simple is a derived class from the Test class
TEST_INSTANTIATE(TST_simple); 

int main(int argc, char* argv[])
{
#ifdef QT_WIDGETS_ENABLED
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#ifdef QT_ENABLED
	QApplication app(argc, argv);
#endif

	Log::LibraryInfo::printInfo();

	std::cout << "Running "<< UnitTest::Test::getTests().size() << " tests...\n";
	UnitTest::Test::TestResults results;
	UnitTest::Test::runAllTests(results);
	UnitTest::Test::printResults(results);

	return results.getSuccess();
}
