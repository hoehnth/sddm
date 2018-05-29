#ifndef PROMPT_H
#define PROMPT_H

#include <QtCore/QObject>

namespace SDDM {
    class AuthRequest;
    class Prompt;

    class AuthPrompt : public QObject {
        Q_OBJECT
    public:
        virtual ~AuthPrompt();
        /**
        * \note In hex not for binary operations but to leave space for adding other codes
        */
        enum Type {
            NONE = 0x0000,           ///< No type
            UNKNOWN = 0x0001,        ///< Unknown type
            LOGIN_USER = 0x0010,     ///< On logging in: The username
            LOGIN_PASSWORD = 0x0020, ///< On logging in: The password
            CHANGE_PASSWORD = 0x0040 ///< On changing the password: The current, new or repeat password
        };
        /**
        * @return the type of the prompt
        */
        Type type() const;
        /**
        * @warning the preferred way is to use \ref type
        * @return message from the stack
        */
        QString message() const;
        /**
        * @return true if user's input should not be shown in readable form
        */
        bool hidden() const;
        /**
         * Public getter for the response data.
         * The property is write-only though, so it returns garbage.
         * Contained only to keep the MOC parser happy.
         * @warning do not use, doesn't return valid data
         * @return empty byte array
         */
        QByteArray responseFake();
        /**
        * Setter for the response data
        * @param r data entered by the user
        */
        void setResponse(const QByteArray &r);
        /**
         * @brief get string representation of AuthPrompt type, see enum @ref Type
         * @note for debug logging
         */
        static const QString &typeToString(int type);

    private:
        AuthPrompt(const Prompt *prompt, AuthRequest *parent = 0);
        QByteArray response() const;
        friend class AuthRequest;
        class Private;
        Private *d { nullptr };
    };
}

#endif //PROMPT_H
