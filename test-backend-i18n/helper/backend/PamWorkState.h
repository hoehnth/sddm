#ifndef PAMWORKSTATE_H
#define PAMWORKSTATE_H

namespace SDDM {

    class PamData;
    class PamHandle;

    class PamWorkState
    {
        // allow them full access on value
        friend PamData;
        friend PamHandle;

    public:

        PamWorkState() { }

        // tracking state of PAM stack progress
        enum State {
            STATE_INITIAL = 0,      ///< pam stack not started, no handle
            STATE_STARTED,          ///< pam_start ok
            STATE_AUTHENTICATE,     ///< pam_authentication running, authentication requested
            STATE_CHANGEAUTHTOK,    ///< change auth token requested by pam_authentication
            STATE_AUTHENTICATED,    ///< pam_authentication or pam_chauthtok ok
            STATE_AUTHORIZED,       ///< pam_acct_mgmt ok
            STATE_CREDITED,         ///< pam_setcred ok
            STATE_SESSION_STARTED,  ///< pam_open_session ok
            STATE_FINISHED,         ///< pam_close_session or pam_end ok
            _STATE_LAST
        };

    private:
        State value { STATE_INITIAL }; ///< work state of pam stack
    };
}

#endif // PAMWORKSTATE_H
