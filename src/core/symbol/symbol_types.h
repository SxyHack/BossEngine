#pragma once

#include <QString>
#include <QList>

struct IndexName
{
	QString name;
	quint32 index;

	bool operator<(const IndexName& b) const
	{
		return name.compare(b.name) < 0;
	}
};


typedef QList<IndexName> LIST_INDEX_NAME;
