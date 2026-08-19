#pragma once

#include "UnitTest.h"
#include "Logger.h"




class TST_receiverFilter : public UnitTest::Test
{
	TEST_CLASS(TST_receiverFilter)
public:
	TST_receiverFilter()
		: Test("TST_receiverFilter")
	{
		ADD_TEST(TST_receiverFilter::filterByLoggerID_includeChildren);
		ADD_TEST(TST_receiverFilter::filter_agrees_with_filterByLoggerID);
		ADD_TEST(TST_receiverFilter::copy_preserves_includeChildren);
	}

private:

	// Include mode + includeChildren: root and its child accepted, unrelated rejected.
	TEST_FUNCTION(filterByLoggerID_includeChildren)
	{
		TEST_START;

		Log::LogObject root("root");
		Log::LogObject child(root.getID(), "child");
		Log::LogObject unrelated("unrelated");

		Log::LoggerIDFilter f;
		f.setMode(Log::LoggerIDFilter::Include);
		f.setIncludeChildren(true);
		f.addLoggerID(root.getID());

		TEST_ASSERT_M(f.filterByLoggerID(root.getID()), "root should be accepted");
		TEST_ASSERT_M(f.filterByLoggerID(child.getID()), "child should be accepted (includeChildren)");
		TEST_ASSERT_M(!f.filterByLoggerID(unrelated.getID()), "unrelated should be rejected");
	}

	// filter(Message) must delegate to filterByLoggerID.
	TEST_FUNCTION(filter_agrees_with_filterByLoggerID)
	{
		TEST_START;

		Log::LogObject root("root2");
		Log::LogObject unrelated("unrelated2");

		Log::LoggerIDFilter f;
		f.setMode(Log::LoggerIDFilter::Include);
		f.addLoggerID(root.getID());

		Log::Message inMsg("hi");
		inMsg.setLoggerID(root.getID());
		Log::Message outMsg("nope");
		outMsg.setLoggerID(unrelated.getID());

		TEST_ASSERT_M(f.filter(inMsg) == f.filterByLoggerID(root.getID()), "filter must agree for in-set id");
		TEST_ASSERT_M(f.filter(outMsg) == f.filterByLoggerID(unrelated.getID()), "filter must agree for out-of-set id");
		TEST_ASSERT_M(f.filter(inMsg), "in-set message accepted");
		TEST_ASSERT_M(!f.filter(outMsg), "out-of-set message rejected");
	}

	// Copy-ctor and operator= must preserve m_includeChildren (ISS-002).
	TEST_FUNCTION(copy_preserves_includeChildren)
	{
		TEST_START;

		Log::LoggerIDFilter f;
		f.setMode(Log::LoggerIDFilter::Include);
		f.setIncludeChildren(true);
		f.addLoggerID(42);

		Log::LoggerIDFilter copied = f;
		TEST_ASSERT_M(copied.getIncludeChildren(), "copy-ctor must preserve includeChildren");
		TEST_ASSERT_M(copied == f, "copy-ctor must produce an equal filter");

		Log::LoggerIDFilter assigned;
		assigned = f;
		TEST_ASSERT_M(assigned.getIncludeChildren(), "operator= must preserve includeChildren");
		TEST_ASSERT_M(assigned == f, "operator= must produce an equal filter");
	}

};
