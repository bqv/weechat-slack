// This Source Code Form is subject to the terms of the Mozilla Public
// License, version 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _SLACK_API_MESSAGE_SLACKBOT_RESPONSE_H_
#define _SLACK_API_MESSAGE_SLACKBOT_RESPONSE_H_

int slack_api_message_slackbot_response(
        struct t_slack_workspace *workspace,
        json_object *message);

#endif /*SLACK_API_MESSAGE_SLACKBOT_RESPONSE_H*/
