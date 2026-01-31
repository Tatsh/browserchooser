/** @file */
#pragma once

#include <expected>

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

/** Represents a desktop entry (@c .desktop file). */
class DesktopEntry {
public:
    /** Constructor. */
    DesktopEntry() = default; // LCOV_EXCL_LINE
    /**
     * Constructor that parses the given @c .desktop file.
     * @param filename The path to the @c .desktop file.
     */
    explicit DesktopEntry(const QString &filename);
    /**
     * Parses the given @c .desktop file.
     * @param filename The path to the @c .desktop file.
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
    /** Returns the executable name (first token of @c Exec) for @c PATH lookup. */
    [[nodiscard]] QString executableName() const;
    /** Returns the icon name of the desktop entry. */
    [[nodiscard]] QString icon() const {
        return icon_;
    }
    /** Returns the comment/description of the desktop entry. */
    [[nodiscard]] QString comment() const;
    // LCOV_EXCL_START
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
    // LCOV_EXCL_STOP
    /** Returns whether the desktop entry has @c NoDisplay set (hidden from menus). */
    [[nodiscard]] bool noDisplay() const {
        return noDisplay_;
    }
#ifdef Q_OS_MAC
    /**
     * Parses a macOS .app bundle (Contents/Info.plist).
     * @param bundlePath Path to the .app bundle.
     * @returns True if parsing was successful, false otherwise.
     */
    bool parseAppBundle(const QString &bundlePath);
#elif defined(Q_OS_WIN)
    /**
     * Parses a Windows executable path into a synthetic entry (name from exe).
     * @param exePath Full path to the .exe.
     * @returns True if the file exists and parsing was successful, false otherwise.
     */
    bool parseFromExecutable(const QString &exePath);
#endif

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

/** Error code for reading a desktop entry. */
enum class DesktopEntryError { ParseFailed };

/**
 * Reads a desktop entry from the given file.
 * @param filename @c Path to the @c .desktop file.
 * @return The parsed DesktopEntry on success, or an error.
 */
[[nodiscard]] std::expected<DesktopEntry, DesktopEntryError>
readDesktopEntry(const QString &filename);
