#pragma once

#include "UnitTest.h"
#include "Logger.h"

#ifdef QT_WIDGETS_LIB
// Implementation-detail header: the delegate is internal to the library and
// deliberately not installed, so it is reached by relative path.
#include "../../../core/src/ui/Widgets/ReadOnlyCellEditorDelegate.h"
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QWidget>
#endif


// ISS-007: clicking a cell replaced the painted (multi-line) text with a
// single-line QLineEdit, collapsing every '\n' in the message.
class TST_cellEditorMultiline : public UnitTest::Test
{
	TEST_CLASS(TST_cellEditorMultiline)
public:
	TST_cellEditorMultiline()
		: Test("TST_cellEditorMultiline")
	{
#ifdef QT_WIDGETS_LIB
		ADD_TEST(TST_cellEditorMultiline::editor_keepsLineBreaks);
		ADD_TEST(TST_cellEditorMultiline::setEditorData_keepsSelection);
#endif
	}

private:

#ifdef QT_WIDGETS_LIB

	// A cell editor opened over "line one\nline two" must still carry both
	// lines — that is exactly what QLineEdit could not do.
	TEST_FUNCTION(editor_keepsLineBreaks)
	{
		TEST_START;

		QStandardItemModel model(1, 1);
		model.setData(model.index(0, 0), QString("line one\nline two"), Qt::DisplayRole);

		QWidget parent;
		Log::UIWidgets::Internal::ReadOnlyCellEditorDelegate delegate(0, nullptr, nullptr);
		QStyleOptionViewItem option;
		QWidget* editor = delegate.createEditor(&parent, option, model.index(0, 0));
		TEST_ASSERT_M(editor != nullptr, "createEditor must return an editor");

		QPlainTextEdit* te = qobject_cast<QPlainTextEdit*>(editor);
		TEST_ASSERT_M(te != nullptr, "the cell editor must be a QPlainTextEdit, not a single-line widget");

		const QString text = te->toPlainText();
		TEST_ASSERT_M(text.contains('\n'),
			"the editor must keep the line break, got: " + text.toStdString());
		TEST_ASSERT_M(text == QString("line one\nline two"),
			"the editor text must match the display text verbatim, got: " + text.toStdString());
		TEST_ASSERT_M(te->document()->blockCount() == 2,
			"the editor must lay the text out as 2 lines, got " +
			std::to_string(te->document()->blockCount()));

		delete editor;
	}

	// Qt calls setEditorData on a persistent editor whenever it thinks the data
	// changed. The user's selection must survive that, or every incoming log
	// message would clear it.
	TEST_FUNCTION(setEditorData_keepsSelection)
	{
		TEST_START;

		QStandardItemModel model(1, 1);
		model.setData(model.index(0, 0), QString("line one\nline two"), Qt::DisplayRole);

		QWidget parent;
		Log::UIWidgets::Internal::ReadOnlyCellEditorDelegate delegate(0, nullptr, nullptr);
		QStyleOptionViewItem option;
		QPlainTextEdit* te = qobject_cast<QPlainTextEdit*>(
			delegate.createEditor(&parent, option, model.index(0, 0)));
		TEST_ASSERT_M(te != nullptr, "the cell editor must be a QPlainTextEdit");

		// Select "line" on the first line.
		QTextCursor cursor = te->textCursor();
		cursor.setPosition(0);
		cursor.setPosition(4, QTextCursor::KeepAnchor);
		te->setTextCursor(cursor);
		TEST_ASSERT_M(te->textCursor().selectedText() == QString("line"),
			"precondition: the selection must be set before the refresh");

		// The row's text changes under the open editor.
		model.setData(model.index(0, 0), QString("line one\nline three"), Qt::DisplayRole);
		delegate.setEditorData(te, model.index(0, 0));

		TEST_ASSERT_M(te->toPlainText() == QString("line one\nline three"),
			"the editor must pick up the new text, got: " + te->toPlainText().toStdString());
		const QTextCursor after = te->textCursor();
		TEST_ASSERT_M(after.hasSelection(), "the user's selection must survive the refresh");
		TEST_ASSERT_M(after.selectionStart() == 0 && after.selectionEnd() == 4,
			"the selection must keep its range, got [" +
			std::to_string(after.selectionStart()) + ", " +
			std::to_string(after.selectionEnd()) + "]");

		delete te;
	}

#endif
};
