#include "tagengine.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDomDocument>
#include <QUrl>
#include <QDebug>
#include <sys/xattr.h>

static const QList<TagDefinition> s_supportedTags = {
    {"red", "Rojo", "🔴", "🔴 Rojo", "#FF3B30"},
    {"orange", "Naranja", "🟠", "🟠 Naranja", "#FF9500"},
    {"yellow", "Amarillo", "🟡", "🟡 Amarillo", "#FFCC00"},
    {"green", "Verde", "🟢", "🟢 Verde", "#34C759"},
    {"blue", "Azul", "🔵", "🔵 Azul", "#007AFF"},
    {"purple", "Morado", "🟣", "🟣 Morado", "#AF52DE"},
    {"gray", "Gris", "⚪", "⚪ Gris", "#8E8E93"}
};

const QList<TagDefinition> &TagEngine::supportedTags()
{
    return s_supportedTags;
}

const TagDefinition *TagEngine::findTag(const QString &query)
{
    const QString q = query.trimmed().toLower();
    for (const auto &tag : s_supportedTags) {
        if (tag.id == q || tag.name.toLower() == q || tag.emoji == query.trimmed() || tag.folderName.toLower() == q) {
            return &tag;
        }
    }
    return nullptr;
}

QString TagEngine::tagsRootDirectory()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (base.isEmpty()) {
        base = QDir::homePath() + "/.local/share";
    }
    return base + "/tags";
}

bool TagEngine::setFileXattr(const QString &filePath, const QString &attrName, const QString &value)
{
    const QByteArray pathBa = filePath.toLocal8Bit();
    const QByteArray attrBa = attrName.toLocal8Bit();
    const QByteArray valBa = value.toUtf8();

    int res = setxattr(pathBa.constData(), attrBa.constData(), valBa.constData(), valBa.size(), 0);
    return res == 0;
}

QString TagEngine::getFileXattr(const QString &filePath, const QString &attrName)
{
    const QByteArray pathBa = filePath.toLocal8Bit();
    const QByteArray attrBa = attrName.toLocal8Bit();
    char buffer[1024];

    ssize_t len = getxattr(pathBa.constData(), attrBa.constData(), buffer, sizeof(buffer) - 1);
    if (len >= 0) {
        buffer[len] = '\0';
        return QString::fromUtf8(buffer, len);
    }
    return QString();
}

bool TagEngine::removeFileXattr(const QString &filePath, const QString &attrName)
{
    const QByteArray pathBa = filePath.toLocal8Bit();
    const QByteArray attrBa = attrName.toLocal8Bit();
    return removexattr(pathBa.constData(), attrBa.constData()) == 0;
}

bool TagEngine::tagFile(const QString &filePath, const QString &colorQuery)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }

    const TagDefinition *tag = findTag(colorQuery);
    if (!tag) {
        return false;
    }

    const QString absPath = fileInfo.absoluteFilePath();
    const QString fileName = fileInfo.fileName();

    // 1. Remove previous tag symlinks if any
    removeTags(absPath);

    // 2. Set Linux/FreeDesktop standard extended attributes
    setFileXattr(absPath, "user.color.tag", tag->id);
    setFileXattr(absPath, "user.xdg.tags", tag->name);

    // 3. Create target folder and symlink
    const QString targetDir = tagsRootDirectory() + "/" + tag->folderName;
    QDir().mkpath(targetDir);

    const QString linkPath = targetDir + "/" + fileName;
    if (QFile::exists(linkPath)) {
        QFile::remove(linkPath);
    }

    return QFile::link(absPath, linkPath);
}

bool TagEngine::removeTags(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    const QString fileName = fileInfo.fileName();
    const QString absPath = fileInfo.absoluteFilePath();

    // Remove extended attributes
    removeFileXattr(absPath, "user.color.tag");
    removeFileXattr(absPath, "user.xdg.tags");

    // Remove from all color tag directories
    const QString root = tagsRootDirectory();
    for (const auto &tag : s_supportedTags) {
        const QString linkPath = root + "/" + tag.folderName + "/" + fileName;
        QFileInfo linkInfo(linkPath);
        if (linkInfo.exists() || linkInfo.isSymLink()) {
            if (linkInfo.isSymLink()) {
                if (linkInfo.symLinkTarget() == absPath || !QFile::exists(linkInfo.symLinkTarget())) {
                    QFile::remove(linkPath);
                }
            }
        }
    }

    return true;
}

QString TagEngine::getFileTag(const QString &filePath)
{
    return getFileXattr(filePath, "user.color.tag");
}

int TagEngine::cleanBrokenTags()
{
    int cleaned = 0;
    const QString root = tagsRootDirectory();
    for (const auto &tag : s_supportedTags) {
        QDir dir(root + "/" + tag.folderName);
        if (!dir.exists()) continue;

        const QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (const auto &fi : list) {
            if (fi.isSymLink() && !QFile::exists(fi.symLinkTarget())) {
                QFile::remove(fi.absoluteFilePath());
                cleaned++;
            }
        }
    }
    return cleaned;
}

bool TagEngine::setupPlaces()
{
    QString placesPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/user-places.xbel";
    if (!QFile::exists(placesPath)) {
        placesPath = QDir::homePath() + "/.local/share/user-places.xbel";
    }

    QFile file(placesPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "xbel") {
        return false;
    }

    const QString rootTags = tagsRootDirectory();
    QDir().mkpath(rootTags);

    // Check existing bookmarks to avoid duplicates
    QSet<QString> existingUrls;
    QDomNodeList bookmarks = root.elementsByTagName("bookmark");
    for (int i = 0; i < bookmarks.count(); ++i) {
        QDomElement bm = bookmarks.at(i).toElement();
        existingUrls.insert(bm.attribute("href"));
    }

    bool added = false;
    for (const auto &tag : s_supportedTags) {
        const QString folderPath = rootTags + "/" + tag.folderName;
        QDir().mkpath(folderPath);

        const QString url = QUrl::fromLocalFile(folderPath).toString();
        if (existingUrls.contains(url)) {
            continue;
        }

        QDomElement bookmark = doc.createElement("bookmark");
        bookmark.setAttribute("href", url);

        QDomElement titleElem = doc.createElement("title");
        QDomText titleText = doc.createTextNode(tag.folderName);
        titleElem.appendChild(titleText);
        bookmark.appendChild(titleElem);

        QDomElement infoElem = doc.createElement("info");

        // Freedesktop icon
        QDomElement metaFd = doc.createElement("metadata");
        metaFd.setAttribute("owner", "http://freedesktop.org");
        QDomElement iconElem = doc.createElement("bookmark:icon");
        iconElem.setAttribute("name", "tag");
        metaFd.appendChild(iconElem);
        infoElem.appendChild(metaFd);

        // KDE Places metadata
        QDomElement metaKde = doc.createElement("metadata");
        metaKde.setAttribute("owner", "http://www.kde.org");

        QDomElement idElem = doc.createElement("ID");
        QDomText idText = doc.createTextNode(QString("tag_%1").arg(tag.id));
        idElem.appendChild(idText);
        metaKde.appendChild(idElem);

        QDomElement isSystem = doc.createElement("isSystemItem");
        QDomText sysText = doc.createTextNode("false");
        isSystem.appendChild(sysText);
        metaKde.appendChild(isSystem);

        infoElem.appendChild(metaKde);
        bookmark.appendChild(infoElem);

        root.appendChild(bookmark);
        added = true;
    }

    if (added) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        file.write(doc.toByteArray(2));
        file.close();
    }

    return true;
}
