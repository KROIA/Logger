// @file ReadOnlyCellEditorDelegate.h
// @brief Internal item delegate shared by QConsoleWidget and QContextLoggerTreeWidget.
//
// Implementation detail: this header lives under core/src, is not installed and
// carries no LOGGER_API. Both widgets used to hold a byte-identical private copy
// of this class; it is defined once here instead.
#pragma once

#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPersistentModelIndex>
#include <QPlainTextEdit>
#include <QStyledItemDelegate>
#include <QTextCursor>
#include <QTextOption>
#include <functional>

namespace Log
{
	namespace UIWidgets
	{
		namespace Internal
		{
			// Read-only in-place editor: click a cell to enter text-selection mode.
			// A double-click anywhere (on the view OR on an open editor) copies the
			// whole message text of the clicked row to the clipboard. The delegate
			// installs an event filter on every editor it creates so double-click
			// works even after an editor has taken focus. Escape (pressed while the
			// editor has focus) invokes the owner's deselect handler.
			//
			// The editor is a QPlainTextEdit, not a QLineEdit: a QLineEdit is
			// single-line by construction and collapses the embedded '\n' of a
			// multi-line log message, while the painted cell (QStyledItemDelegate's
			// QTextLayout) honours them. Both paths now break lines the same way.
			class ReadOnlyCellEditorDelegate : public QStyledItemDelegate
			{
			public:
				ReadOnlyCellEditorDelegate(int messageColumn, std::function<void()> onEscape, QObject* parent)
					: QStyledItemDelegate(parent), m_messageColumn(messageColumn), m_onEscape(std::move(onEscape)) {}

				QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
				{
					QPlainTextEdit* editor = new QPlainTextEdit(parent);
					editor->setReadOnly(true);
					editor->setFrameStyle(QFrame::NoFrame);
					// The views paint with setWordWrap(false) / ElideNone, so the
					// editor must not wrap either.
					editor->setLineWrapMode(QPlainTextEdit::NoWrap);
					editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
					editor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
					editor->setFont(option.font);
					editor->setAutoFillBackground(true);
					editor->setContextMenuPolicy(Qt::NoContextMenu);
					editor->viewport()->setContextMenuPolicy(Qt::NoContextMenu);
					editor->setContentsMargins(0, 0, 0, 0);
					// Without this the text jumps by the default 4 px document
					// margin the moment the editor replaces the painted cell.
					editor->document()->setDocumentMargin(0);
					setHorizontalAlignment(editor, option.displayAlignment);

					QPalette pal = editor->palette();
					const QVariant bg = index.data(Qt::BackgroundRole);
					if (bg.isValid() && bg.canConvert<QBrush>())
					{
						const QColor bgc = bg.value<QBrush>().color();
						if (bgc.isValid())
						{
							pal.setColor(QPalette::Base, bgc);
							pal.setColor(QPalette::Window, bgc);
						}
					}
					const QVariant fg = index.data(Qt::ForegroundRole);
					if (fg.isValid() && fg.canConvert<QBrush>())
					{
						const QColor fgc = fg.value<QBrush>().color();
						if (fgc.isValid() && fgc.alpha() > 0)
							pal.setColor(QPalette::Text, fgc);
					}
					// Keep selection highlight visible even when the editor loses focus
					// (Qt would otherwise switch to the Inactive palette group, which on
					// most themes renders the selection as invisible or nearly so).
					pal.setColor(QPalette::Inactive, QPalette::Highlight,
						pal.color(QPalette::Active, QPalette::Highlight));
					pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
						pal.color(QPalette::Active, QPalette::HighlightedText));
					editor->setPalette(pal);

					editor->setPlainText(index.data(Qt::DisplayRole).toString());
					editor->setProperty("__persistentIndex", QVariant::fromValue(QPersistentModelIndex(index)));
					editor->installEventFilter(const_cast<ReadOnlyCellEditorDelegate*>(this));
					// Mouse events land on the viewport of a scroll area, not on the
					// editor itself — a filter on the editor alone would never see
					// the double-click (a QLineEdit had no viewport to lose it to).
					editor->viewport()->installEventFilter(const_cast<ReadOnlyCellEditorDelegate*>(this));
					return editor;
				}
				void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const override {}
				// Preserve the editor's text selection and cursor across model
				// refreshes (Qt calls setEditorData on persistent editors whenever it
				// thinks the underlying data may have changed).
				void setEditorData(QWidget* editor, const QModelIndex& index) const override
				{
					QPlainTextEdit* te = qobject_cast<QPlainTextEdit*>(editor);
					if (!te)
					{
						QStyledItemDelegate::setEditorData(editor, index);
						return;
					}
					const QString newText = index.data(Qt::DisplayRole).toString();
					if (te->toPlainText() == newText)
						return;
					const QTextCursor old = te->textCursor();
					const bool hadSelection = old.hasSelection();
					const int selStart = old.selectionStart();
					const int selEnd = old.selectionEnd();
					const int position = old.position();
					// A block separator counts as one position, so the plain text
					// length is the highest reachable cursor position.
					const int maxPos = newText.length();
					const bool wasBlocked = te->blockSignals(true);
					te->setPlainText(newText);
					QTextCursor restored = te->textCursor();
					if (hadSelection)
					{
						restored.setPosition(qMin(selStart, maxPos));
						restored.setPosition(qMin(selEnd, maxPos), QTextCursor::KeepAnchor);
					}
					else
					{
						restored.setPosition(qMin(position, maxPos));
					}
					te->setTextCursor(restored);
					te->blockSignals(wasBlocked);
				}
				void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override
				{
					editor->setGeometry(option.rect);
				}

				bool eventFilter(QObject* obj, QEvent* ev) override
				{
					if (ev->type() == QEvent::KeyPress)
					{
						QKeyEvent* ke = static_cast<QKeyEvent*>(ev);
						if (ke->key() == Qt::Key_Escape && m_onEscape)
						{
							// Consume before QStyledItemDelegate's default Escape
							// handling, which would close the editor widget but leave
							// the owner's selection bookkeeping (and follow-pause) set.
							m_onEscape();
							return true;
						}
					}
					if (ev->type() == QEvent::MouseButtonDblClick)
					{
						QPlainTextEdit* editor = editorOf(obj);
						if (editor)
						{
							QPersistentModelIndex idx = editor->property("__persistentIndex").value<QPersistentModelIndex>();
							if (idx.isValid())
							{
								const QString text = idx.model()->index(idx.row(), m_messageColumn, idx.parent())
									.data(Qt::DisplayRole).toString();
								if (!text.isEmpty())
									QApplication::clipboard()->setText(text);
								// Consume so the editor's word-select doesn't fight the copy gesture.
								return true;
							}
						}
					}
					return QStyledItemDelegate::eventFilter(obj, ev);
				}
			private:
				// obj is either the editor or its viewport, depending on the event.
				static QPlainTextEdit* editorOf(QObject* obj)
				{
					if (QPlainTextEdit* te = qobject_cast<QPlainTextEdit*>(obj))
						return te;
					return obj ? qobject_cast<QPlainTextEdit*>(obj->parent()) : nullptr;
				}
				// QPlainTextEdit has no setAlignment covering both axes. The
				// horizontal flags of option.displayAlignment are carried over via
				// the document's default text option (which, unlike a block format,
				// survives setPlainText); vertical centering has no equivalent and
				// is not applied — the row is sized to the text, so there is
				// nothing to center within.
				static void setHorizontalAlignment(QPlainTextEdit* editor, Qt::Alignment displayAlignment)
				{
					const Qt::Alignment horizontal = displayAlignment & Qt::AlignHorizontal_Mask;
					QTextOption textOption = editor->document()->defaultTextOption();
					textOption.setAlignment(horizontal != 0 ? horizontal : Qt::AlignLeft);
					editor->document()->setDefaultTextOption(textOption);
				}

				int m_messageColumn;
				std::function<void()> m_onEscape;
			};
		}
	}
}
#endif
