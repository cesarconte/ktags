#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFile>
#include "../src/tagengine.h"

class TestTagEngine : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testFindTag();
    void testTagFileAndGetTag();
    void testSwitchTag();
    void testRemoveTag();
    void testCleanBrokenTags();
};

void TestTagEngine::testFindTag()
{
    const auto *red = TagEngine::findTag("red");
    QVERIFY(red != nullptr);
    QCOMPARE(red->name, QStringLiteral("Rojo"));
    QCOMPARE(red->emoji, QStringLiteral("🔴"));

    const auto *blue = TagEngine::findTag("Azul");
    QVERIFY(blue != nullptr);
    QCOMPARE(blue->id, QStringLiteral("blue"));

    const auto *green = TagEngine::findTag("🟢");
    QVERIFY(green != nullptr);
    QCOMPARE(green->id, QStringLiteral("green"));

    QVERIFY(TagEngine::findTag("invalid_color") == nullptr);
}

void TestTagEngine::testTagFileAndGetTag()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("test content");
    tempFile.close();

    const QString path = tempFile.fileName();
    QVERIFY(TagEngine::tagFile(path, "red"));

    QString currentTag = TagEngine::getFileTag(path);
    QCOMPARE(currentTag, QStringLiteral("red"));

    // Verify symlink in tags directory
    const QString linkPath = TagEngine::tagsRootDirectory() + "/🔴 Rojo/" + QFileInfo(path).fileName();
    QVERIFY(QFile::exists(linkPath));

    // Cleanup
    TagEngine::removeTags(path);
}

void TestTagEngine::testSwitchTag()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("switch test");
    tempFile.close();

    const QString path = tempFile.fileName();
    const QString fileName = QFileInfo(path).fileName();

    QVERIFY(TagEngine::tagFile(path, "red"));
    QCOMPARE(TagEngine::getFileTag(path), QStringLiteral("red"));

    // Switch to blue
    QVERIFY(TagEngine::tagFile(path, "blue"));
    QCOMPARE(TagEngine::getFileTag(path), QStringLiteral("blue"));

    // Verify red link removed and blue link exists
    const QString redLink = TagEngine::tagsRootDirectory() + "/🔴 Rojo/" + fileName;
    const QString blueLink = TagEngine::tagsRootDirectory() + "/🔵 Azul/" + fileName;

    QVERIFY(!QFile::exists(redLink));
    QVERIFY(QFile::exists(blueLink));

    TagEngine::removeTags(path);
}

void TestTagEngine::testRemoveTag()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("remove test");
    tempFile.close();

    const QString path = tempFile.fileName();
    const QString fileName = QFileInfo(path).fileName();

    QVERIFY(TagEngine::tagFile(path, "green"));
    QCOMPARE(TagEngine::getFileTag(path), QStringLiteral("green"));

    QVERIFY(TagEngine::removeTags(path));
    QVERIFY(TagEngine::getFileTag(path).isEmpty());

    const QString greenLink = TagEngine::tagsRootDirectory() + "/🟢 Verde/" + fileName;
    QVERIFY(!QFile::exists(greenLink));
}

void TestTagEngine::testCleanBrokenTags()
{
    const QString brokenTarget = "/tmp/non_existent_file_for_test_12345.xyz";
    const QString linkPath = TagEngine::tagsRootDirectory() + "/🔴 Rojo/non_existent_file_for_test_12345.xyz";

    QDir().mkpath(TagEngine::tagsRootDirectory() + "/🔴 Rojo");
    QFile::link(brokenTarget, linkPath);

    int cleaned = TagEngine::cleanBrokenTags();
    QVERIFY(cleaned >= 1);
    QVERIFY(!QFile::exists(linkPath));
}

QTEST_MAIN(TestTagEngine)
#include "test_tagengine.moc"
