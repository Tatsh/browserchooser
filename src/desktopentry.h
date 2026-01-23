#pragma once

#include <expected>

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

/** Represents a desktop entry (.desktop file). */
class DesktopEntry {
public:
    /** Constructor. */
    DesktopEntry() = default;
    /**
     * Constructor that parses the given .desktop file.
     * @param filename The path to the .desktop file.
     */
    explicit DesktopEntry(const QString &filename);
    /**
     * Parses the given .desktop file.
     * @param filename The path to the .desktop file.
     * @returns True if parsing was successful, false otherwise.
     */
    bool parse(const QString &filename);
    /** Checks if the desktop entry is valid. */
    [[nodiscard]] bool isValid() const {
        return valid_;
    }
    /** Returns the filename of the desktop entry. */
    [[nodiscard]] QString filename() const {
        return filename_;
    }
    /** Returns the name of the desktop entry. */
    [[nodiscard]] QString name() const;
    /** Returns the executable command of the desktop entry. */
    [[nodiscard]] QString exec() const {
        return exec_;
    }
    /** Returns the icon name of the desktop entry. */
    [[nodiscard]] QString icon() const {
        return icon_;
    }
    /** Returns the comment/description of the desktop entry. */
    [[nodiscard]] QString comment() const;
    /** Returns the startup WM class of the desktop entry. */
    [[nodiscard]] QString startupWMClass() const {
        return startupWMClass_;
    }
    /** Returns the categories of the desktop entry. */
    [[nodiscard]] QStringList categories() const {
        return categories_;
    }
    /** Returns the MIME types of the desktop entry. */
    [[nodiscard]] QStringList mimeTypes() const {
        return mimeTypes_;
    }
    /** Returns whether the desktop entry should not be displayed in menus. */
    [[nodiscard]] bool noDisplay() const {
        return noDisplay_;
    }

private:
    QString getValue(const QString &key) const;
    QString getLocalizedValue(const QString &key) const;
    QStringList getListValue(const QString &key) const;

    QString filename_;
    QString exec_;
    QString icon_;
    QString startupWMClass_;
    QStringList categories_;
    QStringList mimeTypes_;
    bool noDisplay_ = false;
    bool valid_ = false;
    QMap<QString, QString> entries_;
};

/** Reads a desktop entry from the given file. */
[[nodiscard]] std::expected<DesktopEntry, QString> readDesktopEntry(const QString &filename);
