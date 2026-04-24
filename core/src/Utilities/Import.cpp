#include "Utilities/Import.h"
#include <QJsonDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>

namespace Log
{
	bool Import::loadFromFile(std::vector<std::pair<LogObject::Info, std::vector<Message>>>& contexts, const std::string& file)
	{
		QFile inFile(QString::fromStdString(file));
		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			return false;
		}
		QJsonDocument doc = QJsonDocument::fromJson(inFile.readAll());
		inFile.close();
        QJsonArray arr;
        if (doc.isArray())
        {
            arr = doc.array();
        }
        else
        {
			arr = parseLargeJson(file);
            if(arr.isEmpty())
            {
                return false;
			}
        }
		contexts.clear();

		std::map<LoggerID, LogObject::Info> logObjectsInfo;
		std::vector<Message> messages;

		for (const auto& value : arr)
		{
			LogObject::Info infoData;
			if (infoData.fromJson(value))
			{
				logObjectsInfo.emplace(infoData.id, infoData);
				continue;
			}
			Message messageData;
			if(messageData.fromJson(value))
			{
				messages.emplace_back(messageData);
				continue;
			}
		}

		// Group messages by context
		std::unordered_map<LoggerID, std::vector<Message>> contextMessages;
		for (const auto& message : messages)
		{
			contextMessages[message.getLoggerID()].push_back(message);
		}

		for (const auto& pair : logObjectsInfo)
		{
			LoggerID id = pair.first;
			const LogObject::Info& info = pair.second;
			if (contextMessages.find(id) != contextMessages.end())
				contexts.emplace_back(info, contextMessages[id]);
			else
				contexts.emplace_back(info, std::vector<Message>());			
		}
		return true;
	}

    QJsonArray  Import::parseLargeJson(const std::string& filePath)
	{
        QFile file(QString::fromStdString(filePath));
        if (!file.open(QIODevice::ReadOnly)) {  // No Text mode — handle bytes raw
            qWarning() << "Cannot open file";
            return QJsonArray();
        }

        QJsonArray result;
        QByteArray chunk;
        QByteArray leftover;  // carries over incomplete multi-byte sequences
        int depth = 0;
        bool inString = false;
        bool escape = false;
        bool started = false;

        auto processChunk = [&](const QByteArray& data)
            {
                for (unsigned char c : data)
                {
                    if (escape) {
                        escape = false;
                        if (started) chunk.append((char)c);
                        continue;
                    }
                    if ((char)c == '\\' && inString) {
                        escape = true;
                        if (started) chunk.append((char)c);
                        continue;
                    }
                    if ((char)c == '"') {
                        inString = !inString;
                        if (started) chunk.append((char)c);
                        continue;
                    }
                    if (inString) {
                        if (started) chunk.append((char)c);
                        continue;
                    }

                    if ((char)c == '[' && depth == 0) {
                        depth++;
                        continue;
                    }
                    if ((char)c == '{' || (char)c == '[') {
                        depth++;
                        started = true;
                        chunk.append((char)c);
                        continue;
                    }
                    if ((char)c == '}' || (char)c == ']') {
                        depth--;
                        chunk.append((char)c);

                        if (depth == 1) {
                            // Decode as UTF-8 explicitly before parsing
                            QString unicodeStr = QString::fromUtf8(chunk);
                            QByteArray utf8Clean = unicodeStr.toUtf8();

                            QJsonParseError err;
                            QJsonDocument doc = QJsonDocument::fromJson(utf8Clean, &err);
                            if (err.error == QJsonParseError::NoError) {
                                if (doc.isObject())
                                    result.append(doc.object());
                                else if (doc.isArray())
                                    result.append(doc.array());
                            }
                            else {
                                qWarning() << "Chunk parse error:" << err.errorString()
                                    << "\nChunk was:" << unicodeStr.left(200);
                            }
                            chunk.clear();
                            started = false;
                        }
                        continue;
                    }

                    if (started) chunk.append((char)c);
                }
            };

        while (!file.atEnd())
        {
            QByteArray buffer = leftover + file.read(4096);
            leftover.clear();

            // Check if buffer ends in the middle of a multi-byte UTF-8 sequence.
            // UTF-8 multi-byte sequences are 2-4 bytes:
            //   2-byte: 110xxxxx 10xxxxxx
            //   3-byte: 1110xxxx 10xxxxxx 10xxxxxx
            //   4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            // A sequence is incomplete if the last N bytes start a sequence
            // but its expected continuation bytes are missing.

            int cutAt = buffer.size();
            for (int i = buffer.size() - 1; i >= 0 && i >= buffer.size() - 4; --i)
            {
                unsigned char byte = (unsigned char)buffer[i];

                if ((byte & 0x80) == 0x00) break;          // plain ASCII — no issue

                if ((byte & 0xC0) == 0x80) continue;       // continuation byte — keep looking back

                // Found a leading byte — check if all expected continuations are present
                int expectedLen = 0;
                if ((byte & 0xE0) == 0xC0) expectedLen = 2;
                else if ((byte & 0xF0) == 0xE0) expectedLen = 3;
                else if ((byte & 0xF8) == 0xF0) expectedLen = 4;

                int actualLen = buffer.size() - i;
                if (actualLen < expectedLen) {
                    // Incomplete sequence — carry it over to next iteration
                    cutAt = i;
                }
                break;
            }

            if (cutAt < buffer.size()) {
                leftover = buffer.mid(cutAt);
                buffer = buffer.left(cutAt);
            }

            processChunk(buffer);
        }

        // Process any remaining bytes
        if (!leftover.isEmpty())
            processChunk(leftover);

        return result;
	}
}