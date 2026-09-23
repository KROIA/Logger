#pragma once

#include "UnitTest.h"
#include "Logger.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <atomic>
#include <string>

// Regression pins for the two crashes found by TST_receiverLoadBenchmark.
class TST_receiverRegressions : public UnitTest::Test
{
	TEST_CLASS(TST_receiverRegressions)
public:
	TST_receiverRegressions()
		: Test("TST_receiverRegressions")
	{
		ADD_TEST(TST_receiverRegressions::statsView_survivesQueuedBurst);
		ADD_TEST(TST_receiverRegressions::parentCycle_isRejected);
	}

private:

	class CountingReceiver : public Log::AbstractReceiver
	{
	public:
		std::atomic<int> messages{ 0 };
	protected:
		void onNewLogger(Log::LogObject::Info) override {}
		void onLoggerInfoChanged(Log::LogObject::Info) override {}
		void onLogMessage(Log::Message) override { ++messages; }
		void onChangeParent(Log::LoggerID, Log::LoggerID) override {}
	};

	static void drain(CountingReceiver& counter, int expected)
	{
		for (int i = 0; i < 64 && counter.messages.load() < expected; ++i)
			QCoreApplication::processEvents(QEventLoop::AllEvents);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
	}

	// ISS-005: QStatsConsoleView used to rebuild every bar widget on every
	// message. With the window shown, a burst of ~1000 queued messages then
	// terminated the process with a stack overflow during the drain. The view
	// is shown on purpose here — hiding it is what made the old code survive.
	TEST_FUNCTION(statsView_survivesQueuedBurst)
	{
		TEST_START;

		static constexpr int kMessages = 2000;

		const bool previousAutoProcessing = Log::LogManager::getEnableAutomaticEventProcessing();
		Log::LogManager::setEnableAutomaticEventProcessing(false);

		CountingReceiver counter;
#ifdef QT_WIDGETS_LIB
		Log::UI::QStatsConsoleView* view = new Log::UI::QStatsConsoleView();
		view->show();
#endif
		Log::LogObject logger("statsBurst");
		drain(counter, 0);
		counter.messages = 0;

		for (int i = 0; i < kMessages; ++i)
			logger.log("stats burst " + std::to_string(i),
				static_cast<Log::Level>(i % static_cast<int>(Log::Level::__count)));

		TEST_ASSERT_M(counter.messages.load() == 0,
			"the burst must still be queued — nothing may have run the event loop");

		drain(counter, kMessages);

		TEST_ASSERT_M(counter.messages.load() == kMessages,
			"every queued message must be delivered: expected " +
			std::to_string(kMessages) + ", got " + std::to_string(counter.messages.load()));

#ifdef QT_WIDGETS_LIB
		delete view;
#endif
		Log::LogManager::setEnableAutomaticEventProcessing(previousAutoProcessing);
	}

	// ISS-006: a cyclic parent chain crashed the tree view and hung every
	// ancestor walk. LogManager must refuse to create one.
	TEST_FUNCTION(parentCycle_isRejected)
	{
		TEST_START;

		Log::LogObject root("cycle.root");
		Log::LogObject child(root.getID(), "cycle.child");
		Log::LogObject grandChild(child.getID(), "cycle.grandChild");

		TEST_ASSERT_M(root.getParentID() == 0, "root starts without a parent");
		TEST_ASSERT_M(child.getParentID() == root.getID(), "child starts under root");

		// Direct self-parenting.
		root.setParentID(root.getID());
		TEST_ASSERT_M(root.getParentID() == 0, "a logger must not become its own parent");

		// One level down.
		root.setParentID(child.getID());
		TEST_ASSERT_M(root.getParentID() == 0, "a logger must not become a child of its own child");

		// Two levels down.
		root.setParentID(grandChild.getID());
		TEST_ASSERT_M(root.getParentID() == 0, "a logger must not become a child of its own grandchild");

		// A legal reparent still has to go through.
		Log::LogObject other("cycle.other");
		grandChild.setParentID(other.getID());
		TEST_ASSERT_M(grandChild.getParentID() == other.getID(), "a non-cyclic reparent must be applied");

		// The ancestor walk must terminate — this used to be an unbounded loop.
		TEST_ASSERT_M(Log::LogManager::isChildOf(child.getID(), root.getID()),
			"child is still a descendant of root");
		TEST_ASSERT_M(!Log::LogManager::isChildOf(root.getID(), child.getID()),
			"root must not be reported as a descendant of its own child");
	}
};
