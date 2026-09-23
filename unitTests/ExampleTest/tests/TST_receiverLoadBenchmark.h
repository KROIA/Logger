#pragma once

#include "UnitTest.h"
#include "Logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QEventLoop>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>

// Load benchmark for the receiver pipeline.
//
// Purpose: a repeatable reference measurement of what every receiver costs per
// message, so batching / dirty-flag optimizations can be compared against a
// fixed baseline. Benchmark first, test second — the only hard assertions are
// that the burst really was queued and that no message was dropped.
//
// Two phases, deliberately different:
//   1. "burst"     — automatic event processing off, kBurstMessages log calls
//                    with the Qt event loop never running, so they all pile up
//                    in the queue and are delivered in one drain. This is the
//                    case where per-message UI work is pure waste: nobody can
//                    see 4000 intermediate states.
//   2. "streaming" — one message, one processEvents(), repeated. Worst case for
//                    receivers that rebuild their UI on every message.
//
// Build the static-profile profile (LOGGER_PROFILING) to also get
// "ReceiverLoadBenchmark.prof" for the easy_profiler GUI; without it
// Profiler::start/stop are no-ops and only the TEST_MESSAGE timings appear.
//
// Runtime knobs (so the load can be varied without a rebuild):
//   BENCH_BURST=<n>    burst message count           (default 4000)
//   BENCH_STREAM=<n>   streaming message count       (default 1000)
//   BENCH_VIEWS=<mask> 1=table 2=tree 4=timeline 8=stats 16=FilePlotter,
//                      OR-ed (default 31 = everything)
//   BENCH_SHOW=1       show the view windows         (default: hidden)
//
// Windows are hidden by default: shown windows add window-manager and repaint
// timing noise that makes a before/after comparison unreliable. Use
// BENCH_SHOW=1 deliberately when the paint cost is what you want to measure;
// TST_receiverRegressions pins that the shown case no longer crashes.
//
// Not covered on purpose: NativeConsoleView (its console I/O would dominate
// every number) and QCombinedConsoleView (a composition of the views already
// instantiated here — it would double-count them).
class TST_receiverLoadBenchmark : public UnitTest::Test
{
	TEST_CLASS(TST_receiverLoadBenchmark)
public:
	TST_receiverLoadBenchmark()
		: Test("TST_receiverLoadBenchmark")
	{
		ADD_TEST(TST_receiverLoadBenchmark::messageFlood);
	}

private:

	// Message count queued without the event loop running (phase 1).
	static inline int kBurstMessages = 4000;
	// Message count delivered one-by-one through the event loop (phase 2).
	static inline int kStreamMessages = 1000;
	// Context tree width. More contexts make the per-message rebuild loops in
	// the stats and tree views more expensive, so this number is part of the
	// baseline — do not change it without re-recording the reference.
	static constexpr int kLoggerCount = 8;
	// How often a lifecycle event (new logger / rename / reparent) is mixed
	// into the message stream.
	static constexpr int kLifecycleEveryNth = 500;

	// Minimal receiver that only counts. Used to prove nothing was dropped and
	// to detect when the queue has actually been drained.
	class CountingReceiver : public Log::AbstractReceiver
	{
	public:
		std::atomic<int> messages{ 0 };
		std::atomic<int> loggers{ 0 };
		std::atomic<int> infoChanges{ 0 };
		std::atomic<int> reparents{ 0 };

		void reset()
		{
			messages = 0;
			loggers = 0;
			infoChanges = 0;
			reparents = 0;
		}
	protected:
		void onNewLogger(Log::LogObject::Info) override { ++loggers; }
		void onLoggerInfoChanged(Log::LogObject::Info) override { ++infoChanges; }
		void onLogMessage(Log::Message) override { ++messages; }
		void onChangeParent(Log::LoggerID, Log::LoggerID) override { ++reparents; }
	};

	static int envInt(const char* name, int fallback)
	{
		const char* value = std::getenv(name);
		return value ? std::atoi(value) : fallback;
	}

	static double msSince(const std::chrono::steady_clock::time_point& start)
	{
		const auto end = std::chrono::steady_clock::now();
		return std::chrono::duration<double, std::milli>(end - start).count();
	}

	static std::string report(const std::string& label, double totalMs, int count)
	{
		const double perMsgUs = count > 0 ? (totalMs * 1000.0 / count) : 0.0;
		return label + ": " + std::to_string(totalMs) + " ms total, " +
			std::to_string(perMsgUs) + " us/message (" + std::to_string(count) + " messages)";
	}

	// Drains the Qt event queue. Receivers re-post their own flush events while
	// being drained, so a single processEvents() is not enough; loop until the
	// counting receiver has seen everything, then flush the tail.
	static void drainEventQueue(CountingReceiver& counter, int expectedMessages)
	{
		for (int i = 0; i < 64 && counter.messages.load() < expectedMessages; ++i)
			QCoreApplication::processEvents(QEventLoop::AllEvents);
		// Tail pass: the last batch's flush and repaint events were posted
		// during the final iteration above and are still pending.
		QCoreApplication::processEvents(QEventLoop::AllEvents);
	}

	TEST_FUNCTION(messageFlood)
	{
		TEST_START;

		kBurstMessages = envInt("BENCH_BURST", kBurstMessages);
		kStreamMessages = envInt("BENCH_STREAM", kStreamMessages);
		const int viewMask = envInt("BENCH_VIEWS", 31);
		const bool showViews = envInt("BENCH_SHOW", 0) != 0;

		const bool previousAutoProcessing = Log::LogManager::getEnableAutomaticEventProcessing();
		// The whole point of phase 1 is that the event loop never runs during
		// the emit loop. LogManager's built-in periodic processEvents() would
		// drain the queue behind our back.
		Log::LogManager::setEnableAutomaticEventProcessing(false);

		const QString plotterPath = QDir::temp().filePath("logger_load_benchmark.json");
		QFile::remove(plotterPath);

		CountingReceiver counter;

		// Receivers under test. Raw pointers plus explicit delete: the views are
		// QWidgets that must outlive the flood, and the FilePlotter has to be
		// closed before its file is removed.
		Log::FilePlotter* plotter = (viewMask & 16)
			? new Log::FilePlotter(plotterPath.toStdString())
			: nullptr;
#ifdef QT_WIDGETS_LIB
		Log::UI::QConsoleView* tableView = (viewMask & 1) ? new Log::UI::QConsoleView() : nullptr;
		Log::UI::QTreeConsoleView* treeView = (viewMask & 2) ? new Log::UI::QTreeConsoleView() : nullptr;
		Log::UI::QVerticalTimelineView* timelineView = (viewMask & 4) ? new Log::UI::QVerticalTimelineView() : nullptr;
		Log::UI::QStatsConsoleView* statsView = (viewMask & 8) ? new Log::UI::QStatsConsoleView() : nullptr;
		if (showViews)
		{
			if (tableView) tableView->show();
			if (treeView) treeView->show();
			if (timelineView) timelineView->show();
			if (statsView) statsView->show();
		}
#else
		LOGGER_UNUSED(viewMask);
		LOGGER_UNUSED(showViews);
#endif

		// Context tree: logger 0 is the root, the rest are its children, so the
		// tree view has real nesting to maintain.
		std::vector<Log::LogObject*> loggers;
		loggers.reserve(kLoggerCount);
		loggers.push_back(new Log::LogObject("bench.root"));
		for (int i = 1; i < kLoggerCount; ++i)
			loggers.push_back(new Log::LogObject(loggers[0]->getID(), "bench.child" + std::to_string(i)));
		// The only logger that is ever reparented during the flood. It is a
		// leaf, so moving it can never build a parent cycle — and a parent
		// cycle is not survivable, see ISS-006.
		Log::LogObject* mover = loggers.back();

		// Let the receivers finish registering the contexts before measuring.
		drainEventQueue(counter, 0);
		counter.reset();

		Log::Profiler::start();

		// ---- Phase 1: burst -------------------------------------------------
		double burstEmitMs = 0.0;
		double burstDrainMs = 0.0;
		int burstLifecycleEvents = 0;
		{
			LOGGER_GENERAL_PROFILING_BLOCK("Benchmark: burst emit", LOGGER_COLOR_STAGE_1);
			const auto start = std::chrono::steady_clock::now();
			for (int i = 0; i < kBurstMessages; ++i)
			{
				Log::LogObject* logger = loggers[i % kLoggerCount];
				const Log::Level level = static_cast<Log::Level>(i % static_cast<int>(Log::Level::__count));
				logger->log("burst message " + std::to_string(i) +
					" from " + logger->getName(), level);

				// Mix in the lifecycle events the receivers also have to handle.
				if (i > 0 && (i % kLifecycleEveryNth) == 0)
				{
					switch ((i / kLifecycleEveryNth) % 3)
					{
					case 0:
						loggers.push_back(new Log::LogObject(loggers[0]->getID(),
							"bench.late" + std::to_string(i)));
						break;
					case 1:
						logger->setName(logger->getName() + "'");
						break;
					default:
						mover->setParentID(mover->getParentID() == loggers[0]->getID()
							? loggers[1]->getID()
							: loggers[0]->getID());
						break;
					}
					++burstLifecycleEvents;
				}
			}
			burstEmitMs = msSince(start);
		}

		// Nothing may have been delivered yet — that is what makes this a
		// burst. If this fails, something processed events during the emit loop
		// and the drain measurement below is not the scenario we think it is.
		const int deliveredBeforeDrain = counter.messages.load();

		{
			LOGGER_GENERAL_PROFILING_BLOCK("Benchmark: burst drain", LOGGER_COLOR_STAGE_2);
			const auto start = std::chrono::steady_clock::now();
			drainEventQueue(counter, kBurstMessages);
			burstDrainMs = msSince(start);
		}
		const int burstDelivered = counter.messages.load();

		// ---- Phase 2: streaming --------------------------------------------
		counter.reset();
		double streamMs = 0.0;
		{
			LOGGER_GENERAL_PROFILING_BLOCK("Benchmark: streaming", LOGGER_COLOR_STAGE_3);
			const auto start = std::chrono::steady_clock::now();
			for (int i = 0; i < kStreamMessages; ++i)
			{
				Log::LogObject* logger = loggers[i % kLoggerCount];
				const Log::Level level = static_cast<Log::Level>(i % static_cast<int>(Log::Level::__count));
				logger->log("stream message " + std::to_string(i), level);
				QCoreApplication::processEvents(QEventLoop::AllEvents);
			}
			streamMs = msSince(start);
		}
		drainEventQueue(counter, kStreamMessages);
		const int streamDelivered = counter.messages.load();

		Log::Profiler::stop("ReceiverLoadBenchmark.prof");

		// ---- Results --------------------------------------------------------
		TEST_MESSAGE(std::string("config: views=") + std::to_string(viewMask) +
			(showViews ? std::string(" shown") : std::string(" hidden")));
		TEST_MESSAGE(report("burst emit   ", burstEmitMs, kBurstMessages));
		TEST_MESSAGE(report("burst drain  ", burstDrainMs, kBurstMessages));
		TEST_MESSAGE(report("burst total  ", burstEmitMs + burstDrainMs, kBurstMessages));
		TEST_MESSAGE(report("streaming    ", streamMs, kStreamMessages));
		TEST_MESSAGE(std::string("lifecycle events mixed into the burst: ") +
			std::to_string(burstLifecycleEvents));

		TEST_ASSERT_M(deliveredBeforeDrain == 0,
			"burst phase must not deliver anything before the drain, but " +
			std::to_string(deliveredBeforeDrain) + " messages arrived — "
			"something processed events during the emit loop");
		TEST_ASSERT_M(burstDelivered == kBurstMessages,
			"burst phase must deliver every message: expected " +
			std::to_string(kBurstMessages) + ", got " + std::to_string(burstDelivered));
		TEST_ASSERT_M(streamDelivered == kStreamMessages,
			"streaming phase must deliver every message: expected " +
			std::to_string(kStreamMessages) + ", got " + std::to_string(streamDelivered));

		// ---- Cleanup --------------------------------------------------------
#ifdef QT_WIDGETS_LIB
		delete statsView;
		delete timelineView;
		delete treeView;
		delete tableView;
#endif
		delete plotter;
		QFile::remove(plotterPath);
		for (Log::LogObject* logger : loggers)
			delete logger;

		Log::LogManager::setEnableAutomaticEventProcessing(previousAutoProcessing);
	}
};
