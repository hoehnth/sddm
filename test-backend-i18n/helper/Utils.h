#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>

#include "backend/PamWorkState.h"

#include <security/pam_appl.h>

#define PAM_LOG_PREFIX "[PAM] "

// Log function call
#define LOG_FCN_CALL(classname, fcn)                \
    qDebug().noquote().nospace()                    \
        << "[" << #classname << "] " << #fcn << "()"

// Log pam function status code (rc)
#define LOG_PAM_RC(fcn, rc)      \
    qDebug().noquote().nospace() \
        << PAM_LOG_PREFIX        \
        << #fcn << ": rc = "     \
        << Utils::pamRcString(rc)

// Log pam function pointer result, e.g. from pam_getenvlist
#define LOG_PAM_PTR_RC(fcn, var)     \
    qDebug().noquote().nospace()     \
        << PAM_LOG_PREFIX            \
        << #fcn << ": result = "     \
        << (var ? "NOT NULL" : "NULL")

// For pam_get_item and pam_set_item log pam item and status code (rc)
#define LOG_PAM_ITEM(fcn, item, rc)                  \
    qDebug().noquote().nospace()                     \
        << PAM_LOG_PREFIX << #fcn                    \
        << ": item = " << Utils::pamItemString(item) \
        << ", rc = " << Utils::pamRcString(rc)

// Log pam stack work state
#define LOG_WORK_STATE(fcn, val)     \
    qDebug().noquote().nospace()     \
        << PAM_LOG_PREFIX            \
        << #fcn << ": state = "      \
        << Utils::workStateString(val)

class Utils : public QObject
{
    Q_OBJECT

public:
    explicit Utils(QObject *parent = 0);

    static const QString &msgStyleString(int msg_style);
    static const QString &workStateString(SDDM::PamWorkState::State work_state);
    static const QString pamItemString(int item_type);
    static const QString pamRcString(int rc);

    static QVector<QStringList> getPamEnvList(pam_handle_t *pam_handle);
    static QVector<QStringList> getEnvList();

    static void printEnvList(QVector<QStringList> &list, bool pam_env = false);
    static void printPamEnvList(pam_handle_t *pam_handle);
    static void printEnvList();

private:
    static QVector<QStringList> getEnvList(char **env);
};

#endif // UTILS_H
