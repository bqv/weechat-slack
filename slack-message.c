#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "../weechat-plugin.h"
#include "../slack.h"
#include "../slack-workspace.h"
#include "../slack-channel.h"
#include "../slack-user.h"
#include "../slack-message.h"

static const char format_regex[] = "<(.*?)>";
static const size_t max_groups = 2;

char *slack_message_translate_code(struct t_slack_workspace *workspace,
                                   const char *code)
{
    struct t_slack_channel *channel;
    struct t_slack_user *user;
    char *command;

    switch (code[0])
    {
        case '#': /* channel */
            channel = slack_channel_search(workspace, code+1);
            if (channel)
                return strdup(channel->name);
            else
                return strdup(code);
        case '@': /* user */
            user = slack_user_search(workspace, code+1);
            if (user)
                return strdup(user->profile.display_name);
            else
                return strdup(code);
        case '!': /* special */
            command = strdup(code);
            command[0] = '@';
            return command;
        default: /* url */
            return strdup(code);
    }
}

char *slack_message_decode(struct t_slack_workspace *workspace,
                           const char *text)
{
    int rc;
    regex_t reg;
    regmatch_t groups[max_groups];
    char msgbuf[100];
    char *decoded_text, *pos;
    const char *cursor;
    size_t offset;

    if ((rc = regcomp(&reg, format_regex, REG_EXTENDED)))
    {
        regerror(rc, &reg, msgbuf, sizeof(msgbuf));
        weechat_printf(
            workspace->buffer,
            _("%s%s: error compiling message formatting regex: %s"),
            weechat_prefix("error"), SLACK_PLUGIN_NAME,
            msgbuf);
        return strdup(text);
    }

    decoded_text = malloc(SLACK_MESSAGE_MAX_LENGTH);
    if (!decoded_text)
    {
        regfree(&reg);
        weechat_printf(
            workspace->buffer,
            _("%s%s: error allocating space for message"),
            weechat_prefix("error"), SLACK_PLUGIN_NAME);
        return strdup(text);
    }
    pos = decoded_text;
    pos[0] = '\0';

    for (cursor = text; regexec(&reg, cursor, max_groups, groups, 0) == 0; cursor += offset)
    {
        offset = groups[0].rm_eo;

        char *copy = strdup(cursor);
        if (!copy)
        {
            regfree(&reg);
            weechat_printf(
                workspace->buffer,
                _("%s%s: error allocating space for message"),
                weechat_prefix("error"), SLACK_PLUGIN_NAME);
            return strdup(text);
        }
        copy[groups[1].rm_eo] = '\0';

        char *match = strdup(copy + groups[1].rm_so);
        if (!match)
        {
            free(copy);
            regfree(&reg);
            weechat_printf(
                workspace->buffer,
                _("%s%s: error allocating space for message"),
                weechat_prefix("error"), SLACK_PLUGIN_NAME);
            return strdup(text);
        }
        copy[groups[0].rm_so] = '\0';

        char *prematch = strdup(copy);
        if (!prematch)
        {
            free(match);
            free(copy);
            regfree(&reg);
            weechat_printf(
                workspace->buffer,
                _("%s%s: error allocating space for message"),
                weechat_prefix("error"), SLACK_PLUGIN_NAME);
            return strdup(text);
        }
        free(copy);

        pos = strncat(decoded_text, prematch,
                SLACK_MESSAGE_MAX_LENGTH - strlen(decoded_text) - 1);
        free(prematch);

        char *replacement = slack_message_translate_code(workspace, match);
        free(match);

        pos = strncat(decoded_text, replacement,
                SLACK_MESSAGE_MAX_LENGTH - strlen(decoded_text) - 1);
        free(replacement);
    }
    pos = strncat(decoded_text, cursor,
            SLACK_MESSAGE_MAX_LENGTH - strlen(decoded_text) - 1);

    regfree(&reg);
    return decoded_text;
}
