#pragma once

#include <QString>
#include <QList>
#include <QFileInfo>

struct TagDefinition {
    QString id;
    QString name;
    QString emoji;
    QString folderName;
    QString hexColor;
};

class TagEngine {
public:
    static const QList<TagDefinition> &supportedTags();
    static const TagDefinition *findTag(const QString &query);
    static QString tagsRootDirectory();

    static bool tagFile(const QString &filePath, const QString &colorQuery);
    static bool removeTags(const QString &filePath);
    static QString getFileTag(const QString &filePath);
    static bool setupPlaces();
    static int cleanBrokenTags();

private:
    static bool setFileXattr(const QString &filePath, const QString &attrName, const QString &value);
    static QString getFileXattr(const QString &filePath, const QString &attrName);
    static bool removeFileXattr(const QString &filePath, const QString &attrName);
};
